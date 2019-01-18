#include "downloadmanager.h"

#include "utilities/utils.h"

#include "downloadcollectionmodel.h"
#include "settings_declaration.h"
#include "global_functions.h"
#include "torrentmanager.h"
#include "torrentslistener.h"

#include <algorithm>
#include <functional>
#include <numeric>
#include <climits>

using global_functions::GetMaximumNumberLoadsActual;
using global_functions::GetTrafficLimitActual;

DownloadManager::DownloadManager(QObject* parent)
    : QObject(parent), m_bStopDLManager(false), m_kbps(0)
{
    const auto model = &DownloadCollectionModel::instance();

    VERIFY(connect(model, SIGNAL(signalDeleteURLFromModel(int, DownloadType::Type, int)),    SLOT(on_deleteTaskWithID(int, DownloadType::Type, int))));
    VERIFY(connect(model, SIGNAL(signalPauseDownloadItemWithID(int, DownloadType::Type)),    SLOT(on_pauseTaskWithID(int, DownloadType::Type))));
    VERIFY(connect(model, SIGNAL(signalContinueDownloadItemWithID(int, DownloadType::Type)), SLOT(startLoad())));
    VERIFY(connect(model, SIGNAL(onDownloadStarted()),                                       SLOT(siftDownloads())));
    VERIFY(connect(model, SIGNAL(onItemsReordered()),                                        SLOT(onItemsReordered())));

#ifdef ALLOW_TRAFFIC_CONTROL
    VERIFY(connect(model, &DownloadCollectionModel::signalModelUpdated, this, &DownloadManager::UpdateSpeedLimits));
    setSpeedLimit(GetTrafficLimitActual());
#endif // ALLOW_TRAFFIC_CONTROL

    VERIFY(connect(&TorrentsListener::instance(), SIGNAL(signalTryNewtask()), SLOT(tryNewTask())));
}

DownloadManager::~DownloadManager()
{
    qDeleteAll(m_activeTasks);
    m_activeTasks.clear();

    qDeleteAll(m_prepareTasks);
    m_prepareTasks.clear();

    qDebug() << "DLManager::~DLManager()";
}


void DownloadManager::startLoad()
{
    pushQueuedDownloads();

    if (!isPossibleStartDownload())
    {
        return;
    }

    if (!m_prepareTasks.isEmpty())
    {
        auto taskIt = std::find_if(m_prepareTasks.begin(), m_prepareTasks.end(), std::mem_fn(&DownloadTask::ready_to_download));
        if (taskIt != m_prepareTasks.end())
        {
            startTaskDownload((*taskIt)->task_id());
            return;
        }
    }

    while (auto it = DownloadCollectionModel::instance().findItem(
        [](const TreeItem* te)
        {
            return te->getStatus() == ItemDC::eQUEUED ||
                (te->getStatus() == ItemDC::eERROR && te->getWaitingTime() > 0 
                    && QDateTime::currentDateTimeUtc() > te->statusLastChanged().addSecs(te->getWaitingTime()));
        }))
    {
        auto l_item = it->copyItemDC();
        if (!l_item.isValid() || createNewTask(l_item))
            break;
    }
}

void DownloadManager::onDownloadFinished(int a_id)
{
    if (!m_bStopDLManager)
    {
        TasksMap::iterator it = m_activeTasks.find(a_id);
        if (m_activeTasks.end() != it)
        {
            DownloadTask* task = it.value();

            if (task->is_torrent_file())//DownloadType::TorrentFile == DownloadType::determineType(task->fileName()))
            {
                addItemsToModel(QStringList() << task->fileName(), DownloadType::TorrentFile);
            }

            killTask(task);
            m_activeTasks.erase(it);
        }

        it = m_prepareTasks.find(a_id);
        if (m_prepareTasks.end() != it)
        {
            DownloadTask* task = it.value();
            killTask(task);
            m_prepareTasks.erase(it);
        }
    }
#ifdef ALLOW_TRAFFIC_CONTROL
    UpdateSpeedLimits();
#endif // ALLOW_TRAFFIC_CONTROL
}

void DownloadManager::tryNewTask()
{
    if (!m_bStopDLManager)
    {
        startLoad();
        emit updateButtons();
    }
}

bool DownloadManager::createNewTask(ItemDC& a_item)
{
    qDebug() << "DLManager::createNewTask() id = " << a_item.getID();
    a_item.setStatus(ItemDC::eCONNECTING);
    DownloadCollectionModel::instance().on_statusChange(a_item);
    DownloadType::Type dlType = (DownloadType::Unknown == a_item.downloadType()) 
        ? DownloadType::determineType(a_item.initialURL()) : a_item.downloadType();

    if (DownloadType::isDirectDownload(dlType))
    {
        DownloadTask* dlTask = new DownloadTask(a_item.getID(), a_item.initialURL(), this);
        VERIFY(connect(dlTask, SIGNAL(signalDownloadFinished(int)),                  SLOT(onDownloadFinished(int))));
        VERIFY(connect(dlTask, SIGNAL(signalTryNewtask()),                           SLOT(tryNewTask())));
        VERIFY(connect(dlTask, SIGNAL(readyToDownload(int)),                         SLOT(startTaskDownload(int))));
        VERIFY(connect(dlTask, SIGNAL(needLogin(utilities::ICredentialsRetriever*)), SIGNAL(needLogin(utilities::ICredentialsRetriever*))));

        dlTask->start();
        m_prepareTasks[a_item.getID()] = dlTask;
        return true;
    }
    else if (DownloadType::isTorrentDownload(dlType))
    {
        if (TorrentManager::Instance()->resumeTorrent(a_item.getID()))
        {
            return true;
        }
        a_item.setStatus(ItemDC::eERROR);
        a_item.setWaitingTime(0); // no recovery
        DownloadCollectionModel::instance().on_statusChange(a_item);
        DownloadCollectionModel::instance().on_waitingTimeChange(a_item);
    }

    return false;
}

void DownloadManager::startTaskDownload(int id)
{
    TasksMap::iterator it = m_prepareTasks.find(id);
    if (m_prepareTasks.end() != it)
    {
        QString hoster = global_functions::GetNormalizedDomain((*it)->url());
        DownloadTask* task = it.value();

        m_activeTasks[id] = task;
        m_prepareTasks.erase(it);

        qDebug() << "DLManager::startTaskDownload() id = "<< id;
        task->download();
    }
    else
    {
        qDebug("Can't find task in m_prepareTasks.count=%d DLManager::startTaskDownload() id = %d", m_prepareTasks.count(), id);
    }

#ifdef ALLOW_TRAFFIC_CONTROL
    UpdateSpeedLimits();
#endif // ALLOW_TRAFFIC_CONTROL
}

void DownloadManager::on_deleteTaskWithID(int a_id, DownloadType::Type type, int deleteWithFiles)
{
    if (DownloadType::isDirectDownload(type))
    {
        TasksMap::iterator it = m_activeTasks.find(a_id);
        if (m_activeTasks.end() != it)
        {
            qDebug() << "DLManager::on_deleteTaskWithID() id = " << a_id;
            (*it)->cancelTask();
        }

        auto myPrepare = m_prepareTasks.find(a_id);
        if (myPrepare != m_prepareTasks.end())
        {
            qDebug() << "DLManager::on_deleteTaskWithID() killing prepareTask with id = " << a_id;
            killTask(*myPrepare);
            m_prepareTasks.erase(myPrepare);
        }
    }

    startLoad();
}

void DownloadManager::on_pauseTaskWithID(int a_id, DownloadType::Type type)
{
    if (DownloadType::isDirectDownload(type))
    {
        TasksMap::iterator it = m_activeTasks.find(a_id);
        if (m_activeTasks.end() != it)
        {
            qDebug() << "DLManager::on_pauseTaskWithID() id = " << a_id;
            (*it)->interruptTask();
            killTask(*it);
            m_activeTasks.erase(it);
        }

        auto myPrepare = m_prepareTasks.find(a_id);
        if (myPrepare != m_prepareTasks.end())
        {
            qDebug() << "DLManager::on_pauseTaskWithID() prepareTask with id = " << a_id;
            (*myPrepare)->interruptTask();
            killTask(*myPrepare);
            m_prepareTasks.erase(myPrepare);
        }
    }

    startLoad();
}

bool DownloadManager::isPossibleStartDownload()
{
    using namespace app_settings;

    if (m_bStopDLManager)
    {
        return false;
    }

    const int maxDl = GetMaximumNumberLoadsActual();

    int activeTasks(0);
    DownloadCollectionModel::instance().forAll([this, &activeTasks](const TreeItem & ti)
    {
        if (isActiveTask(ti))
        {
            ++activeTasks;
        }
    });

    bool result = (activeTasks < maxDl);
    if (result && QSettings().value(IsTrafficLimited, IsTrafficLimited_Default).toBool())
    {
        const float sumSpeed = std::accumulate(m_activeTasks.constBegin(), m_activeTasks.constEnd(), 0.f,
                                         [](float v, const DownloadTask * task) { return v + task->getSpeed(); });
        result = sumSpeed < QSettings().value(TrafficLimitKbs, TrafficLimitKbs_Default).toInt();
    }

    return result;
}

void DownloadManager::stopDLManager()
{
    m_bStopDLManager = true;
}


void DownloadManager::killTask(DownloadTask* task)
{
    qDebug() << "Deleting task " << task->task_id();
    task->deleteLater();
}

void DownloadManager::prepareToExit()
{
    qDebug() << Q_FUNC_INFO;

    stopDLManager();

    // Shutting down torrent (if running)
    if (TorrentManager::isSessionExists())
    {
        TorrentManager::Instance()->close();
    }

    DownloadCollectionModel::instance().saveToFile();

    TorrentManager::dispose();
}

#ifdef ALLOW_TRAFFIC_CONTROL
void DownloadManager::setSpeedLimit(int kbps)
{
    if (m_kbps == kbps)
    {
        return;
    }

    Q_ASSERT(kbps >= 0);

    m_kbps = kbps;

    UpdateSpeedLimits();
}

void DownloadManager::UpdateSpeedLimits()
{
    QTimer::singleShot(1000, this, &DownloadManager::UpdateSpeedLimitsImpl);
}

const int RESERVE_TASK_LIMIT = 2;

void DownloadManager::UpdateSpeedLimitsImpl()
{
    if (m_activeTasks.empty())
    {
        return;
    }

    if (m_kbps <= 0)
    {
        for (DownloadTask* task : qAsConst(m_activeTasks))
            task->setSpeedLimit(0);
    }
    else
    {
        std::vector<DownloadTask*> sortedTasks(m_activeTasks.size());
        std::partial_sort_copy(
            m_activeTasks.constBegin(), m_activeTasks.constEnd(), sortedTasks.begin(), sortedTasks.end(),
            [](DownloadTask * t1, DownloadTask * t2) {return t1->priority_level() < t2->priority_level(); });

        int kbps = m_kbps - m_activeTasks.size() * RESERVE_TASK_LIMIT; // reserve n kb to each task

        for (DownloadTask* task : qAsConst(sortedTasks))
        {
            int curLimit = std::max(RESERVE_TASK_LIMIT, kbps); // it may take all traffic when it will start

            qDebug() << QString("Task with id %1 and priority %2  gets limit: %3")
                             .arg(task->task_id()).arg(task->priority_level()).arg(curLimit);
            task->setSpeedLimit(curLimit);
            kbps -= curLimit;
        }
    }
}

#endif // ALLOW_TRAFFIC_CONTROL

void DownloadManager::addItemsToModel(const QStringList& urls, DownloadType::Type type)
{
    DownloadCollectionModel::instance().addItemsToModel(urls, type);
    startLoad();
    DownloadCollectionModel::instance().queueSaveToFile();
}

void DownloadManager::siftDownloads()
{
    const int maxDl = GetMaximumNumberLoadsActual();

    int activeTasks(0);
    DownloadCollectionModel::instance().forAll([this, &activeTasks, maxDl](TreeItem & ti)
    {
        if (isActiveTask(ti))
        {
            if (activeTasks < maxDl)
            {
                ++activeTasks;
            }
            else
            {
                DownloadCollectionModel::instance().deactivateDownloadItem(&ti);
            }
        }
        else if (ItemDC::eSTALLED == ti.getStatus() && activeTasks >= maxDl)
        {
            DownloadCollectionModel::instance().deactivateDownloadItem(&ti);
        }
    });

    tryNewTask();
}

void DownloadManager::pushQueuedDownloads()
{
    const int maxDl = GetMaximumNumberLoadsActual();

    int firstClassTasks(0);
    DownloadCollectionModel::instance().findItem([this, &firstClassTasks, maxDl](const TreeItem * ti)
    {
        if (firstClassTasks >= maxDl)
        {
            return true;
        }

        if (ItemDC::eQUEUED == ti->getStatus())
        {
            auto tempItemDC = ti->copyItemDC();
            createNewTask(tempItemDC);
            ++firstClassTasks;
        }
        else if (isActiveTask(*ti))
        {
            ++firstClassTasks;
        }

        return false;
    });
}

void DownloadManager::onItemsReordered()
{
    pushQueuedDownloads();

    const int maxDl = GetMaximumNumberLoadsActual();

    int activeTasks(0);
    DownloadCollectionModel::instance().forAll([this, &activeTasks, maxDl](TreeItem & ti)
    {
        if (ItemDC::eDOWNLOADING == ti.getStatus())
        {
            if (activeTasks < maxDl)
            {
                ++activeTasks;
            }
        }
        else if (ItemDC::eSTALLED == ti.getStatus() && activeTasks >= maxDl)
        {
            DownloadCollectionModel::instance().deactivateDownloadItem(&ti);
        }
    });
}

bool DownloadManager::isActiveTask(const TreeItem& ti) const
{
    if (DownloadType::isDirectDownload(ti.downloadType()))
    {
        return m_activeTasks.find(ti.getID()) != m_activeTasks.end();
    }

    const auto status = ti.getStatus();
    return (ItemDC::eDOWNLOADING == status
            || ItemDC::eCONNECTING == status
            || ItemDC::eSTARTING == status
            || (ItemDC::eSTALLED == status && QDateTime::currentDateTimeUtc() < ti.statusLastChanged().addSecs(30)));
}

#pragma once

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QAbstractItemModel>

#include "downloadtask.h"
#include "treeitem.h"
#include "utilities/credsretriever.h"
#include "downloadtype.h"

class DownloadCollectionModel;

class DownloadManager: public QObject
{
    Q_OBJECT
public:

    DownloadManager(QObject* parent = nullptr);
    ~DownloadManager();

    inline bool isWorking() const { return !(m_activeTasks.isEmpty() && m_prepareTasks.isEmpty()); }
    void prepareToExit(); // may be called from another thread

    void addItemsToModel(const QStringList& urls, DownloadType::Type type);

#ifdef ALLOW_TRAFFIC_CONTROL
    void setSpeedLimit(int kbps);
#endif // ALLOW_TRAFFIC_CONTROL

public Q_SLOTS:
    void startLoad();
    void on_deleteTaskWithID(int a_id, DownloadType::Type type, int deletWithFiles = 0);
    void on_pauseTaskWithID(int a_id, DownloadType::Type type);
    void siftDownloads();
    void onItemsReordered();

Q_SIGNALS:
    void updateButtons();
    void needLogin(utilities::ICredentialsRetriever*);

private Q_SLOTS:
    void onDownloadFinished(int ID);
    void tryNewTask();
    void startTaskDownload(int id);
#ifdef ALLOW_TRAFFIC_CONTROL
    void UpdateSpeedLimits();
    void UpdateSpeedLimitsImpl();
#endif // ALLOW_TRAFFIC_CONTROL

private:
    typedef QMap<int, DownloadTask*> TasksMap;
    TasksMap m_activeTasks;
    TasksMap m_prepareTasks;
    bool m_bStopDLManager;
    int m_kbps;

    bool isPossibleStartDownload();
    bool createNewTask(ItemDC& a_item);

    void killTask(DownloadTask* task);
    void stopDLManager();

    bool isActiveTask(const TreeItem& ti) const;
    void pushQueuedDownloads();
};

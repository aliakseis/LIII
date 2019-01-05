#include "torrentdetailsform.h"
#include "ui_torrentdetailsform.h"
#include "torrentcontentfiltermodel.h"
#include "torrentcontentmodel.h"
#include "proplistdelegate.h"
#include "utilities/utils.h"

#include <libtorrent/torrent_status.hpp>

#include "peersinfomodel.h"
#include "peersinfoform_proxy_model.h"

#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QDebug>

#include <QKeyEvent>
#include "branding.hxx"

#include <algorithm>

TorrentDetailsForm::TorrentDetailsForm(const libtorrent::torrent_handle& handle, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::TorrentDetailsForm),
    m_contentModel(nullptr),
    m_torrentInfo(nullptr),
    m_torrentAddParams(nullptr),
    m_torrentHandle(handle),
    m_PeersInfomodel(nullptr),
    m_PeersInfoproxy(nullptr),
    m_updateTimeId(-1)
{
    ui->setupUi(this);
    // Initialize using torrent handle
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);
    initialize();
}

TorrentDetailsForm::~TorrentDetailsForm()
{
    delete ui;
    delete m_PeersInfomodel;
}

void TorrentDetailsForm::initialize()
{
    // setup common torrent info
    ui->savePathEdit->setText("");

    // using torrent info from handle
    if (m_torrentHandle.is_valid())
    {
        m_torrentInfo = &m_torrentHandle.get_torrent_info();
        setSavePath(QString::fromStdString(m_torrentHandle.save_path()));
    }
    else if (m_torrentAddParams)
    {
        m_torrentInfo = &(*m_torrentAddParams->ti);
        setSavePath(QString::fromStdString(m_torrentAddParams->save_path));
    }

    // Setting up other saving path stuffs
    VERIFY(connect(ui->savePathButton, SIGNAL(clicked()), SLOT(browseSavePath())));
    VERIFY(connect(ui->savePathEdit, SIGNAL(textEdited(QString)), SLOT(savePathEdited(QString))));

    Q_ASSERT(m_torrentInfo->is_valid());

    ui->lblName->setText(QString::fromStdString(m_torrentInfo->name()));
    QString comment = QString::fromStdString(m_torrentInfo->comment());
    ui->lblComment->setText(comment.replace('\n', ' '));

    initTorrentContentTab();

    updateDiskSpaceLabel();

    initPeersInfoTab();
}


void TorrentDetailsForm::initPeersInfoTab()
{
    m_PeersInfomodel = new PeersInfoModel(m_torrentHandle);
    m_PeersInfoproxy = new PeersSortFilterProxyModel(this);
    m_PeersInfoproxy->setSourceModel(m_PeersInfomodel);
    m_PeersInfoproxy->setDynamicSortFilter(true);
    ui->peersView->setRootIsDecorated(false);
    ui->peersView->setModel(m_PeersInfoproxy);
    ui->peersView->setSortingEnabled(true);
    VERIFY(connect(ui->peersView->header(), SIGNAL(sectionDoubleClicked(int)), SLOT(adaptColumns(int))));
    ui->peersView->setSortingEnabled(true);
    m_updateTimeId = startTimer(5000);

#ifndef DEVELOPER_FEATURES
    ui->peersView->header()->setSectionHidden(PeersInfoModel::Country, true);
#endif
    ui->peersView->header()->resizeSection(PeersInfoModel::ClientName, 200); // 250 in spec (too large)
    ui->peersView->header()->resizeSection(PeersInfoModel::IpAddress, 120);
    ui->peersView->header()->resizeSection(PeersInfoModel::PeerProgress, 80);
    ui->peersView->header()->resizeSection(PeersInfoModel::SpeedUpload, 63);
    ui->peersView->header()->resizeSection(PeersInfoModel::SpeedDownload, 63);
}

void TorrentDetailsForm::initTorrentContentTab()
{
    m_contentModel = new TorrentContentFilterModel(this);
    m_contentModel->model()->setSavePath(savePath());
    ui->treeTorrentContent->setModel(m_contentModel);

    auto* contentDelegate = new PropListDelegate(this);
    ui->treeTorrentContent->setItemDelegate(contentDelegate);
    VERIFY(connect(ui->treeTorrentContent, SIGNAL(expanded(QModelIndex)), SLOT(onItemExpanded(QModelIndex))));

    // List files in torrent
    m_contentModel->model()->setupModelData(*m_torrentInfo, (m_torrentHandle.is_valid() ? m_torrentHandle.status(0) : libtorrent::torrent_status()));

    qDebug() << m_contentModel->model()->rowCount();

    if (m_torrentHandle.is_valid())
    {
        m_contentModel->model()->updateFilesPriorities(m_torrentHandle.file_priorities());

        std::vector<boost::int64_t> fp;
        m_torrentHandle.file_progress(fp);
        m_contentModel->model()->updateFilesProgress(fp);
    }
    else if (m_torrentAddParams)
    {
        m_contentModel->model()->updateFilesPriorities(m_torrentAddParams->file_priorities);
    }

    openPersistentEditors();

    // Sized
    ui->treeTorrentContent->header()->resizeSection(TorrentContentModelItem::COL_NAME, 166);
    ui->treeTorrentContent->header()->resizeSection(TorrentContentModelItem::COL_SIZE, 63);
    ui->treeTorrentContent->header()->resizeSection(TorrentContentModelItem::COL_STATUS, 88);
    ui->treeTorrentContent->header()->resizeSection(TorrentContentModelItem::COL_PROGRESS, 125);

    VERIFY(connect(m_contentModel->model(), SIGNAL(filteredFilesChanged()), SLOT(updateDiskSpaceLabel())));
    VERIFY(connect(m_contentModel->model(), SIGNAL(filteredFilesChanged()), ui->treeTorrentContent, SLOT(updateEditorData())));

    ui->treeTorrentContent->header()->setSortIndicator(TorrentContentModelItem::COL_NAME, Qt::AscendingOrder);
}

void TorrentDetailsForm::onItemExpanded(const QModelIndex& index)
{
    libtorrent::torrent_status tStatus(torrentHandle().status());
    if (!tStatus.is_seeding)
    {
        for (int i = 0; i < m_contentModel->rowCount(index); ++i)
        {
            QModelIndex ind = index.child(i, TorrentContentModelItem::COL_PRIO);
            ui->treeTorrentContent->openPersistentEditor(ind);
        }
    }
}

void TorrentDetailsForm::updateDiskSpaceLabel()
{
    Q_ASSERT_X(m_contentModel, Q_FUNC_INFO, "Content model must be initialized before");

    qulonglong torrent_size = 0;
    if (m_contentModel)
    {
        std::vector<int> priorities;
        m_contentModel->model()->getFilesPriorities(priorities);
        Q_ASSERT(priorities.size() == (uint) m_torrentInfo->num_files());
        for (uint i = 0; i < priorities.size(); ++i)
        {
            if (priorities[i] > 0)
            {
                torrent_size += m_torrentInfo->file_at(i).size;
            }
        }
    }
    else
    {
        torrent_size = m_torrentInfo->total_size();
    }

    QString size_string = utilities::SizeToString(torrent_size);
    ui->lblSize->setText(size_string);
}

QStringList TorrentDetailsForm::filesPriorities() const
{
    std::vector<int> priorities;
    m_contentModel->model()->getFilesPriorities(priorities);
    QStringList res;
    res.reserve(priorities.size());
    for (int p : priorities)
        res << QString::number(p);
    return res;
}

template<class Val_t> bool isNotAllSkipped(std::vector<Val_t> const& priorities)
{
    return std::any_of(priorities.begin(), priorities.end(), [](Val_t pr)
    {
        return pr != prio::IGNORED && pr != prio::PARTIAL;
    });
};

void TorrentDetailsForm::accept()
{
    QCoreApplication::processEvents(); // must call this to stop editing operations

    bool isEverithingOk = true;
    if (m_torrentHandle.is_valid())
    {
        std::vector<int> priorities;
        m_contentModel->model()->getFilesPriorities(priorities);
        isEverithingOk = isNotAllSkipped(priorities);
        if (isEverithingOk)
        {
            m_torrentHandle.prioritize_files(priorities);
            if (m_torrentHandle.save_path().compare(savePath().toUtf8().constData()) != 0)
            {
                if (QDir().mkpath(savePath()))
                {
                    m_torrentHandle.move_storage(savePath().toUtf8().constData());
                }
                else
                {
                    QMessageBox::critical(this, tr("Error"), tr("Cannot create folder: %1").arg(savePath()));
                }
            }
        }
    }
    else if (m_torrentAddParams)
    {
        m_contentModel->model()->getFilesPriorities(m_torrentAddParams->file_priorities);
        isEverithingOk = isNotAllSkipped(m_torrentAddParams->file_priorities);
        m_torrentAddParams->save_path = savePath().toUtf8().constData();
    }

    if (isEverithingOk)
    {
        QDialog::accept();
    }
    else
    {
        QMessageBox msgBox(
            QMessageBox::Information,
            utilities::Tr::Tr(PROJECT_FULLNAME_TRANSLATION),
            tr("You cannot skip all files in torrent."),
            QMessageBox::Ok,
            this);
        msgBox.setInformativeText(tr("If you don't want to download it please\nremove or pause the downloading."));
        msgBox.exec();
    }
}


void TorrentDetailsForm::browseSavePath()
{
    QString dir = QFileDialog::getExistingDirectory(
                      this, tr("Download torrent to..."),
                      "/home",
                      QFileDialog::ShowDirsOnly
                      | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
    {
        setSavePath(dir);
    }
}

QString TorrentDetailsForm::savePath() const
{
    return m_savePath;
}

void TorrentDetailsForm::setSavePath(const QString& savePath)
{
    m_savePath = savePath;
    ui->savePathEdit->setText(m_savePath);

    if (m_contentModel)
    {
        m_contentModel->model()->setSavePath(m_savePath);
    }
}

void TorrentDetailsForm::adaptColumns(int col)
{
    ui->peersView->resizeColumnToContents(col);
}

void TorrentDetailsForm::timerEvent(QTimerEvent* event)
{
    if (ui->chbAutoRefresh->checkState() == Qt::Checked && event->timerId() == m_updateTimeId)
    {
        m_PeersInfomodel->updatePeersInfo();
    }
    return QDialog::timerEvent(event);
}

void TorrentDetailsForm::onProgressUpdated()
{
    std::vector<boost::int64_t> fp;
    m_torrentHandle.file_progress(fp);
    m_contentModel->model()->updateFilesProgress(fp);
    ui->treeTorrentContent->viewport()->repaint();
}

void TorrentDetailsForm::savePathEdited(const QString& sPath)
{
    setSavePath(sPath);
}

void TorrentDetailsForm::openPersistentEditors()
{
    libtorrent::torrent_status tStatus(torrentHandle().status());
    if (!tStatus.is_seeding)
    {
        for (int i = 0; i < m_contentModel->model()->rowCount(); ++i)
        {
            QModelIndex ind = m_contentModel->index(i, TorrentContentModelItem::COL_PRIO);
            ui->treeTorrentContent->openPersistentEditor(ind);
        }
    }
    /// must be after priorities update
    ui->treeTorrentContent->setExpanded(m_contentModel->index(0, 0), true);
}

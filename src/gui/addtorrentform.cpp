#include "addtorrentform.h"
#include "ui_addtorrentform.h"
#include "torrentcontentfiltermodel.h"
#include "torrentcontentmodel.h"
#include "proplistdelegate.h"
#include "utilities/utils.h"

#include <QDateTime>
#include <QFileDialog>
#include <QDebug>
#include "settings_declaration.h"
#include "libtorrent/torrent_handle.hpp"
#include "libtorrent/torrent_status.hpp"


AddTorrentForm::AddTorrentForm(const libtorrent::torrent_handle& handle, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::AddTorrentForm),
    m_contentModel(nullptr),
    m_torrentInfo(nullptr),
    m_torrentAddParams(nullptr),
    m_torrentHandle(handle)

{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);
    // Initialize using torrent handle
    initialize();
}

AddTorrentForm::AddTorrentForm(libtorrent::add_torrent_params* info, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::AddTorrentForm),
    m_contentModel(nullptr),
    m_torrentInfo(nullptr),
    m_torrentAddParams(info)

{
    ui->setupUi(this);
    // Initialize using torrent add params
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);
    initialize();
}

AddTorrentForm::~AddTorrentForm()
{
    delete ui;
}

void AddTorrentForm::initialize()
{
    ui->savePathLineEdit->setText("");

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

    Q_ASSERT(m_torrentInfo->is_valid());

    m_contentModel = new TorrentContentFilterModel(this);
    m_contentModel->model()->setSavePath(savePath());
    ui->treeTorrentContent->setModel(m_contentModel);

    // Setting up other saving path stuffs
    VERIFY(connect(ui->savePathButton, SIGNAL(clicked()), SLOT(browseSavePath())));
    VERIFY(connect(ui->savePathLineEdit, SIGNAL(textEdited(QString)), SLOT(savePathEdited(QString))));
    VERIFY(connect(ui->selectNoneButton, SIGNAL(clicked()), SLOT(selectNone())));
    VERIFY(connect(ui->selectAllButton, SIGNAL(clicked()), SLOT(selectAll())));


    auto* contentDelegate = new PropListDelegate(this);
    ui->treeTorrentContent->setItemDelegate(contentDelegate);
    VERIFY(connect(ui->treeTorrentContent, SIGNAL(clicked(const QModelIndex&)), 
        ui->treeTorrentContent, SLOT(edit(const QModelIndex&))));

    // List files in torrent
    m_contentModel->model()->setupModelData(*m_torrentInfo, libtorrent::torrent_status());
    ui->lblName->setText(QString::fromStdString(m_torrentInfo->name()));
    QString comment = QString::fromStdString(m_torrentInfo->comment());
    ui->lblComment->setText(comment.replace('\n', ' '));
    if (auto dt = m_torrentInfo->creation_date())
    {
        ui->lblDate->setText(QDateTime::fromTime_t(*dt).toString("dd/MM/yyyy hh:mm"));
    }
    updateDiskSpaceLabel();
    VERIFY(connect(m_contentModel->model(), SIGNAL(filteredFilesChanged()), SLOT(updateDiskSpaceLabel())));
    VERIFY(connect(m_contentModel->model(), SIGNAL(filteredFilesChanged()), SLOT(checkAcceptAvailable())));

    // Expand root folder
    ui->treeTorrentContent->setExpanded(m_contentModel->index(0, 0), true);
    ui->treeTorrentContent->header()->setSectionResizeMode(0, QHeaderView::Stretch);

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

    // hide columns
    ui->treeTorrentContent->hideColumn(TorrentContentModelItem::COL_STATUS);
    ui->treeTorrentContent->hideColumn(TorrentContentModelItem::COL_PROGRESS);
    ui->treeTorrentContent->hideColumn(TorrentContentModelItem::COL_PRIO);

    ui->treeTorrentContent->header()->setSortIndicator(TorrentContentModelItem::COL_NAME, Qt::AscendingOrder);
}

void AddTorrentForm::updateDiskSpaceLabel()
{
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

std::vector<int> AddTorrentForm::filesPriorities() const
{
    std::vector<int> priorities;
    m_contentModel->model()->getFilesPriorities(priorities);
    return priorities;
}

void AddTorrentForm::accept()
{
    if (m_torrentHandle.is_valid())
    {
        std::vector<int> priorities;
        m_contentModel->model()->getFilesPriorities(priorities);
        m_torrentHandle.prioritize_files(priorities);
        m_torrentHandle.move_storage(savePath().toUtf8().constData());
    }
    else if (m_torrentAddParams)
    {
        m_contentModel->model()->getFilesPriorities(m_torrentAddParams->file_priorities);
        m_torrentAddParams->save_path = savePath().toUtf8().constData();
    }
    if (ui->checkBoxDontShowAgain->isChecked())
    {
        QSettings().setValue(app_settings::ShowAddTorrentDialog, false);
    }
    QDialog::accept();
}


void AddTorrentForm::browseSavePath()
{
    QString dir = QFileDialog::getExistingDirectory(
                      this, tr("Download torrent to..."),
                      "/home",
                      QFileDialog::ShowDirsOnly
                      | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
    {
        return;
    }

    setSavePath(dir);
}

QString AddTorrentForm::savePath() const
{
    return m_savePath;
}

void AddTorrentForm::setSavePath(const QString& savePath)
{
    m_savePath = savePath;
    if (ui->savePathLineEdit->text() != m_savePath)
    {
        ui->savePathLineEdit->setText(m_savePath);
    }

    if (m_contentModel)
    {
        m_contentModel->model()->setSavePath(m_savePath);
    }
}

int AddTorrentForm::exec()
{
#ifdef Q_OS_WIN
    // Retrieve your application's window Id
    WId mwWinId = winId();

    // Restore your application, should it be minimized
    if (IsIconic((HWND)mwWinId))
    {
        SendMessage((HWND)mwWinId, WM_SYSCOMMAND, SC_RESTORE, 0);
    }

    // Bring your application to the foreground
    DWORD foregroundThreadPId = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    DWORD mwThreadPId         = GetWindowThreadProcessId((HWND)mwWinId, NULL);

    if (foregroundThreadPId != mwThreadPId)
    {
        // Your application's thread process Id is not that of the foreground window, so
        // attach the foreground thread to your application's, set your application to the
        // foreground, and detach the foreground thread from your application's
        AttachThreadInput(foregroundThreadPId, mwThreadPId, true);
        SetForegroundWindow((HWND)mwWinId);
        AttachThreadInput(foregroundThreadPId, mwThreadPId, false);
    }
    else
    {
        // Your application's thread process Id is that of the foreground window, so
        // just set your application to the foreground
        SetForegroundWindow((HWND)mwWinId);
    }
#else
    activateWindow();
#endif

    return QDialog::exec();
}

void AddTorrentForm::checkAcceptAvailable()
{
    if (QPushButton* okBtn = ui->buttonBox_2->button(QDialogButtonBox::Ok))
    {
        okBtn->setEnabled(!m_contentModel->model()->allFiltered());
    }
}

void AddTorrentForm::selectAll()
{
    m_contentModel->selectAll();
}

void AddTorrentForm::selectNone()
{
    m_contentModel->selectNone();
}

void AddTorrentForm::savePathEdited(const QString& sPath)
{
    if (QDir(sPath).exists())
    {
        setSavePath(sPath);
    }
}

#include "mainwindow.h"
#include "application.h"

#include <QList>
#include <QPair>
#include <QMimeData>

#include <functional>
#include <iostream>
#include <utility>
#include <stdio.h>

#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QClipboard>
#include <QDesktopWidget>
#include <QCheckBox>
#include <QScrollBar>

#include "utilities/credential.h"
#include "utilities/utils.h"
#include "utilities/translation.h"
#include "utilities/associate_app.h"
#include "utilities/filesystem_utils.h"

#include "torrentmanager.h"

#include "add_links.h"
#include "preferences.h"
#include "settings_declaration.h"
#include "global_functions.h"
#include "globals.h"
#include "qnamespace.h"
#include "branding.hxx"
#include "version.hxx"


#if defined (Q_OS_WIN)
#include <windows.h>
#elif defined (Q_OS_MAC)
#include "darwin/DarwinSingleton.h"
#include "darwin/AppHandler.h"
#endif


namespace Tr = utilities::Tr;

using namespace app_settings;

namespace {

QString extractLinkFromFile(const QString& fn)
{
    QString link;

    if (QFile::exists(fn))
    {
        QSettings settings(fn, QSettings::IniFormat);
        link = settings.value("InternetShortcut/URL").toString();
    }
    return link;
}

}

MainWindow::MainWindow()
    : ui_utils::MainWindowWithTray(nullptr, QIcon(PROJECT_ICON), PROJECT_FULLNAME_TRANSLATION),
      ui(new Ui::MainWindow),
      m_dlManager(nullptr)
      , isAutorun(false)
{
    QApplication::setWindowIcon(QIcon(PROJECT_ICON));

    ui->setupUi(this);

    // remove "Stop" action temporary
    ui->stopButton->hide();

    setWindowIcon(QIcon(PROJECT_ICON));
    setIconSize(QSize(48, 48));
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    setAcceptDrops(true);

    ui->buttonOptions->setText(" ");

    ::Tr::SetTr(this, &QWidget::setWindowTitle, PROJECT_FULLNAME_TRANSLATION);
    ::Tr::SetTr(ui->actionAbout_LIII, &QAction::setText, ABOUT_TITLE, PROJECT_NAME);

    auto* horizontalLayout = new QHBoxLayout(ui->menuBar);
    horizontalLayout->setSpacing(6);
    horizontalLayout->setObjectName(QStringLiteral("horizontalMenuBarLayout"));
    horizontalLayout->setContentsMargins(0, 2, 8, 2);

    auto* horizSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout->addItem(horizSpacer1);

    DownloadCollectionModel* pModel = &DownloadCollectionModel::instance();

    VERIFY(connect(pModel, SIGNAL(signalDeleteURLFromModel(int,DownloadType::Type,int)), SLOT(refreshButtons())));
    VERIFY(connect(pModel, SIGNAL(overallProgress(int)), SLOT(onOverallProgress(int))));
    VERIFY(connect(pModel, SIGNAL(activeDownloadsNumberChanged(int)), SLOT(onActiveDownloadsNumberChanged(int))));

    m_dlManager = new DownloadManager(this);
    VERIFY(connect(m_dlManager, SIGNAL(updateButtons()), SLOT(refreshButtons())));

    ui->listUrls->setItemDelegate(new DownloadCollectionDelegate(this));
    ui->listUrls->setModel(pModel);

    ui->actionPaste_Links->setShortcut(QKeySequence::Paste);

    VERIFY(connect(ui->listUrls, SIGNAL(signalOpenFolder(QString)), SLOT(onButtonOpenFolderClicked(QString))));
    VERIFY(connect(ui->listUrls, SIGNAL(signalOpenTorrentFolder(QString,QString)), SLOT(openTorrentDownloadFolder(QString,QString))));
    VERIFY(connect(ui->listUrls, SIGNAL(signalButtonChangePauseImage(bool,bool,bool,bool)), SLOT(onChangePauseCancelState(bool,bool,bool,bool))));
    VERIFY(connect(ui->listUrls, SIGNAL(signalDownloadFinished(QString)), SLOT(showTrayNotifDwnldFinish(QString))));

    VERIFY(connect(ui->actionOpen, SIGNAL(triggered()), SLOT(on_openTorrent_clicked())));
    VERIFY(connect(ui->actionClose_Link, SIGNAL(triggered()), SLOT(onActionCloseLinkClicked())));
    VERIFY(connect(ui->actionExit_Link, SIGNAL(triggered()), SLOT(closeApp())));
    VERIFY(connect(ui->actionPaste_Links, SIGNAL(triggered()), SLOT(on_buttonPaste_clicked())));
    VERIFY(connect(ui->actionPause_selected, SIGNAL(triggered()), SLOT(on_pauseButton_clicked())));
    VERIFY(connect(ui->actionCancel_selected, SIGNAL(triggered()), SLOT(on_cancelButton_clicked())));
    VERIFY(connect(ui->actionStart, SIGNAL(triggered()), SLOT(on_startButton_clicked())));
    //    VERIFY(connect(ui->actionStop, SIGNAL(triggered()), SLOT(on_stopButton_clicked())));

    VERIFY(connect(ui->actionAbout_LIII, SIGNAL(triggered()), SLOT(onAboutClicked())));

    VERIFY(connect(ui->actionStartAllDownloads, SIGNAL(triggered()), ui->listUrls, SLOT(resumeAllItems())));
    VERIFY(connect(ui->actionPauseAllDownloads, SIGNAL(triggered()), ui->listUrls, SLOT(pauseAllItems())));
    //    VERIFY(connect(ui->actionStopAllDownloads, SIGNAL(triggered()), ui->listUrls, SLOT(stopAllItems())));
    VERIFY(connect(ui->buttonOpenFolder, SIGNAL(clicked()), SLOT(onButtonOpenFolderClicked())));

    VERIFY(connect(ui->actionFind, SIGNAL(triggered()), SLOT(onFind())));
    VERIFY(connect(ui->actionSelect_Completed, SIGNAL(triggered()), SLOT(onSelectCompleted())));
    VERIFY(connect(ui->actionInvert_Selection, SIGNAL(triggered()), SLOT(onInvertSelection())));

#ifdef Q_OS_MAC
    VERIFY(connect(&DarwinSingleton::Instance(), SIGNAL(showPreferences()), SLOT(on_buttonOptions_clicked())));
    VERIFY(connect(&DarwinSingleton::Instance(), SIGNAL(showAbout()), SLOT(onAboutClicked())));
    VERIFY(connect(&DarwinSingleton::Instance(), SIGNAL(addTorrent(QStringList)), SLOT(openTorrent(QStringList))));

    ui->menuFile->menuAction()->setVisible(false);
    ui->menuTools->menuAction()->setVisible(false);
    ui->linkEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listUrls->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->actionClose_Link->setShortcut(QKeySequence("Cmd+W"));
#endif //Q_OS_MAC

    VERIFY(connect(ui->actionPreferences, SIGNAL(triggered()), SLOT(on_buttonOptions_clicked())));

    populateTrayMenu();

    VERIFY(connect(ui->linkEdit, SIGNAL(linksAdd(bool)), ui->lblClearText, SLOT(setVisible(bool))));
    VERIFY(connect(ui->linkEdit, SIGNAL(returnPressed()), SLOT(on_buttonStart_clicked())));
    VERIFY(connect(ui->lblClearText, SIGNAL(clicked()), SLOT(onlblClearTextClicked())));

    refreshButtons();
}

#if defined(Q_OS_WIN)
bool MainWindow::nativeEvent(const QByteArray& /*eventType*/, void* message, long* /*result*/)
{
    if (message && static_cast<MSG*>(message)->message == ui_utils::TaskBar::InitMessage())
    {
        m_taskBar.Init(winId());
    }
    return false;
}
#endif

void MainWindow::showMainWindowAndPerformChecks()
{
    readPositionSettings();

    m_dlManager->startLoad();
    VERIFY(connect(qApp, SIGNAL(aboutToQuit()), SLOT(prepareToExit())));

    refreshButtons();

    if (!isAutorun)
    {
        show();
        checkDefaultTorrentApplication();
    }
    if (auto* myApp = dynamic_cast<Application*>(qApp))
    {
        myApp->checkFirewallException(this);
    }
}

void MainWindow::checkDefaultTorrentApplication()
{
    if (QSettings().value(ShowAssociateTorrentDialog, !utilities::IsPortableMode()).toBool()
        && !utilities::isDefaultTorrentApp())
    {
        QMessageBox msgBox(
            QMessageBox::NoIcon,
            ::Tr::Tr(PROJECT_FULLNAME_TRANSLATION),
            ::Tr::Tr(ASSOCIATE_TORRENT_TEXT).arg(PROJECT_NAME),
            QMessageBox::Yes | QMessageBox::No,
            this);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setCheckBox(new QCheckBox(::Tr::Tr(DONT_SHOW_THIS_AGAIN)));

        if (msgBox.exec() == QMessageBox::Yes)
        {
            utilities::setDefaultTorrentApp(winId());
        }

        QSettings().setValue(ShowAssociateTorrentDialog, !msgBox.checkBox()->isChecked());
    }

    if (QSettings().value(ShowAssociateMagnetDialog, !utilities::IsPortableMode()).toBool()
        && !utilities::isDefaultMagnetApp())
    {
        QMessageBox msgBox(
            QMessageBox::NoIcon,
            ::Tr::Tr(PROJECT_FULLNAME_TRANSLATION),
            ::Tr::Tr(ASSOCIATE_MAGNET_TEXT).arg(PROJECT_NAME),
            QMessageBox::Yes | QMessageBox::No,
            this);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setCheckBox(new QCheckBox(::Tr::Tr(DONT_SHOW_THIS_AGAIN)));

        if (msgBox.exec() == QMessageBox::Yes)
        {
            utilities::setDefaultMagnetApp(winId());
        }

        QSettings().setValue(ShowAssociateMagnetDialog, !msgBox.checkBox()->isChecked());
    }
}

void MainWindow::onChangePauseCancelState(bool canPause, bool canResume, bool canCancel, bool canStop)
{
    ui->startButton->setEnabled(canResume);
    ui->pauseButton->setEnabled(canPause);
    ui->stopButton->setEnabled(canStop);
    ui->actionPause_selected->setEnabled(canPause);
    ui->cancelButton->setEnabled(canCancel);
    ui->actionCancel_selected->setEnabled(canCancel);
    ui->actionStart->setEnabled(canResume);
}

void MainWindow::populateTrayMenu()
{
    addTrayMenuItem(TrayMenu::Show);
    addTrayMenuItem(ui->actionPreferences);
    addTrayMenuItem(TrayMenu::Separator);
    addTrayMenuItem(ui->actionStartAllDownloads);
    addTrayMenuItem(ui->actionPauseAllDownloads);
    //addTrayMenuItem(ui->actionStopAllDownloads);
    addTrayMenuItem(TrayMenu::Separator);
    addTrayMenuItem(TrayMenu::Exit);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onAboutClicked()
{
    raise();
    activateWindow();
    QMessageBox::about(
        this,
        QString(::Tr::Tr(ABOUT_TITLE)).arg(PROJECT_NAME),
        ::Tr::Tr(PROJECT_FULLNAME_TRANSLATION) + " " PROJECT_VERSION);
}

void MainWindow::onActionCloseLinkClicked()
{
    hide();
}

void MainWindow::closeApp()
{
    static bool isrunning = false;
    if (!isrunning)
    {
        isrunning = true;
        if (m_dlManager->isWorking() 
            && QSettings().value(ShowExitWarning, true).toBool())
        {
            QMessageBox msgBox(
                QMessageBox::NoIcon,
                ::Tr::Tr(PROJECT_FULLNAME_TRANSLATION),
                ::Tr::Tr(EXIT_TEXT).arg(PROJECT_NAME),
                QMessageBox::Yes | QMessageBox::No,
                this);
            msgBox.setDefaultButton(QMessageBox::No);
            msgBox.setCheckBox(new QCheckBox(::Tr::Tr(DONT_SHOW_THIS_AGAIN)));

            if (msgBox.exec() != QMessageBox::Yes)
            {
                isrunning = false;
                return;
            }
            const bool isDontShowMeChecked = msgBox.checkBox()->isChecked();
            QSettings().setValue(ShowExitWarning, !isDontShowMeChecked);
        }
        MainWindowWithTray::closeApp();
        isrunning = false;
    }
}

void MainWindow::prepareToExit()
{
    qDebug() << __FUNCTION__;
    writePositionSettings();
    m_dlManager->prepareToExit();
#ifdef Q_OS_WIN
    m_taskBar.Uninit();
#endif
}

void MainWindow::onlblClearTextClicked()
{
    ui->linkEdit->setText(QString());
    refreshButtons();
}

void MainWindow::on_buttonStart_clicked()
{
    if (!ui->linkEdit->text().isEmpty())
    {
        addLinks(utilities::ParseUrls(ui->linkEdit->text()));
    }
    else
    {
        ui->linkEdit->setErrorState();
    }
    ui->linkEdit->setText(QString());
    if (DownloadCollectionModel::instance().rowCount() == 0)
    {
        return;
    }

    m_dlManager->startLoad();
    refreshButtons();
}

void MainWindow::addLinks(QStringList urls)
{
    if (!urls.empty())
    {
        // show links dialog if adding multiple links
        if (urls.size() > 1)
        {
            AddLinks dlg(urls, this);
            if (dlg.exec() != QDialog::Accepted)
            {
                return;
            }
            urls = dlg.urls();
        }

        // add links to model
        m_dlManager->addItemsToModel(urls, DownloadType::Unknown);

        // update UI according model changes
        refreshButtons();
    }
}


void MainWindow::on_buttonOptions_clicked()
{
    raise();
    activateWindow();

    Preferences prefDlg(this, Preferences::GENERAL);
    connect(&prefDlg, &Preferences::newPreferencesApply, m_dlManager, &DownloadManager::siftDownloads);
    connect(&prefDlg, &Preferences::onProxySettingsChanged, 
        TorrentManager::Instance(), &TorrentManager::onProxySettingsChanged);
    if (prefDlg.exec() == QDialog::Accepted)
    {
#ifdef ALLOW_TRAFFIC_CONTROL
        m_dlManager->setSpeedLimit(global_functions::GetTrafficLimitActual()); // just in case
#endif // ALLOW_TRAFFIC_CONTROL
        m_dlManager->startLoad();
        refreshButtons();
    }
}

void MainWindow::onFind()
{
    ui->listUrls->findItems();
}

void MainWindow::onSelectCompleted()
{
    ui->listUrls->selectCompleted();
}

void MainWindow::onInvertSelection()
{
    ui->listUrls->invertSelection();
}

void MainWindow::onButtonOpenFolderClicked(const QString& filename)
{
    utilities::SelectFile(filename, global_functions::GetVideoFolder());
}

void MainWindow::openTorrentDownloadFolder(const QString& filename, const QString& downloadDirectory)
{
    utilities::SelectFile(filename, downloadDirectory);
}

void MainWindow::on_buttonPaste_clicked()
{
    QClipboard* clipboard = QApplication::clipboard();

    QStringList links = utilities::ParseUrls(clipboard->mimeData()->text());
    if (links.isEmpty())
    {
        QMessageBox msgBox(
            QMessageBox::NoIcon,
            ::Tr::Tr(PROJECT_FULLNAME_TRANSLATION),
            ::Tr::Tr(NO_LINKS_IN_CLIPBOARD),
            QMessageBox::Ok,
            this);
        msgBox.exec();
    }
    else
    {
        addLinks(links);
    }
}

void MainWindow::on_startButton_clicked()
{
    ui->listUrls->startDownloadItem();
}

void MainWindow::on_pauseButton_clicked()
{
    ui->listUrls->pauseDownloadItem();
}

//void MainWindow::on_stopButton_clicked()
//{
//    ui->listUrls->stopDownloadItem();
//}

void MainWindow::on_cancelButton_clicked()
{
    ui->listUrls->deleteSelectedRows();
    refreshButtons();
}

void MainWindow::refreshButtons()
{
    ui->lblClearText->setVisible(!ui->linkEdit->text().isEmpty());
    ui->listUrls->getUpdateItem();
}

bool MainWindow::setError(const QString& strHeader, const QString& strErrorText)
{
    bool isNotEmptyHeader = !strHeader.isEmpty();
    if (isNotEmptyHeader)
    {
        QMessageBox msgBox(
            QMessageBox::Critical,
            ::Tr::Tr(PROJECT_FULLNAME_TRANSLATION),
            strHeader,
            QMessageBox::Ok,
            this);
        msgBox.setInformativeText(strErrorText);
        msgBox.exec();
    }
    return isNotEmptyHeader;
}

void MainWindow::languageChange()
{
    // will be called after call of RetranslateApp
    ::Tr::RetranslateAll(this);
    ui->retranslateUi(this);
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        languageChange();
    }
}


void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/plain") || event->mimeData()->hasFormat("text/uri-list"))
    {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    QString text;
    QRegExp intrestedDataRx;

    if (event->mimeData()->hasHtml())
    {
        text = event->mimeData()->html();
        intrestedDataRx = QRegExp("(http|ftp|file|magnet|[a-z]):[^\"'<>\\s\\n]+", Qt::CaseInsensitive);
    }
    else if (event->mimeData()->hasText())
    {
        text = event->mimeData()->text();
        intrestedDataRx = QRegExp("(http|ftp|file|magnet|[a-z]):[^\\s\\n]+", Qt::CaseInsensitive);
    }
    else if (event->mimeData()->hasFormat("text/uri-list"))
    {
        text = event->mimeData()->data("text/uri-list");
        intrestedDataRx = QRegExp("(http|ftp|file|magnet|[a-z]):[^\\s\\n]+", Qt::CaseInsensitive);
    }
    else
    {
        return;
    }

    qDebug() << "Some data dropped to program. Trying to manage it.";

    int pos = 0;
    QStringList linksForDownload;
    while ((pos = intrestedDataRx.indexIn(text, pos)) != -1)
    {
        QString someLink = intrestedDataRx.cap(0);
        QString typeOfLink = intrestedDataRx.cap(1);
        qDebug() << QString(PROJECT_NAME) + " takes " + someLink;
        pos += intrestedDataRx.matchedLength();


        if (typeOfLink == "file")
        {
            someLink = QUrl(QUrl::fromPercentEncoding(someLink.toLatin1())).toLocalFile();
            DownloadType::Type fileType = DownloadType::determineType(someLink);
            switch (fileType)
            {
            case DownloadType::TorrentFile:
                break;
            case DownloadType::LocalFile:
                someLink = extractLinkFromFile(someLink);
                break;
            default:
                Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown file type");
                continue;
            }
        }
        linksForDownload << someLink;
    }
    addLinks(linksForDownload);

    event->acceptProposedAction();
}

void MainWindow::showHideNotify()
{
    if (QSettings().value(ShowSysTrayNotifications, true).toBool()
        && QSettings().value(ShowSysTrayNotificationOnHide, true).toBool())
    {
        QSettings().setValue(ShowSysTrayNotificationOnHide, false);
        showTrayMessage(::Tr::Tr(MINIMIZE_TEXT).arg(PROJECT_NAME));
    }
}

void MainWindow::showTrayNotifDwnldFinish(const QString& str)
{

    if (QSettings().value(ShowSysTrayNotifications, true).toBool())
    {
#ifdef Q_OS_WIN
        showTrayMessage(tr("File \"%1\" was downloaded").arg(str));
#elif defined(Q_OS_MAC)
        Darwin::showNotification(tr("File downloaded"), tr("File \"%1\" was downloaded").arg(str));
#endif //Q_OS_WIN
    }
}


void MainWindow::on_openTorrent_clicked()
{
    QString startPath = utilities::getPathForDownloadFolder();

    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Select a Torrent to open"), startPath, "Torrent Files(*.torrent);;All(*.*)");
    if (!filenames.empty())
    {
        addLinks(filenames);
    }
}


void MainWindow::keyPressEvent(QKeyEvent* event)
{
#ifdef DEVELOPER_FEATURES
    if (event->key() == Qt::Key_F5)
    {
        qDebug() << "apply custom css";
#ifndef Q_OS_MAC
        QFile file(QApplication::applicationDirPath() + "/../../../resources/LIII/style.css");
        if (file.exists() && file.open(QIODevice::ReadOnly))
        {
            setStyleSheet(file.readAll());
            file.close();
        }
#else
        QFile file(QApplication::applicationDirPath() + "/../../../../../../resources/LIII/style.css");
        QFile macfile(QApplication::applicationDirPath() + "/../../../../../../resources/LIII/macstyle.css");
        if (file.exists() && macfile.exists() && file.open(QIODevice::ReadOnly) && macfile.open(QIODevice::ReadOnly))
        {
            setStyleSheet(file.readAll() + "\n" + macfile.readAll());
            file.close();
            macfile.close();
        }

#endif //!Q_OS_MAC
    }
#endif
    QMainWindow::keyPressEvent(event);
}

#ifdef Q_OS_MAC
QString MainWindow::findApplicationPath(const QString& appBrand)
{
    QString findCommandLine = QString("echo \"set brand to \\\"%1\\\" \n\
set brandBundleID to id of application brand \n\
try \n\
set brandFiles to paragraphs of (do shell script \\\"mdfind \\\\\\\"kMDItemCFBundleIdentifier='\\\" & brandBundleID & \\\"'\\\\\\\"\\\") \n\
on error errText number errNum \n\
set brandFiles to {} \n\
end try \n\
do shell script \\\"echo \\\" & item 1 in brandFiles\" | /usr/bin/osascript"
                                     ).arg(appBrand);

    FILE* pipe = popen(findCommandLine.toAscii().data(), "r");
    if (!pipe)
    {
        return QString();
    }
    else
    {
        char buffer[128];
        std::string result = "";
        while (!feof(pipe))
        {
            if (fgets(buffer, 128, pipe) != NULL)
            {
                result += buffer;
            }
        }
        pclose(pipe);
        return QString::fromStdString(result).replace("\n", "");
    }
}
#endif //Q_OS_MAC

void MainWindow::openTorrent(QStringList magnetUrls)
{
    addLinks(std::move(magnetUrls));
}

void MainWindow::onOverallProgress(int progress)
{
#ifdef Q_OS_WIN
    m_taskBar.setProgress(progress);
#elif defined(Q_OS_MAC)
    Darwin::setOverallProgress(progress);
#endif
}

void MainWindow::onActiveDownloadsNumberChanged(int number)
{
#ifdef Q_OS_WIN
#elif defined(Q_OS_MAC)
    Darwin::setDockBadge(number);
#endif
}

const char SETTINGS_MAINWINDOW[] = "mainwindow";
const char MAINWINDOW_GEOMETRY[] = "geometry";
const char MAINWINDOW_SAVESTATE[] = "savestate";
const char MAINWINDOW_MAXIMIZED[] = "maximized";
const char MAINWINDOW_POS[] = "pos";
const char MAINWINDOW_SIZE[] = "size";

const char MAINWINDOW_SCROLL_X[] = "scrollX";
const char MAINWINDOW_SCROLL_Y[] = "scrollY";

// https://stackoverflow.com/questions/74690/how-do-i-store-the-window-size-between-sessions-in-qt
void MainWindow::writePositionSettings()
{
    QSettings qsettings;

    qsettings.beginGroup(SETTINGS_MAINWINDOW);

    qsettings.setValue(MAINWINDOW_GEOMETRY, saveGeometry());
    qsettings.setValue(MAINWINDOW_SAVESTATE, saveState());
    qsettings.setValue(MAINWINDOW_MAXIMIZED, isMaximized());
    if (!isMaximized()) {
        qsettings.setValue(MAINWINDOW_POS, pos());
        qsettings.setValue(MAINWINDOW_SIZE, size());
    }

    if (auto s = ui->listUrls->horizontalScrollBar())
    {
        qsettings.setValue(MAINWINDOW_SCROLL_X, s->value());
    }
    if (auto s = ui->listUrls->verticalScrollBar())
    {
        qsettings.setValue(MAINWINDOW_SCROLL_Y, s->value());
    }

    qsettings.endGroup();
}


void MainWindow::readPositionSettings()
{
    QSettings qsettings;

    qsettings.beginGroup(SETTINGS_MAINWINDOW);

    restoreGeometry(qsettings.value(MAINWINDOW_GEOMETRY, saveGeometry()).toByteArray());
    restoreState(qsettings.value(MAINWINDOW_SAVESTATE, saveState()).toByteArray());
    move(qsettings.value(MAINWINDOW_POS, pos()).toPoint());
    resize(qsettings.value(MAINWINDOW_SIZE, size()).toSize());
    if (qsettings.value(MAINWINDOW_MAXIMIZED, isMaximized()).toBool())
        showMaximized();

    if (auto s = ui->listUrls->horizontalScrollBar())
    {
        s->setValue(qsettings.value(MAINWINDOW_SCROLL_X, 0).toInt());
    }
    if (auto s = ui->listUrls->verticalScrollBar())
    {
        s->setValue(qsettings.value(MAINWINDOW_SCROLL_Y, 0).toInt());
    }

    qsettings.endGroup();
}

#pragma once

#include <QListWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QLabel>
#include <QSharedPointer>
#include <QStandardItemModel>
#include <QNetworkReply>
#include <QTime>
#include <QToolButton>


#include "utilities/singleton.h"
#include "ui_utils/mainwindowwithtray.h"

#include "downloadcollectionmodel.h"
#include "downloadcollectiondelegate.h"
#include "downloadmanager.h"
#include "globals.h"
#include "ui_utils/taskbar.h"


#ifdef Q_OS_WIN
using ui_utils::TaskBar;
#endif //Q_OS_WIN


namespace Ui
{
class MainWindow;
}

class MainWindow : public ui_utils::MainWindowWithTray
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();
    void addLinks(QStringList urls);

    void setAutorunMode() {isAutorun = true;}; // start in tray only in case of autorun.
    void showMainWindowAndPerformChecks();

private:
    bool setError(const QString& strHeader = QString(), const QString& strErrorText = QString());
    void populateTrayMenu();

    void languageChange();
    virtual void changeEvent(QEvent* event) override;

    void checkDefaultTorrentApplication();

    template<class Tab_t> // as we cannot use Preferences::TAB here
    void OpenPreferences(Tab_t);

    void moveToScreenCenter();

    Ui::MainWindow*                ui;
    DownloadManager*                    m_dlManager;
    bool                        isAutorun;

#ifdef Q_OS_WIN
    TaskBar m_taskBar;
    bool winEvent(MSG* msg, long* result);
#endif

Q_SIGNALS:
    void urlsWereUpdated(QStringList lUrls);
    void autoPlayUpdated(int state);

public Q_SLOTS:
    void onChangePauseCancelState(bool canPause, bool canResume, bool canCancel, bool canStop);
    void refreshButtons();
    void needLogin(utilities::ICredentialsRetriever* th);
    void on_buttonStart_clicked();
    void onActionCloseLinkClicked();
    void closeApp();
    void onAboutClicked();
    void on_openTorrent_clicked();
    void onlblClearTextCliced();

private Q_SLOTS:
    void prepairToExit();
    void on_startButton_clicked();
    void on_pauseButton_clicked();
    //    void on_stopButton_clicked();
    void on_cancelButton_clicked();
    void on_clearButton_clicked();
    void on_buttonPaste_clicked();
    void onButtonOpenFolderClicked(const QString& filename = QString());
    void on_buttonOptions_clicked();
    void openTorrentDownloadFolder(const QString& filename, const QString& downloadDirectory);
    void onOverallProgress(int progress);
    void onActiveDownloadsNumberChanged(int number);
    void showTrayNotifDwnldFinish(const QString& str);
    void openTorrent(QStringList magnetUrls);
#ifdef Q_OS_MAC
    QString findApplicationPath(const QString& appBrand);
#endif //Q_OS_MAC

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void showHideNotify() override;
};

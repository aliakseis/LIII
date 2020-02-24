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

    void setAutorunMode() {isAutorun = true;} // start in tray only in case of autorun.
    void showMainWindowAndPerformChecks();

private:
    bool setError(const QString& strHeader = QString(), const QString& strErrorText = QString());
    void populateTrayMenu();

    void languageChange();
    void changeEvent(QEvent* event) override;

    void checkDefaultTorrentApplication();

    void writePositionSettings();
    void readPositionSettings();

    Ui::MainWindow* ui;
    DownloadManager* m_dlManager;
    bool isAutorun;

#ifdef Q_OS_WIN
    ui_utils::TaskBar m_taskBar;
    bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
#endif

Q_SIGNALS:
    void urlsWereUpdated(QStringList lUrls);
    void autoPlayUpdated(int state);

public Q_SLOTS:
    void onChangePauseCancelState(bool canPause, bool canResume, bool canCancel, bool canStop);
    void refreshButtons();
    void on_buttonStart_clicked();
    void onActionCloseLinkClicked();
    void closeApp() override;
    void onAboutClicked();
    void on_openTorrent_clicked();
    void onlblClearTextClicked();

private Q_SLOTS:
    void prepareToExit();
    void on_startButton_clicked();
    void on_pauseButton_clicked();
    //    void on_stopButton_clicked();
    void on_cancelButton_clicked();
    //void on_clearButton_clicked();
    void on_buttonPaste_clicked();
    void onButtonOpenFolderClicked(const QString& filename = QString());
    void on_buttonOptions_clicked();
    void onFind();
    void openTorrentDownloadFolder(const QString& filename, const QString& downloadDirectory);
    void onOverallProgress(int progress);
    void onActiveDownloadsNumberChanged(int number);
    void showTrayNotifDwnldFinish(const QString& str);
    void openTorrent(QStringList magnetUrls);
#ifdef Q_OS_MAC
    QString findApplicationPath(const QString& appBrand);
#endif //Q_OS_MAC

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void showHideNotify() override;
};

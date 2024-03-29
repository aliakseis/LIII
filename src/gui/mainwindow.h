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
#include <QProgressBar>

#include "utilities/singleton.h"
#include "ui_utils/mainwindowwithtray.h"

#include "downloadcollectionmodel.h"
#include "downloadcollectiondelegate.h"
#include "downloadmanager.h"
#include "globals.h"
#include "ui_utils/taskbar.h"

#include <boost/cstdint.hpp>

#include <vector>

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

    long long m_prevSessionStatsUnixTime {};

    unsigned long long m_prevSentPayloadBytes {};
    unsigned long long m_prevRecvPayloadBytes {};

    quint64 m_downloadedProgress {};
    quint64 m_totalProgress {};

    QProgressBar* m_statusProgressBar;
    QLabel* m_statusDhtNodes;
    QLabel* m_statusTotalSpeed;
    QLabel* m_statusDiskFreeSpace;

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
    void on_buttonPaste_clicked();
    void onButtonOpenFolderClicked(const QString& filename = QString());
    void on_buttonOptions_clicked();
    void onFind();
    void onSelectCompleted();
    void onInvertSelection();
    void openTorrentDownloadFolder(const QString& filename, const QString& downloadDirectory);
    void onOverallProgress(quint64 downloaded, quint64 total);
    void onActiveDownloadsNumberChanged(int number);
    void showTrayNotifDwnldFinish(const QString& str);
    void openTorrent(QStringList magnetUrls);
    void onSessionStats(long long unixTime, const std::vector<boost::uint64_t>& stats);
#ifdef Q_OS_MAC
    QString findApplicationPath(const QString& appBrand);
#endif //Q_OS_MAC

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void showHideNotify() override;
};

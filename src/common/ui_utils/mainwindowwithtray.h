#pragma once

#include <QMainWindow>
#include <QSystemTrayIcon>

class QEvent;
class QCloseEvent;
class QIcon;
namespace utilities {namespace Tr {struct Translation;}}
class Test_MainWindowWithTray;

namespace ui_utils
{

class MainWindowWithTray
    : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindowWithTray(QWidget* parent, QIcon const& icon, utilities::Tr::Translation const& projFullNameTr);
    ~MainWindowWithTray();

    struct TrayMenu
    {
        enum ItemType
        {
            Separator,
            Exit,
            About,
            Preferences,
            Show
            // TODO: add more common item types here
        };
    };

    void addTrayMenuItem(QAction* action);
    QAction* addTrayMenuItem(TrayMenu::ItemType itemType);
    void showTrayMessage(const QString& message);

public Q_SLOTS:
    void restore();
    virtual void closeApp();
    void setVisible(bool visible) override;

private Q_SLOTS:
    void trayAction(QSystemTrayIcon::ActivationReason reason);

signals:
    void trayMenuShouldBeUpdated();

protected:
    void closeEvent(QCloseEvent* event) override;
    virtual void showHideNotify(); // override this function on first hide

private:
    void createTrayMenu(QIcon const& icon, utilities::Tr::Translation const& projFullNameTr);
    QAction* createAction(const char* actName, utilities::Tr::Translation translation, QKeySequence shortcut, const char* onTriggered);

    QSystemTrayIcon* m_tray;
    bool m_isExiting;

};//class MainWindowWithTray

}//namespace ui_utils

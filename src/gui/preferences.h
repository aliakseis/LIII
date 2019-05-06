#pragma once

#include <QDialog>
#include "ui_utils/tabbeddialogcombo.h"

using ui_utils::TabbedDialogCombo;

class QModelIndex;

namespace Ui
{
class Preferences;
}

class Preferences
    : public QDialog
{

    Q_OBJECT

public:
    enum TAB {GENERAL = 0, TORRENT};
    Preferences(QWidget* parent = 0, TAB tab = GENERAL);
    ~Preferences();

private:
    bool m_dataChanged;
    Ui::Preferences* ui;
#ifdef Q_OS_WIN
    bool m_startLIIIOnStartingWindows;
#endif


    void fillLanguageComboBox();
    void initMainSettings();


    inline bool isDataChanged()const {return m_dataChanged;};

    bool checkData();

Q_SIGNALS:
    void anyDataChanged();
    void newPreferencesApply();

private Q_SLOTS:
    void accept();
    void apply();
    void on_btnResetWarnings_clicked();
    void on_pbBrowse_clicked();
    void on_torrentAssociateCheckBoxChanged(int);

    void on_radioTorrentPortAuto_toggled(bool);
    void on_torrentRandomPortButton_clicked();
    void on_torrentPortSettingEdit_textChanged(const QString& text);

    void dataChanged(bool b = true);

    void checkPortAvalible(const QString& textPort);
    bool checkPortOk(int targetPort, const char** reason = 0);

    void onProxyStateChanged(int state);

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;
};


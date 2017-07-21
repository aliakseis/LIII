#include "preferences.h"
#include "ui_preferences.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QTranslator>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

#ifdef DEVELOPER_FEATURES
#include <QKeyEvent>
#endif

#include "utilities/utils.h"
#include "utilities/translatable.h"
#include "utilities/autorun_utils.h"

#include "settings_declaration.h"
#include "global_functions.h"

#include "branding.hxx"

#include "torrentmanager.h"
#include "utilities/associate_app.h"

#include "globals.h"

namespace Tr = utilities::Tr;

using namespace app_settings;

const int maxPort = 65535;


Preferences::Preferences(QWidget* parent, TAB tab)
    : QDialog(parent)
    , m_dataChanged(false)
    , ui(new Ui::Preferences)
#ifdef Q_OS_WIN
    , m_startLIIIOnStartingWindows(utilities::isAutorunEnabled())
#endif
{
    ui->setupUi(this);
    utilities::Tr::MakeRetranslatable(this, ui);

    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    VERIFY(connect(ui->buttonOK, SIGNAL(clicked()), this, SLOT(accept())));
    VERIFY(connect(ui->buttonApply, SIGNAL(clicked()), this, SLOT(apply())));
    VERIFY(connect(ui->buttonCancel, SIGNAL(clicked()), this, SLOT(reject())));

    //handle data changes signals
    VERIFY(connect(ui->comboBoxLanguage, SIGNAL(currentIndexChanged(int)), this, SIGNAL(anyDataChanged())));
    VERIFY(connect(ui->sbMaxNum, SIGNAL(valueChanged(int)), this, SIGNAL(anyDataChanged())));
    VERIFY(connect(ui->sbTrafficLimit, SIGNAL(valueChanged(int)), this, SIGNAL(anyDataChanged())));
    VERIFY(connect(ui->cbTrafficLimit, SIGNAL(stateChanged(int)), this, SIGNAL(anyDataChanged())));
    VERIFY(connect(ui->leFolder, SIGNAL(textEdited(const QString&)), this, SIGNAL(anyDataChanged())));
    VERIFY(connect(ui->maximumNumberLoads, SIGNAL(stateChanged(int)), this, SIGNAL(anyDataChanged())));
    VERIFY(connect(ui->cbHideAllTrayNotification, SIGNAL(stateChanged(int)), this, SIGNAL(anyDataChanged())));

    VERIFY(connect(this, SIGNAL(anyDataChanged()), this, SLOT(dataChanged())));

    VERIFY(connect(ui->torrentAssociateCheckbox, SIGNAL(stateChanged(int)), this, SLOT(on_torrentAssociateCheckBoxChanged(int))));
    VERIFY(connect(ui->torrentAssociateCheckbox, SIGNAL(stateChanged(int)), this, SIGNAL(anyDataChanged())));
    VERIFY(connect(ui->torrentPortSettingEdit, SIGNAL(textChanged(QString)), this, SIGNAL(anyDataChanged())));
    VERIFY(connect(ui->torrentSpeedUploadLimitSpin, SIGNAL(valueChanged(QString)), this, SIGNAL(anyDataChanged())));
    VERIFY(connect(ui->torrentStartAsSequel, SIGNAL(stateChanged(int)), this, SIGNAL(anyDataChanged())));
    VERIFY(connect(ui->torrentSpeedLimitedCheckbox, SIGNAL(stateChanged(int)), this, SIGNAL(anyDataChanged())));

    ui->tabWidget->setCurrentIndex(tab);

    initMainSettings();


    fillLanguageComboBox();


#ifdef Q_OS_WIN
    Tr::SetTr(ui->startLIIIOnStartingWindows, &QCheckBox::setText, START_LIII_ON_STARTING_WINDOWS_LABEL, PROJECT_NAME);
    ui->startLIIIOnStartingWindows->setChecked(m_startLIIIOnStartingWindows);
    VERIFY(connect(ui->startLIIIOnStartingWindows, SIGNAL(stateChanged(int)), this, SIGNAL(anyDataChanged())));
#else
    ui->startLIIIOnStartingWindows->hide();

    ui->leFolder->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->sbMaxNum->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->sbTrafficLimit->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->torrentSpeedUploadLimitSpin->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->torrentPortSettingEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->gridLayout->removeItem(ui->horizontalLayout_1);
#endif

    //reset state
    dataChanged(false);
}

Preferences::~Preferences()
{
    delete ui;
}

void Preferences::apply()
{
    {
        QSettings settings;
        settings.setValue(IsTrafficLimited, ui->cbTrafficLimit->isChecked());
        settings.setValue(TrafficLimitKbs, ui->sbTrafficLimit->value());
        settings.setValue(VideoFolder, ui->leFolder->text());
        settings.setValue(UnlimitedLabel, !ui->maximumNumberLoads->isChecked());
        settings.setValue(MaximumNumberLoads, ui->sbMaxNum->value());
        settings.setValue(ShowSysTrayNotifications, !ui->cbHideAllTrayNotification->isChecked());

        int torrent_port = ui->torrentPortSettingEdit->text().toInt();
        if (torrent_port < 1 && torrent_port > maxPort)
        {
            torrent_port = 6881;    // default value;
        }
        settings.setValue(TorrentsPort, torrent_port);
        settings.setValue(TorrentsPortIsAuto, ui->radioTorrentPortAuto->isChecked());
        if (!ui->radioTorrentPortAuto->isChecked() && TorrentManager::Instance()->port() != torrent_port)
        {
            TorrentManager::Instance()->setListeningPort(torrent_port, torrent_port);
        }

        settings.setValue(TorrentsSequentialDownload, ui->torrentStartAsSequel->isChecked());

        // Speeds
        settings.setValue(IsTrafficUploadLimited, ui->torrentSpeedLimitedCheckbox->isChecked());
        settings.setValue(TrafficUploadLimitKbs, ui->torrentSpeedUploadLimitSpin->value());
        TorrentManager::Instance()->setDownloadLimit(ui->cbTrafficLimit->isChecked() ? ui->sbTrafficLimit->value() * 1024 : 0);
        TorrentManager::Instance()->setUploadLimit(ui->torrentSpeedLimitedCheckbox->isChecked() ? ui->torrentSpeedUploadLimitSpin->value() * 1024 : 0);

        bool shouldBeDefaultTorrentApp = ui->torrentAssociateCheckbox->isChecked();
        if (shouldBeDefaultTorrentApp != utilities::isDefaultTorrentApp())
        {
            if (shouldBeDefaultTorrentApp)
            {
                utilities::setDefaultTorrentApp(winId());
            }
            else
            {
                utilities::unsetDefaultTorrentApp();
            }
        }

        QString language = ui->comboBoxLanguage->itemData(ui->comboBoxLanguage->currentIndex()).toString();
        if (settings.value(ln, ln_Default) != language)
        {
            settings.setValue(ln, language);

            if (auto trans = dynamic_cast<utilities::Translatable*>(qApp))
            {
                trans->retranslateApp(language);
            }
            else
            {
                qDebug() << "Translation error: Application is not Translatable";
            }

            ui->retranslateUi(this);
        }

#ifdef Q_OS_WIN
        const bool startLIIIOnStartingWindows = ui->startLIIIOnStartingWindows->isChecked();
        if (m_startLIIIOnStartingWindows != startLIIIOnStartingWindows
            && utilities::setAutorun(startLIIIOnStartingWindows))
        {
            m_startLIIIOnStartingWindows = startLIIIOnStartingWindows;
        }
#endif

    } // settings scope

    initMainSettings(); // to re-init fixed parameters

    dataChanged(false); // reset changed flag

    emit newPreferencesApply();
}

bool Preferences::checkData()
{
    // check output folder
    QString pathDir = QDir::toNativeSeparators(QDir::cleanPath(ui->leFolder->text()));
    QDir path(pathDir);
    // absolute path is necessary since mkpath works weird otherwise (2 dirs are created instead of one)
    if (path.isRelative())
    {
        pathDir = path.absolutePath();
    }

    if (!path.exists(pathDir) && !path.mkpath(pathDir))
    {
        ui->tabWidget->setCurrentIndex(0);
        QMessageBox::critical(this, ::Tr::Tr(PROJECT_FULLNAME_TRANSLATION), tr("The folder \"%1\" cannot be created. Please change.").arg(pathDir));
        ui->leFolder->setFocus();
        return false;
    }

    bool canProceed = false;
    QFileInfo newDirPath(pathDir);
    if (newDirPath.exists() && newDirPath.isDir())
    {
        QString testFileName(pathDir);
        if (!testFileName.endsWith(QDir::separator()))
        {
            testFileName.append(QDir::separator());
        }
        testFileName.append("test");
        while (QFile::exists(testFileName))
        {
            testFileName.append("-");
        }
        QFile testFile(testFileName);
        if (testFile.open(QIODevice::WriteOnly))
        {
            canProceed = true;
            testFile.close();
            testFile.remove();
        }
    }
    if (!canProceed)
    {
        ui->tabWidget->setCurrentIndex(0);
        QMessageBox::critical(
            this, ::Tr::Tr(PROJECT_FULLNAME_TRANSLATION),
            tr("The folder \"%1\" cannot be used, as there is no write access for %2. Please choose another one.").arg(pathDir, ::Tr::Tr(PROJECT_FULLNAME_TRANSLATION)));
        ui->leFolder->setFocus();
        return false;
    }

    // check torrents port
    if (!checkPortOk(ui->torrentPortSettingEdit->text().toInt()))
    {
        return false;
    }

    // TODO: add other checks here

    return true;
}

void Preferences::accept()
{
    if (!checkData())
    {
        return;
    }
    if (isDataChanged())
    {
        apply();
    }
    QDialog::accept();
}

void Preferences::on_btnResetWarnings_clicked()
{
    QMessageBox msgBox(
        QMessageBox::NoIcon,
        ::Tr::Tr(PROJECT_FULLNAME_TRANSLATION),
        ::Tr::Tr(RESET_WARNINGS_TEXT),
        QMessageBox::Yes | QMessageBox::No,
        this);

    if (msgBox.exec() == QMessageBox::Yes)
    {
        QSettings settings;
        settings.setValue(ShowCancelWarning, true);
        settings.setValue(ShowCleanupWarning, true);
        settings.setValue(ShowExitWarning, true);
        settings.setValue(ShowSysTrayNotifications, true);
        settings.setValue(ShowAddTorrentDialog, true);
        settings.setValue(ShowAssociateTorrentDialog, true);
    }
}

void Preferences::on_pbBrowse_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, ::Tr::Tr(DOWNLOAD_TO_LABEL), ui->leFolder->text());
    if (!dir.isEmpty())
    {
        ui->leFolder->setText(dir);
        dataChanged();
    }
}

void Preferences::fillLanguageComboBox()
{
    QStringList fileNames = utilities::Translatable::getFilenames();
    QTranslator translator;

    bool added = false;

    Q_FOREACH(const QString & fileName, fileNames)
    {
        if (!translator.load(fileName))
        {
            continue;
        }

        QString locName = utilities::locationString(fileName);
        Q_ASSERT(!locName.isEmpty());

        QString langStr = utilities::languageString(LANGUAGE_NAME, locName, translator);
        Q_ASSERT(!locName.isEmpty());

        if (!langStr.isEmpty())
        {
            ui->comboBoxLanguage->addItem(langStr, locName);
            added = true;
        }
    }

    if (!added)
    {
        ui->comboBoxLanguage->addItem(LANGUAGE_NAME.key, "en");
    }

    int index = ui->comboBoxLanguage->findData(
        QSettings().value(ln, ln_Default));
    if (index != -1)
    {
        ui->comboBoxLanguage->setCurrentIndex(index);
    }
}

void Preferences::dataChanged(bool b)
{
    m_dataChanged = b;
    ui->buttonApply->setEnabled(m_dataChanged);
}

void Preferences::initMainSettings()
{
    ui->leFolder->setText(global_functions::GetVideoFolder());

    QSettings settings;

    const QString portStr = settings.value(TorrentsPort, TorrentsPort_Default).toString();
    ui->torrentPortSettingEdit->setText(portStr);
    (settings.value(TorrentsPortIsAuto, TorrentsPortIsAuto_Default).toBool()
        ? ui->radioTorrentPortAuto : ui->radioTorrentPortManual)->setChecked(true);

    ui->torrentStartAsSequel->setChecked(settings.value(TorrentsSequentialDownload, TorrentsSequentialDownload_Default).toBool());
    ui->torrentAssociateCheckbox->setChecked(utilities::isDefaultTorrentApp());

    ui->torrentSpeedLimitedCheckbox->setChecked(settings.value(IsTrafficUploadLimited, IsTrafficUploadLimited_Default).toBool());
    ui->torrentSpeedUploadLimitSpin->setValue(settings.value(TrafficUploadLimitKbs, TrafficUploadLimitKbs_Default).toInt());

    ui->maximumNumberLoads->setChecked(!settings.value(UnlimitedLabel, UnlimitedLabel_Default).toBool());
    ui->sbMaxNum->setValue(settings.value(MaximumNumberLoads, MaximumNumberLoads_Default).toInt());

    ui->cbTrafficLimit->setChecked(settings.value(IsTrafficLimited, IsTrafficLimited_Default).toBool());
    ui->sbTrafficLimit->setValue(settings.value(TrafficLimitKbs, TrafficLimitKbs_Default).toInt());

    ui->cbHideAllTrayNotification->setChecked(!settings.value(ShowSysTrayNotifications, ShowSysTrayNotifications_Default).toBool());
}

void Preferences::on_torrentRandomPortButton_clicked()
{
    // Update of data will be called from on_torrentPortSettingEdit_textChanged()
    ui->torrentPortSettingEdit->setText(QString::number(rand() % 0xFFFF));
}

void Preferences::on_radioTorrentPortAuto_toggled(bool is_set)
{
    ui->torrentPortSettingEdit->setEnabled(!is_set);
    ui->torrentRandomPortButton->setEnabled(!is_set);

    VERIFY(QMetaObject::invokeMethod(this, "checkPortAvalible", Qt::QueuedConnection, Q_ARG(QString, ui->torrentPortSettingEdit->text())));
    dataChanged();
}


void Preferences::on_torrentPortSettingEdit_textChanged(const QString& text)
{
    if (!ui->radioTorrentPortManual->isChecked())
    {
        // Update of data will be called from on_radioTorrentPortAuto_toggled()
        ui->radioTorrentPortManual->setChecked(true);
    }
    else
    {
        VERIFY(QMetaObject::invokeMethod(this, "checkPortAvalible", Qt::QueuedConnection, Q_ARG(QString, text)));
        dataChanged();
    }
}


void Preferences::checkPortAvalible(const QString& textPort)
{
    int targetPort = textPort.toInt();

    const char* reason = 0;
    bool isPortOk = checkPortOk(targetPort, &reason);

    ui->label_portNumberInvalid->setVisible(!isPortOk);

#ifdef DEVELOPER_FEATURES
    utilities::Tr::Translation msgText = {"Preferences", reason};
    Tr::SetTr(ui->label_portNumberInvalid, &QLabel::setText, msgText);
#endif
}

bool Preferences::checkPortOk(int targetPort, const char** reason)
{
    return
        ui->radioTorrentPortAuto->isChecked() ||
        TorrentManager::Instance()->port() == targetPort ||
        utilities::CheckPortAvailable(targetPort, reason);
}

void Preferences::keyPressEvent(QKeyEvent* event)
{
#ifdef DEVELOPER_FEATURES
    if (event->key() == Qt::Key_F5)
    {
        QFile file(QApplication::applicationDirPath() + "/../../../resources/LIII/style.css");
        if (file.exists() && file.open(QIODevice::ReadOnly))
        {
            setStyleSheet(file.readAll());
            file.close();
        }
    }
#endif
    QDialog::keyPressEvent(event);
}

void Preferences::on_torrentAssociateCheckBoxChanged(int state)
{
#ifndef DEVELOPER_FEATURES
    if (Qt::Checked == state)
    {
        ui->torrentAssociateCheckbox->setEnabled(false);
    }
#endif
}

//#endif

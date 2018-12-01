#include "translatable.h"

#include <QApplication>
#include <QTranslator>
#include <QDir>
#include <QStringList>
#include "translation.h"
#include <algorithm>

namespace {

QString translationFilePrefix = "translations_";

QString getTranslationsFolder()
{
#ifdef Q_OS_WIN32
    return QDir(QApplication::applicationDirPath()).absoluteFilePath("Translations");
#elif defined(Q_OS_LINUX)
    QDir trPath = QDir(QString("/usr/share/") + PROJECT_NAME);
    if (trPath.exists())
    {
        return trPath.absoluteFilePath("translations");
    }
    return QDir(QApplication::applicationDirPath()).absoluteFilePath("translations");
#elif defined(Q_OS_MAC)
    QStringList binPathList = QApplication::applicationDirPath().split(QDir::separator(), QString::SkipEmptyParts);
    binPathList.removeLast();
    binPathList <<  "Resources" << "Translations";
    return  binPathList.join(QDir::separator()).prepend(QDir::separator());
#else
    Q_ASSERT(false && "Translations forlder probably has not been set correctly");
    return QDir(QApplication::applicationDirPath()).absoluteFilePath("Translations"); // just to return something
#endif
}

QStringList getFilenames()
{
    QDir dir(getTranslationsFolder());

    // get names of language resources files
    QStringList fileNames = dir.entryList(QStringList(translationFilePrefix + "*.qm"));

    // replace files names with full path
    for (auto& str : fileNames)
        str = dir.absoluteFilePath(str);

    return fileNames;
}

QString languageString(const utilities::Tr::Translation& translation, const QString& locName, QTranslator& translator)
{
    QLocale loc(locName);
    QString langStr = translator.translate(translation.context, translation.key, translation.disambiguation);
    if (langStr.isEmpty())
    {
        langStr = QString("%1 (%2)").arg(loc.nativeLanguageName(), QLocale::languageToString(loc.language()));
    }

    return langStr;
}

QString locationString(const QString& fileName)
{
    QString locName;

    // parse location from filenames like LIII_en.qm ONLY
    QRegExp rx("^.+_(\\S{2})\\.qm$", Qt::CaseInsensitive);
    if (rx.exactMatch(fileName))
    {
        locName = rx.cap(1).toLower();
    }

    return locName;
}


} // namespace

namespace utilities
{


void Translatable::retranslateApp(const QString& locale)
{
    if (translator_)
    {
        QCoreApplication::removeTranslator(translator_);
        translator_->deleteLater();
    }
    translator_ = new QTranslator(qApp);

    QString filename = translationFilePrefix + locale;
    if (translator_->load(filename, getTranslationsFolder()))
    {
        QCoreApplication::installTranslator(translator_);
    }

    Tr::RetranslateAll(qApp);
}


static const Tr::Translation LANGUAGE_NAME = {"Preferences", "English (English)"};

std::map<QString, QString> Translatable::availableLanguages()
{
    std::map<QString, QString> result;

    for (const auto& filename : getFilenames())
    {
        QTranslator translator;
        if (translator.load(filename))
        {
            QString locName = locationString(filename);
            Q_ASSERT(!locName.isEmpty());

            QString langStr = languageString(LANGUAGE_NAME, locName, translator);
            Q_ASSERT(!langStr.isEmpty());

            if (!langStr.isEmpty())
            {
                result.insert(std::make_pair(locName, langStr));
            }
        }
    }

    if (result.empty())
    {
        result.insert(std::make_pair("en", LANGUAGE_NAME.key));
    }

    return result;
}

} // namespace utilities

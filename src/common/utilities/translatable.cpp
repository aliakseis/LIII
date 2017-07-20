#include "translatable.h"

#include <QApplication>
#include <QTranslator>
#include <QDir>
#include <QStringList>
#include "translation.h"
#include <algorithm>

namespace utilities
{

QString Translatable::translationFilePrefix = "translations_";

QString Translatable::getTranslationsFolder()
{
#ifdef Q_OS_WIN32
    return QDir(QApplication::applicationDirPath()).absoluteFilePath("Translations");
#elif defined(Q_OS_LINUX)
    QDir trPath = QDir(QString("/usr/share/") + PROJECT_NAME);
    if (trPath.exists())
    {
        return trPath.absoluteFilePath("translations");
    }
    else
    {
        return QDir(QApplication::applicationDirPath()).absoluteFilePath("translations");
    }
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

void Translatable::retranslateApp(const QString& locale)
{
    if (translator_)
    {
        qApp->removeTranslator(translator_);
        translator_->deleteLater();
    }
    translator_ = new QTranslator(qApp);

    QString filename = translationFilePrefix + locale;
    if (translator_->load(filename, getTranslationsFolder()))
    {
        qApp->installTranslator(translator_);
    }

    Tr::RetranslateAll(qApp);
}

QStringList Translatable::getFilenames()
{
    QDir dir(getTranslationsFolder());

    // get names of language resources files
    QStringList fileNames = dir.entryList(QStringList(translationFilePrefix + "*.qm"));

    // replace files names with full path
    for (auto& str : fileNames)
        str = dir.absoluteFilePath(str);

    return fileNames;
}

static const Tr::Translation LANGUAGE_NAME = {"Preferences", "English (English)"};

std::map<QString, QString> Translatable::availableLanguages() const
{
    std::map<QString, QString> result;
    QStringList fileNames = utilities::Translatable::getFilenames();

    for (const auto& filename : qAsConst(fileNames))
    {
        QTranslator translator;
        if (translator.load(filename))
        {
            QString locName = utilities::locationString(filename);
            Q_ASSERT(!locName.isEmpty());

            QString langStr = utilities::languageString(LANGUAGE_NAME, locName, translator);
            Q_ASSERT(!langStr.isEmpty());

            if (!langStr.isEmpty())
            {
                result.insert(std::make_pair(locName, langStr));
            }
        }
    }

    return result;
}

} // namespace utilities
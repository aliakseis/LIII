#include "translation.h"

#include <QtGlobal>
#include <QString>
#include <QVariant>
#include <QCoreApplication>
#include <QDebug>
#include <QTranslator>
#include <QRegExp>

namespace utilities
{

namespace Tr
{


QString TranslationRule::GetText() const
{
    return expand_.isEmpty() ? Tr(translateArgs_) : multiArg(Tr(translateArgs_), expand_.size(), expand_.data());
}

///////////////////////////////////////////////////////////////////////////////


Translation translate(const char* context, const char* sourceText, const char* disambiguation, int n)
{
    Translation tr = { context, sourceText, disambiguation, n };
    return tr;
}

QString Tr(const Translation& translateArgs)
{
    return QCoreApplication::translate(
            translateArgs.context, translateArgs.key, translateArgs.disambiguation,
            translateArgs.n);
}

Translation Plural(const Translation& translateArgs, int n)
{
    Translation plural = translateArgs;
    plural.n = n;
    return plural;
}

void Retranslate(QObject* obj)
{
    if (!obj) { return; }
    QVariant variant = obj->property(kProperty);
    if (variant.canConvert<TranslationRules>())
    {
        TranslationRules rules = variant.value<TranslationRules>();
        for (auto it = rules.begin(); it != rules.end(); ++it)
        {
            it.key()->execute(it.value()->GetText());
        }
    }
}

void RetranslateAll(QObject* obj)
{
    if (!obj) { return; }
    Retranslate(obj);
    auto objects = obj->findChildren<QObject*>();
    for (auto o : qAsConst(objects))
    {
        RetranslateAll(o);
    }
}

} // namespace Tr

QString languageString(const Tr::Translation& translation, const QString& locName, QTranslator& translator)
{
    QLocale loc(locName);
    QString langStr = translator.translate(translation.context, translation.key, translation.disambiguation);
    if (langStr.isEmpty())
    {
        langStr = QString("%1 (%2)").arg(loc.nativeLanguageName())
                  .arg(QLocale::languageToString(loc.language()));
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


} // namespace utilities
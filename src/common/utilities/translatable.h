#pragma once

#include <QString>
#include <map>

class QTranslator;

namespace utilities
{

// inherite your application from this class and have fun ;)
class Translatable
{
public:
    Translatable() : translator_(nullptr) {}
    void retranslateApp(const QString& locale);
    static QStringList getFilenames();
    std::map<QString, QString> availableLanguages() const;
    static void setTransFilesPrefix(const QString& langPref) {translationFilePrefix = langPref + "_";}

private:
    static QString getTranslationsFolder();
    static QString translationFilePrefix;
    QTranslator* translator_;
};

} // namespace utilities

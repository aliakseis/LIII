#pragma once

#include <QString>
#include <map>

class QTranslator;

namespace utilities
{

// inherite your application from this class
class Translatable
{
public:
    void retranslateApp(const QString& locale);
    static std::map<QString, QString> availableLanguages();

private:
    QTranslator* translator_ {};
};

} // namespace utilities

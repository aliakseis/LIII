#pragma once

#include <QMap>
#include <QMetaType>
#include <QMetaProperty>
#include <QStringList>

enum class QCborSimpleType : quint8;

typedef QMap<QString, QObject*> QObjectMap;

// for serializing
Q_DECLARE_METATYPE(QObjectList)
Q_DECLARE_METATYPE(QObjectMap)


namespace utilities
{

extern const QString classNameAttribute;
extern const QString objectIdAttribute;
extern const QString keyValueAttribute;
extern const char kObjectNameProperty[];

} // namespace utilities

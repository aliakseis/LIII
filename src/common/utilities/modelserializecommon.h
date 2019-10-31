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

const char kClassNameAttribute[] = "className";
const char kKeyValueAttribute[] = "keyValue";
const char kObjectIdAttribute[] = "objectId";
const char kObjectNameProperty[] = "objectName";

} // namespace utilities


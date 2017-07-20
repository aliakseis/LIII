#pragma once

#include <QXmlStreamWriter>
#include <unordered_set>
#include <map>
#include <QStringList>

namespace utilities
{

typedef QString StringCache[20]; // enough for primitive types

class ModelSerializer
{

public:
    explicit ModelSerializer(QXmlStreamWriter& stream)
        : m_stream(stream)
    {
    }
    void serialize(QObject* object, const QString& name)
    {
        serializeObjectInternal(object, name);
    }

private:
    void serializeObjectInternal(QObject* object, const QString& name, const QString* keyValue = 0);

    const QStringList& getObjectPropNames(const QMetaObject* metaObject);

private:
    QXmlStreamWriter& m_stream;
    std::unordered_set<QObject*> m_allObjects;

    std::map<QLatin1String, QStringList> m_objectPropNames;
    StringCache m_cache;
};

} // namespace utilities

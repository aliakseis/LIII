#pragma once

#include <QXmlStreamReader>
#include <map>
#include <QHash>
#include <QPointer>
#include <QMetaProperty>
#include <stddef.h>

#include "patricia.h"

namespace utilities
{

class ModelDeserializer
{
public:
    explicit ModelDeserializer(QXmlStreamReader& stream)
        : m_stream(stream)
    {
    }
    bool deserialize(QObject* object, const QString& name = QString())
    {
        return m_stream.readNextStartElement()
               && (name.isEmpty() || m_stream.name().toString() == name)
               && deserializeObjectInternal(object);
    }

private:
    bool deserializeObjectInternal(QObject* object);

    typedef PatriciaTrie<QMetaProperty> MetaTrie;
    struct MetaTries
    {
        MetaTrie properties, children, mapChildren;
    };

    const MetaTries* getMetaTries(QObject* object);

private:
    QXmlStreamReader& m_stream;
    QHash<uintptr_t, QPointer<QObject> > m_allObjects;

    std::map<QLatin1String, MetaTries> m_objectProperties;
};

} // namespace utilities

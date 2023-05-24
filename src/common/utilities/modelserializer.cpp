#include "modelserializer.h"

#include "modelserializecommon.h"

#include <algorithm>
#include <iterator>
#include <type_traits>

#ifdef _MSC_VER
#pragma intrinsic(memcpy, strcmp)
#endif

namespace utilities
{

bool isObjectType(int type)
{
    const int objectTypeIds[] =
    {
        qMetaTypeId<QObjectList>(),
        qMetaTypeId<QObjectMap>(),
        qMetaTypeId<QObject*>(),
        QVariant::ByteArray,
    };

    return std::find(std::begin(objectTypeIds), std::end(objectTypeIds), type) != std::end(objectTypeIds);
}

template <int radix, typename T>
inline QString ttoa(T value, StringCache& m_cache)
{
    QChar buf[24];

    QChar* const end = std::end(buf) - 1;
    QChar* cur = end;
    *cur = 0;

    const bool minus = radix == 10 && std::is_signed<T>::value && value < 0;
    if (minus)
    {
        value = T(-(typename std::make_signed<T>::type)value);
    }

    do
    {
        const auto digval = unsigned(value % radix);
        *(--cur) = (radix <= 10 || digval < 10) ? digval + '0' : (digval - 10 + 'a');
        value /= radix;
    }
    while (value > 0);

    if (minus)
    {
        *(--cur) = '-';
    }

    const int len = end - cur;

    if (len > 0 && (size_t)len <= sizeof(m_cache) / sizeof(m_cache[0]))
    {
        QString& s = m_cache[len - 1];
        if (s.isEmpty())
        {
            s = QString(cur, len);
            return s;
        }
        if (s.isDetached())
        {
            Q_ASSERT(s.length() == len);
            memcpy(s.data(), cur, len * sizeof(QChar));
            Q_ASSERT(QChar(0) == s.data()[len]);
            return s;
        }
    }

    return QString(cur, len);
}


template<typename T> QString primitiveTypeToString(QVariant& v, StringCache& cache)
{
    return ttoa<10>(v.value<T>(), cache);
}

template<>
QString primitiveTypeToString<void>(QVariant&, StringCache&)
{
    return QString();
}

template<>
QString primitiveTypeToString<std::nullptr_t>(QVariant&, StringCache&)
{
    return QString();
}

template<>
QString primitiveTypeToString<QCborSimpleType>(QVariant&, StringCache&)
{
    return QString();
}

template<>
QString primitiveTypeToString<bool>(QVariant& v, StringCache&)
{
    static const QString values[2] = { QStringLiteral("0"), QStringLiteral("1") };
    return values[v.toBool()];
}

template<>
QString primitiveTypeToString<float>(QVariant& v, StringCache&)
{
    return v.toString();
}

template<>
QString primitiveTypeToString<double>(QVariant& v, StringCache&)
{
    return v.toString();
}


void ModelSerializer::serializeObjectInternal(
    QObject* object,
    const QString& name,
    const QString* keyValue /* = 0 */)
{
    if (0 == object)
    {
        return;
    }

    m_stream.writeStartElement(name);
    if (keyValue != 0)
    {
        m_stream.writeAttribute(keyValueAttribute, *keyValue);
    }
    m_stream.writeAttribute(objectIdAttribute, ttoa<36>((qulonglong) object, m_cache));

    if (m_allObjects.insert(object).second)
    {
        const QMetaObject* metaObject = object->metaObject();

        m_stream.writeAttribute(classNameAttribute, metaObject->className());

        const QStringList& objectPropNames = getObjectPropNames(metaObject);

        const int propCount = metaObject->propertyCount();

        for (int i = 0; i < propCount; ++i)
        {
            QMetaProperty prop = metaObject->property(i);

            if (isObjectType(prop.userType()))
            {
                continue;
            }

            const char* szPropName = prop.name();
            if (0 == strcmp(kObjectNameProperty, szPropName))
            {
                continue;
            }
            const QString& propName = objectPropNames[i];

            QVariant value = prop.read(object);
            if (QVariant::StringList == value.type())
            {
                QStringList qStringList = value.toStringList();
                QString separatedList = qStringList.join(QStringLiteral("|"));
                m_stream.writeAttribute(propName, separatedList);
            }
            else
            {
                if (prop.isEnumType())
                {
                    QMetaEnum qme = prop.enumerator();
                    const char* key = qme.valueToKey(value.toInt());
                    if (key != 0)
                    {
                        value = key;
                    }
                }

                QString strValue;

#define PRIMITIVE_TYPE_CASE(TypeName, TypeNameID, RealType) \
case QMetaType::TypeName: strValue = primitiveTypeToString<RealType>(value, m_cache); break;

                switch (value.userType())
                {
                    QT_FOR_EACH_STATIC_PRIMITIVE_TYPE(PRIMITIVE_TYPE_CASE)
                default: strValue = value.toString();
                }

#undef PRIMITIVE_TYPE_CASE

                m_stream.writeAttribute(propName, strValue);
            }
        }

        // handle nodes after handling attributes
        for (int i = 0; i < propCount; ++i)
        {
            QMetaProperty prop = metaObject->property(i);

            if (!isObjectType(prop.userType()))
            {
                continue;
            }

            const QString& propName = objectPropNames[i];

            QVariant value = prop.read(object);
            const int userType = value.userType();
            if (qMetaTypeId<QObjectMap>() == userType)
            {
                const QObjectMap& qObjectList = *static_cast<const QObjectMap*>(value.constData());
                for (auto it(qObjectList.begin()), itEnd(qObjectList.end()); it != itEnd; ++it)
                {
                    const QString& key = it.key();
                    serializeObjectInternal(*it, propName, &key);
                }
            }
            else if (qMetaTypeId<QObjectList>() == userType)
            {
                const QObjectList& qObjectList = *static_cast<const QObjectList*>(value.constData());
                for (QObject* item : qObjectList)
                {
                    serializeObjectInternal(item, propName);
                }
            }
            else if (qMetaTypeId<QObject*>() == userType)
            {
                QObject* const qObject = *static_cast<QObject* const*>(value.constData());
                serializeObjectInternal(qObject, propName);
            }
            else if (QVariant::ByteArray == userType)
            {
                const QByteArray& qByteArray = *static_cast<const QByteArray*>(value.constData());
                m_stream.writeStartElement(propName);
                m_stream.writeCDATA(qByteArray);
                m_stream.writeEndElement();
            }
        }
    }

    m_stream.writeEndElement();
}

const QStringList& ModelSerializer::getObjectPropNames(const QMetaObject* metaObject)
{
    QLatin1String name(metaObject->className());
    auto it = m_objectPropNames.find(name);
    if (it != m_objectPropNames.end())
    {
        return it->second;
    }

    QStringList& list = m_objectPropNames[name];
    Q_ASSERT(list.isEmpty());
    const int propCount = metaObject->propertyCount();
    for (int i = 0; i < propCount; ++i)
    {
        list.push_back(metaObject->property(i).name());
    }
    return list;
}


} // namespace utilities

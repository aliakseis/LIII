#define _SCL_SECURE_NO_WARNINGS

#include "modeldeserializer.h"

#include <algorithm>
#include <type_traits>
#include <utility>

#include "iparentadvice.h"

#include "modelserializecommon.h"

#include "instantiator.h"

#include <QDateTime>
#include <QDebug>

namespace {

template<int radix, typename C, typename T>
inline bool atot(T& result, const C* begin, const C** end = nullptr)
{
    bool ok = false;
    result = 0;
    for (;; ++begin)
    {
        const C ch = *begin;
        const int elem = (radix <= 10 || ch <= '9') ? ch - '0' : ((ch | 0x20) - 'a' + 10);
        if (elem < 0 || elem >= radix)
        {
            break;
        }
        result = result * radix + elem;
        ok = true;
    }

    if (end)
    {
        *end = begin;
    }

    return ok;
}

/*
    If the \a format is Qt::ISODate, the string format corresponds
    to the ISO 8601 extended specification for representations of
    dates and times, taking the form YYYY-MM-DDTHH:MM:SS[Z|[+|-]HH:MM],
    depending on the timeSpec() of the QDateTime. If the timeSpec()
    is Qt::UTC, Z will be appended to the string; if the timeSpec() is
    Qt::OffsetFromUTC, the offset in hours and minutes from UTC will
    be appended to the string.
*/
template<typename C>
inline QDateTime parseIsoDateTime(const C* str, bool& ok)
{
    int year, month, day, hour, min, sec = 0;
    ok = atot<10>(year, str, &str)
         && '-' == *str++
         && atot<10>(month, str, &str)
         && '-' == *str++
         && atot<10>(day, str, &str)
         && 'T' == *str++
         && atot<10>(hour, str, &str)
         && ':' == *str++
         && atot<10>(min, str, &str)
         && (':' != *str++ || atot<10>(sec, str, &str));

    return ok ? QDateTime(QDate(year, month, day), QTime(hour, min, sec)) : QDateTime();
}


typedef std::remove_cv_t<std::remove_pointer_t<decltype(std::declval<QString>().utf16())>> Utf16Char;

inline const Utf16Char* utf16(const QStringRef& s)
{
    static const Utf16Char zero(0);
    return s.isNull() ? &zero : s.string()->utf16() + s.position();
}


template<typename T>
inline QVariant parsePrimitiveType(const QStringRef& s, bool& ok)
{
    auto str = utf16(s);
    T result(0);
    const bool isNegative(std::is_signed<T>::value && '-' == *str);
    if (isNegative)
    {
        ++str;
    }
    ok = atot<10>(result, str);
    if (isNegative)
    {
        result = T(-(typename std::make_signed<T>::type)result);
    }

    return QVariant::fromValue(result);
}

template<>
inline QVariant parsePrimitiveType<void>(const QStringRef&, bool&)
{
    return QVariant();
}

template<>
inline QVariant parsePrimitiveType<std::nullptr_t>(const QStringRef&, bool&)
{
    return QVariant();
}

template<>
inline QVariant parsePrimitiveType<QCborSimpleType>(const QStringRef&, bool&)
{
    return QVariant();
}

// Returns true if the variant has type String or ByteArray and its lower-case content is not empty, "0" or "false";
// otherwise returns false.
template<>
inline QVariant parsePrimitiveType<bool>(const QStringRef& s, bool& ok)
{
    ok = true;
    if (s.isEmpty())
    {
        return false;
    }
    auto str = utf16(s);
    return !((1 == s.length() && '0' == str[0])
             || (5 == s.length() && 'f' == str[0] && 'a' == str[1] && 'l' == str[2] && 's' == str[3] && 'e' == str[4]));
}

template<>
inline QVariant parsePrimitiveType<double>(const QStringRef& s, bool& ok)
{
    return s.toString().toDouble(&ok);
}

template<>
inline QVariant parsePrimitiveType<float>(const QStringRef& s, bool& ok)
{
    return s.toString().toFloat(&ok);
}


typedef PatriciaTrie<QMetaProperty>::TrieNode MetaNode;

// self cleaning containers
template <typename T>
class MapStringToT : public QMap<MetaNode*, T>
{
public:
    MapStringToT() = default;
    ~MapStringToT()
    {
        std::for_each<typename QMap<MetaNode*, T>::const_iterator, void (*)(const T&)>(
            this->constBegin(), this->constEnd(), qDeleteAll<T>);
    }
    MapStringToT(const MapStringToT&) = delete;
    MapStringToT& operator =(const MapStringToT&) = delete;
};

} // namespace


namespace utilities
{


const QString classNameAttribute(kClassNameAttribute);
const QString objectIdAttribute(kObjectIdAttribute);
const QString keyValueAttribute(kKeyValueAttribute);

const int qObjectListId = qMetaTypeId<QObjectList>();
const int qStringListId = qMetaTypeId<QStringList>();
const int qObjectMapId = qMetaTypeId<QObjectMap>();
const int qObjectPtrId = qMetaTypeId<QObject*>();

template<typename T, typename F>
inline void setChildren(QObject* object, T& children, F fun)
{
    for (auto it(children.begin()), itEnd(children.end()); it != itEnd;)
    {
        auto item = it.key();

        const QMetaProperty& prop = *item->get_value();
        const int userType = prop.userType();
        if (void* pData = fun(it, userType))
        {
            QVariant value(userType, pData);
            prop.write(object, value);
            // we transfer ownership here
            it = children.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool ModelDeserializer::deserializeObjectInternal(QObject* object)
{
    if (0 == object)
    {
        return false;
    }

    Q_ASSERT(qObjectListId != 0);
    Q_ASSERT(qStringListId != 0);
    Q_ASSERT(qObjectMapId != 0);
    Q_ASSERT(qObjectPtrId != 0);

    MapStringToT<QObjectList> children;
    MapStringToT<QObjectMap> mapChildren;

    // save attributes before proceeding further
    QXmlStreamAttributes attributes = m_stream.attributes();

    const MetaTries* tries = getMetaTries(object);

    while (m_stream.readNextStartElement())
    {
        QStringRef name = m_stream.name();
        QStringRef className = m_stream.attributes().value(classNameAttribute);
        QStringRef strObjectId = m_stream.attributes().value(objectIdAttribute);
        if (className.isEmpty() && strObjectId.isEmpty())
        {
            QString text = m_stream.readElementText();
            object->setProperty(name.toLatin1().data(), text);
            continue;
        }
        QString keyValue = m_stream.attributes().value(keyValueAttribute).toString();

        QObject* instance = 0;

        QHash<uintptr_t, QPointer<QObject> >::iterator it;

        uintptr_t objectId = 0;
        if (!strObjectId.isEmpty())
        {
            atot<36>(objectId, utf16(strObjectId));
        }

        if (objectId && (it = m_allObjects.find(objectId)) != m_allObjects.end())
        {
            if (it.value().isNull())
            {
                qWarning() << "Repeating object was deleted";
                return false;
            }
            instance = it.value();
            if (m_stream.readNextStartElement()) // don't remove - deserialize magic
            {
                qWarning() << "Document structure is wrong: child element found in the alias node";
                return false;
            }
        }
        else
        {
            instance = Instantiator::instance().instantiate(className, object);

            if (!deserializeObjectInternal(instance))
            {
                qWarning()
                        << __FUNCTION__ << ": cannot serialize object with name: " << name
                        << ", className: " << className;
                return false;
            }

            if (objectId)
            {
                m_allObjects[objectId] = instance;
            }
        }

        const auto utfName(utf16(name));
        if (keyValue.isEmpty())
        {
            auto item = tries->children.find(utfName, name.length());
            if (0 != item && 0 != item->get_value())
            {
                children[item].push_back(instance);
            }
        }
        else
        {
            auto item = tries->mapChildren.find(utfName, name.length());
            if (0 != item && 0 != item->get_value())
            {
                mapChildren[item][keyValue] = instance;
            }
        }
    }

    if (m_stream.hasError())
    {
        qWarning()
                << __FUNCTION__ << ": serialization error: " << m_stream.errorString()
                << " at line: " << m_stream.lineNumber() << ", column: " << m_stream.columnNumber();
        return false;
    }

    // Do stuffing after validation
    // so that serialized object would not be touched if error happens

    for (const auto& attribute : qAsConst(attributes))
    {
        const QStringRef propName(attribute.name());
        const auto utfPropName(utf16(propName));
        auto item = tries->properties.find(utfPropName, propName.length());
        if (0 == item)
        {
            continue;
        }
        if (0 == item->get_value())
        {
            continue;
        }
        //if (propName != item->get_key())
        if (!(std::equal(utfPropName, utfPropName + propName.length(), item->get_key())
                && item->get_key()[propName.length()] == 0))
        {
            continue;
        }

        const QMetaProperty& prop = *item->get_value();
        const int userType = prop.userType();

        if (qStringListId == userType)
        {
            QString separatedList = attribute.value().toString();
            QStringList list = separatedList.split('|');
            QVariant value(list);
            prop.write(object, value);
        }
        else if (prop.isEnumType())
        {
            QByteArray strValue = attribute.value().toLatin1().trimmed();
            if (!strValue.isEmpty())
            {
                bool ok = false;
                int iValue = strValue.toInt(&ok);
                QVariant value = ok ? iValue : prop.enumerator().keyToValue(strValue.data());
                prop.write(object, value);
            }
        }
        else
        {
            QStringRef strValue = attribute.value();
            QVariant value;

            bool ok = true;

#define PRIMITIVE_TYPE_CASE(TypeName, TypeNameID, RealType) \
case QMetaType::TypeName: value = parsePrimitiveType<RealType>(strValue, ok); break;

            switch (userType)
            {
                QT_FOR_EACH_STATIC_PRIMITIVE_TYPE(PRIMITIVE_TYPE_CASE)
            case QMetaType::QDateTime: value = parseIsoDateTime(utf16(strValue), ok); break;
            default: value = strValue.toString();
            }

#undef PRIMITIVE_TYPE_CASE

            if (!ok)
            {
                if (!strValue.isEmpty())
                {
                    qWarning()
                            << __FUNCTION__ << ": could not convert property with name: " << propName
                            << " and value: " << attribute.value() << " to type: " << prop.type();
                }
            }
            else if (!prop.write(object, value) && !strValue.isEmpty())
            {
                qWarning()
                        << __FUNCTION__ << ": could not write property with name: " << propName
                        << " and value: " << attribute.value() << " and type: " << prop.type();
            }
        }
    }

    setChildren(
        object,
        mapChildren,
        [](MapStringToT<QObjectMap>::iterator it, const int userType)
    {
        return (qObjectMapId == userType) ? (void*)&*it : 0;
    });

    setChildren(
        object,
        children,
        [](MapStringToT<QObjectList>::iterator it, const int userType)
    {
        return (qObjectListId == userType) ? (void*)&*it : ((qObjectPtrId == userType) ? & (*it)[0] : 0);
    });

    Q_ASSERT_X(children.empty(),  Q_FUNC_INFO, "Document structure has changed");
    Q_ASSERT_X(mapChildren.empty(), Q_FUNC_INFO, "Document structure has changed");

    return true;
}

const ModelDeserializer::MetaTries* ModelDeserializer::getMetaTries(QObject* object)
{
    const QMetaObject* metaObject = object->metaObject();
    QLatin1String name(metaObject->className());
    auto it = m_objectProperties.find(name);
    if (it != m_objectProperties.end())
    {
        return &it->second;
    }

    Q_ASSERT(qObjectListId != 0);
    Q_ASSERT(qObjectMapId != 0);
    Q_ASSERT(qObjectPtrId != 0);

    MetaTries& tries = m_objectProperties[name];

    const int propCount = metaObject->propertyCount();
    for (int i = 0; i < propCount; ++i)
    {
        QMetaProperty prop = metaObject->property(i);
        const char* propName = prop.name();
        if (0 == strcmp(kObjectNameProperty, propName))
        {
            continue;
        }
        const int userType = prop.userType();
        if (qObjectListId == userType || qObjectPtrId == userType)
        {
            tries.children.insert(prop, propName);
        }
        else if (qObjectMapId == userType)
        {
            tries.mapChildren.insert(prop, propName);
        }
        else
        {
            tries.properties.insert(prop, propName);
        }
    }

    return &tries;
}

} // namespace utilities

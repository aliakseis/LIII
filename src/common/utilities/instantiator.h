#pragma once

// http://lists.trolltech.com/qt-interest/2006-01/thread00087-0.html

#include <QObject>
#include <QString>
#include <QVector>
#include <algorithm>

#include "singleton.h"
#include "iparentadvice.h"


struct FactoryInterface
{
    explicit FactoryInterface(const QString& className) : m_className(className) {}
    virtual ~FactoryInterface() {}
    virtual QObject* make(QObject* adviceParent) const = 0;

    const QString m_className;
};

struct FactoryInterfaceComparator
{
    bool operator()(const FactoryInterface* left, const QString& right) const
    {
        return left->m_className < right;
    }
    bool operator()(const QString& left, const FactoryInterface* right) const
    {
        return left < right->m_className;
    }

    bool operator()(const FactoryInterface* left, const QStringRef& right) const
    {
        return left->m_className.compare(right) < 0;
    }
    bool operator()(const QStringRef& left, const FactoryInterface* right) const
    {
        return left.compare(right->m_className) < 0;
    }
};


template<typename F> class ClassFactory : public FactoryInterface
{
public:
    explicit ClassFactory(const QString& className) : FactoryInterface(className) {}
    QObject* make(QObject* /*adviceParent*/) const override { return new F(); }
};


template<typename F> class AdvisingClassFactory : public FactoryInterface
{
public:
    explicit AdvisingClassFactory(const QString& className) : FactoryInterface(className) {}
    QObject* make(QObject* adviceParent) const override
    {
        F* result = new F();
        static_cast<IParentAdvice*>(result)->setAdviceParent(adviceParent);
        return result;
    }
};


class Instantiator : public Singleton<Instantiator>
{
friend class Singleton<Instantiator>;
public:
    template<typename T>
    QObject* instantiate(const T& className, QObject* adviceParent) const
    {
        auto it = std::lower_bound(m_factories.begin(), m_factories.end(), className, FactoryInterfaceComparator());
        if (it != m_factories.end() && (*it)->m_className == className)
        {
            return (*it)->make(adviceParent);
        }
        return 0;
    }

    template<typename T>
    void registerClass(const QString& className)
    {
        auto it = std::lower_bound(m_factories.begin(), m_factories.end(), className, FactoryInterfaceComparator());
        Q_ASSERT(it == m_factories.end() || (*it)->m_className > className);
        m_factories.insert(it, newFactory<T>(className, (T*)0));
    }

private:
    Instantiator() {}
    ~Instantiator() { qDeleteAll(m_factories); }

    template<typename T>
    FactoryInterface* newFactory(const QString& className, ...)
    {
        return new ClassFactory<T>(className);
    }
    template<typename T>
    FactoryInterface* newFactory(const QString& className, IParentAdvice*)
    {
        return new AdvisingClassFactory<T>(className);
    }

private:
    QVector<FactoryInterface*> m_factories;
};


#define REGISTER_QOBJECT_METATYPE(TypeName) \
    static struct Register##TypeName##MetaType \
    { \
        Register##TypeName##MetaType() \
        { \
            Instantiator::instance().registerClass<TypeName>(#TypeName); \
        } \
    } s_register##TypeName##MetaType;


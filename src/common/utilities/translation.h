#pragma once

#include <qglobal.h>
#include <functional>
#include <numeric>
#include <iterator>

#include <QCoreApplication>
#include <QMetaType>
#include <QVariant>
#include <QList>
#include <QString>
#include <QVector>
#include <QSharedPointer>
#include <QHash>

#include "utils.h"

class QTranslator;

namespace utilities
{

namespace Tr
{

const char kProperty[] = "TranslationProperty";

struct Translation
{
    const char* context;
    const char* key;
    const char* disambiguation;
    int n;
};

typedef QVector<QString> QStringVector;


struct ITranslationRule
{
    virtual ~ITranslationRule() {}
    virtual QString GetText() const = 0;
};

// regular translation rule
class TranslationRule : public ITranslationRule
{
public:
    TranslationRule(const Translation& translateArgs, std::initializer_list<QString> expand)
        : translateArgs_(translateArgs)
        , expand_(expand)
    {}

    QString GetText() const override;

private:
    Translation translateArgs_;
    QStringVector expand_;
};

class TextTranslationRule : public ITranslationRule
{
public:
    explicit TextTranslationRule(const QString& text = QString())
        : text_(text)
    {}

    QString GetText() const override { return text_; }

private:
    QString text_;
};


struct IQStringSetterDelegate
{
    virtual ~IQStringSetterDelegate() {};
    virtual void execute(const QString&) = 0;
    virtual bool equals(const IQStringSetterDelegate* other) const = 0;
    virtual size_t hash() const = 0;
};

template<typename T>
class QStringSetterDelegateImpl : public IQStringSetterDelegate
{
    typedef void (T::*Method)(const QString&);
public:
    QStringSetterDelegateImpl(T* obj, Method method)
        : m_obj(obj), m_method(method)
    {
    }
    void execute(const QString& param) override
    {
        (m_obj->*m_method)(param);
    }
    bool equals(const IQStringSetterDelegate* other) const override
    {
        const QStringSetterDelegateImpl<T>* otherImpl = dynamic_cast<const QStringSetterDelegateImpl<T>*>(other);
        return otherImpl != 0 && m_method == otherImpl->m_method;
    }
    size_t hash() const override
    {
        return std::accumulate(std::begin(m_hashArray), std::end(m_hashArray), (size_t) 0);
    }
private:
    T* m_obj;
    enum { METHOD_ARR_SIZE = (sizeof(Method) - 1) / sizeof(size_t) + 1 };
    union
    {
        Method m_method;
        size_t m_hashArray[METHOD_ARR_SIZE];
    };
};

// delegate to call retranslateUI
template<typename T, typename T_ui>
class RetranslateUICallerImpl : public IQStringSetterDelegate
{
public:
    RetranslateUICallerImpl(T* obj, T_ui* uiObj)
        : m_obj(obj), m_uiObj(uiObj)
    {
    }
    void execute(const QString&) override
    {
        m_uiObj->retranslateUi(m_obj);
    }
    bool equals(const IQStringSetterDelegate* other) const override
    {
        const RetranslateUICallerImpl<T, T_ui>* otherImpl 
            = dynamic_cast<const RetranslateUICallerImpl<T, T_ui>*>(other);
        return otherImpl != 0 && m_obj == otherImpl->m_obj;
    }
    size_t hash() const override
    {
        return std::accumulate(std::begin(m_hashArray), std::end(m_hashArray), (size_t) 0);
    }
private:
    T* m_obj;
    enum { METHOD_ARR_SIZE = (sizeof(void*) - 1) / sizeof(size_t) + 1 };
    union
    {
        T_ui* m_uiObj;
        size_t m_hashArray[METHOD_ARR_SIZE];
    };
};


inline bool operator == (const QSharedPointer<IQStringSetterDelegate>& l, const QSharedPointer<IQStringSetterDelegate>& r)
{
    return l != 0 && l->equals(r.data());
}


typedef QHash<QSharedPointer<IQStringSetterDelegate>, QSharedPointer<ITranslationRule> > TranslationRules;


Translation translate(const char* context, const char* sourceText, const char* disambiguation = 0, int n = -1);
QString Tr(const Translation& translateArgs);
Translation Plural(const Translation& translateArgs, int n);
void Retranslate(QObject* obj);
void RetranslateAll(QObject* obj);

template <typename Ty, typename MemberFunc, typename T>
QString DoSetTr(Ty* obj, MemberFunc memFunc, const T& trRule)
{
    if (!obj) { return QString(); }

    TranslationRules rules;
    QVariant variant = obj->property(kProperty);
    if (variant.canConvert<TranslationRules>())
    {
        rules = variant.value<TranslationRules>();
    }

    QSharedPointer<IQStringSetterDelegate> func(new QStringSetterDelegateImpl<Ty>(obj, memFunc));

    rules[func] = QSharedPointer<ITranslationRule>(new T(trRule));
    obj->setProperty(kProperty, QVariant::fromValue(rules));

    // update translation for now as well
    QString text = trRule.GetText();
    func->execute(text);

    return text;
}

template <typename Ty, typename MemberFunc>
QString SetTr(Ty* obj, MemberFunc memFunc, const QString& text)
{
    TextTranslationRule trRule(text);
    return DoSetTr(obj, memFunc, trRule);
}

template <typename Ty, typename Ty_ui>
void MakeRetranslatable(Ty* obj, Ty_ui* ui)
{
    TranslationRules rules;
    QVariant variant = obj->property(kProperty);
    if (variant.canConvert<TranslationRules>())
    {
        rules = variant.value<TranslationRules>();
    }

    QSharedPointer<IQStringSetterDelegate> func(new RetranslateUICallerImpl<Ty, Ty_ui>(obj, ui));

    rules[func] = QSharedPointer<ITranslationRule>(new TextTranslationRule());
    obj->setProperty(kProperty, QVariant::fromValue(rules));
}

// this one uses function object
template <typename T>
class TranslationRuleImpl : public ITranslationRule
{
public:
    TranslationRuleImpl(const Translation& translateArgs, const T& func)
        : m_translateArgs(translateArgs), m_func(func) {}
    QString GetText() const { return m_func(Tr(m_translateArgs)); }
private:
    Translation m_translateArgs;
    T m_func;
};


template <typename Ty, typename MemberFunc>
QString SetTr(Ty* obj, MemberFunc memFunc, const Translation& translateArgs, std::function<QString(const QString&)> expand)
{
    TranslationRuleImpl<std::function<QString(const QString&)> > trRule(translateArgs, expand);
    return DoSetTr(obj, memFunc, trRule);
}


template <typename Ty, typename MemberFunc, typename... Args>
QString SetTr(Ty* obj, MemberFunc memFunc, const Translation& translateArgs, const Args&... xs)
{
    TranslationRule trRule(translateArgs, { asString(xs)... });
    return DoSetTr(obj, memFunc, trRule);
}

} // namespace Tr

QString locationString(const QString& fileName);
QString languageString(const Tr::Translation& translation, const QString& locName, QTranslator& translator);

} // namespace utilities

Q_DECLARE_METATYPE(utilities::Tr::TranslationRules)

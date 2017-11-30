#pragma once

#include "utilities/errorcode.h"

#include "downloadtype.h"

#include <QStringList>
#include <QUrl>
#include <QDateTime>

#include <utility>

typedef int ItemID;
const ItemID nullItemID = -1;

class CopyableQObject : public QObject
{
public:
    CopyableQObject() : QObject() {}
    CopyableQObject(const CopyableQObject&) {}
    CopyableQObject& operator =(const CopyableQObject&) { return *this; }
};

#define DECLARE_Q_PROPERTY(prop, read, write) \
    public: \
        prop() const { return read##_; } \
        void write(prop) { read##_ = std::move(read); } \
    private: \
        prop##_ {};

#define DECLARE_Q_PROPERTY_ADAPTOR(params) DECLARE_Q_PROPERTY params

class ItemDC
    : public CopyableQObject
{
    Q_OBJECT
    Q_ENUMS(eSTATUSDC)
public:
    ItemDC()
        : m_ID(nullItemID)
        , m_eStatus(eQUEUED)
        , m_speed(0)
        , m_speedUpload(0)
        , m_iWaitingTime(0)
        , m_errorCode(utilities::ErrorCode::eNOTERROR)
        , m_priority(0)
    {}

    enum eSTATUSDC {
        eQUEUED,
        eDOWNLOADING,
        eCONNECTING,
        eFINISHED,
        ePAUSED,
        eERROR,
        eROOTSTATUS,
        eSEEDING,
        eSTALLED,
        eSTARTING,
        eSTOPPED,
        eUNKNOWN
    };

    bool isValid() const { return m_ID != nullItemID; }
    explicit operator bool() const { return isValid(); }

    inline ItemID getID() const { return m_ID; }
    void setID(ItemID val) { m_ID = val; }

    Q_PROPERTY(eSTATUSDC status READ getStatus WRITE setStatusEx)
    inline eSTATUSDC getStatus() const { return m_eStatus; }
    void setStatus(eSTATUSDC val)
    {
        if (m_eStatus != val)
        {
            m_statusLastChanged = QDateTime::currentDateTimeUtc();
            m_eStatus = val;
        }
    }

    QDateTime statusLastChanged() const { return m_statusLastChanged; }

    float getSpeed() const { return m_speed; }
    void setSpeed(float val) { m_speed = val; }

    float getSpeedUpload() const { return m_speedUpload; }
    void setSpeedUpload(float val) { m_speedUpload = val; }

    int getWaitingTime() const { return m_iWaitingTime; }
    void setWaitingTime(int val) { m_iWaitingTime = val; }

    utilities::ErrorCode::ERROR_CODES getErrorCode() const { return m_errorCode; }
    void setErrorCode(utilities::ErrorCode::ERROR_CODES val) { m_errorCode = val; }

    int priority() const { return m_priority; }

    bool isCompleted() const
    {
        const ItemDC::eSTATUSDC st(getStatus());
        return
            st == ItemDC::eFINISHED ||
            st == ItemDC::eSEEDING  ||
            st == ItemDC::eERROR;
    }

protected:
    void setStatusEx(int val);

    ItemID m_ID;
    eSTATUSDC m_eStatus;
    float m_speed;
    float m_speedUpload;
    int m_iWaitingTime;

    utilities::ErrorCode::ERROR_CODES m_errorCode;

    int m_priority;

    QDateTime m_statusLastChanged;

#ifndef Q_MOC_RUN
#define READ ,
#define WRITE ,
#undef Q_PROPERTY
#define Q_PROPERTY(...) DECLARE_Q_PROPERTY_ADAPTOR((__VA_ARGS__))
#endif // Q_MOC_RUN

    Q_PROPERTY(qint64 sizeCurrDownl READ sizeCurrDownl WRITE setSizeCurrDownl)
    Q_PROPERTY(qint64 size READ size WRITE setSize)

    Q_PROPERTY(QString initialURL READ initialURL WRITE setInitialURL)
    Q_PROPERTY(QString actualURL READ actualURL WRITE setActualURL)
    Q_PROPERTY(QString source READ source WRITE setSource)
    Q_PROPERTY(QString downloadedFileName READ downloadedFileName WRITE setDownloadedFileName)
    Q_PROPERTY(QStringList torrentFilesPriorities READ torrentFilesPriorities WRITE setTorrentFilesPriorities)
    Q_PROPERTY(QString hash READ hash WRITE setHash)
    Q_PROPERTY(QString torrentSavePath READ torrentSavePath WRITE setTorrentSavePath)
    Q_PROPERTY(QString errorDescription READ errorDescription WRITE setErrorDescription)

    Q_PROPERTY(DownloadType::Type downloadType READ downloadType WRITE setDownloadType)

#undef Q_PROPERTY
#define Q_PROPERTY(text)
#undef READ
#undef WRITE

}; // class ItemDC

class TreeItem : public ItemDC
{
    Q_OBJECT
public:
    TreeItem(const QString& a_url = QString(), TreeItem* a_parent = 0);
    virtual ~TreeItem();

    void appendChild(TreeItem* child);
    TreeItem* child(int row);
    int childCount() const;

    int row() const;
    TreeItem* parent() const;
    int lastIndexOf(TreeItem* a_child) const;
    bool removeChildItem(TreeItem* a_item);

    bool insertChildren(int position, int count, int columns);
    bool removeChildren(int position, int count);

    template <typename Pr>
    TreeItem* findItem(Pr pr);

    TreeItem* findItemByID(ItemID);
    TreeItem* findItemByURL(const QString&);

    Q_PROPERTY(QObjectList childItems READ getChildItems WRITE setChildItems)
    QObjectList getChildItems() const;
    void setChildItems(const QObjectList& items);

    template<class Fn_t> void forAll(Fn_t fn);

    void setPriority(int priority) {m_priority = priority;}
    static int currentCounter() { return l_count; }

    bool canPause() const
    {
        const ItemDC::eSTATUSDC st(getStatus());
        return !(st == ItemDC::ePAUSED || st == ItemDC::eSTOPPED || st == ItemDC::eERROR || st == ItemDC::eFINISHED);
    }

    bool canResume() const
    {
        const ItemDC::eSTATUSDC st(getStatus());
        return st == ItemDC::ePAUSED  || st == ItemDC::eSTOPPED || st == ItemDC::eERROR 
            || (st == ItemDC::eFINISHED && DownloadType::isTorrentDownload(downloadType()));
    }

    bool canCancel() const
    {
        const ItemDC::eSTATUSDC st(getStatus());
        return st != ItemDC::eFINISHED && st != ItemDC::eSEEDING;
    }

private:
    static int l_count;

    QList<TreeItem*> childItems;
    TreeItem* parentItem;
}; // class TreeItem


template<class Fn_t>
void TreeItem::forAll(Fn_t fn)
{
    fn(*this);
    for (TreeItem* ti : qAsConst(childItems))
    {
        ti->forAll(fn);
    }
}

template <typename Pr>
TreeItem* TreeItem::findItem(Pr pr)
{
    if (pr(this))
    {
        return this;
    }

    for (TreeItem* ti : qAsConst(childItems))
    {
        if (TreeItem* l_child = ti->findItem(pr))
        {
            return l_child;
        }
    }

    return nullptr;
}


QString itemDCStatusToString(const ItemDC::eSTATUSDC status);

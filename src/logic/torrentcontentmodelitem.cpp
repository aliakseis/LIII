#include <QFileInfo>
#include "torrentcontentmodelitem.h"
#include <QDebug>

#include <algorithm>
#include <iterator>


void TorrentContentModelItem::init(QString name, const qlonglong size)
{
    // Do not display incomplete extensions
    if (name.endsWith(".!qB"))
    {
        name.chop(4);
    }
    m_itemData << name
               << size
               << QVariant()
               << 0.
               << prio::NORMAL;

    /* Update parent */
    m_parentItem->appendChild(this);
}


TorrentContentModelItem::TorrentContentModelItem(
    const libtorrent::file_entry& f,
    TorrentContentModelItem* parent)
    : m_parentItem(parent), m_type(TFILE), m_totalDone(0)
{
    Q_ASSERT(parent);
    m_path = QString::fromStdString(f.path);
    QString name = QFileInfo(m_path).fileName();

    init(name, f.size);

    m_parentItem->updateSize();
}

TorrentContentModelItem::TorrentContentModelItem(const QString& name, TorrentContentModelItem* parent)
    : m_parentItem(parent), m_type(FOLDER), m_totalDone(0)
{
    m_path = name;
    if (parent != nullptr && parent->isFolder())
    {
        m_path = parent->getPath() + '/' + m_path;
    }

    init(name, 0);
}

TorrentContentModelItem::TorrentContentModelItem(const QList<QVariant>& data)
    : m_parentItem(nullptr), m_type(ROOT), m_itemData(data), m_totalDone(0)
{
    Q_ASSERT(data.size() == NB_COL);
}

TorrentContentModelItem::~TorrentContentModelItem()
{
    qDeleteAll(m_childItems);
}

TorrentContentModelItem::FileType TorrentContentModelItem::getType() const
{
    return m_type;
}

void TorrentContentModelItem::deleteAllChildren()
{
    Q_ASSERT(m_type == ROOT);
    qDeleteAll(m_childItems);
    m_childItems.clear();
}

const QList<TorrentContentModelItem*>& TorrentContentModelItem::children() const
{
    return m_childItems;
}

QString TorrentContentModelItem::getPath() const
{
    return m_path;
}

QString TorrentContentModelItem::getName() const
{
    return m_itemData.at(COL_NAME).toString();
}

void TorrentContentModelItem::setName(const QString& name)
{
    Q_ASSERT(m_type != ROOT);
    m_itemData.replace(COL_NAME, name);
}

qulonglong TorrentContentModelItem::getSize() const
{
    return m_itemData.value(COL_SIZE).toULongLong();
}

void TorrentContentModelItem::setSize(qulonglong size)
{
    Q_ASSERT(m_type != ROOT);
    if (getSize() == size)
    {
        return;
    }
    m_itemData.replace(COL_SIZE, (qulonglong)size);
    m_parentItem->updateSize();
}

void TorrentContentModelItem::updateSize()
{
    if (m_type == ROOT)
    {
        return;
    }
    Q_ASSERT(m_type == FOLDER);
    const qulonglong size = std::accumulate(m_childItems.constBegin(), m_childItems.constEnd(), 0ull,
        [](qulonglong sum, TorrentContentModelItem* child)
        {
            return (child->getPriority() != prio::IGNORED) ? sum + child->getSize() : sum;
        });
    setSize(size);
}

void TorrentContentModelItem::setProgress(qulonglong done)
{
    Q_ASSERT(m_type != ROOT);
    if (getPriority() == 0) { return; }
    m_totalDone = done;
    const qulonglong size = getSize();
    Q_ASSERT(m_totalDone <= size);
    const qreal progress = (size > 0) ? m_totalDone / (qreal)size : 1.;
    Q_ASSERT(progress >= 0. && progress <= 1.);
    m_itemData.replace(COL_PROGRESS, progress);
    m_parentItem->updateProgress();

    // STATUS COLUMN
    if ((done == size || progress == 1.) && m_type == TFILE)
    {
        setStatus(ItemDC::eFINISHED);
    }
}

qulonglong TorrentContentModelItem::getTotalDone() const
{
    return m_totalDone;
}

ItemDC::eSTATUSDC TorrentContentModelItem::getStatus() const
{
    QVariant v = m_itemData.value(COL_STATUS, ItemDC::eUNKNOWN);
    return (!v.isValid() || v.isNull())
        ? m_parentItem->getStatus()
        : (ItemDC::eSTATUSDC)v.toInt();
}

void TorrentContentModelItem::setStatus(ItemDC::eSTATUSDC status)
{
    m_itemData.replace(COL_STATUS, status);
}

void TorrentContentModelItem::updateStatus()
{
    ItemDC::eSTATUSDC status = ItemDC::eUNKNOWN;
    if (m_totalDone == getSize())
    {
        status = ItemDC::eFINISHED;
    }
    else if (getPriority() == prio::IGNORED)
    {
        status = ItemDC::ePAUSED;
    }
    else
    {
        status = m_parentItem->getStatus();
    }
    setStatus(status);
}

float TorrentContentModelItem::getProgress() const
{
    Q_ASSERT(m_type != ROOT);
    if (getPriority() == 0)
    {
        return -1;
    }
    qulonglong size = getSize();
    if (size > 0)
    {
        return m_totalDone / (float) getSize();
    }
    return 1.;
}

void TorrentContentModelItem::updateProgress()
{
    if (m_type == ROOT) { return; }
    Q_ASSERT(m_type == FOLDER);
    m_totalDone = std::accumulate(m_childItems.constBegin(), m_childItems.constEnd(), 0ull,
        [](qulonglong sum, TorrentContentModelItem* child)
        {
            return (child->getPriority() != prio::IGNORED) ? sum + child->getTotalDone() : sum;
        });
    Q_ASSERT(m_totalDone <= getSize());
    setProgress(m_totalDone);
}

int TorrentContentModelItem::getPriority() const
{
    return m_itemData.value(COL_PRIO).toInt();
}

void TorrentContentModelItem::setPriority(int new_prio, bool update_parent)
{
    Q_ASSERT(new_prio != prio::PARTIAL || m_type == FOLDER); // PARTIAL only applies to folders
    if (getPriority() == new_prio)
    { 
        return; 
    }
    qDebug("setPriority(%s, %d)", qPrintable(getName()), new_prio);

    m_itemData.replace(COL_PRIO, new_prio);

    // Update parent
    if (update_parent && m_parentItem)
    {
        qDebug("Updating parent item");
        m_parentItem->updateSize();
        m_parentItem->updateProgress();
        m_parentItem->updatePriority();
    }

    // Update children
    if (new_prio != prio::PARTIAL && !m_childItems.empty())
    {
        qDebug("Updating children items");
        for (TorrentContentModelItem* child : qAsConst(m_childItems))
        {
            // Do not update the parent since
            // the parent is causing the update
            child->setPriority(new_prio, false);
        }
    }
    if (m_type == FOLDER)
    {
        updateSize();
        updateProgress();
    }
    else
    {
        updateStatus();
    }
}

// Only non-root folders use this function
void TorrentContentModelItem::updatePriority()
{
    if (m_type == ROOT) { return; }
    Q_ASSERT(m_type == FOLDER);
    if (m_childItems.isEmpty()) { return; }
    // If all children have the same priority
    // then the folder should have the same priority
    const int priority = m_childItems.first()->getPriority();
    const bool same_priorities = std::all_of(std::next(m_childItems.begin()), m_childItems.end(),
        [priority](auto item) { return item->getPriority() == priority; });
    setPriority(same_priorities? priority : prio::PARTIAL);
}

TorrentContentModelItem* TorrentContentModelItem::childWithName(const QString& name) const
{
    for (TorrentContentModelItem* child : qAsConst(m_childItems))
    {
        if (child->getName() == name)
        {
            return child;
        }
    }
    return 0;
}

bool TorrentContentModelItem::isFolder() const
{
    return (m_type == FOLDER);
}

void TorrentContentModelItem::appendChild(TorrentContentModelItem* item)
{
    Q_ASSERT(item);
    Q_ASSERT(m_type != TFILE);
    m_childItems.append(item);
}

TorrentContentModelItem* TorrentContentModelItem::child(int row) const
{
    return m_childItems.value(row, 0);
}

int TorrentContentModelItem::childCount() const
{
    return m_childItems.count();
}

int TorrentContentModelItem::columnCount() const
{
    return m_itemData.count();
}

QVariant TorrentContentModelItem::data(int column) const
{
    if (m_type != ROOT)
    {
        if (column == COL_PROGRESS)
        {
            return getProgress();
        }
        if (column == COL_STATUS)
        {
            return itemDCStatusToString(getStatus());
        }
    }

    return m_itemData.value(column);
}

int TorrentContentModelItem::row() const
{
    return m_parentItem ? m_parentItem->children().indexOf(const_cast<TorrentContentModelItem*>(this)) : 0;
}

TorrentContentModelItem* TorrentContentModelItem::parent() const
{
    return m_parentItem;
}

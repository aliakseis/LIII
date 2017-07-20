#include <QFileInfo>
#include "torrentcontentmodelitem.h"
#include <QDebug>


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
    TorrentContentModelItem* parent,
    int file_index):
    m_parentItem(parent), m_type(TFILE), m_fileIndex(file_index), m_totalDone(0)
{
    Q_ASSERT(parent);
    m_path = QString::fromUtf8(f.path.c_str());
    QString name = QFileInfo(m_path).fileName();

    init(name, f.size);

    m_parentItem->updateSize();
}

TorrentContentModelItem::TorrentContentModelItem(const QString& name, TorrentContentModelItem* parent):
    m_parentItem(parent), m_type(FOLDER), m_totalDone(0)
{
    init(name, 0);
}

TorrentContentModelItem::TorrentContentModelItem(const QList<QVariant>& data):
    m_parentItem(0), m_type(ROOT), m_itemData(data), m_fileIndex(0), m_totalDone(0)
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

int TorrentContentModelItem::getFileIndex() const
{
    Q_ASSERT(m_type == TFILE);
    return m_fileIndex;
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
    qulonglong size = 0;
    Q_FOREACH(TorrentContentModelItem * child, m_childItems)
    {
        if (child->getPriority() != prio::IGNORED)
        {
            size += child->getSize();
        }
    }
    setSize(size);
}

void TorrentContentModelItem::setProgress(qulonglong done)
{
    Q_ASSERT(m_type != ROOT);
    if (getPriority() == 0) { return; }
    m_totalDone = done;
    qulonglong size = getSize();
    Q_ASSERT(m_totalDone <= size);
    qreal progress;
    if (size > 0)
    {
        progress = m_totalDone / (float)size;
    }
    else
    {
        progress = 1.;
    }
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

ItemDC::eSTATUSDC TorrentContentModelItem::getStatus()const
{
    QVariant v = m_itemData.value(COL_STATUS, ItemDC::eUNKNOWN);
    ItemDC::eSTATUSDC result = ItemDC::eUNKNOWN;
    if (!v.isValid() || v.isNull())
    {
        result = m_parentItem->getStatus();
    }
    else
    {
        result = (ItemDC::eSTATUSDC)v.toInt();
    }
    return result;
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
    m_totalDone = 0;
    Q_FOREACH(TorrentContentModelItem * child, m_childItems)
    {
        if (child->getPriority() > 0)
        {
            m_totalDone += child->getTotalDone();
        }
    }
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
    const int old_prio = getPriority();
    if (old_prio == new_prio) { return; }
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
        Q_FOREACH(TorrentContentModelItem * child, m_childItems)
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
    const int prio = m_childItems.first()->getPriority();
    for (int i = 1; i < m_childItems.size(); ++i)
    {
        if (m_childItems.at(i)->getPriority() != prio)
        {
            setPriority(prio::PARTIAL);
            return;
        }
    }
    // All child items have the same priority
    // Update own if necessary
    if (prio != getPriority())
    {
        setPriority(prio);
    }
}

TorrentContentModelItem* TorrentContentModelItem::childWithName(const QString& name) const
{
    Q_FOREACH(TorrentContentModelItem * child, m_childItems)
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
    int i = 0;
    for (; i < m_childItems.size(); ++i)
    {
        QString newchild_name = item->getName();
        if (QString::localeAwareCompare(newchild_name, m_childItems.at(i)->getName()) < 0)
        {
            break;
        }
    }
    m_childItems.insert(i, item);
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
        if (column == COL_PROGRESS)
        {
            return getProgress();
        }
        else if (column == COL_STATUS)
        {
            return itemDCStatusToString(getStatus());
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

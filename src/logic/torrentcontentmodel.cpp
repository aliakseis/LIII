#include "torrentcontentmodel.h"
#include "torrentcontentmodelitem.h"
#include "torrentslistener.h"

#include <QDir>

TorrentContentModel::TorrentContentModel(QObject* parent):
    QAbstractItemModel(parent),
    m_rootItem(
        new TorrentContentModelItem(
            QList<QVariant>() << tr("Name") << tr("Size") << tr("Status") << tr("Progress") << tr("Priority")))
{
}

TorrentContentModel::~TorrentContentModel()
{
    delete m_rootItem;
}

void TorrentContentModel::updateFilesProgress(const std::vector<boost::int64_t>& fp)
{
    emit layoutAboutToBeChanged();
    Q_ASSERT(m_filesIndex.size() == (int)fp.size());
    if (m_filesIndex.size() != (int)fp.size()) { return; }
    for (uint i = 0; i < fp.size(); ++i)
    {
        m_filesIndex[i]->setProgress(fp[i]);
    }
    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

bool TorrentContentModel::allFiltered() const
{
    for (int i = 0; i < m_rootItem->childCount(); ++i)
    {
        if (m_rootItem->child(i)->getPriority() != prio::IGNORED)
        {
            return false;
        }
    }
    return true;
}

int TorrentContentModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return static_cast<TorrentContentModelItem*>(parent.internalPointer())->columnCount();
    }
    return m_rootItem->columnCount();
}

bool TorrentContentModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
    {
        return false;
    }

    if (index.column() == 0 && role == Qt::CheckStateRole)
    {
        TorrentContentModelItem* item = static_cast<TorrentContentModelItem*>(index.internalPointer());
        qDebug("setData(%s, %d", qPrintable(item->getName()), value.toInt());
        if (item->getPriority() != value.toInt())
        {
            if (value.toInt() == Qt::PartiallyChecked)
            {
                item->setPriority(prio::PARTIAL);
            }
            else if (value.toInt() == Qt::Unchecked)
            {
                item->setPriority(prio::IGNORED);
            }
            else
            {
                item->setPriority(prio::NORMAL);
            }
            emit dataChanged(this->index(0, 0), this->index(rowCount() - 1, columnCount() - 1));
            emit filteredFilesChanged();
        }
        return true;
    }


    if (role == Qt::EditRole)
    {
        TorrentContentModelItem* item = static_cast<TorrentContentModelItem*>(index.internalPointer());
        switch (index.column())
        {
        case TorrentContentModelItem::COL_NAME:
            item->setName(value.toString());
            break;
        case TorrentContentModelItem::COL_SIZE:
            item->setSize(value.toULongLong());
            break;
        case TorrentContentModelItem::COL_STATUS:
            item->setStatus(ItemDC::eSTATUSDC(value.toInt()));
            break;
        case TorrentContentModelItem::COL_PROGRESS:
            item->setProgress(value.toDouble());
            break;
        case TorrentContentModelItem::COL_PRIO:
        {
            item->setPriority(value.toInt());
            emit filteredFilesChanged();
        }
        break;
        default:
            return false;
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

TorrentContentModelItem::FileType TorrentContentModel::getType(const QModelIndex& index) const
{
    const TorrentContentModelItem* item = static_cast<const TorrentContentModelItem*>(index.internalPointer());
    return item->getType();
}

int TorrentContentModel::getFileIndex(const QModelIndex& index)
{
    TorrentContentModelItem* item = static_cast<TorrentContentModelItem*>(index.internalPointer());
    return item->getFileIndex();
}

TorrentContentModelItem* TorrentContentModel::getTorrentContentModelItem(const QModelIndex& index)const
{
    TorrentContentModelItem* item = static_cast<TorrentContentModelItem*>(index.internalPointer());
    return item;
}

QVariant TorrentContentModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    TorrentContentModelItem* item = static_cast<TorrentContentModelItem*>(index.internalPointer());
    if (index.column() == 0 && role == Qt::DecorationRole)
    {
        if (item->isFolder())
        {
            return m_iconProvider.icon(QFileIconProvider::Folder);
        }

        QString path = m_savePath + item->getPath();
        if ((m_savePath.isEmpty() || !QFile::exists(path)))
        {
            return m_iconProvider.icon(QFileIconProvider::File);
        }
        else
        {
            return m_iconProvider.icon(QFileInfo(path));
        }
    }

    if (index.column() == 0 && role == Qt::CheckStateRole)
    {
        switch (item->data(TorrentContentModelItem::COL_PRIO).toInt())
        {
        case prio::IGNORED:
            return Qt::Unchecked;
        case prio::PARTIAL:
            return Qt::PartiallyChecked;
        default:
            return Qt::Checked;
        };
    }

    if (role == Qt::DisplayRole)
    {
        return item->data(index.column());
    }

    return QVariant();
}

Qt::ItemFlags TorrentContentModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return 0;
    }

    if (getType(index) == TorrentContentModelItem::FOLDER)
    {
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsTristate;
    }
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

QVariant TorrentContentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return m_rootItem->data(section);
    }

    return QVariant();
}

QModelIndex TorrentContentModel::index(int row, int column, const QModelIndex& parent) const
{
    if ((parent.isValid() && parent.column() != 0) || column >= TorrentContentModelItem::NB_COL)
    {
        return QModelIndex();
    }

    TorrentContentModelItem* parentItem = parent.isValid() ? static_cast<TorrentContentModelItem*>(parent.internalPointer()) : m_rootItem;

    Q_ASSERT(parentItem);
    if (row >= parentItem->childCount())
    {
        return QModelIndex();
    }

    if (TorrentContentModelItem* childItem = parentItem->child(row))
    {
        return createIndex(row, column, childItem);
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex TorrentContentModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    TorrentContentModelItem* childItem = static_cast<TorrentContentModelItem*>(index.internalPointer());
    if (!childItem)
    {
        return QModelIndex();
    }
    TorrentContentModelItem* parentItem = childItem->parent();

    if (parentItem == m_rootItem)
    {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int TorrentContentModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }

    TorrentContentModelItem* parentItem = parent.isValid() ? static_cast<TorrentContentModelItem*>(parent.internalPointer()) : m_rootItem;

    return parentItem->childCount();
}

void TorrentContentModel::clear()
{
    qDebug("clear called");
    beginResetModel();
    m_filesIndex.clear();
    m_rootItem->deleteAllChildren();
    endResetModel();
}

void TorrentContentModel::setupModelData(const libtorrent::torrent_info& t, const libtorrent::torrent_status& tStatus)
{
    qDebug("setup model data called");
    if (t.num_files() == 0)
    {
        return;
    }

    emit layoutAboutToBeChanged();
    // Initialize files_index array
    qDebug("Torrent contains %d files", t.num_files());
    m_filesIndex.reserve(t.num_files());

    // Iterate over files
    for (int i = 0; i < t.num_files(); ++i)
    {
        const libtorrent::file_entry& fentry = t.file_at(i);
        TorrentContentModelItem* current_parent = m_rootItem;

        QString path = QString::fromStdString(fentry.path);

        // Iterate of parts of the path to create necessary folders
        QStringList pathFolders = path.split(QRegExp("[/\\\\]"), QString::SkipEmptyParts);
        pathFolders.removeLast();
        Q_FOREACH(const QString & pathPart, pathFolders)
        {
            if (pathPart == ".unwanted")
            {
                continue;
            }
            TorrentContentModelItem* new_parent = current_parent->childWithName(pathPart);
            if (!new_parent)
            {
                new_parent = new TorrentContentModelItem(pathPart, current_parent);
            }
            current_parent = new_parent;
        }
        // Actually create the file
        if (current_parent != m_rootItem)
        {
            current_parent->setStatus(torrentStatus2ItemDCStatus(tStatus.state));
        }
        m_filesIndex.push_back(new TorrentContentModelItem(fentry, current_parent, i));
    }
    emit layoutChanged();
}

void TorrentContentModel::selectAll()
{
    for (int i = 0; i < m_rootItem->childCount(); ++i)
    {
        TorrentContentModelItem* child = m_rootItem->child(i);
        if (child->getPriority() == prio::IGNORED)
        {
            child->setPriority(prio::NORMAL);
        }
    }
    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

void TorrentContentModel::selectNone()
{
    for (int i = 0; i < m_rootItem->childCount(); ++i)
    {
        m_rootItem->child(i)->setPriority(prio::IGNORED);
    }
    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

void TorrentContentModel::setSavePath(const QString& savePath)
{
    m_savePath = savePath;
}

QString TorrentContentModel::getSavePath()
{
    return m_savePath;
}

#include "downloadcollectionmodel.h"

#include <string>
#include <QFile>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMimeData>
#include <QMessageBox>
#include <QDebug>

#include "utilities/utils.h"
#include "utilities/translation.h"

#include "global_functions.h"
#include "globals.h"
#include "utilities/errorcode.h"

#include "utilities/customutf8codec.h"
#include "utilities/filesaveguard.h"
#include "branding.hxx"

#include "torrentslistener.h"
#include "torrentmanager.h"
#include "addtorrentform.h"
#include "treeitem.h"

namespace Tr = utilities::Tr;

DownloadCollectionModel::DownloadCollectionModel()
    : rootItem(new TreeItem())
    , isDropAction(false)
{
    loadFromFile();

    rootItem->setStatus(ItemDC::eROOTSTATUS);

    VERIFY(qRegisterMetaType<ItemDC>("ItemDC"));
    VERIFY(connect(&TorrentsListener::instance(), SIGNAL(statusChange(ItemDC)), SLOT(on_statusChange(ItemDC))));
    VERIFY(connect(&TorrentsListener::instance(), SIGNAL(sizeChange(ItemDC)), SLOT(on_sizeChange(ItemDC))));
    VERIFY(connect(&TorrentsListener::instance(), SIGNAL(sizeCurrDownlChange(ItemDC)), SLOT(on_sizeCurrDownlChange(ItemDC))));
    VERIFY(connect(&TorrentsListener::instance(), SIGNAL(speedChange(ItemDC)), SLOT(on_speedChange(ItemDC))));
    VERIFY(connect(&TorrentsListener::instance(), SIGNAL(itemMetadataReceived(ItemDC)), SLOT(on_magnetLinkInfoReceived(ItemDC))));
    VERIFY(connect(&TorrentsListener::instance(), SIGNAL(torrentMoved(ItemDC)), SLOT(on_torrentMoved(ItemDC))));
}

DownloadCollectionModel::~DownloadCollectionModel()
{
    delete rootItem;
}

TreeItem* DownloadCollectionModel::getItem(const QModelIndex& index) const
{
    if (index.isValid())
    {
        if (TreeItem* item = static_cast<TreeItem*>(index.internalPointer())) 
        { 
            return item; 
        }
    }
    return rootItem;
}


QModelIndex DownloadCollectionModel::index(int row, int column, const QModelIndex& parent /* = QModelIndex() */) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    TreeItem* parentItem = parent.isValid() ? static_cast<TreeItem*>(parent.internalPointer()) : rootItem;

    if (TreeItem* childItem = parentItem->child(row))
    {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex DownloadCollectionModel::index(TreeItem* item, int column) const
{
    if (!item || (item == getRootItem()))
    {
        return QModelIndex();
    }

    TreeItem* parentItem = item->parent();

    if (!parentItem)
    {
        parentItem = getRootItem();
    }
    int row = parentItem->lastIndexOf(item);

    return createIndex(row, column, item);
}

QModelIndex DownloadCollectionModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem* parentItem = childItem->parent();

    if (parentItem == rootItem)
    {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int DownloadCollectionModel::columnCount(const QModelIndex& parent) const
{
    return eDC_columnsCount;
}

int DownloadCollectionModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }

    TreeItem* parentItem = parent.isValid()? static_cast<TreeItem*>(parent.internalPointer()) : rootItem;

    return parentItem->childCount();
}

bool DownloadCollectionModel::insertRows(int position, int rows, const QModelIndex& parent  /* = QModelIndex()*/)
{
    TreeItem* parentItem = getItem(parent);

    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChildren(position, rows, columnCount());
    endInsertRows();

    return success;
}

bool DownloadCollectionModel::removeRows(int position, int rows, const QModelIndex& parent /* = QModelIndex()*/)
{
    TreeItem* parentItem = getItem(parent);

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

Qt::ItemFlags DownloadCollectionModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return  Qt::ItemIsDropEnabled;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

Qt::DropActions DownloadCollectionModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QVariant DownloadCollectionModel::data(const QModelIndex& index, int role /* = Qt::DisplayRole*/) const
{
    if (!index.isValid())
    {
        return {};
    }

    if (role == Qt::TextAlignmentRole)
    {
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    }

    const int l_colunm = index.column();

    if (role == Qt::ToolTipRole)
    {
        if (l_colunm == eDC_Status)
        {
            TreeItem* l_item = static_cast<TreeItem*>(index.internalPointer());
            if (l_item && l_item->getStatus() == ItemDC::eERROR)
            {
                QString errorDescr = l_item->errorDescription();
                if (!DownloadType::isTorrentDownload(l_item->downloadType()))
                {
                    QString tooltipText = ::Tr::Tr(
                        utilities::ErrorCode::instance().getDescription(l_item->getErrorCode()));
                    if (!errorDescr.isEmpty())
                    {
                        tooltipText += '\n' + errorDescr;
                    }
                    return tooltipText;
                }
                return (!errorDescr.isEmpty() ? tr("Error: ") + errorDescr : QString());
            }
            return {};
        }
        return {};
    }

    if (role != Qt::DisplayRole)
    {
        return {};
    }

    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (!item)
    {
        return {};
    }

    switch (l_colunm)
    {
    case eDC_ID:
        return item->getID();
    case eDC_url:
        {
            QString title = item->downloadedFileName();
            return utilities::GetFileName(title.isEmpty() ? item->initialURL() : title);
        }
#ifdef DEVELOPER_FEATURES
    case eDC_priority:
        return item->priority();
#endif
    case eDC_Status:
        if (item->getStatus() != ItemDC::eERROR)
        {
            return statusName(item);
        }
        else
        {
            const QString errorDescr = item->errorDescription();
            return (!errorDescr.isEmpty() ? tr("Error: ") + errorDescr : statusName(item));
        }
    case eDC_Speed:
    case eDC_Speed_Uploading:
        {
            const float speed = (l_colunm == eDC_Speed) ? item->getSpeed() : item->getSpeedUpload();
            return (speed <= std::numeric_limits<float>::epsilon())
                ? QString()
                : QString("%1 KB/s").arg(speed, 0, 'f', 1);
        }
    case eDC_Size:
        return (item->size() > 0) 
            ? QString("%1 / %2").arg(utilities::SizeToString(item->sizeCurrDownl()), utilities::SizeToString(item->size()))
            : ::Tr::Tr(TREEVIEW_UNKNOWN_SIZE);
    case eDC_Source:
        return item->source();
    case eDC_downlFileName:
        return item->downloadedFileName();
    case eDC_percentDownl:
        return (item->size() > 0) ? (item->sizeCurrDownl() * 100.) / item->size() : 0.;
    default:
        return {};
    }
}

bool DownloadCollectionModel::setData(const QModelIndex& index, const QVariant& value, int role /* = Qt::EditRole*/)
{
    if (role != Qt::DisplayRole)
    {
        return false;
    }

    TreeItem* item = getItem(index);

    if (index.column() == eDC_ID)
    {
        const auto id = value.value<ItemID>();
        if (isDropAction)   // TODO: remove?
        {
            if (TreeItem* source = getRootItem()->findItemByID(id))
            {
                *item = *source;
            }
        }
        item->setID(id);
    }

    item->setPriority(index.row());
    emit dataChanged(index, index);
    return true;
}


QVariant DownloadCollectionModel::headerData(int section, Qt::Orientation orientation, int role /* = Qt::DisplayRole*/) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case eDC_ID:
            return QString("ID");
        case eDC_url:
            return ::Tr::Tr(TREEVIEW_TITLE_HEADER);
#ifdef DEVELOPER_FEATURES
        case eDC_priority:
            return "#";
#endif
        case eDC_Status:
            return ::Tr::Tr(TREEVIEW_STATUS_HEADER);
        case eDC_Speed:
            return ::Tr::Tr(TREEVIEW_SPEED_HEADER);
        case eDC_Speed_Uploading:
            return ::Tr::Tr(TREEVIEW_SPEED_UPLOAD_HEADER);
        case eDC_percentDownl:
            return ::Tr::Tr(TREEVIEW_PROGR_HEADER);
        case eDC_Size:
            return ::Tr::Tr(TREEVIEW_SIZE_HEADER);
        case eDC_Source:
            return ::Tr::Tr(TREEVIEW_SOURCE_HEADER);
        case eDC_downlFileName:
            return QString("Download fileName");
        }
    }
    return QVariant();
}

void DownloadCollectionModel::addItemsToModel(const QStringList& urls, DownloadType::Type a_type)
{
    for (const QString& l_strUrl : urls)
    {
        libtorrent::torrent_handle handle;
        
        DownloadType::Type type 
            = (DownloadType::Unknown == a_type)? DownloadType::determineType(l_strUrl) : a_type;
        if (DownloadType::isDirectDownload(type))
        {
            if (TreeItem* item = findItemByURL(l_strUrl))
            {
                if (1 == urls.size())
                {
                    emit existingItemAdded(index(item, 0));
                }
                continue;
            }
        }
        else if (DownloadType::isTorrentDownload(type))
        {
            handle = TorrentManager::Instance()->addTorrent(l_strUrl, TreeItem::currentCounter() + 1, true);
            if (!handle.is_valid())
            {
                if (1 == urls.size())
                {
                    if (TreeItem* item = findItemByURL(l_strUrl))
                    {
                        emit existingItemAdded(index(item, 0));
                    }
                }
                continue;
            }
        }
        else
        {
            Q_ASSERT_X(false, Q_FUNC_INFO, "Cannot be unknown");
        }

        beginInsertRows(QModelIndex(), getRootItem()->childCount(), getRootItem()->childCount() + 1);
        TreeItem* ti = new TreeItem(l_strUrl, getRootItem());
        ti->setDownloadType(type);
        if (DownloadType::isTorrentDownload(type))
        {
            ti->setSource("Torrent");
            std::string hash(handle.info_hash().to_string());
            ti->setHash(QByteArray(hash.data(), hash.size()).toBase64());
            ti->setTorrentSavePath(QString::fromStdString(handle.save_path()));
            try
            {
                libtorrent::torrent_status status = handle.status(0x0);
                if (status.has_metadata)
                {
                    ti->setSize(handle.status(0).total_wanted);
                    ti->setDownloadType(DownloadType::TorrentFile);
                    ti->setDownloadedFileName(QString::fromStdString(handle.get_torrent_info().name()));
                }
            }
            catch (libtorrent::libtorrent_exception const& e)
            {
                qDebug() << Q_FUNC_INFO << " caught exception: " << e.what();
            }
        }
        getRootItem()->appendChild(ti);

        endInsertRows();

        emit signalUrlAdded(QUrl(l_strUrl), type);

    }
    emit signalModelUpdated();
}

bool DownloadCollectionModel::deleteURLFromModel(ItemID a_ID, int deleteWithFiles)
{
    TreeItem* item = getRootItem()->findItemByID(a_ID);
    if (!item)
    {
        return false;
    }

    const auto type = item->downloadType();

    TreeItem* parentTI = item->parent();
    if (!parentTI)
    {
        return false;
    }

    int rowNum = parentTI->lastIndexOf(item);
    if (rowNum < 0)
    {
        return false;
    }

    beginRemoveRows(index(parentTI, 0), rowNum, rowNum);
    parentTI->removeChildItem(item);
    endRemoveRows();

    emit signalDeleteURLFromModel(a_ID, type, deleteWithFiles);
    onModelUpdated();
    return true;
}


void DownloadCollectionModel::deleteALLFinished()
{
    for (int i = getRootItem()->childCount(); --i >= 0; )
    {
        TreeItem* item = getRootItem()->child(i);
        if (item->isCompleted())
        {
            int l_ID = item->getID();
            deleteURLFromModel(l_ID);
        }
    }

    queueSaveToFile();
}


void DownloadCollectionModel::on_statusChange(const ItemDC& a_item)
{
    TreeItem* item = getRootItem()->findItemByID(a_item.getID());
    if (!item)
    {
        return;
    }

    const ItemDC::eSTATUSDC prevStatus(item->getStatus());

    if (prevStatus == a_item.getStatus())
    {
        return;
    }

    if (prevStatus == ItemDC::eSTARTING && a_item.getStatus() == ItemDC::eSTALLED)
    {
        return;
    }

    ItemDC l_itm;
    l_itm.setID(a_item.getID());
    if (DownloadType::isTorrentDownload(item->downloadType()))
    {
        if (prevStatus == ItemDC::eSTOPPED && a_item.getStatus() != ItemDC::eQUEUED)
        {
            return;    // ignore such status change
        }
        if (prevStatus == ItemDC::eQUEUED
                && (a_item.getStatus() == ItemDC::ePAUSED 
                    || a_item.getStatus() == ItemDC::eDOWNLOADING || a_item.getStatus() == ItemDC::eSTALLED))
        {
            return;    // ignore such status change
        }
        if (prevStatus == ItemDC::eERROR)
        {
            return;   // ignore any other states because of error
        }
        if (a_item.getStatus() == ItemDC::eERROR && !a_item.errorDescription().isEmpty())
        {
            item->setErrorDescription(a_item.errorDescription()); // We interested only in first error
        }
    }

    item->setStatus(a_item.getStatus());

    if (a_item.getStatus() == ItemDC::eDOWNLOADING)
    {
        emit onDownloadStarted();
    }
    emit statusChanged();
    emit dataChanged(index(item, eDC_url), index(item, eDC_Status));

    if (prevStatus == ItemDC::eDOWNLOADING && a_item.isCompleted())
    {
        emit downloadingFinished(*item);
    }
    calculateAllProgress();
    activeDownloadsChanged();
}

void DownloadCollectionModel::on_speedChange(const ItemDC& a_item)
{
    if (TreeItem* item = getRootItem()->findItemByID(a_item.getID()))
    {
        item->setSpeed(a_item.getSpeed());
        emit dataChanged(index(item, eDC_Speed), index(item, eDC_Speed));
        item->setSpeedUpload(a_item.getSpeedUpload());
        emit dataChanged(index(item, eDC_Speed_Uploading), index(item, eDC_Speed_Uploading));
    }
}

void DownloadCollectionModel::on_sizeChange(const ItemDC& a_item)
{
    if (TreeItem* item = getRootItem()->findItemByID(a_item.getID()))
    {
        item->setSize(a_item.size());
        emit dataChanged(index(item, eDC_Size), index(item, eDC_Size));
    }
}

void DownloadCollectionModel::on_downloadedFileNameChange(const ItemDC& a_item)
{
    TreeItem* item = getRootItem()->findItemByID(a_item.getID());
    if (!item)
    {
        return;
    }

    const QString downloadedFileName = a_item.downloadedFileName();
    if (QString::compare(downloadedFileName, item->downloadedFileName()
#ifdef Q_OS_WIN
                         , Qt::CaseInsensitive
#endif
                        ) != 0)
    {
        item->setDownloadedFileName(downloadedFileName);
        emit dataChanged(index(item, eDC_downlFileName), index(item, eDC_downlFileName));
    }

    queueSaveToFile();
}

void DownloadCollectionModel::on_sizeCurrDownlChange(const ItemDC& a_item)
{
    TreeItem* item = getRootItem()->findItemByID(a_item.getID());
    if (!item)
    {
        return;
    }

    const float l_fSize = (a_item.size() > 0) ? a_item.size() : item->size();
    const float l_fSizeCurr = a_item.sizeCurrDownl();

    item->setSizeCurrDownl(a_item.sizeCurrDownl());

    if (l_fSize > 0)
    {
        if (l_fSize < l_fSizeCurr)
        {
            item->setSize(l_fSizeCurr);
        }
        else
        {
            emit dataChanged(index(item, eDC_percentDownl), index(item, eDC_percentDownl));
            calculateAllProgress();
        }
    }

    emit dataChanged(index(item, eDC_Size), index(item, eDC_Size));
}

void DownloadCollectionModel::calculateAllProgress()
{
    unsigned long long total = 0;
    unsigned long long downloaded = 0;
    rootItem->forAll(
        [&total, &downloaded](TreeItem & item)
    {
        if (item.getStatus() == ItemDC::eDOWNLOADING)
        {
            total += item.size();
            downloaded += item.sizeCurrDownl();
        }
    });
    emit overallProgress(total ? (downloaded * 100) / total : 100);
}

void DownloadCollectionModel::activeDownloadsChanged()
{
    int activeDownloadsNumber = 0;
    rootItem->forAll(
        [&activeDownloadsNumber](TreeItem & item)
    {
        if (item.getStatus() == ItemDC::eDOWNLOADING)
        {
            activeDownloadsNumber++;
        }
    });
    emit activeDownloadsNumberChanged(activeDownloadsNumber);
}


void DownloadCollectionModel::on_waitingTimeChange(const ItemDC& a_item)
{
    if (TreeItem* item = getRootItem()->findItemByID(a_item.getID()))
    {
        item->setWaitingTime(a_item.getWaitingTime());
        emit dataChanged(index(item, eDC_Status), index(item, eDC_Status));
    }
}


void DownloadCollectionModel::on_ItemDCchange(const ItemDC& a_item)
{
    if (DownloadType::TorrentFile == DownloadType::determineType(a_item.downloadedFileName()))
    {
        deleteURLFromModel(a_item.getID());
        return;
    }

    TreeItem* item = getRootItem()->findItemByID(a_item.getID());
    if (!item)
    {
        return;
    }

    item->setSpeed(a_item.getSpeed());
    item->setSize(a_item.size());
    item->setDownloadedFileName(a_item.downloadedFileName());
    item->setSizeCurrDownl(a_item.sizeCurrDownl());
    item->setWaitingTime(a_item.getWaitingTime());
    item->setErrorCode(a_item.getErrorCode());
    item->setErrorDescription(a_item.errorDescription());

    on_actualURLChange(a_item);
    on_statusChange(a_item);

    emit dataChanged(index(item, eDC_url), index(item, eDC_Source));
}

void DownloadCollectionModel::on_magnetLinkInfoReceived(const ItemDC& a_item)
{
    TreeItem* item = getRootItem()->findItemByID(a_item.getID());
    if (!item)
    {
        return;
    }

    item->setSize(a_item.size());
    item->setDownloadedFileName(a_item.downloadedFileName());
    item->setSource(a_item.source());
    item->setDownloadType(DownloadType::TorrentFile);

    emit dataChanged(index(item, eDC_ID), index(item, eDC_Source));

    queueSaveToFile();
}

ItemDC DownloadCollectionModel::getItemByID(ItemID a_item)
{
    if (TreeItem* itm = getRootItem()->findItemByID(a_item))
    {
        return *itm;
    }
    return {};
}

QModelIndex DownloadCollectionModel::moveItem(const QModelIndex& a_index, int numSteps)
{
    if (!a_index.isValid())
    {
        return QModelIndex();
    }

    const auto i_id = getItem(a_index)->getID();
    int old_row = a_index.row();
    int new_row = qBound(0, old_row + numSteps, getRootItem()->childCount() - 1);
    if (old_row == new_row)
    {
        return a_index;
    }

    if (numSteps > 0)
    {
        new_row += numSteps;
    }
    else
    {
        old_row -= numSteps;
    }

    insertRows(new_row, 1, a_index.parent());
    copyItemInRow(a_index, new_row);
    removeRows(old_row, 1, a_index.parent());

    TreeItem* itm = getRootItem()->findItemByID(i_id);
    return index(itm, 0);
}

template <bool isUp> struct rowPred
{
    bool operator()(const QModelIndex& a, const QModelIndex& b) {return a.row() < b.row();}
};
template <> struct rowPred<false>
{
    bool operator()(const QModelIndex& a, const QModelIndex& b)
    {
        return a.row() > b.row();
    }
};
inline bool isRowEqual(const QModelIndex& x, const QModelIndex& y) {return x.row() == y.row();}

template<bool isUp>
QModelIndexList DownloadCollectionModel::moveItems_helper(QModelIndexList&& selectedInds, int step)
{
    QModelIndexList result;
    result.reserve(selectedInds.size());
    /// sort rows to move according moving direction: if moving down, the last row should be moved first
    qSort(selectedInds.begin(), selectedInds.end(), rowPred<isUp>());

    /// move items and find their new indices (in the same order they was sorted, so new indices are also sorted)
    int minInd = 0;
    int maxInd = getRootItem()->childCount() - 1;
    for (const auto& ind : qAsConst(selectedInds))
    {
        int preNewInd = ind.row() + step;
        if (isUp && preNewInd < minInd)   //==isUp
        {
            preNewInd = minInd;
            ++minInd;
        }
        else if (! isUp && preNewInd > maxInd)
        {
            preNewInd = maxInd;
            --maxInd;
        }

        result.push_back(moveItem(ind, preNewInd - ind.row()));
    }

    // find all rows that change their position and need to update priority value
    QModelIndexList touchedRows;
    touchedRows.reserve(selectedInds.size() * 2);
    // merge old and new indices and exclude duplicated
    std::merge(selectedInds.constBegin(), selectedInds.constEnd(), 
        result.constBegin(), result.constEnd(), 
        std::back_inserter(touchedRows), rowPred<isUp>());
    touchedRows.erase(
        std::unique(touchedRows.begin(), touchedRows.end(), isRowEqual),
        touchedRows.end()
    );
    // update priorities
    for (const auto& x : qAsConst(touchedRows))
        rootItem->child(x.row())->setPriority(x.row());

    if (!isUp)
    {
        std::reverse(result.begin(), result.end());
    }

    emit signalModelUpdated();

    return result;
}


QModelIndexList DownloadCollectionModel::moveItems(QModelIndexList&& selectedInds, int step)
{
    if (step == 0 || selectedInds.size() == 0)
    {
        return selectedInds;
    }

    QModelIndexList result;

    if (step < 0)
    {
        result = moveItems_helper<true >(std::move(selectedInds), step);
    }
    else
    {
        result = moveItems_helper<false>(std::move(selectedInds), step);
    }

    emit onItemsReordered();

    return result;
}

void DownloadCollectionModel::copyItemInRow(const QModelIndex& indexFrom, int rowTo)
{
    TreeItem* itmSource = getItem(indexFrom);
    QModelIndex intexTO = index(rowTo, 0, indexFrom.parent());

    TreeItem* itmTO = getItem(intexTO);
    if (itmTO == getRootItem())
    {
        return;
    }

    itmTO->ItemDC::operator=(*itmSource);

    emit dataChanged(index(itmTO, eDC_ID), index(itmTO, eDC_Source));
}

ItemDC::eSTATUSDC DownloadCollectionModel::getItemStatus(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return ItemDC::eROOTSTATUS;
    }

    TreeItem* itmSource = getItem(index);

    return itmSource->getStatus();

}

void DownloadCollectionModel::setPauseDownloadItem(const QModelIndex& a_index)
{
    if (!a_index.isValid())
    {
        return;
    }

    if (TreeItem* itmSource = getItem(a_index))
    {
        setPauseDownloadItem(itmSource);
    }
}

void DownloadCollectionModel::setPauseDownloadItem(TreeItem* itmSource)
{
    doSetPauseStopDownloadItem(itmSource, ItemDC::ePAUSED);
}

void DownloadCollectionModel::setStopDownloadItem(const QModelIndex& a_index)
{
    if (!a_index.isValid())
    {
        return;
    }

    if (TreeItem* itmSource = getItem(a_index))
    {
        setStopDownloadItem(itmSource);
    }
}

void DownloadCollectionModel::setStopDownloadItem(TreeItem* itmSource)
{
    doSetPauseStopDownloadItem(itmSource, ItemDC::eSTOPPED);
}

void DownloadCollectionModel::deactivateDownloadItem(TreeItem* itmSource)
{
    doSetPauseStopDownloadItem(itmSource, ItemDC::eQUEUED);
}


void DownloadCollectionModel::doSetPauseStopDownloadItem(TreeItem* itmSource, ItemDC::eSTATUSDC itemStatus)
{
    int id = itmSource->getID();
    ItemDC l_oItm;
    l_oItm.setID(id);
    if (DownloadType::isTorrentDownload(itmSource->downloadType()))
    {
        libtorrent::torrent_handle handle = TorrentManager::Instance()->torrentByModelId(id);
        Q_ASSERT_X(handle.is_valid(), Q_FUNC_INFO, "handle cannot be null!");
        l_oItm.setStatus(handle.is_seed() ? ItemDC::eFINISHED : itemStatus);
    }
    else
    {
        l_oItm.setStatus(itemStatus);
    }
    l_oItm.setSpeed(0.f);
    on_statusChange(l_oItm);
    on_speedChange(l_oItm);

    emit signalPauseDownloadItemWithID(id, itmSource->downloadType());
}

void DownloadCollectionModel::setContinueDownloadItem(const QModelIndex& a_index)
{
    if (!a_index.isValid())
    {
        return;
    }

    if (TreeItem* itmSource = getItem(a_index))
    {
        itmSource->setPriority(a_index.row());
        setContinueDownloadItem(itmSource);
    }
}

void DownloadCollectionModel::setContinueDownloadItem(TreeItem* itmSource)
{
    int id = itmSource->getID();
    ItemDC::eSTATUSDC status = itmSource->getStatus();
    if (DownloadType::isTorrentDownload(itmSource->downloadType()))
    {
        if (ItemDC::eFINISHED == status)
        {
            itmSource->setStatus(ItemDC::eSTARTING);
            emit dataChanged(index(itmSource, eDC_Status), index(itmSource, eDC_Status));
            TorrentManager::Instance()->resumeTorrent(id); // seeding
            return;
        }
        else if (ItemDC::eERROR == status)
        {
            if (TorrentManager::Instance()->restartTorrent(id))
            {
                itmSource->setStatus(ItemDC::eQUEUED);
                emit dataChanged(index(itmSource, eDC_url), index(itmSource, eDC_Status));
                emit signalContinueDownloadItemWithID(id, itmSource->downloadType());
            }
            return;
        }
    }
    ItemDC l_oItm;
    l_oItm.setID(id);
    l_oItm.setStatus(ItemDC::eQUEUED);
    on_statusChange(l_oItm);

    emit signalContinueDownloadItemWithID(id, itmSource->downloadType());
}

bool DownloadCollectionModel::loadFromFile()
{
    bool succeeded = false;

    const QString filePath = utilities::PrepareCacheFolder() + MODEL_STATE_FILE_NAME;
    QString bakFile  = filePath + "-";

    while (QFile::exists(bakFile + "-")) { bakFile = bakFile + "-"; }

    QFile input(bakFile);
    const bool bakFileUsed = input.open(QIODevice::ReadOnly); // try opening at once
    if (bakFileUsed || (input.setFileName(filePath), input.open(QIODevice::ReadOnly)))
    {
        QXmlStreamReader stream(&input);

        succeeded = utilities::DeserializeObject(&stream, this);
        if (succeeded)
        {
            onModelUpdated();
        }
        input.close();
    }

    if (bakFileUsed)
    {
        utilities::DeleteFileWithWaiting(filePath);
        input.rename(filePath);
    }
    else
    {
        utilities::DeleteFileWithWaiting(bakFile);
    }

    return succeeded;
}

void DownloadCollectionModel::saveToFile()
{
    qDebug() << __FUNCTION__;
    const QString filePath = utilities::PrepareCacheFolder() + MODEL_STATE_FILE_NAME;

#ifdef Q_OS_WIN32
    QString folder = utilities::PrepareCacheFolder();
    const wchar_t* ch = qUtf16Printable(folder);
    ULARGE_INTEGER freeBytesAvailable;
    BOOL b = GetDiskFreeSpaceExW(ch, &freeBytesAvailable, 0, 0);
    if (b && freeBytesAvailable.QuadPart < 1024 * 1024 * 5)
    {
        QMessageBox::critical(0, tr("Error"), tr("Not enough space on disk '%1:'\nCannot save data!").arg(folder.at(0)));
        qWarning() << "Not enough space on system disk!";
        return;
    }
#endif

    utilities::FileSaveGuard fileSafer(filePath);

    QFile output(filePath);
    if (fileSafer.isTempFileNoError() && output.open(QIODevice::WriteOnly))
    {
        QXmlStreamWriter stream(&output);

        stream.setCodec(utilities::CustomUtf8Codec::Instance());

        stream.setAutoFormatting(true);
        stream.writeStartDocument();

        utilities::SerializeObject(&stream, this, "model");

        stream.writeEndDocument();
        const bool failed = stream.hasError();
        output.close();
        if (!failed)
        {
            fileSafer.ok();
        }
    }
    else
    {
        qWarning() << "Could not save model data to the file: " << filePath;
    }
}

void DownloadCollectionModel::queueSaveToFile()
{
    VERIFY(QMetaObject::invokeMethod(this, "saveToFile", Qt::QueuedConnection));
}

void DownloadCollectionModel::setTorrentFilesPriorities(ItemID a_ID, QStringList priorities)
{
    if (TreeItem* item = getRootItem()->findItemByID(a_ID))
    {
        item->setTorrentFilesPriorities(std::move(priorities));
        queueSaveToFile();
    }
}

TreeItem* DownloadCollectionModel::findItemByURL(const QString& a_url) const
{
    return getRootItem()->findItemByURL(a_url);
}

void DownloadCollectionModel::on_actualURLChange(const ItemDC& a_item)
{
    QString actual = a_item.actualURL();
    if (actual.isEmpty())
    {
        return;
    }

    if (TreeItem* item = getRootItem()->findItemByID(a_item.getID()))
    {
        item->setActualURL(actual);
        item->setSource(global_functions::GetNormalizedDomain(actual));
        emit dataChanged(index(item, eDC_Source), index(item, eDC_Source));
    }
}


QMimeData* DownloadCollectionModel::mimeData(const QModelIndexList& indexes) const
{
    QModelIndexList copy(indexes);

    qSort(copy.begin(), copy.end(), rowPred<true>());

    int prevRow = -1;

    const int iEnd(copy.size());
    for (int i(0); i != iEnd; ++i)
    {
        const int row = copy[i].row();
        if (row != prevRow)
        {
            copy.push_back(index(row, eDC_ID, copy[i].parent()));
            prevRow = row;
        }
    }
    return QAbstractItemModel::mimeData(copy);
}

bool DownloadCollectionModel::dropMimeData(
    const QMimeData* data,
    Qt::DropAction action,
    int row, int column,
    const QModelIndex& parent)
{
    isDropAction = true;
    bool result = QAbstractItemModel::dropMimeData(data, action, row, 0, parent);
    isDropAction = false;
    onModelUpdated();

    emit onItemsReordered();

    return result;
}

void DownloadCollectionModel::onModelUpdated()
{
    int priorityCounter = -1; // to set -1 to root and start from 0

    forAll([&priorityCounter](TreeItem & item)
    {
        item.setPriority(priorityCounter++);
    });
    emit signalModelUpdated();

    calculateAllProgress();
    activeDownloadsChanged();
}

void DownloadCollectionModel::init()
{
    forAll([](TreeItem & ti)
    {
        if (DownloadType::isTorrentDownload(ti.downloadType()))
        {
            QByteArray hash = QByteArray::fromBase64(ti.hash().toLatin1());
            char hexstring[41];
            libtorrent::to_hex((char const*)hash.constData(), libtorrent::sha1_hash::size, hexstring);
            QString torrOrMagnet = utilities::PrepareCacheFolder(TORRENTS_SUB_FOLDER) + hexstring + ".torrent";
            if (!QFile::exists(torrOrMagnet))
            {
                torrOrMagnet = ti.initialURL();
            }

            std::vector<boost::uint8_t> file_priorities;
            const auto list = ti.torrentFilesPriorities();
            if (!list.isEmpty() && !(list.size() == 1 && list.at(0).isEmpty()))
            {
                for (const auto& p : list)
                    file_priorities.push_back(p.toInt());
            }
            libtorrent::torrent_handle handle = TorrentManager::Instance()->addTorrent(
                torrOrMagnet, 
                ti.getID(), 
                false/*without choose files dialog*/, 
                ti.torrentSavePath(),
                file_priorities.empty()? nullptr : &file_priorities);
            if (handle.is_valid())
            {
                if (ItemDC::eSEEDING == ti.getStatus())
                {
                    qDebug() << __FUNCTION__ << "resuming torrent that is to be seeding:" << handle.name().c_str();
                    handle.resume();
                }
            }
            else
            {
                ti.setStatus(ItemDC::eERROR); // TODO set error description
                ti.setWaitingTime(0); // no recovery
            }
        }
    });
}

inline QVariant DownloadCollectionModel::statusName(TreeItem* item) const
{
    const ItemDC::eSTATUSDC status = item->getStatus();
    return itemDCStatusToString(status);
}

void DownloadCollectionModel::on_torrentMoved(const ItemDC& a_item)
{
    TreeItem* item = getRootItem()->findItemByID(a_item.getID());
    item->setTorrentSavePath(a_item.torrentSavePath());
    // No data update in view? Ok.
}

#pragma once

#include <QAbstractItemModel>
#include <QStringList>
#include <QUrl>
#include <QByteArray>

#include "treeitem.h"
#include "downloadtype.h"


enum eDCMODEL
{
    eDC_ID = 0,
    eDC_url,
#ifdef DEVELOPER_FEATURES
    eDC_priority,
#endif
    eDC_Status,
    eDC_Speed,
    eDC_Speed_Uploading,
    eDC_percentDownl,
    eDC_Size,
    eDC_downlFileName,
    eDC_extrFileName,
    eDC_Source,

    // must be in the end of the enum
    eDC_columnsCount
};

class DownloadCollectionModel : public QAbstractItemModel, public Singleton<DownloadCollectionModel>
{
    Q_OBJECT

friend class Singleton<DownloadCollectionModel>;

public:
    void init();

    QModelIndex parent(const QModelIndex& child) const override;

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual Qt::DropActions supportedDropActions() const override;

    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action,
                              int row, int column, const QModelIndex& parent) override;
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const override;

    QModelIndex moveItem(const QModelIndex& a_index, int numSteps);
    QModelIndexList moveItems(QModelIndexList&& selectedInds, int step);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool insertRows(int position, int rows,    const QModelIndex& parent = QModelIndex()) override;
    bool removeRows(int position, int rows,    const QModelIndex& parent = QModelIndex()) override;

    QModelIndex index(TreeItem* item, int column) const;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    TreeItem* getItem(const QModelIndex& index) const;

    bool deleteURLFromModel(ItemID a_ID, int deleteWithFiles = 0); // deleteWithFiles onlyFor torrents for now
    void deleteALLFinished();

    Q_PROPERTY(QObject* rootItem READ getRootItem WRITE setRootItem)
    TreeItem* getRootItem() const { return rootItem; }

    Q_PROPERTY(QString torrentSessionState READ getTorrentSessionState WRITE setTorrentSessionState)
    QString getTorrentSessionState() const
    {
        return m_torrentSessionState;
    }
    void setTorrentSessionState(const QString& value)
    {
        m_torrentSessionState = value;
    }

    template<class Pred>
    TreeItem* findItem(Pred fn)
    {
        return rootItem->findItem(fn);
    }

    ItemDC getItemByID(ItemID a_item);
    TreeItem* findItemByURL(const QString& a_url) const;
    ItemDC::eSTATUSDC getItemStatus(const QModelIndex& index);
    void setPauseDownloadItem(const QModelIndex& a_index);
    void setPauseDownloadItem(TreeItem* itmSource);
    void setContinueDownloadItem(const QModelIndex& a_index);
    void setContinueDownloadItem(TreeItem* itmSource);
    void setStopDownloadItem(const QModelIndex& a_index);
    void setStopDownloadItem(TreeItem* itmSource);

    void deactivateDownloadItem(TreeItem* itmSource);

    bool loadFromFile();

    template<class Fn_t> void forAll(Fn_t fn)
    {
        rootItem->forAll(fn);
    }

public slots:
    void saveToFile();

    void on_actualURLChange(const ItemDC& a_item);
    void on_statusChange(const ItemDC& a_item);
    void on_torrentMoved(const ItemDC& a_item);
    void on_speedChange(const ItemDC& a_item);
    void on_percentDownloadChange(const ItemDC& a_item);
    void on_sizeChange(const ItemDC& a_item);
    void on_downloadedFileNameChange(const ItemDC& a_item);
    void on_extractedFileNameChange(const ItemDC& a_item);
    void on_sizeCurrDownlChange(const ItemDC& a_item);
    void on_waitingTimeChange(const ItemDC& a_item);
    void on_ItemDCchange(const ItemDC& a_item);
    void on_magnetLinkInfoReceived(const ItemDC& a_item);
protected:

    void setRootItem(QObject* item)
    {
        // TODO verify cast
        rootItem = static_cast<TreeItem*>(item);
    }

    void copyItemInRow(const QModelIndex& indexFrom, int rowTo);

    template<bool isUp>
    QModelIndexList moveItems_helper(QModelIndexList&& selectedInds, int step);

    void addItemsToModel(const QStringList& urls, DownloadType::Type type);

signals:
    void signalDeleteURLFromModel(int a_ID, DownloadType::Type type, int deleteWithFiles = 0);
    void signalPauseDownloadItemWithID(int a_ID, DownloadType::Type type);
    void signalContinueDownloadItemWithID(int a_ID, DownloadType::Type type);
    void signalModelUpdated();
    void signalUrlAdded(const QUrl& url, DownloadType::Type type);

    void onDownloadStarted();
    void onItemsReordered();
    void statusChanged();
    void downloadingFinished(const ItemDC& item);

    void existingItemAdded(const QModelIndex& index);
    void overallProgress(int progress);
    void activeDownloadsNumberChanged(int number);

private:
    DownloadCollectionModel();
    virtual ~DownloadCollectionModel();

    QVariant statusName(TreeItem* item) const;

    void doSetPauseStopDownloadItem(TreeItem* itmSource, ItemDC::eSTATUSDC status);
    void calculateAllProgress();
    void activeDownloadsChanged();

private:
    TreeItem* rootItem;
    bool isDropAction;
    QString m_torrentSessionState;

    void onModelUpdated();

    friend class DownloadManager;
    friend class MainForm;
};

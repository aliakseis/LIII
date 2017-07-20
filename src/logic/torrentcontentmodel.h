#pragma once

#include "torrentcontentmodelitem.h"

#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_handle.hpp>

#include <vector>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVector>
#include <QVariant>
#include <QString>


class TorrentContentModel:  public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TorrentContentModel(QObject* parent = 0);
    ~TorrentContentModel();

    void updateFilesProgress(const std::vector<boost::int64_t>& fp);
    template <class Prior_t> void updateFilesPriorities(const std::vector<Prior_t>& fprio);
    template <class Prior_t> void getFilesPriorities(std::vector<Prior_t>& prio) const;
    bool allFiltered() const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    TorrentContentModelItem::FileType getType(const QModelIndex& index) const;
    int getFileIndex(const QModelIndex& index);
    TorrentContentModelItem* getTorrentContentModelItem(const QModelIndex& index)const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& index) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    void clear();
    void setupModelData(const libtorrent::torrent_info& t, const libtorrent::torrent_status& tStatus);
    void setSavePath(const QString& savePath);
    QString getSavePath();

Q_SIGNALS:
    void filteredFilesChanged();

public Q_SLOTS:
    void selectAll();
    void selectNone();

private:
    TorrentContentModelItem* m_rootItem;
    QVector<TorrentContentModelItem*> m_filesIndex;
    QString m_savePath;
};

//================================= implementation ==========================================

template <class Prior_t>
void TorrentContentModel::updateFilesPriorities(const std::vector<Prior_t>& fprio)
{
    emit layoutAboutToBeChanged();
    Q_ASSERT(m_filesIndex.size() == (int)fprio.size());
    if (m_filesIndex.size() != (int)fprio.size()) { return; }
    for (size_t i = 0; i < fprio.size(); ++i)
    {
        m_filesIndex[i]->setPriority(fprio[i]);
    }
    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

template<class Prior_t>
void TorrentContentModel::getFilesPriorities(std::vector<Prior_t>& prio) const
{
    prio.clear();
    prio.reserve(m_filesIndex.size());
    std::transform(
        m_filesIndex.constBegin(), m_filesIndex.constEnd(),
        std::back_inserter(prio),
        std::bind(&TorrentContentModelItem::getPriority, std::placeholders::_1));
}

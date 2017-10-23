#pragma once

#include "torrentcontentmodelitem.h"

#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_handle.hpp>

#include <functional>
#include <vector>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVector>
#include <QVariant>
#include <QString>
#include <QFileIconProvider>


class TorrentContentModel:  public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TorrentContentModel(QObject* parent = 0);
    ~TorrentContentModel();

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    void updateFilesProgress(const std::vector<boost::int64_t>& fp);
    template <class Prior_t> void updateFilesPriorities(const std::vector<Prior_t>& fprio);
    template <class Prior_t> void getFilesPriorities(std::vector<Prior_t>& prio) const;
    bool allFiltered() const;
    TorrentContentModelItem::FileType getType(const QModelIndex& index) const;
    TorrentContentModelItem* getTorrentContentModelItem(const QModelIndex& index)const;
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
    QFileIconProvider m_iconProvider;
};

// implementation

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
        std::mem_fn(&TorrentContentModelItem::getPriority));
}

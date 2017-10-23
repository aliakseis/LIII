#pragma once

#include <QSortFilterProxyModel>
#include "torrentcontentmodelitem.h"

class TorrentContentModel;

class TorrentContentFilterModel
    : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit TorrentContentFilterModel(QObject* parent = 0);
    virtual ~TorrentContentFilterModel();

    TorrentContentModel* model() const;
    TorrentContentModelItem::FileType getType(const QModelIndex& index) const;
    TorrentContentModelItem* getTorrentContentModelItem(const QModelIndex& index) const;
    QModelIndex parent(const QModelIndex& child) const override;

signals:
    void filteredFilesChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

public slots:
    void selectAll();
    void selectNone();

private:
    TorrentContentModel* m_model;
};

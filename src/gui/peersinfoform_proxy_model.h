#pragma once

#include <QSortFilterProxyModel>

class PeersSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
};

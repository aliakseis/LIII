#pragma once

#include <QSortFilterProxyModel>

class PeersSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    typedef  QSortFilterProxyModel base_class;
    explicit PeersSortFilterProxyModel(QObject* parent = 0);
protected:
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
};

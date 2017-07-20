#pragma once

#include <QStyledItemDelegate>

class DownloadCollectionDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DownloadCollectionDelegate(QWidget* parent = 0);
    ~DownloadCollectionDelegate();

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

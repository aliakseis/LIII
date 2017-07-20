#pragma once

#include <QStyledItemDelegate>
#include "torrentcontentmodelitem.h"

// Defines for properties list columns
typedef TorrentContentModelItem::TreeItemColumns PropColumn;


class PropListDelegate: public QStyledItemDelegate
{
    Q_OBJECT

private:

signals:
    void filteredFilesChanged() const;

public:
    explicit PropListDelegate(QObject* parent = 0);
    ~PropListDelegate();

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void setEditorData(QWidget* editor, const QModelIndex& index) const;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /* option */, const QModelIndex& index) const;

    public slots:
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const;
};

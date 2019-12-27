#pragma once

#include <QStyledItemDelegate>
#include "torrentcontentmodelitem.h"


class PropListDelegate: public QStyledItemDelegate
{
    Q_OBJECT

private:

signals:
    void filteredFilesChanged() const;

public:
    explicit PropListDelegate(QObject* parent = 0);
    ~PropListDelegate();

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const override;

    public slots:
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const override;
};

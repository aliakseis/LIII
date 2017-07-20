#pragma once

#include <QDialog>
#include <QRgb>
#include <QList>
#include <QStyledItemDelegate>

class QTreeWidgetItem;
class QUrl;

namespace Ui
{
class AddLinks;
}


class ComboboxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    typedef QStyledItemDelegate BaseClass;
    ComboboxDelegate(QObject* parent = 0);

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;
};

class AddLinks : public QDialog
{
    Q_OBJECT

public:
    explicit AddLinks(const QStringList& urls, QWidget* parent = 0);
    ~AddLinks();

    QStringList urls() const;

private:
    Ui::AddLinks* ui;

    void addUrlToList(const QString& url);
    void selectAll(bool select);
    void updateSelectButtons();

private Q_SLOTS:
    void on_selectAllButton_clicked();
    void on_selectNoneButton_clicked();
    void on_listWidget_itemClicked(QTreeWidgetItem* item);
};

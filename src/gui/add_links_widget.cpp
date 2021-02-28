#include "add_links_widget.h"
#include <QHeaderView>

AddLinksWidget::AddLinksWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setIndentation(0);
    setColumnCount(3);
    QTreeWidgetItem* head = headerItem();
    head->setText(0, tr("Link"));
    head->setText(1, "");
    head->setText(2, tr("Priority"));
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
    header()->setSectionResizeMode(2, QHeaderView::Fixed);
    header()->setSectionsMovable(false);

    setColumnWidth(0, 30);
    setColumnWidth(1, 380);
    header()->setStyleSheet("QHeaderView::section:horizontal{background-color: #f4f4f4;} QHeaderView::section:horizontal:first{border-right-color: #f4f4f4;}");
}

#include "torrentdetailscontentview.h"

#include <QMenu>
#include <QFileInfo>
#include <QDir>
#include <QFocusEvent>
#include <QApplication>
#include "utilities/utils.h"
#include "utilities/filesystem_utils.h"
#include "torrentcontentmodel.h"
#include "global_functions.h"

#include <utility>

TorrentDetailsContentView::TorrentDetailsContentView(QWidget* parent /*= 0*/): QTreeView(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    VERIFY(connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(on_showTreeTorentContextMenu(const QPoint&))));
}

void TorrentDetailsContentView::setModel(TorrentContentFilterModel* a_model)
{
    QTreeView::setModel(a_model);

    VERIFY(connect(this, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(on_ItemOpenFolder())));
}

TorrentContentFilterModel* TorrentDetailsContentView::model()
{
    return static_cast<TorrentContentFilterModel*>(QTreeView::model());
}

void TorrentDetailsContentView::on_showTreeTorentContextMenu(const QPoint& pos)
{
    QModelIndex index = indexAt(pos);
    if (index.isValid())
    {
        QMenu menu;
        menu.setObjectName(QStringLiteral("TorrentDetailsContextMenu"));
        menu.addAction(QIcon(), tr("Open in folder"), this, SLOT(on_ItemOpenFolder()));
        menu.addAction(QIcon(), tr("Open File"), this, SLOT(on_ItemOpenFile()));

        menu.exec(QCursor::pos());
    }
}

void TorrentDetailsContentView::on_ItemOpenFolder()
{
    QModelIndex curr_index = selectionModel()->currentIndex();
    if (!curr_index.isValid())
    {
        return;
    }

    TorrentContentModelItem* torrentItem = model()->getTorrentContentModelItem(curr_index);
    QString pathFile = torrentItem->getPath();
    if (pathFile.isEmpty())
    {
        pathFile = torrentItem->getName();
    }
    QString savePath = model()->model()->getSavePath();
    QString filename = savePath + pathFile;
    QFileInfo downloadFile(filename);
    utilities::SelectFile(downloadFile.absoluteFilePath(), downloadFile.dir().path());
}

void TorrentDetailsContentView::on_ItemOpenFile()
{
    QModelIndex curr_index = selectionModel()->currentIndex();
    if (!curr_index.isValid())
    {
        return;
    }

    TorrentContentModelItem* torrentItem = model()->getTorrentContentModelItem(curr_index);
    QString pathFile = torrentItem->getPath();
    QString savePath = model()->model()->getSavePath();
    QString filename = savePath + pathFile;
    if (torrentItem->isFolder() || !QFile::exists(filename))
    {
        return;
    }

    global_functions::openFile(filename);
}

bool TorrentDetailsContentView::event(QEvent *event)
{
    if (event->type() == QEvent::FocusOut
        && static_cast<QFocusEvent*>(event)->reason() == Qt::MouseFocusReason)
    {
        if (QWidget *widget = QApplication::focusWidget()) 
        {
            const QModelIndexList selectedRows = selectionModel()->selectedRows(TorrentContentModelItem::COL_PRIO);
            for (const auto& idx : qAsConst(selectedRows))
            {
                if (indexWidget(idx) == widget)
                {
                    return QAbstractScrollArea::event(event);
                }
            }
        }
    }

    return QTreeView::event(event);
}

void TorrentDetailsContentView::commitData(QWidget *editor)
{
    QModelIndexList selectedRows = selectionModel()->selectedRows(TorrentContentModelItem::COL_PRIO);
    for (const auto& idx : qAsConst(selectedRows))
    {
        if (indexWidget(idx) == editor)
        {
            model()->setSelectedRows(std::move(selectedRows));
            break;
        }
    }

    QTreeView::commitData(editor);

    model()->setSelectedRows(QModelIndexList());
}

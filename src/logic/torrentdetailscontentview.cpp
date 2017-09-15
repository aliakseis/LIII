#include "torrentdetailscontentview.h"

#include <QMenu>
#include <QFileInfo>
#include <QDir>
#include "utilities/utils.h"
#include "utilities/filesystem_utils.h"
#include "torrentcontentmodel.h"
#include "global_functions.h"

TorrentDetailsContentView::TorrentDetailsContentView(QWidget* parent /*= 0*/): QTreeView(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    VERIFY(connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(on_showTreeTorentContextMenu(const QPoint&))));
}

void TorrentDetailsContentView::setModel(TorrentContentFilterModel* a_model)
{
    QTreeView::setModel(a_model);

    VERIFY(connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(on_ItemOpenFolder())));
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

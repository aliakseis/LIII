#include "downloadcollectiontreeview.h"

#include <functional>
#include <array>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QKeyEvent>
#include <QPainter>
#include <QMenu>
#include <QFile>
#include <QMessageBox>
#include <QCheckBox>

#include "utilities/utils.h"

#include "settings_declaration.h"
#include "global_functions.h"
#include "globals.h"

#include "mainwindow.h"

#include "branding.hxx"

#include "addtorrentform.h"
#include "torrentdetailsform.h"
#include "torrentmanager.h"
#include "torrentslistener.h"
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

DownloadCollectionTreeView::DownloadCollectionTreeView(QWidget* parent)
    : QTreeView(parent)
{
    header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setRootIsDecorated(false);
    setDragDropMode(QAbstractItemView::InternalMove);

    setContextMenuPolicy(Qt::CustomContextMenu);
    VERIFY(connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(on_showContextMenu(const QPoint&))));
}

void DownloadCollectionTreeView::drawText(const QString& text)
{
    QPainter painter;
    painter.begin(viewport());
    painter.setPen(Qt::gray);
    painter.setFont(utilities::GetAdaptedFont(12, 2));
    painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignVCenter, text);
    painter.end();
}

void DownloadCollectionTreeView::paintEvent(QPaintEvent* ev)
{
    if (model()->rowCount())
    {
        QTreeView::paintEvent(ev);
    }
    else
    {
        drawText(utilities::Tr::Tr(PASTE_LINKS_CTRLV));
    }
}

void DownloadCollectionTreeView::setModel(DownloadCollectionModel* a_model)
{
    QTreeView::setModel(a_model);

    VERIFY(connect(this, SIGNAL(clicked(const QModelIndex&)), this, SLOT(on_clicked(const QModelIndex&))));
    VERIFY(connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(on_doubleClicked(const QModelIndex&))));
    VERIFY(connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                   this, SLOT(on_ItemSelectChanged(const QItemSelection&, const QItemSelection&))));

    VERIFY(connect(a_model, SIGNAL(existingItemAdded(const QModelIndex&)), this, SLOT(onExistingItemAdded(const QModelIndex&))));
    VERIFY(connect(a_model, SIGNAL(statusChanged()), this, SLOT(getUpdateItem())));

    VERIFY(connect(a_model, SIGNAL(downloadingFinished(const ItemDC&)), this, SLOT(downloadingFinished(const ItemDC&))));

    header()->setSectionsMovable(false);
    setSortingEnabled(false);

    HeaderResize();
}

DownloadCollectionModel* DownloadCollectionTreeView::model()
{
    return static_cast<DownloadCollectionModel*>(QTreeView::model());
}


void DownloadCollectionTreeView::keyPressEvent(QKeyEvent* event)
{
    const bool isOnlyCtrlKeyPressed = (event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier;
    const bool isOnlyShiftKeyPressed = (event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier;
    switch (event->key())
    {
    case Qt::Key_Delete:
        deleteSelectedRows(isOnlyShiftKeyPressed);
        break;
    case Qt::Key_A:
        if (isOnlyCtrlKeyPressed)
        {
            selectAll();
        }
        break;
    case Qt::Key_C:
        if (isOnlyCtrlKeyPressed)
        {
            copyURLToClipboard();
        }
        break;
    case Qt::Key_Up:
        if (isOnlyCtrlKeyPressed)
        {
            on_MoveUp();
        }
        break;
    case Qt::Key_Down:
        if (isOnlyCtrlKeyPressed)
        {
            on_MoveDown();
        }
        break;
    }
    QWidget::keyPressEvent(event);
}

void DownloadCollectionTreeView::HeaderResize(/*const int width*/)
{
    header()->setSectionHidden(eDC_ID, true);
    header()->setSectionHidden(eDC_downlFileName, true);

    header()->resizeSection(eDC_url, 200);
    header()->resizeSection(eDC_Speed, 73);
    header()->resizeSection(eDC_Speed_Uploading, 70);
    header()->resizeSection(eDC_percentDownl, 100);
    header()->resizeSection(eDC_Size, 100);

    header()->resizeSection(eDC_Source, 80);
}

void DownloadCollectionTreeView::copyURLToClipboard()
{
    QStringList strList;
    QClipboard* clipboard = QApplication::clipboard();
    const QModelIndexList selectedRows = selectionModel()->selectedRows();
    for (const auto& idx : qAsConst(selectedRows))
        strList.append(model()->getItem(idx)->initialURL().replace(" ", "%20"));
    if (!strList.isEmpty())
    {
        clipboard->setText(strList.join("\n"));
    }
}

void DownloadCollectionTreeView::deleteSelectedRows(bool totally)
{
    if (cancelDownloadingQuestion(totally))
    {
        for (const QModelIndex & index : selectionModel()->selectedRows())
        {
            TreeItem* item = model()->getItem(index);
            if (totally || (item->getStatus() != ItemDC::eFINISHED && item->getStatus() != ItemDC::eSEEDING))
            {
                QString arch_file = item->downloadedFileName();
                safelyDeleteVideoFile(arch_file);
            }
            int deleteWithFiles = totally ? libtorrent::session::delete_files : 0;
            model()->deleteURLFromModel(item->getID(), deleteWithFiles);
        }

        VERIFY(QMetaObject::invokeMethod(model(), "saveToFile", Qt::QueuedConnection));
    }
}

void DownloadCollectionTreeView::safelyDeleteVideoFile(QString const& file)
{
    using namespace app_settings;

    if (QSettings().value(DeleteToRecycleBin, DeleteToRecycleBin_Default).toBool())
    {
        utilities::MoveToTrash(file);
    }
    else
    {
        utilities::DeleteFileWithWaiting(file);
    }
}

void DownloadCollectionTreeView::downloadingFinished(const ItemDC& a_item)
{
    if (a_item.getStatus() == ItemDC::eFINISHED)
    {
        QString fileName;
        if (DownloadType::isTorrentDownload(a_item.downloadType()))
        {
            fileName = utilities::GetFileName(TorrentManager::Instance()->torrentRootItemPath(a_item.getID()));
        }
        else
        {
            fileName = utilities::GetFileName(a_item.downloadedFileName());
        }
        if (!fileName.isEmpty())
        {
            emit signalDownloadFinished(fileName);
        }
    }
}

void DownloadCollectionTreeView::getUpdateItem()
{
    auto prc = canPRCSEnabled();

    emit signalButtonChangePauseImage(
        std::get<0>(prc),
        std::get<1>(prc),
        std::get<2>(prc),
        std::get<3>(prc));
}
void DownloadCollectionTreeView::on_ItemSelectChanged(const QItemSelection& current, const QItemSelection& previous)
{
    getUpdateItem();
}

std::array<bool, 4> DownloadCollectionTreeView::canPRCSEnabled()
{
    std::array<bool, 4> result; // bool canPause, bool canResume, bool canCancel
    std::fill(result.begin(), result.end(), false);
    Q_FOREACH(const QModelIndex & ind, selectionModel()->selectedRows())
    {
        TreeItem* ti = model()->getItem(ind);
        result[0] = result[0] || ti->canPause();
        result[1] = result[1] || ti->canResume();
        result[2] = result[2] || ti->canCancel();
    }
    return result;
}

void DownloadCollectionTreeView::startDownloadItem()
{
    QModelIndexList items = selectionModel()->selectedRows();
    Q_FOREACH(const QModelIndex & ind, items)
    {
        TreeItem* ti = model()->getItem(ind);
        if (ti && ti->canResume())
        {
            model()->setContinueDownloadItem(ind);
        }
    }
    getUpdateItem();
}

void DownloadCollectionTreeView::pauseDownloadItem()
{
    QModelIndexList items = selectionModel()->selectedRows();
    Q_FOREACH(const QModelIndex & ind, items)
    {
        TreeItem* ti = model()->getItem(ind);
        if (ti && ti->canPause())
        {
            model()->setPauseDownloadItem(ind);
        }
    }
    getUpdateItem();
}


void DownloadCollectionTreeView::on_showContextMenu(const QPoint& a_point)
{
    QPoint cursorPos = QCursor::pos();
    QModelIndex index = selectionModel()->currentIndex();
    if (index.isValid())
    {
        {
            QMenu menu;
            menu.setObjectName("DownloadContextMenu");

            const auto dlType = model()->getItem(index)->downloadType();
            // Show In Folder menu item
            QAction* openFolder = menu.addAction(QIcon(":/icons/Drop-down-folder-icon-normal.png"), utilities::Tr::Tr(TREEVIEW_MENU_OPENFOLDER), this, SLOT(on_OpenFolder()));
            openFolder->setEnabled(dlType != DownloadType::MagnetLink);

            if (dlType == DownloadType::TorrentFile)
            {
                QString path = TorrentManager::Instance()->torrentRootItemPath(model()->getItem(index)->getID());
                openFolder->setEnabled(QFile::exists(path));
            }

            // Torrent Details menu item
            if (DownloadType::isTorrentDownload(dlType))
            {
                menu.addAction(QIcon(":/icons/Drop-down-torrent-icon-normal.png"), utilities::Tr::Tr(TORRENT_DETAILS_INFO), this, SLOT(on_showTorrentDetails()))
                ->setEnabled(dlType == DownloadType::TorrentFile);
            }

            menu.addSeparator();
            // Download Control section: start, pause, stop, remove, cancel
            QAction* re = menu.addAction(QIcon(":/icons/Drop-down-start-icon-normal.png"), utilities::Tr::Tr(START_LABEL), this, SLOT(on_ItemResume()));
            QAction* pa = menu.addAction(QIcon(":/icons/Drop-down-pause-icon-normal.png"),  utilities::Tr::Tr(PAUSE_LABEL) ,  this, SLOT(on_ItemPause()));

            auto prc = canPRCSEnabled();
            if (std::get<2>(prc)) // can Cancel?
            {
                menu.addAction(QIcon(":/icons/Drop-down-cancel-icon-normal.png"), utilities::Tr::Tr(TREEVIEW_MENU_CANCEL), this, SLOT(on_ItemCancel()));
            }
            else
            {
                menu.addAction(QIcon(":/icons/Drop-down-clean-icon-normal.png"), utilities::Tr::Tr(TREEVIEW_MENU_REMOVE), this, SLOT(on_ItemCancel()));
            }

            pa->setEnabled(std::get<0>(prc));
            re->setEnabled(std::get<1>(prc));

            menu.addSeparator();
            menu.addAction(QIcon(":/icons/Drop-down-up-icon-normal.png"), utilities::Tr::Tr(TREEVIEW_MENU_MOVEUP), this, SLOT(on_MoveUp()));
            menu.addAction(QIcon(":/icons/Drop-down-down-icon-normal.png"), utilities::Tr::Tr(TREEVIEW_MENU_MOVEDOWN), this, SLOT(on_MoveDown()));

            menu.exec(cursorPos);
        }
    }
}


void DownloadCollectionTreeView::on_OpenFolder()
{
    QModelIndex curr_index = currentIndex();
    TreeItem* item = model()->getItem(curr_index);

    const auto type = item->downloadType();
    if (type == DownloadType::MagnetLink)
    {
        return;
    }

    if (type == DownloadType::TorrentFile)
    {
        QString filename = TorrentManager::Instance()->torrentRootItemPath(item->getID());
        QFileInfo downloadFile(filename);
        emit signalOpenTorrentFolder(downloadFile.absoluteFilePath(), downloadFile.dir().path());
    }
    else
    {
        QString filename = item->downloadedFileName();
        emit signalOpenFolder(filename);
    }
}

void DownloadCollectionTreeView::on_ItemResume()
{
    startDownloadItem();
}

void DownloadCollectionTreeView::on_ItemPause()
{
    pauseDownloadItem();
}

void DownloadCollectionTreeView::on_ItemCancel()
{
    deleteSelectedRows();
}

void DownloadCollectionTreeView::moveImpl(int step)
{
    QModelIndexList newInds = model()->moveItems(selectionModel()->selectedRows(), step);

    selectRows(newInds);
}

void DownloadCollectionTreeView::on_MoveUp()
{
    moveImpl(-1);
}

void DownloadCollectionTreeView::on_MoveDown()
{
    moveImpl(1);
}

void DownloadCollectionTreeView::on_clicked(const QModelIndex& a_index)
{
    // Action?
}

void DownloadCollectionTreeView::on_doubleClicked(const QModelIndex& a_index)
{
    on_OpenFolder();
}

bool DownloadCollectionTreeView::cancelDownloadingQuestion(bool totally)
{
    using namespace app_settings;

    bool result = true;

    if (QSettings().value(ShowCancelWarning, ShowCancelWarning_Default).toBool() && !totally)
    {
        QModelIndexList selectedRows = selectionModel()->selectedRows();

        auto foundNotFinished = std::find_if(selectedRows.constBegin(), selectedRows.constEnd(), 
            [this](const QModelIndex& idx)
            {
                return !model()->getItem(idx)->isCompleted();
            });

        if (foundNotFinished != selectedRows.constEnd())
        {
            QMessageBox msgBox(
                QMessageBox::NoIcon,
                utilities::Tr::Tr(PROJECT_FULLNAME_TRANSLATION),
                utilities::Tr::Tr(CANCEL_DOWNLOAD_TEXT),
                QMessageBox::Yes | QMessageBox::No,
                qobject_cast<QWidget*>(parent()));
            msgBox.setCheckBox(new QCheckBox(utilities::Tr::Tr(DONT_SHOW_THIS_AGAIN)));
            result = msgBox.exec() == QMessageBox::Yes;
            if (result)
            {
                const bool isDontShowMeChecked = msgBox.checkBox()->isChecked();
                QSettings().setValue(ShowCancelWarning, !isDontShowMeChecked);
            }
        }
    }
    else if (totally)
    {
        QMessageBox msgBox(
            QMessageBox::NoIcon,
            utilities::Tr::Tr(PROJECT_FULLNAME_TRANSLATION),
            utilities::Tr::Tr(DELETE_DOWNLOAD_TEXT),
            QMessageBox::Yes | QMessageBox::No,
            qobject_cast<QWidget*>(parent()));
        result = msgBox.exec() == QMessageBox::Yes;
    }
    return result;
}


void DownloadCollectionTreeView::on_showTorrentDetails()
{
    qDebug() << Q_FUNC_INFO;
    QModelIndex index = currentIndex();
    TreeItem* item = model()->getItem(index);
    showTorrentDetailsDialog(item);
}

void DownloadCollectionTreeView::showTorrentDetailsDialog(TreeItem* item)
{
    libtorrent::torrent_handle handle = TorrentManager::Instance()->torrentByModelId(item->getID());
    Q_ASSERT_X(handle.is_valid(), Q_FUNC_INFO, "invalid torrent handle");

    if (handle.is_valid())
    {
        try
        {
            TorrentDetailsForm dlg(handle, utilities::getMainWindow());
            const int id = item->getID();
            auto connection = connect(
                &TorrentsListener::instance(),
                &TorrentsListener::sizeCurrDownlChange,
                [id, &dlg](const ItemDC& it)
                {
                    if (it.getID() == id)
                    { 
                        VERIFY(QMetaObject::invokeMethod(&dlg, "onProgressUpdated", Qt::QueuedConnection));
                    }
                });
            const bool accepted = dlg.exec() == QDialog::Accepted;
            disconnect(connection);
            if (accepted)
            {
                item->setSize(handle.status(0).total_wanted);
                const auto priorities = dlg.filesPriorities();
                if (priorities != item->torrentFilesPriorities())
                {
                    item->setTorrentFilesPriorities(priorities);
                    VERIFY(QMetaObject::invokeMethod(model(), "saveToFile", Qt::QueuedConnection));
                }
            }
        }
        catch (std::exception const& e)
        {
            qWarning() << Q_FUNC_INFO << "caught exception:" << e.what();
        }
    }
}

void DownloadCollectionTreeView::mouseReleaseEvent(QMouseEvent* me)
{
    if (Qt::RightButton == me->button())
    {
        me->ignore();
    }
    else
    {
        QTreeView::mouseReleaseEvent(me);
    }
}

void DownloadCollectionTreeView::resumeAllItems()
{
    model()->forAll([this](TreeItem & ti)
    {
        if (ti.canResume())
        {
            model()->setContinueDownloadItem(&ti);
        }
    });
    getUpdateItem();
}

void DownloadCollectionTreeView::pauseAllItems()
{
    model()->forAll([this](TreeItem & ti)
    {
        if (ti.canPause())
        {
            model()->setPauseDownloadItem(&ti);
        }
    });
    getUpdateItem();
}

void DownloadCollectionTreeView::selectRows(const QModelIndexList& newInds)
{
    selectionModel()->select(QItemSelection(), QItemSelectionModel::Clear);
    Q_FOREACH(const QModelIndex & idx, newInds)
    {
        selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

void DownloadCollectionTreeView::onExistingItemAdded(const QModelIndex& index)
{
    selectRows(QModelIndexList() << index);
}

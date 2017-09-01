#pragma once

#include <QTreeView>
#include <QHeaderView>

#include <array>

class DownloadCollectionModel;
class TreeItem;
class ItemDC;

class DownloadCollectionTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit DownloadCollectionTreeView(QWidget* parent = 0);
    virtual ~DownloadCollectionTreeView() {}

    void setModel(DownloadCollectionModel* a_model);
    void paintEvent(QPaintEvent*);
    void deleteSelectedRows(bool totally = false);

    void copyURLToClipboard();
    bool cancelDownloadingQuestion(bool totally);
protected:
    void keyPressEvent(QKeyEvent* event);
    void HeaderResize();
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    void safelyDeleteVideoFile(QString const& file);
    void showTorrentDetailsDialog(TreeItem* item);
    DownloadCollectionModel* model();

public slots:
    void startDownloadItem();
    void pauseDownloadItem();
    void pauseAllItems();
    void resumeAllItems();
    void on_OpenFolder();
    void on_ItemResume();
    void on_ItemPause();
    void on_ItemCancel();
    void on_MoveUp();
    void on_MoveDown();
    void getUpdateItem();
    void downloadingFinished(const ItemDC& a_item);
    void on_showTorrentDetails();

protected slots:
    void on_ItemSelectChanged(const QItemSelection& current, const QItemSelection& previous);
    void on_clicked(const QModelIndex& a_index);
    void on_doubleClicked(const QModelIndex& index);
    void on_showContextMenu(const QPoint& a_point);

    void onExistingItemAdded(const QModelIndex& index);

signals:
    void signalButtonChangePauseImage(bool canPause, bool canResume, bool canCancel, bool canStop);
    void signalOpenFolder(const QString& filename);
    void signalOpenTorrentFolder(const QString& filename, const QString& downloadDirectory);
    void signalDownloadFinished(const QString& filename);

private:
    std::array<bool, 4> canPRCSEnabled();// using all selected items
    void drawText(const QString& text);

    void moveImpl(int step);

    void selectRows(const QModelIndexList& newInds);
};

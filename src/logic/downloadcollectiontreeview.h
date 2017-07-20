#pragma once

#include <QTreeView>
#include "downloadcollectionmodel.h"
#include <QHeaderView>

#include <array>

class MainHeaderTreeView : public QHeaderView
{
    Q_OBJECT
public:
    MainHeaderTreeView(Qt::Orientation orientation, QWidget* parent = 0);
    virtual ~MainHeaderTreeView() {};
protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const;
};

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
    virtual void mouseReleaseEvent(QMouseEvent*) override;

private:
    void safelyDeleteVideoFile(QString const& file);
    void showTorrentDetailsDialog(TreeItem* item);

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
    void on_internetConnectedChanged(bool isConnected);

    void onExistingItemAdded(const QModelIndex& index);

signals:
    void signalButtonChangePauseImage(bool canPause, bool canResume, bool canCancel, bool canStop);
    void signalOpenFolder(const QString& filename);
    void signalOpenTorrentFolder(const QString& filename, const QString& downloadDirectory);
    void signalDownloadFinished(const QString& filename);
protected:
    DownloadCollectionModel* m_model;
private:

    std::array<bool, 4> canPRCSEnabled();// using all selected items
    void drawText(const QString& text);

    void moveImpl(int step);

    void selectRows(const QModelIndexList& newInds);

    bool m_internetConnected;
};

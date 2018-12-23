#pragma once

#include <QTreeView>
#include "torrentcontentfiltermodel.h"

class TorrentDetailsContentView : public QTreeView
{
    Q_OBJECT
public:
    explicit TorrentDetailsContentView(QWidget* parent = 0);
    virtual ~TorrentDetailsContentView() {}

    void setModel(TorrentContentFilterModel* a_model);

    bool event(QEvent *event) override;
    void commitData(QWidget *editor) override;


private Q_SLOTS:
    void on_showTreeTorentContextMenu(const QPoint& pos);
    void on_ItemOpenFolder();
    void on_ItemOpenFile();

private:
    TorrentContentFilterModel* model();
};

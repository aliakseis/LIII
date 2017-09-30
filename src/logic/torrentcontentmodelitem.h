#pragma once

#include <QList>
#include <QVariant>
#include <libtorrent/torrent_info.hpp>
#include "treeitem.h"

namespace prio
{
enum FilePriority {IGNORED = 0, LOW = 1, NORMAL = 2, HIGH = 7, PARTIAL = -1};
}

class TorrentContentModelItem
{
public:
    enum TreeItemColumns {COL_NAME, COL_SIZE, COL_STATUS, COL_PROGRESS, COL_PRIO, NB_COL};
    enum FileType {TFILE, FOLDER, ROOT};

    // File Construction
    TorrentContentModelItem(const libtorrent::file_entry& f,
                            TorrentContentModelItem* parent);
    // Folder constructor
    TorrentContentModelItem(const QString& name, TorrentContentModelItem* parent = 0);
    // Invisible root item constructor
    explicit TorrentContentModelItem(const QList<QVariant>& data);

    ~TorrentContentModelItem();

    FileType getType() const;

    QString getPath() const;
    QString getName() const;
    void setName(const QString& name);

    qulonglong getSize() const;
    void setSize(qulonglong size);
    void updateSize();
    qulonglong getTotalDone() const;

    ItemDC::eSTATUSDC getStatus()const;
    void setStatus(ItemDC::eSTATUSDC);
    void updateStatus();

    void setProgress(qulonglong done);
    float getProgress() const;
    void updateProgress();

    int getPriority() const;
    void setPriority(int new_prio, bool update_parent = true);
    void updatePriority();

    TorrentContentModelItem* childWithName(const QString& name) const;
    bool isFolder() const;

    void appendChild(TorrentContentModelItem* item);
    TorrentContentModelItem* child(int row) const;
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;

    TorrentContentModelItem* parent() const;
    void deleteAllChildren();
    const QList<TorrentContentModelItem*>& children() const;

private:
    TorrentContentModelItem* m_parentItem;
    FileType m_type;
    QList<TorrentContentModelItem*> m_childItems;
    QList<QVariant> m_itemData;
    qulonglong m_totalDone;
    QString m_path;

    void init(QString name, const qlonglong size);
};

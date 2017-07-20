#include <QApplication>
#include <QPainter>

#include "downloadcollectiondelegate.h"
#include "downloadcollectionmodel.h"

DownloadCollectionDelegate::DownloadCollectionDelegate(QWidget* parent/* = 0*/) : QStyledItemDelegate(parent)
{
}


DownloadCollectionDelegate::~DownloadCollectionDelegate()
{
}

void DownloadCollectionDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return;
    }

    QStyledItemDelegate::paint(painter, option, index);
    if (index.column() == eDC_percentDownl)
    {
        // Set up a QStyleOptionProgressBar to precisely mimic the
        // environment of a progress bar.
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.state = option.state;
        progressBarOption.direction = QApplication::layoutDirection();
#ifdef Q_OS_WIN
        progressBarOption.rect = option.rect.adjusted(2, 6, -2, -7);
#else
        progressBarOption.rect = option.rect.adjusted(2, 4, -2, -7);
#endif //Q_OS_WIN

        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.textVisible = true;

        // Set the progress and text values of the style option.
        TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
        int progress = childItem->getPercentDownload();
        progressBarOption.progress = progress < 0 ? 0 : progress;

        progressBarOption.text = QString().sprintf("%d%%", progressBarOption.progress);

        // Draw the progress bar onto the view.
        for (auto s : { QStyle::CE_ProgressBarGroove, QStyle::CE_ProgressBarContents, QStyle::CE_ProgressBarLabel })
        {
            QApplication::style()->drawControl(s, &progressBarOption, painter);
        }
    }
}

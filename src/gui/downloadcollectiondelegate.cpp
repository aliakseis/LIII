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
        const double progress = (childItem->size() > 0) ? (childItem->sizeCurrDownl() * 100.) / childItem->size() : 0;
        progressBarOption.progress = static_cast<int>(progress);
        progressBarOption.text = 
            (progress <= 0) ? "0%" : ((progress >= 100) ? "100%" : QString("%1%").arg(progress, 0, 'f', 1));

        // Draw the progress bar onto the view.
        for (auto s : { QStyle::CE_ProgressBarGroove, QStyle::CE_ProgressBarContents, QStyle::CE_ProgressBarLabel })
        {
            QApplication::style()->drawControl(s, &progressBarOption, painter);
        }
    }
}

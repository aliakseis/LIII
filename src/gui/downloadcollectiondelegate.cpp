#include <QApplication>
#include <QPainter>

#include "downloadcollectiondelegate.h"
#include "downloadcollectionmodel.h"

#include "utilities/utils.h"
#include "ui_utils/getstyleoptionprogressbar.h"

#include <algorithm>

DownloadCollectionDelegate::DownloadCollectionDelegate(QWidget* parent/* = 0*/) : QStyledItemDelegate(parent)
{
}

DownloadCollectionDelegate::~DownloadCollectionDelegate()
= default;

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
        auto progressBarOption = getStyleOptionProgressBar();
        progressBarOption.state = option.state;
#ifdef Q_OS_WIN
        progressBarOption.rect = option.rect.adjusted(2, 6, -2, -7);
#else
        progressBarOption.rect = option.rect.adjusted(2, 4, -2, -7);
#endif //Q_OS_WIN

        // Set the progress and text values of the style option.
        const double progress = index.data(Qt::DisplayRole).toDouble();

        progressBarOption.progress = std::min(100, static_cast<int>(progress));
        progressBarOption.text = utilities::ProgressString(progress);

        // Draw the progress bar onto the view.
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    }
}

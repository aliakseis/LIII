#include "getstyleoptionprogressbar.h"

#include <QApplication>

QStyleOptionProgressBar getStyleOptionProgressBar()
{
    QStyleOptionProgressBar progressBarOption;
    progressBarOption.state = QStyle::State_Enabled;
    progressBarOption.direction = QApplication::layoutDirection();
    progressBarOption.fontMetrics = QApplication::fontMetrics();
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.textAlignment = Qt::AlignCenter;
    progressBarOption.textVisible = true;

    return progressBarOption;
}

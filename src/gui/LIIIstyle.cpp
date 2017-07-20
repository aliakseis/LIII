#include "LIIIstyle.h"
#include <QStyleOption>
#include <QPainter>

#ifdef Q_OS_WIN
#define ROUND_RADIUS 3
#else
#define ROUND_RADIUS 7
#endif //Q_OS_WIN

#ifdef Q_OS_WIN32
QRect windowsClassicBug;
#endif

void LIIIStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::CE_ProgressBarContents)
    {
        const QStyleOptionProgressBar* progressOptions = qstyleoption_cast<const QStyleOptionProgressBar*>(option);
        if (progressOptions->progress == 0)
        {
            return;
        }
        // calculate progress width

        QRect rectPrBar = progressOptions->rect;
#ifdef Q_OS_WIN32
        rectPrBar = windowsClassicBug;
#endif
        int totalWidth = rectPrBar.width();
        rectPrBar.setWidth(((float)rectPrBar.width() / (float)100) * progressOptions->progress);

        //gradient
        QBrush brush(QColor("#50BD40"));
        QPen pen(QColor("#50BD40"));

        painter->save();
        // draw  progress
        painter->setBrush(brush);
        painter->setPen(pen);
        painter->setClipRect(rectPrBar.adjusted(0, 0, 1, 1));

        int width = rectPrBar.width();
        if (width <= 1)
        {
            painter->drawRoundedRect(rectPrBar.adjusted(0, 2, 0, -2) , ROUND_RADIUS, ROUND_RADIUS);
        }
        else if (width <= ROUND_RADIUS)
        {
            painter->drawPie(rectPrBar, 1440, 2880);
        }
        else if (width <= totalWidth - ROUND_RADIUS)
        {
            QPainterPath path;
            path.setFillRule(Qt::WindingFill);
            path.addRoundedRect(rectPrBar, ROUND_RADIUS, ROUND_RADIUS);
            path.addRect(QRect(rectPrBar.topLeft() + QPoint(ROUND_RADIUS, 0), rectPrBar.bottomRight()));
            painter->drawPath(path.simplified());
        }
        else
        {
            painter->drawRoundedRect(rectPrBar , ROUND_RADIUS, ROUND_RADIUS);
        }

        painter->restore();
    }
    else if (element == QStyle::CE_ProgressBarGroove)
    {
        painter->save();
        QRect rctGroove = option->rect;

#ifdef Q_OS_WIN32
        windowsClassicBug = rctGroove;
#endif

        QColor color = (option->state & QStyle::State_Selected) ? QColor("#ffffff") : QColor("#e5e5e5");

        QPen pen = QPen(color);
        QBrush brush = QBrush(color);
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawRoundedRect(rctGroove, ROUND_RADIUS, ROUND_RADIUS);
        painter->restore();
    }
    else if (element == QStyle::CE_ProgressBarLabel)
    {
        const QStyleOptionProgressBar* progressOptions = qstyleoption_cast<const QStyleOptionProgressBar*>(option);

        painter->save();
        QFont font = painter->font();
#ifdef Q_OS_WIN
        font.setPointSize(8);
#else
        font.setPointSize(10);
#endif //Q_OS_WIN
        painter->setFont(font);
        QPen pen = QPen(QColor(Qt::black));
        painter->setPen(pen);
        painter->drawText(progressOptions->rect, Qt::AlignCenter, progressOptions->text);
        painter->restore();
    }
    else
    {
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}

void LIIIStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    // we don't draw focus frame
    if (element == QStyle::PE_FrameFocusRect)
    {
        return;
    }

    // we don't draw progress bar chunks
    if (element == QStyle::PE_IndicatorProgressChunk)
    {
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

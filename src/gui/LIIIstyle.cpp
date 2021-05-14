#include "LIIIstyle.h"
#include <QStyleOption>
#include <QPainter>
#include <QComboBox>
#include <QAbstractItemView>

#ifdef Q_OS_WIN
enum { ROUND_RADIUS = 3 };
#else
enum { ROUND_RADIUS = 7 };
#endif //Q_OS_WIN

namespace {

#ifdef Q_OS_WIN
QRect windowsClassicBug;
#endif

} // namespace

void LIIIStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    switch (element)
    {
    case QStyle::CE_ProgressBarContents:
    {
        const auto* progressOptions = qstyleoption_cast<const QStyleOptionProgressBar*>(option);
        if (progressOptions->progress == 0)
        {
            return;
        }
        // calculate progress width

        QRect rectPrBar = progressOptions->rect;
#ifdef Q_OS_WIN
        rectPrBar = windowsClassicBug;
#endif
        const int totalWidth = rectPrBar.width();
        rectPrBar.setWidth((totalWidth * progressOptions->progress) / 100);

        //gradient
        QColor green(0x50BD40);
        QBrush brush(green);
        QPen pen(green);

        painter->save();
        // draw  progress
        painter->setBrush(brush);
        painter->setPen(pen);
        painter->setClipRect(rectPrBar.adjusted(0, 0, 1, 1));

        int width = rectPrBar.width();
        if (width <= 1)
        {
            painter->drawRoundedRect(rectPrBar.adjusted(0, 2, 0, -2), ROUND_RADIUS, ROUND_RADIUS);
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
            painter->drawRoundedRect(rectPrBar, ROUND_RADIUS, ROUND_RADIUS);
        }

        painter->restore();
        break;
    }
    case QStyle::CE_ProgressBarGroove:
    {
        painter->save();
        QRect rctGroove = option->rect;
#ifdef Q_OS_WIN
        windowsClassicBug = rctGroove;
#endif
        QColor color = (option->state & QStyle::State_Selected) 
            ? QColor(Qt::white) : QColor(0xe5e5e5);

        QPen pen(color);
        QBrush brush(color);
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawRoundedRect(rctGroove, ROUND_RADIUS, ROUND_RADIUS);
        painter->restore();
        break;
    }
    case QStyle::CE_ProgressBarLabel:
    {
        const auto* progressOptions = qstyleoption_cast<const QStyleOptionProgressBar*>(option);

        painter->save();
        QPen pen(Qt::black);
        painter->setPen(pen);
        painter->drawText(progressOptions->rect, Qt::AlignCenter, progressOptions->text);
        painter->restore();
        break;
    }
    default:
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}

void LIIIStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    // we don't draw focus frame, progress bar chunks
    if (element != QStyle::PE_FrameFocusRect && element != QStyle::PE_IndicatorProgressChunk)
    {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}

// https://stackoverflow.com/questions/20554940/qcombobox-pop-up-expanding-and-qtwebkit/20909625#20909625
int LIIIStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    if (hint == QStyle::SH_ComboBox_Popup) 
    {
        auto combo = static_cast<const QComboBox *>(widget);
        QAbstractItemView* view = nullptr;
        for (auto child : combo->children())
        {
            for (auto v : child->children())
            {
                view = qobject_cast<QAbstractItemView*>(v);
                if (view)
                {
                    break;
                }
            }

            if (view) 
            {
                break;
            }
        }

        if (view) 
        {
            const int iconSize = combo->iconSize().width();
            const QFontMetrics fontMetrics1 = view->fontMetrics();
            const QFontMetrics fontMetrics2 = combo->fontMetrics();
            const int j = combo->count();
            int width = combo->width(); //default width

            for (int i = 0; i < j; ++i) {
                const int textWidth = qMax(
                    fontMetrics1.width(combo->itemText(i) + "WW"),
                    fontMetrics2.width(combo->itemText(i) + "WW")
                );

                if (combo->itemIcon(i).isNull()) {
                    width = qMax(width, textWidth);
                }
                else {
                    width = qMax(width, textWidth + iconSize);
                }
            }

            view->setFixedWidth(width);
        }
    }

    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

#pragma once

#include <QProxyStyle>


class LIIIStyle
    : public QProxyStyle
{
    Q_OBJECT

public:
    LIIIStyle();

    void drawControl(ControlElement element,    const QStyleOption* option,    QPainter* painter,    const QWidget* widget) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const override;
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const override;
    QSize sizeFromContents(ContentsType ct, const QStyleOption* opt, const QSize & csz, const QWidget* widget = 0) const override;
};

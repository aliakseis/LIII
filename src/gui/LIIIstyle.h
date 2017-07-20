#pragma once

#include <QProxyStyle>


class LIIIStyle
    : public QProxyStyle
{
    Q_OBJECT

public:
    LIIIStyle() {};
    ~LIIIStyle() {};

    void drawControl(ControlElement element,    const QStyleOption* option,    QPainter* painter,    const QWidget* widget) const;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};

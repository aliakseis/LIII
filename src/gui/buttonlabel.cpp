#include "buttonlabel.h"
#include <QBoxLayout>
#include <QEvent>
#include <QMouseEvent>

#include <QDebug>


ButtonLabel::ButtonLabel(QWidget* parent)
    : QLabel(parent)
    , m_state(sNormal)
{
}

void ButtonLabel::mouseDoubleClickEvent(QMouseEvent* event)
{
    if ((m_state != sDisabled)
            && (event->button() == Qt::LeftButton))
    {
        emit doubleClicked();
    }
    QLabel::mouseDoubleClickEvent(event);
}

void ButtonLabel::mousePressEvent(QMouseEvent* event)
{
    if ((m_state != sDisabled)
            && (event->button() == Qt::LeftButton))
    {
        m_state = sPressed;
        setStyleSheet(styleSheet());
    }
    QLabel::mousePressEvent(event);
}

void ButtonLabel::mouseReleaseEvent(QMouseEvent* event)
{
    if ((m_state == sPressed)
            && (event->button() == Qt::LeftButton)
            && rect().contains(event->pos()))
    {
        m_state = sHovered;
        setStyleSheet(styleSheet());
        emit clicked();
    }
    else if (m_state != sDisabled)
    {
        m_state = sNormal;
        setStyleSheet(styleSheet());
    }
    QLabel::mouseReleaseEvent(event);
}

void ButtonLabel::enterEvent(QEvent* event)
{
    if (m_state != sDisabled)
    {
        m_state = sHovered;
        setStyleSheet(styleSheet());
    }
    QLabel::enterEvent(event);
}

void ButtonLabel::leaveEvent(QEvent* event)
{
    if (m_state == sHovered)
    {
        m_state = sNormal;
        setStyleSheet(styleSheet());
    }
    QLabel::leaveEvent(event);
}

void ButtonLabel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        m_state = isEnabled() ? sNormal : sDisabled;
        setStyleSheet(styleSheet());
    }
    QLabel::changeEvent(event);
}

QString ButtonLabel::getStateString()
{
    switch (m_state)
    {
    case sHovered: return "hovered";
    case sPressed: return "pressed";
    case sDisabled: return "disabled";
    default: return "normal";
    }
}

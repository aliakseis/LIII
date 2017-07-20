#pragma once

#include <QLabel>

class ButtonLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QString state READ getStateString())

public:
    explicit ButtonLabel(QWidget* parent = 0);
    virtual QString getStateString();

protected:
    enum State
    {
        sNormal,
        sHovered,
        sPressed,
        sDisabled,
    };

    void mouseDoubleClickEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void changeEvent(QEvent* event);

    State m_state;

signals:
    void doubleClicked();
    void clicked();
};

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

    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void changeEvent(QEvent* event) override;

    State m_state;

signals:
    void doubleClicked();
    void clicked();
};

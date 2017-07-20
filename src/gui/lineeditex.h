#pragma once

#include <QLineEdit>

class LineEditEx: public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(QString state READ getStateString())

public:
    explicit LineEditEx(QWidget* parent = 0);
    QString getStateString();
    void setErrorState();
    void setNormalState();

Q_SIGNALS:
    void linksAdd(bool);
public Q_SLOTS:
    void on_linkAdd(const QString&);
protected:
    enum State
    {
        sNormal,
        sError,
    };

    void focusInEvent(QFocusEvent* event);
    virtual void keyPressEvent(QKeyEvent*);

    State m_state;
};

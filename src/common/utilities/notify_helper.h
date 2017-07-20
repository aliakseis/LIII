#pragma once

#include <QObject>

class NotifyHelper : public QObject
{
    Q_OBJECT
public:
    NotifyHelper(QObject* parent = 0) : QObject(parent) {}

public slots:
    virtual void slotNoParams() {}
};

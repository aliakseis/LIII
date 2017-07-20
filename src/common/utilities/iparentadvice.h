#pragma once

class QObject;

struct IParentAdvice // analogy to COM IConnectionPoint::Advise()
{
    virtual void setAdviceParent(QObject* parent) = 0;
};

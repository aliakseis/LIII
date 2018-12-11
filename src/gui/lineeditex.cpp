#include "lineeditex.h"
#include "utilities/utils.h"
#include <QTimer>
#include <QKeyEvent>

LineEditEx::LineEditEx(QWidget* parent) :
    QLineEdit(parent) ,
    m_state(sNormal)
{
    VERIFY(connect(this, SIGNAL(textChanged(QString)), SLOT(on_linkAdd(QString))));
}

void LineEditEx::on_linkAdd(const QString& val)
{
    setNormalState();
    emit linksAdd(!val.isEmpty());
}

void LineEditEx::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        setText(QString());
    }
    QLineEdit::keyPressEvent(e);
}

void LineEditEx::setErrorState()
{
    m_state =  sError;
    setText(QString());

    setStyleSheet(styleSheet());
}

void LineEditEx::setNormalState()
{
    m_state =  sNormal;
    setStyleSheet(styleSheet());
}

void LineEditEx::focusInEvent(QFocusEvent* event)
{
    setNormalState();
    QLineEdit::focusInEvent(event);
}

QString LineEditEx::getStateString()
{
    return (m_state == sError) ? "error" : "normal";
}

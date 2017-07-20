#pragma once

#include <QComboBox>

namespace ui_utils
{

class TabbedDialogCombo : public QComboBox
{
    Q_OBJECT
public:
    explicit TabbedDialogCombo(QWidget* parent = 0);

    virtual bool eventFilter(QObject* receiver, QEvent* event) override;
    virtual void showPopup();

private:
    QTabWidget* getTabParent();
};

}

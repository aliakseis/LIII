#pragma once

#include <QComboBox>

namespace ui_utils
{

class TabbedDialogCombo : public QComboBox
{
    Q_OBJECT
public:
    explicit TabbedDialogCombo(QWidget* parent = 0);

    bool eventFilter(QObject* receiver, QEvent* event) override;
    void showPopup() override;

private:
    QTabWidget* getTabParent();
};

}

#pragma once

#include <QDialog>

namespace Ui
{
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT
    QString res;
public:
    AboutDialog(QWidget* parent);
    ~AboutDialog();
private:
    Ui::AboutDialog* m_ui;
};

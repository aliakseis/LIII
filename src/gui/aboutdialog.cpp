#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "settings_declaration.h"
#include "globals.h"
#include "version.hxx"

using utilities::Tr::SetTr;

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent), m_ui(new Ui::AboutDialog)
{
    m_ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    SetTr(this, &QWidget::setWindowTitle, ABOUT_TITLE, QString(PROJECT_FULLNAME));
    SetTr(m_ui->lBrand, &QLabel::setText, PROJECT_FULLNAME);
}

AboutDialog::~AboutDialog()
{
    delete m_ui;
}

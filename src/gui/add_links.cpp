#include "add_links.h"
#include "ui_add_links.h"

#include <algorithm>

#include <QCheckBox>
#include <QListWidgetItem>
#include <QUrl>
#include <QComboBox>
#include <QPainter>

#include "globals.h"


namespace
{
const char* defaultPriority = "High";
}

ComboboxDelegate::ComboboxDelegate(QObject* parent)
    : BaseClass(parent)
{
}

void ComboboxDelegate::paint(QPainter* painter,
                             const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    BaseClass::paint(painter, option, index);
    if (option.state & QStyle::State_Enabled)
    {
        QString text = index.data(Qt::DisplayRole).toString();
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);

        QStyleOptionComboBox box;
        box.palette = option.palette;
        box.rect = option.rect;
        box.state = option.state;
        box.currentText = text;
        box.frame = false;

        QRect rect = QApplication::style()->subControlRect(QStyle::CC_ComboBox, &box, QStyle::SC_ComboBoxArrow, opt.widget);
        opt.rect = rect;
        QApplication::style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, painter, opt.widget);
    }
}

QWidget* ComboboxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const
{
    auto* box = new QComboBox(parent);
    box->addItems(QStringList()  << defaultPriority << "Medium" << "Low");
    return box;
}

void ComboboxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    if (auto* comboBox = qobject_cast<QComboBox*>(editor))
    {
        comboBox->setCurrentIndex(comboBox->findText(value));
    }
    else
    {
        BaseClass::setEditorData(editor, index);
    }
}

void ComboboxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                    const QModelIndex& index) const
{
    if (auto* comboBox = qobject_cast<QComboBox*>(editor))
    {
        model->setData(index, comboBox->currentText(), Qt::EditRole);
    }
    else
    {
        BaseClass::setModelData(editor, model, index);
    }
}


AddLinks::AddLinks(const QStringList& urls, QWidget* parent)
    : QDialog(parent), ui(new Ui::AddLinks)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    ui->selectAllButton->setEnabled(false);

    for (const QString& url : urls)
    {
        addUrlToList(url);
    }

    setBaseSize(minimumSizeHint());
    auto* delegate = new ComboboxDelegate(ui->listWidget);
    ui->listWidget->setItemDelegateForColumn(2, delegate);

    const int count = ui->listWidget->topLevelItemCount();
    for (int i = 0; i < count; ++i)
    {
        if (QTreeWidgetItem* item = ui->listWidget->topLevelItem(i))
        {
            ui->listWidget->openPersistentEditor(item, 2);
        }
    }
}

AddLinks::~AddLinks()
{
    delete ui;
}

QStringList AddLinks::urls() const
{
    QStringList res;
    for (int i = 0; i < ui->listWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = ui->listWidget->topLevelItem(i);
        if (item && item->checkState(0) == Qt::Checked)
        {
            res.push_back(item->text(1));
        }
    }
    return res;
}

void AddLinks::on_selectAllButton_clicked()
{
    selectAll(true);
}

void AddLinks::on_selectNoneButton_clicked()
{
    selectAll(false);
}

void AddLinks::on_listWidget_itemClicked(QTreeWidgetItem* item)
{
    updateSelectButtons();
}

void AddLinks::addUrlToList(const QString& url)
{
    if (ui->listWidget->findItems(url, Qt::MatchExactly).isEmpty())
    {
        auto* item = new QTreeWidgetItem(ui->listWidget);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEditable | item->flags());
        item->setCheckState(0, Qt::Checked);
        item->setText(1, url);
        item->setText(2, defaultPriority);
    }
}

void AddLinks::selectAll(bool select)
{
    for (int i = 0; i < ui->listWidget->topLevelItemCount(); ++i)
    {
        if (QTreeWidgetItem* item = ui->listWidget->topLevelItem(i))
        {
            item->setCheckState(0, select ? Qt::Checked : Qt::Unchecked);
        }
    }
    updateSelectButtons();
}

void AddLinks::updateSelectButtons()
{
    bool selectAll = false;
    bool selectNone = false;
    for (int i = 0; i < ui->listWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = ui->listWidget->topLevelItem(i);
        if (item && item->checkState(0) == Qt::Checked)
        {
            selectNone = true;
        }
        else
        {
            selectAll = true;
        }
    }
    ui->selectAllButton->setEnabled(selectAll);
    ui->selectNoneButton->setEnabled(selectNone);
}
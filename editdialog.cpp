#include "editdialog.h"
#include "ui_editdialog.h"
#include <QDebug>

EditDialog::EditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDialog)
{
    ui->setupUi(this);
    mType = tr("String");

    mBtnGroup.addButton(ui->jsonButton, 1);
    mBtnGroup.addButton(ui->xmlButton, 2);
    mBtnGroup.setExclusive(true);

    connect(&mBtnGroup, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),[=](int id, bool checked){
            qDebug() << "id : " << id << " is checked: " << checked;
            if(checked){
                if(id == 1){
                    ui->comboBox->setEnabled(true);
                }else if(id == 2){
                    ui->comboBox->setEnabled(false);
                }
            }
    });

    connect(ui->comboBox, static_cast<void (QComboBox::*)(const QString& text)>(&QComboBox::currentIndexChanged), [=](const QString& text){
        mType = text;
        if(mType == tr("Number")){
            ui->valueLineEdit->setValidator(&mValidator);
            ui->valueLineEdit->clear();
        }
        else
            ui->valueLineEdit->setValidator(0);
    });

    connect(ui->sureButton, &QPushButton::clicked, [=](){
        this->accept();
    });

    connect(ui->cancelButton, &QPushButton::clicked, [=](){
        this->reject();
    });
}

EditDialog::~EditDialog()
{
    delete ui;
}

QString EditDialog::getType()
{
    return mType;
}

QString EditDialog::getKey()
{
    return ui->keyLineEdit->text();
}

QVariant EditDialog::getValue()
{
    auto text = ui->valueLineEdit->text();
    return QVariant(text);
}

int EditDialog::getCount()
{
    return ui->spinBox->value();
}

MainWindow::Data_Type EditDialog::getDataType()
{
    Q_ASSERT(mBtnGroup.checkedId() != -1);
    return static_cast<MainWindow::Data_Type>(mBtnGroup.checkedId());
}

QStringList EditDialog::getValueList()  //格式：key 0, type 1, value 2, count 3
{
    QStringList vl;
    vl << getKey() << mType;

    if(mType == tr("Bool"))
        vl << (getValue().toBool() ? tr("true") : tr("false"));
    else if(mType == tr("Null"))
        vl << tr("null");
    else
        vl << getValue().toString();

    vl << QString::number(getCount());
    return vl;
}



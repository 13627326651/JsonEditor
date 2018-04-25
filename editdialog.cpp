#include "editdialog.h"
#include "ui_editdialog.h"
#include <QDebug>

EditDialog::EditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDialog),
    mType(Type_String)
{
    ui->setupUi(this);

    connect(ui->comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index){
        Q_ASSERT(index >= 0 && index < 6);
        mType = static_cast<JsonType>(index);

        if(mType == Type_Number){
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

EditDialog::JsonType EditDialog::getType()
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


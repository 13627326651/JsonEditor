#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include <QDialog>
#include <QDoubleValidator>
#include <QButtonGroup>
#include "mainwindow.h"

namespace Ui {
class EditDialog;
}

class EditDialog : public QDialog
{
    Q_OBJECT
public:
//    enum JsonType{

//        Type_String = 0,
//        Type_Number,
//        Type_Bool,
//        Type_Null,
//        Type_Object,
//        Type_Array
//    };
    explicit EditDialog(QWidget *parent = 0);
    ~EditDialog();
    QString getType();
    QString getKey();
    QVariant getValue();
    int getCount();
    MainWindow::Data_Type getDataType();
    QStringList getValueList();
private:
    Ui::EditDialog *ui;

    QString mType;
    QDoubleValidator mValidator;
    QButtonGroup mBtnGroup;
};

#endif // EDITDIALOG_H

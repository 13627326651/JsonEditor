#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include <QDialog>
#include <QDoubleValidator>

namespace Ui {
class EditDialog;
}

class EditDialog : public QDialog
{
    Q_OBJECT
public:
    enum JsonType{

        Type_String = 0,
        Type_Number,
        Type_Bool,
        Type_Null,
        Type_Object,
        Type_Array
    };
    explicit EditDialog(QWidget *parent = 0);
    ~EditDialog();
    JsonType getType();
    QString getKey();
    QVariant getValue();
    int getCount();
private:
    Ui::EditDialog *ui;

    JsonType mType;
    QDoubleValidator mValidator;
};

#endif // EDITDIALOG_H

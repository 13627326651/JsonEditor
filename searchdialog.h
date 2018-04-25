#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>

namespace Ui {
class SearchDialog;
}

class MainWindow;
class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent);
    ~SearchDialog();
    void findNext(bool isCircle = true);
private:
    Ui::SearchDialog *ui;

    MainWindow* mMainWindow;
    int mCurrentIndex;
};

#endif // SEARCHDIALOG_H

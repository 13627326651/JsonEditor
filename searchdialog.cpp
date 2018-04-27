#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "mainwindow.h"
#include <QDebug>

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
    ui->setupUi(this);
    Q_ASSERT(mMainWindow = dynamic_cast<MainWindow*>(parent));

    connect(ui->comboBox, &QComboBox::editTextChanged, [=](){
        mCurrentIndex = 0;
    });

    connect(ui->nextButton, &QPushButton::clicked, [=](){
        auto searchText = ui->comboBox->currentText();
        if(searchText.isEmpty())
            return;

        auto& items = mMainWindow->searchString(searchText, ui->radioButton->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
        if(items.isEmpty()){
            ui->statusLabel->setText(tr("未找到匹配串： %1").arg(searchText));
            return;
        }
        ui->statusLabel->clear();
        findNext(true);
    });

    connect(ui->countButton, &QPushButton::clicked, [=](){
        auto searchText = ui->comboBox->currentText();
        if(searchText.isEmpty()) return;
        auto& items = mMainWindow->searchString(searchText, ui->radioButton->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
        ui->statusLabel->setText(tr("计数： %1次匹配").arg(items.count()));
    });
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::findNext(bool isCircle)
{
    auto& items = mMainWindow->getSearchResult();
    if(isCircle && mCurrentIndex >= items.count())
        mCurrentIndex = 0;
    if(mCurrentIndex >= 0 && mCurrentIndex < items.count()){        
        mMainWindow->setCurrentItem(items.at(mCurrentIndex++));
    }
}

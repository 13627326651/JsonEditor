#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "searchdialog.h"
#include "editdialog.h"
#include <QFileInfo>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include "jsonhandler.h"
#include "xmlhandler.h"

using namespace rapidjson;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mSearchDialog(nullptr),
    mEditDialog(nullptr),
    mType(Type_Unknow)
{
    ui->setupUi(this);
    setWindowTitle(tr("JsonEditor"));

    //设置treeWidget
    ui->treeWidget->setColumnCount(3);
    ui->treeWidget->setHeaderLabels(QStringList() << tr("Name") << tr("Type") << tr("Value"));
    //设置动作图标
    ui->actionOpen->setIcon(QIcon(":/open.png"));
    ui->actionSaveAs->setIcon(QIcon(":/save.png"));
    ui->actionNewItem->setIcon(QIcon(":/add.png"));
    ui->actionClearAll->setIcon(QIcon(":/delete.png"));
    ui->actionFind->setIcon(QIcon(":/search.png"));
    ui->actionDeleteItem->setIcon(QIcon(":/remove.png"));
    ui->actionRefresh->setIcon(QIcon(":/refresh.png"));
    ui->actionBack->setIcon(QIcon(":/back.png"));
    ui->actionCopy->setIcon(QIcon(":/copy.png"));
    ui->actionPaste->setIcon(QIcon(":/paste.png"));
    //添加到工具栏
    ui->mainToolBar->addAction(ui->actionBack);
    ui->mainToolBar->addAction(ui->actionRefresh);
    ui->mainToolBar->addAction(ui->actionFind);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(ui->actionClearAll);
    //打开文件，保存至文件
    connect(ui->actionOpen, &QAction::triggered,[=](){
        auto fileName = QFileDialog::getOpenFileName(this, tr("Open File"), qApp->applicationDirPath(), tr("JSON (*.json *.xml)"));
        this->parseFromFile(fileName);
    });
    connect(ui->actionSaveAs, &QAction::triggered, [=](){
        auto fileName = QFileDialog::getSaveFileName(this, tr("Save File"), qApp->applicationDirPath(), tr("JSON (*.json *.xml)"));
        this->saveAs(fileName);
    });

    //搜索，清空，刷新，回退，复制，粘贴
    connect(ui->actionFind, &QAction::triggered, [=](){
        if(!mSearchDialog) mSearchDialog = new SearchDialog(this);
        mSearchDialog->show();
    });
    connect(ui->actionClearAll, &QAction::triggered, [=](){
        clearWidget();
    });
    connect(ui->actionRefresh, &QAction::triggered, [=](){
        this->refresh();
    });
    connect(ui->actionBack, &QAction::triggered, [=](){
        this->goBack();
    });
    connect(ui->actionCopy, &QAction::triggered, [=](){
        this->copy();
    });
    connect(ui->actionPaste, &QAction::triggered, [=](){
        this->paste(QApplication::clipboard()->text());
    });

    //打开添加新项的ui，删除单项
    connect(ui->actionNewItem, &QAction::triggered, [=](){
        if(!mEditDialog) mEditDialog = new EditDialog(this);

        if(mEditDialog->exec()){
            if(ui->treeWidget->currentItem())
                this->addItem(ui->treeWidget->currentItem());
            else
                this->addItem(ui->treeWidget->invisibleRootItem());
        }
    });
    connect(ui->actionDeleteItem, &QAction::triggered, [=](){
        auto item = ui->treeWidget->currentItem();
        if(item){
            deleteOneItem(item);
            if(ui->treeWidget->invisibleRootItem()->childCount() <= 0)
                mType = Type_Unknow;
        }
    });

    //双击后可以部分编辑
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, [=](QTreeWidgetItem* item, int column){
        //不是Type列，类型不是Object，Array的可以编辑
        if(column != 1)
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable);
        else
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    });
    //设置右键菜单策略
    this->setContextMenuPolicy(Qt::DefaultContextMenu);
}

MainWindow::~MainWindow()
{
    delete ui;
}
///////////需要更新 1
void MainWindow::parseFromFile(const QString &fileName)
{    
    QFileInfo fInfo(fileName);
    if(fInfo.completeSuffix() == tr("json")){
        mType = Type_Json;
        JsonHandler h(ui->treeWidget->invisibleRootItem());
        qDebug() << "parse json file state : " << h.parseFromFile(fileName);
    }else if( fInfo.completeSuffix() == tr("xml")){
        mType = Type_Xml;
        XmlHandler h(ui->treeWidget->invisibleRootItem());
        qDebug() << "parse xml file state : " <<  h.parseFromFile(fileName);
    }
}

void MainWindow::execSearch(const QString &pattern, QTreeWidgetItem* parent, Qt::CaseSensitivity cs)
{
    if(!parent){
        parent = ui->treeWidget->invisibleRootItem();
        mSearchItems.clear();
    }
    for(int i = 0; i < parent->childCount(); ++i){
        auto item = parent->child(i);
        if(item->text(0).contains(pattern, cs) || item->text(1).contains(pattern, cs) || item->text(2).contains(pattern, cs))
            mSearchItems << item;
        execSearch(pattern, item);
    }
}

void MainWindow::setCurrentItem(QTreeWidgetItem *currentItem)
{
    ui->treeWidget->setCurrentItem(currentItem);
}

QList<QTreeWidgetItem *>& MainWindow::searchString(const QString &pattern, Qt::CaseSensitivity cs = Qt::CaseInsensitive)
{
    if(mSearchText == pattern)
        return mSearchItems;
    execSearch(pattern, ui->treeWidget->invisibleRootItem(), cs);
    mSearchText = pattern;
    return mSearchItems;
}

QList<QTreeWidgetItem *>& MainWindow::getSearchResult()
{
    return mSearchItems;
}

///////////需要更新 2
//从EditDialog获取信息添加到某项
void MainWindow::addItem(QTreeWidgetItem *parent)
{
    if(mEditDialog->getDataType() == Type_Json){
        if(mType == Type_Xml) return;
        mType = Type_Json;
        if(parent == ui->treeWidget->invisibleRootItem())
            parent->setText(1, tr("Object")); //确保子项添加到object对象下
        JsonHandler h(parent);
        h.addChild(mEditDialog->getValueList());
    }else if(mEditDialog->getDataType() == Type_Xml){
        if(mType == Type_Json) return;
        mType = Type_Xml;
        XmlHandler h(parent);
        h.addChild(mEditDialog->getValueList());
    }
}

///////////需要更新 3
//保存至文件
void MainWindow::saveAs(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::critical(this, tr("Error"), tr("Open file error."));
        return;
    }
    QFileInfo fInfo(file);
    if(fInfo.completeSuffix() == tr("json")){
        JsonHandler h(ui->treeWidget->invisibleRootItem());
        file.write(h.stringifyToString().toUtf8());
    }else if(fInfo.completeSuffix() == tr("xml")){
        //XmlHandler h(ui->treeWidget->invisibleRootItem());
        //file.write(h.stringifyToString().toUtf8().date());
    }
    file.close();
}

///////////需要更新 4
//刷新
void MainWindow::refresh()
{
    if(mType == Type_Json){
        JsonHandler h(ui->treeWidget->invisibleRootItem());
        h.parseFromString(h.stringifyToString());
    }else if(mType == Type_Xml){
        //XmlHandler h(ui->treeWidget->invisibleRootItem());
        //h.parseFromString(h.stringifyToString());
    }
}

//清理界面
void MainWindow::clearWidget()
{
    //删除根节点所有孩子
    auto rootItem = ui->treeWidget->invisibleRootItem();
    removeItemChildren(rootItem);
    //清空搜索结果
    mSearchItems.clear();
    mSearchText.clear();
    //设置数据类型未定义
    mType = Type_Unknow;
}

//删除指定的项
void MainWindow::deleteOneItem(QTreeWidgetItem *item)
{
    Q_ASSERT(item);
    removeItemChildren(item);
    delete item;
}

void MainWindow::goBack()
{
//    if(mLastJson.isEmpty()) return;

//    Document d;
//    d.Parse(mLastJson.toUtf8().data());
//    if(d.HasParseError()){
//        qDebug() << "goBack pase error";
//        return;
//    }
//    clearWidget();
//    parseFromOneNode(ui->treeWidget->invisibleRootItem(), d);
}

///////////需要更新 5
void MainWindow::copy()
{
    if(mType == Type_Json){
        JsonHandler h(ui->treeWidget->invisibleRootItem());
        QApplication::clipboard()->setText(h.stringifyToString());
    }else if(mType == Type_Xml){
        //XmlHandler h(ui->treeWidget->invisibleRootItem());
        //QApplication::clipboard()->setText(h.stringifyToString());
    }
}

///////////需要更新 6
//粘贴
void MainWindow::paste(QString text)
{    
    text = text.trimmed();
    if(text.startsWith(QChar('{'))){
        JsonHandler h(ui->treeWidget->invisibleRootItem());
        if(h.parseFromString(text))
            mType = Type_Json;
    }else if(text.startsWith(QChar('<'))){
        XmlHandler h(ui->treeWidget->invisibleRootItem());
        if(h.parseFromString(text))
            mType = Type_Xml;
    }
}

//去除某项的所有孩子
void MainWindow::removeItemChildren(QTreeWidgetItem *item)
{
    auto children = item->takeChildren();
    for(auto& child : children){
        removeItemChildren(child);
    }
    qDeleteAll(children);
}

//右键菜单
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    auto point = ui->treeWidget->viewport()->mapFromGlobal(QCursor::pos());
    ui->treeWidget->setCurrentItem(ui->treeWidget->itemAt(point));

    QMenu *menu = new QMenu();
    menu->addAction(ui->actionNewItem);
    menu->addAction(ui->actionDeleteItem);
    menu->addAction(ui->actionCopy);
    menu->addAction(ui->actionPaste);
    menu->addAction(ui->actionFind);
    menu->addSeparator();
    menu->addAction(ui->actionClearAll);
    menu->exec(QCursor::pos());
    delete menu;

    QMainWindow::contextMenuEvent(event);
}

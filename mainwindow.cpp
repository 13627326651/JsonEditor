#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "searchdialog.h"
#include "editdialog.h"
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

using namespace rapidjson;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mSearchDialog(nullptr),
    mEditDialog(nullptr)
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
        auto fileName = QFileDialog::getOpenFileName(this, tr("Open File"), qApp->applicationDirPath(), tr("JSON (*.json)"));
        if(fileName.isEmpty()) return;
        parseFromFile(fileName);
    });
    connect(ui->actionSaveAs, &QAction::triggered, [=](){
        auto fileName = QFileDialog::getSaveFileName(this, tr("Save File"), qApp->applicationDirPath(), tr("JSON (*.json)"));
        if(fileName.isEmpty()) return;
        saveAs(fileName);
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
        if(item) deleteFromItem(item);
    });

    //双击后可以部分编辑
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, [=](QTreeWidgetItem* item, int column){
        //不是Type列，类型不是Object，Array的可以编辑
        if(column != 1 && item->text(1) != tr("Object") && item->text(1) != tr("Array"))
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

void MainWindow::parseFromFile(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::critical(this, "Error", "Open file " + fileName + " error!");
        return;
    }
    Document d;
    d.Parse(file.readAll().data());
    file.close();

    if(d.GetParseError()){
        QMessageBox::critical(this, "Error", "Parse " + fileName + " error!");
        return;
    }
    clearWidget();
    parseFromOneNode(ui->treeWidget->invisibleRootItem(), d);

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    d.Accept(writer);
    mLastJson = tr(buffer.GetString());
}

void MainWindow::parseFromOneNode(QTreeWidgetItem *parent, Value &node)
{
    if(node.IsObject()){
        for(auto itr = node.MemberBegin(); itr != node.MemberEnd(); ++itr){
            QStringList sl;
            sl << QString(itr->name.GetString());
            if(itr->value.IsObject()){
                sl << tr("Object");
                auto item = new QTreeWidgetItem(parent, sl);
                parseFromOneNode(item, itr->value);
            }else if(itr->value.IsArray()){
                sl << tr("Array");
                auto item = new QTreeWidgetItem(parent, sl);
                parseFromOneNode(item, itr->value);
            }else if(itr->value.IsString()){
                sl << tr("String") << tr(itr->value.GetString());
                new QTreeWidgetItem(parent, sl);
            }else if(itr->value.IsBool()){
                sl << tr("Bool") << tr(itr->value.IsTrue() ? "true" : "false");
                new QTreeWidgetItem(parent, sl);
            }else if(itr->value.IsNull()){
                sl << tr("Null") << tr("null");
                new QTreeWidgetItem(parent, sl);
            }else if(itr->value.IsNumber()){
                sl << tr("Number");
                if(itr->value.IsInt()){
                    sl << tr("%1").arg(itr->value.GetInt());
                }else{
                    sl << tr("%1").arg(itr->value.GetDouble());
                }
                new QTreeWidgetItem(parent, sl);
            }
        }
    }else if(node.IsArray()){
        for(SizeType i = 0; i < node.Size(); ++i){
            QStringList sl;
            if(node[i].IsArray()){
                sl << tr("%1").arg(i) << tr("Array");
                auto item = new QTreeWidgetItem(parent, sl);
                parseFromOneNode(item, node[i]);
            }else if(node[i].IsObject()){
                sl << tr("%1").arg(i) << tr("Object");
                auto item = new QTreeWidgetItem(parent, sl);
                parseFromOneNode(item, node[i]);
            }else if(node[i].IsString()){
                sl << tr("%1").arg(i) << tr("String") << tr(node[i].GetString());
                new QTreeWidgetItem(parent, sl);
            }else if(node[i].IsBool()){
                sl << tr("%1").arg(i) << tr("Bool") << tr(node[i].IsTrue() ? "true" : "false");
                new QTreeWidgetItem(parent, sl);
            }else if(node[i].IsNull()){
                sl << tr("%1").arg(i) << tr("Null") << tr("null");
                new QTreeWidgetItem(parent, sl);
            }else if(node[i].IsNumber()){                
                sl << tr("%1").arg(i) << tr("Number");
                if(node[i].IsInt()){
                    sl << tr("%1").arg( node[i].GetInt());
                }else{
                    sl << tr("%1").arg( node[i].GetDouble());
                }
                new QTreeWidgetItem(parent, sl);
            }
        }
    }
}

void MainWindow::execSearch(const QString &pattern, QTreeWidgetItem* parent)
{
    if(!parent){
        parent = ui->treeWidget->invisibleRootItem();
        mSearchItems.clear();
    }

    for(int i = 0; i < parent->childCount(); ++i){
        auto item = parent->child(i);
        if(item->text(0).contains(pattern) || item->text(2).contains(pattern)){
            mSearchItems << item;
        }
        execSearch(pattern, item);
    }
}

void MainWindow::setCurrentItem(QTreeWidgetItem *currentItem)
{
    ui->treeWidget->setCurrentItem(currentItem);
}

QList<QTreeWidgetItem *>& MainWindow::searchString(const QString &pattern)
{
    if(mSearchText == pattern)
        return mSearchItems;
    execSearch(pattern);
    mSearchText = pattern;
    return mSearchItems;
}

QList<QTreeWidgetItem *>& MainWindow::getSearchResult()
{
    return mSearchItems;
}

void MainWindow::addItem(QTreeWidgetItem *parent)
{
    auto typeStr = parent->text(1);
    if(parent == ui->treeWidget->invisibleRootItem() || typeStr == tr("Array") || typeStr == tr("Object")){
        auto count = mEditDialog->getCount();
        auto type = mEditDialog->getType();
        switch(type){
        case EditDialog::Type_String:
            for(int i = 0; i < count; ++i)
                parent->addChild(new QTreeWidgetItem(QStringList() << mEditDialog->getKey() << tr("String") << mEditDialog->getValue().toString()));
            break;
        case EditDialog::Type_Number:
            for(int i = 0; i < count; ++i)
                parent->addChild(new QTreeWidgetItem(QStringList() << mEditDialog->getKey() << tr("Number") << mEditDialog->getValue().toString()));
            break;
        case EditDialog::Type_Bool:
            for(int i = 0; i < count; ++i)
                parent->addChild(new QTreeWidgetItem(QStringList() << mEditDialog->getKey() << tr("Bool") << (mEditDialog->getValue().toBool() ? tr("true") : tr("false"))));
            break;
        case EditDialog::Type_Null:
            for(int i = 0; i < count; ++i)
                parent->addChild(new QTreeWidgetItem(QStringList() << mEditDialog->getKey() << tr("Null") << tr("null")));
            break;
        case EditDialog::Type_Object:
            for(int i = 0; i < count; ++i)
                parent->addChild(new QTreeWidgetItem(QStringList() << mEditDialog->getKey() << tr("Object")));
            break;
        case EditDialog::Type_Array:
            for(int i = 0; i < count; ++i)
                parent->addChild(new QTreeWidgetItem(QStringList() << mEditDialog->getKey() << tr("Array")));
            break;
        default:
            break;
        }
    }
}

void MainWindow::saveAs(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::critical(this, tr("Error"), tr("Open file error."));
        return;
    }
    Document d;
    d.SetObject();
    toDocument(ui->treeWidget->invisibleRootItem(), d, d);

    StringBuffer buff;
    PrettyWriter<StringBuffer> writer(buff);
    d.Accept(writer);
    file.write(buff.GetString());
    file.close();
}

void MainWindow::refresh()
{
    Document d;
    d.SetObject();
    toDocument(ui->treeWidget->invisibleRootItem(), d, d);

    clearWidget();

    parseFromOneNode(ui->treeWidget->invisibleRootItem(), d);

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    d.Accept(writer);
    mLastJson = tr(buffer.GetString());
}

void MainWindow::clearWidget()
{
    auto rootItem = ui->treeWidget->invisibleRootItem();
    deleteFromItem(rootItem);
    ui->treeWidget->clear();

    mSearchItems.clear();
    mSearchText.clear();
}

void MainWindow::goBack()
{
    if(mLastJson.isEmpty()) return;

    Document d;
    d.Parse(mLastJson.toUtf8().data());
    if(d.HasParseError()){
        qDebug() << "goBack pase error";
        return;
    }
    clearWidget();
    parseFromOneNode(ui->treeWidget->invisibleRootItem(), d);
}

void MainWindow::copy()
{
    Document d;
    d.SetObject();
    toDocument(ui->treeWidget->invisibleRootItem(), d, d);

    StringBuffer buff;
    PrettyWriter<StringBuffer> writer(buff);
    d.Accept(writer);

    auto board = QApplication::clipboard();
    board->setText(QString(buff.GetString()));
}

void MainWindow::paste(QString text)
{
    if(text.isEmpty()) return;

    Document d;
    d.Parse(text.toUtf8().data());

    if(d.HasParseError()){
        QMessageBox::critical(this, tr("Paste Error"), tr("Can not parse the text, please check the text it is correct."));
        return;
    }

    clearWidget();
    parseFromOneNode(ui->treeWidget->invisibleRootItem(), d);
}

void MainWindow::deleteFromItem(QTreeWidgetItem *item)
{
    if(item->childCount() > 0){
        for(int i = 0; i < item->childCount(); ++i){
            deleteFromItem(item->child(i));
        }
    }
    if(item != ui->treeWidget->invisibleRootItem())
        delete item;
}

void MainWindow::toDocument(QTreeWidgetItem *pItem, Value &pValue, Document &d)
{
    if(pValue.IsObject()){
        for(int i = 0; i < pItem->childCount(); ++i){
            auto child = pItem->child(i);
            if(child->text(1) == tr("String")){
                Value v, name;
                v.SetString(child->text(2).toUtf8().data(), d.GetAllocator());
                name.SetString(child->text(0).toUtf8().data(), d.GetAllocator());
                pValue.AddMember(name, v, d.GetAllocator());
            }else if(child->text(1) == tr("Bool")){
                Value v, name;
                v.SetBool(QVariant(child->text(2)).toBool());
                name.SetString(child->text(0).toUtf8().data(), d.GetAllocator());
                pValue.AddMember(name, v, d.GetAllocator());
            }else if(child->text(1) == tr("Number")){
                Value v, name;
                bool ok;
                int r = child->text(2).toInt(&ok);
                if(ok){
                    v.SetInt(r);
                }else
                    v.SetDouble(child->text(2).toDouble());
                name.SetString(child->text(0).toUtf8().data(), d.GetAllocator());
                pValue.AddMember(name, v, d.GetAllocator());
            }else if(child->text(1) == tr("Null")){
                Value v, name;
                v.SetNull();
                name.SetString(child->text(0).toUtf8().data(), d.GetAllocator());
                pValue.AddMember(name, v, d.GetAllocator());
            }else if(child->text(1) == tr("Array")){
                Value v, name;
                v.SetArray();
                toDocument(child, v, d);
                name.SetString(child->text(0).toUtf8().data(), d.GetAllocator());
                pValue.AddMember(name, v, d.GetAllocator());
            }else if(child->text(1) == tr("Object")){
                Value v, name;
                v.SetObject();
                toDocument(child, v, d);
                name.SetString(child->text(0).toUtf8().data(), d.GetAllocator());
                pValue.AddMember(name, v, d.GetAllocator());
            }
        }
    }else if(pValue.IsArray()){
        for(int i = 0; i < pItem->childCount(); ++i){
            auto child = pItem->child(i);
            if(child->text(1) == tr("String")){
                Value v;
                v.SetString(child->text(2).toUtf8().data(), d.GetAllocator());
                pValue.PushBack(v, d.GetAllocator());
            }else if(child->text(1) == tr("Bool")){
                Value v;
                v.SetBool(QVariant(child->text(2)).toBool());
                pValue.PushBack(v, d.GetAllocator());
            }else if(child->text(1) == tr("Number")){
                Value v;
                bool ok;
                int r = child->text(2).toInt(&ok);
                if(ok)
                    v.SetInt(r);
                else
                    v.SetDouble(child->text(2).toDouble());
                pValue.PushBack(v, d.GetAllocator());
            }else if(child->text(1) == tr("Null")){
                Value v;
                v.SetNull();
                pValue.PushBack(v, d.GetAllocator());
            }else if(child->text(1) == tr("Array")){
                Value v;
                v.SetArray();
                toDocument(child, v, d);
                pValue.PushBack(v, d.GetAllocator());
            }else if(child->text(1) == tr("Object")){
                Value v;
                v.SetObject();
                toDocument(child, v, d);
                pValue.PushBack(v, d.GetAllocator());
            }
        }
    }
}

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

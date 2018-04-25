#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <rapidjson/document.h>


namespace Ui {
class MainWindow;
}


class EditDialog;
class SearchDialog;
class QTreeWidgetItem;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void parseFromFile(const QString& fileName);
    QList<QTreeWidgetItem *>& searchString(const QString& pattern);
    void setCurrentItem(QTreeWidgetItem* currentItem);
    QList<QTreeWidgetItem*>& getSearchResult();

    void saveAs(const QString& fileName);
    void refresh();
    void clearWidget();
    void goBack();
    void copy();
    void paste(QString text);
protected:
    void addItem(QTreeWidgetItem* parent);
    void execSearch(const QString& pattern, QTreeWidgetItem *parent = nullptr);
    void parseFromOneNode(QTreeWidgetItem* parent, rapidjson::Value& node);
    void deleteFromItem(QTreeWidgetItem* item);
    void toDocument(QTreeWidgetItem* pItem, rapidjson::Value& pValue, rapidjson::Document &d);
    void contextMenuEvent(QContextMenuEvent* event);
private:
    Ui::MainWindow* ui;
    QString mLastJson;
    QString mFileName;

    SearchDialog* mSearchDialog;
    QList<QTreeWidgetItem*> mSearchItems;
    QString mSearchText;

    EditDialog *mEditDialog;
};

#endif // MAINWINDOW_H

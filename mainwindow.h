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
    enum Data_Type{
        Type_Unknow,
        Type_Json,
        Type_Xml
    };

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void parseFromFile(const QString& fileName);
    QList<QTreeWidgetItem *>& searchString(const QString& pattern, Qt::CaseSensitivity cs);
    void setCurrentItem(QTreeWidgetItem* currentItem);
    QList<QTreeWidgetItem*>& getSearchResult();

    void saveAs(const QString& fileName);
    void refresh();
    void clearWidget();
    void deleteOneItem(QTreeWidgetItem* item);
    void goBack();
    void copy();
    void paste(QString text);
protected:
    void addItem(QTreeWidgetItem* parent);
    void execSearch(const QString& pattern, QTreeWidgetItem *parent = nullptr, Qt::CaseSensitivity cs = Qt::CaseInsensitive);
    void removeItemChildren(QTreeWidgetItem* item);
    void contextMenuEvent(QContextMenuEvent* event);
private:
    Ui::MainWindow* ui;
    QString mLastJson;
    QString mFileName;

    SearchDialog* mSearchDialog;
    QList<QTreeWidgetItem*> mSearchItems;
    QString mSearchText;

    EditDialog *mEditDialog;
    Data_Type mType;
};

#endif // MAINWINDOW_H

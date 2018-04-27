#ifndef HANDLER_H
#define HANDLER_H
#include <QObject>

class QTreeWidgetItem;
class Handler{
public:
    Handler(QTreeWidgetItem* item = nullptr);
    ~Handler();
    void setTreeWidgetItem(QTreeWidgetItem* item);
    QTreeWidgetItem* getTreeWidgetItem();

    virtual void addChild(const QStringList& strs);
    virtual bool parseFromFile(const QString& fileName);		//从文件中显示
    virtual bool parseFromString(const QString& str);		//从给定字符串显示
    virtual QString stringifyToString();	//将显示转为QString


protected:
    void removeItemChildren(QTreeWidgetItem *item);

    QTreeWidgetItem* mItem;
};
#endif // HANDLER_H

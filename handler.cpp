#include "handler.h"
#include <QTreeWidgetItem>


Handler::Handler(QTreeWidgetItem *item) : mItem(item)
{

}

Handler::~Handler()
{

}

void Handler::setTreeWidgetItem(QTreeWidgetItem *item)
{
    mItem = item;
}

QTreeWidgetItem *Handler::getTreeWidgetItem()
{
    return mItem;
}

void Handler::addChild(const QStringList &strs)
{

}


bool Handler::parseFromFile(const QString &fileName)
{
    return false;
}

bool Handler::parseFromString(const QString &str)
{
    return false;
}

QString Handler::stringifyToString()
{
    return QString();
}

void Handler::removeItemChildren(QTreeWidgetItem* item)
{
    Q_ASSERT(item);
    auto children = item->takeChildren();
    for(auto& child : children){
        removeItemChildren(child);
    }
    qDeleteAll(children);
    children.clear();
}

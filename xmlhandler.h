#ifndef XMLHANDLER_H
#define XMLHANDLER_H

#include <QObject>
#include "handler.h"
#include "tinyxml2.h"

class QTreeWidgetItem;
class XmlHandler : public Handler
{
public:
    XmlHandler(QTreeWidgetItem* item = nullptr);

    virtual void addChild(const QStringList& strs);
    virtual bool parseFromFile(const QString& fileName);		//从文件中显示
    virtual bool parseFromString(const QString& str);		//从给定字符串显示
    virtual QString stringifyToString();	//将显示转为QString

    void parseFromOneNode(QTreeWidgetItem* parent, tinyxml2::XMLElement *element);
};

#endif // XMLHANDLER_H

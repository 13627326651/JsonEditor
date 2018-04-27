#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include "handler.h"
#include <rapidjson/document.h>

class JsonHandler : public Handler
{
public:
    JsonHandler(QTreeWidgetItem* item = nullptr);
    virtual ~JsonHandler();
    virtual void addChild(const QStringList &strs);
    virtual bool parseFromFile(const QString& fileName);		//从文件中显示
    virtual bool parseFromString(const QString& str);		//从给定字符串显示
    virtual QString stringifyToString();

private:
    void parseFromOneNode(QTreeWidgetItem *parent, rapidjson::Value &node);
    void toDocument(QTreeWidgetItem *pItem, rapidjson::Value &pValue, rapidjson::Document &d);
};

#endif // JSONHANDLER_H

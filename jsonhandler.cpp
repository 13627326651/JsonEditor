#include "jsonhandler.h"
#include <QFile>
#include <QTreeWidgetItem>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

using namespace rapidjson;
JsonHandler::JsonHandler(QTreeWidgetItem *item) : Handler(item)
{

}

JsonHandler::~JsonHandler()
{

}

void JsonHandler::addChild(const QStringList &strs)     //key 0, type 1, value 2,count 3
{
    Q_ASSERT(mItem);
    auto typeStr = mItem->text(1);
    if(typeStr == QObject::tr("Array") || typeStr == QObject::tr("Object")){
        auto count = strs.value(3).toInt();
        for(int i = 0; i < count; ++i)
            mItem->addChild(new QTreeWidgetItem(strs));
    }
}

bool JsonHandler::parseFromFile(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) return false;
    QString str(file.readAll());
    file.close();
    return parseFromString(str);
}

bool JsonHandler::parseFromString(const QString &str)
{
    Document d;
    d.Parse(str.toUtf8().data());
    if(d.GetParseError()) return false;
    removeItemChildren(mItem);
    parseFromOneNode(mItem, d);
    return true;
}

QString JsonHandler::stringifyToString()
{
    Document d;
    d.SetObject();
    toDocument(mItem, d, d);

    StringBuffer buff;
    PrettyWriter<StringBuffer> writer(buff);
    d.Accept(writer);
    return QString(buff.GetString());
}

void JsonHandler::parseFromOneNode(QTreeWidgetItem *parent, Value &node)
{
    Q_ASSERT(parent);
    if(node.IsObject()){
        for(auto itr = node.MemberBegin(); itr != node.MemberEnd(); ++itr){
            QStringList sl;
            sl << QString(itr->name.GetString());
            if(itr->value.IsObject()){
                sl << QObject::tr("Object");
                auto item = new QTreeWidgetItem(parent, sl);
                parseFromOneNode(item, itr->value);
            }else if(itr->value.IsArray()){
                sl << QObject::tr("Array");
                auto item = new QTreeWidgetItem(parent, sl);
                parseFromOneNode(item, itr->value);
            }else if(itr->value.IsString()){
                sl << QObject::tr("String") << QObject::tr(itr->value.GetString());
                new QTreeWidgetItem(parent, sl);
            }else if(itr->value.IsBool()){
                sl << QObject::tr("Bool") << QObject::tr(itr->value.IsTrue() ? "true" : "false");
                new QTreeWidgetItem(parent, sl);
            }else if(itr->value.IsNull()){
                sl << QObject::tr("Null") << QObject::tr("null");
                new QTreeWidgetItem(parent, sl);
            }else if(itr->value.IsNumber()){
                sl << QObject::tr("Number");
                if(itr->value.IsInt()){
                    sl << QObject::tr("%1").arg(itr->value.GetInt());
                }else{
                    sl << QObject::tr("%1").arg(itr->value.GetDouble());
                }
                new QTreeWidgetItem(parent, sl);
            }
        }
    }else if(node.IsArray()){
        for(SizeType i = 0; i < node.Size(); ++i){
            QStringList sl;
            if(node[i].IsArray()){
                sl << QObject::tr("%1").arg(i) << QObject::tr("Array");
                auto item = new QTreeWidgetItem(parent, sl);
                parseFromOneNode(item, node[i]);
            }else if(node[i].IsObject()){
                sl << QObject::tr("%1").arg(i) << QObject::tr("Object");
                auto item = new QTreeWidgetItem(parent, sl);
                parseFromOneNode(item, node[i]);
            }else if(node[i].IsString()){
                sl << QObject::tr("%1").arg(i) << QObject::tr("String") << QObject::tr(node[i].GetString());
                new QTreeWidgetItem(parent, sl);
            }else if(node[i].IsBool()){
                sl << QObject::tr("%1").arg(i) << QObject::tr("Bool") << QObject::tr(node[i].IsTrue() ? "true" : "false");
                new QTreeWidgetItem(parent, sl);
            }else if(node[i].IsNull()){
                sl << QObject::tr("%1").arg(i) << QObject::tr("Null") << QObject::tr("null");
                new QTreeWidgetItem(parent, sl);
            }else if(node[i].IsNumber()){
                sl << QObject::tr("%1").arg(i) << QObject::tr("Number");
                if(node[i].IsInt()){
                    sl << QObject::tr("%1").arg( node[i].GetInt());
                }else{
                    sl << QObject::tr("%1").arg( node[i].GetDouble());
                }
                new QTreeWidgetItem(parent, sl);
            }
        }
    }
}

void JsonHandler::toDocument(QTreeWidgetItem *pItem, Value &pValue, Document &d)
{
    Q_ASSERT(mItem);
    if(pValue.IsObject()){
        for(int i = 0; i < pItem->childCount(); ++i){
            auto child = pItem->child(i);
            if(child->text(1) == QObject::tr("String")){
                Value v, name;
                v.SetString(child->text(2).toUtf8().data(), d.GetAllocator());
                name.SetString(child->text(0).toUtf8().data(), d.GetAllocator());
                pValue.AddMember(name, v, d.GetAllocator());
            }else if(child->text(1) == QObject::tr("Bool")){
                Value v, name;
                v.SetBool(QVariant(child->text(2)).toBool());
                name.SetString(child->text(0).toUtf8().data(), d.GetAllocator());
                pValue.AddMember(name, v, d.GetAllocator());
            }else if(child->text(1) == QObject::tr("Number")){
                Value v, name;
                bool ok;
                int r = child->text(2).toInt(&ok);
                if(ok){
                    v.SetInt(r);
                }else
                    v.SetDouble(child->text(2).toDouble());
                name.SetString(child->text(0).toUtf8().data(), d.GetAllocator());
                pValue.AddMember(name, v, d.GetAllocator());
            }else if(child->text(1) == QObject::tr("Null")){
                Value v, name;
                v.SetNull();
                name.SetString(child->text(0).toUtf8().data(), d.GetAllocator());
                pValue.AddMember(name, v, d.GetAllocator());
            }else if(child->text(1) == QObject::tr("Array")){
                Value v, name;
                v.SetArray();
                toDocument(child, v, d);
                name.SetString(child->text(0).toUtf8().data(), d.GetAllocator());
                pValue.AddMember(name, v, d.GetAllocator());
            }else if(child->text(1) == QObject::tr("Object")){
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
            if(child->text(1) == QObject::tr("String")){
                Value v;
                v.SetString(child->text(2).toUtf8().data(), d.GetAllocator());
                pValue.PushBack(v, d.GetAllocator());
            }else if(child->text(1) == QObject::tr("Bool")){
                Value v;
                v.SetBool(QVariant(child->text(2)).toBool());
                pValue.PushBack(v, d.GetAllocator());
            }else if(child->text(1) == QObject::tr("Number")){
                Value v;
                bool ok;
                int r = child->text(2).toInt(&ok);
                if(ok)
                    v.SetInt(r);
                else
                    v.SetDouble(child->text(2).toDouble());
                pValue.PushBack(v, d.GetAllocator());
            }else if(child->text(1) == QObject::tr("Null")){
                Value v;
                v.SetNull();
                pValue.PushBack(v, d.GetAllocator());
            }else if(child->text(1) == QObject::tr("Array")){
                Value v;
                v.SetArray();
                toDocument(child, v, d);
                pValue.PushBack(v, d.GetAllocator());
            }else if(child->text(1) == QObject::tr("Object")){
                Value v;
                v.SetObject();
                toDocument(child, v, d);
                pValue.PushBack(v, d.GetAllocator());
            }
        }
    }
}

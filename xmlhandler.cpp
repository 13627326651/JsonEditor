#include "xmlhandler.h"
#include "tinyxml2.h"
#include <QDebug>
#include <QTreeWidgetItem>
using namespace tinyxml2;
/// tinyxml2 主要接口类
/// XMLNode
/// XMLDeclaration : XMLNode
/// XMLComment : XMLNode
/// XMLDocument : XMLNode
/// XMLElement : XMLNode
/// XMLUnknow : XMLNode
/// XMLText : XMLNode


XmlHandler::XmlHandler(QTreeWidgetItem *item) : Handler(item)
{

}

void XmlHandler::addChild(const QStringList &strs)
{
    Q_ASSERT(mItem);
    QStringList sl;
    sl << strs.value(0) << " " << strs.value(2);
    for(int i = 0; i < strs.value(3).toInt(); ++i)
        mItem->addChild(new QTreeWidgetItem(sl));
}

bool XmlHandler::parseFromFile(const QString &fileName)
{
    Q_ASSERT(mItem);
    XMLDocument d;
    if(d.LoadFile(fileName.toUtf8().data()))
        return false;

    XMLElement* root = d.RootElement();
    parseFromOneNode(mItem, root);
    //    while(node != nullptr){
    //        qDebug() << node->Name();

    //        if(node->FirstChildElement())
    //            node = node->FirstChildElement();
    //        else{
    //            auto nextNode = node->NextSiblingElement();
    //            if(!nextNode){
    //                auto parent = dynamic_cast<XMLElement*>(node->Parent());
    //                while(parent){
    //                    if(!parent->NextSiblingElement())
    //                        parent = dynamic_cast<XMLElement*>(parent->Parent());
    //                    else{
    //                        parent = parent->NextSiblingElement();
    //                        break;
    //                    }
    //                }
    //                node = parent;
    //            }
    //            else
    //                node = nextNode;
    //        }
    //    }
    return true;
}



bool XmlHandler::parseFromString(const QString &str)
{
    XMLDocument d;
    if(d.Parse(str.toUtf8().data()))
        return false;

    XMLElement* root = d.RootElement();
    parseFromOneNode(mItem, root);
    return true;
}

QString XmlHandler::stringifyToString()
{
    return QString();
}

void XmlHandler::parseFromOneNode(QTreeWidgetItem *parent, XMLElement* element)
{
    while(element){
        QStringList vl;
        vl << element->Name();
        auto text = element->GetText();
        if(text){
            vl << " " << text;
        }
        auto child = new QTreeWidgetItem(vl);
        parent->addChild(child);
        parseFromOneNode(child, element->FirstChildElement());
        element = element->NextSiblingElement();
    }
}

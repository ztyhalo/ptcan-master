#ifndef PTXML_H
#define PTXML_H

#include "QVector"
#include "xmlprocess.h"

#include <QFile>

using namespace std;


#define EXT_EXIT 0
#define OUT_EXIT 1

#define PT_XML_T_CLASS  <class Dri, class Dev, class Ext, class INode, class ONode,\
    class TDri, class TDev,class TExt, class TINode, class TONode>

template <class Dev, class Ext, class INode, class ONode>
class dev_info
{
public:
    Dev para;            //参数
#if EXT_EXIT == 1
    QVector<Ext>  ext;
#endif
    QVector<INode> inode;
    QVector<ONode> onode;
    QVector<dev_info> child;
};

template <class Dri, class Dev, class Ext, class INode, class ONode,\
            class TDri, class TDev,class TExt, class TINode, class TONode>
class driver_info
{
public:
    QString name;
    Dri dri_info;
    int parentid;
    QVector<dev_info<Dev,Ext,INode, ONode> > dev;

    int driver_write_xml(QString name);
    int driver_read_xml(QString name);

    int continue_write_dev(QVector<dev_info<Dev,Ext,INode, ONode> > devpra, QDomDocument doc, QDomElement father);
    int continue_read_dev(QVector<dev_info<Dev,Ext,INode, ONode> > &devpra, QDomElement father);
};

template <class Dri, class Dev, class Ext, class INode, class ONode,\
          class TDri, class TDev,class TExt, class TINode, class TONode>
int driver_info<Dri,Dev,Ext,INode, ONode,TDri,TDev,TExt,TINode, TONode>::\
continue_write_dev(QVector<dev_info<Dev,Ext,INode, ONode> > devpra, QDomDocument doc, QDomElement father)
{
    if(devpra.size() > 0)
    {
        QDomElement root = doc.createElement("DevList");
        father.appendChild(root);

        for(int i = 0; i < devpra.size(); i++)
        {
            dev_info<Dev,Ext,INode,ONode> mid = devpra[i];
            QDomElement subdev = doc.createElement("SubDev");
            root.appendChild(subdev);

            XMLProcess<TDev, Dev>().write_xml_doc(doc, subdev,"DevPara", mid.para);
#if EXT_EXIT == 1
            XMLProcess<TExt, Ext>().write_xml_doc(doc, subdev,"Extral", mid.ext);
#endif
            XMLProcess<TINode, INode>().write_xml_doc(doc, subdev,"InNodePara", mid.inode);
            XMLProcess<TONode, ONode>().write_xml_doc(doc, subdev,"OutNodePara", mid.inode);

            if(mid.child.empty() == false)
            {
                 continue_write_dev(mid.child, doc,  subdev);
            }
        }
    }
    return 0;
}

template <class Dri, class Dev, class Ext, class INode, class ONode,\
          class TDri, class TDev,class TExt, class TINode, class TONode>
int driver_info<Dri,Dev,Ext,INode, ONode,TDri,TDev,TExt,TINode, TONode>::driver_write_xml(QString name)
{

    int node = 0;
    QFile file(name);

    if(file.exists())
    {
        file.remove();
        QFile file(name);
    }
    if(!file.open(QIODevice::WriteOnly))
        return false;

    QDomDocument doc;
    QDomElement root = doc.createElement("lh");

    doc.appendChild(root);

    XMLProcess<TDri, Dri>().write_xml_doc(doc,  root, "BusPara", dri_info);
    node++;

    continue_write_dev(dev,  doc,  root);

    node += dev.size();

    QTextStream out(&file);

    doc.save(out, node);
    return 0;

}

template <class Dri, class Dev, class Ext, class INode, class ONode,\
          class TDri, class TDev,class TExt, class TINode, class TONode>
int driver_info<Dri,Dev,Ext,INode, ONode,TDri,TDev,TExt,TINode, TONode>::\
continue_read_dev(QVector<dev_info<Dev,Ext, INode, ONode> > &devvec,QDomElement father)
{
    QDomNode sub_dev = father.firstChild();
    int vectorid = 0;
    while(!sub_dev.isNull())
    {
        QDomElement element = sub_dev.toElement();
        QDomNode devinfo = element.firstChild();
        dev_info<Dev, Ext, INode, ONode> devmid;

        devvec.push_back(devmid);
        while(!devinfo.isNull())
        {
            QDomElement devpara = devinfo.toElement();
            if(devpara.tagName() == "DevPara")
            {
              QDomElement devmidpara = devpara.firstChildElement();
              ParaseToType<TDev>().read_xml_data(&devvec[vectorid].para, devmidpara);
            }
#if EXT_EXIT == 1
              else if(devpara.tagName() == "Extral")
              {
                  QDomNode devnode = devpara.firstChild();
                  while(!devnode.isNull())
                  {
                      QDomElement devmidpara = devnode.toElement();
                      Ext extralmid;
                      ParaseToType<TExt>().read_xml_data(&extralmid, devmidpara);

                      devvec[vectorid].ext.append(extralmid);
                      devnode = devnode.nextSibling();
                  }
              }
#endif
              else if(devpara.tagName() == "InNodePara")
              {
                  QDomNode devnode = devpara.firstChild();
                  while(!devnode.isNull())
                  {
                      QDomElement devmidpara = devnode.toElement();
                      INode nodemid;
                      ParaseToType<TINode>().read_xml_data(&nodemid, devmidpara);
                      devvec[vectorid].inode.append(nodemid);
                      devnode = devnode.nextSibling();
                  }
              }
              else if(devpara.tagName() == "OutNodePara")
              {
                  QDomNode devnode = devpara.firstChild();
                  while(!devnode.isNull())
                  {
                      QDomElement devmidpara = devnode.toElement();
                      ONode nodemid;
                      ParaseToType<TONode>().read_xml_data(&nodemid, devmidpara);
                      devvec[vectorid].onode.append(nodemid);
                      devnode = devnode.nextSibling();
                  }
              }
              else if(devpara.tagName() == "DevList")
              {
                  continue_read_dev(devvec[vectorid].child, devpara);
              }

              devinfo = devinfo.nextSibling();
         }
        vectorid++;
        sub_dev = sub_dev.nextSibling();
    }
    return 0;
}

template <class Dri, class Dev, class Ext, class INode, class ONode,\
          class TDri, class TDev,class TExt, class TINode, class TONode>
int driver_info<Dri,Dev,Ext,INode, ONode,TDri,TDev,TExt,TINode, TONode>::driver_read_xml(QString name)
{
    QDomDocument doc;

    QFile file(name);

    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!doc.setContent(&file, true, &errorStr, &errorLine,
                        &errorColumn)) {
        qWarning("Line %d, column %d: %s", errorLine, errorColumn,
                 errorStr.toLatin1().data());
        return FALSE;
    }
     QDomElement doc_root = doc.documentElement();
     QDomNode doc_node = doc_root.firstChild();
     while (!doc_node.isNull())
     {
         QDomElement element;
         element = doc_node.toElement();

         if ((element.tagName() == "BusPara")&&(parentid == 0))
         {
             QDomElement dripara = element.firstChildElement();
             ParaseToType<TDri>().read_xml_data(&dri_info, dripara);
         }
         else if(element.tagName() == "DevList")
         {
            dev_info<Dev, Ext, INode, ONode> devtemp;
            dev.push_back(devtemp);
            continue_read_dev(dev[parentid].child,element);
         }
         doc_node = doc_node.nextSibling();
     }
     return 0;
}


template <class Dri, class TDri>
class Driver_Para
{
public:
    QString name;
    Dri dri_info;

    int read_xml_dri_para(QString name, QString para);
};
template <class Dri, class TDri>
int Driver_Para<Dri,TDri>::read_xml_dri_para(QString name, QString para)
{
    QDomDocument doc;

    QFile file(name);

    QString errorStr;
    int errorLine;
    int errorColumn;


    if (!doc.setContent(&file, true, &errorStr, &errorLine,
                        &errorColumn)) {
        qWarning("Line %d, column %d: %s", errorLine, errorColumn,
                 errorStr.toLatin1().data());
        return FALSE;
    }

     QDomElement doc_root = doc.documentElement();


     QDomNodeList midlist = doc_root.elementsByTagName(para);
     if(midlist.size() == 1)
     {
         QDomElement element = midlist.at(0).toElement();

         QDomElement dripara = element.firstChildElement();
         ParaseToType<TDri>().read_xml_data(&dri_info, dripara);
         return 0;
     }
     return -1;
}

#endif /*PTXML_H*/

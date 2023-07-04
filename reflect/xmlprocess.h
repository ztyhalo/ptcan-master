#ifndef XMLPROCESS_H
#define XMLPROCESS_H
#include "reflect.h"

template<typename T1, typename T2 >
class XMLProcess:public ParaseToType<T1>
{
public:

     bool write_xml_attribute(QDomDocument doc,  QDomElement element, T2 t)
     {
        QDomAttr attr;

        QDomElement userElement = doc.createElement("lh");

        for(auto iter = T1::fieldinfo.begin(); iter != T1::fieldinfo.end(); iter++)
        {

            if ((*iter).type.compare(typeid(int).name()) == 0)
            {
                attr = doc.createAttribute((*iter).name.c_str());
                int * th = (int *)(((byte*)&t) + (*iter).offset);

                attr.setValue(QString("%1").arg(*th));
                userElement.setAttributeNode(attr);


            }
            else if ((*iter).type.compare(typeid(uint).name()) == 0)
            {
                attr = doc.createAttribute((*iter).name.c_str());
                uint * th = (uint *)(((byte*)&t) + (*iter).offset);

                attr.setValue(QString("%1").arg(*th));
                userElement.setAttributeNode(attr);


            }
            else if ((*iter).type.compare(typeid(double).name()) == 0)
            {

                attr = doc.createAttribute((*iter).name.c_str());
                double * th = (double *)(((byte*)&t) + (*iter).offset);

                attr.setValue(QString("%1").arg(*th));
                userElement.setAttributeNode(attr);
            }
            else if ((*iter).type.compare(typeid(string).name()) == 0)
            {
                attr = doc.createAttribute((*iter).name.c_str());
                byte* th = (((byte*)&t) + (*iter).offset);
                string* strmem = (string*)th;

                attr.setValue(strmem->c_str());
                userElement.setAttributeNode(attr);

            }
            else if((*iter).type.compare(typeid(short).name()) == 0)
            {
                attr = doc.createAttribute((*iter).name.c_str());
                short * th = (short *)(((byte*)&t) + (*iter).offset);

                attr.setValue(QString("%1").arg(*th));
                userElement.setAttributeNode(attr);
            }
            else if((*iter).type.compare(typeid(ushort).name()) == 0)
            {
                attr = doc.createAttribute((*iter).name.c_str());
                ushort * th = (ushort *)(((byte*)&t) + (*iter).offset);

                attr.setValue(QString("%1").arg(*th));
                userElement.setAttributeNode(attr);
            }
            else if((*iter).type.compare(typeid(int8_t).name()) == 0)
            {
                attr = doc.createAttribute((*iter).name.c_str());
                int8_t * th = (int8_t *)(((byte*)&t) + (*iter).offset);

                attr.setValue(QString("%1").arg(*th));
                userElement.setAttributeNode(attr);
            }
            else if((*iter).type.compare(typeid(uchar).name()) == 0)
            {
                attr = doc.createAttribute((*iter).name.c_str());
                uchar * th = (uchar *)(((byte*)&t) + (*iter).offset);

                attr.setValue(QString("%1").arg(*th));
                userElement.setAttributeNode(attr);
            }
            else if((*iter).type.compare(typeid(char).name()) == 0)
            {
                attr = doc.createAttribute((*iter).name.c_str());
                char * th = (char *)(((byte*)&t) + (*iter).offset);

                attr.setValue(QString("%1").arg(*th));
                userElement.setAttributeNode(attr);
            }
            else
            {
                /*
                 * Only support the following types
                 * int, double, std::string
                 */
                assert(false);
            }

        }
        element.appendChild(userElement);

        return true;
     }

    bool write_xml_doc(QDomDocument doc, QDomElement root, QString name, T2 t)
    {
        QDomElement element;


        element = doc.createElement(name);
        root.appendChild(element);

        write_xml_attribute(doc,  element, t);

        return true;

    }

    bool write_xml_doc(QDomDocument doc, QDomElement root, QString name, QVector<T2> val)
    {
        QDomElement element;


        element = doc.createElement(name);
        root.appendChild(element);


         for(int i = 0; i < val.size(); i++)
         {
            T2 t = val.at(i);
            write_xml_attribute( doc,  element, t);
         }

         return true;

    }
};


#endif // XMLPROCESS_H


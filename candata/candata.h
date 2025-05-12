/*
 *********************************************************************************************
 *                                      平台项目
 * File：
 * Module:
 * Version:
 * History:
 * ________________________
 * tk100+iocan通讯应用
 *********************************************************************************************
 */
#ifndef __CANDATA_H__
#define __CANDATA_H__

#include "can_protocol.h"
// #include "pro_data.h"
// #include "bitset"
// #include "reflect.h"
#include "driver.h"
#include "ptxml.h"
#include "candatainfo.h"
#include "1030pro.h"
#include "tk200pro.h"
#include "256app/256pro.h"
#include "zprint.h"


using namespace std;

#define CON_OUT_BUF_SIZE 256

#define CAN_XML_DATA driver_info< can_bus_para, can_dev_para, void, can_inode_info, can_onode_info, \
                                  can_bus_para_t, can_dev_para_t, void, can_inode_info_t, can_onode_info_t >

#define CAN_256XML_DATA driver_info< can_bus_para, can_dev_para, void, can_inode_info, can_onode_info, \
can_bus_para_t, can_256dev_para_t, void, can_inode_info_t, can_onode_info_t >

typedef void (*Pro_XML_Process)(void *pro, CAN_XML_DATA &xmldata);

class Can_Data : public PRO_DATA_INFO_T< void >, public Key_Info
{

public:
    int                    canid;
    int                    m_bitrate;
    CAN_XML_DATA           readxml;
    CAN_256XML_DATA        read256xml;
    sParamInfoType         d_info;
    vector< can_dev_para > dev;
    CanDriver             *canbus;
    ncan_protocol         *canpro;
    //    Pro_XML_Process      conf_func;
    cs_can    *cs1030;
    cs_can256 * cs256;
    TK200_Pro *tk200;

public:
    Can_Data( ):canid(0),m_bitrate(0),canbus(NULL),canpro(NULL),cs1030(NULL),cs256(NULL),tk200(NULL)
    {
        memset(&d_info, 0x00, sizeof(d_info));
    }
    ~Can_Data( )
    {
        if (cs1030 != NULL)
        {
            zprintf1("desotry 1030!\n");
            delete cs1030;
            zprintf1("destory 1030 over!\n");
            cs1030 = NULL;
        }
        if (cs256 != NULL)
        {
            zprintf1("desotry cs256!\n");
            delete cs256;
            zprintf1("destory cs256 over!\n");
            cs256 = NULL;
        }

        if (tk200 != NULL)
        {
            zprintf1("delete tk200!\n");
            delete tk200;
            tk200 = NULL;
            zprintf1("delete tk200 over!\n");
        }

        if (canpro != NULL)
        {
            delete canpro;
            canpro = NULL;
            zprintf1("delete canpro!\n");
        }
        if (canbus != NULL)
        {
            delete canbus;
            canbus = NULL;
            zprintf1("delete canbus!\n");
        }
    }
    void creat_can_bus_pro(void);
    int  can_read_xml(const QString  name, const QString  name1, const QString  name2);
    int  can_read_xml(const QString  name);
    int  can_app_init(void);
    int  get_innode_info(int dev, int child, int innode, can_inode_info &val);
};


#endif /*__CANDATA_H__*/

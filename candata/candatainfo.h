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
#ifndef __CANDATAINFO_H__
#define __CANDATAINFO_H__



// #include "can_protocol.h"
// #include "bitset"
#include "reflect.h"
// #include "driver.h"
#include "ptxml.h"
// #include "ptrwdatainfo.h"


using namespace std;

struct can_bus_para
{
    int    brate;         //速度(kbps),1000/800/500/250/125/100/50/25/10/5
    int    canid;         // id
    string name;          //名称
    string protocal;      //协议
    int    crctime;       //循环周期(ms)
    string frametype;     //帧类型
    string frameorder;    //帧格式
    int    auto_reset;    //是否自动复位
};

struct can_bus_para_t
{
    Declare_Struct(can_bus_para_t)

        Define_Field(1, int, brate)
            Define_Field(2, int, canid)
                Define_Field(3, string, name)
                    Define_Field(4, string, protocal)
                        Define_Field(5, int, crctime)
                            Define_Field(6, string, frametype)
                                Define_Field(7, string, frameorder)
                                    Define_Field(8, int, auto_reset)
                                        Define_Field(9, int, heart_ok)
                                            Define_Metadata(9)

                                                Define_Enum(1, 1, 1000000, 0)
                                                    Define_Enum(2, 1, 800000, 1)
                                                        Define_Enum(3, 1, 500000, 2)
                                                            Define_Enum(4, 1, 250000, 3)
                                                                Define_Enum(5, 1, 125000, 4)
                                                                    Define_Enum(6, 1, 100000, 5)
                                                                        Define_Enum(7, 1, 50000, 6)
                                                                            Define_Enum(8, 1, 25000, 7)
                                                                                Define_Enum(9, 1, 10000, 8)
                                                                                    Define_Enum(10, 1, 5000, 9)
                                                                                        Define_Enum(11, 6, standardframe, 0)
                                                                                            Define_Enum(12, 6, extandframe, 1)
                                                                                                Define_Enum(13, 7, dataframe, 0)
                                                                                                    Define_Enum(14, 7, remoteframe, 1)
                                                                                                        Define_Meta_Enumdata(14)
    //    Define_NoMeta_Enumdata
};

struct can_dev_para
{
    int    id;          //设备从机号
    string name;        //名称
    int    innum;       //输入个数
    int    outnum;      //输出个数
    int    polltime;    //查询周期,单位(ms)

    uint16_t type;
    uint16_t enable;
    uint16_t link_num;
    uint16_t auto_reset;    //
};

#define CAN_DEV_PARA_INIT(para) do {\
        para.id = 0;       \
        para.innum = 0;     \
        para.outnum = 0;    \
        para.polltime = 0;   \
        para.type = 0;\
        para.enable = 0;\
        para.link_num = 0;\
        para.auto_reset = 0;} while(0)

struct can_dev_para_t
{
    Declare_Struct(can_dev_para_t)
        Define_Field(1, int, id)
            Define_Field(2, string, name)
                Define_Field(3, int, innum)
                    Define_Field(4, int, outnum)
                        Define_Field(5, int, polltime)
                            Define_Enum_Field(6, uint16_t, type)
                                Define_Field(7, uint16_t, enable)
                                    Define_Field(8, uint16_t, link_num)
                                        Define_Field(9, uint16_t, auto_reset)
                                            Define_Metadata(9)

                                                Define_Enum(1, 6, DEV_256_IO_PHONE, 0)
                                                    Define_Enum(2, 6, DEV_256_MODBUS_LOCK, 1)
                                                        Define_Enum(3, 6, DEV_256_PHONE, 2)
                                                            Define_Enum(4, 6, DEV_256_LOCK, 3)
                                                                Define_Enum(5, 6, LOW_MACH, 4)
                                                                    Define_Enum(6, 6, TK236_IOModule_Salve, 5)
                                                                        Define_Enum(7, 6, TERMINAL, 6)
                                                                            Define_Enum(8, 6, IN_DEV, 7)
                                                                                Define_Enum(9, 6, DEV_256_RELAY, 8)
                                                                                    Define_Enum(10, 6, TK100_CSModule, 9)
                                                                                        Define_Enum(11, 6, TK200_IOModule, 10)
                                                                                            Define_Enum(12, 6, TK100_BS_Module, 11)
                                                                                                Define_Enum(13, 6, TK100_IOModule_IO, 12)
                                                                                                    Define_Enum(14, 6, TK200_CSModule, 13)
                                                                                                        Define_Enum(15, 6, TK200_LOWModule, 14)
                                                                                                            Define_Enum(16, 6, CS_DEV, 15)
                                                                                                                Define_Enum(17, 6, TK200_INModule, 16)
                                                                                                                    Define_Meta_Enumdata(17)
    //    Define_NoMeta_Enumdata
};

struct can_dev_extra_para
{
    int reserved;    //保留字段
};

struct can_dev_extra_para_t
{
    Declare_Struct(can_dev_extra_para_t)
        Define_Field(1, int, reserved)
            Define_Metadata(1)
                Define_NoMeta_Enumdata
};

struct can_inode_info
{
    int      nodeid;        //点的序列号
    int      datatype;      //点的数据类型(开关量常闭/开关量常开/频率量)
    uint16_t node_en;       //使能标志
    uint16_t shake_time;    //去抖时间
    uint16_t threshold_min;
    uint16_t threshold_max;
    uint8_t  notify_time_interval;
    uint8_t  notify_en;
    uint8_t  notify_range;
};

struct can_inode_info_t
{
    Declare_Struct(can_inode_info_t)
        Define_Field(1, int, nodeid)
            Define_Enum_Field(2, int, datatype)
                Define_Field(3, uint16_t, node_en)
                    Define_Field(4, uint16_t, shake_time)
                        Define_Field(5, uint16_t, threshold_min)
                            Define_Field(6, uint16_t, threshold_max)
                                Define_Field(7, uint8_t, notify_time_interval)
                                    Define_Field(8, uint8_t, notify_en)
                                        Define_Field(9, uint8_t, notify_range)
                                            Define_Metadata(9)
                                                Define_Enum(1, 2, switchno, 0)
                                                    Define_Enum(2, 2, switchnc, 1)
                                                        Define_Enum(3, 2, frequency, 2)
                                                            Define_Meta_Enumdata(3)
};
struct can_onode_info
{
    int      nodeid;       //点的序列号
    int      datatype;     //点的数据类型(开关量常闭/开关量常开/频率量)
    uint16_t node_en;      //使能标志
    uint16_t link_stop;    //连锁标志
};

struct can_onode_info_t
{
    Declare_Struct(can_onode_info_t)
        Define_Field(1, int, nodeid)
            Define_Enum_Field(2, int, datatype)
                Define_Field(3, uint16_t, node_en)
                    Define_Field(4, uint16_t, link_stop)
                        Define_Metadata(4)
                            Define_Enum(1, 2, switchno, 0)
                                Define_Enum(2, 2, switchnc, 1)
                                    Define_Enum(3, 2, frequency, 2)
                                        Define_Meta_Enumdata(3)
};




#define CAN_DEV_INFO dev_info< can_dev_para, void, can_inode_info, can_onode_info >






#endif /*__CANDATAINFO_H__*/

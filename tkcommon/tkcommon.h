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
#ifndef __TKCOMMON_H__
#define __TKCOMMON_H__

#include "can_protocol.h"
// #include "pro_data.h"
#include "bitset"
// #include "reflect.h"
// #include "driver.h"
// #include "ptxml.h"
#include "candatainfo.h"
// #include "msgtype.h"
#include "MsgMng.h"
#include "timers.h"
#include "mutex_class.h"
#include "ptrwdatainfo.h"

using namespace std;

#define TK_IO_IN(nodeid) (8 * ((nodeid) / 4) + (3 - ((nodeid) % 4)) * 2)

#define DEV_ID_MASK (~(0x000001f0))

#define SET_FRAM_DEV(FRAM, ID) ((FRAM)&DEV_ID_MASK) | ((uint32_t)(ID) << 4U)

#define TK100INPUT_BUF_SIZE 32
#define TK_IO_SATET_N       0x03

typedef enum
{
    DEV_256_IO_PHONE = 0X00,
    DEV_256_MODBUS_LOCK,
    DEV_256_PHONE,
    DEV_256_LOCK,
    LOW_MACH,
    TK236_IOModule_Salve,
    TERMINAL,
    IN_DEV,
    TK100_CSModule = 0x08,
    DEV_256_RELAY  = 0x09,
    TK200_IOModule,
    TK100_BS_Module,
    TK100_IOModule_IO,
    TK200_CSModule,
    TK200_LOWModule,
    CS_DEV,
    TK200_INModule,
    CS_DEVSTY_MAX
} CSDEVSTYLE;
enum
{
    TK_IO_BUG = 0,
    TK_IO_NORMAL,
    TK_IO_INIT
};
enum
{
    TK_SW_IN = 0,
    TK_FREQ_IN
};

enum
{
    DEV_OFF_LINE = 0,
    DEV_ON_LINE,
    DEV_NOT_CONFIG,
    DEV_CONFIGING,
    DEV_LOCK,
    DEV_STATE_MAX
};
/****************************************************************************************************
 IO CAN TX MESSAGE  ID
 ***************************************************************************************************/
enum
{
    IO_START_ACK_ID = 0x0450,       //上电认可 config parameter
    IO_OUTCONTR_ASK_ID,             //输出控制
    IO_INCHANGE_ACK_ID = 0x0457,    //输入口变化响应
    IO_DATA_ASK_ID     = 0x045f     //数据请求帧

};
/****************************************************************************************************
 tk IO CAN RX MESSAGE  ID
 ***************************************************************************************************/
enum
{
    IO_START_ASK_ID = 0x0650,    //上电认可帧

};
/****************************************************************************************************
 tk IO CAN TX MESSAGE
 ***************************************************************************************************/
enum
{
    IO_START_ACK = 0x0,
    IO_OUTCONTR_ASK,
    IO_DATA_ASK,
    IO_CHANGE_RESPOND,
    IO_TXMES_MAX
};

/****************************************************************************************************
 tk IO CAN RX MESSAGE
 ***************************************************************************************************/
enum
{
    IO_START_ASK = 0,       //上电认可信息请求
    IO_OUTCONTR_ACK,        //输出控制指令应答
    IO_DATA_ACK_ONE,        //数据请求帧应答1
    IO_DATA_ACK_TWO,        //数据请求帧应答帧2
    IO_DATA_ACK_THR,        //数据请求帧应答帧3
    IO_DATA_ACK_FOU = 5,    //数据请求帧应答帧4
    IO_CONFIG_ACK,          //上电认可及配置的应答
    IO_INCHANGE_ASK,        //输出口变化请求
    IO_RXMES_MAX

};
extern sem_t gTk200_Rest_Meg;
/****************************************************************************************************

 tk100+ funciton declare
 ***************************************************************************************************/

enum
{
    SENSOR_FAULT = 0,    //传感器故障
    SWITCH_BROKEN,       //开关量断路
    SWITCH_SHORT,        //开关量短路
    FREQ_LOW,            //频率量小于200
    FREQ_HIGHT,          //频率量大于1000
    NODE_OK = 7          //

};
typedef union
{
    uint16_t outinfo;
    struct
    {
        uint16_t swvale1 : 1;
        uint16_t swignor11 : 11;
        uint16_t swstate3 : 3;
        uint16_t instyle1 : 1;
    } swin;
    struct
    {
        uint16_t fvale12 : 12;
        uint16_t fstate3 : 3;
        uint16_t instyle1 : 1;
    } fin;

} TKINPUT;

typedef union
{
    uint16_t fram;
    struct
    {
        uint16_t func : 4;    // 功能码
        uint16_t addr : 5;    // 设备地址
        uint16_t dev : 2;     // 设备属性
    } fram_org;
} TK_FRAMEID;

typedef struct
{
    uchar iostate;
    uchar configstate;
    uchar errcount;
} TK_IO_ATTR;
/*****************************************************************************
 *                  设备的消息
 * ***************************************************************************/
enum
{
    DEV_STATE_MEG = 0,
    DEV_REPORT_MEG,
};
class Dev_Message
{
public:
    uint16_t meg_size;     //消息大小
    uint8_t  meg_state;    //目前未用，保留
    uint8_t  dev_type;     //设备类型
    uint8_t  meg_type;     //消息的类型
public:
    Dev_Message(uint8_t type = 0)
    {
        memset(this, 0x00, sizeof(Dev_Message));
        dev_type = type;
    }
    ~Dev_Message()
    {
    }
};

// typedef  QT_Share_MemT<char> TK200_State_Mem;
class TK200_State_Mem;

class PT_Dev_State
{
public:
    uint8_t dev_enbale;
    uint8_t dev_state;

public:
    PT_Dev_State():dev_enbale(0),dev_state(0)
    {
        ;
    }
    void set_dev_enbale(uint8_t val)
    {
        dev_enbale = val;
    }
    void set_dev_state(uint8_t val)
    {
        dev_state = val;
    }
    uint8_t get_dev_state(void)
    {
        return dev_state;
    }
};

class DEV_SData_Pro : public PT_Dev_State
{
public:
    uint8_t          devid;
    PT_Dev_State    *data;
    TK200_State_Mem *mem;

public:
    DEV_SData_Pro():devid(0),data(NULL),mem(NULL)
    {
        ;
    }
    void set_dev_enable(uchar val);
    void set_dev_state(uchar val);
};

/*****************************************************************************
 *                  平台设备基类
 * ***************************************************************************/
class PT_Dev_Virt
{
public:
    //    uint8_t                dev_en;            //设备使能标志
    uint8_t devtype;     //设备类型
    uint8_t dev_off;     //设备在整个驱动中的编号
    uint8_t child_id;    //该设备的子设备编号 0表示不是子设备

    int                polltimer;    //论询定时器id
    ncan_protocol     *pro_p;
    Pt_Devs_ShareData *data_p;
    MsgMng            *msgmng_p;
    can_dev_para       dev_para;     //设备参数
    CANDATAFORM        pollFrame;    // poll frame;

public:
    explicit PT_Dev_Virt(ncan_protocol *pro, Pt_Devs_ShareData *data, uint8_t type = 0):devtype(type),dev_off(0),
        child_id(0),polltimer(0),pro_p(pro),data_p(data)
    {
        msgmng_p  = MsgMng::GetMsgMng();
        memset(&pollFrame, 0x00, sizeof(pollFrame));
        CAN_DEV_PARA_INIT(dev_para);
    }

    PT_Dev_Virt():devtype(0),dev_off(0),
        child_id(0),polltimer(0),pro_p(0),data_p(0)
    {
        CAN_DEV_PARA_INIT(dev_para);
        msgmng_p        = MsgMng::GetMsgMng();
    }
    void delete_dev_timer(void)
    {
        if(polltimer != 0 && pro_p != NULL)
        {
            pro_p->delete_poll_timer(polltimer);
            polltimer = 0;
        }
    }
    void add_dev_timer(double inter, void *arg)
    {
        if(polltimer == 0 && pro_p != NULL)
        {
            polltimer = pro_p->add_poll_frame(inter, arg);
            if(polltimer <= 0)
            {
                zprintf1("Add poll time fali!\n");
                polltimer = 0;
            }
        }
    }

    void pt_dev_virt_init(ncan_protocol *pro, Pt_Devs_ShareData *data, uint8_t type = 0)
    {
        devtype = type;
        pro_p   = pro;
        data_p  = data;
    }

    ~PT_Dev_Virt()
    {
        delete_dev_timer();
        zprintf3("destory PT_Dev_Virt!\n");
    }
    //    void dev_meg_init(int parasize, int repsize);
    void         dev_send_meg(uint8_t megtype, uint8_t *data, uint16_t size);
    virtual void data_send(soutDataUnit data) = 0;
    virtual int  pt_dev_init(void)            = 0;
    virtual void reset_data_init(void);
};

class Node_Shake : public MUTEX_CLASS
{
public:
    uint8_t  node_id;
    uint8_t  node_val;
    uint8_t  node_new_val;
    uint32_t shake_count;
    uint32_t shake_num;

public:
    Node_Shake(uint8_t id = 0, uint32_t num = 0):node_id(id),node_val(0),node_new_val(0),
        shake_count(0),shake_num(num)
    {
        zprintf3("Creat shake_num %d!\n", shake_num);
    }
    void node_init(uint8_t id, uint32_t num)
    {
        node_id   = id;
        shake_num = num;
        zprintf3("Init shake_num %d!\n", shake_num);
    }
    void node_reset(void)
    {
        lock();
        node_val     = 0;
        node_new_val = 0;
        shake_count  = 0;
        unlock();
    }

    void set_new_val(uint8_t val)
    {
        lock();
        if(val != node_new_val)
        {
            node_new_val = val;
            shake_count  = 0;
        }
        unlock();
    }
    int set_node_val(void)
    {
        if(node_val != node_new_val)
        {
            node_val = node_new_val;
            return 1;
        }
        return 0;
    }
    int shake_process(void)
    {
        int err = 0;
        if(shake_num == 0)
        {
            //            zprintf3("err id %d shakecount %d num %d!\n", node_id, shake_count, shake_num);
            return 0;
        }

        //        zprintf3("id %d shakecount %d num %d!\n", node_id, shake_count, shake_num);
        lock();
        shake_count++;
        if(shake_count >= shake_num)
        {

            shake_count = 0;
            err         = set_node_val();
            //            if(node_id == 1)
            //                zprintf3("node shake count err %d !\n", err);
        }
        unlock();
        return err;
    }
};
class Dev_Node_Pro;
#define DEV_NODE_TIMER B_Timer(Dev_Node_Pro)
#define DEV_TIME_ET    B_TEvent(Dev_Node_Pro)
class Dev_Node_Pro
{
public:
    Pt_Devs_ShareData          *share_p;
    uint8_t                     ndev_id;
    uint8_t                     nc_id;
    int                         ms;
    DEV_NODE_TIMER              dtimer;
    QMap< uint8_t, Node_Shake > nodemap;

public:
    Dev_Node_Pro(Pt_Devs_ShareData *s_p = NULL, int mstimer = 100, uint8_t dev = 0, uint8_t child = 0)
        : share_p(s_p), ndev_id(dev), nc_id(child),ms(mstimer)
    {
        ;
    }
    void dev_node_data_init(Pt_Devs_ShareData *s_p, int mstimer, uint8_t dev = 0, uint8_t child = 0)
    {
        share_p = s_p;
        ms      = mstimer;
        ndev_id = dev;
        nc_id   = child;
    }

    void add_dev_node(uint8_t node, uint16_t time);
    void poll_dev_node(void);
    void set_share_data(uint8_t node, uint8_t val);
    void set_node_new_data(uint8_t node, uint8_t val);
    void dev_node_pro_init(void);
    void dev_node_reset(void);
};

class TK_IO_Dev : public PT_Dev_Virt, public DEV_SData_Pro, public Dev_Node_Pro
{

public:
    string  dev_name;
    uint8_t io_id;

    TK_IO_ATTR attr;

    bitset< 48 >    inconfig;     //输入点配置参数
    bitset< 24 >    outconfig;    //输出点配置参数
    bitset< 8 >     pollmark;     //查询帧应答标记
    TKINPUT        *inbuf;        //查询帧buf指针
    uint8_t        *iocandata;    //接收数据
    sem_t           sendSem;      // send sem
    pthread_mutex_t send_mut;

public:
    explicit TK_IO_Dev(ncan_protocol *pro, Pt_Devs_ShareData *data = NULL)
        : PT_Dev_Virt(pro, data),dev_name(""),io_id(0),inbuf(NULL),iocandata(NULL)
    {
        memset(&attr, 0x00, sizeof(TK_IO_ATTR));

        inconfig.reset();
        pollmark.reset();
        outconfig.reset();
        sem_init(&sendSem, 0, 1);
        pthread_mutex_init(&send_mut, NULL);
    }
    TK_IO_Dev():dev_name(""),io_id(0),inbuf(NULL),iocandata(NULL)
    {
        memset(&attr, 0x00, sizeof(TK_IO_ATTR));

        inconfig.reset();
        pollmark.reset();
        outconfig.reset();

        sem_init(&sendSem, 0, 1);
        pthread_mutex_init(&send_mut, NULL);
    }
    ~TK_IO_Dev()
    {

        pthread_mutex_destroy(&send_mut);
        sem_close(&sendSem);
        if(inbuf != NULL)
        {
            delete[] inbuf;
            inbuf = NULL;
        }
        if(iocandata != NULL)
        {
            delete[] iocandata;
            iocandata = NULL;
        }
        zprintf3("destory TK_IO_Dev!\n");
    }

    int  pt_dev_init(void) override;
    int  tk_io_reset_config(void);
    int  tk_io_config(CAN_DEV_INFO &dev, uint8_t devoff);
    void data_send(soutDataUnit data) override;
    void set_io_conf(uint8_t *buf);
    void set_out_node_val(int node, int val);
    void set_in_switch_node_val(int node, int val);
    void set_in_freq_node_val(int node, int val);
    void io_dev_reset()
    {
        delete_dev_timer();
    }

    void set_send_data_fram(uint &framid)
    {
        TK_FRAMEID deframid;
        deframid.fram         = framid;
        deframid.fram_org.dev = 2;
        framid                = deframid.fram;
    }
    void io_dev_reset_data_init(void)
    {

        //         pthread_mutex_unlock(&send_mut);
        pollmark.reset();

        memset(&attr, 0x00, sizeof(TK_IO_ATTR));
        attr.iostate     = TK_IO_INIT;
        attr.configstate = 1;
        dev_node_reset();
    }
    int get_io_node(int node)
    {
        return (dev_para.type == TK100_IOModule_IO) ? node + 1 : node;
    }
    int set_io_val(int val)
    {
        return (dev_para.type == TK100_IOModule_IO) ? !val : val;
    }
};
/***********************************************************************
 * ******************cs 设备发送的消息***************************
 * ********************************************************************/
enum
{
    BS_REPORT_MEG = 1,    //急停上报
    CS_REST_MEG,          //复位上报
    CS_REST_END_MEG,      //复位完成上报
    LOW_NUM_MEG,          //下位机数量变化上报
    LOW_NUM_LOST,         //下设备丢失
    CONFIG_ERROR,         //配置错误
    RESET_REASON,
    DEV_PROGRAM_ERROR,    //设备程序错误
    REPROT_CAN_ERROR,     // CAN错误
    //    IO_REPORT_AUTO,         //IO输入主动上报
    //    BS_BUTTON_MSG,          //闭锁按钮主动上报
    //    CV_REPORT_MSG,
    //    BS_MEG_MAX              //枚举最大值
};

void set_config_state(int value);

// int tk100control_output(int canid, int node, uint16_t value);

int tk100_output(void *midp, soutDataUnit val);

#endif /*__TKCOMMON_H__*/

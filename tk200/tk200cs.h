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
#ifndef __TK200CS_H__
#define __TK200CS_H__



#include "can_protocol.h"
// #include "pro_data.h"
#include "bitset"
// #include "reflect.h"
// #include "driver.h"
// #include "ptxml.h"
#include "candatainfo.h"
#include "tkcommon.h"
// #include "zmap.h"
#include "bsdev.h"
// #include "1030common.h"
// #include "sharemem.h"

using namespace std;

#define MAX_CS_EMSTOPQUANTITY        136



#define CS_VERSION                             1     //CS版本号 2016-12-22 dyq 添加（new = 4；old = 1）

class TK200_State_Mem;



class Low_InNode_Cf
{
public:
    bitset<8> info;
public:
    Low_InNode_Cf(){
        info.reset();
    }
    bool get_in_type(uint8_t in){
        return info[in];
    }
    bool get_in_enable(uint8_t in){
        return info[in+4];
    }
};

class Low_Dev:public Dev_Node_Pro
{
public:
//    int  father;
//    int  dev_off;
    bitset<8>     in;
    bitset<8>     out;


public:
    Low_Dev()
    {;
    }
    uchar get_in_info(void){
        return in.to_ulong();
    }
    uchar get_out_info(void){
        return out.to_ulong();
    }
    bool get_in_type(uint8_t node){
        return in[node];
    }
    bool get_in_enable(uint8_t node){
        return in[node+4];
    }
};

enum{
    LOW_IN_SWITCH = 0,
    LOW_IN_FEQ
};

/****************************************************************************************************
 tk100+ CAN TX MESSAGE  ID
 ***************************************************************************************************/
enum
{
    CS_START_ACK_ID         =     0x0420,          //CS上电请求的应答帧
    LOW_OUTCONTR_ASK_ID,       	                    //下位机输出控制
    LOW_INCHANGE_ACK_ID     = 0x425,                //下位机输入口变化响应
    LOW_BELT_STATE_ID,                             //皮带状态传送帧
    STOP_INCHANGE_ACK_ID      =     0x0427,         	    //急停线状态变化的应答
    LOW_DATA_ASK_ID			 =	   0x042f      	   //下位机数据请求帧

};
/****************************************************************************************************
 tk100+ CAN RX MESSAGE  ID
 ***************************************************************************************************/
enum
{
    CS_START_ASK_ID         =     0x0620,          //CS上电认可帧及应答
    LOW_OUTCONTR_ACK_ID,                           //下位机输出控制应答帧
    CS_COM_ERROR_ID,                               //CS和沿线通讯故障
    LOW_DATA_ACK_ID,                               //下位机数据应答
    CS_STATE_ACK_ID,                               //CS状态查询帧应答
    LOW_INCHANGE_ASK_ID     = 0x625,                //下位机输入口变化
    TERMIAL_STATE_ID,                             //终端状态返回帧
    STOP_INCHANGE_ASK_ID      =     0x0627,         	    //急停线状态变化的应答
    LOW_BUSY_ASK_ID,         	                     //下位机设置地址帧

};

typedef struct
{
    uchar Err_Type[6];    // 错误信息类型
    uchar ErrFlag;        // 错误更新标识    1：最新信息   0：以处理过

}Low_Err_State_Type;

class CS_DataType
{
public:
    uchar CS_Version;                          // CS板卡版本号      0：老版本      其它：新版本 //2016-12-22 dyq 添加
    uchar CS_EN;                               // CS板卡是否使能      0：禁止      1：使能
    uchar OnLine_Flag;                         // CS卡是否在线标识    0：不在线    1：在线  2：未使用  3:正在配置
    uchar OffQuantity;                         // CS卡不在线次数
    uchar resetQuantity;                       // CS卡复位次数
    uchar CardInitiative;                      // 初始化状态    0：未完成 (红)   1：正在配置（黄）   2：配置完成（绿色） 3：无效 灰色
    uchar LowDeviceSetNumber;                  // CS下位机配置台数
    uchar LowDeviceSetingN;                    // 正在配置第几台下位机
    uchar LowSlaveQuantity;				       // CS线实际下位机台数

    Low_Err_State_Type Low_Err_State;                            // 下位机错误信息
    uchar StopLineBreakPoint;                                    // 数据线的虚断位置
    uchar StopLineState;                                         // 急停线状态     1：急停线断    0：急停线正常
    uchar OldStopLineState;                                      // 老急停线状态
    uchar OnLineTerminal;				                         // 终端在线标志   1：没有查询到终端        0：正常
    uchar VoltageTerminal;				                         // 终端电压值
    uchar VoiceTerminal;				                         // 有无语音信号   0:无语音;=1:有语音
    uchar LockedAmount;                                          // 闭锁板数量
    bool  LockedState[MAX_CS_EMSTOPQUANTITY];                    // 沿线闭锁状态
    uchar Locked_InPut_State[2*MAX_CS_EMSTOPQUANTITY];             // 下位机输入点状态
    uchar ConnectionCS; // 与其他控制器是否联系 0:不联系 1:联系
public:
    CS_DataType(){
        memset(this, 0x00, sizeof(CS_DataType));
        LowDeviceSetingN = 1;
    }
    void cs_data_init(void)
    {
        memset(this, 0x00, sizeof(CS_DataType));
        LowDeviceSetingN = 1;
    }
    void cs_reset_data_init(void){

        CS_Version = 0;
        OnLine_Flag = DEV_OFF_LINE;                         // CS卡是否在线标识    0：不在线    1：在线  2：未使用  3:正在配置
//         OffQuantity++;                         // CS卡不在线次数
         resetQuantity++;                       // CS卡复位次数
         CardInitiative = 0;                      // 初始化状态    0：未完成 (红)   1：正在配置（黄）   2：配置完成（绿色） 3：无效 灰色

         LowDeviceSetingN = 1;                    // 正在配置第几台下位机
         LowSlaveQuantity = 0;				       // CS线实际下位机台数

         StopLineBreakPoint = 0;                                    // 数据线的虚断位置
         StopLineState = 0;                                         // 急停线状态     1：急停线断    0：急停线正常
         OldStopLineState = 0;                                      // 老急停线状态
         OnLineTerminal = 0;				                         // 终端在线标志   1：没有查询到终端        0：正常
         VoltageTerminal = 0;				                         // 终端电压值
         VoiceTerminal = 0;				                         // 有无语音信号   0:无语音;=1:有语音
         LockedAmount = 0;                                          // 闭锁板数量
         memset(LockedState, 0x00, sizeof(LockedState));                    // 沿线闭锁状态
         memset(Locked_InPut_State, 0x00, sizeof(Locked_InPut_State));

    }
};



class CS_Data_Pro:public CS_DataType
{
public:
    uint8_t            csid;
    CS_DataType *      data;
    TK200_State_Mem *  mem;
public:
    CS_Data_Pro():csid(0),data(NULL),mem(NULL)
    {
        ;
    }
    ~CS_Data_Pro()
    {
        ;
    }
    void set_ver(uchar val);
    void set_cs_en(uchar val);
    void set_on_flag(uchar val);
    void set_off_size(uchar val);
    void set_reset_num(uchar val);
    void set_cs_state(uchar val);
    void set_low_num(uchar val);
    void set_ing_low_num(uchar val);
    void set_slave_num(uchar val);
    void set_break_addr(uchar val);
    void set_stop_state(uchar val);
    void set_terminal_state(uchar val);
    void set_vol_val(uchar val);
    void set_voice_state(uchar val);
    void set_break_num(uchar val);

    void set_break_state(int num, bool val);
    void set_connect_state(uchar val);
    void set_reset_data_init(void);
    void set_lock_state(int node, uchar val);


};





class TK200_CS:public PT_Dev_Virt,public CS_Data_Pro
{
public:
//    uint8_t                csid;              //该cs的编号  2,3
    uint8_t                use;                 //用途
    uint8_t                errcout;
    BS_Dev                 bs_dev;
    QVector<Low_Dev *>        low_dev;       //下位机设备

public:
    explicit TK200_CS(ncan_protocol * pro, Pt_Devs_ShareData * data = NULL):PT_Dev_Virt(pro,data,TK200_CSModule),
        use(0),errcout(0)
    {
        ;
    }
    TK200_CS():use(0),errcout(0)
    {
        ;
    }
    ~TK200_CS()
    {
           zprintf3("destory TK200_CS!\n");
           for(int i = 0; i <low_dev.size(); i++)
           {
               delete low_dev[i];
           }

    }
    CANDATAFORM send_low_config(void);
    int pt_dev_init(void);
    void tk200cs_delete_frame(CANDATAFORM & fram){
        TK_FRAMEID deleframid;
        deleframid.fram  = fram.StdId;
        deleframid.fram_org.dev = 2;
        fram.StdId = deleframid.fram;
    }
    void data_send(soutDataUnit data);
    int tk200_cs_config(CAN_DEV_INFO & dev, uint8_t devoff);
    void set_bs_value(uint8_t node);
    void set_low_freq_in_val(uint8_t child, uint8_t node , double val);
    void set_low_switch_in_val(uint8_t child, uint8_t node, int val);
    Low_Dev * get_low(uint8_t child);
    void low_init(void);
    void low_node_reset(void);
    void cs_dev_reset(void);


};


void tk200_send_fram(CANDATAFORM & msg, uint8_t func, uint8_t addr);





#endif /*__TK200CS_H__*/

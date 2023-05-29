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
#include "stdio.h"
#include "tk200cs.h"
#include "zprint.h"
#include "candatainfo.h"


#include <QDomDocument>
#include <QFile>
#include <QTextCodec>

#include <QTextStream>
#include <QXmlStreamWriter>
#include <QStringList>
#include <iostream>
#include <QDebug>
#include "tk200pro.h"


void CS_Data_Pro::set_ver(uchar val){
    CS_Version = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->CS_Version = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_cs_en(uchar val){
    CS_EN = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->CS_EN = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_on_flag(uchar val){
    OnLine_Flag = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->OnLine_Flag = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_off_size(uchar val){
    OffQuantity = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->OffQuantity = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_reset_num(uchar val){
    resetQuantity = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->resetQuantity = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_cs_state(uchar val){
    CardInitiative = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->CardInitiative = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_low_num(uchar val){
    LowDeviceSetNumber = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->LowDeviceSetNumber = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_ing_low_num(uchar val){
    LowDeviceSetingN = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->LowDeviceSetingN = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_slave_num(uchar val){
    LowSlaveQuantity = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->LowSlaveQuantity = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_break_addr(uchar val){
    StopLineBreakPoint = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->StopLineBreakPoint = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_stop_state(uchar val){
    OldStopLineState = StopLineState;
    StopLineState = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->StopLineState = val;
    data->OldStopLineState = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_terminal_state(uchar val){
    OnLineTerminal = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->OnLineTerminal = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_vol_val(uchar val){
    VoltageTerminal = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->VoltageTerminal = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_voice_state(uchar val){
    VoiceTerminal = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->VoiceTerminal = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_break_num(uchar val){
    LockedAmount = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->LockedAmount = val;
    mem->unlock_qtshare();
}

void CS_Data_Pro::set_break_state(int num, bool val){
    LockedState[num] = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->LockedState[num] = val;
    mem->unlock_qtshare();
}
void CS_Data_Pro::set_connect_state(uchar val){
    ConnectionCS = val;
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->ConnectionCS = val;
    mem->unlock_qtshare();
}

void CS_Data_Pro::set_lock_state(int node, uchar val)
{
    if(mem == NULL || data == NULL) return;
    if(val == 0)
    {
        //正常
        Locked_InPut_State[(node-1) * 2] = val;

        Locked_InPut_State[(node-1) * 2 + 1] = val;
    }
    else if(val == 1)
    {
        //一级跑偏
        Locked_InPut_State[(node-1) * 2] = val;

        Locked_InPut_State[(node-1) * 2 + 1] = 0;
    }
    else if(val == 2)
    {
        //二级跑偏
        Locked_InPut_State[(node-1) * 2] = 0;

        Locked_InPut_State[(node-1) * 2 + 1] = 2;
    }
    mem->lock_qtshare();

    if(val == 0)
    {
        //正常
        data->Locked_InPut_State[(node-1) * 2] = val;

        data->Locked_InPut_State[(node-1) * 2 + 1] = val;
    }
    else if(val == 1)
    {
        //一级跑偏
        data->Locked_InPut_State[(node-1) * 2] = val;

        data->Locked_InPut_State[(node-1) * 2 + 1] = 0;
    }
    else if(val == 2)
    {
        //二级跑偏
        data->Locked_InPut_State[(node-1) * 2] = 0;

        data->Locked_InPut_State[(node-1) * 2 + 1] = 2;
    }
    mem->unlock_qtshare();
}

void CS_Data_Pro::set_reset_data_init(void)
{
    cs_reset_data_init();
    if(mem == NULL || data == NULL) return;
    mem->lock_qtshare();
    data->OnLine_Flag = DEV_OFF_LINE;                         // CS卡是否在线标识    0：不在线    1：在线  2：未使用  3:正在配置
//         OffQuantity++;                         // CS卡不在线次数
     data->resetQuantity++;                       // CS卡复位次数
     data->CardInitiative = 0;                      // 初始化状态    0：未完成 (红)   1：正在配置（黄）   2：配置完成（绿色） 3：无效 灰色

     data->LowDeviceSetingN = 1;                    // 正在配置第几台下位机
     data->LowSlaveQuantity = 0;				       // CS线实际下位机台数

     data->StopLineBreakPoint = 0;                                    // 数据线的虚断位置
     data->StopLineState = 0;                                         // 急停线状态     1：急停线断    0：急停线正常
     data->OldStopLineState = 0;                                      // 老急停线状态
     data->OnLineTerminal = 0;				                         // 终端在线标志   1：没有查询到终端        0：正常
     data->VoltageTerminal = 0;				                         // 终端电压值
     data->VoiceTerminal = 0;				                         // 有无语音信号   0:无语音;=1:有语音
     data->LockedAmount = 0;                                          // 闭锁板数量
     memset(data->LockedState, 0x00, sizeof(LockedState));                    // 沿线闭锁状态
    mem->unlock_qtshare();
}

/*******************************************************************************************************************************
**函数名：send_low_config
**输入：无
**输出：无
**功能描述：CS板卡参数配置函数
**作者：
**日期：20154.3.19
**修改：
**日期：
**版本：
********************************************************************************************************************************/
void tk200_send_fram(CANDATAFORM & msg, uint8_t func, uint8_t addr)
{
    TK_FRAMEID midframid;
    midframid.fram = 0;
    memset(&msg, 0, sizeof(CANDATAFORM));
    midframid.fram_org.dev = 2;
    midframid.fram_org.func = func;
    midframid.fram_org.addr = addr;
    msg.StdId = midframid.fram;
}

void TK200_CS::low_node_reset(void)
{
    for(uint i = 1; i <= low_dev.size(); i++)
    {
        get_low(i)->dev_node_reset();
    }
}
void TK200_CS::cs_dev_reset(void)
{
    delete_dev_timer();
}

CANDATAFORM TK200_CS::send_low_config(void)
{
    CANDATAFORM sendmeg;
    TK_FRAMEID  framid;

    framid.fram = 0;
    framid.fram_org.addr = csid;
    framid.fram_org.dev = 2;
    framid.fram_org.func = 0;

    memset(&sendmeg, 0x00,sizeof(CANDATAFORM));
    zprintf3("send 0x%x\n",sendmeg.StdId);
    sendmeg.StdId = framid.fram;
    zprintf3("cs %d send 0x%x\n",csid, sendmeg.StdId);
    sendmeg.DLC   = 5;

    if(LowDeviceSetNumber !=0 )
    {
        sendmeg.Data[0] = LowDeviceSetNumber;
        sendmeg.Data[1] = LowDeviceSetingN;
        sendmeg.Data[2] = low_dev[LowDeviceSetingN-1]->get_out_info();
        sendmeg.Data[3] = low_dev[LowDeviceSetingN-1]->get_in_info();

    }
    sendmeg.Data[4] = use;
    sendmeg.Data[4] |= (1 << 4);//CS_VERSION << 4;//2016-12-22 dyq 添加
    zprintf3("0x%x send data 4 use is %d is %d\n",sendmeg.StdId,use, sendmeg.Data[4]);

    return sendmeg;
}
/*******************************************************************************************************************************
**函数名：CanMsg CS_Type::Online (CanMsg *Msg)
**输入：无
**输出：无
**功能描述：CS板卡参数配置函数
**作者：
**日期：20154.3.19
**修改：
**日期：
**版本：
********************************************************************************************************************************/
int cs_start_ask_idprocess(void * tk_cs, CANDATAFORM data)
{
    TK200_CS * cs_p = (TK200_CS *) tk_cs;
    if(cs_p == NULL) return -1;

    TK_FRAMEID midframid;
    zprintf3("receive cs %d\n", cs_p->csid);
    midframid.fram  = data.StdId;
    uint8_t         midcsid = midframid.fram_org.addr;
    if(midcsid != cs_p->csid)           return -2;

    if(data.DLC)
    {
        //版本判断
        //***2016-12-22 dyq 添加
        uchar version = data.Data[4] >> 4;
        if(version == CS_VERSION)
            cs_p->set_ver(0);
//            cs_p->CS_Version = 0;
        else
             cs_p->set_ver(version);
//            cs_p->CS_Version = version;
        //***
        if(data.Data[0] > data.Data[1]) //还有下位机未配置
        {
            cs_p->LowDeviceSetingN++;
            cs_p->pro_p->can_protocol_send(cs_p->send_low_config());


        }
//        else
//        {
//            if(cs_p->dev_para.auto_reset == 1)
//            {
//                if(cs_p->LowDeviceSetingN != cs_p->LowDeviceSetNumber )
//                {
//                    cs_p->delete_dev_timer();
//                    zprintf1("cs %d low dev err %d  %d reset\n", cs_p->csid, cs_p->LowDeviceSetingN,cs_p->LowDeviceSetNumber);
//                    sem_post(&gTk200_Rest_Meg);
//                }
//            }


//        }
        cs_p->tk200cs_delete_frame(data);
        data.Data[4] = cs_p->use;
        data.Data[4] |= (1 << 4);
        if(cs_p->pro_p->pro_del_buf_frame(data))
        {

            zprintf3("delete cs config 0x%x fail!\n", data.StdId);
            for(int i = 0; i <= 4; i++)
            {
                zprintf3("%d %d\n", i, data.Data[i]);
            }
        }
    }
    else
    {
        cs_p->delete_dev_timer();
        if(cs_p->CardInitiative !=0)
        {
            //reset
//           cs_p->cs_data_init();
//           cs_p->set_low_num (cs_p->low_dev.size());
            zprintf1("cs %d reset\n", cs_p->csid);
            sem_post(&gTk200_Rest_Meg);
        }
        else
        {
            cs_p->set_on_flag(DEV_ON_LINE);
            cs_p->pro_p->can_protocol_send(cs_p->send_low_config());
        }
    }
    return 0;

}

int config_overtimer_process(void * tk_cs, CANDATAFORM  overmeg)
{
    zprintf3("cs config over time\n\n\n");
    return 0;
}

/*******************************************************************************************************************************
**函数名：void CS_Type::Port_Control_Answer(CanMsg *Msg)
**输入：无
**输出：无
**功能描述：CS板卡:下位机端口控制应答处理函数
**作者：
**日期：20154.3.19
**修改：
**日期：
**版本：
********************************************************************************************************************************/
int cs_output_ack_idprocess(void * tk_cs, CANDATAFORM data)
{
    TK200_CS * cs_p = (TK200_CS *) tk_cs;
    if(cs_p == NULL) return -1;


    if(data.DLC == 3)
    {
        TK_FRAMEID midframid;
        midframid.fram  = data.StdId;
        midframid.fram_org.dev = 2;
        data.StdId = midframid.fram;
        if(cs_p->pro_p->pro_del_buf_frame(data))
        {
            zprintf3("out ack error 0x%x fail!\n", data.StdId);
            for(int i = 0; i <= 4; i++)
            {
                zprintf3("%d %d\n", i, data.Data[i]);
            }
        }
        else
        {
            cs_p->data_p->set_out_ack_value(cs_p->dev_off, data.Data[0],
                                        4+data.Data[1], data.Data[2]);
        }
    }
    return 0;

}

int cs_output_overtimer_process(void * tk_cs, CANDATAFORM  overmeg)
{
    zprintf3("cs output overtimer!\n");
    sem_post(&gTk200_Rest_Meg);
    return 0;
}

/*******************************************************************************************************************************
**函数名：void CS_Type::Err_Message_Report(CanMsg *Msg)
**输入：CanMsg *Msg ：接收数据帧
**输出：无
**功能描述：CS板卡:错误信息上报处理函数
**作者：
**日期：20154.3.19
**修改：
**日期：
**版本：
********************************************************************************************************************************/
int cs_err_report_idprocess(void * tk_cs, CANDATAFORM data)
{
    TK200_CS * cs_p = (TK200_CS *) tk_cs;
    if(cs_p == NULL) return -1;


    memcpy(cs_p->Low_Err_State.Err_Type,data.Data,data.DLC);

    cs_p->Low_Err_State.ErrFlag = 1;

    if(data.DLC == 1)
    {
        zprintf4("***中位机和下位机通信出错，不需要应答，但是需要通知主程序并且错误计数!\n");
    }
    else
    {
        int error_Type = data.Data[0] & 0x0f;
        switch (error_Type) {
        case 0://系统故障
            if(data.DLC == 2)
            {
               ;
            }
            break;
        case 1://查询脉冲出错
        {
            if(data.DLC == 6)
            {
                int errorNum = (data.Data[0] & 0x0f) >> 4;
                int infoIndex = 0;
                if(errorNum == 0)//闭锁板脉冲出错
                    infoIndex = 11;
                else if(errorNum == 1)//下位机脉冲出错
                    infoIndex = 12;
                else
                {
                    //
                }
                if(infoIndex != 0)
                {
                    ;
                }
            }
        }
            break;
        case 2://通讯数据出错
        if(data.DLC == 3)
        {
            //如果是配置无应答, 发全体复位.
            if(data.Data[2] == 2 || data.Data[2] == 6)
            {
//                cs_p->cs_data_init(); //复位处理
//                cs_p->LowDeviceSetNumber = cs_p->low_dev.size();
                zprintf1("cs communate err\n");
                sem_post(&gTk200_Rest_Meg);
            }
        }
            break;
        case 5://2017-02-22 添加 下位机复位故障指令
        {
            ;
        }
            break;
        default:
            break;
        }
    }

    return 0;
}
/*******************************************************************************************************************************
**函数名：CanMsg CS_Type::Polling_LowState_Report(CanMsg *Msg)
**输入：CanMsg *Msg ：接收数据帧
**输出：int:  1：校验成功    0：失败
**功能描述：CS板卡:下位机应答轮询上报处理函数
**作者：
**日期：20154.3.19
**修改：
**日期：
**版本：
********************************************************************************************************************************/
int cs_poll_ack_idprocess(void * tk_cs, CANDATAFORM data)
{
    TK200_CS * cs_p = (TK200_CS *) tk_cs;
    if(cs_p == NULL) return -1;
    if(data.DLC != 8) return -1;

    uint8_t nodeid = data.Data[1]-1;
    uint8_t lowdevid = data.Data[0] -1;


    for(int i = 0; i < 2; i++)
    {

        if(cs_p->low_dev[lowdevid]->in[nodeid+i+4])  //使能位
        {
            TKINPUT midin;

            midin.outinfo = bswap_16(*(uint16_t *)(&data.Data[2+3*i]));
            if(cs_p->low_dev[lowdevid]->in[nodeid+i]) //频率量
            {
                double midval = 0;

                switch (midin.fin.fstate3)
                {
                    case SENSOR_FAULT:          //传感器故障
                        midval = 0;
                        break;
                    case SWITCH_BROKEN :            //开关量断路
                    case SWITCH_SHORT:             //开关量短路
                      zprintf3("freq node err!\n");
                      break;

                    case FREQ_LOW:                 //频率量小于200
                       midval = 0;
                       break;
                    case FREQ_HIGHT:                //频率量大于1000
                        midval = 0xffff;
                        break;
                    case NODE_OK :                   //
                        midval = ((midin.fin.fvale12/5)+200)*20;
                        break;

                }

//                cs_p->data_p->set_share_data_value(cs_p->dev_off, data.Data[0],
//                                            nodeid+i+1, midval);
                cs_p->set_low_freq_in_val(data.Data[0], nodeid+i+1, midval );
            }
            else
            {
                double midval = 0;

                switch (midin.swin.swstate3)
                {

                    case SWITCH_BROKEN :            //开关量断路
                        midval = 2;
                        break;
                    case SWITCH_SHORT:             //开关量短路
                        midval = 3;
                      break;
                    case SENSOR_FAULT:          //传感器故障
                    case FREQ_LOW:                 //频率量小于200

                    case FREQ_HIGHT:                //频率量大于1000
                     zprintf3(" node switch err!\n");
                    break;

                    case NODE_OK :                   //
                        midval = midin.swin.swvale1;
                        break;

                }
//                cs_p->data_p->set_share_data_value(cs_p->dev_off, data.Data[0],
//                                            nodeid+i+1, midval);
                cs_p->set_low_switch_in_val(data.Data[0], nodeid+i+1, midval);

            }
        }
    }
    if(data.Data[0] == cs_p->LowDeviceSetNumber && data.Data[1] == 3) //最后一个下位机
    {
        cs_p->errcout = 0;
        cs_p->pro_p->pro_del_buf_frame(SET_FRAM_DEV(0x42f,cs_p->csid), 0);
        if(cs_p->OnLine_Flag == 0)
           cs_p->set_on_flag(DEV_ON_LINE);
    }

    return 0;
}
int cs_poll_overtimer_process(void * tk_cs, CANDATAFORM  overmeg)
{
    TK200_CS * cs_p = (TK200_CS *) tk_cs;
    zprintf1("cs%d poll timer over %d!\n", cs_p->csid,  cs_p->errcout);
    cs_p->errcout++;
    if(cs_p->errcout >3)
    {
        cs_p->errcout = 0;
        cs_p->delete_dev_timer();
        cs_p->OffQuantity++;
        cs_p->set_off_size(cs_p->OffQuantity);
        if(cs_p->OnLine_Flag){
            cs_p->set_on_flag(DEV_OFF_LINE);
        }
        zprintf1("cs%d poll reset!\n", cs_p->csid);
        sem_post(&gTk200_Rest_Meg);
    }
    return 0;
}
/*******************************************************************************************************************************
**函数名：CanMsg CS_Type::Polling_LockedState_Report(CanMsg *Msg)
**输入：CanMsg *Msg ：接收数据帧
**输出：
**功能描述：CS板卡:沿线闭锁信息上报处理函数
**作者：
**日期：20154.3.19
**修改：
**日期：
**版本：
********************************************************************************************************************************/
int cs_bs_report_idprocess(void * tk_cs, CANDATAFORM data)
{
    TK200_CS * cs_p = (TK200_CS *) tk_cs;
    if(cs_p == NULL) return -1;


    switch(data.DLC)
    {
    case 8:
        //***2016-12-22 添加 start
        if(cs_p->CS_Version == 0)
        {
            cs_p->set_break_num (data.Data[0] & 0x7f);
            cs_p->set_terminal_state(data.Data[0]>>7);
            cs_p->set_break_addr (data.Data[3] & 0x3f);
        }
        else
        {
            //new CS
            cs_p->set_break_num (data.Data[0]);
            cs_p->set_terminal_state((data.Data[3]>>5)&1);
            cs_p->set_break_addr ( data.Data[1]);
        }
        cs_p->set_vol_val (data.Data[2]);
//        cs_p->StopLineState = data.Data[3]>>7;
        cs_p->set_stop_state(data.Data[3]>>7);
        cs_p->set_voice_state((data.Data[3]>>6)&1);

        for(int n=0;n<4;n++)
        {
            bitset<8> midv(data.Data[4+n]);
            for(int j = 0; j < 8; j++)
                cs_p->set_break_state(n*8+j , midv[7-j]);
        }
        break;
    case 7:
        for(int n=0;n<7;n++)
        {
            bitset<8> midv(data.Data[4+n]);
            for(int j = 0; j < 8; j++)
                cs_p->set_break_state(n*8+32+j , midv[7-j]);
        }
        break;
    case 6:
        for(int n=0;n<6;n++)
        {
            bitset<8> midv(data.Data[4+n]);
            for(int j = 0; j < 8; j++)
                cs_p->set_break_state(n*8+88+j , midv[7-j]);
        }
        if(cs_p->LowDeviceSetNumber == 0)
        {
            cs_p->errcout = 0;
             cs_p->pro_p->pro_del_buf_frame(SET_FRAM_DEV(0x42f,cs_p->csid), 0);
             if(cs_p->OnLine_Flag == 0)
                cs_p->set_on_flag(DEV_ON_LINE);
        }
        for(uint i = 0; i < cs_p->bs_dev.config_num; i++)
            cs_p->set_bs_value(i);
        break;
    default:break;
    }


    return 0;
}
/*******************************************************************************************************************************
**函数名：CanMsg CS_Type::InPut_State_Report(CanMsg *Msg)
**输入：CanMsg *Msg ：接收数据帧
**输出：
**功能描述：CS板卡:智能输入闭锁板及下位机输入点变化上报处理函数
**作者：
**日期：20154.3.19
**修改：
**日期：
**版本：
********************************************************************************************************************************/
int cs_instate_report_idprocess(void * tk_cs, CANDATAFORM data)
{
    TK200_CS * cs_p = (TK200_CS *) tk_cs;
    if(cs_p == NULL) return -1;

    if(data.Data[0]<0x30)
    {
        TK_FRAMEID midframid;

        midframid.fram  = data.StdId;

       double val = 0;

       if((data.Data[2]& 0xf0) == SENSOR_FAULT)
       {
           val = (data.Data[2]&0x01) + 2;
       }
       else
       {
           val = data.Data[2]&0x01;
       }
//       cs_p->data_p->set_share_data_value(cs_p->dev_off, data.Data[0],
//                                    data.Data[1], val);
       cs_p->set_low_switch_in_val(data.Data[0], data.Data[1], val);


      midframid.fram_org.dev = 2;
      data.StdId = midframid.fram;
      cs_p->pro_p->candrip->write_send_data(data);
    }
    else
    {
        switch(data.Data[0])
        {
        /*TODO: 确定一下是否需要发应答帧 */
            case 0x30:
//                if(card_controller.IsAllOnline())//2017-04-27 添加

                cs_p->set_lock_state(data.Data[1], data.Data[2]);
                break;
            case 0x31:break;
            case 0x32:break;
            case 0x33:break;
            case 0x34:break;
            case 0x35:break;
            case 0x36:break;
            case 0x37:break;
            default:break;
        }
    }

    return 0;
}
/*******************************************************************************************************************************
**函数名：CanMsg CS_Type::Stop_State_Report(CanMsg *Msg)
**输入：CanMsg *Msg ：接收数据帧
**输出：
**功能描述：CS板卡:急停线状态上报处理函数
**作者：
**日期：20154.3.19
**修改：
**日期：
**版本：
********************************************************************************************************************************/
int cs_stop_report_idprocess(void * tk_cs, CANDATAFORM data)
{
    TK200_CS * cs_p = (TK200_CS *) tk_cs;
    if(cs_p == NULL) return -1;

    TK_FRAMEID midframid;

    midframid.fram  = data.StdId;
//    cs_p->OldStopLineState = cs_p->StopLineState;


    midframid.fram_org.dev = 2;
    data.StdId = midframid.fram;
    cs_p->pro_p->candrip->write_send_data(data);
    if(cs_p->StopLineState != (data.Data[0]&1)){
        cs_p->set_stop_state(data.Data[0]&1);
        cs_p->dev_send_meg(BS_REPORT_MEG, &(cs_p->StopLineState), sizeof(cs_p->StopLineState));
    }
//    printf("stoplinestate is %d\n", cs_p->StopLineState);

    return 0;
}
/*******************************************************************************************************************************
**函数名：int CS_Type::Set_Address_Answer(CanMsg *Msg)
**输入：CanMsg *Msg ：接收数据帧
**输出：
**功能描述：CS板卡:设址结束处理函数
**作者：
**日期：20154.3.19
**修改：
**日期：
**版本：
********************************************************************************************************************************/
int cs_setaddr_ack_idprocess(void * tk_cs, CANDATAFORM data)
{
    TK200_CS * cs_p = (TK200_CS *) tk_cs;
    if(cs_p == NULL) return -1;
    TK_FRAMEID midframid;

    midframid.fram  = data.StdId;

    switch(data.Data[0])
    {
        case 0:
        {
            cs_p->set_cs_state(1);

            cs_p->set_ing_low_num(data.Data[1]);
        }
            break;
        case 1:

            cs_p->set_slave_num(data.Data[1]);//2016-07-14 添加
            if(cs_p->dev_para.auto_reset == 1)
            {
                if(cs_p->LowSlaveQuantity != cs_p->LowDeviceSetNumber )
                {
                    cs_p->delete_dev_timer();
                    zprintf1("cs %d low dev err %d  %d reset\n", cs_p->csid, cs_p->LowSlaveQuantity,cs_p->LowDeviceSetNumber);
                    sem_post(&gTk200_Rest_Meg);
                    return 0;
                }
            }
            //***2016-07-14 添加
            midframid.fram_org.dev = 2;
            data.StdId = midframid.fram;
            cs_p->pro_p->candrip->write_send_data(data);

//            memset(&data, 0x00, sizeof(CANDATAFORM));
//            data.StdId = (0x42f &DEV_ID_MASK)|(cs_p->csid << 4);
//            cs_p->polltimer = cs_p->pro_p->add_poll_frame(data, 2);
//            cs_p->polltimer = cs_p->pro_p->add_poll_frame(2, &cs_p->pollFrame);
            cs_p->add_dev_timer(2, &cs_p->pollFrame);
            //***2016-07-14 添加

            cs_p->set_cs_state(2);
            cs_p->set_ing_low_num (data.Data[1]);

            break;
        default:break;
    }

    return 0;
}

void TK200_CS::data_send(soutDataUnit data)
{
    CANDATAFORM senddata;

   if(CardInitiative  !=2) return;

    tk200_send_fram(senddata, 1, csid);
    senddata.DLC = 3;
    senddata.Data[0] = data.childid;
    senddata.Data[1] = data.pointid;
    senddata.Data[2] = data.value;

    pro_p->can_protocol_send(senddata);
//     struct timeval tv;
//    gettimeofday(&tv, NULL);
//   qDebug("send tv_sec; %d tv usec %d\n", tv.tv_sec, tv.tv_usec);

}
/*******************************************************************************************************************************
**函数名：void TK200_CS::set_bs_value(uint8_t node)
**输入：CanMsg *Msg ：接收数据帧
**输出：
**功能描述：设置tk200的闭锁值到共享内存
**作者：
**日期：20154.3.19
**修改：
**日期：
**版本：
********************************************************************************************************************************/
void TK200_CS::set_bs_value(uint8_t node)
{
    if(node >= bs_dev.config_num) return;
    if(bs_dev.bs[node] != LockedState[node])
    {
        data_p->set_share_data_value(dev_off,0, node+1, LockedState[node]);
        bs_dev.bs[node] = LockedState[node];
    }
}

void TK200_CS::set_low_freq_in_val(uint8_t child, uint8_t node , double val)
{
    if(data_p != NULL)
    {
        data_p->set_share_data_value(dev_off, child, node, val);
    }
}
Low_Dev * TK200_CS::get_low(uint8_t child)
{
    return (low_dev[child-1]);
}

void TK200_CS::set_low_switch_in_val(uint8_t child, uint8_t node, int val)
{
    get_low(child)->set_node_new_data(node, val);
}

void TK200_CS::low_init(void)
{
    for(uint i = 1; i <= low_dev.size(); i++)
    {
        zprintf3("cs low id %d  %d init!\n", i, low_dev.size());
        get_low(i)->dev_node_pro_init();
    }
}

int TK200_CS::tk200_cs_config(CAN_DEV_INFO & dev, uint8_t devoff)
{
    csid = dev.para.id;
    dev_off = devoff;

    LowDeviceSetNumber = 0;
    pro_p = pro_p;
    data_p = data_p;
    dev_para = dev.para;
    if(dev_para.innum != 0)                    //闭锁设备
    {
        bs_dev.config_num = dev_para.innum;
    }

//    dev_node_data_init(data_p, 100, dev_off);
    for(int i = 0 ; i < dev.child.size(); i++)  //添加下位机
    {
        if( dev.child[i].para.enable){
            Low_Dev * low_d = new Low_Dev;
            LowDeviceSetNumber++;

            low_d->dev_node_data_init(data_p, 100, dev_off, i+1);
            data_p->dev_share_data_init(devoff, i+1, dev.child[i].para.innum, dev.child[i].para.outnum);
            for(int j = 0; j < 4; j++)
            {
                low_d->in[j] = dev.child[i].inode[j].datatype != 2 ? 0 : 1;    //点类型
                zprintf4("in %d type %d\n", j, dev.child[i].inode[j].datatype);
                low_d->in[4+j] = dev.child[i].inode[j].node_en;   //使能位

                if(low_d->in[j] == 0 && low_d->in[4+j])
                {
                    low_d->add_dev_node(j+1, dev.child[i].inode[j].shake_time);

                }
            }
            for(int j = 0; j < 4; j++)
            {
                low_d->out[j] = dev.child[i].onode[j].link_stop; //是否连锁
                low_d->out[4+j] = 0;                        //断开闭合
            }
            low_dev.push_back(low_d);
        }
    }
    set_low_num(LowDeviceSetNumber);
    return 0;
}

int TK200_CS::pt_dev_init(void)
{
    memset(&pollFrame, 0x00, sizeof(CANDATAFORM));
    pollFrame.StdId = (0x42f &DEV_ID_MASK)|(csid << 4);
    zprintf1("cs id %d fram 0x%x!\n", csid, pollFrame.StdId);
        CANPROHEAD  canmidinfo[] ={
            {
    /*****************上电认可帧 0x420******************************************/
                0,
                (0x420 &DEV_ID_MASK)|(csid << 4),                                  //帧id
                2000,                                   //超时时间
                1,                                      //重发次数
                0x00,                                   //应答帧
                this,
                NULL,                //接收回调函数
                config_overtimer_process,                                  //超时回调函数
                NULL,                                  //发送条件回调函数
            },
    /*****************输出控制帧 0x421******************************************/
            {
                0,
                (0x421&DEV_ID_MASK)|(csid << 4),                                  //帧id
                120,                                     //超时时间
                3,                                      //重发次数
                0x00,                                   //应答帧
                this,
                NULL,                                  //接受回调函数
                cs_output_overtimer_process,                 //超时回调函数
                NULL,                                 //发送条件回调函数
            },

    /*****************输入口变化响应 0x427******************************************/
            {
                0,
                (0x427&DEV_ID_MASK)|(csid << 4),                                  //帧id
                -1,                                     //超时时间
                1,                                      //重发次数
                0x00,                                   //应答帧
                this,
                NULL,                                   //接受回调函数
                NULL,                                   //超时回调函数
                NULL,                    //发送条件回调函数
            },
    /*****************数据请求帧 0x42f******************************************/
            {
                0,
                (0x42f&DEV_ID_MASK)|(csid << 4),                                  //帧id
                300,                                     //超时时间
                1,                                      //重发次数
                0x00,                                   //应答帧
                this,
                NULL,                                   //接受回调函数
                cs_poll_overtimer_process,          //超时回调函数
                NULL,                    //发送条件回调函数
            },
    /****************************以下为接收帧*********************************************/
    /*****************上电认可帧 0x620******************************************/
            {
                0,
                (0x620&DEV_ID_MASK)|(csid << 4),                                  //帧id
                0,                                      //超时时间
                0,                                      //重发次数
                0x00,                                   //应答帧
                this,
                cs_start_ask_idprocess,                    //接受回调函数
                NULL,                                   //超时回调函数
                NULL,                                   //发送条件回调函数
            },
    /*****************输出控制指令应答 0x621******************************************/
            {
                0,
                (0x621&DEV_ID_MASK)|(csid << 4),                                  //帧id
                0,                                      //超时时间
                0,                                      //重发次数
                0x00,                                   //应答帧
                this,
                cs_output_ack_idprocess,               //接受回调函数
                NULL,                                   //超时回调函数
                NULL,                                   //发送条件回调函数
             },

    /*****************cs通讯错误帧 0x622******************************************/
            {
                0,
                (0x622&DEV_ID_MASK)|(csid << 4),                                  //帧id
                0,                                      //超时时间
                0,                                      //重发次数
                0x00,                                   //应答帧
                this,
                cs_err_report_idprocess,              //接受回调函数
                NULL,                                   //超时回调函数
                NULL,                                   //发送条件回调函数
            },

    /*****************数据请求应答帧 0x623******************************************/
            {
                0,
                (0x623&DEV_ID_MASK)|(csid << 4),                                  //帧id
                0,                                      //超时时间
                0,                                      //重发次数
                0x00,                                   //应答帧
                this,
                cs_poll_ack_idprocess,              //接受回调函数
                NULL,                                   //超时回调函数
                NULL,                                   //发送条件回调函数
            },

    /*****************cs状态查询帧应答 0x624******************************************/
            {
                0,
                (0x624&DEV_ID_MASK)|(csid << 4),                                  //帧id
                0,                                      //超时时间
                0,                                      //重发次数
                0x00,                                   //应答帧
                this,
                cs_bs_report_idprocess,              //接受回调函数
                NULL,                                   //超时回调函数
                NULL,                                   //发送条件回调函数
            },

    /*****************输入口变化接收处理 0x625******************************************/
            {
                0,
                (0x625&DEV_ID_MASK)|(csid << 4),                                  //帧id
                0,                                      //超时时间
                0,                                      //重发次数
                0x00,                                   //应答帧
                this,
                cs_instate_report_idprocess,              //接受回调函数
                NULL,                                   //超时回调函数
                NULL,                                   //发送条件回调函数
            },

    /*****************cs急停状态变化上报处理 0x627******************************************/
            {
                0,
                (0x627&DEV_ID_MASK)|(csid << 4),                                  //帧id
                0,                                      //超时时间
                0,                                      //重发次数
                0x00,                                   //应答帧
                this,
                cs_stop_report_idprocess,             //接受回调函数
                NULL,                                   //超时回调函数
                NULL,                                   //发送条件回调函数
            },
    /*****************cs设置指令 0x628***********************************************************/
            {
                0,
                (0x628&DEV_ID_MASK)|(csid << 4),                                  //帧id
                0,                                      //超时时间
                0,                                      //重发次数
                0x00,                                   //应答帧
                this,
                cs_setaddr_ack_idprocess,                   //接受回调函数
                NULL,                                   //超时回调函数
                NULL,                                   //发送条件回调函数
            },

         };
    set_cs_en(1);
    pro_p->init_pro_frame(canmidinfo, sizeof(canmidinfo)/sizeof(CANPROHEAD));
//    attr.iostate = IO_INIT;
    low_init();
    return 0;

}






































/***end*****/

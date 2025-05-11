#ifndef __1030_PRO_C__
#define __1030_PRO_C__

#include "1030pro.h"
#include "timers.h"
#include "zprint.h"

uint8_t dev_type_1030_to_236[16] = {0xff,0xff,0xff,0x0f,
                                    0xff,0x06,0xff,0xff,
                                    0xff,0xff,0x05,0xff,
                                    0xff,0xff,0xff,0xff};
/***********************************************************************************
 * 函数名：cs_config_init
 * 功能：cs配置初始化函数
 ***********************************************************************************/
static FrameId get_cs_mapid(FrameId canframeid)
{
     CANFRAMEID      rxmidframe;
     rxmidframe.canframeid = canframeid;
     return rxmidframe.canframework.func_6;
}
/***********************************************************************************
 * 函数名：add_default_dev
 * 功能：cs读取配置数据并设置配置信息
 ***********************************************************************************/
int cs_can::add_default_dev(int devid,uint16_t type)
{
    CAN_DEV_APP mid;
    int         order;
    if(is_have_dev(devid, order) == TRUE)
    {
        ndev_map.val(order).para.type = type;
        ndev_map.val(order).devdata_p = NULL;
        ndev_map.val(order).reset_default_config(type);
    }
    else
    {
        mid.para.id = devid;
        mid.stateinfo = &state_info;
        mid.devdata_p = NULL;
        ndev_map.insert(pair<int, CAN_DEV_APP>(cf_devnum, mid));
        if(ndev_map.val(cf_devnum).set_default_config(type) == 0){
            cf_devnum++;
        }
    }
    zprintf1("ndev_map.val(%d).para.type = %d\n\n\n\n",order,ndev_map.val(order).para.type);
    return 0;
}
/***********************************************************************************
 * 函数名：pt_configdata_set
 * 功能：cs读取配置数据并设置配置信息
 *
 ***********************************************************************************/
int cs_can::pt_configdata_set(CAN_DEV_INFO & dev, uint8_t dev_off)
{
    CAN_DEV_APP mid;

    mid.dev_off = dev_off;
    mid.devdata_p = data_p;
    mid.stateinfo = &state_info;

    if(dev_off == 0 && dev.para.type == CS_DEV)
    {
        state_info.set_cs_have(1);
    }
    ndev_map.insert(pair<int, CAN_DEV_APP>(cf_devnum, mid));
    if(ndev_map.val(cf_devnum).creat_config_info(dev) == 0){
        cf_devnum++;
        cf_file_num++;
    }
    return 0;
}

int cs_can::get_dev_id(int order)
{
    CAN_DEV_APP * mid = NULL ;
//    mid = devmap.get_datap(order);
    mid = ndev_map.get_order_datap(order);
    if(mid != NULL)
        return mid->para.id;
    return -1;
}

int cs_can::get_dev_off(int devid)
{
    for(int i = 0; i < cf_devnum; i++)
    {
        if(ndev_map.val(i).para.id == devid)
        {
            return ndev_map.val(i).dev_off;
        }
    }
    return -1;
}
/***********************************************************************************
 * 函数名：send_data_ttlproc
 * 功能：根据发送设置发送不同的信息
 ***********************************************************************************/
void cs_can::send_data_ttlproc(CANFRAMEID & sendframe,CANDATAFORM & senddata, uint8_t *data,
                               uint size)
{
    uint i;
    uint framenum;
    framenum = size/8;
    if(size%8)
    {
        framenum += 1;
    }
    for(i = 0; i < framenum; i++)
    {
        sendframe.canframework.ttl_7 = i;
        if(i != framenum-1)
        {
            sendframe.canframework.next_1 = 1;
            senddata.DLC = 8;
        }
        else
        {
            senddata.DLC = size%8 ? size%8 : 8;
            sendframe.canframework.next_1 = 0;
        }
        if(sendframe.canframework.func_6 == CMD_SEND_CFGPARAM &&
                                         sendframe.canframework.dest_7 != cf_endnum)
            sendframe.canframework.next_1 = 1;

        senddata.ExtId = sendframe.canframeid;
        memcpy(senddata.Data, data+(8*i), senddata.DLC);
        tbyte_swap((uint16_t *)senddata.Data, senddata.DLC);

        if(i  || (sendframe.canframework.func_6 == CMD_SEND_CFGPARAM &&
                  sendframe.canframework.dest_7 != 1))
            pro_p->candrip->write_send_data(senddata);
        else
        {
            pro_p->can_protocol_send(senddata);
        }
    }
}
/***********************************************************************************
 * 函数名：cs_send_data
 * 功能：cs应用发送的协议帧
 ***********************************************************************************/
void cs_can::cs_send_data(uint func, int dest, uint8_t * data, uint size)
{
    CANFRAMEID sendmidframe;
    CANDATAFORM sendmiddata;
    if(size)
    {
        if(data == NULL)
            return;
    }

    memset(&sendmidframe, 0, sizeof(CANFRAMEID));
    memset(&sendmiddata, 0, sizeof(CANDATAFORM));
    switch(func)
    {
        case 0:
        break;
        case 1:
        break;
        case CMD_SEND_MASTERREST:
            sendmidframe.canframework.dest_7 = 127;
            sendmidframe.canframework.func_6 = func;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE = 1;

            pro_p->can_protocol_send(sendmiddata);
       break;
       case CMD_SEND_MACCHECK:
            sendmidframe.canframework.dest_7 = 127;
            sendmidframe.canframework.func_6 = func;
            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE = 1;
            pro_p->can_protocol_send(sendmiddata);
       break;
       case CMD_SEND_PARACORRECT:
            sendmidframe.canframework.dest_7 = dest;
            sendmidframe.canframework.func_6 = func;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE = 1;
            pro_p->candrip->write_send_data(sendmiddata);
       break;
       case CMD_SEND_CFGPARAM:
       {
            CANPROHEAD * midinfop = NULL;
            sendmidframe.canframework.dest_7 = dest;
            sendmidframe.canframework.func_6 = func;
            if((midinfop = pro_p->get_protocol_frameinfo(func, 1)) != NULL)
                midinfop->overtime = csmacorder*10000;

            sendmiddata.IDE = 1;
            send_data_ttlproc(sendmidframe, sendmiddata, data, size);
        }
        break;
        case 12:
        case CMD_SEND_ERRREPORT_ACK:
            sendmidframe.canframework.dest_7 = dest;
            sendmidframe.canframework.func_6 = func;
            sendmidframe.canframework.ack_1 = 1;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE = 1;
            sendmiddata.DLC = 1;
            memcpy(sendmiddata.Data, data, sendmiddata.DLC);
            pro_p->candrip->write_send_data(sendmiddata);
//            pro_p->candrip->writeframe(sendmiddata);
        break;
        case CMD_SEND_AUTOREPORT_ACK:
        case CMD_SEND_KEY_ACK:
            sendmidframe.canframework.dest_7 = dest;
            sendmidframe.canframework.func_6 = func;
            sendmidframe.canframework.ack_1 = 1;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE = 1;
            sendmiddata.DLC = size;
            memcpy(sendmiddata.Data, data, sendmiddata.DLC);
            pro_p->candrip->write_send_data(sendmiddata);
//            pro_p->candrip->writeframe(sendmiddata);
        break;
        case 15:
            sendmidframe.canframework.dest_7 = 127;
            sendmidframe.canframework.func_6 = func;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE = 1;
            sendmiddata.DLC = 0;
            pro_p->can_protocol_send(sendmiddata);
        break;
        case 16:
            sendmidframe.canframework.dest_7 = 1;
            sendmidframe.canframework.func_6 = func;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE = 1;
            sendmiddata.DLC = 0;
            pro_p->can_protocol_send(sendmiddata);
        break;
        case 20:
        case 22:
        {
            CANPROHEAD * midinfop = NULL;

            if((midinfop = pro_p->get_protocol_frameinfo(func, 1)) != NULL)
                midinfop->overtime = dest*40;
            sendmidframe.canframework.dest_7 = dest;
            sendmidframe.canframework.func_6 = func;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE = 1;
            sendmiddata.DLC = 3;
            memcpy(sendmiddata.Data, data, sendmiddata.DLC);
            pro_p->can_protocol_send(sendmiddata);
        }
        break;
        case CMD_SEND_FOUTPUT:
        case CMD_SEND_SWOUTPUT:
        case 21:
        case 23:
        {
            CANPROHEAD * midinfop = NULL;

            if((midinfop = pro_p->get_protocol_frameinfo(func, 1)) != NULL)
                midinfop->overtime = (dest-1)*10+ 150;
            sendmidframe.canframework.dest_7 = dest;
            sendmidframe.canframework.func_6 = func;
            sendmiddata.IDE = 1;

            send_data_ttlproc(sendmidframe, sendmiddata, data, size);
//            printf("send val  %d %d\n",dest,data[0]);

        }
        break;
       case CMD_SEND_HEARTCHECK:
       {
            sendmidframe.canframework.dest_7 = dest;
            sendmidframe.canframework.func_6 = func;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE = 1;
            if(ndev_map.val(0).para.id == dest)
                heartcout = 0;
            pro_p->can_protocol_send(sendmiddata);
        }
        break;
        default:
        break;

    }
}
/***********************************************************************************
 * 函数名：is_have_dev
 * 功能：该设备是否存在
 ***********************************************************************************/
bool cs_can::is_have_dev(int devid, int & order)
{
    for(int i = 0; i < cf_devnum; i++)
    {
        if(ndev_map.val(i).para.id == devid)
        {
            order = i;
            return TRUE;
        }
    }
    return FALSE;
}

/***********************************************************************************
 * 函数名：is_have_mac_dev
 * 功能：该设备是否存在
 ***********************************************************************************/
bool cs_can::is_have_mac_dev(int devid, int & order)
{
    for(int i = 0; i < csmacorder; i++)
    {
        if(ndev_map.val(i).para.id == devid)
        {
            order = i;
            return TRUE;
        }
    }
    return FALSE;
}

/***********************************************************************************
 * 函数名：power_resetovertime_process
 * 功能：上电复位超时处理帧
 ***********************************************************************************/
static int power_resetovertime_process(void * pro1030, CANDATAFORM  overmeg)
{
    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);
    (void)overmeg;
    if(csrxcanp == NULL) return 0;
    zprintf1("1030 reset delay over!\n");
    csrxcanp->csstate = CS_CAN_RESET;
    sem_post(&csrxcanp->statechg);
    zprintf3("1030 init post sem\n");
    return 1;
}

/***********************************************************************************
 * 函数名：mac_inquire_ack_proc
 * 功能：mac查询应答帧处理
 ***********************************************************************************/
static int mac_inquire_ack_proc(void * pro1030, CANDATAFORM rxmeg)
{
    CANFRAMEID      rxmidframe;
    CANFRAMEID      checkframe;
    CANPRODATA      *rxprodata = NULL;
    int             bufmax = 0;
    int             devid = 0;
    cs_can          *csrxcanp =  static_cast<cs_can *>(pro1030);
    uint16_t       get_boot_v, get_app_v;
    (void)get_boot_v;
    tbyte_swap((uint16_t *)rxmeg.Data, rxmeg.DLC);

    rxmidframe.canframeid = rxmeg.ExtId;
    if(csrxcanp->csstate == CS_CAN_RESET)
    {
        if( csrxcanp->csmacstate == MAC_NORMAL)
        {
            if((csrxcanp->csmacorder+1) != rxmidframe.canframework.souc_7 && csrxcanp->csdevtyle == 0)
            {
               zprintf1("mac MAC_ORDER_ERR\n");
               csrxcanp->csmacstate = MAC_ORDER_ERR;
            }
            else
            {
                bufmax = rxmidframe.canframework.souc_7;
                rxmeg.Data[5] = 0;
                uint16_t devtype = 0;
                memcpy(&devtype, &rxmeg.Data[4], 2);
                csrxcanp->state_info.set_dev_type(0, csrxcanp->csmacorder, dev_type_1030_to_236[rxmeg.Data[4]]);
                csrxcanp->state_info.set_dev_ID(0, csrxcanp->csmacorder, bufmax);
                get_boot_v = rxmeg.Data[1] << 8 | rxmeg.Data[0];
                get_app_v  = rxmeg.Data[3] << 8 | rxmeg.Data[2];
                csrxcanp->state_info.set_dev_version_state(0, csrxcanp->csmacorder, 0, get_app_v);

                if(csrxcanp->is_have_dev(bufmax, devid))          //存在该设备的配置
                {
                    if(csrxcanp->ndev_map.val(devid).para.type != devtype)
                    {
                        zprintf1("mac type %d %d\n", csrxcanp->ndev_map.val(devid).para.type, devtype);
                        if(devtype != TERMINAL)
                        {
                            zprintf1("Need new config para!\n");
                            return -1;
                        }
                    }
                    else
                    {
                        // csrxcanp->ndev_map.val(devid).cscrc = *(uint16_t *)(&rxmeg.Data[6]);
                        memcpy(&csrxcanp->ndev_map.val(devid).cscrc, &rxmeg.Data[6], sizeof(uint16_t));
                        memcpy(csrxcanp->ndev_map.val(devid).csmac, rxmeg.Data, 6);
                    }
                    checkframe = rxmidframe;
                    checkframe.canframework.dest_7 = 127;
                    checkframe.canframework.souc_7 = 0;
                    checkframe.canframework.ack_1 = 0;
                    rxprodata = csrxcanp->pro_p->read_deldata_buf(checkframe.canframeid, 1);
                    if(rxprodata == NULL)
                    {
                         zprintf1("error:mac rxprodata is NULL!\n");
                         csrxcanp->csmacstate = MAC_KERNEL_ERR;
                         return -2;
                    }

                    rxprodata->runtime = 0;
                }
                else  //没有该设备的配置信息，添加默认配置
                {
                    zprintf3("add dev %d\n", devtype);
                    csrxcanp->add_default_dev(bufmax,devtype);
                    if(csrxcanp->is_have_dev(bufmax, devid))
                    {
                        // csrxcanp->ndev_map.val(devid).cscrc = *(uint16_t *)(&rxmeg.Data[6]);
                        memcpy(&csrxcanp->ndev_map.val(devid).cscrc, &rxmeg.Data[6], sizeof(uint16_t));
                        memcpy(csrxcanp->ndev_map.val(devid).csmac, rxmeg.Data, 6);
                    }
                }
                csrxcanp->csmacorder++;
                if(devtype == CS_DEV)
                    csrxcanp->mac_cs_have = 1;

                if(devtype == TERMINAL)
                {
                    zprintf3("delet dev type %d\n",devtype);
                    csrxcanp->mac_terminal_have = 1;
    //                csrxcanp->terminal_num = csrxcanp->csmacorder;
                    checkframe = rxmidframe;
                    checkframe.canframework.dest_7 = 127;
                    checkframe.canframework.souc_7 = 0;
                    checkframe.canframework.ack_1 = 0;
                    rxprodata = csrxcanp->pro_p->read_deldata_buf(checkframe.canframeid, 1);
                    if(rxprodata != NULL){
                         zprintf3("delet dev type %d\n",devtype);
                        csrxcanp->pro_p->del_buf_frame(rxprodata);
                    }
                    csrxcanp->csstate = CS_CAN_MACREQ;
                    sem_post(&csrxcanp->statechg);
                }
            }
        }
    }
    else if (csrxcanp->csstate == CS_CAN_NORMAL)
    {
        bufmax = rxmidframe.canframework.souc_7;
        rxmeg.Data[5] = 0;
        uint16_t devtype = 0;
        memcpy(&devtype, &rxmeg.Data[4], 2);
        if(devtype == TERMINAL)
        {
            if(csrxcanp->mac_terminal_have == 0)
            {
                csrxcanp->mac_terminal_have = 1;
                csrxcanp->add_default_dev(rxmidframe.canframework.souc_7, devtype);
            }
            csrxcanp->set_devonline_num(rxmidframe.canframework.souc_7);
            if(csrxcanp->get_config_low_num() != csrxcanp->get_mac_low_num())
            {
                csrxcanp->cs_can_reset_pro();
            }
            csrxcanp->state_info.set_slaveio_num(0, csrxcanp->get_mac_low_num());
            csrxcanp->state_info.set_dev_type(0, rxmidframe.canframework.souc_7 - 1, dev_type_1030_to_236[rxmeg.Data[4]]);
            csrxcanp->state_info.set_dev_num(0, rxmidframe.canframework.souc_7);
            csrxcanp->set_dev_state(rxmidframe.canframework.souc_7, DEV_ON_LINE);
            csrxcanp->cs_send_data(CMD_SEND_PARACORRECT, 127,NULL, 0);
        }
    }
    return 0;
}
/***********************************************************************************
 * 函数名：macreqdata_overtimeproc
 * 功能：mac查询超时处理函数
 ***********************************************************************************/
static int macreqdata_overtimeproc(void * pro1030, CANDATAFORM  overmeg)
{
    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);
    (void)overmeg;
    if(csrxcanp->csstate != CS_CAN_NORMAL)
    {
        zprintf1("mac over\n");
        csrxcanp->csstate = CS_CAN_MACREQ;
        sem_post(&csrxcanp->statechg);
    }
    if (csrxcanp->csmacorder == 0)
    {
        csrxcanp->state_info.set_cs_state2( 0, CS_WORK_STATUS_LIVEOUT );
    }
    return 0;
}
/***********************************************************************************
 * 函数名：int configdata_ack_proc(int csnum, CANDATAFORM  rxmeg)
 * 功能：配置数据应答帧处理
 ***********************************************************************************/
static int configdata_ack_proc(void * pro1030, CANDATAFORM  rxmeg)
{
    int soureid;
    CANFRAMEID rxmidframe;
    CANFRAMEID checkframe;
    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);

    if(csrxcanp == NULL)           return -1;

    rxmidframe.canframeid = rxmeg.ExtId;

    if(csrxcanp->is_have_dev(rxmidframe.canframework.souc_7, soureid) == FALSE) return -2;

    if(rxmidframe.canframework.souc_7 == csrxcanp->cf_endnum)
    {
        checkframe = rxmidframe;
//        checkframe.canframework.dest_7 = csrxcanp->devmap.zmap[0].para.id;
        checkframe.canframework.dest_7 = csrxcanp->ndev_map.val(0).para.id;

        checkframe.canframework.souc_7 = 0;
        checkframe.canframework.ack_1 = 0;
        checkframe.canframework.next_1 = 1;

        csrxcanp->pro_p->pro_del_buf_frame(checkframe.canframeid, 1);
        for(int i = 1 ; i <= csrxcanp->csmacorder; i++)
        {
            csrxcanp->set_dev_state(i, DEV_ON_LINE);
        }
        csrxcanp->cs_send_data(CMD_SEND_PARACORRECT, 127, NULL, 0);
        csrxcanp->csstate = CS_CAN_NORMAL;
        sem_post(&csrxcanp->statechg);
    }
    return 0;
}
/***********************************************************************************
 * 函数名：configdata_overtimeproc
 * 功能：配置数据超时处理           //有问题
 ***********************************************************************************/
static int configdata_overtimeproc(void * pro1030, CANDATAFORM  overmeg)
{
    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);
    (void) overmeg;
    if(csrxcanp == NULL)  return -1;
    csrxcanp->send_configdata();
    return 0;
}
/***********************************************************************************
 * 函数名：output_controlack_frameproc
 * 功能：频率输出控制应答帧处理
 ***********************************************************************************/
static int foutput_controlack_frameproc(void * pro1030, CANDATAFORM  rxmeg)
{
    CANFRAMEID      rxmidframe;
    CANFRAMEID      checkframe;
    int             soureid;

    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);

    if(csrxcanp == NULL)  return 0;

    rxmidframe.canframeid = rxmeg.ExtId;
    soureid = rxmidframe.canframework.souc_7;
    checkframe = rxmidframe;
    checkframe.canframework.dest_7 = rxmidframe.canframework.souc_7;
    checkframe.canframework.souc_7 = 0;
    checkframe.canframework.ack_1 = 0;
    rxmeg.ExtId = checkframe.canframeid;
    csrxcanp->ndev_map.val(soureid-1).errcount = 0;
    zprintf1("recv foutput!\n");
    if(csrxcanp->pro_p->pro_del_buf_frame(rxmeg) != 0)
    {
        zprintf1("f output control delete error!\n");
        //error
    }
    else
    {
        zprintf1("receive souc = %d, data[0] = %x, data[1] = %x data[2] = %x\n",rxmidframe.canframework.souc_7 ,rxmeg.Data[0], rxmeg.Data[1], rxmeg.Data[2]);
        uint16_t midval  = 0;
        midval = rxmeg.Data[1]*0x100 + rxmeg.Data[2];
        csrxcanp->data_p->set_out_ack_value(csrxcanp->ndev_map.val(soureid-1).dev_off, 0,
        csrxcanp->ndev_map.val(soureid-1).para.innum + rxmeg.Data[0], midval);
        zprintf1("write freq val: %d\n", midval);
    }
    return 1;
}
/***********************************************************************************
 * 函数名：output_controlack_frameproc
 * 功能：输出控制应答帧处理
 ***********************************************************************************/
static int output_controlack_frameproc(void * pro1030, CANDATAFORM  rxmeg)
{
    CANFRAMEID      rxmidframe;
    CANFRAMEID      checkframe;
    int             soureid;

    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);

    if(csrxcanp == NULL)  return 0;

    rxmidframe.canframeid = rxmeg.ExtId;
    soureid = rxmidframe.canframework.souc_7;
    checkframe = rxmidframe;
    checkframe.canframework.dest_7 = rxmidframe.canframework.souc_7;
    checkframe.canframework.souc_7 = 0;
    checkframe.canframework.ack_1 = 0;
    rxmeg.ExtId = checkframe.canframeid;
    csrxcanp->ndev_map.val(soureid-1).errcount = 0;
    zprintf1("enter switch\n");
    if(csrxcanp->pro_p->pro_del_buf_frame(rxmeg) != 0)
    {
        zprintf1("switch output control delete error 0x%x data 0x%x!\n", rxmeg.ExtId, rxmeg.Data[0]);
    }
    else
    {
        zprintf1("receive sour = %d,  num = %d, value = %x,+++++++++++++++++++++++++++++\n", rxmidframe.canframework.souc_7, rxmeg.Data[0] / 4, rxmeg.Data[0] % 4);
        csrxcanp->data_p->set_out_ack_value(csrxcanp->ndev_map.val(soureid -1).dev_off, 0,
        csrxcanp->ndev_map.val(soureid -1).para.innum +(rxmeg.Data[0] >> 2), rxmeg.Data[0]&0x03);
        zprintf1("write switch val: %x\n", rxmeg.Data[0]&0x03);
    }
    return 1;
}
/***********************************************************************************
 * 函数名：output_controlack_overproc
 * 功能：输出控制超时处理
 ***********************************************************************************/
static int output_controlack_overproc(void * pro1030, CANDATAFORM  overmeg)
{
    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);
    if(csrxcanp == NULL)  return -1;
     CANFRAMEID      rxmidframe;
     rxmidframe.canframeid = overmeg.ExtId;

//     printf("donot receive command B %d,%d,%x,-----------------------------------------------\n",rxmidframe.canframework.souc_7 ,overmeg.Data[0] / 4, overmeg.Data[0] % 4);
//     csrxcanp->state_info.set_dev_state(rxmidframe.canframework.dest_7, DEV_OFF_LINE); //离线
     csrxcanp->set_dev_state(rxmidframe.canframework.dest_7, DEV_OFF_LINE); //离线
     zprintf1("1030dev%d out control failed!\n", rxmidframe.canframework.dest_7);
     for(int i = 0; i <overmeg.DLC; i++)
     {
         zprintf3(" 0x%x ", overmeg.Data[i]);
     }
     zprintf3("\n");
     csrxcanp->cs_can_reset_pro();

    return 0;

}

/***********************************************************************************
 * 函数名：stop_report_frameproc
 * 功能：闭锁上报帧处理
 ***********************************************************************************/
// static int stop_report_frameproc(void * pro1030, CANDATAFORM  rxmeg)
// {
//     CANFRAMEID      rxmidframe;
//     cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);

//     if(csrxcanp == NULL)  return 0;
//     rxmidframe.canframeid = rxmeg.ExtId;

//     csrxcanp->bs100info.jtstate = rxmeg.Data[0];
//     //待添加急停处理
// //     csrxcanp->dev_send_meg(BS_REPORT_MEG, &(cs_p->StopLineState), sizeof(cs_p->StopLineState));
//     if(rxmeg.Data[0] == 0)
//     {
//         csrxcanp->csstopnum++;
//     }
//     else
//     {
//         csrxcanp->csstopnum--;
//     }
//     csrxcanp->cs_send_data(CMD_SEND_STOPREPORT_ACK, rxmidframe.canframework.souc_7,rxmeg.Data, 1);
//     return 1;
// }

/***********************************************************************************
 * 函数名：max_stop_report_frameproc
 * 功能：闭锁上报帧处理
 ***********************************************************************************/
static int max_stop_report_frameproc(void * pro1030, CANDATAFORM  rxmeg)
{
    CANFRAMEID      rxmidframe;
    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);

    if(csrxcanp == NULL)  return 0;

    rxmidframe.canframeid = rxmeg.ExtId;

    //待添加急停处理
    csrxcanp->ndev_map.val(0).dev_send_meg(BS_REPORT_MEG, &rxmeg.Data[0], 1);
    if(rxmeg.Data[0] == 0)
    {
        csrxcanp->csstopnum++;
    }
    else
    {
        csrxcanp->csstopnum--;
    }
    csrxcanp->cs_send_data(CMD_SEND_STOPREPORT_ACK, rxmidframe.canframework.souc_7,rxmeg.Data, 1);
    csrxcanp->state_info.set_cs_bs_state(0, rxmeg.Data[0]);
    return 1;
}
/***********************************************************************************
 * 函数名：auto_report_frameproc
 * 功能：主动上报帧处理
 ***********************************************************************************/
static int auto_report_frameproc(void * pro1030, CANDATAFORM  rxmeg)
{
    int soureid;
    CANFRAMEID rxmidframe;
    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);

    if(csrxcanp == NULL)          return 0;
    INCOFPARA1 inpara;
    RXINDATA rxdata;
    gBugState = 0;
    gBugCanInfo = rxmeg;
    rxmidframe.canframeid = rxmeg.ExtId;
    soureid = rxmidframe.canframework.souc_7;
    tbyte_swap((uint16_t *)rxmeg.Data, 2);

    if(csrxcanp->is_have_dev(soureid, soureid))
    {
        if(csrxcanp->ndev_map.val(soureid).devenable == 0)
        {
            goto ACK_PROSS;
        }
        if(memcmp(rxmeg.Data, &csrxcanp->ndev_map.val(soureid).para.type, 1) == 0)
        {
            if(rxmeg.Data[0] != TK100_CSModule)
            {
                csrxcanp->ndev_map.val(soureid).dev_normal_process();
                rxdata.rxinval = rxmeg.Data[2];
                if(csrxcanp->ndev_map.val(soureid).config_p != NULL)
                {
                    inpara.paraval = csrxcanp->ndev_map.val(soureid).config_p[INNODEINFO(rxdata.rxinattr.innum5 -1)];
                    if(inpara.inval.inenable1 != 1 || rxdata.rxinattr.instyle1 != inpara.inval.instyle1)
                    {
                        zprintf1("%d auto in error\n", soureid);
                    }
                    else
                    {
                       if(rxdata.rxinattr.instyle1 == 0)
                       {
                           csrxcanp->data_p->set_share_data_value(csrxcanp->ndev_map.val(soureid).dev_off, 0, rxdata.rxinattr.innum5,rxdata.rxinattr.instate2);
                       }
                       else
                       {
                           if(rxmeg.DLC < 5)
                           {
                               zprintf1("%d auto in data error\n", soureid);
                           }
                           gBugState = 1;
                           {
                               uint16_t devvalue = ((uint16_t)rxmeg.Data[3]) << 8;
                               devvalue += rxmeg.Data[4];
                               csrxcanp->data_p->set_share_data_value(csrxcanp->ndev_map.val(soureid).dev_off, 0, rxdata.rxinattr.innum5, devvalue);
                           }
                           gBugState = 2;
                       }
                    }
                }
            }
            else  //沿线设备
            {
                BSINFO midbsinfo;
                midbsinfo.bsvalue = rxmeg.Data[3];
                csrxcanp->bs100info.set_bs_info(rxmeg.Data[2] -1, midbsinfo);
                csrxcanp->bs100info.data_process();
                csrxcanp->data_p->set_share_data_value(csrxcanp->bs100info.dev_off,0,rxmeg.Data[2],rxmeg.Data[3]);
            }
        }
        else
        {
            zprintf1("data %d %d  soureid is %d\n", rxmeg.Data[0], rxmeg.Data[1], soureid);
        }
    }
ACK_PROSS:
    csrxcanp->ndev_map.val(soureid).errcount = 0;
    gBugState = 3;
    tbyte_swap((uint16_t *)rxmeg.Data, 2);
    csrxcanp->cs_send_data(CMD_SEND_AUTOREPORT_ACK, rxmidframe.canframework.souc_7,rxmeg.Data, rxmeg.DLC);
    gBugState = 4;
    return 1;
}

/***********************************************************************************
 * 函数名：heart_frame_over
 * 功能：心跳应答帧处理函数调用的子函数
 ***********************************************************************************/
static void heart_frame_over(CANPRODATA  * rxprodata, cs_can * csrxcanp)
{
    if(rxprodata == NULL || csrxcanp == NULL)  return;

    csrxcanp->csreqmark.csnumstate = 0;
    csrxcanp->heartcout = 0;
    csrxcanp->pro_p->del_buf_frame(rxprodata);
}

static __inline__ void tk100_data_pro(cs_can *csrxcanp, CANDATAFORM  rxmeg)
{
    csrxcanp->bs100info.bsnum = rxmeg.Data[2];
    csrxcanp->bs100info.zdstate = rxmeg.Data[4];
    csrxcanp->bs100info.jtstate = rxmeg.Data[6];
    csrxcanp->bs100info.powerstate = rxmeg.Data[6]>>7;
}

static void com_heart_nextprocess(CANPRODATA *rxprodata, cs_can *csrxcanp,uint8_t devnum, int ttl, CANDATAFORM  rxmeg)
{
     uint16_t devtyle = 0;

    if(rxprodata == NULL || csrxcanp == NULL)  return;

    if(csrxcanp->ndev_map.val(devnum).framark[ttl] == 1)
        return;

    devtyle = csrxcanp->ndev_map.val(devnum).para.type;
    csrxcanp->ndev_map.val(devnum).heart_ok = 1;

    if(ttl == 0)
    {
        if(rxmeg.DLC < 2 || comdevio[devtyle].innum != rxmeg.Data[1] || comdevio[devtyle].outnum != rxmeg.Data[0])
            return ;
        if(devtyle != TK100_CSModule)
        {
             memcpy(csrxcanp->ndev_map.val(devnum).iodata, &rxmeg.Data[2], 6);
        }
        else
        {
            tk100_data_pro(csrxcanp, rxmeg);
        }
    }
    else
    {
        if(devtyle != TK100_CSModule)
        {
            memcpy(&(csrxcanp->ndev_map.val(devnum).iodata[(ttl-1)*4+3]), rxmeg.Data, 8);
        }
        else
        {
            csrxcanp->bs100info.save_poll_data((ttl-1)*8,8,rxmeg.Data);
        }
    }
    csrxcanp->ndev_map.val(devnum).framark.set(ttl);
}

static __inline__ bool count_heart_nextprocess(cs_can *csrxcanp, uint8_t devnum, uint ttl)
{
    return  csrxcanp->ndev_map.val(devnum).framark.to_ulong() ==
                       ((0x0001U << (ttl)) -1U);
}

static bool is_heartframe_correct(cs_can *csrxcanp, uint8_t devnum, int ttl, CANDATAFORM  rxmeg, uint16_t devtyle)
{
    bool ret = FALSE;
    if(((ttl *8)+rxmeg.DLC) == DEVPOLLSIZE(devtyle) || devtyle == TK100_CSModule)
    {
        if(count_heart_nextprocess(csrxcanp, devnum,ttl))
        {
           ret = TRUE;
        }
        csrxcanp->ndev_map.val(devnum).framark.reset();
    }
    return ret;
}

static void heart_nonextprocess(CANPRODATA *rxprodata,cs_can *csrxcanp, uint8_t devnum, int ttl, CANDATAFORM  rxmeg)
{
    static uint16_t c_last[2] = {0};
    static uint8_t v_last[2] = {0};
    uint16_t devtyle = 0;

    if(rxprodata == NULL || csrxcanp == NULL)  return;
    devtyle = csrxcanp->get_dev_type(devnum); //ndev_map.val(devnum).para.type;    //待确定设备类型
//    zprintf3("id %d type %d\n",devnum, devtyle);
    if(devtyle != CS_DEV && devtyle != TERMINAL)
    {
        if(is_heartframe_correct(csrxcanp,devnum,ttl, rxmeg, devtyle) == 0)
        {
            zprintf1("error: receive 30 frame is not complate");
            return;
        }
    }
    csrxcanp->ndev_map.val(devnum).heart_ok = 1;
    csrxcanp->ndev_map.val(devnum).errcount = 0;
    if(ttl == 0)
    {
        if((devtyle != CS_DEV) && (devtyle != TERMINAL))
        {
            memcpy(csrxcanp->ndev_map.val(devnum).iodata, &rxmeg.Data[2], rxmeg.DLC-2);
            csrxcanp->set_dev_state(devnum +1, DEV_ON_LINE); //在线
        }
        else if(devtyle != TERMINAL)
        {
            tbyte_swap((uint16_t *)rxmeg.Data, rxmeg.DLC);

            uint16_t c1 =  (rxmeg.Data[4] << 8)+rxmeg.Data[3];
            uint16_t c2 = (rxmeg.Data[7] << 8) + rxmeg.Data[6];

            csrxcanp->state_info.set_cs_state(0,rxmeg.Data[2], c1, rxmeg.Data[5], c2);
            csrxcanp->state_info.set_cs_bs_state(0, rxmeg.Data[1]);
            csrxcanp->set_dev_state(devnum +1, DEV_ON_LINE); //在线
            if (abs(c_last[0]-c1) > 10 || abs(c_last[1]-c2) > 10 || \
                abs(v_last[0]-rxmeg.Data[2]) > 100 || abs(v_last[1]-rxmeg.Data[5]) > 100 )
            {
                zprintf1("c_last[0] = %d, c_last[1] = %d, v_last[0] = %d, v_last[1] = %d\n", c_last[0],c_last[1],v_last[0],v_last[1]);
                zprintf1("c[0] = %d, c[1] = %d, v[0] = %d, v[1] = %d\n", c1,c2,rxmeg.Data[2],rxmeg.Data[5]);
                c_last[0] = c1;
                c_last[1] = c2;
                v_last[0] = rxmeg.Data[2];
                v_last[1] = rxmeg.Data[5];
            }
        }
        else          //TERMINAL设备
        {
//            printf("set terminal v = %d\n",rxmeg.Data[2]);
//            qDebug()<<"set terminal v1 ="<<rxmeg.Data[2];
            csrxcanp->set_dev_state(devnum + 1, DEV_ON_LINE); //在线
            csrxcanp->state_info.set_termal_vol(0, rxmeg.Data[2]);
        }
    }
    else
    {
        if(devtyle != CS_DEV)
        {
            // zprintf1("devnum = %x\n",devnum);
            memcpy(&(csrxcanp->ndev_map.val(devnum).iodata[(ttl)*4+3]), rxmeg.Data, rxmeg.DLC);
            csrxcanp->ndev_map.val(devnum).set_share_data();
            csrxcanp->set_dev_state(devnum +1, DEV_ON_LINE); //在线
            // for (int i = 0; i < 50;i++)
            // {
            //     zprintf3(" data = %x\n", csrxcanp->ndev_map.val(devnum).iodata[i]);
            // }
            // printf("\n");
        }

    }
}
/***********************************************************************************
 * 函数名：max_heart_ackframe_proc
 * 功能：心跳应答帧处理函数
 ***********************************************************************************/
static int max_heart_ackframe_proc(void * pro1030, CANDATAFORM  rxmeg)
{
    int soureid;
    CANFRAMEID rxmidframe;
    CANFRAMEID checkframe;

    CANPRODATA  * rxprodata = NULL;
    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);

    if(csrxcanp == NULL) return -1;
    rxmidframe.canframeid = rxmeg.ExtId;
    if(csrxcanp->is_have_dev(rxmidframe.canframework.souc_7,soureid) == FALSE) return -2;
    tbyte_swap((uint16_t *)rxmeg.Data, rxmeg.DLC);

    checkframe = rxmidframe;
    checkframe.canframework.dest_7 = rxmidframe.canframework.souc_7;
    checkframe.canframework.souc_7 = 0;
    checkframe.canframework.ack_1 = 0;
    checkframe.canframework.next_1 = 0;
    checkframe.canframework.ttl_7 = 0;

    rxprodata = csrxcanp->pro_p->read_deldata_buf(checkframe.canframeid, 1);
    //zprintf1("receive heart frame: ID is : %x, date is : ",rxmeg.ExtId);
    // for (int i = 0; i < rxmeg.DLC; i++)
    // {
    //     zprintf3("%x ",rxmeg.Data[i]);
    // }
    // zprintf3("\n");
    if(rxprodata != NULL)
    {
        if(rxmidframe.canframework.next_1 == 1)
        {
            com_heart_nextprocess(rxprodata,csrxcanp,soureid,rxmidframe.canframework.ttl_7,rxmeg);
        }
        else
        {
            heart_nonextprocess(rxprodata, csrxcanp,soureid,rxmidframe.canframework.ttl_7, rxmeg);
             if(rxmidframe.canframework.souc_7 == csrxcanp->csmacorder)  //
             {
                 heart_frame_over(rxprodata, csrxcanp);
             }
             else if(csrxcanp->csdevtyle == 1)  //分次发送查询帧
             {
                  csrxcanp->pro_p->del_buf_frame(rxprodata);
                  csrxcanp->heartcout++;
                  if(csrxcanp->is_have_dev(csrxcanp->heartcout,soureid))
                    csrxcanp->cs_send_data(CMD_SEND_HEARTCHECK, soureid,NULL, 0);
             }
        }
    }
    return 0;
}
/***********************************************************************************
 * 函数名：max_heartframe_overtimeproc
 * 功能：心跳帧超时处理函数
 ***********************************************************************************/
static int max_heartframe_overtimeproc(void * pro1030, CANDATAFORM  overmeg)
{
    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);
    CAN_DEV_APP   * dev_pro_p = NULL;
    if(csrxcanp == NULL) return -1;
    (void)overmeg;
    int soureid =0;
    int devtype = 0;
    int reset_mark = 0;
    if(csrxcanp->is_have_mac_dev(csrxcanp->heartcout+1, soureid))
    {
        dev_pro_p  = &csrxcanp->ndev_map.val(soureid);
        if ( dev_pro_p->heart_ok == 0 )
        {
            if(++dev_pro_p->errcount >= 2)
            {
                csrxcanp->mac_terminal_have = 0;
                zprintf1("1030dev %d heart overtime!\n", soureid+1);
                dev_pro_p->devstate = 0;

                devtype = csrxcanp->get_dev_type(soureid); //ndev_map.val(soureid).para.type;
                zprintf1("1030dev %d type!\n", devtype);
                if (dev_pro_p->errcount == 2)
                {
                    csrxcanp->set_dev_state(soureid+1, DEV_OFF_LINE); //离线
                    if(devtype == CS_DEV)
                    {
                        csrxcanp->state_info.set_cs_state2(0, CS_WORK_STATUS_LIVEOUT);
                        csrxcanp->state_info.set_cs_state(0, 0, 0, 0, 0);
                        csrxcanp->state_info.set_cs_bs_state(0, 1);
                    }
                }
                else
                {
                    if (devtype != TERMINAL)
                    {
                        zprintf1("1030dev %d offline!\n", soureid+1);
                        if(soureid+1 == csrxcanp->devonnum)
                        {
                            csrxcanp->state_info.set_slaveio_num(0, soureid - 1);
                            if(csrxcanp->state_info.set_dev_num(0, soureid))
                            {
                                if(csrxcanp->csmacorder != soureid && csrxcanp->auto_reset == 0)
                                {
                                    uint8_t dev_num[2];
                                    dev_num[0] = csrxcanp->get_config_low_num();
                                    dev_num[1] = soureid ? (soureid - 1) : 0;
                                    zprintf3("onlin  %d %d !\n", dev_num[0], dev_num[1]);
                                    csrxcanp->ndev_map.val(0).dev_send_meg(LOW_NUM_MEG, dev_num, sizeof(dev_num));
                                }
                            }
                        }
                        if(devtype != CS_DEV)
                        {
                            reset_mark = csrxcanp->cs_can_reset_pro();
                        }
                        else
                        {
                            reset_mark = csrxcanp->cs_can_reset_sem();
                        }
                    }
                    else
                    {
                        csrxcanp->state_info.set_termal_vol(0, 0);
                        if(soureid+1 == csrxcanp->devonnum)
                        {
                            csrxcanp->state_info.set_dev_num(0, soureid);
                        }
                    }
                    dev_pro_p->errcount = 0;
                }
            }
        }
        else
        {
            dev_pro_p->heart_ok = 0;
        }
        if(reset_mark != 1){
            csrxcanp->heartcout++;
            if(csrxcanp->is_have_mac_dev(csrxcanp->heartcout+1, soureid))
            {
                dev_pro_p  = &csrxcanp->ndev_map.val(soureid);
//                if(dev_pro_p->devenable)
                {
                  csrxcanp->cs_send_data(CMD_SEND_HEARTCHECK, dev_pro_p->para.id,NULL,0);
                }
            }else{
                csrxcanp->heartcout = 0;
            }
        }
    }else {
        csrxcanp->heartcout = 0;
    }
    return 0;
}
/***********************************************************************************
 * 函数名：err_reportframe_proc
 * 功能：错误上报帧处理
 ***********************************************************************************/
static int err_reportframe_proc(void * pro1030, CANDATAFORM  rxmeg)
{
    CANFRAMEID rxmidframe;

    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);

    tbyte_swap((uint16_t *)rxmeg.Data, rxmeg.DLC);

    if(csrxcanp == NULL) return -1;

    rxmidframe.canframeid = rxmeg.ExtId;
    csrxcanp->cs_send_data(CMD_SEND_ERRREPORT_ACK, rxmidframe.canframework.souc_7,rxmeg.Data, rxmeg.DLC);


    return 0;
}
/***********************************************************************************
 * 函数名：slavereset_frame_proc
 * 功能：从机复位帧处理
 ***********************************************************************************/
static int slavereset_frame_proc(void * pro1030, CANDATAFORM  rxmeg)
{
    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);
    if(csrxcanp == NULL) return 0;

    tbyte_swap((uint16_t *)rxmeg.Data, rxmeg.DLC);
    rxmeg.Data[5] = 0;
    uint16_t devtype = 0;
    memcpy(&devtype, &rxmeg.Data[4], 2);
    printf("reset\n");
    if(devtype != TERMINAL)
    {
        zprintf1("1030 slaver reset!\n");
        csrxcanp->cs_can_reset_sem();
    }
    else
    {
        printf("terminal reset\n");
        csrxcanp->mac_terminal_have = 1;
        csrxcanp->cs_send_data(CMD_SEND_MACCHECK, 127,NULL, 0);
    }
    return 0;
}



/***********************************************************************************
 * 函数名：key_report_frameproc
 * 功能：按键上报帧处理
 ***********************************************************************************/
static int key_report_frameproc(void * pro1030, CANDATAFORM  rxmeg)
{
    uint soureid;
    CANFRAMEID rxmidframe;

    cs_can        * csrxcanp = static_cast<cs_can *>(pro1030);
    if(csrxcanp == NULL)          return 0;

    rxmidframe.canframeid = rxmeg.ExtId;
    soureid = rxmidframe.canframework.souc_7;

    if(soureid > 0)
    {
//        CanKeyboard::instance().handle(rxmeg);
        //带添加数据保存
    }
    csrxcanp->cs_send_data(CMD_SEND_KEY_ACK,rxmidframe.canframework.souc_7, rxmeg.Data, rxmeg.DLC);
    return 0;
}

/***********************************************************************************
 * 函数名：cs_can
 * 功能：cs初始化函数
 ***********************************************************************************/

cs_can::cs_can(ncan_protocol * pro, const QString  key, int reset_enable)
{
    auto_reset = reset_enable;
    sem_init(&statechg, 0, 0);
    sem_init(&reset_sem, 0, 0);
    pro_p = pro;
    state_info.max_state_pro_init(key, 1);
}
int cs_can::cs_can_reset_sem(void)
{
    if(init_over == 1)
    {
//        delete_1030_dev_timer();
        if(reset_state != DEV_RESET_ING){
            reset_state = DEV_RESET_ING;
            zprintf1("send 1030 reset!\n");
            sem_post(&reset_sem);
            return 1;
        }
    }
    return 0;
}
int cs_can::cs_can_reset_pro(void)
{
    if(auto_reset)
    {
       zprintf1("1030 auto reset!\n");
       return cs_can_reset_sem();
    }
    return 0;
}


uint8_t cs_can::get_config_low_num(void)
{

    return cf_file_num ? cf_file_num-1 : cf_file_num;
}
uint8_t cs_can::get_mac_low_num(void)
{
    if(csmacorder < (mac_cs_have + mac_terminal_have))
    {
        if(mac_cs_have)
            return 0;
        else
            return 0xff;
    }
    if(csmacorder == 0)
        return 0xff;
//    qDebug()<<"mac_terminal_have = "<<mac_terminal_have<<"csmacorder = "<<csmacorder<<"mac_cs_have = "<<mac_cs_have;
//    zprintf3("mac_terminal_have = %d csmacorder = %d mac_cs_have = %d\n", csmacorder, mac_cs_have, mac_terminal_have);
    if (mac_terminal_have) {
        return csmacorder - mac_cs_have - mac_terminal_have;
    } else {
        return csmacorder - mac_cs_have;
    }
}



/***********************************************************************************
 * 函数名：send_configdata
 * 功能：cs发送配置参数
 ***********************************************************************************/
void cs_can::max_reset_data(void)
{
      csstate = CS_CAN_INIT;        //1030协议初始化时的状态变迁
      devonnum = 0;
      // cf_devnum = cf_devnum;      //配置的设备个数
      cf_endnum = 0;                //重配的结束设备编号
      en_devmax = 0;                //使能的最大设备号
      csmacstate = MAC_NORMAL;      //mac查询状态
      csmacorder = 0;     //mac查询应答顺序编号
      mac_cs_have = 0;
      mac_terminal_have = 0; //mac查询终端存在
      heartcout = 0;      //心跳发送顺序计数
      csconfstate = CSCONF_NORMAL;    //配置状态
      csstopnum = 0;      //闭锁数量
      initproid = 0;      //1030协议初始化线程id号
      polltimer_id = 0;
//      terminal_num = 0;
      memset(&csreqmark, 0x00, sizeof(REQDEVMK));
      state_info.set_all_state_clear();
      for(uint i = 0; i < ndev_map.size(); i++){
          ndev_map.val(i).reset_dev_data();
      }
      state_info.set_all_state(DEV_OFF_LINE);
      //data_p->reset_data_value();           //IO no reset
}

/***********************************************************************************
 * 函数名：send_configdata
 * 功能：cs发送配置参数
 ***********************************************************************************/
void cs_can::send_configdata(void)
{
    uint i;
    uint configsize = mac_terminal_have ? csmacorder -1 : csmacorder;

    zprintf3("configsize %d!\n", configsize);
    for(i = 0; i < configsize; i++)
    {
        if(ndev_map.val(i).conf_state == CS_CONFIGING)
        {
            zprintf3("send config %d!\n", i);
            cs_send_data(CMD_SEND_CFGPARAM, ndev_map.val(i).para.id, (uint8_t *)ndev_map.val(i).config_p,
                                    ndev_map.val(i).configsize*2);
            linuxDly(200);
        }

    }
    zprintf3("send config over!\n");
}

static void *heartManageThread(void *para)
{
    zprintf1("|||heartManageThread||| thread start...\n");

    //绑定共享内存
    QString key = "ManageHparameter2@localhost";
    int offset = 2;
    QSharedMemory shareMemory(key);

    while(!shareMemory.attach(QSharedMemory::ReadOnly))
    {
        sleep(1);
    }

    zprintf1("|||heartManageThread||| attach success, key is %s\n", shareMemory.key().toStdString().c_str());

    //监听logic心跳数据
    char *shareData = static_cast<char*>(shareMemory.data());
    uint16_t data = 0, dataLast = 0;
    uint8_t heartErrCnt = 0;
    uint8_t logicState = 1;    //logic初始值为异常状态，等待其心跳刷新后开始进行心跳超时判断
    if(shareData != nullptr)
    {
        while(1)
        {
            linuxDly(500);
            data = *(uint16_t *)(shareData + offset);

            if(data == dataLast)
            {
                if((logicState == 0) && (++heartErrCnt >= 14))
                {
                    zprintf1("|||heartManageThread||| logic heart timeout, cs reset\n");
                    (static_cast<cs_can *>(para))->cs_can_reset_sem();
                    heartErrCnt = 0;
                    logicState = 1; //logic状态异常
                }
            }
            else
            {
                heartErrCnt = 0;
                logicState = 0;     //logic状态正常
            }

            dataLast = data;
        }
    }
    else
    {
        zprintf1("|||heartManageThread||| failed to get sharedata pointer");
    }

    return NULL;
}


/***********************************************************************************
 * 函数名：cs_protocol_init
 * 功能：cs协议初始化函数 供协议层回调的cs协议初始化函数
 ***********************************************************************************/
void * cs_can::cs_protocol_init(void *para)
{

    if((static_cast<cs_can *>(para))->cs_init() != 0){
        zprintf1("init fail send sem!\n");
        (static_cast<cs_can *>(para))->cs_can_reset_sem();
    }
    return NULL;

}

/***********************************************************************************
 * 函数名：cs_protocol_init
 * 功能：cs协议初始化函数 供协议层回调的cs协议初始化函数
 ***********************************************************************************/
static void * max_reset_process(void *para)
{
    int reset_err = 1;
    cs_can * cs_pro_p = (static_cast<cs_can *>(para));
    uint8_t  reset_msg[3];
    // static uint8_t reset_cnt = 0;
    while(1){
        reset_err = 1;
        sem_wait(&cs_pro_p->reset_sem);
        cs_pro_p->state_info.set_cs_state2(0, CS_WORK_STATUS_LIVEOUT);
        cs_pro_p->delete_1030_dev_timer();

        while (reset_err != 0) {
            cs_pro_p->ndev_map.val(0).dev_send_meg(CS_REST_MEG, NULL, 0);
            cs_pro_p->max_reset_data();
            zprintf1("1030 reset process start!\n");
            reset_err = cs_pro_p->cs_init();

            if ( reset_err == -1 )
            {
                cs_pro_p->state_info.set_cs_state2(0, CS_WORK_STATUS_LIVEOUT);
//                if ( reset_cnt++ > 2 )
//                {
//                    break;
//                }
            }
            if (reset_err != 0 ) {
                zprintf1("reset err %d\n", reset_err);
                reset_msg[0] = RESET_FAIL;
                reset_msg[1] = cs_pro_p->get_config_low_num();
                reset_msg[2] = cs_pro_p->get_mac_low_num();
                zprintf1("mac order is %d\n", cs_pro_p->csmacorder);
                zprintf3("fail %d %d\n", reset_msg[1], reset_msg[2]);
                cs_pro_p->ndev_map.val(0).dev_send_meg(CS_REST_END_MEG, reset_msg, sizeof(reset_msg));
                sleep(4);
            }
        }
        cs_pro_p->reset_state = DEV_RESET_OVER;
    }
    return NULL;
}

/***********************************************************************************
 * 函数名：poll_send_condition
 * 功能：帧发送条件
 *
 ***********************************************************************************/
static int max_poll_send_condition(void * para)
{
    cs_can * cs_pro_p = (static_cast<cs_can *>(para));
     if(cs_pro_p->csstate == CS_CAN_NORMAL){
         return 1;
     }
     return 0;
}

void cs_can::set_dev_state(uint8_t devid, uint8_t state)  //Start from 1
{
    uint16_t devtyle = get_dev_type(devid -1);
    uint16_t dev_off;
    dev_off = devid;

    if(devtyle != TERMINAL)
    {
        if (state)
        {
            state_info.set_dev_state_OR(dev_off, state);
        }
        else
        {
            state_info.set_dev_state_AND(dev_off, state);
        }
    }
    else          //TERMINAL设备
    {
        state_info.set_termal_state(dev_off / FATHER_DEV_MAX, state);
    }
    if(state == DEV_ON_LINE)
    {
        if(devid > devonnum)
            devonnum = devid;
    }
    else
    {
        if(devid < devonnum)
            devonnum = devid;
    }
}

/***********************************************************************************
 * 函数名：cs_config_init
 * 功能：cs配置初始化函数
 ***********************************************************************************/
int cs_can::cs_config_init(void)
{
    CANFRAMEID overmidframe;
//    CANDATAFORM data;
    memset(&pollFrame, 0x00,sizeof(pollFrame));
    overmidframe.canframework.ack_1 = 0;
//    overmidframe.canframework.dest_7 = devmap.zmap[0].para.id;
    overmidframe.canframework.dest_7 = ndev_map.val(0).para.id;
    overmidframe.canframework.souc_7 = 0;
    overmidframe.canframework.func_6 = CMD_SEND_HEARTCHECK;
    overmidframe.canframework.ttl_7 = 0;
    overmidframe.canframework.next_1 = 0;
    pollFrame.ExtId = overmidframe.canframeid;
    pollFrame.IDE = 1;
    CANPROHEAD  csmidinfo[] ={
        {
/*****************上电复位帧 功能码2******************************************/
            1,
            2,                                      //映射编号
            1000,                                   //超时时间
            1,                                      //重发次数
            0x00,                                   //应答帧
            this,
            NULL,                                   //接收回调函数
            power_resetovertime_process,                 //超时回调函数
            NULL,                               //发送条件回调函数
        },
/*****************mac 地址查询帧 功能码3******************************************/
        {
            1,
            3,                                      //映射编号
            100,                                     //超时时间
            1,                                      //重发次数
            0x00,                                   //应答帧
            this,
            mac_inquire_ack_proc,                                   //接受回调函数
            macreqdata_overtimeproc,                 //超时回调函数
            NULL,                    //发送条件回调函数
        },

/*****************从站配置参数正确确认帧 功能码4******************************************/
        {
            1,
            4,                                      //映射编号
            0,                                     //超时时间
            0,                                      //重发次数
            0x00,                                   //应答帧
            this,
            NULL,                                   //接受回调函数
            NULL,                                   //超时回调函数
            NULL,                                   //发送条件回调函数
        },
/*****************整体配置帧及应答 功能码5******************************************/
        {
            1,
            5,                                  //映射编号
            0,                                     //超时时间
            0,                                      //重发次数
            0x00,                                   //应答帧
            this,
            configdata_ack_proc,                                   //接受回调函数
            configdata_overtimeproc,          //超时回调函数
            NULL,                    //发送条件回调函数
        },
/*****************从机上电复位通知帧 功能码6******************************************/
        {
            1,
            6,                                      //映射编号
            0,                                      //超时时间
            0,                                      //重发次数
            0x00,                                   //应答帧
            this,
            slavereset_frame_proc,                  //接受回调函数
            NULL,                                   //超时回调函数
            NULL,                                   //发送条件回调函数
        },
/*****************输出控制模拟量指令及应答 功能码10******************************************/
        {
            1,
            10,                                  //帧id
            0,                                      //超时时间
            3,                                      //重发次数
            0x00,                                   //应答帧
            this,
            foutput_controlack_frameproc,               //接受回调函数
            output_controlack_overproc,               //接受回调函数,                                   //超时回调函数
            NULL,                                   //发送条件回调函数
         },

/*****************输出控制开关量指令及应答 功能码11******************************************/
        {
            1,
            11,                                     //帧id
            0,                                      //超时时间
            3,                                      //重发次数
            0x00,                                   //应答帧
            this,
            output_controlack_frameproc,              //接受回调函数
            output_controlack_overproc,                                   //超时回调函数
            NULL,                                   //发送条件回调函数
        },

/*****************闭锁急停上报帧及应答 功能码12******************************************/
        {
            1,
            12,                                     //帧id
            0,                                      //超时时间
            0,                                      //重发次数
            0x00,                                   //应答帧
            this,
            max_stop_report_frameproc,              //接受回调函数
            NULL,                                   //超时回调函数
            NULL,                                   //发送条件回调函数
        },

/*****************错误上报帧及应答 功能码13******************************************/
        {
            1,
            13,                                     //帧id
            0,                                      //超时时间
            0,                                      //重发次数
            0x00,                                   //应答帧
            this,
            err_reportframe_proc,                   //接受回调函数
            NULL,                                   //超时回调函数
            NULL,                                   //发送条件回调函数
        },

/*****************主动上报帧及应答 功能码14******************************************/
        {
            1,
            14,                                     //帧id
            0,                                      //超时时间
            0,                                      //重发次数
            0x00,                                   //应答帧
            this,
            auto_report_frameproc,                  //接受回调函数
            NULL,                                   //超时回调函数
            NULL,                                   //发送条件回调函数
        },
/*****************按键上报帧及应答 功能码18******************************************/
        {
            1,
            18,                                     //帧id
            0,                                      //超时时间
            0,                                      //重发次数
            0x00,                                   //应答帧
            this,
            key_report_frameproc,                  //接受回调函数
            NULL,                                   //超时回调函数
            NULL,                                   //发送条件回调函数
        },

/*****************心跳查询帧及应答 功能码30******************************************/
        {
            1,
            30,                                     //帧id
            300,                                      //超时时间
            1,                                      //重发次数
            0x00,                                   //应答帧
            this,
            max_heart_ackframe_proc,                    //接受回调函数
            max_heartframe_overtimeproc,                //超时回调函数
            max_poll_send_condition,                                   //发送条件回调函数
        }

     };

    pro_p->init_pro_frame(csmidinfo, sizeof(csmidinfo)/sizeof(CANPROHEAD));
    pro_p->idfunc[1] = get_cs_mapid;

    pthread_create(&reset_id, NULL, max_reset_process, (void*)this);
    pthread_create(&initproid, NULL, cs_protocol_init, (void*)this);
    pthread_create(&heartManageId, NULL, heartManageThread, (void *)this);

    return 0;
}
/***********************************************************************************
 * 函数名：cs_reset_config
 * 功能：cs重新配置函数
 ***********************************************************************************/
// static int poll1030_callback(CANp_TIME_ET * poll)
// {

//     poll->father->can_protocol_send((static_cast<cs_can *>(poll->para))->pollFrame);

//    return 0;

// }
void cs_can::reset_all_dev(void)
{
    CANFRAMEID sendmidframe;
    CANDATAFORM sendmiddata;

    memset(&sendmidframe, 0, sizeof(CANFRAMEID));
    memset(&sendmiddata, 0, sizeof(CANDATAFORM));

    sendmidframe.canframework.dest_7 = 127;
    sendmidframe.canframework.func_6 = CMD_SEND_MASTERREST;

    sendmiddata.ExtId = sendmidframe.canframeid;
    sendmiddata.IDE = 1;

    pro_p->candrip->write_send_data(sendmiddata);
}

void cs_can::add_poll_frame(void)
{
    if(polltimer_id == 0)
        polltimer_id =  pro_p->add_poll_frame(2, &pollFrame);
}
/***********************************************************************************
 * 函数名：get_dev_type
 * 功能：获得设备类型
 ***********************************************************************************/
uint16_t cs_can::get_dev_type(uint8_t devid)
{
    if(mac_terminal_have && (devid+1) == csmacorder)
        return TERMINAL;
    else
        return ndev_map.val(devid).para.type;
}

void cs_can::set_devonline_num(uint8_t val)
{
    csmacorder = val;
    zprintf1("csmacorder is %d\n", csmacorder);
}

/***********************************************************************************
 * 函数名：cs_send_data
 * 功能：cs应用发送的协议帧       configtype 0：配置类型第一次配置 1：重新配置
 ***********************************************************************************/
/*configtype ; 0 :No register can of dev 1: Haved register can of dev*/

int cs_can::cs_init(void)
{
    uint      i;
    uint8_t   reset_msg[3];

    state_info.set_cs_state2( 0, CS_WORK_STATUS_NO_CONFIG );
    state_info.set_ptcan_version(0);
    cs_send_data(CMD_SEND_MASTERREST, 127,NULL, 0);

    do
    {
        sem_wait(&statechg);
    }
    while(csstate !=  CS_CAN_RESET);
    zprintf3("1030 send reset end!\n");

    cs_send_data(CMD_SEND_MACCHECK, 127,NULL, 0);
    do
    {
        sem_wait(&statechg);
    }
    while(csstate != CS_CAN_MACREQ);

    zprintf3("1030 mac req end!\n");

    if (csmacstate != MAC_NORMAL || csmacorder == 0) {
        zprintf1("csmacstate err %d\n", csmacstate);
        state_info.set_dev_num(0, 0);
        state_info.set_slaveio_num(0, 0);
        state_info.set_bs_num(0, 0);
        memset(state_info.share_state.m_data->line_state.BS_Buttion, 0, BS_MAX_NUM);
        csmacorder = 0;
        init_over = 1;
        state_info.set_cs_state2( 0, CS_WORK_STATUS_LIVEOUT );
        return -1;
    }
    state_info.set_dev_num(0, csmacorder);
    state_info.set_slaveio_num(0, get_mac_low_num());
    zprintf3("get_config_low_num() = %d\n",get_config_low_num());
    zprintf3("get_mac_low_num() = %d\n",get_mac_low_num());
    if(get_config_low_num() != get_mac_low_num())
    {
        if(auto_reset)
        {
            init_over = 1;
            csconfstate = csmacorder < cf_file_num ? CSCONF_MORE :CSCONF_LESS;
            return csconfstate;
        }
    }
    zprintf3("csmacorder  %d cf_devnum %d\n", csmacorder ,cf_devnum);
    for(i = 0; i < csmacorder; i++)
    {
        if(csdevtyle == 0 && (get_dev_type(i) != TERMINAL))
        if(ndev_map.val(i).cscrc != ndev_map.val(i).config_p[3])
        {
            zprintf3("Config %d style is %d!\n",i, ndev_map.val(i).para.type);

            ndev_map.val(i).cscrc = ndev_map.val(i).config_p[3];
            ndev_map.val(i).conf_state = CS_CONFIGING;
            csconfstate = CSCONF_NEED;
            cf_endnum = ndev_map.val(i).para.id;
        }
    }
    zprintf3("1030 config start!\n");
    if(csconfstate == CSCONF_NEED)
    {
        zprintf3("1030 config need!\n");
        send_configdata();
        do
        {
            sem_wait(&statechg);
        }
        while(csstate != CS_CAN_NORMAL);
    }
    else
    {
        for(i = 1 ; i <= csmacorder; i++)
        {
            set_dev_state(i, DEV_ON_LINE);
        }
        cs_send_data(CMD_SEND_PARACORRECT, 127,NULL, 0);
        csstate = CS_CAN_NORMAL;
    }
    state_info.set_cs_state2(0, CS_WORK_STATUS_RIGHT);
    zprintf3("1030 config end!\n");
    cs_send_data(CMD_SEND_HEARTCHECK, ndev_map.val(0).para.id, NULL, 0);

    add_poll_frame();
    zprintf3("1030 successful!\n");
    init_over = 1;

    reset_msg[0] = RESET_SUCCESS;
    reset_msg[1] = get_config_low_num();
    reset_msg[2] = get_mac_low_num();
    zprintf3("suc init %d %d !\n", reset_msg[1], reset_msg[2]);
    if ( reset_msg[2] == 0xFF )
    {
        //TODO: zj
        state_info.set_cs_state2( 0, CS_WORK_STATUS_LIVEOUT );
    }
    ndev_map.val(0).dev_send_meg(CS_REST_END_MEG, reset_msg, sizeof(reset_msg));
    return 0;
}
/***********************************************************************************
 * 函数名：~cs_can()
 * 功能：cs析构函数
 ***********************************************************************************/
cs_can::~cs_can()
{
    reset_all_dev();
    zprintf3("destory 1030 cs can!\n");
    if(reset_id != 0){
        pthread_cancel(reset_id);
        pthread_join(reset_id, NULL);
        reset_id = 0;
    }
    if(heartManageId != 0)
    {
        pthread_cancel(heartManageId);
        pthread_join(heartManageId, NULL);
        heartManageId = 0;
    }
}

/***********************************************************************************
 * 函数名：int dev1030_output(void * midp, sDataUnit val)
 * 功能：cs应用层控制输出函数
 *
 ***********************************************************************************/
int dev1030_output(void * midp, soutDataUnit val)
{

    cs_can * pro = static_cast<cs_can *>(midp);

    uint8_t         outvalue[3];
    uint16_t     data = (uint16_t)val.value;

//    if(val.parentid == 3)
//     zprintf1("send val  %d %d %d %d\n", val.parentid, val.childid, val.pointid, (int)val.value);
//    qDebug("send val  %d %d %d %d\n", val.parentid, val.childid, val.pointid, (int)val.value);
//    printf("send val  %d %d %d %d\n", val.parentid, val.childid, val.pointid, (int)val.value);
    if(val.parentid > pro->csmacorder) {
//        qDebug("dev  num is error!\n");
        return -1;
    }
    if( pro->reset_state == DEV_RESET_ING)
    {
         zprintf1("max cs reseting!\n");
         return -2;
    }
    if(pro->get_dev_type(val.parentid-1) == CS_DEV ||
         pro->get_dev_type(val.parentid-1) ==  TERMINAL)
    {
            zprintf1("max cs no output!\n");
         return -2;
     }
    if(pro->ndev_map.val(val.parentid-1).devenable != 1){

          zprintf1("dev disable!\n");
         return -3;
     }
    if(pro->state_info.get_dev_state(val.parentid) != DEV_ON_LINE){
        zprintf1("max dev%d off line!\n", val.parentid);
        return -2;
    }
//    else
//    {
//        zprintf1("max dev%d online!\n", val.parentid);
//    }
//    zprintf1("send val  %d %d %d %d\n", val.parentid, val.childid, val.pointid, (int)val.value);
//    printf("send val  %d %d %d %d\n", val.parentid, val.childid, val.pointid, (int)val.value);
    if(pro->ndev_map.val(val.parentid-1).get_outnode_tyle(val.pointid) == 0)
    {
        zprintf1("send switch val  %d %d %d %d\n", val.parentid, val.childid, val.pointid, (int)val.value);
        outvalue[0] = data;
        outvalue[0] |= (val.pointid << 2);

        pro->cs_send_data(CMD_SEND_SWOUTPUT, val.parentid, outvalue, 1);

    }
    else
    {
        zprintf1("send val freq cs = %d childid = %d point = %d freq = %d\n", val.parentid, val.childid, val.pointid, data);
        outvalue[0] = val.pointid;
        tbyte_swap((uint16_t *)(&data), 2);
        memcpy(&outvalue[1], &data, 2);
        pro->cs_send_data(CMD_SEND_FOUTPUT, val.parentid, outvalue, 3);
    }

    return 0;
}

#endif /*__1030_PRO_C__*/

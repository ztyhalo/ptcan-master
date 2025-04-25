#ifndef __1030_PRO_C__
#define __1030_PRO_C__

#include "1030pro.h"

#include "timers.h"

#include "zprint.h"


/***********************************************************************************
 * 函数名：cs_config_init
 * 功能：cs配置初始化函数
 ***********************************************************************************/
FrameId get_cs_mapid(FrameId canframeid)
{
    CANFRAMEID rxmidframe;
    rxmidframe.canframeid = canframeid;
    return rxmidframe.canframework.func_5;
}
/***********************************************************************************
 * 函数名：add_default_dev
 * 功能：cs读取配置数据并设置配置信息
 ***********************************************************************************/
int cs_can::add_default_dev(int zjnum, int csnum, int devid, int io_num, uint16_t type)
{
    CAN_DEV_APP mid;
    int         order;
    int         dev_off = devid - 1;

    if (is_have_dev(zjnum, csnum, devid, order) == TRUE)
    {
        nconfig_map.val(order).para.type = type;
        nconfig_map.val(order).devdata_p = NULL;
        nconfig_map.val(order).para.name.clear( );
        nconfig_map.val(order).reset_default_config(type);
    }
    else
    {
        if (zjnum)
        {
            devid |= 0x100 << (csnum - 2);
            dev_off %= FATHER_DEV_MAX;
            dev_off += FATHER_DEV_MAX * (csnum - 1);
        }
        mid.dev_off   = dev_off;
        mid.para.id   = devid;
        mid.para.type = type;
        mid.para.name.clear( );
        mid.slaveio_order = io_num;
        mid.stateinfo     = &state_info;
        mid.devdata_p     = NULL;
        mid.order_in_cfg  = cf_devnum;
        nconfig_map.insert(pair< int, CAN_DEV_APP >(cf_devnum, mid));

        if (nconfig_map.val(cf_devnum).set_default_config(type) == 0)
        {
            cf_devnum++;
        }
    }
    return 0;
}

int cs_can::pt_configdata_set(CAN_DEV_INFO &dev, uint8_t cs_num, uint8_t dev_num)
{
    CAN_DEV_APP mid;
    uint16_t    dev_off = dev_num + cs_num * FATHER_DEV_MAX;

    mid.dev_off   = dev_off;
    mid.devdata_p = data_p;
    mid.stateinfo = &state_info;

    // TODO: del cs_have
    if (dev_off == 0 && dev.para.type == CS_DEV)
    {
        state_info.set_cs_have(1);
    }

    nconfig_map.insert(pair< int, CAN_DEV_APP >(cf_devnum, mid));
    nconfig_map.val(cf_devnum).slaveio_order = dev_off + 1;
    nconfig_map.val(cf_devnum).order_in_cfg  = cf_devnum;

    if (nconfig_map.val(cf_devnum).creat_config_info(dev) == 0)
    {
        cf_devnum++;
        cf_file_num++;
    }
    return 0;
}
/***********************************************************************************
 * 函数名：pt_configdata_set
 * 功能：cs读取配置数据并设置配置信息
 *
 ***********************************************************************************/
int cs_can::pt_configdata_set(CAN_DEV_INFO &dev, uint16_t dev_off)
{
    CAN_DEV_APP mid;

    mid.dev_off   = dev_off;
    mid.devdata_p = data_p;
    mid.stateinfo = &state_info;

    // TODO: del cs_have
    if (dev_off == 0 && dev.para.type == CS_DEV)
    {
        state_info.set_cs_have(1);
    }

    nconfig_map.insert(pair< int, CAN_DEV_APP >(cf_devnum, mid));
    nconfig_map.val(cf_devnum).slaveio_order = dev_off + 1;
    nconfig_map.val(cf_devnum).order_in_cfg  = cf_devnum;

    if (nconfig_map.val(cf_devnum).creat_config_info(dev) == 0)
    {
        cf_devnum++;
        cf_file_num++;
    }
    return 0;
}

int cs_can::get_dev_id(int order)
{
    CAN_DEV_APP *mid = NULL;
    mid              = nconfig_map.get_order_datap(order);
    if (mid != NULL)
        return mid->para.id;
    return -1;
}

int cs_can::get_dev_off(int devid)
{
    for (int i = 0; i < cf_devnum; i++)
    {
        if (nconfig_map.val(i).para.id == devid)
        {
            return nconfig_map.val(i).dev_off;
        }
    }
    return -1;
}
/***********************************************************************************
 * 函数名：send_data_ttlproc
 * 功能：根据发送设置发送不同的信息
 ***********************************************************************************/
void cs_can::send_data_ttlproc(CANFRAMEID &sendframe, CANDATAFORM &senddata, uint8_t *data, uint size)
{
    uint i;
    uint framenum;
    framenum = size / 8;
    if (size % 8)
    {
        framenum += 1;
    }
    for (i = 0; i < framenum; i++)
    {
        sendframe.canframework.ttl_7 = i;
        if (i != framenum - 1)
        {
            sendframe.canframework.next_1 = 1;
            senddata.DLC                  = 8;
        }
        else
        {
            senddata.DLC                  = size % 8 ? size % 8 : 8;
            sendframe.canframework.next_1 = 0;
        }
        if (sendframe.canframework.func_5 == CMD_SEND_CFGPARAM && sendframe.canframework.devaddr_8 != cf_endnum)
            sendframe.canframework.next_1 = 1;

        senddata.ExtId = sendframe.canframeid;
        memcpy(senddata.Data, data + (8 * i), senddata.DLC);
        tbyte_swap((uint16_t *)senddata.Data, senddata.DLC);

        if (i || (sendframe.canframework.func_5 == CMD_SEND_CFGPARAM && sendframe.canframework.devaddr_8 != 1))
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
void cs_can::cs_send_data(uint func, int zjnum, int csnum, int dest, uint8_t *data, uint size)
{
    CANFRAMEID  sendmidframe;
    CANDATAFORM sendmiddata;
    if (size)
    {
        if (data == NULL)
            return;
    }
    memset(&sendmidframe, 0, sizeof(CANFRAMEID));
    memset(&sendmiddata, 0, sizeof(CANDATAFORM));
    switch (func)
    {
        case 0:
            break;
        case 1:
            break;
        case CMD_SEND_MASTERREST:
            sendmidframe.canframework.devaddr_8 = 255;
            sendmidframe.canframework.func_5    = func;
            sendmidframe.canframework.csaddr_2  = csnum;
            sendmidframe.canframework.zjaddr_3  = zjnum;
            sendmidframe.canframework.dir_1     = 1;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE   = 1;

            pro_p->can_protocol_send(sendmiddata);
            break;
        case CMD_SEND_MACCHECK:
            sendmidframe.canframework.devaddr_8 = 255;
            sendmidframe.canframework.func_5    = func;
            sendmidframe.canframework.csaddr_2  = csnum;
            sendmidframe.canframework.zjaddr_3  = zjnum;
            sendmidframe.canframework.dir_1     = 1;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE   = 1;
            pro_p->can_protocol_send(sendmiddata);
            break;
        case CMD_SEND_PARACORRECT:
            sendmidframe.canframework.devaddr_8 = dest;
            sendmidframe.canframework.func_5    = func;
            sendmidframe.canframework.csaddr_2  = csnum;
            sendmidframe.canframework.zjaddr_3  = zjnum;
            sendmidframe.canframework.dir_1     = 1;
            if (dest > 0x200)
            {
                sendmidframe.canframework.csaddr_2 = 3;
                sendmidframe.canframework.zjaddr_3 = 1;
            }
            else if (dest > 0x100)
            {
                sendmidframe.canframework.csaddr_2 = 2;
                sendmidframe.canframework.zjaddr_3 = 1;
            }

            sendmiddata.ExtId   = sendmidframe.canframeid;
            sendmiddata.IDE     = 1;
            sendmiddata.DLC     = 2;
            sendmiddata.Data[0] = 0;
            sendmiddata.Data[1] = 0;
            pro_p->candrip->write_send_data(sendmiddata);
            break;
        case CMD_SEND_CFGPARAM:
        {
            CANPROHEAD *midinfop                = NULL;
            sendmidframe.canframework.devaddr_8 = dest;
            sendmidframe.canframework.func_5    = func;
            sendmidframe.canframework.csaddr_2  = csnum;
            sendmidframe.canframework.zjaddr_3  = zjnum;
            sendmidframe.canframework.dir_1     = 1;
            if (dest > 0x200)
            {
                sendmidframe.canframework.csaddr_2 = 3;
                sendmidframe.canframework.zjaddr_3 = 1;
            }
            else if (dest > 0x100)
            {
                sendmidframe.canframework.csaddr_2 = 2;
                sendmidframe.canframework.zjaddr_3 = 1;
            }

            if ((midinfop = pro_p->get_protocol_frameinfo(func, 1)) != NULL)
            {
                midinfop->overtime = csmacorder * 10000;
                midinfop->overtime = cszjorder[0] * 10000;
            }

            sendmiddata.IDE = 1;
            send_data_ttlproc(sendmidframe, sendmiddata, data, size);
        }
        break;
        case 12:
        case CMD_SEND_ERRREPORT_ACK:
            sendmidframe.canframework.devaddr_8 = dest;
            sendmidframe.canframework.func_5    = func;
            sendmidframe.canframework.csaddr_2  = csnum;
            sendmidframe.canframework.zjaddr_3  = zjnum;
            sendmidframe.canframework.dir_1     = 1;
            sendmidframe.canframework.ack_1     = 1;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE   = 1;
            sendmiddata.DLC   = 1;
            memcpy(sendmiddata.Data, data, sendmiddata.DLC);
            pro_p->candrip->write_send_data(sendmiddata);
            break;
        case CMD_SEND_AUTOREPORT_ACK:
        case CMD_SEND_BROKENTAIL:
            sendmidframe.canframework.devaddr_8 = dest;
            sendmidframe.canframework.func_5    = func;
            sendmidframe.canframework.csaddr_2  = csnum;
            sendmidframe.canframework.zjaddr_3  = zjnum;
            sendmidframe.canframework.dir_1     = 1;
            sendmidframe.canframework.ack_1     = 1;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE   = 1;
            sendmiddata.DLC   = size;
            memcpy(sendmiddata.Data, data, sendmiddata.DLC);
            pro_p->candrip->write_send_data(sendmiddata);
            break;
        case 15:
            sendmidframe.canframework.devaddr_8 = 255;
            sendmidframe.canframework.func_5    = func;
            sendmidframe.canframework.csaddr_2  = csnum;
            sendmidframe.canframework.zjaddr_3  = zjnum;
            sendmidframe.canframework.dir_1     = 1;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE   = 1;
            sendmiddata.DLC   = 0;
            pro_p->can_protocol_send(sendmiddata);
            break;
        case CMD_REV_LOCK_LOCATION_ACK:
            sendmidframe.canframework.devaddr_8 = dest;
            sendmidframe.canframework.func_5    = func;
            sendmidframe.canframework.csaddr_2  = csnum;
            sendmidframe.canframework.zjaddr_3  = zjnum;
            sendmidframe.canframework.dir_1     = 1;
            sendmidframe.canframework.ack_1     = 1;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE   = 1;
            sendmiddata.DLC   = 3;
            memcpy(sendmiddata.Data, data, sendmiddata.DLC);
            pro_p->candrip->write_send_data(sendmiddata);    // 不需要应答
            break;
        case CMD_SEND_READFUNC_HIGH:
        {
            CANPROHEAD *midinfop = NULL;

            if ((midinfop = pro_p->get_protocol_frameinfo(func, 1)) != NULL)
                midinfop->overtime = (dest - 1) * 10 + 150;
            sendmidframe.canframework.devaddr_8 = dest;
            sendmidframe.canframework.func_5    = func;
            sendmidframe.canframework.csaddr_2  = csnum;
            sendmidframe.canframework.zjaddr_3  = zjnum;
            sendmidframe.canframework.dir_1     = 1;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE   = 1;
            sendmiddata.DLC   = size;
            memcpy(sendmiddata.Data, data, sendmiddata.DLC);
            pro_p->can_protocol_send(sendmiddata);
        }
        break;
        case CMD_SEND_FOUTPUT:
        case CMD_SEND_SWOUTPUT:
        case 21:
        case 23:
        {
            CANPROHEAD *midinfop = NULL;

            if ((midinfop = pro_p->get_protocol_frameinfo(func, 1)) != NULL)
                midinfop->overtime = (dest - 1) * 10 + 150;
            sendmidframe.canframework.devaddr_8 = dest;
            sendmidframe.canframework.func_5    = func;
            sendmidframe.canframework.csaddr_2  = csnum;
            sendmidframe.canframework.zjaddr_3  = zjnum;
            sendmidframe.canframework.dir_1     = 1;

            sendmiddata.IDE = 1;

            send_data_ttlproc(sendmidframe, sendmiddata, data, size);
        }
        break;
            //        case CMD_SEND_WORK_DEV_CHECK:
            //            sendmidframe.canframework.devaddr_8 = 255;
            //            sendmidframe.canframework.func_5    = func;
            //            sendmidframe.canframework.csaddr_2  = csnum;
            //            sendmidframe.canframework.zjaddr_3  = zjnum;
            //            sendmidframe.canframework.dir_1     = 1;

            //            sendmiddata.ExtId = sendmidframe.canframeid;
            //            sendmiddata.IDE   = 1;
            //            pro_p->can_protocol_send(sendmiddata);
            //            break;
        case CMD_SEND_BREAK_CHECK:
            sendmidframe.canframework.devaddr_8 = 255;
            sendmidframe.canframework.func_5    = func;
            sendmidframe.canframework.csaddr_2  = csnum;
            sendmidframe.canframework.zjaddr_3  = zjnum;
            sendmidframe.canframework.dir_1     = 1;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE   = 1;
            pro_p->can_protocol_send(sendmiddata);
            break;
        case CMD_SEND_HEARTCHECK:
        {
            sendmidframe.canframework.devaddr_8 = dest;
            sendmidframe.canframework.func_5    = func;
            sendmidframe.canframework.csaddr_2  = csnum;
            sendmidframe.canframework.zjaddr_3  = zjnum;
            if (dest > 0x200)
            {
                sendmidframe.canframework.csaddr_2 = 3;
                sendmidframe.canframework.zjaddr_3 = 1;
            }
            else if (dest > 0x100)
            {
                sendmidframe.canframework.csaddr_2 = 2;
                sendmidframe.canframework.zjaddr_3 = 1;
            }
            sendmidframe.canframework.dir_1 = 1;

            sendmiddata.ExtId = sendmidframe.canframeid;
            sendmiddata.IDE   = 1;

            if (ndev_map.val(0).id == dest)
            {
                // printf("heartout 04\r\n");
                heartcout = 0;
            }
            heart_check_last_id = 0;
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
bool cs_can::is_have_dev(int zjnum, int csnum, int devid, int &order)
{
    // devnum
    if (zjnum)
    {
        devid |= 0x100 << (csnum - 2);
    }
    for (int i = 0; i < cf_devnum; i++)
    {
        if (nconfig_map.val(i).para.id == devid)
        {
            order = i;
            return TRUE;
        }
    }
    return FALSE;
}

bool cs_can::is_have_config_slaveio(int zjnum, int csnum, int slaveio_order, int &order)
{
    if (zjnum)
    {
        slaveio_order |= FATHER_DEV_MAX << (csnum - 2);
    }
    for (int i = 0; i < cf_devnum; i++)
    {
        if (nconfig_map.val(i).slaveio_order == slaveio_order)
        {
            order = i;
            return TRUE;
        }
        order = i;
    }
    return FALSE;
}

/***********************************************************************************
 * 函数名：is_have_mac_dev
 * 功能：该设备是否存在
 ***********************************************************************************/
bool cs_can::is_have_mac_dev(int paraid, int &order)
{
    if (ndev_map.size( ) == 0)
    {
        zprintf1("map size is 0\r\n");
        return FALSE;
    }
    //   zprintf1("0000000nedv_map = %d, csmacorder = %d\r\n", ndev_map.size(), csmacorder);
    for (int i = 0; i < csmacorder; i++)
    {
        if (ndev_map.val(i).id == paraid)
        {
            order = i;
            return TRUE;
        }
    }
    return FALSE;
}

bool cs_can::is_have_mac_dev(int zjnum, int csnum, int devid, int &order)
{
    if (zjnum)
    {
        devid |= 0x100 << (csnum - 2);
    }
    if (ndev_map.size( ) == 0)
    {
        return FALSE;
    }

    for (int i = 0; i < csmacorder; i++)
    {
        if (ndev_map.val(i).id == devid)
        {
            order = i;
            return TRUE;
        }
    }
    return FALSE;
}
/***********************************************************************************
 * 函数名：is_have_check_dev
 * 功能：该设备是否存在
 ***********************************************************************************/
bool cs_can::is_have_check_dev(int devid, int &order)
{
    if (devid > cszjorder[0] + cszjorder[1])
    {
        devid -= cszjorder[0] + cszjorder[1];
        devid |= 0x200;
    }
    else if (devid > cszjorder[0])
    {
        devid -= cszjorder[0];
        devid |= 0x100;
    }

    if (ndev_map.size( ) == 0)
    {
        return FALSE;
    }

    for (int i = 0; i < csmacorder; i++)
    {
        if (ndev_map.val(i).id == devid)
        {
            order = i;
            return TRUE;
        }
    }
    return FALSE;
}

/***********************************************************************************
 * 函数名：is_have_config_dev
 * 功能：该设备是否存在
 ***********************************************************************************/
bool cs_can::is_have_config_dev(int devid, int &order)
{
    if (devid > cszjorder[0] + cszjorder[1] + cszjorder[2])
    {
        return FALSE;
    }
    else if (devid > cszjorder[0] + cszjorder[1])
    {
        devid -= cszjorder[0] + cszjorder[1];
        devid |= 0x200;
    }
    else if (devid > cszjorder[0])
    {
        devid -= cszjorder[0];
        devid |= 0x100;
    }
    //    printf("==========csioorder = %d==========\r\n",csioorder);
    for (int i = 0; i < 255; i++)
    {
        //        printf("i = %d para id = %d  type = %x devid
        //        =%d\r\n",i,nconfig_map.val(i).para.id,nconfig_map.val(i).para.type, devid);
        if (nconfig_map.val(i).para.id == devid)
        {
            //            printf("=======return==========\r\n");
            order = i;
            return TRUE;
        }
    }

    //     printf("=======end=========\r\n");
    return FALSE;
}
/***********************************************************************************
 * 函数名：get_branch
 * 功能：该设备是否存在
 ***********************************************************************************/
CS_BRANCH_TYPE cs_can::get_branch(CANFRAMEID rxmidframe)
{
    CS_BRANCH_TYPE branch;
    if (rxmidframe.canframework.zjaddr_3)
    {
        branch = (CS_BRANCH_TYPE)(rxmidframe.canframework.csaddr_2 - 1);
    }
    else
    {
        branch = BRANCH_SELF;
    }
    return branch;
}

/***********************************************************************************
 * 函数名：power_resetovertime_process
 * 功能：上电复位超时处理帧
 ***********************************************************************************/
int power_resetovertime_process(void *pro1030, CANDATAFORM overmeg)
{
    cs_can *csrxcanp = static_cast<cs_can *>(pro1030);
    Q_UNUSED(overmeg);
    if (csrxcanp == NULL)
    {
        return 0;
    }
    csrxcanp->csstate = CS_CAN_RESET;
    sem_post(&csrxcanp->statechg);
    return 1;
}
/***********************************************************************************
 * 函数名：mac_inquire_ack_proc
 * 功能：mac查询应答帧处理
 ***********************************************************************************/
int mac_inquire_ack_proc(void *pro1030, CANDATAFORM rxmeg)
{
    cs_can        *csrxcanp = static_cast<cs_can *>(pro1030);
    int            devid = 0, config_devid = 0, slave_io_num_tmp;
    CS_BRANCH_TYPE branch;
    CANFRAMEID     rxmidframe, checkframe;
    CANPRODATA    *rxprodata = NULL;
    uint8_t        get_dev_addr, get_zj_index, get_cs_index;
    uint8_t        error_status[2] = { 0 };
    uint16_t       get_boot_v, get_app_v;
    tbyte_swap((uint16_t *)rxmeg.Data, rxmeg.DLC);

    rxmidframe.canframeid = rxmeg.ExtId;
    branch                = csrxcanp->get_branch(rxmidframe);

    get_dev_addr = rxmidframe.canframework.devaddr_8;
    get_zj_index = rxmidframe.canframework.zjaddr_3;
    get_cs_index = rxmidframe.canframework.csaddr_2;

    if (csrxcanp->csstate == CS_CAN_RESET)
    {
        if ((csrxcanp->cszjorder[branch] + 1) != get_dev_addr && csrxcanp->csdevtyle == 0)
        {
            zprintf1("mac MAC_ORDER_ERR, csmacorder = %d cszjorder =  %d, %d, %d, \
                        zjaddr_3 = %d csaddr_2 = %d devaddr_8 = %d\n",
                     csrxcanp->csmacorder, csrxcanp->cszjorder[0], csrxcanp->cszjorder[1], csrxcanp->cszjorder[2],
                     get_zj_index, get_cs_index, get_dev_addr);
            csrxcanp->csmacstate = MAC_ORDER_ERR;
            return 0;
        }
        if (csrxcanp->csmacstate == MAC_NORMAL)
        {
            int paraid, dev_off;

            uint16_t devtype = 0;

            rxmeg.Data[5] = 0;
            memcpy(&devtype, &rxmeg.Data[4], 2);

            if (get_zj_index)
            {
                paraid  = (csrxcanp->cszjorder[get_cs_index - 1] + 1) | (0x100 << (get_cs_index - 2));
                dev_off = (csrxcanp->cszjorder[get_cs_index - 1] + 1) | (FATHER_DEV_MAX << (get_cs_index - 2));
                csrxcanp->state_info.set_dev_type(
                    get_cs_index - 1, csrxcanp->cszjorder[get_cs_index - 1], rxmeg.Data[4]);
            }
            else
            {
                paraid     = csrxcanp->cszjorder[branch] + 1;
                dev_off    = csrxcanp->cszjorder[branch] + 1;
                get_boot_v = rxmeg.Data[1] << 8 | rxmeg.Data[0];
                get_app_v  = rxmeg.Data[3] << 8 | rxmeg.Data[2];
                csrxcanp->state_info.set_dev_type(branch, csrxcanp->cszjorder[branch], rxmeg.Data[4]);
                csrxcanp->state_info.set_dev_version_state(branch, csrxcanp->cszjorder[branch], get_boot_v, get_app_v);
            }

            if ((rxmeg.Data[1] & 0xf0) == 0x00)
            {
                error_status[0] = get_dev_addr;
                while (1)
                {
                    csrxcanp->nconfig_map.val(0).dev_send_meg(DEV_PROGRAM_ERROR, error_status, sizeof(error_status));
                    sleep(1);
                }
                csrxcanp->csmacstate = MAC_KERNEL_ERR;
            }
            if (csrxcanp->is_have_mac_dev(paraid, devid) == false)
            {
                can_dev_para temppara;
                temppara.id   = paraid;
                temppara.type = devtype;
                csrxcanp->ndev_map.insert(pair< int, can_dev_para >(csrxcanp->csmacorder, temppara));
                devid = csrxcanp->ndev_map.val(csrxcanp->csmacorder).id;
            }
            if ((devtype == CS_DEV) || (devtype == TK236_IOModule_Salve) || (devtype == DEV_256_MODBUS_LOCK) || (devtype == DEV_256_IO_PHONE) || (devtype == TERMINAL))
            {
                csrxcanp->csioorder++;
                if (get_zj_index)
                {
                    slave_io_num_tmp = ++csrxcanp->slave_io_num[branch] | FATHER_DEV_MAX << (get_cs_index - 2);
                    csrxcanp->state_info.set_dev_ID(
                        branch, csrxcanp->cszjorder[get_cs_index - 1], slave_io_num_tmp % 0x100);
                }
                else
                {
                    slave_io_num_tmp = ++csrxcanp->slave_io_num[branch];
                    csrxcanp->state_info.set_dev_ID(branch, csrxcanp->cszjorder[branch], slave_io_num_tmp % 0x100);
                }
                if (csrxcanp->is_have_config_slaveio(
                        get_zj_index, get_cs_index, slave_io_num_tmp, config_devid))    // 存在该设备的配置
                {
                    csrxcanp->nconfig_map.val(config_devid).para.id       = paraid;
                    csrxcanp->nconfig_map.val(config_devid).zjnum         = get_zj_index;
                    csrxcanp->nconfig_map.val(config_devid).csnum         = get_cs_index;
                    csrxcanp->nconfig_map.val(config_devid).slaveio_order = slave_io_num_tmp;
                    csrxcanp->nconfig_map.val(config_devid).dev_off       = dev_off;
                    if (csrxcanp->nconfig_map.val(config_devid).para.type != devtype)
                    {
                        if (csrxcanp->nconfig_map.val(config_devid).para.name.size( ) == 0)
                        {
                            csrxcanp->nconfig_map.val(config_devid).para.type = devtype;
                            csrxcanp->add_default_dev(
                                get_zj_index, get_cs_index, get_dev_addr, slave_io_num_tmp, devtype);
                            if (csrxcanp->is_have_dev(get_zj_index, get_cs_index, get_dev_addr, config_devid))
                            {
                                memcpy(&csrxcanp->nconfig_map.val(config_devid).cscrc, &rxmeg.Data[6],
                                       sizeof(csrxcanp->nconfig_map.val(config_devid).cscrc));
                                memcpy(csrxcanp->nconfig_map.val(config_devid).csmac, rxmeg.Data, 6);
                            }
                        }
                        else
                        {
                            // printf("compare data = %s\r\n",csrxcanp->nconfig_map.val( config_devid
                            // ).para.name.c_str()); printf("compare = %d\r\n",csrxcanp->nconfig_map.val( config_devid
                            // ).para.name.find("Module"));
                            switch (devtype)
                            {
                                case DEV_256_MODBUS_LOCK:
                                case DEV_256_IO_PHONE:
                                    // 文件中本身有该设备，第一次复位因为数量不符，该设备被配置成终端
                                    // 第二次复位时，又改为了该设备
                                    if (csrxcanp->nconfig_map.val(config_devid).para.name.find("Lock") > 0)
                                    {
                                        csrxcanp->nconfig_map.val(config_devid).para.type = devtype;
                                        memcpy(&csrxcanp->nconfig_map.val(config_devid).cscrc, &rxmeg.Data[6],
                                               sizeof(csrxcanp->nconfig_map.val(devid).cscrc));
                                        memcpy(csrxcanp->nconfig_map.val(config_devid).csmac, rxmeg.Data, 6);
                                    }
                                    else
                                    {
                                        error_status[0] = config_devid;
                                        error_status[1] = devtype;
                                        while (1)
                                        {
                                            csrxcanp->nconfig_map.val(0).dev_send_meg(
                                                CONFIG_ERROR, error_status, sizeof(error_status));
                                            zprintf1("Need new config para config_devid 2 = %d!\n", config_devid);
                                            sleep(1);
                                        }
                                        return -1;
                                    }
                                    break;
                                case TK236_IOModule_Salve:
                                    if (csrxcanp->nconfig_map.val(config_devid).para.name.find("Module") > 0)
                                    {
                                        csrxcanp->nconfig_map.val(config_devid).para.type = devtype;
                                        memcpy(&csrxcanp->nconfig_map.val(config_devid).cscrc, &rxmeg.Data[6],
                                               sizeof(csrxcanp->nconfig_map.val(devid).cscrc));
                                        memcpy(csrxcanp->nconfig_map.val(config_devid).csmac, rxmeg.Data, 6);
                                    }
                                    else
                                    {
                                        error_status[0] = config_devid;
                                        error_status[1] = devtype;
                                        while (1)
                                        {
                                            csrxcanp->nconfig_map.val(0).dev_send_meg(
                                                CONFIG_ERROR, error_status, sizeof(error_status));
                                            zprintf1("Need new config para config_devid 1 = %d!\n", config_devid);
                                            sleep(1);
                                        }
                                        return -1;
                                    }
                                    break;
                                case TERMINAL:
                                    csrxcanp->nconfig_map.val(config_devid).para.type = devtype;
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                    else
                    {
                        memcpy(&csrxcanp->nconfig_map.val(config_devid).cscrc, &rxmeg.Data[6],
                               sizeof(csrxcanp->nconfig_map.val(devid).cscrc));
                        memcpy(csrxcanp->nconfig_map.val(config_devid).csmac, rxmeg.Data, 6);
                    }
                    checkframe                        = rxmidframe;
                    checkframe.canframework.devaddr_8 = 255;
                    checkframe.canframework.csaddr_2  = 0;
                    checkframe.canframework.zjaddr_3  = 0;
                    checkframe.canframework.dir_1     = 1;
                    checkframe.canframework.ack_1     = 0;
                    rxprodata                         = csrxcanp->pro_p->read_deldata_buf(checkframe.canframeid, 1);
                    if (rxprodata == NULL)
                    {
                        zprintf1("error: mac rxprodata is NULL! 1\n");
                        csrxcanp->csmacstate = MAC_KERNEL_ERR;
                        return -2;
                    }
                    rxprodata->runtime = 0;
                }
                else    // 没有该设备的配置信息，添加默认配置
                {
                    zprintf1("error: slave is no math\r\n");
                    csrxcanp->add_default_dev(get_zj_index, get_cs_index, get_dev_addr, slave_io_num_tmp, devtype);
                    if (csrxcanp->is_have_dev(get_zj_index, get_cs_index, get_dev_addr, config_devid))
                    {
                        memcpy(&csrxcanp->nconfig_map.val(config_devid).cscrc, &rxmeg.Data[6],
                               sizeof(csrxcanp->nconfig_map.val(config_devid).cscrc));
                        memcpy(csrxcanp->nconfig_map.val(config_devid).csmac, rxmeg.Data, 6);
                    }
                }
            }
            csrxcanp->csmacorder++;
            csrxcanp->cszjorder[branch]++;
            csrxcanp->set_dev_status(csrxcanp->csmacorder, DEV_ON_LINE);
            if (devtype == CS_DEV)
            {
                int     cs_num  = 0;
                uint8_t tmp_key = get_cs_index + (get_zj_index << 2);
                csrxcanp->mac_cszd_have.val(tmp_key).mac_cs_have++;
                cs_num               = csrxcanp->mac_cszd_have.val(0).mac_cs_have + csrxcanp->mac_cszd_have.val(6).mac_cs_have + csrxcanp->mac_cszd_have.val(7).mac_cs_have;
                csrxcanp->mac_cs_num = cs_num;
            }
            else if (devtype == TERMINAL)
            {
                int     zd_num                                         = 0;
                uint8_t tmp_key                                        = get_cs_index + (get_zj_index << 2);
                csrxcanp->mac_cszd_have.val(tmp_key).mac_terminal_have = 1;
                zd_num                                                 = csrxcanp->mac_cszd_have.val(0).mac_terminal_have + csrxcanp->mac_cszd_have.val(6).mac_terminal_have + csrxcanp->mac_cszd_have.val(7).mac_terminal_have;
                csrxcanp->mac_terminal_num                             = zd_num;

                checkframe                        = rxmidframe;
                checkframe.canframework.devaddr_8 = 255;
                checkframe.canframework.csaddr_2  = 0;
                checkframe.canframework.zjaddr_3  = 0;
                checkframe.canframework.dir_1     = 1;
                checkframe.canframework.ack_1     = 0;
                rxprodata                         = csrxcanp->pro_p->read_deldata_buf(checkframe.canframeid, 1);
                if (rxprodata != NULL)
                {
                    zprintf1("delet dev type %d\n", devtype);
                }
            }
        }
    }
#ifdef END_NO_RESET
    else if (csrxcanp->csstate == CS_CAN_NORMAL)
    {
        rxmeg.Data[5]    = 0;
        uint16_t devtype = 0;
        memcpy(&devtype, &rxmeg.Data[4], 2);
        if (devtype == TERMINAL)
        {
            if (csrxcanp->mac_cszd_have.val(0).mac_terminal_have == 0)
            {
                csrxcanp->state_info.set_tail_location(branch, 0);
                csrxcanp->mac_cszd_have.val(0).mac_terminal_have = 1;
                csrxcanp->add_default_dev(get_zj_index, get_cs_index, get_dev_addr, 0, devtype);
            }
            csrxcanp->cut_location_shake[0] = 0;
            csrxcanp->cut_location_shake[1] = 0;
            // TODO: zj

            //            zprintf1("ndev_map first num = %d\r\n", csrxcanp->ndev_map.size());
            //
            if (csrxcanp->is_have_mac_dev(get_dev_addr, devid))
            {
                zprintf1("zty have mac:%d!\n", get_dev_addr);
                csrxcanp->ndev_map.val(get_dev_addr - 1).id   = get_dev_addr;
                csrxcanp->ndev_map.val(get_dev_addr - 1).type = devtype;
            }
            else
            {
                can_dev_para temppara_1;
                temppara_1.id   = get_dev_addr;
                temppara_1.type = devtype;
                csrxcanp->ndev_map.insert(pair< int, can_dev_para >(get_dev_addr - 1, temppara_1));
                csrxcanp->csmacorder++;
                csrxcanp->cszjorder[branch]++;
                zprintf1("zty 11 have mac:%d!\n", get_dev_addr);
            }
            //            zprintf1("ndev_map num = %d\r\n", csrxcanp->ndev_map.size());
            //            for (int i = 0; i < get_dev_addr; i++)
            //            {
            //                zprintf1("mac ckeck num:%d, id:%d, type:%d\r\n", i, csrxcanp->ndev_map.val(i).id,
            //                    csrxcanp->ndev_map.val(i).type);
            //            }
            csrxcanp->state_info.set_dev_type(0, get_dev_addr - 1, rxmeg.Data[4]);
            // zprintf1("set normal 03 mac check\r\n");

            get_boot_v = rxmeg.Data[1] << 8 | rxmeg.Data[0];
            get_app_v  = rxmeg.Data[3] << 8 | rxmeg.Data[2];
            csrxcanp->state_info.set_dev_version_state(0, get_dev_addr, get_boot_v, get_app_v);
            csrxcanp->cs_send_data(CMD_SEND_PARACORRECT, get_zj_index, get_cs_index, get_dev_addr, NULL, 0);
        }
    }
#endif
    return 0;
}
/***********************************************************************************
 * 函数名：macreqdata_overtimeproc
 * 功能：mac查询超时处理函数
 ***********************************************************************************/
int macreqdata_overtimeproc(void *pro1030, CANDATAFORM overmeg)
{
    cs_can *csrxcanp = static_cast<cs_can *>(pro1030);
    Q_UNUSED(overmeg);
    zprintf1("mac check time over1111111111111111\n");
    if (csrxcanp->csstate != CS_CAN_NORMAL)
    {
        zprintf1("mac check time over\n");
        csrxcanp->csstate = CS_CAN_MACREQ;
        sem_post(&csrxcanp->statechg);
    }
    if (csrxcanp->csmacorder == 0)
    {
        csrxcanp->state_info.set_termal_vol(0, 0);
        csrxcanp->state_info.set_line_work_state(0, CS_WORK_STATUS_LIVEOUT);
    }
    //    printf("==============mac====================\r\n");
    //    printf("csmacorder == %d\r\n", csrxcanp->csmacorder);
    //    printf("ndev_map.size() == %d\r\n", csrxcanp->ndev_map.size());
    //    for (int i = 0; i < csrxcanp->csmacorder; i++)
    //    {
    //        printf("ndev_map.val(%d).type is %d \r\n", i, csrxcanp->ndev_map.val(i).type);
    //        printf("ndev_map.val(%d).id is %d \r\n", i, csrxcanp->ndev_map.val(i).id);
    //    }
    //    printf("=============mac======================\r\n");
    return 0;
}

int enter_workmode_frameproc(void *pro1030, CANDATAFORM rxmeg)
{
    Q_UNUSED(pro1030);
    Q_UNUSED(rxmeg);
    // printf("!!!!!!!!!!!!!!!!enter work~~~~~~~~~~~~~~~~~~~~\r\n");
    return 0;
}
/***********************************************************************************
 * 函数名：int configdata_ack_proc(int csnum, CANDATAFORM  rxmeg)
 * 功能：配置数据应答帧处理
 ***********************************************************************************/
int configdata_ack_proc(void *pro1030, CANDATAFORM rxmeg)
{
    int        soureid;
    CANFRAMEID rxmidframe;
    CANFRAMEID checkframe;
    cs_can    *csrxcanp = static_cast<cs_can *>(pro1030);

    if (csrxcanp == NULL)
        return -1;

    rxmidframe.canframeid = rxmeg.ExtId;

    if (csrxcanp->is_have_dev(rxmidframe.canframework.zjaddr_3, rxmidframe.canframework.csaddr_2,
                              rxmidframe.canframework.devaddr_8, soureid)
        == FALSE)
        return -2;
    if (csrxcanp->nconfig_map.val(soureid).para.id == csrxcanp->cf_endnum)
    {
        checkframe                        = rxmidframe;
        checkframe.canframework.devaddr_8 = csrxcanp->nconfig_map.val(0).para.id;

        checkframe.canframework.dir_1  = 1;
        checkframe.canframework.ack_1  = 0;
        checkframe.canframework.next_1 = 1;

        csrxcanp->pro_p->pro_del_buf_frame(checkframe.canframeid, 1);
        csrxcanp->cs_send_data(CMD_SEND_PARACORRECT, 0x07, 0, 255, NULL, 0);
        csrxcanp->csstate = CS_CAN_NORMAL;
        sem_post(&csrxcanp->statechg);
    }
    return 0;
}
/***********************************************************************************
 * 函数名：configdata_overtimeproc
 * 功能：配置数据超时处理           //有问题
 ***********************************************************************************/
int configdata_overtimeproc(void *pro1030, CANDATAFORM rxmeg)
{
    (void)rxmeg;
    cs_can    *csrxcanp = static_cast<cs_can *>(pro1030);
    // CANFRAMEID rxmidframe;
    // rxmidframe.canframeid = rxmeg.ExtId;
    if (csrxcanp == NULL)
    {
        return -1;
    }

    csrxcanp->send_configdata( );
    return 0;
}
/***********************************************************************************
 * 函数名：output_controlack_frameproc
 * 功能：频率输出控制应答帧处理
 ***********************************************************************************/
int foutput_controlack_frameproc(void *pro1030, CANDATAFORM rxmeg)
{
    CANFRAMEID rxmidframe;
    CANFRAMEID checkframe;
    int        soureid;

    cs_can *csrxcanp = static_cast<cs_can *>(pro1030);

    if (csrxcanp == NULL)
        return 0;

    rxmidframe.canframeid = rxmeg.ExtId;

    //    soureid = rxmidframe.canframework.devaddr_8;
    csrxcanp->is_have_dev(
        rxmidframe.canframework.zjaddr_3, rxmidframe.canframework.csaddr_2, rxmidframe.canframework.devaddr_8, soureid);
    checkframe = rxmidframe;

    checkframe.canframework.dir_1               = 1;
    checkframe.canframework.ack_1               = 0;
    rxmeg.ExtId                                 = checkframe.canframeid;
    csrxcanp->nconfig_map.val(soureid).errcount = 0;
    if (csrxcanp->pro_p->pro_del_buf_frame(rxmeg) != 0)
    {
        zprintf1("f output control delete error!\n");
        // error
    }
    else
    {
        uint16_t midval = 0;
        zprintf1("f output control!\n");
        //        memcpy(&midval, &rxmeg.Data[1], 2);
        midval = rxmeg.Data[1] * 0x100 + rxmeg.Data[2];
        csrxcanp->data_p->set_out_ack_value(csrxcanp->nconfig_map.val(soureid).slaveio_order - 1, 0,
                                            csrxcanp->nconfig_map.val(soureid).para.innum + rxmeg.Data[0], midval);
    }
    return 1;
}

/***********************************************************************************
 * read_reg_entry
 * 功能：读寄存器应答帧处理
 ***********************************************************************************/
void read_reg_entry(cs_can *csrxcanp, CANDATAFORM rxmeg)
{
    // uint16_t   devtyle = 0;
    uint8_t    branch;
    int        sourceid;
    CANFRAMEID rxmidframe;
    rxmidframe.canframeid = rxmeg.ExtId;
    if (rxmidframe.canframework.zjaddr_3)
    {
        branch = rxmidframe.canframework.csaddr_2 - 1;
    }
    else
    {
        branch = 0;
    }

    if (csrxcanp->is_have_mac_dev(rxmidframe.canframework.zjaddr_3, rxmidframe.canframework.csaddr_2,
                                  rxmidframe.canframework.devaddr_8, sourceid))
    {
        uint16_t devtyle = csrxcanp->ndev_map.val(sourceid).type;
        if (devtyle == CS_DEV)
        {
            return;
        }
        else
        {
            if (rxmidframe.canframework.ttl_7 == 0)
            {
                if (rxmeg.DLC == 4)
                {
                    memcpy(&(csrxcanp->nconfig_map.val(sourceid).recv_reg_begin), rxmeg.Data,
                           sizeof(csrxcanp->nconfig_map.val(sourceid).recv_reg_begin));
                    memcpy(&(csrxcanp->nconfig_map.val(sourceid).recv_reg_offset), rxmeg.Data + 2,
                           sizeof(csrxcanp->nconfig_map.val(sourceid).recv_reg_offset));
                }
            }
            else
            {
                if ((rxmidframe.canframework.ttl_7 - 1) * 8 + rxmeg.DLC > csrxcanp->nconfig_map.val(sourceid).recv_reg_offset)
                {
                    zprintf1("error:read reg offser is out \r\n");
                    return;
                }
                if (csrxcanp->nconfig_map.val(sourceid).recv_reg_begin + csrxcanp->nconfig_map.val(sourceid).recv_reg_offset > READ_REG_MAX_NUM)
                {
                    zprintf1("error:read reg total is out \r\n");
                    return;
                }
                memcpy(&(csrxcanp->nconfig_map.val(sourceid).recv_reg_data), &rxmeg.Data, rxmeg.DLC);
            }
        }
        if (rxmidframe.canframework.next_1 == 0)
        {
            if (csrxcanp->nconfig_map.val(sourceid).recv_reg_begin == 0x4C)
            {
                uint8_t value_v;
                uint8_t value_c;
                value_v = rxmeg.Data[0];
                value_c = rxmeg.Data[2] | rxmeg.Data[3] << 8;
                csrxcanp->state_info.set_dev_cv_state(branch, sourceid, value_v, value_c);
            }
        }
    }
    else
    {
        zprintf1("error:can not find dev %d\r\n", rxmidframe.canframework.devaddr_8);
    }
}

/***********************************************************************************
 * readreg_callback_frameproc
 * 功能：读寄存器应答帧处理
 ***********************************************************************************/
int readreg_callback_frameproc(void *pro1030, CANDATAFORM rxmeg)
{
    cs_can *csrxcanp = static_cast<cs_can *>(pro1030);

    if (csrxcanp == NULL)
    {
        return -1;
    }

    read_reg_entry(csrxcanp, rxmeg);

    return 0;
}
/***********************************************************************************
 * 函数名：output_controlack_frameproc
 * 功能：输出控制应答帧处理
 ***********************************************************************************/
int output_controlack_frameproc(void *pro1030, CANDATAFORM rxmeg)
{
    CANFRAMEID rxmidframe;
    CANFRAMEID checkframe;
    int        soureid;

    cs_can *csrxcanp = static_cast<cs_can *>(pro1030);

    if (csrxcanp == NULL)
    {
        return 0;
    }

    if (csrxcanp->csstate != CS_CAN_NORMAL)
    {
        return 0;
    }

    rxmidframe.canframeid = rxmeg.ExtId;
    csrxcanp->is_have_dev(
        rxmidframe.canframework.zjaddr_3, rxmidframe.canframework.csaddr_2, rxmidframe.canframework.devaddr_8, soureid);
    checkframe                                  = rxmidframe;
    checkframe.canframework.dir_1               = 1;
    checkframe.canframework.ack_1               = 0;
    rxmeg.ExtId                                 = checkframe.canframeid;
    csrxcanp->nconfig_map.val(soureid).errcount = 0;
    if (csrxcanp->pro_p->pro_del_buf_frame(rxmeg) != 0)
    {
        zprintf1("switch output control delete error 0x%x data 0x%x!\n", rxmeg.ExtId, rxmeg.Data[0]);
        // error
    }
    else
    {
        zprintf1("switch output control 0x%x data 0x%x!\n", rxmeg.ExtId, rxmeg.Data[0] & 0x03);
        csrxcanp->data_p->set_out_ack_value(csrxcanp->nconfig_map.val(soureid).slaveio_order - 1, 0,
                                            csrxcanp->nconfig_map.val(soureid).para.innum + (rxmeg.Data[0] >> 2), rxmeg.Data[0] & 0x03);
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    // printf("millisecond:%ld\n",tv.tv_sec*1000 + tv.tv_usec/1000);
    return 1;
}
/***********************************************************************************
 * 函数名：output_controlack_overproc
 * 功能：输出控制超时处理
 ***********************************************************************************/
int output_controlack_overproc(void *pro1030, CANDATAFORM overmeg)
{
    CS_BRANCH_TYPE branch;
    uint8_t        reset_reason[4] = { 0, 0, 0, 0 };
    cs_can        *csrxcanp        = static_cast<cs_can *>(pro1030);
    int            order;
    if (csrxcanp == NULL)
    {
        return -1;
    }
    CANFRAMEID rxmidframe;
    rxmidframe.canframeid = overmeg.ExtId;
    uint8_t get_dev_addr;
    // uint8_t get_zj_index;
    // uint8_t get_cs_index;
    get_dev_addr = rxmidframe.canframework.devaddr_8;
    // get_zj_index = rxmidframe.canframework.zjaddr_3;
    // get_cs_index = rxmidframe.canframework.csaddr_2;
    branch       = csrxcanp->get_branch(rxmidframe);
    csrxcanp->is_have_dev(
        rxmidframe.canframework.zjaddr_3, rxmidframe.canframework.csaddr_2, rxmidframe.canframework.devaddr_8, order);
    csrxcanp->set_dev_status(get_dev_addr, DEV_OFF_LINE);    // 离线
    reset_reason[0] = PTCAN_RESET_IO_NOACK;
    reset_reason[1] = csrxcanp->auto_reset;
    reset_reason[2] = get_dev_addr;
    reset_reason[3] = csrxcanp->state_info.get_dev_type(branch, get_dev_addr);
    if (csrxcanp->auto_reset)
    {
        csrxcanp->nconfig_map.val(0).dev_send_meg(RESET_REASON, reset_reason, sizeof(reset_reason));
    }
    csrxcanp->cs_can_reset_pro( );

    return 0;
}

/***********************************************************************************
 * 函数名：stop_report_frameproc
 * 功能：闭锁上报帧处理
 ***********************************************************************************/
int stop_report_frameproc(void *pro1030, CANDATAFORM rxmeg)
{
    CANFRAMEID rxmidframe;
    cs_can    *csrxcanp = static_cast<cs_can *>(pro1030);

    if (csrxcanp == NULL)
        return 0;
    rxmidframe.canframeid       = rxmeg.ExtId;
    csrxcanp->bs100info.jtstate = rxmeg.Data[0];
    // 待添加急停处理
    csrxcanp->cs_send_data(CMD_SEND_STOPREPORT_ACK, rxmidframe.canframework.zjaddr_3, rxmidframe.canframework.csaddr_2,
                           rxmidframe.canframework.devaddr_8, rxmeg.Data, 1);
    return 1;
}

/***********************************************************************************
 * 函数名：max_stop_report_frameproc
 * 功能：闭锁上报帧处理
 ***********************************************************************************/
int max_stop_report_frameproc(void *pro1030, CANDATAFORM rxmeg)
{
    CANFRAMEID rxmidframe;
    cs_can    *csrxcanp = static_cast<cs_can *>(pro1030);
    int        branch;
    int        soureid;
    int        zjnum;
    int        csnum;

    enum class AutoReportType
    {
        CSStopOK = 0,
        CSStopLock,
        CSStopA18Error = 4,
        CSStopA18OK,
        CSStopB18Error,
        CSStopB18OK
    };

    if (csrxcanp == NULL)
        return 0;

    rxmidframe.canframeid = rxmeg.ExtId;

    soureid = rxmidframe.canframework.devaddr_8;
    zjnum   = rxmidframe.canframework.zjaddr_3;
    csnum   = rxmidframe.canframework.csaddr_2;

    if (zjnum)
    {
        branch = csnum - 1;
    }
    else
    {
        branch = 0;
    }

    csrxcanp->cs_send_data(CMD_SEND_STOPREPORT_ACK, zjnum, csnum, soureid, rxmeg.Data, 1);
    switch ((AutoReportType)rxmeg.Data[0])
    {
        case AutoReportType::CSStopOK:
        case AutoReportType::CSStopLock:
            csrxcanp->bs_type_location_report(rxmeg.Data[0]);
            csrxcanp->state_info.set_bs_type(branch, rxmeg.Data[0]);
            csrxcanp->cs_jt_status = rxmeg.Data[0];
            if (rxmeg.Data[0] == CS_JT_OK)
            {
                //                csrxcanp->state_info.set_tail_location(branch, 0);
                csrxcanp->state_info.set_bs_location(branch, 0);
                csrxcanp->state_info.set_break_location(branch, 0);
            }
            break;
        case AutoReportType::CSStopA18Error:
        case AutoReportType::CSStopA18OK:
        case AutoReportType::CSStopB18Error:
        case AutoReportType::CSStopB18OK:
        {
            uint8_t state, channelNum;
            channelNum = (rxmeg.Data[0] - (uint8_t)AutoReportType::CSStopA18Error) / 2;
            state      = rxmeg.Data[0] & 0x01;
            csrxcanp->state_info.set_line_18v_state(branch, state, channelNum);
        }
        break;
        default:
            break;
    }

    return 1;
}

/***********************************************************************************
 * 函数名：break_location_report_frameproc
 * 功能：段路位置帧处理
 ***********************************************************************************/
int break_location_report_frameproc(void *pro1030, CANDATAFORM rxmeg)
{
    CANFRAMEID rxmidframe;
    cs_can    *csrxcanp = static_cast<cs_can *>(pro1030);
    int        branch;

    if (csrxcanp == NULL)
        return 0;
    rxmidframe.canframeid = rxmeg.ExtId;

    if (rxmidframe.canframework.zjaddr_3)
    {
        branch = rxmidframe.canframework.csaddr_2 - 1;
    }
    else
    {
        branch = 0;
    }
    if (rxmidframe.canframework.func_5 == CMD_SEND_LOCK_LOCATION)
    {
        uint8_t offset = 0;
        uint8_t cmd;
        uint8_t location;
        if ((rxmeg.Data[0] & 0x80) != 0x80)
        {
            offset = 1;
        }
        cmd      = rxmeg.Data[0] & 0x7f;
        location = rxmidframe.canframework.devaddr_8 - offset;
        csrxcanp->cs_send_data(CMD_REV_LOCK_LOCATION_ACK, rxmidframe.canframework.zjaddr_3,
                               rxmidframe.canframework.csaddr_2, rxmidframe.canframework.devaddr_8, rxmeg.Data, 2);
        if (cmd == 0x02)
        {

            csrxcanp->state_info.set_bs_location(branch, location);
            if (csrxcanp->state_info.get_dev_state(location) & 0x04 || csrxcanp->state_info.get_dev_state(location + 1) & 0x04)
            {
                csrxcanp->break_line_pretreatment[branch] = 0;
                //                csrxcanp->state_info.set_break_location(branch, 0);
                return 0;
            }
            csrxcanp->break_line_pretreatment[branch] = location;
            if (csrxcanp->cs_jt_status != CS_JT_OK)
            {
                usleep(50000);
                if (csrxcanp->cs_jt_status != CS_JT_OK)
                {
                    if (!csrxcanp->state_info.get_bs_is_have(branch))
                    {
                        // csrxcanp->state_info.set_break_location(branch, location);
                    }
                }
            }
        }
        if (cmd == 0x03)
        {
            csrxcanp->state_info.set_tail_location(branch, location);
            csrxcanp->state_info.set_bs_location(branch, location);
            csrxcanp->state_info.set_termal_vol(branch, 0);
        }
    }
    return 0;
}

/***********************************************************************************
 * 函数名：auto_report_frameproc
 * 功能：主动上报帧处理
 ***********************************************************************************/
int auto_report_frameproc(void *pro1030, CANDATAFORM rxmeg)
{
    int     soureid;
    int     zjnum;
    int     csnum;
    int     branch;
    uint8_t dev_function = 0;
    enum
    {
        BS_FUNCTION = 0,
        IO_FUNCTION
    };
    CANFRAMEID rxmidframe;
    cs_can    *csrxcanp = static_cast<cs_can *>(pro1030);

    if (csrxcanp == NULL)
        return 0;

    INCOFPARA1 inpara;
    RXINDATA   rxdata;
    gBugState             = 0;
    gBugCanInfo           = rxmeg;
    rxmidframe.canframeid = rxmeg.ExtId;
    soureid               = rxmidframe.canframework.devaddr_8;
    zjnum                 = rxmidframe.canframework.zjaddr_3;
    csnum                 = rxmidframe.canframework.csaddr_2;
    tbyte_swap((uint16_t *)rxmeg.Data, 2);
    if (rxmidframe.canframework.zjaddr_3)
    {
        branch = rxmidframe.canframework.csaddr_2 - 1;
    }
    else
    {
        branch = 0;
    }

    rxdata.rxinval = rxmeg.Data[2];

    if (rxmeg.Data[0] == DEV_256_PHONE || rxmeg.Data[0] == DEV_256_LOCK || rxmeg.Data[0] == CS_DEV || rxmeg.Data[0] == DEV_256_RELAY)
    {
        dev_function |= 1 << BS_FUNCTION;
    }
    else if (rxmeg.Data[0] == TK236_IOModule_Salve)
    {
        dev_function |= 1 << IO_FUNCTION;
    }
    else if (rxmeg.Data[0] == DEV_256_IO_PHONE || rxmeg.Data[0] == DEV_256_MODBUS_LOCK)
    {
        dev_function |= 1 << BS_FUNCTION;
        dev_function |= 1 << IO_FUNCTION;
    }
    if (rxdata.rxinattr.innum5 == 0x1e)
    {
        csrxcanp->state_info.set_voip_state(branch, rxdata.rxinattr.instate2 & 0x01);
    }

    if ((dev_function >> BS_FUNCTION & 0x01) == 0x01 && rxdata.rxinattr.innum5 == 0x1f)
    {
        if (csrxcanp->nconfig_map.val(csrxcanp->slave_io_num[branch] - 1).para.type == TERMINAL)
        {
            csrxcanp->data_p->set_share_data_value(csrxcanp->slave_io_num[branch] - 1,
                                                   rxmidframe.canframework.zjaddr_3 ? rxmidframe.canframework.csaddr_2 - 1 : 0,
                                                   rxmidframe.canframework.devaddr_8, rxdata.rxinattr.instate2 & 0x01);
        }

        csrxcanp->state_info.set_bs_state(
            branch, rxmidframe.canframework.devaddr_8 - 1, rxdata.rxinattr.instate2 & 0x01);
        //        if (!csrxcanp->state_info.get_bs_is_have(branch))
        //        {
        //            csrxcanp->state_info.set_break_location(branch, csrxcanp->break_line_pretreatment[branch]);
        //        }

        csrxcanp->bs_button_report[0] = branch;
        csrxcanp->bs_button_report[1] = rxmidframe.canframework.devaddr_8;
        csrxcanp->bs_button_report[2] = rxdata.rxinattr.innum5;
        csrxcanp->bs_button_report[3] = rxdata.rxinattr.instate2;
        // csrxcanp->nconfig_map.val(0).dev_send_meg(BS_BUTTON_MSG, csrxcanp->bs_button_report, 4);
    }
    if ((dev_function >> IO_FUNCTION & 0x01) == 0x01)
    {
        if (csrxcanp->is_have_dev(zjnum, csnum, soureid, soureid))
        {
            if (csrxcanp->nconfig_map.val(soureid).devenable == 0)
            {
                goto ACK_PROSS;
            }
            if (memcmp(rxmeg.Data, &csrxcanp->nconfig_map.val(soureid).para.type, 1) == 0)
            {
                rxdata.rxinval = rxmeg.Data[2];
                if (rxdata.rxinattr.innum5 != 0x1f)
                {
                    csrxcanp->nconfig_map.val(soureid).dev_normal_process( );
                    if (csrxcanp->nconfig_map.val(soureid).config_p != NULL)
                    {
                        inpara.paraval =
                            csrxcanp->nconfig_map.val(soureid).config_p[INNODEINFO(rxdata.rxinattr.innum5 - 1)];
                        if (rxdata.rxinattr.instyle1 != inpara.inval.instyle1)
                        {
                            zprintf1("%d auto in error, %d, %d, %d\n", soureid, inpara.inval.inenable1,
                                     rxdata.rxinattr.instyle1, inpara.inval.instyle1);
                        }
                        else
                        {
                            if (rxdata.rxinattr.instyle1 == 0)
                            {
                                csrxcanp->data_p->set_share_data_value(
                                    csrxcanp->nconfig_map.val(soureid).slaveio_order - 1,
                                    /*rxmidframe.canframework.csaddr_2*/ 0, rxdata.rxinattr.innum5,
                                    rxdata.rxinattr.instate2);
                            }
                            else
                            {
                                if (rxmeg.DLC < 5)
                                {
                                    zprintf1("%d auto in data error\n", soureid);
                                }
                                gBugState = 1;
                                {
                                    uint16_t devvalue = (rxmeg.Data[3] & 0x1f) << 8;
                                    devvalue += rxmeg.Data[4];

                                    if ((rxmeg.Data[3] & 0x20) != 0x20)
                                    {
                                        devvalue *= 20;
                                    }
                                    csrxcanp->data_p->set_share_data_value(
                                        csrxcanp->nconfig_map.val(soureid).slaveio_order - 1,
                                        /*rxmidframe.canframework.csaddr_2*/ 0, rxdata.rxinattr.innum5, devvalue);
                                }
                                gBugState = 2;
                            }
                        }
                    }
                }
            }
        }
    }
ACK_PROSS:
    //    csrxcanp->nconfig_map.val(soureid).errcount = 0;
    gBugState = 3;
    tbyte_swap((uint16_t *)rxmeg.Data, 2);
    if (rxdata.rxinattr.instyle1 == 0)
    {
        csrxcanp->cs_send_data(CMD_SEND_AUTOREPORT_ACK, rxmidframe.canframework.zjaddr_3,
                               rxmidframe.canframework.csaddr_2, rxmidframe.canframework.devaddr_8, rxmeg.Data, rxmeg.DLC);
    }
    else
    {
        // printf("rxdata.rxinattr.instyle1 == %d\n", rxdata.rxinattr.instyle1);
    }
    gBugState = 4;

    return 1;
}

/***********************************************************************************
 * dev_break_check_callback
 * 功能：断线查询帧接收回调
 ***********************************************************************************/
int dev_break_check_callback(void *pro1030, CANDATAFORM rxmeg)
{
    cs_can        *csrxcanp = static_cast<cs_can *>(pro1030);
    CS_BRANCH_TYPE branch;
    CANFRAMEID     rxmidframe;
    CANFRAMEID     checkframe;
    uint8_t        get_dev_addr, get_zj_index, get_cs_index;
    static uint8_t dc_status_previous = 0;
    uint8_t        dc_status_current  = 0;
    int            slave_io_num_tmp;
    bool           lock_status;
    tbyte_swap((uint16_t *)rxmeg.Data, rxmeg.DLC);
    rxmidframe.canframeid = rxmeg.ExtId;
    (void)slave_io_num_tmp;
    branch = csrxcanp->get_branch(rxmidframe);

    get_dev_addr = rxmidframe.canframework.devaddr_8;
    get_zj_index = rxmidframe.canframework.zjaddr_3;
    get_cs_index = rxmidframe.canframework.csaddr_2;

    if ((csrxcanp->cszjorder_temp[branch] + 1) != get_dev_addr || csrxcanp->csstate != CS_CAN_NORMAL || csrxcanp->reset_state == DEV_RESET_ING)    // 号不连续
    {
        zprintf1("===error===: del 31 cmd with overtime\r\n");
        checkframe                        = rxmidframe;
        checkframe.canframework.devaddr_8 = 255;
        checkframe.canframework.csaddr_2  = 0;
        checkframe.canframework.zjaddr_3  = 0;
        checkframe.canframework.dir_1     = 1;
        checkframe.canframework.ack_1     = 0;
        csrxcanp->pro_p->pro_del_buf_frame(checkframe.canframeid, 1);
        return 0;
    }
    csrxcanp->cszjorder_temp[branch]++;

    if (IS_ONE(csrxcanp->break_send_status, 1))
    {
        if (get_dev_addr == 1)
        {
            dc_status_previous = 0;
        }
        if ((rxmeg.Data[2] >> 2) & 0x01)
        {
            dc_status_current = 1;
        }
        else
        {
            dc_status_current = 0;
        }
        if (dc_status_current == 1 && dc_status_previous == 0)
        {
            csrxcanp->break_location_temp = get_dev_addr;
        }
        dc_status_previous = dc_status_current;
    }
    if (IS_ONE(csrxcanp->break_send_status, 0))
    {
        uint16_t devtype = 0;
        devtype          = rxmeg.Data[1];
        csrxcanp->csmacorder_temp++;
        if (rxmeg.Data[2] & 0x01)
        {
            lock_status = 0;
        }
        else
        {
            lock_status = 1;
        }
        csrxcanp->state_info.set_bs_state(branch, get_dev_addr - 1, lock_status);
        if ((devtype == CS_DEV) || (devtype == TK236_IOModule_Salve) || (devtype == DEV_256_MODBUS_LOCK) || (devtype == DEV_256_IO_PHONE) || (devtype == TERMINAL))
        {
            if (devtype != TERMINAL)
            {
                slave_io_num_tmp = ++csrxcanp->slave_io_num_temp[branch];
                csrxcanp->csioorder_temp++;

                if (get_zj_index)
                {
                    slave_io_num_tmp |= FATHER_DEV_MAX << (get_cs_index - 2);
                }
            }
        }
        if (devtype == CS_DEV)
        {
            int     cs_num  = 0;
            uint8_t tmp_key = get_cs_index + (get_zj_index << 2);
            csrxcanp->mac_cszd_have.val(tmp_key).mac_cs_have_tmp++;
            cs_num                    = csrxcanp->mac_cszd_have.val(0).mac_cs_have_tmp + csrxcanp->mac_cszd_have.val(6).mac_cs_have_tmp + csrxcanp->mac_cszd_have.val(7).mac_cs_have_tmp;
            csrxcanp->mac_cs_num_temp = cs_num;
        }
    }
    return 0;
}

/***********************************************************************************
 * dev_break_check_overtime
 * 功能：断线查询帧接收超时
 ***********************************************************************************/
int dev_break_check_overtime(void *pro1030, CANDATAFORM rxmeg)
{
    (void)rxmeg;
    cs_can        *csrxcanp            = static_cast<cs_can *>(pro1030);
    static uint8_t break_location_last = 0;
    if (IS_ONE(csrxcanp->break_send_status, 1))
    {
        if (csrxcanp->break_location_temp != 0)
        {
            //            zprintf1("last = %d temp = %d\r\n", break_location_last, csrxcanp->break_location_temp);
            if (break_location_last == csrxcanp->break_location_temp)
            {
                csrxcanp->break_location = break_location_last;
                csrxcanp->state_info.set_break_location(0, csrxcanp->break_location - 1);
                csrxcanp->break_location_temp = 0;
                csrxcanp->break_location      = 0;
            }
            break_location_last           = csrxcanp->break_location_temp;
            csrxcanp->break_location_temp = 0;
        }
        else
        {
            break_location_last      = 0;
            csrxcanp->break_location = 0;
        }
    }
    if (csrxcanp->mac_cszd_have.val(0).mac_terminal_have == 1)
    {
        return 0;
    }
    if (IS_ONE(csrxcanp->break_send_status, 0))
    {
        if (csrxcanp->csstate == CS_CAN_NORMAL)
        {
            if (csrxcanp->reset_state == DEV_RESET_OVER && csrxcanp->reset_msg[0] == RESET_SUCCESS)
            {
                csrxcanp->nconfig_map.val(0).dev_send_meg(
                    CS_REST_END_MEG, csrxcanp->reset_msg, sizeof(csrxcanp->reset_msg));
                csrxcanp->reset_msg[0] = RESET_FAIL;
            }

            uint8_t tail_location;

            for (int i = 0; i < BRANCH_ALL; i++)
            {
                if (csrxcanp->cszjorder[i] != csrxcanp->cszjorder_temp[i])
                {
                    for (uint8_t j = csrxcanp->cszjorder_temp[i]; j < csrxcanp->cszjorder[i]; j++)
                    {
                        csrxcanp->set_dev_status(j + 1, DEV_OFF_LINE);
                    }
                }
                csrxcanp->state_info.set_dev_num(i, csrxcanp->cszjorder_temp[i]);

                csrxcanp->state_info.set_bs_num(i, csrxcanp->get_bs_mac_num(i));
                csrxcanp->state_info.set_slaveio_num(
                    i, csrxcanp->slave_io_num_temp[i] - csrxcanp->mac_cszd_have.get_order_data(i).mac_cs_have_tmp);

                tail_location = csrxcanp->state_info.get_tail_location(i);

                if (tail_location != csrxcanp->cszjorder_temp[i] || tail_location == 0)
                {
                    csrxcanp->state_info.set_termal_vol(i, 0);

                    csrxcanp->state_info.set_tail_location(i, csrxcanp->cszjorder_temp[i]);
                    csrxcanp->state_info.set_bs_location(i, csrxcanp->cszjorder_temp[i]);
                }
            }
        }
    }
    return 0;
}

/***********************************************************************************
 * receive_a_v_callback
 * 功能：电压电流主动上报
 ***********************************************************************************/
int receive_a_v_callback(void *pro1030, CANDATAFORM rxmeg)
{
    cs_can    *csrxcanp = static_cast<cs_can *>(pro1030);
    uint8_t    branch;
    int        sourceid;
    CANFRAMEID rxmidframe;
    rxmidframe.canframeid = rxmeg.ExtId;
    if (rxmidframe.canframework.zjaddr_3)
    {
        branch = rxmidframe.canframework.csaddr_2 - 1;
    }
    else
    {
        branch = 0;
    }

    if (csrxcanp->is_have_mac_dev(rxmidframe.canframework.zjaddr_3, rxmidframe.canframework.csaddr_2,
                                  rxmidframe.canframework.devaddr_8, sourceid))
    {
        uint8_t value_v;
        uint8_t value_c;
        value_v = rxmeg.Data[0];
        value_c = rxmeg.Data[2] | rxmeg.Data[3] << 8;
        csrxcanp->state_info.set_dev_cv_state(branch, sourceid, value_v, value_c);
    }
    else
    {
        zprintf1("error:can not find dev %d\r\n", rxmidframe.canframework.devaddr_8);
    }
    return 0;
}

/***********************************************************************************
 * 函数名：heart_frame_over
 * 功能：心跳应答帧处理函数调用的子函数
 ***********************************************************************************/
void heart_frame_over(CANPRODATA *rxprodata, cs_can *csrxcanp)
{
    if (rxprodata == NULL || csrxcanp == NULL)
        return;

    csrxcanp->heart_print_mark      = 0;
    csrxcanp->cut_location_shake[0] = 0;
    csrxcanp->cut_location_shake[1] = 0;
    csrxcanp->cut_location_shake[2] = 0;
    csrxcanp->csreqmark.csnumstate  = 0;
    // printf("|||||||||||||||||||||||||||heartout 03\r\n");
    csrxcanp->heartcout           = 0;
    csrxcanp->heart_check_last_id = 0;
    csrxcanp->pro_p->del_buf_frame(rxprodata);

    csrxcanp->state_info.set_tail_location(0, 0);
    csrxcanp->state_info.set_bs_location(0, 0);

    if (csrxcanp->reset_state == DEV_RESET_OVER && csrxcanp->reset_msg[0] == RESET_SUCCESS)
    {
        csrxcanp->nconfig_map.val(0).dev_send_meg(
            CS_REST_END_MEG, csrxcanp->reset_msg, sizeof(csrxcanp->reset_msg));
        csrxcanp->reset_msg[0] = RESET_FAIL;
    }
}

__inline__ void tk100_data_pro(cs_can *csrxcanp, CANDATAFORM rxmeg)
{
    csrxcanp->bs100info.bsnum      = rxmeg.Data[2];
    csrxcanp->bs100info.zdstate    = rxmeg.Data[4];
    csrxcanp->bs100info.jtstate    = rxmeg.Data[6];
    csrxcanp->bs100info.powerstate = rxmeg.Data[6] >> 7;
}

void com_heart_nextprocess(CANPRODATA *rxprodata, cs_can *csrxcanp, uint8_t devnum, int ttl, CANDATAFORM rxmeg)
{
    uint16_t       devtyle = 0;
    CS_BRANCH_TYPE branch;
    CANFRAMEID     rxmidframe;
    int            config_id = 0;
    // int            battery   = 0;
    rxmidframe.canframeid    = rxmeg.ExtId;
    // uint8_t get_dev_addr, get_zj_index, get_cs_index;
    // get_dev_addr = rxmidframe.canframework.devaddr_8;
    // get_zj_index = rxmidframe.canframework.zjaddr_3;
    // get_cs_index = rxmidframe.canframework.csaddr_2;

    if (rxprodata == NULL || csrxcanp == NULL)
    {
        return;
    }

    branch = csrxcanp->get_branch(rxmidframe);



    devtyle = csrxcanp->ndev_map.val(devnum).type;

    if (csrxcanp->framark[devnum][ttl] == 1)    // 本设备重复的ttl心跳
    {
        return;
    }
    csrxcanp->is_have_config_dev(devnum + 1, config_id);
    switch (devtyle)
    {
        case DEV_256_MODBUS_LOCK:
        case DEV_256_IO_PHONE:
        case TK236_IOModule_Salve:
            memcpy(&(csrxcanp->nconfig_map.val(config_id).iodata[ttl * 4]), rxmeg.Data, rxmeg.DLC);
            // if (csrxcanp->nconfig_map.val(config_id).para.innum > ttl * 4)
            // {
            //     for (int i = 0; i < csrxcanp->nconfig_map.val(config_id).para.innum; i++)
            //     {

            //     }
            //     csrxcanp->state_info.set_dev_misc1_state(branch, devnum, );
            // }
            for (int i = 0; i < rxmeg.DLC; i += 2)
            {
                if (csrxcanp->nconfig_map.val(config_id).para.innum > ttl * 4)
                {
                    if ((rxmeg.Data[i + 1] & 0x40) == 0x40)
                    {
                        csrxcanp->state_info.set_dev_misc1_state(branch, devnum, ttl * 4 + i / 2, true);
                    }
                    else
                    {
                        csrxcanp->state_info.set_dev_misc1_state(branch, devnum, ttl * 4 + i / 2, false);
                    }
                }
            }
            break;
        case TERMINAL:
            csrxcanp->state_info.set_termal_vol(branch, rxmeg.Data[2]);
            break;
        case CS_DEV:
        {
            csrxcanp->heart_cs_error_count_g = 0;
            tbyte_swap((uint16_t *)rxmeg.Data, rxmeg.DLC);

            uint16_t c1 = (rxmeg.Data[4] * 0x100) + rxmeg.Data[3];
            uint16_t c2 = (rxmeg.Data[7] * 0x100) + rxmeg.Data[6];

            csrxcanp->state_info.set_cs_av_state(branch, rxmeg.Data[2], c1, rxmeg.Data[5], c2);
            csrxcanp->state_info.set_bs_type(branch, rxmeg.Data[1]);
            csrxcanp->cs_jt_status = rxmeg.Data[1];
            if (rxmeg.Data[1] == 0)
            {
                csrxcanp->state_info.set_bs_location(branch, 0);
                csrxcanp->state_info.set_break_location(branch, 0);
            }
        }
        break;

        case DEV_256_PHONE:
        case DEV_256_LOCK:
        case DEV_256_RELAY:

            break;
        default:
            break;
    }

    csrxcanp->framark[devnum].set(ttl);
}

__inline__ bool count_heart_nextprocess(cs_can *csrxcanp, uint8_t devnum, uint ttl)
{
    return csrxcanp->framark[devnum].to_ulong( ) == ((0x0001U << (ttl)) - 1U);
}

bool is_heartframe_correct(cs_can *csrxcanp, uint8_t devnum, int ttl, CANDATAFORM rxmeg, uint16_t devtyle)
{
    bool ret = FALSE;
    Q_UNUSED(rxmeg);
    Q_UNUSED(devtyle);
    if (count_heart_nextprocess(csrxcanp, devnum, ttl))
    {
        ret = TRUE;
    }
    csrxcanp->framark[devnum].reset( );
    //    zprintf1("no next framark[%d]= %d\r\n", devnum, csrxcanp->framark[devnum]);
    return ret;
}

void heart_nonextprocess(CANPRODATA *rxprodata, cs_can *csrxcanp, uint8_t devnum, int ttl, CANDATAFORM rxmeg)
{
    uint16_t   devtyle = 0;
    CANFRAMEID rxmidframe;
    int        branch;
    uint8_t    get_dev_addr; // get_zj_index, get_cs_index;
    int        config_id = 0;
    int        bs_num = 0, io_num = 0;
    uint8_t    devtype;

    if (rxprodata == NULL || csrxcanp == NULL)
        return;

    rxmidframe.canframeid = rxmeg.ExtId;

    get_dev_addr = rxmidframe.canframework.devaddr_8;
    // get_zj_index = rxmidframe.canframework.zjaddr_3;
    // get_cs_index = rxmidframe.canframework.csaddr_2;

    if (rxmidframe.canframework.zjaddr_3)
    {
        branch = rxmidframe.canframework.csaddr_2 - 1;
    }
    else
    {
        branch = 0;
    }

    devtyle = csrxcanp->ndev_map.val(devnum).type;

    static uint8_t can_error_data[255][7];
    static bool    flag = 0;
    if (flag == 0)
    {
        flag = 1;
        memset(&can_error_data, 0, sizeof(can_error_data));
    }
    //    zprintf1("receive %d last frame\r\n", get_dev_addr);
    if (rxmeg.Data[4] != 0 || rxmeg.Data[6] != 0 || rxmeg.Data[7] != 0)
    {
        // can error
        can_error_data[get_dev_addr - 1][0] = get_dev_addr;
        can_error_data[get_dev_addr - 1][1] += (rxmeg.Data[4] >> 4) & 0x0F;
        can_error_data[get_dev_addr - 1][2] += rxmeg.Data[4] & 0x0F;

        can_error_data[get_dev_addr - 1][3] += (rxmeg.Data[7] >> 4) & 0x0F;
        can_error_data[get_dev_addr - 1][4] += rxmeg.Data[7] & 0x0F;

        can_error_data[get_dev_addr - 1][5] += (rxmeg.Data[6] >> 4) & 0x0F;
        can_error_data[get_dev_addr - 1][6] += rxmeg.Data[6] & 0x0F;

        csrxcanp->nconfig_map.val(0).dev_send_meg(
            REPROT_CAN_ERROR, can_error_data[get_dev_addr - 1], sizeof(can_error_data[0]));
    }

    uint8_t value_v;
    uint8_t value_c;
    value_v = rxmeg.Data[1];
    value_c = rxmeg.Data[3] | rxmeg.Data[2] << 8;
    csrxcanp->state_info.set_dev_cv_state(branch, devnum, value_v, value_c);

    bool lock_status;
    if (rxmeg.Data[5] & 0x01)
    {
        lock_status = 0;
    }
    else
    {
        lock_status = 1;
    }
    csrxcanp->state_info.set_bs_state(branch, get_dev_addr - 1, lock_status);
    //    zprintf1("receive dev %d type =%d\r\n", devnum, devtyle);
    if (is_heartframe_correct(csrxcanp, devnum, ttl, rxmeg, devtyle) == 0)
    {
        zprintf1("nonext error! %d\r\n", devnum);
        return;
    }

    if (get_dev_addr == csrxcanp->csmacorder)
    {
        if (csrxcanp->reset_state == DEV_RESET_OVER && csrxcanp->reset_msg[0] == RESET_SUCCESS)
        {
            csrxcanp->nconfig_map.val(0).dev_send_meg(CS_REST_END_MEG, csrxcanp->reset_msg, sizeof(csrxcanp->reset_msg));
            csrxcanp->reset_msg[0] = RESET_FAIL;
        }
    }
    switch (devtyle)
    {
        case DEV_256_MODBUS_LOCK:
        case DEV_256_IO_PHONE:
        case TK236_IOModule_Salve:
            csrxcanp->is_have_config_dev(devnum + 1, config_id);
            csrxcanp->nconfig_map.val(config_id).set_share_data( );
            break;
        case TERMINAL:
            for (uint8_t i = 0; i < get_dev_addr; i++)
            {
                devtype = csrxcanp->ndev_map.val(i).type;
                if ((devtype == DEV_256_PHONE) || (devtype == DEV_256_LOCK) || (devtype == DEV_256_IO_PHONE) || (devtype == DEV_256_MODBUS_LOCK) || (devtype == CS_DEV) || (devtype == DEV_256_RELAY))
                {
                    bs_num++;
                }
                if ((devtype == TK236_IOModule_Salve) || (devtype == DEV_256_MODBUS_LOCK) || (devtype == DEV_256_IO_PHONE))
                {
                    io_num++;
                }
                // csrxcanp->set_dev_map_state(i + 1, DEV_OFF_LINE);  //20231130
            }
            csrxcanp->set_devonline_num(get_dev_addr);

            csrxcanp->state_info.set_dev_num(0, get_dev_addr);
            csrxcanp->state_info.set_bs_num(0, bs_num);
            csrxcanp->state_info.set_slaveio_num(0, io_num);
            heart_frame_over(rxprodata, csrxcanp);
            break;
        case CS_DEV:
            csrxcanp->heart_cs_error_count_g = 0;
            break;

        case DEV_256_PHONE:
        case DEV_256_LOCK:
        case DEV_256_RELAY:

            break;
        default:
            break;
    }
}
/***********************************************************************************
 * 函数名：max_heart_ackframe_proc
 * 功能：心跳应答帧处理函数
 ***********************************************************************************/
int max_heart_ackframe_proc(void *pro1030, CANDATAFORM rxmeg)
{
    int        soureid = 0;
    CANFRAMEID rxmidframe;
    CANFRAMEID checkframe;

    CANPRODATA *rxprodata = NULL;
    cs_can     *csrxcanp  = static_cast<cs_can *>(pro1030);

    uint8_t get_dev_addr;//    get_zj_index, get_cs_index;

    if (csrxcanp == NULL)
    {
        return -1;
    }

    rxmidframe.canframeid = rxmeg.ExtId;
    get_dev_addr          = rxmidframe.canframework.devaddr_8;
    // get_zj_index          = rxmidframe.canframework.zjaddr_3;
    // get_cs_index          = rxmidframe.canframework.csaddr_2;

    // rxmidframe.canframeid = rxmeg.ExtId;
    if (csrxcanp->csstate != CS_CAN_NORMAL)
    {
        return -1;
    }
    //    zprintf1("receive source id is %d\r\n", get_dev_addr);
    if (get_dev_addr > csrxcanp->cszjorder[0])
    {
        zprintf1("heart reset\r\n");
        csrxcanp->cs_can_reset_sem( );
        return 0;
    }
    if (csrxcanp->is_have_mac_dev(rxmidframe.canframework.zjaddr_3, rxmidframe.canframework.csaddr_2,
                                  rxmidframe.canframework.devaddr_8, soureid)
        == FALSE)
    {
        return -2;
    }
    csrxcanp->heart_check_last_id = get_dev_addr;
    tbyte_swap((uint16_t *)rxmeg.Data, rxmeg.DLC);
    checkframe = rxmidframe;

    checkframe.canframework.dir_1     = 1;
    checkframe.canframework.ack_1     = 0;
    checkframe.canframework.next_1    = 0;
    checkframe.canframework.ttl_7     = 0;
    checkframe.canframework.devaddr_8 = 0xFF;
    rxprodata                         = csrxcanp->pro_p->read_deldata_buf(checkframe.canframeid, 1);

    if (rxprodata != NULL)
    {
        if (rxmidframe.canframework.next_1 == 1)
        {
            // printf("receive have next\r\n");
            com_heart_nextprocess(rxprodata, csrxcanp, soureid, rxmidframe.canframework.ttl_7, rxmeg);
        }

        else
        {
            // printf("receive not next\r\n");
            csrxcanp->set_dev_map_state(get_dev_addr, DEV_ON_LINE);
            heart_nonextprocess(rxprodata, csrxcanp, soureid, rxmidframe.canframework.ttl_7, rxmeg);
        }
    }
    else
    {
        // printf("exit %d\r\n", get_dev_addr);
    }
    return 0;
}

/***********************************************************************************
 * 函数名：max_heartframe_overtimeproc
 * 功能：心跳帧超时处理函数
 ***********************************************************************************/
int max_heartframe_overtimeproc(void *pro1030, CANDATAFORM overmeg)
{
    uint8_t reset_reason[4] = { 0, 0, 0, 0 };
    cs_can *csrxcanp        = static_cast<cs_can *>(pro1030);
    // Q_UNUSED(overmeg);
    (void)overmeg;
    int soure_id = 0;
    int devtype  = 0;
    int bs_num = 0, io_num = 0;
    int lost_io_dev = false;
    if (csrxcanp == NULL)
    {
        return -1;
    }
    if (csrxcanp->heart_check_last_id == csrxcanp->cszjorder[0])    // 终端收到一帧，丢一帧，退出，不然可能会报终端断线。
    {
        return -1;
    }
    if (csrxcanp->heart_print_mark == 0)
    {
        zprintf1("heart check overtime %d %d\r\n", csrxcanp->heart_check_last_id, csrxcanp->reset_state);
        zprintf1(
            "ndev_map size is %d,csrxcanp->cszjorder[0] = %d\r\n", csrxcanp->ndev_map.size( ), csrxcanp->cszjorder[0]);
        csrxcanp->heart_print_mark = 1;
    }

    if (csrxcanp->reset_state == DEV_RESET_OVER && csrxcanp->reset_msg[0] == RESET_SUCCESS)
    {
        csrxcanp->nconfig_map.val(0).dev_send_meg(CS_REST_END_MEG, csrxcanp->reset_msg, sizeof(csrxcanp->reset_msg));
        csrxcanp->reset_msg[0] = RESET_FAIL;
    }
    if (csrxcanp->csstate != CS_CAN_NORMAL)
    {
        return -1;
    }
    if (csrxcanp->cut_location_shake[0] != csrxcanp->heart_check_last_id)
    {
        zprintf3("heart check break location is %d\r\n", csrxcanp->heart_check_last_id);
    }
    if (csrxcanp->heart_check_last_id == 0)
    {
        if (++csrxcanp->heart_cs_error_count_g > 4)
        {
            reset_reason[0] = PTCAN_RESET_HEART_OVERTIME;
            reset_reason[1] = csrxcanp->auto_reset;
            reset_reason[2] = csrxcanp->heart_check_last_id;
            reset_reason[3] = devtype;
            csrxcanp->state_info.set_line_work_state(0, CS_WORK_STATUS_LIVEOUT);
            csrxcanp->nconfig_map.val(0).dev_send_meg(RESET_REASON, reset_reason, sizeof(reset_reason));
            csrxcanp->cs_can_reset_sem( );
        }
        else if (csrxcanp->heart_cs_error_count_g > 3)
        {
            csrxcanp->state_info.set_line_work_state(0, CS_WORK_STATUS_LIVEOUT);
        }

        return -1;
    }

    if (csrxcanp->cut_location_shake[2] == csrxcanp->heart_check_last_id && csrxcanp->cut_location_shake[1] == csrxcanp->heart_check_last_id && csrxcanp->cut_location_shake[0] == csrxcanp->heart_check_last_id)
    {
        csrxcanp->state_info.set_termal_vol(0, 0);
        csrxcanp->state_info.set_tail_location(0, csrxcanp->heart_check_last_id);
        csrxcanp->state_info.set_bs_location(0, csrxcanp->heart_check_last_id);

        for (uint8_t i = 0; i < csrxcanp->heart_check_last_id; i++)
        {
            // zprintf1("heart i:%d,id:%d,type:%d\r\n", i, csrxcanp->ndev_map.val(i).id,
            // csrxcanp->ndev_map.val(i).type);
            devtype = csrxcanp->ndev_map.val(i).type;
            if ((devtype == DEV_256_PHONE) || (devtype == DEV_256_LOCK) || (devtype == DEV_256_IO_PHONE) || (devtype == DEV_256_MODBUS_LOCK) || (devtype == CS_DEV) || (devtype == DEV_256_RELAY))
            {
                bs_num++;
            }
            if ((devtype == TK236_IOModule_Salve) || (devtype == DEV_256_MODBUS_LOCK) || (devtype == DEV_256_IO_PHONE))
            {
                io_num++;
            }
        }

        csrxcanp->state_info.set_dev_num(0, csrxcanp->heart_check_last_id);
        csrxcanp->state_info.set_bs_num(0, bs_num);
        csrxcanp->state_info.set_slaveio_num(0, io_num);

        if (csrxcanp->is_have_mac_dev(0, 0, csrxcanp->heart_check_last_id, soure_id))
        {
            for (uint8_t j = csrxcanp->heart_check_last_id; j < csrxcanp->cszjorder[0] - 1; j++)
            {
                csrxcanp->set_dev_map_state(j + 1, DEV_OFF_LINE);
                devtype = csrxcanp->ndev_map.val(j).type;
                if ((devtype == TK236_IOModule_Salve) || (devtype == DEV_256_MODBUS_LOCK) || (devtype == DEV_256_IO_PHONE))
                {
                    lost_io_dev = true;
                }
            }
            if (lost_io_dev == true)
            {
                reset_reason[0] = PTCAN_RESET_HEART_OVERTIME;
                reset_reason[1] = csrxcanp->auto_reset;
                reset_reason[2] = 0;
                reset_reason[3] = devtype;
                if (csrxcanp->auto_reset)
                {
                    csrxcanp->nconfig_map.val(0).dev_send_meg(
                        RESET_REASON, reset_reason, sizeof(reset_reason));
                }
                csrxcanp->cs_can_reset_pro( );
            }
        }
    }
    csrxcanp->cut_location_shake[2] = csrxcanp->cut_location_shake[1];
    csrxcanp->cut_location_shake[1] = csrxcanp->cut_location_shake[0];
    csrxcanp->cut_location_shake[0] = csrxcanp->heart_check_last_id;
    csrxcanp->heart_check_last_id   = 0;
    return 0;
}
/***********************************************************************************
 * 函数名：err_reportframe_proc
 * 功能：错误上报帧处理
 ***********************************************************************************/
int err_reportframe_proc(void *pro1030, CANDATAFORM rxmeg)
{
    uint8_t    error_status[2];
    CANFRAMEID rxmidframe;
    cs_can    *csrxcanp = static_cast<cs_can *>(pro1030);
    uint8_t    get_dev_addr;// get_zj_index, get_cs_index;

    rxmidframe.canframeid = rxmeg.ExtId;
    tbyte_swap((uint16_t *)rxmeg.Data, rxmeg.DLC);

    get_dev_addr = rxmidframe.canframework.devaddr_8;
    // get_zj_index = rxmidframe.canframework.zjaddr_3;
    // get_cs_index = rxmidframe.canframework.csaddr_2;

    if (csrxcanp == NULL)
        return -1;
    csrxcanp->cs_send_data(CMD_SEND_ERRREPORT_ACK, rxmidframe.canframework.zjaddr_3, rxmidframe.canframework.csaddr_2,
                           rxmidframe.canframework.devaddr_8, rxmeg.Data, rxmeg.DLC);

    if (rxmeg.Data[0] == 0x03)
    {
        error_status[0] = get_dev_addr;
        error_status[1] = rxmeg.Data[0];
        while (1)
        {
            csrxcanp->nconfig_map.val(0).dev_send_meg(CONFIG_ERROR, error_status, sizeof(error_status));
            sleep(4);
        }
    }
    return 0;
}
/***********************************************************************************
 * 函数名：slavereset_frame_proc
 * 功能：从机复位帧处理
 ***********************************************************************************/
int slavereset_frame_proc(void *pro1030, CANDATAFORM rxmeg)
{
    uint8_t        reset_reason[5] = { 0, 0, 0, 0, 0 };
    uint8_t        get_dev_addr; //get_zj_index, get_cs_index;
    CANFRAMEID     rxmidframe;
    static uint8_t report_addr = 0;
    cs_can        *csrxcanp    = static_cast<cs_can *>(pro1030);
    if (csrxcanp == NULL)
    {
        return 0;
    }
    rxmidframe.canframeid = rxmeg.ExtId;

    get_dev_addr = rxmidframe.canframework.devaddr_8;
    // get_zj_index = rxmidframe.canframework.zjaddr_3;
    // get_cs_index = rxmidframe.canframework.csaddr_2;

    tbyte_swap((uint16_t *)rxmeg.Data, rxmeg.DLC);
    rxmeg.Data[5]    = 0;
    uint16_t devtype = 0;
    memcpy(&devtype, &rxmeg.Data[4], 2);
#ifdef END_NO_RESET
    if (devtype != TERMINAL)
    {
#endif
        if (csrxcanp->reset_state == DEV_RESET_OVER)
        {
            reset_reason[0] = PTCAN_RESET_SALVE;
            reset_reason[1] = csrxcanp->auto_reset;
            reset_reason[2] = get_dev_addr;
            reset_reason[3] = devtype;
            report_addr     = reset_reason[2];
            if (((rxmeg.Data[3] >> 4) & 0x0F) == 0x0F)
            {
                reset_reason[4] = 8;
            }
            else
            {
                reset_reason[4] = (rxmeg.Data[3] >> 4) & 0x0F;
            }
            zprintf3("reset reason is %x\r\n", reset_reason[4]);
            csrxcanp->nconfig_map.val(0).dev_send_meg(RESET_REASON, reset_reason, sizeof(reset_reason));
            if (reset_reason[4] == 7)
            {
                return 0;
            }
        }
        else if (csrxcanp->reset_state == DEV_RESET_ING)
        {
            if (get_dev_addr < report_addr)
            {
                reset_reason[0] = PTCAN_RESET_SALVE;
                reset_reason[1] = csrxcanp->auto_reset;
                reset_reason[2] = get_dev_addr;
                reset_reason[3] = devtype;
                report_addr     = reset_reason[2];
                if (((rxmeg.Data[3] >> 4) & 0x0F) == 0x0F)
                {
                    reset_reason[4] = 8;
                }
                else
                {
                    reset_reason[4] = (rxmeg.Data[3] >> 4) & 0x0F;
                }
                csrxcanp->nconfig_map.val(0).dev_send_meg(RESET_REASON, reset_reason, sizeof(reset_reason));
            }
            zprintf3("reset reason is %d %d %d\r\n", reset_reason[2], get_dev_addr, report_addr);
        }

        csrxcanp->cs_can_reset_sem( );
#ifdef END_NO_RESET
    }
    else
    {
        if (csrxcanp->reset_state == DEV_RESET_OVER)
        {
            zprintf1("send mac check frame1\r\n");
            csrxcanp->cs_send_data(CMD_SEND_MACCHECK, 0, 0, 255, NULL, 0);
        }
    }
#endif
    return 0;
}

/***********************************************************************************
 * 函数名：cs_can
 * 功能：cs初始化函数
 ***********************************************************************************/

cs_can::cs_can(ncan_protocol *pro, const QString & key, int branch_num, int reset_enable)
{
    // memset(this, 0x00, offsetof(cs_can, nconfig_map));
    //    memset(this, 0x00, offsetof(cs_can, mac_cszd_have));
    auto_reset = reset_enable;
    sem_init(&statechg, 0, 0);
    sem_init(&reset_sem, 0, 0);
    pro_p = pro;
    state_info.max_state_pro_init(key, branch_num);
    cs_jt_status = CS_JT_OK;
}

int cs_can::cs_can_reset_sem(void)
{
    if (init_over == 1)
    {
        if (reset_state != DEV_RESET_ING)
        {
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
    if (auto_reset)
    {
        zprintf1("1030 auto reset!\n");
        return cs_can_reset_sem( );
    }
    return 0;
}

uint8_t cs_can::get_config_low_num(void)
{
    return cf_file_num ? cf_file_num - cf_cs_num[0] - cf_cs_num[1] - cf_cs_num[2] : cf_file_num;
}
uint8_t cs_can::get_mac_low_num(void)
{
    if (csmacorder < (mac_cs_num + mac_terminal_num))
    {
        if (mac_cs_num)
        {
            return 0;
        }
        else
        {
            return 0xff;
        }
    }
    if (csmacorder == 0)
    {
        return 0xff;
    }
    return slave_io_num[0] + slave_io_num[1] + slave_io_num[2] - mac_terminal_num - mac_cs_num;
}

uint8_t cs_can::get_bs_mac_num(uint8_t branch)
{
    uint8_t bsnum = 0;
    for (uint8_t i = (branch ? cszjorder[branch - 1] : branch); i < cszjorder[branch]; i++)
    {
        if ((ndev_map.val(i).type == DEV_256_PHONE) || (ndev_map.val(i).type == DEV_256_LOCK) || (ndev_map.val(i).type == DEV_256_IO_PHONE) || (ndev_map.val(i).type == DEV_256_MODBUS_LOCK) || (ndev_map.val(i).type == CS_DEV) || (ndev_map.val(i).type == DEV_256_RELAY))
        {
            bsnum++;
        }
    }
    return bsnum;
}

void cs_can::add_mac_cszd_branch(int zjnum, int csnum)
{
    uint8_t      branch_line;
    CSZD_IS_HAVE tmp;

    branch_line           = csnum + (zjnum << 2);
    tmp.mac_cs_have       = false;
    tmp.mac_terminal_have = false;

    if (mac_cszd_have.find(branch_line) == mac_cszd_have.end( ))
    {
        mac_cszd_have.insert(pair< uint8_t, CSZD_IS_HAVE >(branch_line, tmp));
    }
}

void cs_can::set_mac_cs_have(int zjnum, int csnum)
{
    uint8_t branch_line;

    branch_line = csnum + (zjnum << 2);

    if (mac_cszd_have.find(branch_line) != mac_cszd_have.end( ))
    {
        mac_cszd_have.val(branch_line).mac_cs_have = 1;
    }
}

void cs_can::bs_type_location_report(uint8_t bs_type)
{
    uint8_t data[2];

    if (!(bs_type_r == bs_type))
    {
        data[0] = bs_type_r = bs_type;
        nconfig_map.val(0).dev_send_meg(BS_REPORT_MEG, data, 1);
        if (bs_type_r == 0)
        {
            bs_location_r = 0;
        }
    }
}

void cs_can::change_report_bs_location(uint8_t branch, uint8_t bs_location)
{
    if (branch == 0)
    {
        bs_location_r = bs_location;
    }
}

void cs_can::bs_type_location_report(uint8_t branch, uint8_t bs_type, uint8_t bs_location)
{
    uint8_t data[2];

    if (branch == 0)
    {
        if (!(bs_type_r == bs_type && bs_location_r == bs_location))
        {
            zprintf3("bs report: branch = %d, bs_type = %d, bs_location = %d\n", branch, bs_type, bs_location);
            zprintf3("bs report: branch = %d, bs_type_r = %d, bs_location_r = %d\n", branch, bs_type_r, bs_location_r);
            data[0] = bs_type_r = bs_type;
            data[1] = bs_location_r = bs_location;
            nconfig_map.val(0).dev_send_meg(BS_REPORT_MEG, data, 2);
        }
    }
}

void cs_can::bs_type_location_report(uint8_t bs_type, uint8_t bs_location)
{
    uint8_t data[2];

    if (!((bs_type_r == bs_type) && (bs_location_r == bs_location)))
    {
        zprintf2("bs report: bs_type = %d, bs_location = %d\n", bs_type, bs_location);
        data[0] = bs_type_r = bs_type;
        data[1] = bs_location_r = bs_location;
        nconfig_map.val(0).dev_send_meg(BS_REPORT_MEG, data, 2);
    }
}

/***********************************************************************************
 * 函数名：send_configdata
 * 功能：cs发送配置参数
 ***********************************************************************************/
void cs_can::max_reset_data(void)
{
    csstate    = CS_CAN_INIT;    // 1030协议初始化时的状态变迁
    devonnum   = 0;
    // cf_devnum  = cf_devnum;     // 配置的设备个数
    cf_endnum  = 0;             // 重配的结束设备编号
    csmacstate = MAC_NORMAL;    // mac查询状态
    csmacorder = 0;             // mac查询应答顺序编号
    csioorder  = 0;
    memset(slave_io_num, 0, 3);
    memset(cszjorder, 0, 3);
    zprintf1(">?>?>?>?>?clear frist?>?>?>>?>?>?>?%d\r\n", ndev_map.size( ));
    ndev_map.clear( );
    zprintf1(">?>?>?>?>?clear end>?>?>?>>?>?>?>?%d\r\n", ndev_map.size( ));
    mac_cs_num            = 0;
    mac_terminal_num      = 0;                // mac查询终端存在
    heartcout             = 0;                // 心跳发送顺序计数
    csconfstate           = CSCONF_NORMAL;    // 配置状态
    initproid             = 0;                // 1030协议初始化线程id号
    polltimer_id          = 0;
    break_location        = 0;
    break_location_temp   = 0;
    cut_check_flag        = 0;
    cs_jt_status          = CS_JT_OK;
    heart_check_last_id   = 0;
    cut_location_shake[0] = 0;
    cut_location_shake[1] = 0;
    cut_location_shake[2] = 0;
    //    mac_cszd_have.val(0).mac_cs_have = 0;
    //    mac_cszd_have.val(6).mac_cs_have = 0;
    //    mac_cszd_have.val(7).mac_cs_have = 0;
    memset(&csreqmark, 0x00, sizeof(REQDEVMK));

    state_info.set_all_state_clear( );
    for (uint i = 0; i < nconfig_map.size( ); i++)
    {
        nconfig_map.val(i).reset_dev_data( );
    }
    mac_cszd_have.get_order_datap(0)->clear( );
    mac_cszd_have.get_order_datap(1)->clear( );
    mac_cszd_have.get_order_datap(2)->clear( );
    for (uint8_t i = ndev_map.size( ) - 1; ndev_map.size( ) > 0; i--)
    {
        if (ndev_map.get_order_datap(i) != NULL)
        {
            delete ndev_map.get_order_datap(i);
        }
    }
    state_info.set_all_dev_state(DEV_OFF_LINE);

    // data_p->reset_data_value();        //IO no reset
    zprintf1("data_p->reset_data_value()  OK!\n");
}

void cs_can::set_dev_enable( )
{
    int order;
    for (uint8_t k = 0; k < cszjorder[0]; k++)
    {
        state_info.set_dev_enable_state(k + 1, 1);
    }
    for (uint8_t k = 0; k < slave_io_num[0]; k++)
    {
        if (is_have_config_slaveio(0, 0, k + 1, order))
        {
            state_info.set_dev_enable_state(nconfig_map.val(order).para.id, nconfig_map.val(order).devenable);
        }
    }
    for (uint8_t k = 0; k < cszjorder[1]; k++)
    {
        state_info.set_dev_enable_state(k + 1, 1);
    }
    for (uint8_t k = 0; k < slave_io_num[1]; k++)
    {
        if (is_have_config_slaveio(1, 2, k + 1, order))
        {
            state_info.set_dev_enable_state(nconfig_map.val(order).para.id, nconfig_map.val(order).devenable);
        }
    }
    for (uint8_t k = 0; k < cszjorder[2]; k++)
    {
        state_info.set_dev_enable_state(k + 1, 1);
    }
    for (uint8_t k = 0; k < slave_io_num[2]; k++)
    {
        if (is_have_config_slaveio(1, 3, k + 1, order))
        {
            state_info.set_dev_enable_state(nconfig_map.val(order).para.id, nconfig_map.val(order).devenable);
            zprintf3("state_info.set_dev_enable_state(%d, %d)\n", nconfig_map.val(order).para.id,
                     nconfig_map.val(order).devenable);
        }
    }
}

/***********************************************************************************
 * 函数名：send_configdata
 * 功能：cs发送配置参数
 ***********************************************************************************/
void cs_can::send_configdata(void)
{
    uint i;
    uint configsize = csioorder;

    zprintf3("|||cs_init|||configsize %d!\n", configsize);
    for (i = 0; i < configsize; i++)
    {
        if (nconfig_map.val(i).conf_state == CS_CONFIGING)
        {
            zprintf3("|||cs_init|||can send 05: send config %d!\n", i);
            cs_send_data(CMD_SEND_CFGPARAM, 0, 0, nconfig_map.val(i).para.id, (uint8_t *)nconfig_map.val(i).config_p,
                         nconfig_map.val(i).configsize * 2);
            linuxDly(200);
        }
    }
    zprintf3("|||cs_init|||send config over!\n");
}

void *heartManageThread(void *para)
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
void *cs_can::cs_protocol_init(void *para)
{
    uint8_t reset_reason[4] = { 0, 0, 0, 0 };
    if ((static_cast<cs_can *>(para))->cs_init( ) != 0)
    {
        zprintf1("init fail send sem!\n");
        reset_reason[0] = PTCAN_RESET_INIT;
        reset_reason[1] = (static_cast<cs_can *>(para))->auto_reset;
        (static_cast<cs_can *>(para))->nconfig_map.val(0).dev_send_meg(RESET_REASON, reset_reason, sizeof(reset_reason));
        (static_cast<cs_can *>(para))->cs_can_reset_sem( );
    }
    // printf("cs_protocol_init end\r\n");
    (static_cast<cs_can *>(para))->reset_state = DEV_RESET_OVER;
    return NULL;
}

/***********************************************************************************
 * 函数名：cs_protocol_init
 * 功能：cs协议初始化函数 供协议层回调的cs协议初始化函数
 ***********************************************************************************/
void *max_reset_process(void *para)
{
    // int     reset_no_err;
    cs_can *cs_pro_p     = static_cast<cs_can *>(para);
    while (1)
    {
        int reset_no_err = true;
        zprintf1("||||||||||||||||||||||||||||||wait reset_Sem|||||||||||||||||||||||||||\r\n");
        sem_wait(&cs_pro_p->reset_sem);


        cs_pro_p->delete_1030_dev_timer( );

        while (reset_no_err != 0)
        {
            cs_pro_p->nconfig_map.val(0).dev_send_meg(CS_REST_MEG, NULL, 0);
            sleep(2);
            for (int branch = 0; branch < BRANCH_ALL; branch++)
            {
                cs_pro_p->state_info.set_termal_vol(branch, 0);
                cs_pro_p->state_info.set_line_work_state(branch, CS_WORK_STATUS_LIVEOUT);
            }
            cs_pro_p->max_reset_data( );
            reset_no_err = cs_pro_p->cs_init( );
            zprintf3("|||cs_init|||reset_no_err = %d\r\n", reset_no_err);
            if (reset_no_err != 0)
            {
                cs_pro_p->reset_msg[0] = RESET_FAIL;
                cs_pro_p->reset_msg[1] = cs_pro_p->get_config_low_num( );
                cs_pro_p->reset_msg[2] = cs_pro_p->get_mac_low_num( );
                cs_pro_p->nconfig_map.val(0).dev_send_meg(
                    CS_REST_END_MEG, cs_pro_p->reset_msg, sizeof(cs_pro_p->reset_msg));
                sleep(1);
            }
        }
        cs_pro_p->reset_state = DEV_RESET_OVER;
    }
    return NULL;
}

int mac_send_entry(void *para)
{
    cs_can *cs_pro_p                           = (static_cast<cs_can *>(para));
    cs_pro_p->mac_cszd_have.val(0).mac_cs_have = 0;
    cs_pro_p->mac_cszd_have.val(6).mac_cs_have = 0;
    cs_pro_p->mac_cszd_have.val(7).mac_cs_have = 0;
    return 1;
}

int dev_break_send_entry(void *para)
{
    cs_can *cs_pro_p = (static_cast<cs_can *>(para));

    memset(cs_pro_p->cszjorder_temp, 0, sizeof(cs_pro_p->cszjorder_temp));
    memset(cs_pro_p->slave_io_num_temp, 0, sizeof(cs_pro_p->slave_io_num_temp));
    cs_pro_p->mac_cs_num_temp = 0;
    cs_pro_p->csioorder_temp  = 0;
    cs_pro_p->csmacorder_temp = 0;

    cs_pro_p->mac_cszd_have.get_order_datap(0)->clear( );
    cs_pro_p->mac_cszd_have.get_order_datap(1)->clear( );
    cs_pro_p->mac_cszd_have.get_order_datap(2)->clear( );

    cs_pro_p->mac_cszd_have.val(0).mac_cs_have_tmp = 0;
    cs_pro_p->mac_cszd_have.val(6).mac_cs_have_tmp = 0;
    cs_pro_p->mac_cszd_have.val(7).mac_cs_have_tmp = 0;
    return 1;
}

/***********************************************************************************
 * 函数名：poll_send_condition
 * 功能：帧发送条件
 *
 ***********************************************************************************/
int max_poll_send_condition(void *para)
{
    cs_can *cs_pro_p = (static_cast<cs_can *>(para));
    if (cs_pro_p->csstate == CS_CAN_NORMAL)
    {
        return 1;
    }
    return 0;
}

void cs_can::set_dev_status(uint8_t devid, uint8_t state)
{
    int      max_id = 3 * FATHER_DEV_MAX;
    // uint16_t dev_off;
    if (devid < 1 || devid > max_id)
    {
        zprintf1("set dev state id%d err!\n", devid);
        return;
    }
    // if (nconfig_map.val(devid - 1).para.id != 0)
    // {
    //     dev_off = nconfig_map.val(devid - 1).dev_off;
    // }
    state_info.set_dev_status(devid, state);
}

void cs_can::set_dev_map_state(uint8_t devid, uint8_t state)
{
    if (state)
    {
        // printf("set dev <%d> on line\r\n", devid);
        state_info.set_dev_state_OR(devid, state);
    }
    else
    {
        // printf("set dev <%d> off line\r\n", devid);
        state_info.set_dev_state_AND(devid, state);
    }
}

void cs_can::set_dev_state(uint8_t devid, uint8_t state)    // Start from 1
{
    uint16_t dev_off;

    dev_off = nconfig_map.val(devid - 1).dev_off;
    if (state)
    {
        printf("set dev <%d> on line\r\n", devid);
        state_info.set_dev_state_OR(dev_off, state);
    }
    else
    {
        printf("set dev <%d> off line\r\n", devid);
        state_info.set_dev_state_AND(dev_off, state);
    }
    if (state == DEV_ON_LINE)
    {
        if (devid > devonnum)
            devonnum = devid;
    }
    else
    {
        if (devid < devonnum)
            devonnum = devid;
    }
}
void *func(void *arg)
{
    Q_UNUSED(arg);
    cs_can *cs_param           = static_cast<cs_can *>(arg);
    uint8_t time_delay         = 0;
    uint8_t button_press_shake = 0;
    while (1)
    {
        if (!cs_param->state_info.get_bs_is_have(0))
        {
            if (button_press_shake < 2)
            {
                button_press_shake++;
            }
        }
        else
        {
            if (cs_param->cut_check_flag == 0)
            {
                time_delay = 0;
            }
            button_press_shake = 0;
            cs_param->state_info.set_break_location(0, 0);
            cs_param->break_location = 0;
        }
        if (cs_param->cut_check_flag == 1)
        {
            cs_param->break_send_status = SET_NTH_BIT(cs_param->break_send_status, 0);
        }
        else
        {
            cs_param->break_send_status = CLEAR_NTH_BIT(cs_param->break_send_status, 0);
        }

        if (cs_param->cs_jt_status != CS_JT_OK && button_press_shake == 2)
        {
            cs_param->break_send_status = SET_NTH_BIT(cs_param->break_send_status, 1);
        }
        else
        {
            cs_param->break_send_status = CLEAR_NTH_BIT(cs_param->break_send_status, 1);
        }
        if ((cs_param->cs_jt_status != CS_JT_OK && button_press_shake == 2) || cs_param->cut_check_flag == 1)
        {
            if (time_delay == 0 || time_delay == 6)
            {
                time_delay = 0;
                if (cs_param->reset_state == DEV_RESET_OVER)
                {
                    cs_param->cs_send_data(CMD_SEND_BREAK_CHECK, 0, 0, 255, NULL, 0);
                }
            }
            time_delay++;
        }
        else
        {
            time_delay = 0;
            cs_param->state_info.set_break_location(0, 0);
            cs_param->break_location      = 0;
            cs_param->break_location_temp = 0;
        }
        usleep(200000);
    }
    return NULL;
}

/***********************************************************************************
 * 函数名：cs_config_init
 * 功能：cs配置初始化函数
 ***********************************************************************************/
int cs_can::cs_config_init(void)
{
    CANFRAMEID overmidframe;
    memset(&pollFrame, 0x00, sizeof(pollFrame));
    overmidframe.canframework.ack_1     = 0;
    overmidframe.canframework.devaddr_8 = 0xFF;
    overmidframe.canframework.csaddr_2  = 0;
    overmidframe.canframework.zjaddr_3  = 0;
    overmidframe.canframework.func_5    = CMD_SEND_HEARTCHECK;
    overmidframe.canframework.ttl_7     = 0;
    overmidframe.canframework.next_1    = 0;
    overmidframe.canframework.dir_1     = 1;
    pollFrame.ExtId                     = overmidframe.canframeid;
    pollFrame.IDE                       = 1;
    CANPROHEAD csmidinfo[]              = { /*****************上电复位帧 功能码2******************************************/
                               {

                                   1,
                                   2,       // 映射编号
                                   1000,    // 超时时间
                                   1,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   NULL,                           // 接收回调函数
                                   power_resetovertime_process,    // 超时回调函数
                                   NULL,                           // 发送条件回调函数
                               },
                               /*****************mac 地址查询帧 功能码3******************************************/
                               {
                                   1,
                                   3,       // 映射编号
                                   1200,    // 超时时间
                                   1,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   mac_inquire_ack_proc,       // 接受回调函数
                                   macreqdata_overtimeproc,    // 超时回调函数
                                   mac_send_entry,             // 发送条件回调函数
                               },

                               /*****************从站配置参数正确确认帧 功能码4******************************************/
                               {
                                   1,
                                   4,       // 映射编号
                                   0,       // 超时时间
                                   0,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   enter_workmode_frameproc,    // 接受回调函数
                                   NULL,                        // 超时回调函数
                                   NULL,                        // 发送条件回调函数
                               },
                               /*****************整体配置帧及应答 功能码5******************************************/
                               {
                                   1,
                                   5,       // 映射编号
                                   0,       // 超时时间
                                   0,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   configdata_ack_proc,        // 接受回调函数
                                   configdata_overtimeproc,    // 超时回调函数
                                   NULL,                       // 发送条件回调函数
                               },
                               /*****************从机上电复位通知帧 功能码6******************************************/
                               {
                                   1,
                                   6,       // 映射编号
                                   0,       // 超时时间
                                   0,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   slavereset_frame_proc,    // 接受回调函数
                                   NULL,                     // 超时回调函数
                                   NULL,                     // 发送条件回调函数
                               },
                               /*****************输出控制模拟量指令及应答 功能码10******************************************/
                               {
                                   1,
                                   10,      // 帧id
                                   0,       // 超时时间
                                   3,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   foutput_controlack_frameproc,    // 接受回调函数
                                   output_controlack_overproc,      // 超时回调函数
                                   NULL,                            // 发送条件回调函数
                               },

                               /*****************输出控制开关量指令及应答 功能码11******************************************/
                               {
                                   1,
                                   11,      // 帧id
                                   0,       // 超时时间
                                   3,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   output_controlack_frameproc,    // 接受回调函数
                                   output_controlack_overproc,     // 超时回调函数
                                   NULL,                           // 发送条件回调函数
                               },

                               /*****************闭锁急停上报帧及应答 功能码12******************************************/
                               {
                                   1,
                                   12,      // 帧id
                                   0,       // 超时时间
                                   0,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   //            stop_report_frameproc,                  //接受回调函数
                                   max_stop_report_frameproc,    // 接受回调函数
                                   NULL,                         // 超时回调函数
                                   NULL,                         // 发送条件回调函数
                               },

                               /*****************错误上报帧及应答 功能码13******************************************/
                               {
                                   1,
                                   13,      // 帧id
                                   0,       // 超时时间
                                   0,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   err_reportframe_proc,    // 接受回调函数
                                   NULL,                    // 超时回调函数
                                   NULL,                    // 发送条件回调函数
                               },

                               /*****************主动上报帧及应答 功能码14******************************************/
                               {
                                   1,
                                   14,      // 帧id
                                   0,       // 超时时间
                                   0,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   auto_report_frameproc,    // 接受回调函数
                                   NULL,                     // 超时回调函数
                                   NULL,                     // 发送条件回调函数
                               },
                               /*****************断路位置帧及应答 功能码19******************************************/
                               {
                                   1,
                                   19,      // 帧id
                                   0,       // 超时时间
                                   0,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   break_location_report_frameproc,    // 接受回调函数
                                   NULL,                               // 超时回调函数
                                   NULL,                               // 发送条件回调函数
                               },
                               /*****************按键上报帧及应答 功能码16******************************************/
                               {
                                   1,
                                   16,      // 帧id
                                   0,       // 超时时间
                                   0,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   NULL,    // 接受回调函数
                                   NULL,    // 超时回调函数
                                   NULL,    // 发送条件回调函数
                               },
                               /*****************寄存器读取功能 功能码20******************************************/
                               {
                                   1,
                                   20,      // 帧id
                                   0,       // 超时时间
                                   0,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   readreg_callback_frameproc,    // 接受回调函数
                                   NULL,                          // 超时回调函数
                                   NULL,                          // 发送条件回调函数
                               },
                               /*****************心跳查询帧及应答 功能码30******************************************/
                               {
                                   1,
                                   30,      // 帧id
                                   1500,    // 超时时间
                                   1,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   //            heart_ackframe_proc,                    //接受回调函数r
                                   //            heartframe_overtimeproc,                //超时回调函数
                                   max_heart_ackframe_proc,        // 接受回调函数
                                   max_heartframe_overtimeproc,    // 超时回调函数
                                   max_poll_send_condition,        // 发送条件回调函数
                               },
                               /*****************工作中设备查询阵 功能码24******************************************/
                               {
                                   1,
                                   24,      // 映射编号
                                   1000,    // 超时时间
                                   1,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   dev_break_check_callback,    // 接受回调函数
                                   dev_break_check_overtime,    // 超时回调函数
                                   dev_break_send_entry,        // 发送条件回调函数
                               },
                               /*****************工作中设备查询阵 功能码31******************************************/
                               {
                                   1,
                                   31,      // 映射编号
                                   0,       // 超时时间
                                   0,       // 重发次数
                                   0x00,    // 应答帧
                                   this,
                                   NULL,    // 接受回调函数
                                   NULL,    // 超时回调函数
                                   NULL,    // 发送条件回调函数
                               }
    };
    pro_p->init_pro_frame(csmidinfo, sizeof(csmidinfo) / sizeof(CANPROHEAD));
    pro_p->idfunc[1] = get_cs_mapid;
    pthread_create(&reset_id, NULL, max_reset_process, (void *)this);
    pthread_create(&initproid, NULL, cs_protocol_init, (void *)this);
    pthread_create(&thread_shake, NULL, func, (void *)this);
    pthread_create(&heartManageId, NULL, heartManageThread, (void *)this);
    return 0;
}
/***********************************************************************************
 * 函数名：cs_reset_config
 * 功能：cs重新配置函数
 ***********************************************************************************/
int poll1030_callback(CANp_TIME_ET *poll)
{
    poll->father->can_protocol_send(static_cast<cs_can *>(poll->para)->pollFrame);
    return 0;
}
void cs_can::reset_all_dev(void)
{
    CANFRAMEID  sendmidframe;
    CANDATAFORM sendmiddata;

    memset(&sendmidframe, 0, sizeof(CANFRAMEID));
    memset(&sendmiddata, 0, sizeof(CANDATAFORM));

    sendmidframe.canframework.devaddr_8 = 255;
    sendmidframe.canframework.func_5    = CMD_SEND_MASTERREST;
    sendmidframe.canframework.dir_1     = 1;
    sendmidframe.canframework.zjaddr_3  = 0x07;

    sendmiddata.ExtId = sendmidframe.canframeid;
    sendmiddata.IDE   = 1;

    pro_p->candrip->write_send_data(sendmiddata);
}

void cs_can::add_poll_frame(void)
{
    if (polltimer_id == 0)
    {
        polltimer_id = pro_p->add_poll_frame(2.5, &pollFrame);
    }
}
/***********************************************************************************
 * 函数名：get_dev_type
 * 功能：获得设备类型
 ***********************************************************************************/
uint16_t cs_can::get_dev_type(uint8_t devid)
{
    //    printf("devid = %d,  csmacorder = %d, mac_terminal_num = %d\n",devid, csmacorder, mac_terminal_num);
    if ((devid + 1) > csmacorder)
    {
        return TERMINAL;
    }
    else
    {
        return nconfig_map.val(devid).para.type;
    }
}

void cs_can::set_devonline_num(uint8_t val)
{
    csmacorder = val;
    // zprintf1("csmacorder is %d\n", csmacorder);
}

/***********************************************************************************
 * 函数名：cs_send_data
 * 功能：cs应用发送的协议帧       configtype 0：配置类型第一次配置 1：重新配置
 ***********************************************************************************/
/*configtype ; 0 :No register can of dev 1: Haved register can of dev*/
int cs_can::cs_init(void)
{
    uint32_t test = 0;
    for (uint8_t i = 0; i < BRANCH_ALL; i++)
    {
        state_info.set_ptcan_version(i);
        state_info.set_line_work_state(i, CS_WORK_STATUS_NO_CONFIG);
    }
    zprintf3("|||cs_init||| send reset\n");
    cs_send_data(CMD_SEND_MASTERREST, 0x07, 0, 255, NULL, 0);
    do
    {
        sem_wait(&statechg);
        zprintf3("|||cs_init||| send reset %d\n", test++);
    } while (csstate != CS_CAN_RESET);

    zprintf3("|||cs_init||| send mac check\r\n");
    zprintf1("send mac check frame 1\r\n");
    cs_send_data(CMD_SEND_MACCHECK, 0, 0, 255, NULL, 0);
    do
    {
        sem_wait(&statechg);
    } while (csstate != CS_CAN_MACREQ);
    reset_state = DEV_RESET_CONFIG;
    zprintf3("|||cs_init||| cs mac req end!\n");
    if (csmacstate != MAC_NORMAL || csmacorder == 0)
    {
        zprintf3("|||cs_init||| csmacstate = %d\n", csmacstate);
        for (uint8_t branch = 0; branch < BRANCH_ALL; branch++)
        {
            state_info.set_tail_location(branch, 0);
            state_info.set_dev_num(branch, 0);
            state_info.set_bs_num(branch, 0);
            state_info.set_slaveio_num(branch, 0);
            state_info.set_termal_vol(0, 0);
            // memset( ( state_info.share_state.m_data + i )->line_state.line_state.BS_Buttion, 0, BS_MAX_NUM);
            state_info.set_line_work_state(0, CS_WORK_STATUS_LIVEOUT);
            mac_cszd_have.get_order_datap(branch)->clear( );
        }
        csmacorder             = 0;
        csioorder              = 0;
        cut_check_flag         = 0;
        heart_cs_error_count_g = 0;
        memset(slave_io_num, 0, BRANCH_ALL);
        memset(cszjorder, 0, BRANCH_ALL);
        zprintf1(">?>?>?>?>?clear frist?>?>?>>?>?>?>?%d\r\n", ndev_map.size( ));
        ndev_map.clear( );
        zprintf1(">?>?>?>?>?clear end>?>?>?>>?>?>?>?%d\r\n", ndev_map.size( ));
        init_over = 1;
        return -1;
    }
    for (uint8_t i = 0; i < BRANCH_ALL; i++)
    {
        zprintf3("|||cs_init||| set cszjorder %d mac check\r\n", i);
        state_info.set_dev_num(i, cszjorder[i]);
        state_info.set_bs_num(i, get_bs_mac_num(i));
        state_info.set_slaveio_num(i, slave_io_num[i] - mac_cszd_have.get_order_data(i).mac_terminal_have - mac_cszd_have.get_order_data(i).mac_cs_have);
    }
    set_dev_enable( );
    if (get_config_low_num( ) != get_mac_low_num( ))
    {
        if (auto_reset)
        {
            init_over   = 1;
            csconfstate = csmacorder < cf_file_num ? CSCONF_MORE : CSCONF_LESS;
            // TODO:add zj
            state_info.set_tail_location(0, 0);
            state_info.set_bs_location(0, 0);
            state_info.set_break_location(0, 0);
            return csconfstate;
        }
    }
    uint16_t dev_type;
    for (uint32_t i = 0; i < (uint32_t)slave_io_num[0] + slave_io_num[1] + slave_io_num[2]; i++)
    {
        dev_type = get_dev_type(i);
        if (csdevtyle == 0 && dev_type != TERMINAL)
        {
            if (nconfig_map.val(i).cscrc != nconfig_map.val(i).config_p[3])
            {
                zprintf3("|||cs_init||| config %d style is %x %x %x!\n", i, nconfig_map.val(i).para.type,
                         nconfig_map.val(i).cscrc, nconfig_map.val(i).config_p[3]);
                nconfig_map.val(i).cscrc      = nconfig_map.val(i).config_p[3];
                nconfig_map.val(i).conf_state = CS_CONFIGING;
                csconfstate                   = CSCONF_NEED;
                cf_endnum                     = nconfig_map.val(i).para.id;
            }
        }
    }
    zprintf3("|||cs_init||| cs config start!\n");
    if (csconfstate == CSCONF_NEED)
    {
        zprintf3("|||cs_init||| cs config need!\n");
        send_configdata( );
        do
        {
            sem_wait(&statechg);
        } while (csstate != CS_CAN_NORMAL);
    }
    else
    {
        for (int i = 1; i <= csioorder; i++)
        {
            set_dev_state(i, DEV_ON_LINE);
        }
        cs_send_data(CMD_SEND_PARACORRECT, 0x07, 0, 255, NULL, 0);
        csstate = CS_CAN_NORMAL;
    }
    for (uint8_t i = 0; i < BRANCH_ALL; i++)
    {
        state_info.set_line_work_state(i, CS_WORK_STATUS_RIGHT);
    }
    zprintf3("|||cs_init||| cs config end!\n");
    cs_send_data(CMD_SEND_HEARTCHECK, 0, 0, 0xFF, NULL, 0);

    add_poll_frame( );
    init_over = 1;

    state_info.set_all_dev_state(DEV_ON_LINE);
    reset_msg[0] = RESET_SUCCESS;
    reset_msg[1] = get_config_low_num( );
    reset_msg[2] = get_mac_low_num( );
    reset_msg[3] = cszjorder[0];    // TODO：中继
    if (reset_msg[2] == 0xFF)
    {
        // TODO: zj
        state_info.set_termal_vol(0, 0);
        state_info.set_line_work_state(0, CS_WORK_STATUS_LIVEOUT);
    }
    uint8_t bs_data[2] = { 0 };
    bs_data[0]         = state_info.get_bs_type(0);
    bs_data[1]         = state_info.get_bs_location(0);
    if (bs_data[0] == CS_JT_ERROR)
    {
        nconfig_map.val(0).dev_send_meg(BS_REPORT_MEG, bs_data, 2);
    }
    else
    {
        nconfig_map.val(0).dev_send_meg(BS_REPORT_MEG, bs_data, 1);
    }

    //    uint8_t low_num[2] = {0};

    //    for (int i = 0; i < BRANCH_ALL; i++)
    //    {
    //        low_num[0] = 0;
    //        low_num[1] = slave_io_num[i] - mac_cszd_have.get_order_data(i).mac_terminal_have -
    //                     mac_cszd_have.get_order_data(i).mac_cs_have;
    //        nconfig_map.val(0).dev_send_meg(LOW_NUM_MEG, low_num, sizeof(low_num));
    //    }
    zprintf3("|||cs_init||| cs init end!\n");
    return 0;
}

/***********************************************************************************
 * 函数名：~cs_can()
 * 功能：cs析构函数
 ***********************************************************************************/
cs_can::~cs_can( )
{
    reset_all_dev( );
    zprintf3("destory 1030 cs can!\n");
    if (reset_id != 0)
    {
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
int dev1030_output(void *midp, soutDataUnit val)
{
    cs_can * pro = static_cast<cs_can *>(midp);

    int     devoff;

    uint8_t  outvalue[3];
    uint16_t data = (uint16_t)val.value;

    devoff = val.parentid + val.childid * 0x100;

    if (val.pointid == CMD_SEND_READFUNC_HIGH)
    {
        uint16_t reg_c_v_begin = 0x4C;
        uint16_t read_offset   = 0x04;
        uint8_t  can_data[4]   = { 0 };
        can_data[0]            = reg_c_v_begin;
        can_data[2]            = read_offset;
        zprintf1("read reg \r\n");
        pro->cs_send_data(CMD_SEND_READFUNC_HIGH, 0, 0, val.parentid, can_data, 4);
    }
    else
    {
        struct timeval tv;
        int     order;
        gettimeofday(&tv, NULL);
        // printf("millisecond:%ld\n",tv.tv_sec*1000 + tv.tv_usec/1000);
        if (pro->is_have_config_slaveio(0, 0, devoff, order) == FALSE)
        {
            return -1;
        }

        if (val.parentid > pro->cszjorder[val.childid])
        {
            return -1;
        }
        if (pro->reset_state == DEV_RESET_ING)
        {
            zprintf1("max cs reseting!\n");
            return -2;
        }
        if (pro->get_dev_type(order) == CS_DEV || pro->get_dev_type(order) == TERMINAL)
        {
            zprintf1("max cs no output!\n");
            return -2;
        }

        if (pro->nconfig_map.val(order).devenable != 1)
        {
            zprintf1("dev disable!\n");
            return -3;
        }
        if ((pro->state_info.get_dev_state(pro->nconfig_map.val(order).dev_off) & 0x01) != DEV_ON_LINE)
        {
            zprintf1("max dev%d off line!\n", val.parentid);
            return -2;
        }

        zprintf1("send val  %d %d %d %d\n", val.parentid, val.childid, val.pointid, (int)val.value);
        if (pro->nconfig_map.val(order).get_outnode_tyle(val.pointid) == 0)
        {
            outvalue[0] = data;
            outvalue[0] |= (val.pointid << 2);
            if (val.childid)
            {
                pro->cs_send_data(
                    CMD_SEND_SWOUTPUT, 1, val.childid + 1, pro->nconfig_map.val(order).dev_off, outvalue, 1);
            }
            else
            {
                pro->cs_send_data(CMD_SEND_SWOUTPUT, 0, 0, pro->nconfig_map.val(order).dev_off, outvalue, 1);
            }
        }
        else
        {
            outvalue[0] = val.pointid;
            tbyte_swap((uint16_t *)(&data), 2);
            memcpy(&outvalue[1], &data, 2);
            if (val.childid)
            {
                zprintf1("send value is %d\r\n", outvalue);
                pro->cs_send_data(CMD_SEND_FOUTPUT, 1, val.childid + 1, pro->nconfig_map.val(order).dev_off, outvalue, 3);
            }
            else
            {
                pro->cs_send_data(CMD_SEND_FOUTPUT, 0, 0, pro->nconfig_map.val(order).dev_off, outvalue, 3);
            }
        }
    }
    return 0;
}

#endif /*__1030_PRO_C__*/

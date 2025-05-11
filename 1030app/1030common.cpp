#include "1030common.h"
#include "modbuscrc.h"

const IONUM comdevio[CS_DEVSTY_MAX] = { { 0, 0 }, { 3, 1 }, { 3, 1 }, { 12, 12 }, { 8, 4 }, { 0, 0 }, { 0, 0 }, { 16, 0 }, { 0, 16 }, { 0, 0 }, { 12, 12 }, { 125, 0 }, { 16, 12 } };
/***********************************************************************************
 * 函数名：set_config_head
 * 功能：根据设备类型设置配置参数头
 ***********************************************************************************/

int CAN_DEV_APP::creat_config_info(CAN_DEV_INFO &info)
{
    int i;

    INCOFPARA1   inparam;
    INCOFPARA2   inparam2;
    OUTCOFPARA   outparam;
    bitset< 16 > link_set;

    para = info.para;
    if(para.type == CS_DEV)
    {
        set_cs_config_head();
        if(info.para.link_num & 0x0001)
        {
            config_p[4] = 1;
            zprintf3("cs1 link!\n");
        }
        if(info.para.link_num & 0x0002)
        {
            config_p[5] = 1;
            zprintf3("cs2 link!\n");
        }
    }
    else
    {
        set_config_head();
    }
    devenable = para.enable;
    if(stateinfo != NULL)
    {
        zprintf3("dev %d enable %d\n", para.id, devenable);
        stateinfo->set_dev_enable_state(para.id, devenable);
    }

    uint8_t turnf[configsize * 2];

    for(i = 0; i < para.innum; i++)
    {
        inparam.paraval          = 0;
        inparam.inconf.inenable1 = info.inode[i].node_en;    //使能目前都为1
        inparam.inconf.instyle1  = info.inode[i].datatype != 2 ? 0 : 1;
        inparam.inconf.fautoen1  = info.inode[i].notify_en;
        if(inparam.inconf.instyle1)
        {
            config_p[CONF_HEAD + i * PT_INPUTPARA_MAX]     = inparam.paraval;
            inparam2.para2val                              = 0;
            inparam2.inpara2.reprang8                      = info.inode[i].notify_range;
            inparam2.inpara2.repgap8                       = info.inode[i].notify_time_interval;
            config_p[CONF_HEAD + 1 + i * PT_INPUTPARA_MAX] = inparam2.para2val;
            config_p[CONF_HEAD + 2 + i * PT_INPUTPARA_MAX] = info.inode[i].threshold_max;
            config_p[CONF_HEAD + 3 + i * PT_INPUTPARA_MAX] = info.inode[i].threshold_min;
        }
        else
        {
            config_p[CONF_HEAD + i * PT_INPUTPARA_MAX]     = inparam.paraval;
            config_p[CONF_HEAD + 1 + i * PT_INPUTPARA_MAX] = 0;
            config_p[CONF_HEAD + 2 + i * PT_INPUTPARA_MAX] = 0;
            config_p[CONF_HEAD + 3 + i * PT_INPUTPARA_MAX] = info.inode[i].shake_time;
        }
    }

    link_set.reset();
    for(i = 0; i < para.outnum; i++)
    {
        outparam.oparaval           = 0;
        outparam.outconf.outenable1 = info.onode[i].node_en;
        outparam.outconf.outstyle1  = info.onode[i].datatype != 2 ? 0 : 1;
        if(outparam.outconf.outstyle1)
            outparam.outconf.initval = 200;
        else
            outparam.outconf.initval = 0;

        config_p[CONF_HEAD + PT_INPUTPARA_MAX * para.innum + i] = outparam.oparaval;
        link_set[i]                                             = info.onode[i].link_stop;
        //        zprintf3("dev %d out %d link %d %d\n", para.id, i, info.onode[i].link_stop, info.onode[i].node_en);
    }
    int size = 0;
    if(para.type != CS_DEV)
    {
        size = (para.innum * PT_INPUTPARA_MAX + para.outnum * PT_OUTPUTPARA_MAX) * 2;
    }
    else
    {
        size = 22;
    }
    if(para.outnum > 0)
    {
        info.para.link_num               = (uint16_t)link_set.to_ulong();
        config_p[configsize - CONF_TAIL] = (uint16_t)link_set.to_ulong();
        //        zprintf3("config link 0x%x\n", config_p[configsize-CONF_TAIL]);
        config_p[2] += CONF_TAIL;
        size += CONF_TAIL * 2;
    }
    else if(para.type == TK100_CSModule)
    {
        config_p[configsize - CONF_TAIL] = 6;
        config_p[2] += CONF_TAIL;
        size += CONF_TAIL * 2;
    }
    else if(para.type != CS_DEV)
        configsize -= CONF_TAIL;

    memcpy(turnf, &config_p[CONF_HEAD], size);

    tbyte_swap((uint16_t *)turnf, size);
    Modbus_CRCCal(turnf, size, (uint8_t *)(&config_p[3]));
    //    printf("crc %d\n", config_p[3]);

    return 0;
}

/***********************************************************************************
 * 函数名：set_config_head
 * 功能：根据设备类型设置配置参数头
 ***********************************************************************************/

int CAN_DEV_APP::set_default_config(uint8_t type)
{
    para.type   = type;
    para.innum  = comdevio[type].innum;
    para.outnum = comdevio[type].outnum;

    set_config_head();
    if(type == TERMINAL)
    {
        devenable = 1;
        if(stateinfo != NULL)
        {
            stateinfo->set_dev_enable_state(para.id, devenable);
        }
    }
    else
    {
        devenable = 0;
        if(stateinfo != NULL)
        {
            stateinfo->set_dev_enable_state(para.id, devenable);
        }
    }

    // uint8_t turnf[configsize * 2];
    vector<uint8_t> turnf(configsize * 2);

    int size = (para.innum * PT_INPUTPARA_MAX + para.outnum * PT_OUTPUTPARA_MAX) * 2;
    if(para.outnum > 0)
    {
        config_p[configsize - CONF_TAIL] = 0;
        config_p[2] += CONF_TAIL;
        size += CONF_TAIL * 2;
    }
    else if(para.type == TK100_CSModule)
    {
        zprintf1("tk100 cs must enable\n");
        return -2;
    }
    else
        configsize -= CONF_TAIL;

    memcpy(turnf.data(), &config_p[CONF_HEAD], size);

    tbyte_swap((uint16_t *)turnf.data(), size);
    Modbus_CRCCal(turnf.data(), size, (uint8_t *)(&config_p[3]));

    return 0;
}


int CAN_DEV_APP::reset_default_config(uint8_t type)
{
    if(config_p != NULL)
    {
        delete[] config_p;
        config_p = NULL;
    }
    if(iodata != NULL)
    {
        delete[] iodata;
        iodata = NULL;
    }
    return set_default_config(type);
}

int CAN_DEV_APP::set_share_data(void)
{
    int i;
    if(devdata_p == NULL)
        return -1;
    for(i = 1; i <= para.innum; i++)
    {
        devdata_p->set_share_data_value(dev_off, 0, i, get_input_data(i));
    }
    for(i = 1; i <= para.outnum; i++)
    {
        devdata_p->set_share_data_value(dev_off, 0, i + para.innum, get_output_data(i));
        //zprintf1("write heart id: %d, data: %x\n",i, get_output_data(i));
    }
    return 0;
}

/***********************************************************************************
 * 函数名：void CAN_DEV_APP::dev_send_meg(uint8_t type, uint8_t *data)
 * 功能：设备的消息初始化
 *
 *
 ***********************************************************************************/
void CAN_DEV_APP::dev_send_meg(uint8_t megtype, uint8_t *data, uint16_t size)
{
    if(msgmng_p->dest_id == 0)
    {
        zprintf1("ddddd is %d\n\n", msgmng_p->dest_id);
        return;
    }
    sMsgUnit    pkt;
    Dev_Message devmeg(para.type);
    zprintf3("dev send meg%d %d %d\n\n\n", megtype, sizeof(Dev_Message), size + sizeof(Dev_Message));

    pkt.dest.app                = msgmng_p->dest_id;
    pkt.source.driver.id_driver = msgmng_p->soure_id.driver.id_driver;
    pkt.source.driver.id_parent = dev_off;
    pkt.source.driver.id_child  = 0;
    pkt.source.driver.id_point  = 0;

    pkt.type = MSG_TYPE_DevAutoReport;

    devmeg.meg_type = megtype;
    devmeg.meg_size = size;

    memcpy(pkt.data, &devmeg, sizeof(Dev_Message));
    if(data != NULL)
    {
        memcpy(&pkt.data[sizeof(Dev_Message)], data, size);
    }
    //    printf("pdata 0 is %d\n", pkt.data[sizeof(Dev_Message)]);
    msgmng_p->msgmng_send_msg(&pkt, size + sizeof(Dev_Message));
}

void CAN_DEV_APP::reset_dev_data(void)
{
    errcount   = 0;
    devstate   = 0;
    mac_state  = 0;                        // mac查询应答状态
    conf_state = 0;                        //设备的配置状态
    heart_ok   = 0;                        //心跳ok标志
    memset(csmac, 0x00, sizeof(csmac));    //设备的mac
    cscrc = 0;                             // crc校验

    framark.reset();
}

int CAN_DEV_APP::dev_overtime_process(void)
{
    if(heart_ok == 0)
    {
        if(++errcount > 3)
        {
            if(para.type != TERMINAL)
            {
                stateinfo->set_dev_state_AND(dev_off + 1, DEV_OFF_LINE);    //离线
                zprintf1("1030 dev %d offline reset!\n", dev_off + 1);
                stateinfo->set_dev_num(dev_off / FATHER_DEV_MAX, dev_off % FATHER_DEV_MAX);
            }
            else
            {
                stateinfo->set_termal_state(dev_off / FATHER_DEV_MAX, DEV_OFF_LINE);    //离线
            }
            errcount = 0;
            return DEV_OFF_LINE;
        }
    }
    else
    {
        heart_ok = 0;
    }
    return DEV_ON_LINE;
}

int CAN_DEV_APP::dev_normal_process(void)
{
    // if(heart_ok != 0)
    // {
        heart_ok = 0;
    // }
    return 1;
}

int Max_State_Pro::max_state_pro_init(QString key, int branch_num)
{
    branch_num    = 1;
    void *creat_p = share_state.creat_data(branch_num * sizeof(Max_State_Data), key, ZQTShareMem::Create);
    zprintf1("create max share state size = %d\n", branch_num * MAX_STATE_SIZE);
    if(creat_p == NULL)
    {
        zprintf1("create max share state fail!\n");
        return -1;
    }
    dev_map_init((char *)creat_p);

    for(int k = 0; k < branch_num; k++)
    {
        add_dev(MAX_STATE_LINEID + k * FATHER_DEV_MAX, sizeof(sLine_State));
        add_dev(MAX_STATE_DEV + k * FATHER_DEV_MAX, sizeof(sDevc_State) * DEV_MAX_NUM);
    }
    zprintf3("create max share state!\n");
    return 0;
}


void Max_State_Pro::set_dev_enable_state(int id, uint8_t val)
{
    int max_id = 3 * FATHER_DEV_MAX;
    int min_id = 1;
    // int branch = id / FATHER_DEV_MAX;
    if(id < min_id || id > max_id)
    {
        zprintf1("set dev enable id%d err!\n", id);
        return;
    }
    zprintf1("set dev enable id %d = %d!\n", id, val);
    // share_state.lock_qtshare();
    // //     (share_state.m_data + branch)->devc_state[(id % FATHER_DEV_MAX) - 1].Dev_Enable = val;
    // share_state.unlock_qtshare();
}

void Max_State_Pro::set_dev_state_AND(int id, uint8_t val)    //修改为自动区分id号的设备 设备号从1开始计数
{
    int max_id = 3 * FATHER_DEV_MAX /*cs_have ? (1+PUMP_MAX_NUM +1) : PUMP_MAX_NUM +1*/;
    int branch = id / FATHER_DEV_MAX;
    if(id < 1 || id > max_id)
    {
        zprintf1("set dev state id%d err!\n", id);
        return;
    }
    //        zprintf1("set dev state id %d = %d!\n", id, val);
    share_state.lock_qtshare();
    (share_state.m_data + branch)->devc_state[(id % FATHER_DEV_MAX) - 1].Dev_State &= ~val;
    //      get_dev_info(id)->set_dev_state(val);
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_dev_state_OR(int id, uint8_t val)    //修改为自动区分id号的设备 设备号从1开始计数
{
    int max_id = 3 * FATHER_DEV_MAX /*cs_have ? (1+PUMP_MAX_NUM +1) : PUMP_MAX_NUM +1*/;
    int branch = id / FATHER_DEV_MAX;
    if(id < 1 || id > max_id)
    {
        zprintf1("set dev state id%d err!\n", id);
        return;
    }
    //        zprintf1("set dev state id %d = %d!\n", id, val);
    share_state.lock_qtshare();
    (share_state.m_data + branch)->devc_state[(id % FATHER_DEV_MAX) - 1].Dev_State |= val;
    //      get_dev_info(id)->set_dev_state(val);
    share_state.unlock_qtshare();
}

// void Max_State_Pro::set_dev_in_num(uint8_t branch, uint8_t num, uint8_t val)
// {
//     share_state.lock_qtshare();
//     // (share_state.m_data + branch)->devc_state[num].in_num = val;
//     share_state.unlock_qtshare();
// }

// void Max_State_Pro::set_dev_out_num(uint8_t branch, uint8_t num, uint8_t val)
// {
//     share_state.lock_qtshare();
//     // (share_state.m_data + branch)->devc_state[num].out_num = val;
//     share_state.unlock_qtshare();
// }

void Max_State_Pro::set_dev_version_state(uint8_t branch, uint8_t num, uint16_t boot_v, uint16_t app_v)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->devc_state[num].boot_v = boot_v;
    (share_state.m_data + branch)->devc_state[num].app_v  = app_v;
    share_state.unlock_qtshare();
}


void Max_State_Pro::set_ptcan_version(uint8_t branch)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->line_state.ptcan_v = PTCAN_VERSION_H << 8 | PTCAN_VERSION_M << 4 | PTCAN_VERSION_L;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_config_Slave_IO_Set(uint8_t branch, uint8_t num)
{
    (void)branch;
    (void)num;
    // share_state.lock_qtshare();
    // //    (share_state.m_data + branch)->line_state.para_config.Slave_IO_Set = num;
    // share_state.unlock_qtshare();
}

void Max_State_Pro::set_config_Auto_Reset(uint8_t branch, uint8_t flag)
{
    (void)branch;
    (void)flag;
    // share_state.lock_qtshare();
    // //    (share_state.m_data + branch)->line_state.para_config.Auto_Reset = flag;
    // share_state.unlock_qtshare();
}

void Max_State_Pro::set_termal_state(uint8_t branch, uint8_t val)
{
    (void)branch;
    (void)val;
    // share_state.lock_qtshare();
    // //    (share_state.m_data + branch)->line_state.line_state.ZD_State = val;
    // share_state.unlock_qtshare();
}

bool Max_State_Pro::set_dev_num(uint8_t branch, uint8_t num)
{
    bool ret;
    share_state.lock_qtshare();
    if((share_state.m_data + branch)->line_state.Dev_Exist == num)
    {
        ret = false;
    }
    else
    {
        (share_state.m_data + branch)->line_state.Dev_Exist = num;
        ret                                               = true;
    }
    share_state.unlock_qtshare();
    return ret;
}

bool Max_State_Pro::set_bs_num(uint8_t branch, uint8_t num)
{
    bool ret;
    share_state.lock_qtshare();
    if((share_state.m_data + branch)->line_state.BS_Num_Exist == num)
        ret = false;
    else
    {
        (share_state.m_data + branch)->line_state.BS_Num_Exist = num;
        ret                                                  = true;
    }
    share_state.unlock_qtshare();
    return ret;
}


void Max_State_Pro::set_dev_type(uint8_t branch, uint8_t num, uint8_t val)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->devc_state[num].Dev_Type = val;
    share_state.unlock_qtshare();
}


void Max_State_Pro::set_dev_ID(uint8_t branch, uint8_t num, uint8_t val)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->devc_state[num].Dev_ID = val;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_dev_link(uint8_t branch, uint8_t num, uint8_t val)
{
    (void)branch;
    (void)num;
    (void)val;
    // share_state.lock_qtshare();
    // //    (share_state.m_data + branch)->devc_state[num].Dev_Link = val;
    // share_state.unlock_qtshare();
}

bool Max_State_Pro::set_slaveio_num(uint8_t branch, uint8_t num)
{
    bool ret;
    share_state.lock_qtshare();
    if((share_state.m_data + branch)->line_state.Slave_IO_Exist == num)
        ret = false;
    else
    {
        zprintf3("|||share|||-----set slaveio num is %d\n", num);
        (share_state.m_data + branch)->line_state.Slave_IO_Exist = num;
        ret                                                    = true;
    }
    share_state.unlock_qtshare();
    return ret;
}

uint8_t Max_State_Pro::get_dev_state(int id)    //添加得到设备状态 设备号从1开始计数
{
    int max_id = 3 * FATHER_DEV_MAX /*cs_have ? (1+PUMP_MAX_NUM +1) : PUMP_MAX_NUM +1*/;
    int branch = id / FATHER_DEV_MAX;
    if(id < 1 || id > max_id)
    {
        zprintf1("get dev state id%d err!\n", id);
        return DEV_STATE_MAX;
    }
    return (share_state.m_data + branch)->devc_state[(id % FATHER_DEV_MAX) - 1].Dev_State;
}


void Max_State_Pro::set_cs_state(uint8_t branch, uint8_t v1, uint16_t c1, uint8_t v2, uint16_t c2)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->line_state.CS_Volte[0]   = v1;
    (share_state.m_data + branch)->line_state.CS_Volte[1]   = v2;
    (share_state.m_data + branch)->line_state.CS_Current[0] = c1;
    (share_state.m_data + branch)->line_state.CS_Current[1] = c2;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_cs_state2(uint8_t branch, uint8_t state)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->line_state.CS_State = ((share_state.m_data + branch)->line_state.CS_State & 0xFC) | state;
    share_state.unlock_qtshare();
}
void Max_State_Pro::set_cs_bs_state(uint8_t branch, uint8_t val)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->line_state.BS_State = val;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_termal_vol(uint8_t branch, uint8_t val)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->line_state.ZD_Volte = val;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_bs_state(uint8_t branch, uint8_t addr, uint8_t state)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->devc_state[addr].Dev_State = state;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_bs_location_type(uint8_t branch, uint8_t location, uint8_t type)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->line_state.BS_Location = location;
    (share_state.m_data + branch)->line_state.BS_State    = type;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_bs_location(uint8_t branch, uint8_t location)
{
    share_state.lock_qtshare();

    (share_state.m_data + branch)->line_state.BS_Location = location;

    share_state.unlock_qtshare();
}

uint8_t Max_State_Pro::get_bs_location(uint8_t branch)
{
    uint8_t location;
    share_state.lock_qtshare();

    location = (share_state.m_data + branch)->line_state.BS_Location;

    share_state.unlock_qtshare();
    return location;
}

void Max_State_Pro::set_all_state(uint8_t val)
{
    share_state.lock_qtshare();

    //    for (int i = 0; i<3; i++)
    //    {
    for(int id = 0; id < DEV_MAX_NUM; id++)
    {
        (share_state.m_data)->devc_state[id].Dev_State = val;
        if(val == DEV_OFF_LINE)
        {
            (share_state.m_data)->devc_state[id].Dev_ID   = 0;
            (share_state.m_data)->devc_state[id].Dev_Type = 0;
            // (share_state.m_data)->devc_state[id].in_num   = 0;
            // (share_state.m_data)->devc_state[id].out_num  = 0;
        }
    }
    //    }
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_all_state_clear()
{
    zprintf3("run here first\n");
    set_slaveio_num(0, 0);
    //    set_slaveio_num(1,0);
    //    set_slaveio_num(2,0);
    set_cs_state2(0, 0);
    //    set_cs_state2(1, 0);
    //    set_cs_state2(2, 0);
    set_dev_num(0, 0);
    //    set_dev_num(1, 0);
    //    set_dev_num(2, 0);
    set_bs_num(0, 0);
    //    set_bs_num(1, 0);
    //    set_bs_num(2, 0);
    set_cs_bs_state(0, 0);
    //    set_cs_bs_state(1, 0);
    //    set_cs_bs_state(2, 0);
    //    memset(share_state.m_data->line_state.BS_Buttion, 0, BS_MAX_NUM);
    //    memset((share_state.m_data+1)->line_state.BS_Buttion, 0, BS_MAX_NUM);
    //    memset((share_state.m_data+2)->line_state.BS_Buttion, 0, BS_MAX_NUM);
}

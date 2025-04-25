#include "1030common.h"
#include "modbuscrc.h"
#include <vector>

const IONUM comdevio[CS_DEVSTY_MAX] = { { 4, 2 }, { 12, 12 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 12, 12 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 12, 12 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };
/***********************************************************************************
 * 函数名：set_config_head
 * 功能：根据设备类型设置配置参数头
 ***********************************************************************************/
int CAN_DEV_APP::creat_config_info(CAN_DEV_INFO &info)
{
    int          i;
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
        }
    }
    else
    {
        set_config_head();
    }
    devenable = para.enable;
    if(stateinfo != NULL)
    {
        //        zprintf3("dev %d enable %d\n", dev_off+1, devenable);
        //        stateinfo->set_dev_enable_state(dev_off+1, devenable);
    }

    // uint8_t turnf[configsize * 2];
    vector<uint8_t> turnf(configsize * 2);

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
        {
            outparam.outconf.initval = 200;
        }
        else
        {
            outparam.outconf.initval = info.onode[i].datatype;
        }
        outparam.outconf.link_stop = info.onode[i].link_stop;

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
        size = 2;
    }
    if(para.outnum > 0)
    {
        info.para.link_num = (uint16_t)link_set.to_ulong();
        configsize -= CONF_TAIL;
    }
    else if(para.type == TK100_CSModule)
    {
        config_p[configsize - CONF_TAIL] = 6;
        config_p[2] += CONF_TAIL;
        size += CONF_TAIL * 2;
    }
    else if(para.type != CS_DEV)
        configsize -= CONF_TAIL;

    memcpy(turnf.data(), &config_p[CONF_HEAD], size);

    tbyte_swap((uint16_t *)turnf.data(), size);
    Modbus_CRCCal(turnf.data(), size, (uint8_t *)(&config_p[3]));
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
            stateinfo->set_dev_enable_state(dev_off + 1, devenable);
        }
    }
    else
    {
        devenable = 0;
        if(stateinfo != NULL)
        {
            stateinfo->set_dev_enable_state(dev_off + 1, devenable);
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
    zprintf3("para.id %d crc %d\n", para.id, config_p[3]);
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
    //    printf("slaveio_order = %d\r\n",slaveio_order);
    for(i = 1; i <= para.innum; i++)
    {
        devdata_p->set_share_data_value(slaveio_order - 1, 0, i, get_input_data(i));
    }
    for(i = 1; i <= para.outnum; i++)
    {
        devdata_p->set_share_data_value(slaveio_order - 1, 0, i + para.innum, get_output_data(i));
    }
    return 0;
}

/***********************************************************************************
 * 函数名：void CAN_DEV_APP::dev_send_meg(uint8_t type, uint8_t *data)
 * 功能：设备的消息初始化
 ***********************************************************************************/
void CAN_DEV_APP::dev_send_meg(uint8_t megtype, uint8_t *data, uint16_t size)
{
    if(msgmng_p->dest_id == 0)
    {
        return;
    }
    sMsgUnit    pkt;
    Dev_Message devmeg(para.type);
    zprintf1("===report===: megtype = %d,data = ", megtype);
    for(uint8_t i = 0; i < size; i++)
    {
        zprintf3(" %x ", data[i]);
    }
    zprintf3("\r\n");

    pkt.dest.app                = msgmng_p->dest_id;
    pkt.source.driver.id_driver = msgmng_p->soure_id.driver.id_driver;
    pkt.source.driver.id_parent = dev_off;
    pkt.source.driver.id_child  = 0;
    pkt.source.driver.id_point  = 0;

    pkt.type = MSG_TYPE_DevAutoReport;

    devmeg.meg_type = megtype;
    devmeg.meg_state = 1;
    devmeg.meg_size = size;

    memcpy(pkt.data, &devmeg, sizeof(Dev_Message));
    if(data != NULL)
    {
        memcpy(&pkt.data[sizeof(Dev_Message)], data, size);
    }
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

    para.id = 0;
    zjnum   = 0;
    csnum   = 0;

    framark.reset();
}

int CAN_DEV_APP::dev_normal_process(void)
{
    // if(heart_ok != 0)
    // {
    //     heart_ok = 0;
    // }
    heart_ok = 0;
    return 1;
}

int Max_State_Pro::max_state_pro_init(QString key, int branch_num)
{
    branch_num    = 3;
    void *creat_p = share_state.creat_data(branch_num * MAX_STATE_SIZE, key);
    zprintf3("create max share state size = %d\n", branch_num * MAX_STATE_SIZE);
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
    (void) val;
    int max_id = 3 * FATHER_DEV_MAX /*cs_have ? (1+PUMP_MAX_NUM +1) : PUMP_MAX_NUM+1*/;
    int min_id = 1;
    // int branch = id / FATHER_DEV_MAX;
    if(id < min_id || id > max_id)
    {
        zprintf1("set dev enable id <%d> err!\n", id);
        return;
    }
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
    share_state.lock_qtshare();
    (share_state.m_data + branch)->devc_state[(id % FATHER_DEV_MAX) - 1].Dev_State &= ~val;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_dev_status(int id, uint8_t val)
{
    int branch = id / FATHER_DEV_MAX;
    // zprintf1("set dev <%d> is %d\r\n",(id % FATHER_DEV_MAX) - 1,val);
    share_state.lock_qtshare();
    if(val)
    {
        SET_NTH_BIT((share_state.m_data + branch)->devc_state[(id % FATHER_DEV_MAX) - 1].Dev_State, 0);
    }
    else
    {
        CLEAR_NTH_BIT((share_state.m_data + branch)->devc_state[(id % FATHER_DEV_MAX) - 1].Dev_State, 0);
    }
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_dev_state_OR(int id, uint8_t val)    //修改为自动区分id号的设备 设备号从1开始计数
{
    int max_id = 3 * FATHER_DEV_MAX /*cs_have ? (1+PUMP_MAX_NUM +1) : PUMP_MAX_NUM +1*/;
    int branch = id / FATHER_DEV_MAX;
    if(id < 1 || id > max_id)
    {
        zprintf1("set dev state id %d err!\n", id);
        return;
    }
    share_state.lock_qtshare();
    (share_state.m_data + branch)->devc_state[(id % FATHER_DEV_MAX) - 1].Dev_State |= val;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_ptcan_version(uint8_t branch)
{
    share_state.lock_qtshare();
       (share_state.m_data + branch)->line_state.ptcan_v = PTCAN_VERSION_H << 8 | PTCAN_VERSION_M << 4 |
       PTCAN_VERSION_L;
    share_state.unlock_qtshare();
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
        zprintf3("|||share|||-----set dev num is %d\r\n", num);
        ret = true;
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
    (share_state.m_data + branch)->devc_state[num].boot_v   = 0;
    (share_state.m_data + branch)->devc_state[num].app_v    = 0;
    (share_state.m_data + branch)->devc_state[num].value_v  = 0;
    (share_state.m_data + branch)->devc_state[num].value_c  = 0;
    share_state.unlock_qtshare();
}

uint8_t Max_State_Pro::get_dev_type(uint8_t branch, uint8_t num)
{
    uint8_t location;
    share_state.lock_qtshare();
    location = (share_state.m_data + branch)->devc_state[num].Dev_Type;
    share_state.unlock_qtshare();
    return location;
}

void Max_State_Pro::set_dev_ID(uint8_t branch, uint8_t num, uint8_t val)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->devc_state[num].Dev_ID = val;
    share_state.unlock_qtshare();
}

bool Max_State_Pro::set_slaveio_num(uint8_t branch, uint8_t num)
{
    bool ret;
    share_state.lock_qtshare();

    if((share_state.m_data + branch)->line_state.Slave_IO_Exist == num)
    {
        ret = false;
    }
    else
    {
        zprintf3("|||share|||-----set slaveio num is %d\r\n", num);
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

uint8_t Max_State_Pro::get_dev_num(int branch)    //添加得到设备状态 设备号从1开始计数
{
    uint8_t location;
    share_state.lock_qtshare();
    location = (share_state.m_data + branch)->line_state.Dev_Exist;
    share_state.unlock_qtshare();
    return location;
}

void Max_State_Pro::set_dev_misc1_state(uint8_t branch, uint8_t num, uint8_t io_num, uint8_t value)
{
    share_state.lock_qtshare();
    if (value == true)
    {
        (share_state.m_data + branch)->devc_state[num].misc_1 = SET_NTH_BIT((share_state.m_data + branch)->devc_state[num].misc_1, io_num);
    }
    else
    {
        (share_state.m_data + branch)->devc_state[num].misc_1 = CLEAR_NTH_BIT((share_state.m_data + branch)->devc_state[num].misc_1, io_num);
    }
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_dev_cv_state(uint8_t branch, uint8_t num, uint8_t v, uint8_t c)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->devc_state[num].value_v = v;
    (share_state.m_data + branch)->devc_state[num].value_c = c;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_dev_version_state(uint8_t branch, uint8_t num, uint16_t boot_v, uint16_t app_v)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->devc_state[num].boot_v = boot_v;
    (share_state.m_data + branch)->devc_state[num].app_v  = app_v;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_cs_av_state(uint8_t branch, uint8_t v1, uint16_t c1, uint8_t v2, uint16_t c2)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->line_state.CS_Volte[0]   = v1;
    (share_state.m_data + branch)->line_state.CS_Volte[1]   = v2;
    (share_state.m_data + branch)->line_state.CS_Current[0] = c1;
    (share_state.m_data + branch)->line_state.CS_Current[1] = c2;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_voip_state(uint8_t branch, uint8_t state)
{
    share_state.lock_qtshare();
    zprintf3("|||share|||-----set_voip_state is %d\r\n", state);
    if(state)
    {
        (share_state.m_data + branch)->line_state.lineState =
            SET_NTH_BIT(((share_state.m_data + branch)->line_state.lineState), 5);
    }
    else
    {
        (share_state.m_data + branch)->line_state.lineState =
            CLEAR_NTH_BIT((share_state.m_data + branch)->line_state.lineState, 5);
    }
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_line_18v_state(uint8_t branch, uint8_t state, uint8_t lineNum)
{
    share_state.lock_qtshare();
    uint8_t lineNumOffset = 3;
    uint8_t lineNumBit    = lineNumOffset + lineNum;
    zprintf3("|||share|||-----set_line_18v_state is %d %d\r\n", lineNum, state);
    if(state)
    {
        (share_state.m_data + branch)->line_state.lineState =
            CLEAR_NTH_BIT((share_state.m_data + branch)->line_state.lineState, lineNumBit);
    }
    else
    {
        (share_state.m_data + branch)->line_state.lineState =
            SET_NTH_BIT(((share_state.m_data + branch)->line_state.lineState), lineNumBit);
    }
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_line_work_state(uint8_t branch, uint8_t state)
{
    share_state.lock_qtshare();

    (share_state.m_data + branch)->line_state.lineState =
        (((share_state.m_data + branch)->line_state.lineState & 0xFC) | state);
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_termal_vol(uint8_t branch, uint8_t val)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->line_state.ZD_Volte = val;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_tail_location(uint8_t branch, uint8_t location)
{
    uint8_t dev_type = get_dev_type(branch, location - 1);
    uint8_t dev_num  = get_dev_num(branch);
    share_state.lock_qtshare();
    if(dev_num >= location && dev_type != TERMINAL)
    {
        if((share_state.m_data + branch)->line_state.Tail_Location != location)
        {
            zprintf3("|||share|||-----set tail location is %d\r\n", location);
            (share_state.m_data + branch)->line_state.Tail_Location = location;
        }
    }
    share_state.unlock_qtshare();
}

uint8_t Max_State_Pro::get_tail_location(uint8_t branch)
{
    uint8_t location;
    share_state.lock_qtshare();
    location = (share_state.m_data + branch)->line_state.Tail_Location;
    share_state.unlock_qtshare();
    return location;
}

void Max_State_Pro::set_bs_state(uint8_t branch, uint8_t addr, uint8_t state)
{
    share_state.lock_qtshare();
    if(state)
    {
        (share_state.m_data + branch)->devc_state[addr].Dev_State |= 4;
        // zprintf3("BS_Buttion dev %d is lock \r\n", addr);
        (share_state.m_data + branch)->line_state.BS_Buttion[addr / 8] |= (1 << (addr % 8));
    }
    else
    {
        (share_state.m_data + branch)->devc_state[addr].Dev_State &= ~4;
        (share_state.m_data + branch)->line_state.BS_Buttion[addr / 8] &= (~(1 << (addr % 8)));
    }
    share_state.unlock_qtshare();
}

uint8_t Max_State_Pro::get_bs_is_have(uint8_t branch)
{
    for(uint8_t i = 0; i < BS_MAX_NUM; i++)
    {
        if((share_state.m_data + branch)->line_state.BS_Buttion[i] != 0)
        {
            return 1;
        }
    }
    return 0;
}
// TODO
// void Max_State_Pro::set_dev_config_state(uint8_t branch, uint8_t addr, uint8_t state)
//{
//    share_state.lock_qtshare();
//    if (state)
//    {
//        (share_state.m_data + branch)->devc_state[addr].Dev_State |= 4;
//        (share_state.m_data + branch)->line_state.BS_Buttion[addr/8] |= (1 << (addr%8));
//    }
//    else
//    {
//        (share_state.m_data + branch)->devc_state[addr].Dev_State &= ~4;
//        (share_state.m_data + branch)->line_state.BS_Buttion[addr/8] &= (~(1 << (addr%8)));
//    }
//    share_state.unlock_qtshare();
//}

void Max_State_Pro::set_bs_type(uint8_t branch, uint8_t type)
{
    share_state.lock_qtshare();
    (share_state.m_data + branch)->line_state.BS_State = type;
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_bs_location(uint8_t branch, uint8_t location)
{
    share_state.lock_qtshare();
#ifdef NO_LOCK_ERROR
    location = 0;
#endif
    if((share_state.m_data + branch)->line_state.BS_Location != location)
    {
        zprintf3("|||share|||-----set banch: %d, break_location is %d\r\n", branch, location);
        (share_state.m_data + branch)->line_state.BS_Location = location;
    }
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_break_location(uint8_t branch, uint8_t location)
{
    uint8_t dev_num = get_dev_num(branch);
    share_state.lock_qtshare();
    if(dev_num >= location)
    {
        if((share_state.m_data + branch)->line_state.break_location != location)
        {
            zprintf3("|||share|||-----set branch: %d, break_location is %d\r\n", branch, location);
            (share_state.m_data + branch)->line_state.break_location = location;
        }
    }
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

uint8_t Max_State_Pro::get_break_location(uint8_t branch)
{
    uint8_t location;
    share_state.lock_qtshare();

    location = (share_state.m_data + branch)->line_state.break_location;

    share_state.unlock_qtshare();
    return location;
}

uint8_t Max_State_Pro::get_bs_type(uint8_t branch)
{
    uint8_t type;
    share_state.lock_qtshare();
    type = (share_state.m_data + branch)->line_state.BS_State;
    share_state.unlock_qtshare();
    return type;
}

void Max_State_Pro::set_all_dev_state(uint8_t val)
{
    share_state.lock_qtshare();

    for(int branch = 0; branch < BRANCH_ALL; branch++)
    {
        if(val == DEV_OFF_LINE)
        {
            for(int id = 0; id < DEV_MAX_NUM; id++)
            {
                (share_state.m_data + branch)->devc_state[id].Dev_State = val;

                (share_state.m_data + branch)->devc_state[id].Dev_ID   = 0;
                (share_state.m_data + branch)->devc_state[id].Dev_Type = 0;
                (share_state.m_data + branch)->devc_state[id].boot_v   = 0;
                (share_state.m_data + branch)->devc_state[id].app_v    = 0;
                (share_state.m_data + branch)->devc_state[id].value_v  = 0;
                (share_state.m_data + branch)->devc_state[id].value_c  = 0;
            }
        }
        else
        {
            for(int id = 0; id < (share_state.m_data + branch)->line_state.Dev_Exist; id++)
            {
                (share_state.m_data + branch)->devc_state[id].Dev_State = val;
            }
        }
    }
    share_state.unlock_qtshare();
}

void Max_State_Pro::set_all_state_clear()
{
    for(uint8_t branch = 0; branch < BRANCH_ALL; branch++)
    {
        set_line_work_state(branch, CS_WORK_STATUS_LIVEOUT);
        set_slaveio_num(branch, 0);
        set_dev_num(branch, 0);
        set_tail_location(branch, 0);
        set_bs_num(branch, 0);
        set_bs_type(branch, 0);
        // memset( ( share_state.m_data + i )->line_state.line_state.BS_Buttion, 0, BS_MAX_NUM );
    }
}

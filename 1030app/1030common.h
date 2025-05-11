#ifndef __1030COMMON_H__
#define __1030COMMON_H__

#include <stdint.h>

#include "candatainfo.h"
#include "tkcommon.h"
#include "zprint.h"
#include "ptcommon/ptdevcommon.h"

using namespace std;


#define CS_INDATA_SIZE      2
#define CS_OUTDATA_SIZE     2
#define CS_POLL_HEAD        1     //16bit为单位

#define CS_CONFIG_BUF       11

#define CSREG_BUF_MAX   (CSDEV_INNODE_NUM*CS_INDATA_SIZE+CSDEV_OUTNODE_NUM*CS_OUTDATA_SIZE)

#define DEVPOLLSIZE(devtyle) ((comdevio[devtyle].innum*CS_INDATA_SIZE+comdevio[devtyle].outnum*CS_OUTDATA_SIZE+CS_POLL_HEAD)*2)




class Max_State_Pro;



class CAN_DEV_COMMON{
public:
    can_dev_para      para;
    uint8_t           devenable;
    uint8_t           conferr;
    uint8_t           devstate;
    uint8_t           conf_state; //设备的配置状态
    CAN_DEV_COMMON(){
        memset(this, 0x00,sizeof(CAN_DEV_COMMON));
    }
};
class CAN_DEV_APP_INFO
{
public:
    uint8_t           dev_off;
    uint8_t           devenable;
    uint8_t           errcount;
    uint8_t           devstate;
    uint8_t           mac_state;   //mac查询应答状态
    uint8_t           conf_state; //设备的配置状态
    uint8_t           heart_ok;   //心跳ok标志
    uint8_t           csmac[6];  //设备的mac
    uint16_t          cscrc;     //crc校验
    uint16_t          configsize;


    uint16_t  *             config_p;
    uint16_t *              iodata;
    Max_State_Pro *         stateinfo;

    MsgMng         *        msgmng_p;
    Pt_Devs_ShareData  *         devdata_p;
public:
    CAN_DEV_APP_INFO()
    {
        memset(this, 0x00, sizeof(CAN_DEV_APP_INFO));
    }
    ~CAN_DEV_APP_INFO()
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
    }
};

class CAN_DEV_APP:public CAN_DEV_APP_INFO
{
public:
    bitset<HEART_MAX>       framark;  //接收帧完整性验证
    can_dev_para            para;
    CAN_DEV_APP  *          child;

public:
    CAN_DEV_APP():child(NULL)
    {

        framark.reset();
        msgmng_p = MsgMng::GetMsgMng();
        CAN_DEV_PARA_INIT(para);
    }
    ~CAN_DEV_APP(){
        zprintf3("CAN_DEV_APP destruct!\n");
    }

    int set_config_head(void)
    {
        configsize = para.innum *PT_INPUTPARA_MAX+ CONF_TAIL+
                                      para.outnum*PT_OUTPUTPARA_MAX+ CONF_HEAD;
        config_p = new uint16_t[configsize];
        iodata = new uint16_t[para.innum * CS_INDATA_SIZE + para.outnum * CS_OUTDATA_SIZE];

        if(config_p == NULL || iodata == NULL)
        {
            zprintf3("creat config info fail\n");
            return -1;
        }
        memset(config_p, 0x00, configsize*2);
        config_p[0] = para.type|0x200;
        config_p[1] = 0x7;
        config_p[2] = configsize - CONF_HEAD- CONF_TAIL;
        config_p[3] = 0;                                 //存放crc的地方
        return 0;

    }

    int set_cs_config_head(void)
    {
        configsize = CS_CONFIG_BUF + CONF_HEAD;
        config_p = new uint16_t[configsize];

        if(config_p == NULL)
        {
            zprintf3("creat config info fail\n");
            return -1;
        }
        memset(config_p, 0x00, configsize*2);
        config_p[0] = para.type|0x200;
        config_p[1] = 0x4;
        config_p[2] = 0X0B;
        config_p[3] = 0;                                 //存放crc的地方

        config_p[7] = 0x32;
        config_p[9] = 0x00;
        config_p[10] = 0x00;
        config_p[12] = 0x32;
        config_p[14] = 0xffff;
        return 0;

    }

    int get_innode_tyle(int node) //点从1开始排序
    {
        INCOFPARA1 inparam;
        inparam.paraval = config_p[CONF_HEAD+(node-1)*PT_INPUTPARA_MAX];
        return  inparam.inconf.instyle1;
    }

    int get_outnode_tyle(int node) //点从1开始排序
    {
         OUTCOFPARA outparam;
        outparam.oparaval = config_p[CONF_HEAD+ para.innum* PT_INPUTPARA_MAX+(node-1)*PT_OUTPUTPARA_MAX];
        return  outparam.outconf.outstyle1;
    }

    uint16_t get_input_data(int node) //点从1开始排序
    {
        if(get_innode_tyle(node) == 0)
        {
           INCOFPARA1      inparam;
            inparam.paraval = iodata[(node-1)*CS_INDATA_SIZE];
            return  inparam.inval.instate7;
        }
        else
        {
            return iodata[(node-1)*CS_INDATA_SIZE+1];
        }
    }

    uint16_t get_output_data(int node) //点从1开始排序
    {
        if(get_outnode_tyle(node) == 0)
        {
            return  iodata[para.innum *CS_INDATA_SIZE+ (node-1)*CS_OUTDATA_SIZE];
        }
        else
        {
            return iodata[para.innum *CS_INDATA_SIZE+(node-1)*CS_OUTDATA_SIZE+1];
        }
    }

    int creat_config_info(CAN_DEV_INFO & info);
    int set_default_config(uint8_t type);
    int reset_default_config(uint8_t type);
    void reset_dev_data(void);
    int set_share_data(void);
    void dev_send_meg(uint8_t megtype, uint8_t *data, uint16_t size);
    int dev_overtime_process(void);   //设备超时处理
    int dev_normal_process(void);


};


class Max_State_Pro:public Dev_Map_T<char>
{
public:

    int                             cs_have;
    QTShareDataT<Max_State_Data>    share_state;
public:
    Max_State_Pro(){
        cs_have = 0;
    }
    ~Max_State_Pro(){
        zprintf3("destory Max_State_Pro!\n");
    }

    int max_state_pro_init(QString key, int branch_num);
    void set_cs_have(int val)
    {
        cs_have = val;
    }

    void set_dev_enable_state(int id, uint8_t val);
    void set_dev_state_AND(int id, uint8_t val);
    void set_dev_state_OR(int id, uint8_t val);
    void set_ptcan_version(uint8_t branch);
    void set_config_Slave_IO_Set(uint8_t branch, uint8_t num);
    void set_config_Auto_Reset(uint8_t branch, uint8_t flag);
    void set_termal_state(uint8_t branch, uint8_t val);
    void set_cs_state(uint8_t branch, uint8_t v1, uint16_t c1, uint8_t v2, uint16_t c2);
    void set_cs_state2(uint8_t branch, uint8_t state);
    void set_bs_location_type(uint8_t branch, uint8_t location, uint8_t type);
    void set_bs_location(uint8_t branch, uint8_t location);
    uint8_t get_bs_location(uint8_t branch);
    void set_all_state(uint8_t val);
    void set_all_state_clear(void);
    void set_cs_bs_state(uint8_t branch, uint8_t val);
    void set_termal_vol(uint8_t branch, uint8_t val);
    void set_bs_state(uint8_t branch, uint8_t addr, uint8_t state);
    bool set_slaveio_num(uint8_t branch, uint8_t num);
    void set_dev_version_state(uint8_t branch, uint8_t num, uint16_t boot_v, uint16_t app_v);
    bool set_dev_num(uint8_t branch, uint8_t num);
    bool set_bs_num(uint8_t branch, uint8_t num);
    void set_dev_type(uint8_t branch, uint8_t num, uint8_t val);
    void set_dev_ID(uint8_t branch, uint8_t num, uint8_t val);
    void set_dev_link(uint8_t branch, uint8_t num, uint8_t val);
    uint8_t get_dev_state(int id);
    PT_Dev_State * get_dev_info(uint id)
    {
        return ((PT_Dev_State *)get_dev_addr(id));
    }
};

extern const IONUM comdevio[CS_DEVSTY_MAX];
#endif // __1030COMMON_H__


#ifndef __1030COMMON_H__
#define __1030COMMON_H__

#include <stdint.h>
#include "candatainfo.h"
#include "tkcommon.h"
#include "zprint.h"

using namespace std;

#define PTCAN_VERSION_H 1
#define PTCAN_VERSION_M 7
#define PTCAN_VERSION_L 2

#define PT_INPUTPARA_MAX  5
#define PT_OUTPUTPARA_MAX 1

#define CS_MACHINE_NUM 3
#define IO_MACHINE_NUM 1

#define READ_REG_MAX_NUM 200

#define CONF_HEAD 4
#define CONF_TAIL 1

#define CSDEV_INNODE_NUM  16
#define CSDEV_OUTNODE_NUM 16

#define CS_INDATA_SIZE  1
#define CS_OUTDATA_SIZE 1
#define CS_POLL_HEAD    1    // 16bit为单位

#define CS_CONFIG_BUF 1

#define CONFIG_BUF_MAX \
    (CONF_HEAD + (CSDEV_INNODE_NUM * PT_INPUTPARA_MAX + CSDEV_OUTNODE_NUM * PT_OUTPUTPARA_MAX) + CONF_TAIL)
#define CSREG_BUF_MAX (CSDEV_INNODE_NUM * CS_INDATA_SIZE + CSDEV_OUTNODE_NUM * CS_OUTDATA_SIZE)

#define DEVPOLLSIZE(devtyle) \
    ((comdevio[devtyle].innum * CS_INDATA_SIZE + comdevio[devtyle].outnum * CS_OUTDATA_SIZE + CS_POLL_HEAD) * 2)
#define INNODEINFO(x) ((x)*PT_INPUTPARA_MAX + CONF_HEAD)

#define SET_NTH_BIT(x, n)   (x | 1U << n)
#define CLEAR_NTH_BIT(x, n) (x & ~(1U << n))
#define IS_ONE(number, n)   ((number >> n) & (0x1))
enum
{
    RESET_SUCCESS = 0,
    RESET_FAIL
};

#define CJ_JT_18V_OFFSET 2
enum
{
    CS_JT_OK = 0,
    CS_JT_ERROR,
};

// enum
//{
//    BS_REPORT_BREAK = 2,
//    BS_REPORT_CUT
//};
typedef enum
{
    BRANCH_SELF = 0,
    BRANCH_ZJ,    // 本沿线中继后的CS2
    BRANCH_LS,    // 本沿线中继后的LS
    BRANCH_ALL
} CS_BRANCH_TYPE;

enum
{
    CS_WORK_STATUS_LIVEOUT = 0,
    CS_WORK_STATUS_RIGHT,
    CS_WORK_STATUS_NO_CONFIG,
    CS_WORK_STATUS_CONFIGING,
};

typedef enum
{
    PTCAN_RESET_NORMAL = 0,
    PTCAN_RESET_SALVE,
    PTCAN_RESET_IO_NOACK,
    PTCAN_RESET_LINE_ERROR,
    PTCAN_RESET_HEART_OVERTIME,
    PTCAN_RESET_INIT
} PTCAN_RESET_E;

typedef struct
{
    uint8_t innum;
    uint8_t outnum;
} IONUM;

class Max_State_Pro;

/*************配置信息结构体***************************************************************/
typedef struct
{
    uint16_t csdevtype;
    uint8_t  csdevenable;
    uint8_t  csconferr;
    uint16_t bufsize;
    uint16_t statepoint[CONFIG_BUF_MAX];
} CSDATAREG;

typedef union
{
    uint16_t paraval;
    struct
    {
        uint16_t ignore13 : 13;
        uint16_t fautoen1 : 1;
        uint16_t inenable1 : 1;
        uint16_t instyle1 : 1;
    } inconf;
    struct
    {
        uint16_t instate7 : 7;
        uint16_t ignorel7 : 7;
        uint16_t inenable1 : 1;
        uint16_t instyle1 : 1;
    } inval;
} INCOFPARA1;

typedef union
{
    struct
    {
        uint16_t reprang8 : 8;
        uint16_t repgap8 : 8;
    } inpara2;
    uint16_t para2val;
} INCOFPARA2;

typedef union
{
    uint16_t oparaval;
    struct
    {
        uint16_t initval : 13;
        uint16_t link_stop : 1;
        uint16_t outenable1 : 1;
        uint16_t outstyle1 : 1;
    } outconf;

} OUTCOFPARA;

typedef struct
{
    uint8_t  intype;
    uint8_t  instate;
    uint16_t invalue;
} INPUTINFO;

typedef union
{
    struct
    {
        uint16_t freq12 : 12;
        uint16_t value2 : 2;
        uint16_t state1 : 1;
        uint16_t type1 : 1;
    } iostate;
    uint16_t iovalue;
} IOPUTDATA;

#define HEART_MAX 16

typedef enum
{
    CS_CONFIGED = 0,
    CS_CONFIGING,
} CSCONFIGSTATE;

class CAN_DEV_APP_INFO
{
public:
    uint16_t  dev_off;
    uint8_t   zjnum;            // 设备中继级数
    uint8_t   csnum;            // 设备CS通道
    uint16_t  slaveio_order;    // 下位机序号
    uint16_t  order_in_cfg;     // 在配置文件中的位置
    uint8_t   devenable;
    uint8_t   errcount;
    uint8_t   devstate;
    uint8_t   mac_state;     // mac查询应答状态
    uint8_t   conf_state;    // 设备的配置状态
    uint8_t   heart_ok;      // 心跳ok标志
    uint8_t   csmac[6];      // 设备的mac
    uint16_t  cscrc;         // crc校验
    uint16_t  configsize;
    uint16_t  recv_reg_begin;
    uint16_t  recv_reg_offset;
    uint16_t  recv_reg_data[4];
    uint16_t *config_p;
    uint16_t *iodata;
    //    uint8_t           config_type;      //the type in 236.xml, if not config this value is 0xFF
    Max_State_Pro      *stateinfo;
    MsgMng             *msgmng_p;
    Pt_Devs_ShareData  *devdata_p;
public:
    CAN_DEV_APP_INFO()
    {
        memset(this, 0x00, sizeof(CAN_DEV_APP_INFO));
    }
    ~CAN_DEV_APP_INFO()
    {
        ;
    }
};

class CAN_DEV_APP:public CAN_DEV_APP_INFO
{
public:

    bitset< HEART_MAX > framark;    // 接收帧完整性验证
    can_dev_para        para;

public:
    CAN_DEV_APP()
    {
        framark.reset();
        msgmng_p  = MsgMng::GetMsgMng();
        para.id = 0;          //设备从机号
        para.innum = 0;       //输入个数
        para.outnum = 0;      //输出个数
        para.polltime = 0;    //查询周期,单位(ms)
        para.type = 0;
        para.enable = 0;
        para.link_num = 0;
        para.auto_reset = 0;
    }
    ~CAN_DEV_APP()
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

    int set_config_head(void)
    {
        configsize = para.innum * PT_INPUTPARA_MAX + CONF_TAIL + para.outnum * PT_OUTPUTPARA_MAX + CONF_HEAD;
        config_p   = new uint16_t[configsize];
        iodata     = new uint16_t[para.innum * CS_INDATA_SIZE + para.outnum * CS_OUTDATA_SIZE];

        if(config_p == NULL || iodata == NULL)
        {
            zprintf3("creat config info fail\n");
            return -1;
        }
        memset(config_p, 0x00, configsize * 2);
        config_p[0] = para.outnum | (para.innum << 8);
        config_p[1] = 0x7;
        config_p[2] = configsize - CONF_HEAD - CONF_TAIL;
        config_p[3] = 0;    // 存放crc的地方
        return 0;
    }

    int set_cs_config_head(void)
    {
        configsize = CS_CONFIG_BUF + CONF_HEAD;
        config_p   = new uint16_t[configsize];

        if(config_p == NULL)
        {
            zprintf3("creat config info fail\n");
            return -1;
        }
        memset(config_p, 0x00, configsize * 2);
        config_p[0] = 0;
        config_p[1] = 0x7;
        config_p[2] = 0X01;
        config_p[3] = 0;    // 存放crc的地方
        return 0;
    }

    int get_innode_tyle(int node)    // 点从1开始排序
    {
        INCOFPARA1 inparam;
        inparam.paraval = config_p[CONF_HEAD + (node - 1) * PT_INPUTPARA_MAX];
        return inparam.inconf.instyle1;
    }

    int get_outnode_tyle(int node)    // 点从1开始排序
    {
        OUTCOFPARA outparam;
        outparam.oparaval = config_p[CONF_HEAD + para.innum * PT_INPUTPARA_MAX + (node - 1) * PT_OUTPUTPARA_MAX];
        return outparam.outconf.outstyle1;
    }

    uint16_t get_input_data(int node)    // 点从1开始排序
    {
        if(get_innode_tyle(node) == 0)    // 开关量
        {
            IOPUTDATA ioputdata;
            ioputdata.iovalue = iodata[(node - 1) * CS_INDATA_SIZE];
            //            printf("get_input_data(i) = %x %x %x %x %x\n",  ioputdata.iovalue ,ioputdata.iostate.type1
            //            ,ioputdata.iostate.state1 ,ioputdata.iostate.value2, ioputdata.iostate.freq12);

            return ioputdata.iostate.value2;
        }
        else    // 频率量
        {
            IOPUTDATA ioputdata;
            ioputdata.iovalue = iodata[(node - 1) * CS_INDATA_SIZE];
            if(ioputdata.iovalue & 0x2000)    // 是否需要扩大20倍
            {
                return ioputdata.iostate.freq12 & 0x7ff;
            }
            else
            {
                return ioputdata.iostate.freq12 * 20;
            }
        }
    }

    uint16_t get_output_data(int node)    // 点从1开始排序
    {
        if(get_outnode_tyle(node) == 0)
        {
            IOPUTDATA ioputdata;
            ioputdata.iovalue = iodata[para.innum * CS_INDATA_SIZE + (node - 1) * CS_OUTDATA_SIZE];
            //            printf("get_output_data(i) = %x %x %x %x %x\n",  ioputdata.iovalue ,ioputdata.iostate.type1
            //            ,ioputdata.iostate.state1 ,ioputdata.iostate.value2, ioputdata.iostate.freq12);
            return ioputdata.iostate.value2 & 0x01;
        }
        else
        {
            IOPUTDATA ioputdata;
            ioputdata.iovalue = iodata[para.innum * CS_INDATA_SIZE + (node - 1) * CS_OUTDATA_SIZE];
            return ioputdata.iostate.freq12;
        }
    }

    int  creat_config_info(CAN_DEV_INFO &info);
    int  set_default_config(uint8_t type);
    int  reset_default_config(uint8_t type);
    void reset_dev_data(void);
    int  set_share_data(void);
    void dev_send_meg(uint8_t megtype, uint8_t *data, uint16_t size);
    int  dev_normal_process(void);
};

#define PUMP_MAX_NUM 10
#define DEV_MAX_NUM  255
#define BS_MAX_NUM   256 / 8

struct sPara_Config
{
    uint8_t Enable;
    uint8_t Slave_IO_Set;
    uint8_t Auto_Reset;
};
struct sLine_State
{
    uint16_t lineState;
    uint16_t Slave_IO_Exist;
    uint16_t Dev_Exist;
    uint16_t BS_Num_Exist;
    uint16_t BS_State;
    uint16_t BS_Location;
    uint16_t break_location;
    uint16_t Tail_Location;
    uint16_t ZD_Volte;
    uint16_t CS_Volte[2];
    uint16_t CS_Current[2];
    uint16_t ptcan_v;
    //    uint16_t reset_reason;
    uint32_t misc_yuliu_1;
    uint32_t misc_yuliu_2;
    uint32_t misc_yuliu_3;
    uint32_t misc_yuliu_4;
    uint8_t BS_Buttion[BS_MAX_NUM];
};
struct sDevc_State
{
    uint16_t Dev_ID;
    uint8_t  Dev_Type;
    uint8_t  Dev_State;
    uint16_t boot_v;
    uint16_t app_v;
    uint8_t  value_v;
    uint8_t  value_c;
    uint32_t misc_1;
    uint32_t misc_yuliu_1;
    uint32_t misc_yuliu_2;
    uint32_t misc_yuliu_3;
    uint32_t misc_yuliu_4;
};

struct Max_State_Data
{
    sLine_State line_state;
    sDevc_State devc_state[DEV_MAX_NUM];
};

#define MAX_STATE_SIZE (sizeof(Max_State_Data))
enum
{
    MAX_STATE_LINEID = 1,
    MAX_STATE_DEV,
};

class Max_State_Pro : public Dev_Map_T< char >
{
public:
    int                             cs_have;
    uint8_t                         tatol_dev;
    QT_Share_MemT< Max_State_Data > share_state;

public:
    Max_State_Pro()
    {
        cs_have   = 0;
        tatol_dev = 0;
    }
    ~Max_State_Pro()
    {
        zprintf3("destory Max_State_Pro!\n");
    }

    int  max_state_pro_init(QString key, int branch_num);
    void set_cs_have(int val)
    {
        cs_have = val;
    }

    void          set_dev_enable_state(int id, uint8_t val);
    void          set_dev_status(int id, uint8_t val);
    void          set_dev_state_AND(int id, uint8_t val);
    void          set_dev_state_OR(int id, uint8_t val);
    void          set_ptcan_version(uint8_t branch);
    void          set_voip_state(uint8_t branch, uint8_t state);
    void          set_cs_av_state(uint8_t branch, uint8_t v1, uint16_t c1, uint8_t v2, uint16_t c2);
    void          set_line_18v_state(uint8_t branch, uint8_t state, uint8_t lineNum);
    void          set_line_work_state(uint8_t branch, uint8_t state);
    void          set_bs_type(uint8_t branch, uint8_t type);
    void          set_bs_location(uint8_t branch, uint8_t location);
    void          set_break_location(uint8_t branch, uint8_t location);
    uint8_t       get_bs_location(uint8_t branch);
    uint8_t       get_break_location(uint8_t branch);
    uint8_t       get_bs_type(uint8_t branch);
    void          set_all_dev_state(uint8_t val);
    void          set_all_state_clear(void);
    void          set_termal_vol(uint8_t branch, uint8_t val);
    void          set_tail_location(uint8_t branch, uint8_t location);
    uint8_t       get_tail_location(uint8_t branch);
    void          set_bs_state(uint8_t branch, uint8_t addr, uint8_t state);
    bool          set_slaveio_num(uint8_t branch, uint8_t num);
    bool          set_dev_num(uint8_t branch, uint8_t num);
    bool          set_bs_num(uint8_t branch, uint8_t num);
    void          set_dev_type(uint8_t branch, uint8_t num, uint8_t val);
    void          set_dev_ID(uint8_t branch, uint8_t num, uint8_t val);
    uint8_t       get_dev_state(int id);
    uint8_t       get_dev_state(int branch, int id);
    void          set_dev_misc1_state(uint8_t branch, uint8_t num, uint8_t io_num, uint8_t value);
    void          set_dev_cv_state(uint8_t branch, uint8_t num, uint8_t v, uint8_t c);
    void          set_dev_version_state(uint8_t branch, uint8_t num, uint16_t boot_v, uint16_t app_v);
    uint8_t       get_dev_num(int branch);
    uint8_t       get_dev_type(uint8_t branch, uint8_t num);
    uint8_t       get_bs_is_have(uint8_t branch);
    PT_Dev_State *get_dev_info(uint id)
    {
        return ((PT_Dev_State *)get_dev_addr(id));
    }
};

extern const IONUM comdevio[CS_DEVSTY_MAX];
#endif    // __1030COMMON_H__

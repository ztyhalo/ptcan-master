#ifndef __256PRO_H__
#define __256PRO_H__

#include "can_bus.h"
#include "can_protocol.h"
#include "256common.h"
#include <stdint.h>
#include "zmap.h"
#include "tkcommon.h"
#include "bsdev.h"



class CSZD_IS_HAVE
{
  public:
    uint8_t mac_cs_have;
    uint8_t mac_cs_have_tmp;
    uint8_t mac_terminal_have;
    void    clear(void)
    {
        mac_cs_have       = 0;
        mac_terminal_have = 0;
    }
};

typedef Zt_Map< uint8_t, CAN_256DEV_APP >  N_ConfigMap;
typedef Zt_Map< uint8_t, CSZD_IS_HAVE > N_CSZDMap;
typedef Zt_Map< uint8_t, can_dev_para > N_Dev256Map;



struct break_msg
{
    uint8_t direction;
    uint8_t location;
};
class cs_can256_info
{
public:
    enum
    {
        CMD_SEND_BOOTENTER  = 0,
        CMD_SEND_MASTERREST = 2,
        CMD_SEND_MACCHECK,
        CMD_SEND_PARACORRECT,
        CMD_SEND_CFGPARAM,
        CMD_SEND_FOUTPUT = 10,
        CMD_SEND_SWOUTPUT,
        CMD_SEND_STOPREPORT_ACK,
        CMD_SEND_ERRREPORT_ACK,
        CMD_SEND_AUTOREPORT_ACK,
        CMD_SEND_BROKENTAIL    = 18,
        CMD_SEND_LOCK_LOCATION = 19,

        CMD_SEND_READFUNC_HIGH = 20,
        CMD_SEND_WRITEFUNC_HIGH,
        CMD_SEND_READFUNC_LOW,
        CMD_SEND_WRITEFUNC_LOW,

        CMD_SEND_BREAK_CHECK = 24,
        CMD_SEND_HEARTCHECK  = 30,
        CMD_SEND_A_V_REPORT  = 31,
    };

    enum
    {
        CMD_REV_MACCHECK_ACK    = 3,
        CMD_REV_CFGPARAMCORRECT = 5,
        CMD_REV_SLAVERESET,
        CMD_REV_FOUTPUT_ACK = 10,
        CMD_REV_SWOUTPUT_ACK,
        CMD_REV_STOP_REPORT,
        CMD_REV_ERROR_REPORT,
        CMD_REV_AUTO_REPORT,
        CMD_REV_BROKENTAIL_ACK = 18,
        CMD_REV_LOCK_LOCATION_ACK,

        CMD_REV_READFUNC_HIGH_ACK,
        CMD_REV_WRITEFUNC_HIGH_ACK,
        CMD_REV_READFUNC_LOW_ACK,
        CMD_REV_WRITEFUNC_LOW_ACK,

        CMD_REV_WORK_DEV_CHECK = 24,
        CMD_REV_HEARTCHECK_ACK = 30,
    };

    typedef union
    {
        struct
        {
            unsigned int ttl_7 : 7;
            unsigned int devaddr_8 : 8;
            unsigned int csaddr_2 : 2;
            unsigned int zjaddr_3 : 3;
            unsigned int dir_1 : 1;
            unsigned int ack_1 : 1;
            unsigned int next_1 : 1;
            unsigned int func_5 : 5;
            unsigned int new_1 : 1;
        } canframework;
        unsigned int canframeid;
    } CANFRAMEID;

    enum
    {
        DEV_NO_RESET = 0,
        DEV_RESET_ING,
        DEV_RESET_CONFIG,
        DEV_RESET_OVER,
    };

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

public:
    CSCANSTATE csstate;               // 1030协议初始化时的状态变迁
    uint8_t    csid;                  // cs id 0,1,2
    uint8_t    csdevtyle;             // cs dev style 0:100+cs 1:io
    uint8_t    devonnum;              //该can总线下的设备数量
    uint8_t    cf_devnum;             //配置的设备个数()
    uint8_t    cf_file_num;           //配置文件配置的设备个数
    uint8_t    cf_cs_num[BRANCH_ALL]; // 0:本沿线 1：本沿线中继后的沿线 2：CS中继后的LS
    uint16_t   cf_endnum;             //重配的结束设备编号
    uint8_t    line_num;
    uint8_t    cs_jt_status;
    uint8_t    csmacstate;                          // mac查询状态
    uint8_t    csmacorder;                          //沿线总设备数
    uint8_t    csioorder;                           // mac查询应答顺序编号
    uint8_t    cszjorder[BRANCH_ALL];               // 0:本沿线 1：本沿线中继后的沿线 2：CS中继后的LS
    uint8_t    slave_io_num[BRANCH_ALL];            // mac查询应答顺序编号
    uint8_t    init_over;                           //初始化进程结束
    uint8_t    mac_cs_num;                          // mac查询cs存在
    uint8_t    mac_terminal_num;                    // mac查询终端存在
    uint8_t    csmacorder_temp;                     // mac查询应答顺序编号
    uint8_t    csioorder_temp;                      // mac查询应答顺序编号
    uint8_t    cszjorder_temp[BRANCH_ALL];          // mac查询应答顺序编号
    uint8_t    slave_io_num_temp[BRANCH_ALL];       // mac查询应答顺序编号
    uint8_t    mac_cs_num_temp;                     // mac查询cs存在
    uint8_t    break_line_pretreatment[BRANCH_ALL]; //有闭锁按下时，断线预处理。
    uint8_t    heartcout;                           //心跳发送顺序计数
    uint8_t    reset_state;                         //复位状态
    CSCONFERR  csconfstate;                         //配置状态
    int        auto_reset;
    pthread_t  initproid;    // 1030协议初始化线程id号
    pthread_t  reset_id;     //复位id
    pthread_t  thread_shake; //复位id
    pthread_t  heartManageId;   //心跳管理线程id
    int        polltimer_id; //论询定时器id
    uint8_t    reset_msg[4];
    //    uint8_t            low_num[2];
    uint8_t             cut_check_flag;
    uint8_t             break_send_status;
    uint8_t             break_location;
    uint8_t             break_location_temp;
    uint8_t             bs_button_report[4];
    uint8_t             bs_type_r     = 0xff;
    uint8_t             bs_location_r = 0xff;
    uint8_t             heart_check_last_id;
    uint8_t             cut_location_shake[3];
    uint8_t             heart_cs_error_count_g;
    uint8_t             heart_print_mark; //心跳打印标记


    ncan_protocol*      pro_p; //使用的can协议指针

    TK_IO_Dev*          tk100io; // tk100的io模块
    Pt_Devs_ShareData*  data_p;
    REQDEVMK            csreqmark;
    CANDATAFORM         pollFrame; // 1030 poll frame;
public:
    cs_can256_info()
    {
        memset(this, 0, sizeof(*this));
    }
    ~cs_can256_info()
    {
        zprintf3("cs_can_info destruct!\n");
    }
};

class cs_can256:public cs_can256_info
{
  public:

    BS_Dev              bs100info;
    N_ConfigMap         nconfig_map;
    bitset< HEART_MAX > framark[255];
    N_CSZDMap   mac_cszd_have;
    N_Dev256Map    ndev_map;

    sem_t         statechg; // can状态改变信号
    sem_t         reset_sem;
    Max_256State_Pro state_info;


  public:
    cs_can256(ncan_protocol* pro, const QString & key, int branch_num, int reset_enable);

    int  add_default_dev(int zjnum, int csnum, int devid, int io_num, uint16_t type);
    int  get_dev_id(int order);
    bool is_have_dev(int zjnum, int csnum, int devid, int& order);
    bool is_have_config_slaveio(int zjnum, int csnum, int slaveio_order, int& order);

    bool is_have_mac_dev(int paraid, int& order);
    bool is_have_mac_dev(int zjnum, int csnum, int devid, int& order);
    bool is_have_config_dev(int devid, int& order);
    bool is_have_check_dev(int devid, int& order);
    bool is_tail(int devid);
    bool is_tail(int zjnum, int csnum, int devid);
    int  cs_config_init(void);
    int  cs_init(void);
    //    int cs_reset_config(int csnum, int canid, uint protyle);
    void         max_reset_data(void);
    void         set_dev_enable(void);
    void         send_configdata(void);
    int          pt_configdata_set(CAN_DEV_INFO& dev, uint8_t cs_num, uint8_t dev_num);
    int          pt_configdata_set(CAN_DEV_INFO& dev, uint16_t dev_off);
    static void* cs_protocol_init(void* para); // cs初始化函数
    void         send_data_ttlproc(CANFRAMEID& sendframe, CANDATAFORM& senddata, uint8_t* data, uint size);
    void         cs_send_data(uint func, int zjnum, int csnum, int dest, uint8_t* data, uint size);
    void         add_poll_frame(void);
    int          get_dev_off(int devid);
    void         delete_1030_dev_timer(void)
    {
        if (polltimer_id != 0 && pro_p != NULL)
        {
            pro_p->delete_poll_timer(polltimer_id);
            polltimer_id = 0;
        }
    }
    int      cs_can_reset_sem(void);
    int      cs_can_reset_pro(void);
    uint8_t  get_config_low_num(void);
    uint8_t  get_mac_low_num(void);
    uint8_t  get_bs_mac_num(uint8_t branch);
    uint16_t get_dev_type(uint8_t devid);
    void     set_dev_status(uint8_t devid, uint8_t state);
    void     set_dev_state(uint8_t devid, uint8_t state);
    void     set_dev_map_state(uint8_t devid, uint8_t state);

    void set_devonline_num(uint8_t val);
    void reset_all_dev(void);

    ~cs_can256();
    void           add_mac_cszd_branch(int zjnum, int csnum);
    void           set_mac_cs_have(int zjnum, int csnum);
    void           change_report_bs_location(uint8_t branch, uint8_t bs_location);
    void           bs_type_location_report(uint8_t bs_type);
    void           bs_type_location_report(uint8_t bs_type, uint8_t bs_location);
    void           bs_type_location_report(uint8_t branch, uint8_t bs_type, uint8_t bs_location);
    CS_BRANCH_TYPE get_branch(CANFRAMEID rxmidframe);

  private:
};
int dev256_output(void* midp, soutDataUnit val);
#endif // __256PRO_H__

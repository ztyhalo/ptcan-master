#ifndef __1030PRO_H__
#define __1030PRO_H__

#include "can_bus.h"
#include "can_protocol.h"
#include "1030common.h"
#include <stdint.h>
#include "zmap.h"
#include "tkcommon.h"
#include "bsdev.h"



#define MAX_STATE_BUF_SIZE       (sizeof(Max_State_Data))

enum{
    BS_NUM_D = 0,
    ZD_STATE,
    JT_STATE,
    POWER_STATE,
    ZS_STATE,
    PP_STATE,
    ZS_NUM,
    CS_STATE_MAX
};
typedef enum
{
    CS_ONLINE_FUNC = 0x01,
    CS_OUT_ASK_FUNC,
    CS_ERR_MSG_FUNC,
    CS_POLL_STATE_FUNC,
    CS_LOCK_STATE_FUNC,
    CS_AUTO_FUNC,
    CS_STOP_STATE_FUNC,
}CSFUNCENUM;



enum
{
    CMD_SEND_BOOTENTER = 0,
    CMD_SEND_MASTERREST = 2,
    CMD_SEND_MACCHECK,
    CMD_SEND_PARACORRECT,
    CMD_SEND_CFGPARAM,
    CMD_SEND_FOUTPUT = 10,
    CMD_SEND_SWOUTPUT,
    CMD_SEND_STOPREPORT_ACK,
    CMD_SEND_ERRREPORT_ACK,
    CMD_SEND_AUTOREPORT_ACK,
    CMD_SEND_TERMINALCHECK = 16,
    CMD_SEND_KEY_ACK = 18,

    CMD_SEND_READFUNC_HIGH = 20,
    CMD_SEND_WRITEFUNC_HIGH,
    CMD_SEND_READFUNC_LOW,
    CMD_SEND_WRITEFUNC_LOW,

    CMD_SEND_HEARTCHECK = 30,


};

enum
{
    CMD_REV_MACCHECK_ACK = 3,
    CMD_REV_CFGPARAMCORRECT =5,
    CMD_REV_SLAVERESET,
    CMD_REV_FOUTPUT_ACK =10,
    CMD_REV_SWOUTPUT_ACK,
    CMD_REV_STOP_REPORT,
    CMD_REV_ERROR_REPORT,
    CMD_REV_AUTO_REPORT,
    CMD_REV_TREMINALCHECK_ACK =16,

    CMD_REV_READFUNC_HIGH_ACK = 20,
    CMD_REV_WRITEFUNC_HIGH_ACK,
    CMD_REV_READFUNC_LOW_ACK,
    CMD_REV_WRITEFUNC_LOW_ACK,

    CMD_REV_HEARTCHECK_ACK = 30,

};
typedef union
{
    struct{
            unsigned int ttl_7:7;
            unsigned int dest_7:7;
            unsigned int souc_7:7;
            unsigned int ack_1:1;
            unsigned int next_1:1;
            unsigned int func_6:6;
         }canframework;
    unsigned int canframeid;
}CANFRAMEID;


















typedef Zt_Map<uint8_t, CAN_DEV_APP> N_DevMap;

enum {
    DEV_NO_RESET = 0,
    DEV_RESET_ING,
    DEV_RESET_OVER
};
class cs_can_info
{
public:
    typedef enum
    {
        NO_INSULATE_SWITCH = 0x01,
        INSULATE_SWITCH,
        CS_DEV,
        LOW_MACH,
        TERMINAL,
        OPERATER,
        IN_DEV,
        OUT_DEV,
        TK100_CSModule = 0x09,
        TK100_IOModule_Salve,
        TK100_BS_Module,
        TK100_IOModule_IO,
        TK200_CSModule,
        TK200_LOWModule,
        TK200_IOModule,
        TK200_INModule,
        CS_DEVSTY_MAX
    }CSDEVSTYLE;
public:
    CSCANSTATE          csstate;        //1030协议初始化时的状态变迁

    uint8_t             csid;           //cs id 0,1,2
    uint8_t             csdevtyle;      //cs dev style 0:100+cs 1:io
    uint8_t             devonnum;         //该can总线下的设备数量
    uint8_t             cf_devnum;      //配置的设备个数
    uint8_t             cf_file_num;    //配置文件配置的设备个数
    uint8_t             cf_endnum;      //重配的结束设备编号
    uint8_t             en_devmax;      //使能的最大设备号
    uint8_t             csmacstate;     //mac查询状态
    uint8_t             csmacorder;     //mac查询应答顺序编号
    uint8_t             init_over;      //初始化进程结束
    uint8_t             mac_cs_have;    //mac查询cs存在
    uint8_t             mac_terminal_have; //mac查询终端存在
    //    uint8_t             terminal_num; //mac查询终端存在

    uint8_t             heartcout;      //心跳发送顺序计数
    uint8_t             reset_state;    //复位状态
    CSCONFERR           csconfstate;    //配置状态
    uint                csstopnum;      //闭锁数量
    int                 auto_reset;
    pthread_t           initproid;      //1030协议初始化线程id号
    pthread_t           reset_id;       //复位id
    pthread_t           heartManageId;  //心跳管理线程id
    int                 polltimer_id;        //论询定时器id

    ncan_protocol  *    pro_p;          //使用的can协议指针


    TK_IO_Dev     *     tk100io;         //tk100的io模块
    Pt_Devs_ShareData  *     data_p;
    REQDEVMK            csreqmark;
    CANDATAFORM         pollFrame;         //1030 poll frame;
public:
    cs_can_info()
    {
        memset(this, 0, sizeof(*this));
    }
    ~cs_can_info()
    {
        zprintf3("cs_can_info destruct!\n");
    }
};

class cs_can:public cs_can_info
{
public:
    BS_Dev              bs100info;
    N_DevMap            ndev_map;
    sem_t               statechg;           //can状态改变信号
    sem_t               reset_sem;
    Max_State_Pro       state_info;


public:
    cs_can(ncan_protocol * pro, const QString  key, int reset_enable);

    int add_default_dev(int devid, uint16_t type);
    int get_dev_id(int order);
    bool is_have_dev(int devid, int & order);
    bool is_have_mac_dev(int devid, int & order);
    int cs_config_init(void);
    int cs_init(void);
//    int cs_reset_config(int csnum, int canid, uint protyle);
    void max_reset_data(void);
    void send_configdata(void);
    int pt_configdata_set(CAN_DEV_INFO & dev, uint8_t dev_off);
    static void * cs_protocol_init(void * para);           //cs初始化函数
    void send_data_ttlproc(CANFRAMEID & sendframe,CANDATAFORM & senddata, uint8_t *data, uint size);
    void cs_send_data(uint func, int dest, uint8_t * data, uint size);
    void add_poll_frame(void);
    int get_dev_off(int devid);
    void delete_1030_dev_timer(void){
        if(polltimer_id != 0 && pro_p != NULL){

            pro_p->delete_poll_timer(polltimer_id);
            polltimer_id = 0;

        }
    }
    int cs_can_reset_sem(void);
    int cs_can_reset_pro(void);
    uint8_t get_config_low_num(void);
    uint8_t get_mac_low_num(void);
    uint16_t get_dev_type(uint8_t devid);
    void set_dev_state(uint8_t devid, uint8_t state);
    void set_devonline_num(uint8_t val);
    void reset_all_dev(void);

    ~cs_can();
};
int dev1030_output(void * midp, soutDataUnit val);
#endif // __1030PRO_H__

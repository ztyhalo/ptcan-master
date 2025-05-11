#ifndef PTDEVCOMMON_H
#define PTDEVCOMMON_H

#include <stdint.h>
#include <sys/types.h>
#include <bitset>
// #include "candatainfo.h"
// #include "tkcommon.h"
// #include "zprint.h"

using namespace std;
#define PTCAN_VERSION_H 1
#define PTCAN_VERSION_M 7
#define PTCAN_VERSION_L 2

#define PT_INPUTPARA_MAX  5
#define PT_OUTPUTPARA_MAX 1

#define CS_MACHINE_NUM 3
#define IO_MACHINE_NUM 1

#define CONF_HEAD 4
#define CONF_TAIL 1

#define CSDEV_INNODE_NUM  16
#define CSDEV_OUTNODE_NUM 16

#define CONFIG_BUF_MAX  (CONF_HEAD+(CSDEV_INNODE_NUM*PT_INPUTPARA_MAX+CSDEV_OUTNODE_NUM*PT_OUTPUTPARA_MAX)+CONF_TAIL)

#define INNODEINFO(x) ((x)*PT_INPUTPARA_MAX + CONF_HEAD)

enum
{
    RESET_SUCCESS = 0,
    RESET_FAIL
};
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
    } out256conf;
    struct
    {
        uint16_t initval:14;
        uint16_t outenable1:1;
        uint16_t outstyle1:1;
    }outconf;

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

struct sPara_Config
{
    uint8_t Enable;
    uint8_t Slave_IO_Set;
    uint8_t Auto_Reset;
};

#define PUMP_MAX_NUM 10
#define DEV_MAX_NUM  255
#define BS_MAX_NUM   256 / 8

struct sLine_State
{
    uint16_t CS_State;
    uint16_t Slave_IO_Exist;
    uint16_t Dev_Exist;
    uint16_t BS_Num_Exist;
    uint16_t BS_State;
    uint16_t BS_Location;
    uint16_t break_Location;
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

struct sLine256_State
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

struct Max_256State_Data
{
    sLine256_State line_state;
    sDevc_State devc_state[DEV_MAX_NUM];
};

#define MAX_STATE_SIZE (sizeof(Max_State_Data))
#define MAX_256STATE_SIZE (sizeof(Max_256State_Data))
enum
{
    MAX_STATE_LINEID = 1,
    MAX_STATE_DEV,
};


enum
{
    CS_WORK_STATUS_LIVEOUT = 0,
    CS_WORK_STATUS_RIGHT,
    CS_WORK_STATUS_NO_CONFIG,
    CS_WORK_STATUS_CONFIGING,
};

typedef struct
{
    uint8_t innum;
    uint8_t outnum;
} IONUM;


typedef enum
{
    CS_CONFIGED = 0,
    CS_CONFIGING,
} CSCONFIGSTATE;



typedef enum
{
    DEVS_OFFLINE = 0x00,
    DEVS_ONLINE  = 0x01,
} DEVSMARK;

typedef union
{
    unsigned char COM;
    struct
    {
        unsigned char enbale : 4;
        unsigned char type : 4; // 点类型:0=开关量; 1=模拟量
    } Bits;
} NODEINFORM;

typedef struct
{
    int        DeviceType; // 0: 无设备 1: 闭锁 2:下位机 3:IO
    NODEINFORM InPort[CSDEV_INNODE_NUM];
    NODEINFORM OutPort[CSDEV_OUTNODE_NUM];
    char       InName[16][100];
    char       OutName[16][100];
} CSDEVINFO;


typedef struct
{
    OUTCOFPARA outattr;
    uint16_t   value;
} OUTDATAAT;

typedef struct
{
    INCOFPARA1 inattr;
    uint16_t   invalue;
} INDATAAT;

typedef union
{
    struct
    {
        uint8_t innum5 : 5;
        uint8_t instate2 : 2;
        uint8_t instyle1 : 1;
    } rxinattr;
    uint8_t rxinval;
} RXINDATA;


typedef struct
{
    uint16_t csnum;
    uint16_t csmachtype;
    uint8_t  csmac[6];
    uint16_t cscrc;
} MACTABLE;

typedef enum
{
    CS_CAN_INIT = 0,
    CS_CAN_RESET,
    CS_CAN_MACREQ,
    CS_CAN_COFIG,
    CS_CAN_NORMAL,
    CS_CAN_ERR,
    CS_CAN_UPDATA
} CSCANSTATE;

typedef struct
{
    uint16_t devtype;
    uint16_t enablemk; // 0 disable; 1 enable
    uint8_t  devstate; // 0 :offline 1:online
    uint8_t  deverrsize;
} DEVSINFO;

enum
{
    MAC_NORMAL = 0,
    MAC_ORDER_ERR,
    MAC_TYPE_ERR,
    MAC_KERNEL_ERR
};

typedef enum
{
    CS_CONFIG_NO = 0,
    CS_CONFIG_YES,
} CSCONFIGSL;

typedef struct
{
    CSCONFIGSL    confsel;
    CSCONFIGSTATE confstate;
} CONFINFO;

typedef enum
{
    CSCONF_NORMAL = 0,
    CSCONF_MORE,
    CSCONF_LESS,
    CSCONF_TYPE_ERR,
    CSCONF_NEED,
} CSCONFERR;
typedef struct
{
    //    uint16_t    reqcsmark;
    uint16_t reqdevnum;
    uint16_t errnum;
    uint16_t csnumstate;
} REQDEVMK;
typedef struct
{
    int  csnum;
    int  canid;
    uint protyle;
    uint configtype;
} CSINITPARA;

typedef struct
{
    uint8_t bsnum;
    uint8_t zdstate;
    uint8_t jtstate;
    uint8_t powerstate;
    uint8_t zsstate;
    uint8_t ppstate;
    uint8_t zsnum;
} TK100CSINFO;

/***********新增心跳接收处理结构体*************/
#define HEART_MAX 16
typedef struct
{
    uint8_t             rxstate;
    uint8_t             framnum;
    bitset< HEART_MAX > framark;
} HEARTRXINF;

// class PtDevCommon
// {
// public:
//     PtDevCommon();
// };

#endif // PTDEVCOMMON_H

#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H
#include <map>
#include "can_bus.h"
#include "timers.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "clist.h"
#include <bitset>

#define CAN_PROP_TIMEINTV 10
#define TIME_INTERVAL     2
#define FRAME_P_MAX       32 //最大的帧个数


using namespace std;
typedef unsigned int FrameId;

typedef void* (*Pro_Init_Process)(void * para);
typedef FrameId (*Get_Frame_Mapid)(FrameId paraid);


typedef struct
{
    unsigned int    frametype;                         //帧类型
    unsigned int    frameid;                           //映射号
    int             overtime;
    int             repeatnum;
    unsigned int    ackframe;
    void         *  father;
    int (*cancallbacfun)(void * para, CANDATAFORM data); //接收回调函数
    int (*overcallbacfun)(void *para,  CANDATAFORM data);                   //超时回调函数
    int (*sendiffunc)(void *);                                //发送条件回调函数

}CANPROHEAD;

template <class T>
struct CANPROTEM
{
    unsigned int    frametype;                         //帧类型
    unsigned int    frameid;                           //映射号
    int             overtime;
    int             repeatnum;
    unsigned int    ackframe;
typedef int(* cancb_p)(T * para, CANDATAFORM data);
    cancb_p cancallbacfun; //接收回调函数
typedef int(* overcb_p)(T * para);
    overcb_p overcallbacfun;                   //超时回调函数
    int (*sendiffunc)();                       //发送条件回调函数
};

typedef struct
{
    CANPROHEAD *    initdata;                        //初始化数据指针
    unsigned int    datasize;                        //初始化数据大小
    char *          initpara;                        //初始化函数参数指针
    unsigned int    initparasize;                    //初始化函数参数大小
    Pro_Init_Process   initfunc;                        //初始化功能函数
    Get_Frame_Mapid mapid;                           //映射id号的获得函数
}PROININSTR;

typedef map<FrameId, CANPROHEAD> FrameMap;

typedef unsigned int ProtocolType;                  //两种协议类型 标准帧和扩展帧
typedef struct
{
    pthread_t       proinitp;
    PROININSTR      initinfo;                        //初始化信息
}INITPARA;

typedef map<ProtocolType, INITPARA> ProInitMap;

typedef unsigned int CanIdInt;
typedef unsigned int ExitPara;

typedef map<CanIdInt, ExitPara>  CanExitMap;
typedef struct
{
    CANPROHEAD          canhead;
    CANDATAFORM         candata;
}CANPROFRAME;
typedef struct can_pro_data
{
    CANPROHEAD          canhead;
    CANDATAFORM         candata;
    int sendnum;
    int runtime;
}CANPRODATA;

//declare a single can of class
#define MAX_PROTOCOL 2     //一条can线上最多能注册的协议个数 标准帧和扩展帧


class ncan_protocol;


#define CANp_TIMER   B_Timer(ncan_protocol)
#define CANp_TIME_ET TimerEvent<ncan_protocol,void>
#define CAN_DATA_LIST List_N<CANPRODATA>

#define CANp_List    C_LIST_T<CANPRODATA,CANBUS_TX_QSIZE>

typedef int ( * Func_Poll)(CANp_TIME_ET * poll);

class ncan_protocol
{
public:
    bitset<8>                       type;     //type std ext
    string                          name;
    int                             canid;
    int                             inter;
    CanDriver     *                 candrip;
    int                             proid;
    int                             errsize;

    FrameMap                        canidmap[2];
    CANp_TIMER                      protm;       //can协议定时器
    CANp_List                       prolist;
    CANPRODATA                      errbuf[CANBUS_TX_QSIZE];
    Z_Buf_T<CANDATAFORM, 8> pollfram;
//    CANDATAFORM         pollfram[2];
    Get_Frame_Mapid     idfunc[2];

public:
    ncan_protocol(const string & n="",int can=0, int pro=0):name(n),canid(can),inter(0),candrip(NULL),
        proid(pro),errsize(0)
    {
        type.reset();
//        memset(pollfram, 0x00, sizeof(pollfram));
        memset(idfunc, 0x00, sizeof(idfunc));
        memset(errbuf, 0x00, sizeof(errbuf));

    }

    ~ncan_protocol(){
        zprintf3("destory ncan_protocol!\n");
    }
    int ncan_pro_init(CanDriver * dri);
    int get_frame_mapidf(ProtocolType protype, FrameId & fid);
    int add_protocol_frame(unsigned int id, CANPROHEAD info);
    int get_protocol_frameinfo(unsigned int id, unsigned int type, CANPROHEAD & info);
    CANPROHEAD * get_protocol_frameinfo(unsigned int id, unsigned int type);
    int can_protocol_send(CANDATAFORM  canprop);
    int can_protocol_send(CANPROFRAME  canprop);
    int can_protocol_send_frame(CANDATAFORM  canprop);
    int init_pro_frame(CANPROHEAD * info, int size);

    int pro_del_buf_frame(unsigned int frameid,unsigned char frametype);
    int pro_del_buf_frame(CANDATAFORM & frame);
    int del_buf_frame(CANPRODATA * canprop);
    CANPRODATA * read_deldata_buf(unsigned int frameid,unsigned char frametype);
    int add_poll_frame(CANDATAFORM &frame, double inter);
    int delete_poll_timer(int id);
    void start(void);
    int add_poll_frame(double inter, Func_Poll callback, void * arg);
    int add_poll_frame(double inter, void * arg);

};

class CanPROMag
{
public:
   static CanPROMag * get_pro_manage(void)
   {
       return &canpropoint;
   }


private:
   CanPROMag()
   {
   }

    static CanPROMag  canpropoint;
};

#define  GETCANPROMAG(x) CanPROMag * (x) = CanPROMag::get_pro_manage()




extern CANDATAFORM   gBugCanInfo;
extern int           gBugState;

#endif // CAN_PROTOCOL_H

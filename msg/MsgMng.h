#ifndef MSGMNG_H
#define MSGMNG_H

#include "msg.h"
#include <semaphore.h>
#include <QList>
#include <QMap>
#include <QString>
#include "driver.h"
#include "semprocess.h"
#include "zprint.h"
#define WAIT_MSG_MAX   100


typedef struct
{
    Type_MsgAddr waitid;
    uint16_t            type;
    sem_t *            pack;
}sWaitMsg;
typedef QList<sWaitMsg> lWaitList;

class MsgMngBase :public SemRevClass<sMsgUnit>
{
public:
    Z_Msg<sMsgUnit> m_SendMsg;
public:
    MsgMngBase();
    virtual ~MsgMngBase();
};

class PtDriverBase
{
public:
    int             m_driverId;
    sParamInfoType  m_paramInfo;
public:
    PtDriverBase():m_driverId(0)
    {
        memset(&m_paramInfo, 0x00, sizeof(m_paramInfo));
    }
    virtual ~PtDriverBase()
    {
        zprintf3("PtDriverBase destruct!\n");
    }
    virtual int  get_innode_info(uint devnum, uint innode, pt_inode_info& val) =0;

};

class MsgMngDriver: public MsgMngBase
{
private:

    int testcycle ;

    MsgMngDriver();
public:
    Type_MsgAddr    soure_id;
    uint32_t        dest_id;
    // int             m_driverId;
    // sParamInfoType  m_paramInfo;
    PtDriverBase *  m_pDriver;


public:
    static MsgMngDriver * GetMsgMngDriver(void)
    {
        static MsgMngDriver gMsgMngDriver;
        return &gMsgMngDriver;
    }
    ~MsgMngDriver();
    bool Init(int recvkey,int sendkey, PtDriverBase * pdriver);
    void sem_rec_process(sMsgUnit val) override;
};



class MsgMng
{
private:
    pthread_t RecvMsg_id;
    msg  *pRecvMsg;
    msg  *pSendMsg;
    lWaitList  WaitDriverList;
    pthread_mutex_t WaitListMutex;
    int testcycle ;

    MsgMng();
    bool CheckWaitMsg( Type_MsgAddr waitid,uint16_t type);
    bool AckWaitMsg( Type_MsgAddr waitid,uint16_t type);

public:
    Type_MsgAddr soure_id;
    uint32_t     dest_id;
public:
    static MsgMng * GetMsgMng(void)
    {
        static MsgMng gMsgMng;
        return &gMsgMng;
    }
    ~MsgMng();
    bool Init(int recvkey,int sendkey, void * arg=NULL);
    bool InsertWaitMsg(const Type_MsgAddr &waitid,uint16_t type,sem_t * pack);
    void RecvMsgProcess(void * arg);
    void msgmng_send_msg(sMsgUnit *pdata, uint16_t size);
};

#endif // MSGMNG_H

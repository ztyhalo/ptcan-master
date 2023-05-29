#ifndef MSGMNG_H
#define MSGMNG_H

#include "msg.h"
#include <semaphore.h>
#include <QList>
#include <QMap>
#include <QString>
//#include "driver.h"


#define WAIT_MSG_MAX   100


typedef struct
{
    Type_MsgAddr waitid;
    uint16_t            type;
    sem_t *            pack;
}sWaitMsg;
typedef QList <sWaitMsg> lWaitList;


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
    bool InsertWaitMsg( Type_MsgAddr &waitid,uint16_t type,sem_t * pack);
    void RecvMsgProcess(void * arg);
    void msgmng_send_msg(sMsgUnit *pdata, uint16_t size);
};

#endif // MSGMNG_H

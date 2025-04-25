#include "MsgMng.h"
#include "candata.h"

MsgMng::MsgMng():pRecvMsg(NULL),pSendMsg(NULL),testcycle(0),dest_id(0)
{
    soure_id.app = 0;
    pthread_mutex_init(&WaitListMutex, NULL);
}

MsgMng::~MsgMng()
{

}

void * RecvMsg_task(void * arg)
{
    MsgMng *pMsgMng = MsgMng::GetMsgMng();

    while(1)
    {
        //TODO:
        pMsgMng->RecvMsgProcess(arg);
    }

    return NULL;
}

bool MsgMng::Init(int recvkey,int sendkey, void * arg)
{
    pRecvMsg = new msg(recvkey);
    pSendMsg = new msg(sendkey);
    WaitDriverList.clear();

    if(!pRecvMsg->create_object())
        return false;
    if(!pSendMsg->create_object())
        return false;

    pthread_create(&RecvMsg_id,NULL,RecvMsg_task, arg);
    return true;
}

bool MsgMng::InsertWaitMsg( const Type_MsgAddr &waitid,uint16_t type,sem_t * pack)
{
    sWaitMsg data;

    pthread_mutex_lock(&WaitListMutex);
    data.waitid = waitid;
    data.type = type;
    data.pack = pack;
    (void) data;
    if(WaitDriverList.size() <WAIT_MSG_MAX)
    {
        // WaitDriverList.append(data);
        // WaitDriverList.
        pthread_mutex_unlock(&WaitListMutex);
        return true;
    }
    pthread_mutex_unlock(&WaitListMutex);
    return false;
}

bool MsgMng::CheckWaitMsg( Type_MsgAddr waitid,uint16_t type)
{
    // lWaitList::iterator item;

    pthread_mutex_lock(&WaitListMutex);
    QList <sWaitMsg>::iterator item =  WaitDriverList.begin();
    while(item != WaitDriverList.end())
    {
        if(((*item).type == type)&&((*item).waitid.app == waitid.app))
        {
            pthread_mutex_unlock(&WaitListMutex);
            return true;
        }
        ++item;
    }
    pthread_mutex_unlock(&WaitListMutex);
    return false;
}

bool MsgMng::AckWaitMsg( Type_MsgAddr waitid,uint16_t type)
{

    pthread_mutex_lock(&WaitListMutex);

    for(lWaitList::iterator item =  WaitDriverList.begin(); item != WaitDriverList.end(); ++item)
    {
        if(((*item).type == type)&&((*item).waitid.app == waitid.app))
        {
            sem_post((*item).pack);
            WaitDriverList.erase(item);
            pthread_mutex_unlock(&WaitListMutex);
            return true;
        }
    }
    pthread_mutex_unlock(&WaitListMutex);
    return false;
}

void MsgMng::RecvMsgProcess(void * arg)
{
    sMsgUnit   pkt;
    uint16_t   pkt_len;
    uint32_t   addr;
    Can_Data *pdriver = (Can_Data *) arg;

    if(!pRecvMsg->ReceiveMsg(&pkt,&pkt_len,RECV_WAIT))
    {
        usleep(10000);
        return;
    }

    if((pkt.dest.driver.id_driver != pdriver->devkey.driverid)&&(pkt.dest.app != BROADCAST_ID))
        return;

    switch(pkt.type)
    {
        case MSG_TYPE_DriverGetInfo:
        uint8_t midchang ;
            soure_id.driver.id_driver = pkt.dest.driver.id_driver;

            dest_id = pkt.source.app;
            zprintf3("receive dest id is %d\n",dest_id);
            addr = pkt.dest.app;
            pkt.dest.app = pkt.source.app;
            pkt.source.app =addr;
//            zprintf1("receive driver info id \n\n\n");

            memcpy(&pkt.data[0] ,&pdriver->d_info,sizeof(sParamInfoType));
            midchang = pkt.data[0];
            pkt.data[0] = pkt.data[1];
            pkt.data[1] = midchang;
            midchang = pkt.data[2];
            pkt.data[2] = pkt.data[3];
            pkt.data[3] = midchang;
            midchang = pkt.data[4];
            pkt.data[4] = pkt.data[5];
            pkt.data[5] = midchang;
            pkt_len = sizeof(sParamInfoType);
            pSendMsg->SendMsg(&pkt,pkt_len);
            break;

        case MSG_TYPE_DriverSendHeart:

            addr = pkt.dest.app;
            pkt.dest.app = pkt.source.app;
            pkt.source.app =addr;
            pSendMsg->SendMsg(&pkt,0);
            break;

        case MSG_TYPE_AppGetIOParam:
            addr = pkt.dest.app;
            pkt.dest.app = pkt.source.app;
            pkt.source.app =addr;
            can_inode_info ininfo;
            pdriver->get_innode_info(pkt.source.driver.id_parent, pkt.source.driver.id_child,
                                     pkt.source.driver.id_point, ininfo);
            memcpy(pkt.data, &ininfo,sizeof(ininfo));
            pSendMsg->SendMsg(&pkt,sizeof(ininfo));
            break;


        default:
            break;
    }
    return;
}

void MsgMng::msgmng_send_msg(sMsgUnit *pdata, uint16_t size)
{
//    for(int i = 0; i < 7; i++){
//        printf("send %d is %d\n", i, pdata->data[i]);
//    }
    pSendMsg->SendMsg(pdata, size);
}

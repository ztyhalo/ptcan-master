
#include <stddef.h>
#include "can_protocol.h"
#include "can_bus.h"
#include "zprint.h"
#include <qdebug.h>
#include "clist.h"
#include <QDateTime>

using namespace std;
CANDATAFORM gBugCanInfo;
int         gBugState;

CanPROMag CanPROMag::canpropoint;

/***********************************************************************************
 * 函数名：get_frame_mapidf
 * 功能：得到映射的map号
 *
 ***********************************************************************************/
int ncan_protocol::get_frame_mapidf(ProtocolType protype, FrameId& fid)
{
    //    printf("fid = 0x%x\n", fid);
    if (idfunc[protype] == NULL) return 0;
    fid = idfunc[protype](fid);
    return 0;
}

/***********************************************************************************
 * 函数名：write_deldata_buf
 * 功能：添加协议帧的信息包括处理函数等 待添加错误管理
 *
 ***********************************************************************************/
int ncan_protocol::add_protocol_frame(unsigned int id, CANPROHEAD info)
{
    canidmap[info.frametype].insert(pair< FrameId, CANPROHEAD >(id, info));
    return 1;
}
/***********************************************************************************
 * 函数名：get_protocol_frameinfo
 * 功能：得到协议帧的信息包括处理函数等 待添加错误管理
 *
 ***********************************************************************************/
int ncan_protocol::get_protocol_frameinfo(unsigned int id, unsigned int type, CANPROHEAD& info)
{
    map< FrameId, CANPROHEAD >::iterator proiter;

    proiter = canidmap[type].find(id);

    if (proiter != canidmap[type].end())
    {
        info = proiter->second;

        return 0;
    }
    //    printf("get pro fram 0x%x type %dfail\n", id, type);
    return -1;
}

/***********************************************************************************
 * 函数名：get_protocol_frameinfo
 * 功能：得到协议帧的信息包括处理函数等 待添加错误管理
 *
 ***********************************************************************************/
CANPROHEAD* ncan_protocol::get_protocol_frameinfo(unsigned int id, unsigned int type)
{
    map< FrameId, CANPROHEAD >::iterator proiter;

    proiter = canidmap[type].find(id);

    if (proiter != canidmap[type].end())
    {
        return &(proiter->second);
    }
    return NULL;
}

/***********************************************************************************
 * 函数名：canpro_timeover_del
 * 功能：can协议对超时帧进行处理
 *
 ***********************************************************************************/
int canpro_time_callback(CANp_TIME_ET* tmpara)
{
    CAN_DATA_LIST* midpoint = NULL;
    CAN_DATA_LIST* copymid  = NULL;
    CAN_DATA_LIST* retval   = NULL;
    int            err      = 0;

    pthread_mutex_lock(&tmpara->father->prolist.list_mut);

    //    qDebug("time mute enter is %d !",list_mut.__align);
    midpoint                = tmpara->father->prolist.rw_P;
    tmpara->father->errsize = 0;
    while (midpoint != NULL)
    {
        //        zprintf3("fram is 0x%x overtime is %d run is %d\n", midpoint->p->canhead.frameid,
        //        midpoint->p->runtime, midpoint->p->canhead.overtime);
        midpoint->p->runtime += tmpara->father->inter;
        if (midpoint->p->runtime >= midpoint->p->canhead.overtime)
        {
            if (midpoint->p->sendnum >= midpoint->p->canhead.repeatnum)
            {
                err++;
                memcpy(&tmpara->father->errbuf[tmpara->father->errsize], midpoint->p, sizeof(CANPRODATA));
                tmpara->father->errsize++;

                copymid  = midpoint;
                midpoint = midpoint->next;

                if (retval == NULL)
                {
                    tmpara->father->prolist.rw_P   = copymid->next;
                    copymid->next                  = tmpara->father->prolist.free_P;
                    tmpara->father->prolist.free_P = copymid;
                }
                else
                {
                    retval->next                   = copymid->next;
                    copymid->next                  = tmpara->father->prolist.free_P;
                    tmpara->father->prolist.free_P = copymid;
                }
            }
            else
            {
                midpoint->p->runtime = 0;
                midpoint->p->sendnum++;
                zprintf1("can 0x%x over timer err state %d err fram 0x%x!\n", midpoint->p->candata.ExtId, gBugState,
                    gBugCanInfo.ExtId);
                tmpara->father->candrip->write_send_data(midpoint->p->candata);
            }
        }
        else
        {
            retval   = midpoint;
            midpoint = midpoint->next;
        }
    }

    err = pthread_mutex_unlock(&tmpara->father->prolist.list_mut);
    for (int i = 0; i < tmpara->father->errsize; i++)
    {
        if (tmpara->father->errbuf[i].canhead.overcallbacfun != NULL)
        {
            tmpara->father->errbuf[i].canhead.overcallbacfun(
                tmpara->father->errbuf[i].canhead.father, tmpara->father->errbuf[i].candata);
        }
        else
        {
            zprintf1("over back is NULL 0x%x 0x%d\n", tmpara->father->errbuf[i].candata.ExtId,tmpara->father->errbuf[i].canhead.frameid);
        }
    }

    return err;
}
/***********************************************************************************
 * 函数名：pro_rxmsg_callback
 * 功能：can协议接收处理回调函数
 *
 ***********************************************************************************/
int pro_rxmsg_callback(CanDriver* pro, CanFrame data)
{
    unsigned int frameid;
    int          err = -1;
    CANPROHEAD   rxheadinfo;

    CANDATAFORM    rxmeg  = lawdata_to_prodata(data);
    ncan_protocol* midpro = static_cast<ncan_protocol*>(pro->father_p);

    frameid = rxmeg.IDE ? rxmeg.ExtId : rxmeg.StdId;

    if (midpro->get_frame_mapidf(rxmeg.IDE, frameid) == 0)
    {
        if (midpro->get_protocol_frameinfo(frameid, rxmeg.IDE, rxheadinfo) == 0)
        {
            // QDateTime dateTime = QDateTime::currentDateTime();

            //            printf("time = %d\n", dateTime.time().msec());
            if (rxheadinfo.cancallbacfun != NULL)
            {
                rxheadinfo.cancallbacfun(rxheadinfo.father, rxmeg);
                err = 0;
            }
            else
            {
                //                printf("frameid = 0x%x\n", frameid);
                zprintf1("rxheadinfo.cancallbacfun = NULL %x\n", frameid);
            }
        }
        else
        {
            err = -2;
        }
    }
    return err;
}

/***********************************************************************************
 * 函数名：can_protocol_send
 * 功能：通过can协议发送can帧
 *
 ***********************************************************************************/
int ncan_protocol::can_protocol_send(CANPROFRAME canprop)
{
    CANPRODATA middata;

    memset(&middata, 0, sizeof(CANPRODATA));
    memcpy(&middata, &canprop, sizeof(CANPROFRAME));
    if (middata.sendnum < middata.canhead.repeatnum) middata.sendnum++;
    if (middata.canhead.overtime != -1)
    {
        prolist.buf_write_data(middata);
    }

    candrip->write_send_data(canprop.candata);
    return 0;
}

/***********************************************************************************
 * 函数名：can_protocol_send
 * 功能：通过can协议发送can帧 参数为can数据帧结构
 *
 *
 ***********************************************************************************/
int ncan_protocol::can_protocol_send(CANDATAFORM canprop)
{
    CANPRODATA   middata;
    CANPROHEAD   midprohead;
    unsigned int frameid;
    int          reghead;
    int          ret = -1;
    frameid          = canprop.IDE ? canprop.ExtId : canprop.StdId;

    if (get_frame_mapidf(canprop.IDE, frameid) != 0)
    {
        return -1;
    }

    reghead = get_protocol_frameinfo(frameid, canprop.IDE, midprohead);
    if (reghead == 0 && midprohead.sendiffunc != NULL)
    {
        if (midprohead.sendiffunc(midprohead.father) == 0) //不满足发送条件
        {
            return -1;
        }
    }
    memset(&middata, 0, sizeof(CANPRODATA));
    if (reghead == 0) //已经注册了该帧的协议头
    {
        middata.candata = canprop;
        middata.canhead = midprohead;

        if (middata.sendnum < middata.canhead.repeatnum) middata.sendnum++;
        if (middata.canhead.overtime != -1)
        {
            prolist.buf_write_data(middata);
        }
    }
    // printf("pro fram 0x%x============================================\n",middata.candata.ExtId);
    ret = candrip->write_send_data(canprop);
    return ret;
}

/***********************************************************************************
 * 函数名：can_protocol_send
 * 功能：通过can协议发送can帧 参数为can数据帧结构
 *
 *
 ***********************************************************************************/
int ncan_protocol::can_protocol_send_frame(CANDATAFORM canprop)
{
    CANPRODATA   middata;
    CANPROHEAD   midprohead;
    unsigned int frameid;
    int          reghead;

    frameid = canprop.IDE ? canprop.ExtId : canprop.StdId;

    //    printf("frameid is 0x%x 0x%x\n", frameid, canprop.StdId);
    if (get_frame_mapidf(canprop.IDE, frameid) != 0)
    {
        //        tkdegprintf("can_protocol_send get frame fail");
        return -1;
    }

    reghead = get_protocol_frameinfo(frameid, canprop.IDE, midprohead);
    if (reghead == 0 && midprohead.sendiffunc != NULL)
    {
        if (midprohead.sendiffunc(midprohead.father) == 0) //不满足发送条件
        {
            return -1;
        }
    }
    memset(&middata, 0, sizeof(CANPRODATA));
    if (reghead == 0) //已经注册了该帧的协议头
    {
        middata.candata = canprop;
        middata.canhead = midprohead;

        if (middata.sendnum < middata.canhead.repeatnum) middata.sendnum++;
        if (middata.canhead.overtime != -1)
        {
            //            printf("pro fram 0x%x\n",middata.candata.StdId);

            prolist.buf_write_data(middata);
        }
    }
    candrip->writeframe(canprop);

    return 0;
}

int frame_com_func(CANPRODATA& f, CANPRODATA& c)
{
    if (c.candata.IDE == 1) //扩展帧
    {
        return f.candata.ExtId == c.candata.ExtId ? 0 : -1;
    }
    else
    {
        return f.candata.StdId == c.candata.StdId ? 0 : -1;
    }
}

int ncan_protocol::pro_del_buf_frame(unsigned int frameid, unsigned char frametype)
{
    CANPRODATA cmp;
    cmp.candata.IDE = frametype;
    frametype == 1 ? cmp.candata.ExtId = frameid : cmp.candata.StdId = frameid;
    return prolist.condition_delete_list_data(frame_com_func, cmp);
}

int frame_full_func(CANPRODATA& f, CANPRODATA& c)
{
    return memcmp(&f.candata, &c.candata, sizeof(CANDATAFORM)) == 0 ? 0 : -1;
}

int ncan_protocol::pro_del_buf_frame(CANDATAFORM& frame)
{
    int        err;
    CANPRODATA cmp;
    memcpy(&cmp.candata, &frame, sizeof(CANDATAFORM));

    err = prolist.condition_delete_list_data(frame_full_func, cmp);
    //    printf("return err %d\n",err);
    return err;
}

/***********************************************************************************
 * 函数名：read_deldata_buf
 * 功能：根据帧id和帧类型从超时处理队列内查找该帧地址
 *
 ***********************************************************************************/
CANPRODATA* ncan_protocol::read_deldata_buf(unsigned int frameid, unsigned char frametype)
{
    CANPRODATA cmp;
    cmp.candata.IDE = frametype;
    frametype == 1 ? cmp.candata.ExtId = frameid : cmp.candata.StdId = frameid;
    return prolist.buf_read_data(frame_com_func, cmp);
}

int ncan_protocol::del_buf_frame(CANPRODATA* canprop)
{
    return prolist.delete_list_data(canprop);
}

int ncan_protocol::ncan_pro_init(CanDriver* dri)
{
    //   GETCANPROMAG(canpro);

    protm.add_event(0.01, canpro_time_callback, this);
    //    protm.start();
    inter         = 10;
    candrip       = dri;
    dri->father_p = this;
    //    dri->canread.z_pthread_init(pro_rxmsg_callback, dri);
    dri->rxcallback = pro_rxmsg_callback;

    return 0;
}

void ncan_protocol::start(void)
{
    protm.start("can protocol timer");
}

int ncan_protocol::init_pro_frame(CANPROHEAD* info, int size)
{
    if (info != NULL)
    {
        for (int i = 0; i < size; i++)
            add_protocol_frame(info[i].frameid, info[i]);
        return 0;
    }
    return -1;
}

/***********************************************************************************
 * 函数名：poll_timer_callback
 * 功能：论询回调函数
 *
 ***********************************************************************************/
int poll_timer_callback(CANp_TIME_ET* poll)
{

    poll->father->can_protocol_send(*(CANDATAFORM*) poll->para);

    return 0;
}

int ncan_protocol::add_poll_frame(CANDATAFORM& frame, double inter)
{

    return protm.add_event(inter, poll_timer_callback, this, pollfram.buf_wr_data(frame));
}

int ncan_protocol::add_poll_frame(double inter, void* arg)
{

    return protm.add_event(inter, poll_timer_callback, this, arg);
}

int ncan_protocol::add_poll_frame(double inter, Func_Poll callback, void* arg)
{

    return protm.add_event(inter, callback, this, arg);
}

int ncan_protocol::delete_poll_timer(int id)
{
    return protm.delete_event(id);
}

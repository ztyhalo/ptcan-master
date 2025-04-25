/*
 *********************************************************************************************
 *                                      平台项目
 * File：
 * Module:
 * Version:
 * History:
 * ________________________
 * tk100+iocan通讯应用
 *********************************************************************************************
*/
#include "stdio.h"
#include "tkcommon.h"
#include "zprint.h"
#include "candatainfo.h"


#include <QDomDocument>
#include <QFile>
#include <QTextCodec>

#include <QTextStream>
#include <QXmlStreamWriter>
#include <QStringList>
#include <QDebug>
#include "tk200pro.h"



sem_t     gTk200_Rest_Meg;


void DEV_SData_Pro::set_dev_enable(uchar val){
    dev_enbale = val;
    mem->lock_qtshare();
    data->dev_enbale = val;
    mem->unlock_qtshare();
}
void DEV_SData_Pro::set_dev_state(uchar val){
    dev_state = val;
    mem->lock_qtshare();
    data->dev_state = val;
    mem->unlock_qtshare();
}

/***********************************************************************************
 * 函数名：canpro_timeover_del
 * 功能：can协议对超时帧进行处理
 *
 ***********************************************************************************/
int dev_node_time_callback(DEV_TIME_ET * tmpara)
{
    Dev_Node_Pro  * midpoint = NULL;

    midpoint = tmpara->father;
    midpoint->poll_dev_node();

    return 0;
}
void Dev_Node_Pro::add_dev_node(uint8_t node, uint16_t time)
{
    uint32_t count = time/ms;
    Node_Shake midnod(node, count);
    nodemap.insert(node, midnod);
}

void Dev_Node_Pro::dev_node_reset(void)
{
    typename QMap<uint8_t, Node_Shake>::iterator iter;
     for(iter = nodemap.begin(); iter != nodemap.end(); ++iter)
     {
        iter.value().node_reset();
     }
}

void Dev_Node_Pro::set_share_data(uint8_t node, uint8_t val)
{
    if(share_p != NULL)
    {
        share_p->set_share_data_value(ndev_id, nc_id, node, val);
    }
}

void Dev_Node_Pro::poll_dev_node(void)
{
    typename QMap<uint8_t, Node_Shake>::iterator iter;
     for(iter = nodemap.begin(); iter != nodemap.end(); ++iter)
     {

            if(iter.value().shake_process())
            {
//                zprintf3("node%d val %d!\n", iter.key(), iter.value().node_val);
                set_share_data(iter.key(),  iter.value().node_val);
            }
     }

}

void Dev_Node_Pro::dev_node_pro_init(void)
{
    if(nodemap.size())
    {
        zprintf3("timer init!\n");

        double t = ((double)ms)/1000;
        dtimer.add_event(t, dev_node_time_callback, this);
        dtimer.start("dev timer");
    }
}

void Dev_Node_Pro::set_node_new_data(uint8_t node, uint8_t val)
{
    typename QMap<uint8_t, Node_Shake>::iterator iter;
    iter = nodemap.find(node);
    if(iter != nodemap.end())
    {
        if(iter.value().shake_num == 0)
        {
            set_share_data(node,  val);
        }
        else
        {
            iter.value().set_new_val(val);
        }
    }
}

/***********************************************************************************
 * 函数名：void PT_Dev_Virt::dev_send_meg(uint8_t type, uint8_t *data)
 * 功能：设备的消息初始化
 *
 *
 ***********************************************************************************/
void PT_Dev_Virt::dev_send_meg(uint8_t megtype, uint8_t *data, uint16_t size)
{
    if(msgmng_p->dest_id == 0)
    {
        zprintf3("ddddd is %d\n\n", msgmng_p->dest_id);
        return;
    }
    sMsgUnit   pkt;
    Dev_Message devmeg(devtype);
    zprintf1("dev send meg tyep %d %d %d\n\n\n", megtype, sizeof(Dev_Message), size+sizeof(Dev_Message));

    pkt.dest.app = msgmng_p->dest_id;
    pkt.source.driver.id_driver = msgmng_p->soure_id.driver.id_driver;
    pkt.source.driver.id_parent = dev_off;
    pkt.source.driver.id_child = child_id;
    pkt.source.driver.id_point = 0;

    pkt.type = MSG_TYPE_DevAutoReport;

    devmeg.meg_type = megtype;
    devmeg.meg_size = size;

    memcpy(pkt.data, &devmeg, sizeof(Dev_Message));
    if(data != NULL){
        memcpy(&pkt.data[sizeof(Dev_Message)], data, size);
    }
//    printf("pdata 0 is %d\n", pkt.data[sizeof(Dev_Message)]);
    msgmng_p->msgmng_send_msg(&pkt, size+sizeof(Dev_Message));
}

void PT_Dev_Virt::reset_data_init(void)
{
    ;
}

/***********************************************************************************
 * 函数名：io_send_condition
 * 功能：帧发送条件
 *
 ***********************************************************************************/
int tk_io_send_condition(void * para)
{
    (void) para;
    return 1;
}

int tk_io_poll_send_condition(void * para)
{
    (void) para;
        return 1; 
}

int TK_IO_Dev::tk_io_reset_config(void)
{
    // int err =0;
    CANDATAFORM data;

    attr.configstate = 0;
    attr.iostate = TK_IO_INIT;
//    err = read_config();

    // if(err == 0){

        data.StdId = SET_FRAM_DEV(0x450, io_id);
        data.IDE = 0;
        data.DLC = (dev_para.innum+dev_para.outnum)/4;
        set_io_conf(data.Data);

        pro_p->can_protocol_send(data);
        return 0;
    // }
    // return -1;
}
void TK_IO_Dev::set_io_conf(uint8_t * buf)
{
    int off = 0;
    if(dev_para.type != TK200_INModule)
        off = 3;
    if(dev_para.outnum)
    {
        memcpy(buf, &outconfig, off);
    }
    memcpy(buf+off, &inconfig, dev_para.innum/4);
}
/***********************************************************************************
 * 函数名：power_requireack_idprocess
 * 功能:上电认可及配置应答帧处理函数
 *
 ***********************************************************************************/

int io_poll_callback(CANp_TIME_ET * poll)
{

   poll->father->can_protocol_send(*(CANDATAFORM *)poll->para);

   return 0;

}
int tk_power_requireack_idprocess(void * tkio_dev, CANDATAFORM data)
{
    // int err = 0;
    TK_IO_Dev * midp = static_cast<TK_IO_Dev *>(tkio_dev);
    uint8_t   dlcsize = 0;
    if((midp->dev_para.innum+midp->dev_para.outnum)%4){
        dlcsize = (midp->dev_para.innum+midp->dev_para.outnum)/4 + 1;
    }else
         dlcsize = (midp->dev_para.innum+midp->dev_para.outnum)/4;


    if(data.DLC == dlcsize)
    {
        if(midp->attr.iostate == TK_IO_INIT)
        {
            midp->attr.iostate = TK_IO_NORMAL;
//            memset(&data, 0x00, sizeof(CANDATAFORM));
//            data.StdId = SET_FRAM_DEV(0x45f, midp->io_id);
            midp->pro_p->can_protocol_send(midp->pollFrame);

//            midp->polltimer = midp->pro_p->add_poll_frame(data, 2);
//            midp->polltimer = midp->pro_p->add_poll_frame(2, &midp->pollFrame);
            midp->add_dev_timer(2, &midp->pollFrame);

        }
        int err = midp->pro_p->pro_del_buf_frame(SET_FRAM_DEV(0x450, midp->io_id), 0);
        if(err == 0)
                zprintf3("config over\n");
        else
             zprintf1("dele fail %d\n",err);
    }
    return 0;
}
/***********************************************************************************
 * 函数名：start_ask_idprocess
 * 功能：开始请求帧的处理
 *
 ***********************************************************************************/
int tk_start_ask_idprocess(void * tkio_dev, CANDATAFORM data)
{
    TK_IO_Dev * midp = static_cast<TK_IO_Dev *>(tkio_dev);
    zprintf3("call back 1\n");
    if(data.DLC == 0)
    {
        midp->delete_dev_timer();
        if(midp->attr.iostate == TK_IO_INIT)
        {
             zprintf3("call back 2\n");
            if(midp->attr.configstate == 1)                   //配置完成
            {
                zprintf3("call back 3\n");
                data.StdId = SET_FRAM_DEV(0x450, midp->io_id);
                data.IDE = 0;
                if(midp->dev_para.type != TK200_IOModule){
                    data.DLC = (midp->dev_para.innum+midp->dev_para.outnum)/4;
                }else{
                     data.DLC = 6;
                }
                midp->set_io_conf(data.Data);;

                midp->pro_p->can_protocol_send(data);
            }
        }
        else if(midp->attr.iostate == TK_IO_BUG)
        {
            midp->attr.iostate = TK_IO_INIT;
        }
        else
        {
            midp->attr.iostate = TK_IO_BUG;
        }
    }
    else
    {
        if(midp->dev_para.type != TK100_IOModule_IO)
        {
            tk_power_requireack_idprocess(tkio_dev, data);
        }
    }
    return 1;
}

/***********************************************************************************
 * 函数名：out_controlack_idprocess
 * 功能:输出控制应答帧的处理
 *
 ***********************************************************************************/
int tk_out_controlack_idprocess(void * tkio_dev, CANDATAFORM data)
{   
    // int err;
    TK_IO_Dev * midp = static_cast<TK_IO_Dev *>(tkio_dev);

    if(data.DLC == 2)
    {
        data.StdId = SET_FRAM_DEV(0x451, midp->io_id);;
         int err = midp->pro_p->pro_del_buf_frame(data);
         if(err == 0)
         {
            zprintf4("output success\n");
//            int out_node = (midp->dev_para.type == TK100_IOModule_IO) ?
//                                                data.Data[0]+1 : data.Data[0];
//            out_node += midp->dev_para.innum;
//             midp->data_p->set_out_ack_value(midp->dev_off, 0, out_node,
//                       midp->dev_para.type == TK100_IOModule_IO? !data.Data[1] : data.Data[1]);
            midp->set_out_node_val(data.Data[0], data.Data[1]);
            sem_post(&midp->sendSem);
         }
         else
              zprintf1("output fail %d\n",err);
    }

    return 1;
}

/***********************************************************************************
 * 函数名：data_requireack_idprocess
 * 功能:数据请求应答帧处理函数    //待添加数据存取
 *
 ***********************************************************************************/
int tk_data_requireack_idprocess(void * tkio_dev, CANDATAFORM data)
{
    TK_IO_Dev * midp = static_cast<TK_IO_Dev *>(tkio_dev);
    TK_FRAMEID  midfram;
    midfram.fram = data.StdId;

    if(midfram.fram_org.func > 5)
        midfram.fram_org.func -= 2;

    midp->pollmark.set(midfram.fram_org.func - 2);
    memcpy(&midp->iocandata[(midfram.fram_org.func - 2)*8], data.Data, 8);
    return 0;
}
/***********************************************************************************
 * 函数名：data_requireack_idprocess
 * 功能:数据请求应答帧处理函数    //待添加数据存取
 *
 ***********************************************************************************/
int poll_end_fram_idprocess(void * tkio_dev, CANDATAFORM data)
{
    uint16_t   *     inmidp = NULL;
    TK_IO_Dev * midp = static_cast<TK_IO_Dev *>(tkio_dev);
    TK_FRAMEID  midfram;
    midfram.fram = data.StdId;

    midp->pollmark.set(midfram.fram_org.func - 2);         //tk200的in模块结束帧让人疯狂 0X674
    memcpy(&midp->iocandata[(midfram.fram_org.func - 2)*8], data.Data, 8);
    if(midp->pollmark.to_ulong() == (uint)((0x01 << (midp->dev_para.innum/4)) -1))
    {
//        printf("io set poll receive data\n");
        midp->pollmark.reset();
        midp->attr.errcount = 0;
        midp->pro_p->pro_del_buf_frame(SET_FRAM_DEV(0x45f, midp->io_id), 0);

        tbyte_swap((uint16_t *)midp->iocandata, midp->dev_para.innum *2);

        for(int i = 0; i < midp->dev_para.innum; i++)
        {
            if(midp->inconfig.test(TK_IO_IN(i)+1))      //使能
            {
                inmidp = (uint16_t *)(&midp->iocandata[i*2]);
                if(inmidp != NULL && midp->data_p != NULL &&
                        *inmidp != midp->inbuf[i].outinfo)
                {
                    midp->inbuf[i].outinfo = *inmidp;

                    if(midp->inbuf[i].swin.instyle1 == TK_SW_IN)
                    {
                        double midval = 0;

                        switch (midp->inbuf[i].swin.swstate3)
                        {

                            case SWITCH_BROKEN :            //开关量断路
                                midval = 2;
                                break;
                            case SWITCH_SHORT:             //开关量短路
                                midval = 3;
                              break;
                            case SENSOR_FAULT:          //传感器故障
                            case FREQ_LOW:                 //频率量小于200

                            case FREQ_HIGHT:                //频率量大于1000
                             zprintf3(" node switch err!\n");
                            break;

                            case NODE_OK :                   //
                                midval = (midp->dev_para.type == TK100_IOModule_IO) ?
                                            !midp->inbuf[i].swin.swvale1 : midp->inbuf[i].swin.swvale1;
                                break;

                        }

                        //midp->data_p->set_share_data_value(midp->dev_off,0, i+1, midval);
                        midp->set_in_switch_node_val(i+1, midval);
                    }
                    else
                    {
                         double midval = 0;
                         switch (midp->inbuf[i].fin.fstate3)
                         {
                             case SENSOR_FAULT:          //传感器故障
                                 midval = 0;
                                 break;
                             case SWITCH_BROKEN :            //开关量断路
                             case SWITCH_SHORT:             //开关量短路
                               zprintf3("freq node err!\n");
                               break;

                             case FREQ_LOW:                 //频率量小于200
                                midval = 0;
                                break;
                             case FREQ_HIGHT:                //频率量大于1000
                                 midval = 0xffff;
                                 break;
                             case NODE_OK :                   //
                                 midval = ((midp->inbuf[i].fin.fvale12/5)+200)*20;
                                 break;

                         }
//                         midp->data_p->set_share_data_value(midp->dev_off,0, i+1, midval);//midp->inbuf[i].fin.fvale12);
                        midp->set_in_freq_node_val(i+1, midval);
                    }

                }
            }
            else
            {
                zprintf4("in %d disable\n", i );
            }
        }
        if(midp->dev_state == 0){
            midp->set_dev_state(DEV_ON_LINE);
        }
    }
    else
        zprintf1("poll fram 0x%x error %ld %d\n", data.StdId, midp->pollmark.to_ulong() , (uint)((0x01 << (midp->dev_para.innum/4)) -1));

    return 0;
}

/***********************************************************************************
 * 函数名：change_ask_idprocess
 * 功能:输入口改变上报帧处理函数 //待添加值存储位置
 *
 ***********************************************************************************/
int tk_change_ask_idprocess(void * tkio_dev, CANDATAFORM data)
{
    TK_IO_Dev * midp = static_cast<TK_IO_Dev *>(tkio_dev);

    if(midp->attr.iostate == TK_IO_NORMAL)
    {
        if(midp->inconfig.test(TK_IO_IN(data.Data[0])) == TK_SW_IN)
        {
//            double midturnval;
//            midturnval    =  !data.Data[1];
//            midp->data_p->set_share_data_value(midp->dev_off, 0,
//                      midp->dev_para.type == TK100_IOModule_IO? data.Data[0]+1 : data.Data[0],
//                      midp->dev_para.type == TK100_IOModule_IO? !data.Data[1] : data.Data[1]);
            midp->set_in_switch_node_val(midp->get_io_node(data.Data[0]),
                                          midp->set_io_val(data.Data[1]));
        }
        else{
//            tkdegprintf("node %d freq", data.Data[0]);
        }
        midp->set_send_data_fram(data.StdId);
        midp->pro_p->candrip->write_send_data(data);
    }
    return 1;
}
/***********************************************************************************
 * 函数名：frame_overtime_process
 * 功能：帧超时处理函数
 *
 ***********************************************************************************/
int tk_frame_overtime_process(void * tkio_dev, CANDATAFORM  overmeg)
{
    (void) overmeg;
   TK_IO_Dev * midp = static_cast<TK_IO_Dev *>(tkio_dev);
    midp->attr.iostate = TK_IO_BUG;
    return 1;
}

/***********************************************************************************
 * 函数名：frame_overtime_process
 * 功能：帧超时处理函数
 *
 ***********************************************************************************/
int tk_outframe_overtime_process(void * tkio_dev, CANDATAFORM  overmeg)
{
    (void)overmeg;
    TK_IO_Dev * midp = static_cast<TK_IO_Dev *>(tkio_dev);
    midp->delete_dev_timer();
    midp->attr.iostate = TK_IO_BUG;
    midp->attr.errcount = 0;
    if(midp->dev_state == 1){
        midp->set_dev_state(DEV_OFF_LINE);
    }
    zprintf1("io dev output reset!\n");
    sem_post(&gTk200_Rest_Meg);
    return 1;
}
/***********************************************************************************
 * 函数名：datareqframe_overtime_process
 * 功能:论询超时处理
 *
 ***********************************************************************************/
int tk_datareqframe_overtime_process(void * tkio_dev, CANDATAFORM  overmeg)
{
    (void) overmeg;
   TK_IO_Dev * midp = static_cast<TK_IO_Dev *>(tkio_dev);
    midp->attr.errcount++;
    zprintf1("io dev overtimer!\n");
    if(midp->attr.errcount >= TK_IO_SATET_N)
    {
        midp->delete_dev_timer();
        midp->attr.iostate = TK_IO_BUG;
        midp->attr.errcount = 0;
        if(midp->dev_state == 1){
            midp->set_dev_state(DEV_OFF_LINE);
        }
        zprintf1("io dev poll reset!\n");
        sem_post(&gTk200_Rest_Meg);
    }
    return 0;
}

// 时间 time 自加 ms 毫秒
void time_add_ms(struct timeval *time, uint ms)
{
        time->tv_usec += ms * 1000; // 微秒 = 毫秒 * 1000
        if(time->tv_usec >= 1000000) // 进位，1000 000 微秒 = 1 秒
        {
                time->tv_sec += time->tv_usec / 1000000;
                time->tv_usec %= 1000000;
        }
}


/***********************************************************************************
 * 函数名：tk100_control_output
 * 功能：cs应用层控制输出函数
 *
 ***********************************************************************************/
void TK_IO_Dev::data_send(soutDataUnit  val)
{
   CANDATAFORM     data;

    if(attr.iostate != TK_IO_NORMAL)
        return;
    struct timespec t;
    struct timeval time;
    gettimeofday(&time, NULL);
    time_add_ms(&time, 10);
    t.tv_sec = time.tv_sec;
    t.tv_nsec = time.tv_usec * 1000;
    int ret = sem_timedwait(&sendSem,  &t);
    if(ret == -1)
    {
        zprintf2("io send over time!\n");
    }
    if(attr.iostate != TK_IO_NORMAL)
        return;
    memset(&data, 0x00, sizeof(CANDATAFORM));
//    zprintf3("send val  %d %d %d %d\n", val.parentid, val.childid, val.pointid, (int)val.value);

    data.StdId = SET_FRAM_DEV(0x451, io_id);
    data.DLC = 2;
    data.Data[0] = val.pointid;                //待确定开始输出号
    if(dev_para.type == TK100_IOModule_IO)
        data.Data[1] = !((int)val.value);
    else
         data.Data[1] = val.value;
    pro_p->can_protocol_send(data);

}

/***********************************************************************************
 * 函数名：tk100_control_output
 * 功能：cs应用层控制输出函数
 *
 ***********************************************************************************/
int tk100_output(void * midp, soutDataUnit val)
{
    TK_IO_Dev * pro = static_cast<TK_IO_Dev *>(midp);
    pro->data_send(val);
    return 1;
}

int TK_IO_Dev::tk_io_config(CAN_DEV_INFO & dev, uint8_t devoff)
{

    dev_off = devoff;
    dev_para = dev.para;
    io_id = dev.para.id;

    dev_node_data_init(data_p, 100, dev_off);

    inbuf = new TKINPUT[dev.para.innum];
    if(inbuf == NULL)
    {
        zprintf1("io creat inbuf fail!\n");
        return -1;
    }
    iocandata = new uint8_t[dev.para.innum *2];
    if(iocandata == NULL)
    {
        zprintf1("io creat iocandata fail!\n");
        return -2;
    }

   for(int i = 0; i < dev.para.innum; i++)
   {
        inconfig.set(TK_IO_IN(i)+1, dev.inode[i].node_en);
        inconfig.set(TK_IO_IN(i), dev.inode[i].datatype != 2 ? 0 : 1);
        if(dev.inode[i].node_en && (dev.inode[i].datatype != 2))
        {
            zprintf3("Add node %d shake %d!\n", i+1, dev.inode[i].shake_time);
            add_dev_node(i+1, dev.inode[i].shake_time);
        }

   }
    for(int i = 0; i <dev.para.outnum; i++)
    {
        outconfig.set(TK_IO_IN(i),    0);           //常开常闭
        outconfig.set(TK_IO_IN(i)+1,  dev.onode[i].link_stop);       //是否连锁
    }
    attr.configstate = 1;
    return 0;

}

void TK_IO_Dev::set_out_node_val(int node, int val)
{
    if(data_p != NULL)
    {
        int out_node = get_io_node(node);
        out_node += dev_para.innum;
         data_p->set_out_ack_value(dev_off, 0, out_node, set_io_val(val));
    }
}

void TK_IO_Dev::set_in_switch_node_val(int node, int val)
{
    set_node_new_data(node, val);
}
void TK_IO_Dev::set_in_freq_node_val(int node, int val)
{
    if(data_p != NULL)
    {
        data_p->set_share_data_value(dev_off, 0, node, val);
    }
}
int TK_IO_Dev::pt_dev_init(void)
{
    set_dev_enable(1);

    memset(&pollFrame, 0x00, sizeof(CANDATAFORM));
    pollFrame.StdId = SET_FRAM_DEV(0x45f, io_id);

        CANPROHEAD  canmidinfo[] ={
            {
    /*****************上电认可帧 0x450******************************************/
                0,
                SET_FRAM_DEV(0x450, io_id),                                  //帧id
                2000,                                   //超时时间
                2,                                      //重发次数
                0x00,                                   //应答帧
                this,
                NULL,                                   //接收回调函数
                tk_frame_overtime_process,                 //超时回调函数
                tk_io_send_condition,                      //发送条件回调函数
            },
    /*****************输出控制帧 0x451******************************************/
            {
                0,
            SET_FRAM_DEV(0x451, io_id),                                  //帧id
                20,                                     //超时时间
                5,                                      //重发次数
                0x00,                                   //应答帧
                this,
                NULL,                                   //接受回调函数
                tk_outframe_overtime_process,                 //超时回调函数
                tk_io_poll_send_condition,                    //发送条件回调函数
            },
    /*****************数据请求帧 0x45f******************************************/
            {
                0,
                SET_FRAM_DEV(0x45f, io_id),                                  //帧id
                80,                                     //超时时间
                1,                                      //重发次数
                0x00,                                   //应答帧
                this,
                NULL,                                   //接受回调函数
                tk_datareqframe_overtime_process,          //超时回调函数
                tk_io_poll_send_condition,                    //发送条件回调函数
            },
    /****************************以下为接收帧*********************************************/
    /*****************上电认可帧 0x650******************************************/
            {
                0,
                SET_FRAM_DEV(0x650, io_id),                                  //帧id
                0,                                      //超时时间
                0,                                      //重发次数
                0x00,                                   //应答帧
                this,
                tk_start_ask_idprocess,                    //接受回调函数
                NULL,                                   //超时回调函数
                NULL,                                   //发送条件回调函数
            },
    /*****************输出控制指令应答 0x651******************************************/
            {
                0,
                SET_FRAM_DEV(0x651, io_id),                                  //帧id
                0,                                      //超时时间
                0,                                      //重发次数
                0x00,                                   //应答帧
                this,
                tk_out_controlack_idprocess,               //接受回调函数
                NULL,                                   //超时回调函数
                NULL,                                   //发送条件回调函数
             },

    /*****************数据请求应答帧1 0x652******************************************/
            {
                0,
                SET_FRAM_DEV(0x652, io_id),                                  //帧id
                0,                                      //超时时间
                0,                                      //重发次数
                0x00,                                   //应答帧
                this,
                tk_data_requireack_idprocess,              //接受回调函数
                NULL,                                   //超时回调函数
                NULL,                                   //发送条件回调函数
            },
    /*****************数据请求应答帧2 0x653******************************************/
            {
                0,
                SET_FRAM_DEV(0x653, io_id),                                  //帧id
                0,                                      //超时时间
                0,                                      //重发次数
                0x00,                                   //应答帧
                this,
                tk_data_requireack_idprocess,              //接受回调函数
                NULL,                                   //超时回调函数
                NULL,                                   //发送条件回调函数
            }
         };
    if(dev_para.type == TK100_IOModule_IO)
    {
        CANPROHEAD  tk100ioinfo[] ={
            /*****************tk100的输入口变化响应 0x457******************************************/
                    {
                        0,
                        0x457,                                  //帧id
                        -1,                                     //超时时间
                        1,                                      //重发次数
                        0x00,                                   //应答帧
                        this,
                        NULL,                                   //接受回调函数
                        NULL,                                   //超时回调函数
                        tk_io_poll_send_condition,                    //发送条件回调函数
                    },

            /*****************数据请求应答帧3 0x654******************************************/
                    {
                        0,
                        0x654,                                  //帧id
                        0,                                      //超时时间
                        0,                                      //重发次数
                        0x00,                                   //应答帧
                        this,
                        tk_data_requireack_idprocess,              //接受回调函数
                        NULL,                                   //超时回调函数
                        NULL,                                   //发送条件回调函数
                    },
            /*****************数据请求应答帧4 0x655******************************************/
                    {
                        0,
                        0x655,                                  //帧id
                        0,                                      //超时时间
                        0,                                      //重发次数
                        0x00,                                   //应答帧
                        this,
                        poll_end_fram_idprocess,              //接受回调函数
                        NULL,                                   //超时回调函数
                        NULL,                                   //发送条件回调函数
                    },
            /*****************上电认可及配置应答帧 0x656******************************************/
                    {
                        0,
                        SET_FRAM_DEV(0x656, io_id),                      //帧id
                        0,                                      //超时时间
                        0,                                      //重发次数
                        0x00,                                   //应答帧
                        this,
                        tk_power_requireack_idprocess,             //接受回调函数
                        NULL,                                   //超时回调函数
                        NULL,                                   //发送条件回调函数
                    },
            /*****************输入口变化请求 0x657***********************************************************/
                    {
                        0,
                        0x657,                                  //帧id
                        0,                                      //超时时间
                        0,                                      //重发次数
                        0x00,                                   //应答帧
                        this,
                        tk_change_ask_idprocess,                   //接受回调函数
                        NULL,                                   //超时回调函数
                        NULL,                                   //发送条件回调函数
                    }
        };
        pro_p->init_pro_frame(tk100ioinfo, sizeof(tk100ioinfo)/sizeof(CANPROHEAD));
    }
    else
    {
        CANPROHEAD  tkioinfo[] ={
            /*****************数据请求应答帧3 0x654******************************************/
                    {
                        0,
                        SET_FRAM_DEV(0x654, io_id),                                  //帧id
                        0,                                      //超时时间
                        0,                                      //重发次数
                        0x00,                                   //应答帧
                        this,
                        poll_end_fram_idprocess,              //接受回调函数
                        NULL,                                   //超时回调函数
                        NULL,                                   //发送条件回调函数
                    },
        /*****************输入口变化请求 0x655***********************************************************/
                {
                    0,
                    SET_FRAM_DEV(0x655, io_id),                                  //帧id
                    0,                                      //超时时间
                    0,                                      //重发次数
                    0x00,                                   //应答帧
                    this,
                    tk_change_ask_idprocess,                   //接受回调函数
                    NULL,                                   //超时回调函数
                    NULL,                                   //发送条件回调函数
                }
        };
        pro_p->init_pro_frame(tkioinfo, sizeof(tkioinfo)/sizeof(CANPROHEAD));
        if(dev_para.type == TK200_INModule)
        {
            CANPROHEAD  tkininfo[] ={

                /*****************IN模块数据查询帧第四帧 0x677***********************************************************/
                        {
                            0,
                            0x677,                                  //帧id
                            0,                                      //超时时间
                            0,                                      //重发次数
                            0x00,                                   //应答帧
                            this,
                            tk_data_requireack_idprocess,                   //接受回调函数
                            NULL,                                   //超时回调函数
                            NULL,                                   //发送条件回调函数
                        },
                /*****************IN模块数据查询帧第5帧 0x678***********************************************************/
                        {
                            0,
                            0x678,                                  //帧id
                            0,                                      //超时时间
                            0,                                      //重发次数
                            0x00,                                   //应答帧
                            this,
                            tk_data_requireack_idprocess,                   //接受回调函数
                            NULL,                                   //超时回调函数
                            NULL,                                   //发送条件回调函数
                        },
                /*****************IN模块数据查询帧第6帧 0x679***********************************************************/
                        {
                            0,
                            0x679,                                  //帧id
                            0,                                      //超时时间
                            0,                                      //重发次数
                            0x00,                                   //应答帧
                            this,
                            tk_data_requireack_idprocess,                   //接受回调函数
                            NULL,                                   //超时回调函数
                            NULL,                                   //发送条件回调函数
                        },
                };
             pro_p->init_pro_frame(tkininfo, sizeof(tkininfo)/sizeof(CANPROHEAD));
        }
    }
    pro_p->init_pro_frame(canmidinfo, sizeof(canmidinfo)/sizeof(CANPROHEAD));
    attr.iostate = TK_IO_INIT;
    dev_node_pro_init();
    return 0;

}






































/***end*****/

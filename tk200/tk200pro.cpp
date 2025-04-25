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
#include "zprint.h"
#include "candatainfo.h"


#include <QDomDocument>
#include <QFile>
#include <QTextCodec>

#include <QTextStream>
#include <QXmlStreamWriter>
#include <QStringList>
// #include <iostream>
#include "zmap.h"
#include <QDebug>
#include "tk200pro.h"

void * tk200_reset_process(void * para)
{
    // int i = 0;
    TK200_Pro * tk200_pro_p = static_cast<TK200_Pro*>(para); //((TK200_Pro *)para);
    while(1){
        sem_wait(&gTk200_Rest_Meg);
        zprintf1("tk200 reset start!\n");
        int i;
        for(i = 0; i < 2; i++){
            tk200_pro_p->cs200[i].cs_dev_reset();
            tk200_pro_p->cs200[i].set_reset_data_init();
            tk200_pro_p->cs200[i].low_node_reset();

        }
        for(i = 0; i < 3; i++){
            tk200_pro_p->io200[i].io_dev_reset();
            tk200_pro_p->io200[i].io_dev_reset_data_init();
            tk200_pro_p->io200[i].set_dev_state(DEV_OFF_LINE);

        }
        tk200_pro_p->data_p->reset_data_value();
        tk200_pro_p->Reset_All();
    }
    return NULL;
}

int TK200_State_Mem::tk200_state_data_init(QString key)
{
    int i = 0;
    if(this->creat_data( 2*sizeof(CS_DataType)+ 3*sizeof(PT_Dev_State), key) == NULL)
        return -1;
    for(i = 0; i < 2; i++){
        this->father->cs200[i].data = (CS_DataType *)(this->m_data+i*sizeof(CS_DataType));
        this->father->cs200[i].mem = this;
    }
    for(i = 0; i < 3; i++){
        this->father->io200[i].data = ((PT_Dev_State *)(this->m_data+ 2*sizeof(CS_DataType))) +i;

        this->father->io200[i].mem = this;
    }
    return 0;
}

/*******************************************************************************************************************************
 **函数名： CanMsg card_controller_type::Reset_All()
 **输入：无
 **输出：无
 **功能描述：板卡控制类:发送全体复位
 **作者：
 **日期：20154.3.19
 **修改：
 **日期：
 **版本：
 ********************************************************************************************************************************/
void TK200_Pro::Reset_All()
{
//    qDebug("******打包全体复位真********");
    CANDATAFORM  reset_msg;

    tk200_send_fram(reset_msg, 0x0F, 0x1F);
    reset_msg.IDE = 0;
    reset_msg.RTR = 0;
    reset_msg.DLC = 0;

    pro_p->candrip->write_send_data(reset_msg);

}




int TK200_Pro::tk200_add_dev(PT_Dev_Virt * dev_p, uint8_t devoff)
{
    devmap.insert(pair<uint8_t,PT_Dev_Virt * >(devoff, dev_p));
    return 0;
}
int TK200_Pro::tk200_dev_init(void)
{
    PT_Dev_Virt * mid_pt = NULL;
    Reset_All();
    for(uint i = 0; i < devmap.size(); i++)
    {
        mid_pt = devmap[i];
        if(mid_pt->dev_para.enable == 1)
            mid_pt->pt_dev_init();
    }
    pthread_create(&reset_id, NULL, tk200_reset_process, (void *)this);
    return 0;
}

//void tk200_xml_config(void * pro, CAN_XML_DATA & xmldata)
//{
//    TK200_Pro * tk200p = (TK200_Pro *) pro;
//    for(int i = 0; i < xmldata.dev.size(); i++)
//    {
//        can_dev_para middevpara;
//        tk200p->ptread.dev_share_data_init(i, 0, xmldata.dev[i].para.innum);
//        middevpara = xmldata.dev[i].para;
//        tk200p->dev.push_back(middevpara);
//        switch (middevpara.type)
//        {
//            case TK200_CSModule:
//            {
//                TK200_CS * midcsdev = new TK200_CS(tk200p->canpro, &tk200p->ptread);
//                if(midcsdev != NULL)
//                {
//                    midcsdev->tk200_cs_config(xmldata.dev[i],i);
//                    tk200p->tk200_add_dev(midcsdev, i);
//                }
//                else
//                    printf("tk200 cs creat fail!\n");
//            }
//            break;
//            case TK200_IOModule:
//            case TK200_INModule:
//            {
//                TK_IO_Dev * midio = new TK_IO_Dev(tk200p->canpro, &tk200p->ptread);
//                if(midio != NULL)
//                {
//                    midio->tk_io_config(xmldata.dev[i],i);
//                    tk200p->tk200_add_dev(midio, i);
//                }
//                else
//                    printf("tk200 io creat fail!\n");
//            }
//            break;
//            default:
//                printf("dev type no exit!\n");
//            break;
//        }
//    }
//}

/***********************************************************************************
 * 函数名：tk200输出管理
 * 功能：tk200输出
 ***********************************************************************************/
int tk200_output(void * midp, soutDataUnit val)
{
//    timeprf("write out!\n\n");
    TK200_Pro * pro = static_cast<TK200_Pro *>(midp);
    if(MAP_IS_HAVE(pro->devmap, val.parentid-1))  //设备存在
    {
        pro->devmap[val.parentid-1]->data_send(val);
    }

    return 0;
}
//void TK200_Pro::tk200_pro_run(QString name)
//{
//    creat_can_bus_pro();
//    conf_func = tk200_xml_config;
//    can_read_xml(name);
//    can_app_init();
//    tk200_dev_init();
//    semwrite.pdata.z_pthread_init(tk200_output,this);

//}





























/***end*****/

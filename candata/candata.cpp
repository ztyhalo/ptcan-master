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
#include "candata.h"
#include "zprint.h"
#include "MsgMng.h"


#include <QDomDocument>
#include <QFile>
#include <QTextCodec>

#include <QTextStream>
#include <QXmlStreamWriter>
#include <QStringList>
// #include <iostream>
#include <QDebug>



Implement_Struct(can_bus_para_t)
Implement_Struct_Enum(can_bus_para_t)
Implement_Struct(can_dev_para_t)
Implement_Struct_Enum(can_dev_para_t)
Implement_Struct(can_256dev_para_t)
Implement_Struct_Enum(can_256dev_para_t)
Implement_Struct(can_dev_extra_para_t)
Implement_Struct_Enum(can_dev_extra_para_t)
Implement_Struct(can_inode_info_t)
Implement_Struct_Enum(can_inode_info_t)
Implement_Struct(can_onode_info_t)
Implement_Struct_Enum(can_onode_info_t)


int Can_Data::get_innode_info(int dev, int child, int innode, can_inode_info &val)
{
    if(dev > readxml.dev[0].child.size())
        return -1;
    CAN_DEV_INFO middev = readxml.dev[0].child[dev - 1];

    if(child > 0)
    {
        if(child > middev.child.size())
            return -2;
        if(innode > middev.child[child - 1].inode.size())
            return -3;
        val = middev.child[child - 1].inode[innode - 1];
    }
    else
    {
        if(innode > middev.inode.size())
            return -3;
        val = middev.inode[innode - 1];
    }

    return 0;
}

void Can_Data::creat_can_bus_pro(void)
{
    canbus = new CanDriver(1);
    canpro = new ncan_protocol();
}


int Can_Data::can_read_xml(QString name)
{
    read256xml.parentid = 0;
    readxml.driver_read_xml(name);
    memset(&d_info,0x00, sizeof(d_info));

    for(int i = 0; i < readxml.dev.size(); i++)
    {
        for(int j = 0; j < readxml.dev[i].child.size(); j++)
        {
            d_info.TotalInCnt += readxml.dev[i].child[j].para.innum;
            d_info.TotalOutCnt += readxml.dev[i].child[j].para.outnum;

            for(int k = 0; k < readxml.dev[i].child[j].child.size(); k++)
            {
                printf("exist 3 层！\n");
                d_info.TotalInCnt += readxml.dev[i].child[j].child[k].para.innum;
                d_info.TotalOutCnt += readxml.dev[i].child[j].child[k].para.outnum;
            }
        }
    }

    zprintf3("d_info.TotalInCnt = %d,d_info.TotalOutCnt = %d\n\n",d_info.TotalInCnt,d_info.TotalOutCnt);
    if(readxml.dri_info.protocal == "1030"){
        d_info.TotalStateCnt = MAX_STATE_BUF_SIZE;
    } else if(readxml.dri_info.protocal == "tk200"){
        d_info.TotalStateCnt = TK200_STATE_BUF_SIZE;
    }
    canid = readxml.dri_info.canid;
    m_bitrate = readxml.dri_info.brate;

    zprintf3("state is %d %d\n",d_info.TotalStateCnt, MAX_STATE_BUF_SIZE);
    QString ckey = QString("%1").arg(devkey.data_key);
    zprintf3("in is %d\n",d_info.TotalInCnt);
    ptread.creat_pt_share((d_info.TotalInCnt + d_info.TotalOutCnt)*sizeof(sDataUnit), ckey);
    QString qtkey = QString("%1").arg(devkey.ctrl_key);
    zprintf3("out is %d\n",d_info.TotalOutCnt);

    semwrite.pdata.creat_sem_data(CON_OUT_BUF_SIZE * sizeof(soutDataUnit), devkey.sem_key, qtkey, ZQTShareMem::Create);
    ptread.dri_id = devkey.driverid;


    if(readxml.dri_info.protocal == "tk100" || readxml.dri_info.protocal == "1030")
    {
        cs1030 = new cs_can(canpro, QString("%1").arg(devkey.state_key), readxml.dri_info.auto_reset);
        if(readxml.dri_info.protocal == "tk100")
            cs1030->tk100io = new TK_IO_Dev(canpro, &ptread);
        cs1030->data_p = &ptread;
        cs1030->state_info.set_config_Slave_IO_Set(0, readxml.dev.size() ? readxml.dev.size() - 1 : readxml.dev.size());
        cs1030->state_info.set_config_Auto_Reset(0, readxml.dri_info.auto_reset);
    }
    else if(readxml.dri_info.protocal == "tk200")
    {
        tk200 = new TK200_Pro(canpro, &ptread, QString("%1").arg(devkey.state_key));
    }
    else
    {
        ;
    }

    MsgMng  *pMsgMng = MsgMng::GetMsgMng();

    if(!pMsgMng->Init(devkey.recvmsg_key, devkey.sendmsg_key, this))
    {
        zprintf1("DriverTest msg init fail!\n");
    }
    else
    {
        zprintf1("DriverTest msg init over!\n");
    }
    for(int i = 0; i < readxml.dev.size(); i++)
    {
        for(uint8_t j = 0; j < readxml.dev[i].child.size(); j++)
        {
            can_dev_para middevpara;
            ptread.dev_share_data_init(j, 0, readxml.dev[i].child[j].para.innum, readxml.dev[i].child[j].para.outnum);
            middevpara = readxml.dev[i].child[j].para;
            printf("dev%d name %s type %d!\n", middevpara.id, middevpara.name.c_str(), middevpara.type);
            dev.push_back(middevpara);
            switch (middevpara.type)
            {
            case TK100_IOModule_IO:
            {
                zprintf3("tk100 io config\n");
                cs1030->tk100io->tk_io_config(readxml.dev[i].child[j],j);
            }
            break;
            case TK100_CSModule:
            case TK100_IOModule_Salve:
            case CS_DEV:             //添加cs设备
            {
                cs1030->pt_configdata_set(readxml.dev[i].child[j], j);
            }
            break;
            case TK100_BS_Module:
            {
                cs1030->bs100info.dev_off = j;
            }
            break;
            case TK200_CSModule:
            {
                if(readxml.dev[i].child[j].para.id == 2 || readxml.dev[i].child[j].para.id == 3)
                {
                    //                    if(readxml.dev[i].para.enable)
                    tk200->cs200[readxml.dev[i].child[j].para.id -2].tk200_cs_config(readxml.dev[i].child[j],j);
                }
            }
            break;
            case TK200_IOModule:
            case TK200_INModule:
            {
                //                if(readxml.dev[i].para.enable)
                {
                    if(readxml.dev[i].child[j].para.id == 5 || readxml.dev[i].child[j].para.id == 6
                        || readxml.dev[i].child[j].para.id == 7)
                        tk200->io200[readxml.dev[i].child[j].para.id -5].tk_io_config(readxml.dev[i].child[j],j);
                }
            }
            break;
            default:
                zprintf1("dev type no exit!\n");
                break;
            }
        }
    }

    return true;
}
int Can_Data::can_read_xml(const QString  name, const QString  name1, const QString  name2)
{
    int zjcs[3][2]   = { {0, 0}, {1, 2}, {1, 3 }};
    read256xml.parentid = 0;
    read256xml.driver_read_xml(name);
    QFile file1(name1);
    if(file1.exists())
    {
        read256xml.parentid++;
        read256xml.driver_read_xml(name1);
    }
    QFile file2(name2);
    if(file2.exists())
    {
        read256xml.parentid++;
        read256xml.driver_read_xml(name2);
    }

    memset(&d_info, 0x00, sizeof(d_info));
    for(int i = 0; i < read256xml.dev.size(); i++)
    {
        for(int j = 0; j < read256xml.dev[i].child.size(); j++)
        {
            read256xml.dev[i].child[j].para.id += i * 0x100;
            d_info.TotalInCnt += read256xml.dev[i].child[j].para.innum;
            d_info.TotalOutCnt += read256xml.dev[i].child[j].para.outnum;
        }
        //增加沿线所有闭锁按钮状态
        d_info.TotalInCnt += 255;
    }

    if(read256xml.dri_info.protocal == "1030")
    {
        d_info.TotalStateCnt = MAX_STATE_BUF_SIZE * 3 /*(readxml.dev.size() ? readxml.dev.size() : 1)*/;
    }
    else if(read256xml.dri_info.protocal == "tk200")
    {
        d_info.TotalStateCnt = TK200_STATE_BUF_SIZE;
    }
    canid = read256xml.dri_info.canid;
    m_bitrate = read256xml.dri_info.brate;

    QString ckey = QString("%1").arg(devkey.data_key);
    zprintf3("dev config: line all in io num is < %d >, line all out io num is < %d >\n", d_info.TotalInCnt - 255, d_info.TotalOutCnt);
    zprintf3("key: io in key = %d\r\n", devkey.data_key);
    zprintf3("key: io out key = %d\r\n", devkey.ctrl_key);
    ptread.creat_pt_share((d_info.TotalInCnt + d_info.TotalOutCnt) * sizeof(sDataUnit), ckey);
    QString qtkey = QString("%1").arg(devkey.ctrl_key);
    semwrite.pdata.creat_sem_data(CON_OUT_BUF_SIZE * sizeof(soutDataUnit), devkey.sem_key, qtkey, ZQTShareMem::Create);
    ptread.dri_id = devkey.driverid;

    if(read256xml.dri_info.protocal == "tk100" || read256xml.dri_info.protocal == "1030")
    {
        cs256 = new cs_can256(canpro, QString("%1").arg(devkey.state_key), (read256xml.dev.size() ? read256xml.dev.size() : 1), read256xml.dri_info.auto_reset);
        if(read256xml.dri_info.protocal == "tk100")
            cs256->tk100io = new TK_IO_Dev(canpro, &ptread);
        cs256->data_p = &ptread;
    }
    else if(read256xml.dri_info.protocal == "tk200")
    {
        tk200 = new TK200_Pro(canpro, &ptread, QString("%1").arg(devkey.state_key));
    }
    else
    {
        ;
    }

    MsgMng *pMsgMng = MsgMng::GetMsgMng();

    if(!pMsgMng->Init(devkey.recvmsg_key, devkey.sendmsg_key, this))
    {
        zprintf1("DriverTest msg init fail!\n");
    }
    else
    {
        zprintf3("DriverTest msg init over!\n");
    }
    cs256->cf_cs_num[0] = 0;
    cs256->cf_cs_num[1] = 0;
    cs256->cf_cs_num[2] = 0;
    for(uint8_t i = 0; i < read256xml.dev.size(); i++)
    {
        for(uint8_t j = 0; j < read256xml.dev[i].child.size(); j++)
        {
            can_dev_para middevpara;
            ptread.dev_share_data_init(j, i, read256xml.dev[i].child[j].para.innum, read256xml.dev[i].child[j].para.outnum);
            middevpara = read256xml.dev[i].child[j].para;
            dev.push_back(middevpara);
            switch(middevpara.type)
            {
                case TK100_256_IOModule_IO:
                {
                    zprintf3("tk100 io config\n");
                    cs256->tk100io->tk_io_config(read256xml.dev[i].child[j], i);
                }
                break;
                case CS_256_DEV:    //添加cs设备
                    cs256->cf_cs_num[i]++;
                case DEV_256_IO_PHONE:
                case DEV_256_MODBUS_LOCK:
                case DEV_256_PHONE:
                case DEV_256_LOCK:
                case TK236_IOModule_Salve:
                case DEV_256_RELAY:
                case TK100_256_CSModule:
                case TK200_256_CSModule:
                {
                    cs256->pt_configdata_set(read256xml.dev[i].child[j], i, j);
                }
                break;
                case TK100_256_BS_Module:
                {
                    cs256->bs100info.dev_off = i;
                }
                break;
                case TK200_256_IOModule:
                case TK200_256_INModule:
                {
                    {
                        if(read256xml.dev[i].para.id == 5 || read256xml.dev[i].para.id == 6
                           || read256xml.dev[i].para.id == 7)
                            tk200->io200[read256xml.dev[i].para.id - 5].tk_io_config(read256xml.dev[i], i);
                    }
                }
                break;

                default:
                    zprintf1("dev type no exit!\n");
                    break;
            }
        }
        //增加沿线所有闭锁按钮状态
        ptread.dev_share_data_init(read256xml.dev[i].child.size(), i, 255, 0);
    }

    for(int i = 0; i < 3; i++)
    {
        cs256->add_mac_cszd_branch(zjcs[i][0], zjcs[i][1]);
    }

    cs256->line_num = 1;
    if(file1.exists())
    {
        cs256->line_num |= 0x02;
    }
    if(file2.exists())
    {
        cs256->line_num |= 0x04;
    }
    return true;
}

int Can_Data::can_app_init(void)
{

    canbus->can_bus_init(canid - 1, m_bitrate);
    canpro->ncan_pro_init(canbus);

    canpro->start();
    canbus->start("can bus");

    if(cs256 != NULL)
    {
        cs256->cs_config_init();
        if(cs256->tk100io != NULL)
        {
            cs256->tk100io->pt_dev_init();
            semwrite.pdata.z_pthread_init(tk100_output, cs256->tk100io, "tk100 output");
        }
        else
        {
            semwrite.pdata.z_pthread_init(dev256_output, cs256, "256 output");
        }
    }

    if(cs1030 != NULL)
    {
        cs1030->cs_config_init();
        if(cs1030->tk100io != NULL)
        {
            cs1030->tk100io->pt_dev_init();
            semwrite.pdata.z_pthread_init(tk100_output, cs1030->tk100io, "tk100 output");
        }
        else
        {
            semwrite.pdata.z_pthread_init(dev1030_output, cs1030, "1030 output");
        }
    }

    if(tk200 != NULL)
    {
        tk200->tk200_dev_init();
        semwrite.pdata.z_pthread_init(tk200_output, tk200);
    }
    return 0;
}





/***end*****/

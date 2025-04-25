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

#include "ptrwdatainfo.h"


using namespace std;


void Pt_ShareData::set_pt_data(sDataUnit * add, double val)
{
    if((add -this->m_data) >= (int)(this->m_size/sizeof(sDataUnit)))
    {
        zprintf1("set data off add 0x%x data 0x%x  %d %d\n", add, this->m_data,
                 add -this->m_data, this->m_size/sizeof(sDataUnit));
        return;
    }
    lock_qtshare();
    add->value = val;
    unlock_qtshare();
}

void Pt_ShareData::set_pt_out_data(sDataUnit * add, double val, int st)
{
    if((add -this->m_data) >= (int)(this->m_size/sizeof(sDataUnit)))
    {
        zprintf1("set data off add 0x%x data 0x%x  %d %d\n", add, this->m_data,
                 add -this->m_data, this->m_size/sizeof(sDataUnit));
        return;
    }
    lock_qtshare();
    add->value = val;
    add->state = st;
    unlock_qtshare();
}

void PRO_ShreDATA::creat_pt_share(int size, QString keyid)
{
    qtread.pdata.creat_data(size, keyid);
}

void PRO_ShreDATA::dev_share_data_init(int devid, int childid,int innum)
{
    int devoff = devid + childid *FATHER_DEV_MAX;
    qtread.add_node_dev(devoff, innum);
    sDataUnit midval = {dri_id,devid+1,childid,1,0, OUT_NODE_IDLE};
    for(int i = 1; i <= innum; i++)
    {
        midval.pointid = i;
        qtread.set_dev_invalue(devoff, i-1, midval);
    }
}

void PRO_ShreDATA::set_share_data_value(int devid, int childid,int innode,double val)
{
    int devoff = devid + childid *FATHER_DEV_MAX;
    sDataUnit midval = {dri_id,devid+1,childid,innode,val, OUT_NODE_IDLE};
    qtread.set_dev_invalue(devoff, innode-1, midval);
}


void Pt_Devs_ShareData::creat_pt_share(int size, QString keyid)
{
    buf_size = size/sizeof(sDataUnit);
    zprintf3(" pt share size = %d \r\n");
    this->mapdata_p = creat_data(size, keyid);
    add_p = this->mapdata_p;
    zprintf3(" pt share size = %d, add_p = %x\r\n", buf_size, add_p);
}
void Pt_Devs_ShareData::reset_data_value(void)
{


    if(add_p != NULL)
    {
        sDataUnit * valp = add_p;
        for(int i = 0; i < buf_size; i++)
        {
            valp->value = 0;
            valp++;
        }
    }
}

void Pt_Devs_ShareData::set_pt_node_val(int devid, int innum, sDataUnit val)
{
    set_data(get_dev_node_addr(devid, innum), val);
}

void Pt_Devs_ShareData::dev_share_data_init(int devid, int childid, int innum, int outnum)
{
    int i;
    int devoff = devid + childid * FATHER_DEV_MAX;

    add_node_dev(devoff, innum + outnum);
    get_dev_node_addr(devoff, 1);

    sDataUnit midval = { 1, devid + 1, childid, 1, 0, OUT_NODE_IDLE};
    for(i = 1; i <= innum; i++)
    {
        midval.pointid = i;
        set_pt_node_val(devoff, i-1, midval);
    }
    for(i = innum+1; i <= innum+outnum; i++)
    {
        midval.num = 2;
        midval.pointid = i - innum;
        set_pt_node_val(devoff, i-1, midval);
    }
}

void Pt_Devs_ShareData::set_share_data_value(int devid, int childid,int innode,double val)
{
    int devoff = devid + childid * FATHER_DEV_MAX;
    set_pt_data(get_dev_node_addr(devoff, innode-1), val);
}

void Pt_Devs_ShareData::set_out_ack_value(int devid, int childid,int innode,double val)
{
    int devoff = devid + childid *FATHER_DEV_MAX;
    set_pt_out_data(get_dev_node_addr(devoff, innode-1), val, OUT_NODE_IDLE);
}


void IO_ShareData::creat_pt_share(int size, QString keyid)
{
    dev_map_init((char *)creat_data(size, keyid));
}
void IO_ShareData::reset_data_value(void)
{
    cDataUnit * valp;
    s_info devinfo = get_info();
    valp = devinfo.p;

    if(valp != NULL)
    {
        lock_qtshare();
        for(int i = 0; i < devinfo.sz; i++, valp++)
        {
            valp->set_val_state(0, OUT_NODE_IDLE);
        }
        unlock_qtshare();
    }
}

void IO_ShareData::reset_dev_value(int devid, int childid)
{
    s_info devinfo;
    if(get_dev_info(get_dev_id(devid, childid), devinfo))
    {
        return;
    }
    lock_qtshare();
    cDataUnit * init_p = devinfo.p;
    for(int i = 0; i < devinfo.sz; i++, init_p++)
    {
        init_p->set_val_state(0, OUT_NODE_IDLE);
    }
    unlock_qtshare();
}

cDataUnit * IO_ShareData::get_node_addr(int devid, int node)
{
    s_info devinfo;
    if(get_dev_info(devid, devinfo))
    {
        return NULL;
    }
    if(node > devinfo.sz)
    {
        return NULL;
    }
    return devinfo.p+node-1;

}

void IO_ShareData::set_pt_node_val(int devid, int innum, cDataUnit val)
{
    this->set_data(get_node_addr(devid, innum), val);
}


void IO_ShareData::dev_share_data_init(int devid, int childid, int innum, int outnum)
{
    int i;
    int devoff = devid + childid *FATHER_DEV_MAX;
    add_dev(devoff, innum + outnum);

    cDataUnit * init_p = get_dev_addr(devoff);

    lock_qtshare();
    cDataUnit midval(1, devid , childid , 1, 0, OUT_NODE_IDLE);
    for(i = 1; (i <= innum)&&(init_p != NULL); i++, init_p++)
    {
        midval.pointid = i;
        init_p->set_all(midval);
    }
    for(i = innum+1; (i <= innum+outnum)&&(init_p != NULL); i++, init_p++)
    {
        midval.num = 2;
        midval.pointid = i - innum;
        init_p->set_all(midval);
    }
    unlock_qtshare();
}

void IO_ShareData::set_share_data_value(int devid, int childid,int innode,double val)
{
    int devoff = devid + childid *FATHER_DEV_MAX;
    cDataUnit * p = get_node_addr(devoff, innode);
    if(p != NULL)
    {
        lock_qtshare();
        p->set_val(val);
        unlock_qtshare();
    }
}


void IO_ShareData::set_out_ack_value(int devid, int childid,int innode,double val)
{
    int devoff = devid + childid *FATHER_DEV_MAX;
    cDataUnit * p = get_node_addr(devoff, innode);
    if(p != NULL)
    {
        lock_qtshare();
        p->set_val_state(val, OUT_NODE_IDLE);
        unlock_qtshare();
    }
}

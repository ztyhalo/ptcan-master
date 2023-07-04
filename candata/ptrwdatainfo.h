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
#ifndef __PTRWDATAINFO_H__
#define __PTRWDATAINFO_H__



#include "can_protocol.h"
#include "bitset"
#include "reflect.h"
#include "driver.h"
#include "ptxml.h"
#include "ptdataapp.h"


#define FATHER_DEV_MAX      256
using namespace std;


enum
{
    OUT_NODE_IDLE = 0x00,
    OUT_NODE_USING
};

typedef struct
{
    int num;
    int parentid;
    int childid;
    int pointid;
    double value;
    int  state;
}sDataUnit;     //主动上报共享内存

class cDataUnit
{
public:
    int num;
    int parentid;
    int childid;
    int pointid;
    double value;
    int  state;
public:
    cDataUnit(int  mark, int devid, int cid, int node, double val, int  st):
        num(mark),parentid(devid), childid(cid),pointid(node),value(val), state(st)
    {
        ;
    }
    cDataUnit()
    {
        ;
    }
    void set_all(cDataUnit val)
    {
        *this = val;
    }
    void set_val(double val)
    {
        value = val;
    }
    void set_val_state(double val, int st)
    {
        value = val;
        state = st;
    }
};

typedef struct
{
    int num;
    int parentid;
    int childid;
    int pointid;
    double value;
    int  state;
}soutDataUnit;


class Pt_ShareData:public QTShareDataT<sDataUnit>
{
public:
    void set_pt_data(sDataUnit * add, double val);
    void set_pt_out_data(sDataUnit * add, double val, int st);
};

class Pt_Devs_ShareData:public Pt_ShareData,public DATAS_Map_T<sDataUnit>
{
public:
    int         dri_id;
    int         buf_size;
    sDataUnit * add_p;
public:
    Pt_Devs_ShareData(int dri = 1){
        dri_id = dri;
        buf_size = 0;
        add_p = NULL;
    }
    ~Pt_Devs_ShareData(){
       zprintf3("destory Pt_Devs_ShareData!\n");

    }
    void creat_pt_share(int size, QString keyid);
    void set_pt_node_val(int devid, int innum, sDataUnit val);
    void dev_share_data_init(int devid, int childid,int innum, int outnum);
    void set_share_data_value(int devid, int childid,int innode,double val);
    void set_out_ack_value(int devid, int childid,int innode,double val);
    void reset_data_value(void);
    template <class DTYPE>
    void set_data_value(int devid, int childid,int s,int size,DTYPE * val)
    {
        zprintf3("continue set %d %d\n", s,size);
        for(int i = 0; i < size; i++){
            set_share_data_value(devid, childid, s+i, *(val+i));
        }
    }
};

class PRO_ShreDATA
{
public:
    int dri_id;
    QTS_DATAS_T<sDataUnit> qtread;
public:
    PRO_ShreDATA(int dri = 1){
        dri_id = dri;
    }
    void creat_pt_share(int size, QString keyid);
    void dev_share_data_init(int devid, int childid,int innum);
    void set_share_data_value(int devid, int childid,int innode,double val);
    template <class DTYPE>
    void set_data_value(int devid, int childid,int s,int size,DTYPE * val){
        int devoff = devid + childid *FATHER_DEV_MAX;
        sDataUnit midval = {dri_id, devid+1, childid, s, 0 };
        for(int i = 0; i < size; i++){
            midval.pointid = s+i;
            midval.value = *(val+i);
            qtread.set_dev_invalue(devoff, s+i-1, midval);
        }
    }
};

class IO_ShareData:public QT_Share_MemT<cDataUnit>,public Dev_Map_T<cDataUnit>
{
public:
    int         dri_id;
public:
    IO_ShareData(int dri = 1){
        dri_id = dri;
    }
    ~IO_ShareData(){
       zprintf3("destory Pt_Devs_ShareData!\n");

    }

    int get_dev_id(int devid, int childid)
    {
        return  devid + childid *FATHER_DEV_MAX;
    }

    void creat_pt_share(int size, QString keyid);
    cDataUnit * get_node_addr(int devid, int node);


    void set_pt_node_val(int devid, int innum, cDataUnit val);
    void dev_share_data_init(int devid, int childid,int innum, int outnum);
    void set_share_data_value(int devid, int childid,int innode,double val);
    void set_out_ack_value(int devid, int childid,int innode,double val);
    void reset_data_value(void);
    void reset_dev_value(int devid, int childid);
};


template <class F>
class PRO_DATA_INFO_T
{
public:

    Pt_Devs_ShareData               ptread;
//     IO_ShareData               ptread;
    SemS_QtDATAS_T<soutDataUnit, F>    semwrite;
public:
    PRO_DATA_INFO_T(int dri = 1){
        ptread.dri_id = dri;
    }
    ~PRO_DATA_INFO_T(){
         zprintf3("destory PRO_DATA_INFO_T!\n");
    }
};








#endif /*__PTRWDATAINFO_H__*/

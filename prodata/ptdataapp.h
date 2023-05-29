/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.1
 *History: 2017.7.3
 *         2017.7.6添加回调类
 *         2017.7.11 将1个文件拆分为多个文件 此文件为针对pt的数据组装
****************************************************/
#ifndef __PTDATAAPP_H__
#define __PTDATAAPP_H__
#include "semshare.h"
//多个设备共用一个共享空间 PTYPE共享内存类型 DTYPE数据类型
template <class PTYPE, class DTYPE>
class DATAS_T
{
public:
    PTYPE                           pdata;
    map<uint, creatdata<DTYPE> >    devrdinfo;
public:
    ~DATAS_T(){
        zprintf3("destory DATAS_T!\n");
    }

    int add_dev(uint devid, creatdata<DTYPE> in);
    int add_node_dev(uint devid, uint node);
    int set_dev_invalue(uint devid, uint in ,DTYPE val);
    int get_dev_invalue(uint devid, uint in ,DTYPE & val);
};

template <class PTYPE, class DTYPE>
int DATAS_T<PTYPE, DTYPE>::add_dev(uint devid, creatdata<DTYPE> in)
{
    devrdinfo.insert(pair<uint, creatdata<DTYPE> >(devid, in));
    return 1;
}

template <class PTYPE, class DTYPE>
int DATAS_T<PTYPE, DTYPE>::set_dev_invalue(uint devid, uint in ,DTYPE val)
{
    typename map<uint, creatdata<DTYPE> >::iterator proiter;

    proiter = devrdinfo.find(devid);

    if(proiter != devrdinfo.end())
    {
        pdata.set_data((proiter->second.data - pdata.data) +in, val);
        return 0;
    }
    printf("set fail\n");
    return -1;
}

template <class PTYPE, class DTYPE>
int DATAS_T<PTYPE, DTYPE>::get_dev_invalue(uint devid, uint in ,DTYPE & val)
{
    typename map<uint, creatdata<DTYPE> >::iterator proiter;

    proiter = devrdinfo.find(devid);

    if(proiter != devrdinfo.end())
    {
        return pdata.get_data((proiter->second.data-pdata.data)+in, val);
    }
    return -1;
}

template <class PTYPE, class DTYPE>
int DATAS_T<PTYPE, DTYPE>::add_node_dev(uint devid, uint node)
{
      creatdata<DTYPE> mid;

      mid.data_init(pdata.addp, node * sizeof(DTYPE));

      pdata.addp += node;

      return add_dev(devid, mid);
}
template <class DTYPE>
class DATAS_Map_T
{
public:
    DTYPE *                         mapdata_p;
    map<uint, creatdata<DTYPE> >    devrdinfo;
public:
    int add_dev(uint devid, creatdata<DTYPE> in);
    int add_node_dev(uint devid, uint node);
    DTYPE * get_dev_node_addr(uint devid, uint in);
};

template <class DTYPE>
int DATAS_Map_T<DTYPE>::add_dev(uint devid, creatdata<DTYPE> in)
{
    devrdinfo.insert(pair<uint, creatdata<DTYPE> >(devid, in));
    return 1;
}

template <class DTYPE>
DTYPE * DATAS_Map_T<DTYPE>::get_dev_node_addr(uint devid, uint in)
{
    typename map<uint, creatdata<DTYPE> >::iterator proiter;

    proiter = devrdinfo.find(devid);

    if(proiter != devrdinfo.end())
    {
        return(proiter->second.data +in);
    }
    return NULL;
}

template <class DTYPE>
int DATAS_Map_T<DTYPE>::add_node_dev(uint devid, uint node)
{
      creatdata<DTYPE> mid(mapdata_p, node * sizeof(DTYPE));
      mapdata_p += node;
      return add_dev(devid, mid);
}
//qt类型的多个设备共享空间
template <class DTYPE>
class QTS_DATAS_T:public DATAS_T<QTShareDataT<DTYPE>, DTYPE>
{

};

//linux共享内存加信号类型的多个设备数据组合
template <class DTYPE, class FAT>
class SemS_DATAS_T:public DATAS_T<Sem_Pth_Data<DTYPE, FAT>, DTYPE>
{

};

//qt共享内存加linux信号类型的多个设备数据组合
template <class DTYPE, class FAT>
class SemS_QtDATAS_T:public DATAS_T<Sem_QtPth_Data<DTYPE, FAT>, DTYPE>
{
public:
    ~SemS_QtDATAS_T(){
       zprintf3("destory SemS_QtDATAS_T!\n");
    }
};


template <class DTYPE>
class Dev_Map_T
{
public:
typedef    struct {
        DTYPE * p;
        int     sz;
    }s_info;
private:
    char  *                 end_p;
    char  *                 start_p;
    int                     allsize;
    map<uint,  s_info  >    devinfo;
public:
    Dev_Map_T()
    {
        end_p = NULL;
        allsize = 0;
        start_p = NULL;
    };
    void dev_map_init(char * s_p)
    {
        end_p = s_p;
        start_p = s_p;
    }

    int add_dev(uint devid,  int size)
    {
        if(end_p == NULL)
        {
            zprintf3("Add dev failed!\n");
            return 1;
        }
        s_info midval;
        midval.p = (DTYPE *)end_p;
        midval.sz = size;
        allsize += size;
        devinfo.insert(pair<uint, s_info>(devid, midval));
        end_p += size*sizeof(DTYPE);
        return 0;
    }

    DTYPE * get_dev_addr(uint devid)
    {
        typename map<uint,  s_info >::iterator proiter;

        proiter = devinfo.find(devid);

        if(proiter != devinfo.end())
        {
            return(proiter->second.p);
        }
        zprintf3("Dev_Map_T find  %d fail!\n", devid);
        return NULL;
    }
    int  get_dev_size(uint devid)
    {
        typename map<uint,  s_info >::iterator proiter;

        proiter = devinfo.find(devid);

        if(proiter != devinfo.end())
        {
            return(proiter->second.sz);
        }
        zprintf3("Dev_Map_T find  %d fail!\n", devid);
        return -1;
    }

    int  get_dev_info(uint devid, s_info & info)
    {
        typename map<uint,  s_info >::iterator proiter;

        proiter = devinfo.find(devid);

        if(proiter != devinfo.end())
        {
           info = proiter->second;
           return 0;
        }
        zprintf3("Dev_Map_T find  %d fail!\n", devid);
        return 1;
    }

    s_info  get_info(void)
    {
        s_info val;
        val.p = (DTYPE *)start_p;
        val.sz = allsize;
        return val;
    }
};


#endif /*__PTDATAAPP_H__*/

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
#ifndef __TK200PRO_H__
#define __TK200PRO_H__

#include "can_protocol.h"
// #include "pro_data.h"
// #include "bitset"
// #include "reflect.h"
// #include "driver.h"
// #include "ptxml.h"
// #include "candatainfo.h"
#include "tk200cs.h"
#include "1030common.h"
#include <pthread.h>


using namespace std;
class TK200_Pro;


class TK200_State_Data
{
public:
    CS_DataType   cs_state[2];
    PT_Dev_State  io_state[3];
};

class TK200_State_Pro
{
public:
    Max_State_Data                    dev_state;
    QT_Share_MemT<TK200_State_Data>   share_state;

};

#define TK200_STATE_BUF_SIZE (sizeof(TK200_State_Data))

class TK200_State_Mem:public QT_Share_MemT<char>
{
public:
   TK200_Pro * father;
public:
      TK200_State_Mem(TK200_Pro * fp = NULL){
          father = fp;
      }

      ~TK200_State_Mem(){
          zprintf3("destory TK200_State_Mem!\n");
      }
      int tk200_state_data_init(QString key);
};






class TK200_Pro
{

public:

    string                       filename;
    pthread_t                    reset_id;
    TK200_CS                     cs200[2];
    TK_IO_Dev                    io200[3];
    map<uint8_t, PT_Dev_Virt *>  devmap;
    ncan_protocol  *       pro_p;
    Pt_Devs_ShareData   *  data_p;
    TK200_State_Mem        tk_mem;



public:
    TK200_Pro(ncan_protocol * pro, Pt_Devs_ShareData *data, const QString & key):pro_p(pro),data_p(data)
    {
        // pro_p = pro;
        // data_p = data;
        int i;
        for(i = 0; i < 2; i++)
        {
            cs200[i].pt_dev_virt_init(pro, data, TK200_CSModule);
            tk200_add_dev(&cs200[i], i);
        }
        for(i = 0; i < 2; i++)
        {
            io200[i].pt_dev_virt_init(pro, data, TK200_IOModule);
            tk200_add_dev(&io200[i], i+2);
        }
        io200[2].pt_dev_virt_init(pro, data, TK200_INModule);
        tk200_add_dev(&io200[2],4);
        tk_mem.father = this;
        tk_mem.tk200_state_data_init(key);


    }
    int tk200_add_dev(PT_Dev_Virt * dev_p, uint8_t devoff);
    int tk200_dev_init(void);
    void Reset_All(void);
    ~TK200_Pro(){
        Reset_All();
        if(reset_id > 0){
            zprintf3("destory tk200 pthread!\n");
            pthread_cancel(reset_id);
            pthread_join(reset_id, NULL);
            reset_id = 0;
        }
    }
    void tk200_pro_run(QString name);
};

int tk200_output(void * midp, soutDataUnit val);
#endif /*__TK200PRO_H__*/

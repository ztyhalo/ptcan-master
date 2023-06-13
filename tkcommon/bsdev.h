#ifndef __BSDEV_H__
#define __BSDEV_H__

#include <stdint.h>
#include "run_mode.h"
#include <string>
#include "candatainfo.h"

using namespace std;
#define BS_MACHINE_NUM      128

typedef union
{
    struct
    {
        uint16_t stop1:1;
        uint16_t zs1:1;
        uint16_t pp1:1;
        uint16_t ppsty1:1;
    }bsbit;
    uint16_t bsvalue;
}BSINFO;
class BS_Dev
{
public:
    uint8_t dev_off;
    uint8_t config_num;    //配置的闭锁个数
    uint8_t bsnum;         //闭锁总个数
    uint8_t bsednum;       //已经拍下闭锁个数
    uint8_t zdstate;
    uint8_t jtstate;
    uint8_t powerstate;
    uint8_t zsstate;
    uint8_t ppstate;
    uint8_t zsnum;
    BSINFO polldata[BS_MACHINE_NUM];
    bitset<BS_MACHINE_NUM>    bs;
    bitset<BS_MACHINE_NUM>    zs;
    bitset<BS_MACHINE_NUM>    pp;
    bitset<BS_MACHINE_NUM>    ppsty;
public:
    BS_Dev(){
        memset(this,0x00, offsetof(BS_Dev,bs));
    }
    void save_poll_data(int addr, int size,uint8_t * data);
    void poll_data_process(void);
    void set_bs_info(int num, BSINFO info)
    {
        bs[num] = info.bsbit.stop1;
        zs[num] = info.bsbit.zs1;
        pp[num] = info.bsbit.pp1;
        ppsty[num] = info.bsbit.ppsty1;
    }

    void data_process(void){
        ppstate = pp.any();
        zsstate = zs.any();
        zsnum = zs.count();
        bsednum = bs.count();
        zprintf3("bs ing %d\n",bsednum);
    }

};
#endif // __BSDEV_H__


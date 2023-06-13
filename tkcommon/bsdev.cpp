#include "bsdev.h"




void BS_Dev::save_poll_data(int addr, int size, uint8_t * data)
{
    if(addr+size > BS_MACHINE_NUM)
        size = BS_MACHINE_NUM -addr;
    memcpy(&polldata[addr/2],data,size);

}
void BS_Dev::poll_data_process(void)
{
    for(int i = 0; i < bsnum; i++)
    {
        set_bs_info(i, polldata[i]);
    }
    data_process();
}

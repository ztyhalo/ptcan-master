#include "can_bus.h"
#include <stdio.h>
#include "timers.h"
// #include <sstream>
// #include "netprint.h"

using namespace std;

int sigsize = 0;



//CanBusMag  CanBusMag::canpoint;



CanFrame prodata_to_lawdata(const CANDATAFORM & f)
{
    CanFrame cansend;
    memset(&cansend, 0 , sizeof(CanFrame));

    if(f.IDE)
    {
        cansend.can_id = (f.IDE<<31)|(f.RTR<<30)|(f.ExtId);
    }
    else
    {
        cansend.can_id = (f.RTR<<30)|(f.StdId);
    }

    cansend.can_dlc = f.DLC;

    memcpy(cansend.data, f.Data, cansend.can_dlc);

    return cansend;

}

CANDATAFORM lawdata_to_prodata(const CanFrame & f)
{
    CANDATAFORM rxcan;

    memset(&rxcan, 0x00, sizeof(CANDATAFORM));
    rxcan.IDE = (f.can_id>>31)&1;
    rxcan.RTR = (f.can_id>>30)&1;
    if(rxcan.IDE)
    {
        rxcan.ExtId = f.can_id&0x1FFFFFFF;
    }
    else
    {
       rxcan.StdId = f.can_id&0x7FF;
    }

    if(rxcan.RTR&1)
    {
       rxcan.DLC = 0;
    }
    else
    {
       rxcan.DLC = f.can_dlc;
       memcpy(rxcan.Data, f.data, rxcan.DLC);
    }
    return rxcan;
}

CANDATAFORM lawdata_to_prodata(CanFrame * f)
{
    CANDATAFORM rxcan;

    memset(&rxcan, 0x00, sizeof(CANDATAFORM));
    rxcan.IDE = (f->can_id>>31)&1;
    rxcan.RTR = (f->can_id>>30)&1;
    if(rxcan.IDE)
    {
        rxcan.ExtId = f->can_id&0x1FFFFFFF;
    }
    else
    {
       rxcan.StdId = f->can_id&0x7FF;
    }

    if(rxcan.RTR&1)
    {
       rxcan.DLC = 0;
    }
    else
    {
       rxcan.DLC = f->can_dlc;
       memcpy(rxcan.Data, f->data, rxcan.DLC);
    }
    return rxcan;
}


int  call_write_back(CanDriver * pro, const CANDATAFORM data)
{

        pro->writeframe(data);
//        linuxDly(pro->interval);
        return 0;
}

 int CanDriver::can_bus_init(int registerdev, int brate)
 {
     int  ret;
     struct sockaddr_can addr;
     struct ifreq ifr;
     char canname[6];
     // ostringstream canset;
//     canset << "/opt/canbrateset.sh can" << registerdev << " " << brate<< endl;

//     memset(canname, 0, sizeof(canname));

//     ret = system(canset.str().c_str());
//     if(ret != 0)
//     {
//         printf("can brate set fail!\n");
//         return -1;
//     }
     (void) brate;

     /* socketCAN连接 */
     CanFileP = socket(PF_CAN, SOCK_RAW, CAN_RAW);
     if(CanFileP < 0)
     {
         zprintf1("Socket PF_CAN failed!\n");
         return -1;
     }


     sprintf(canname, "can%d", registerdev);
     strcpy(ifr.ifr_name, canname);

     ret = ioctl(CanFileP, SIOCGIFINDEX, &ifr);
     if(ret<0)
     {
         zprintf1("Ioctl failed!\n");
         return -2;
     }

     addr.can_family = PF_CAN;
     addr.can_ifindex = ifr.ifr_ifindex;
     ret = ::bind(CanFileP, (struct sockaddr *)&addr, sizeof(addr));
     if(ret<0)
     {
         zprintf1("Bind failed!\n");
         return -3;
     }

     // 设置CAN滤波器
     struct can_filter rfilter[1];
     rfilter[0].can_id= 0x00;
     rfilter[0].can_mask = 0x00;

     ret = setsockopt(CanFileP, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
     if(ret < 0)
     {
         zprintf1("Set sockopt failed!\n");
         return -4;
     }

     e_poll_add(CanFileP);
     canwrite.z_pthread_init(call_write_back, this, "canbus write");
//     start();

     return 0;
 }

void CanDriver::run()
{
    zprintf3("can driver start\n");
//    struct epoll_event events[get_epoll_size()];
    char buf[sizeof(CanFrame)];
    CanFrame * pfram = (CanFrame*)buf;
    while (this->running)
    {
//        memset(&events, 0, sizeof(events));
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 4)
        if(wait_fd_change(-1) != -1)
#else
        if(wait_fd_change(150) != -1)
#endif
        {
//            printf("receive can frame\n");
//            nprintf("receive can data!\n");
           while(read(CanFileP, buf,sizeof(CanFrame)) == sizeof(CanFrame))
           {

//               zprintf1("receive canid 0x%x\n", pfram->can_id&0x1FFFFFFF);
//               canread.buf_write_data((CanFrame*)buf);
               this->rxcallback(this, *pfram);
//               nprintf("receive 0x%x over\n", pfram->can_id&0x1FFFFFFF);
           }

        }
//        else
//        {
//            nprintf("wati over!\n");
//        }
    }
}

int CanDriver::writeframe(const CanFrame& f)
{
    pthread_mutex_lock(&send_mut);
    int nbytes;
    int retry_times = 10;
    int total_write = sizeof(CanFrame);
    while(retry_times)
    {
        nbytes = ::write(CanFileP, &f, total_write);
        if(nbytes==total_write)
        {
//           timeprf("send successful %d!\n", f.can_dlc);
//            unsigned int midid = f.can_id&0x7FF;
//            if(midid == 0x421)
//            {
//            struct timeval tv;
//            gettimeofday(&tv, NULL);
//            printf("can bus tv_sec; %d tv usec %d\n", (int)tv.tv_sec, (int)tv.tv_usec);
//            }
//           zprintf1("send candi 0x%x\n", f.can_id&0x1FFFFFFF);
           break;
        }

        if(nbytes>0)
        {
            zprintf1("can bus error %d!\n",errno);
            zprintf1("%s\n",strerror(errno));
            retry_times--;
            linuxDly(2);
            continue;
        }
        if(nbytes<0)
        {
            zprintf1("can bus write error %d %d!\n",errno, retry_times);
            zprintf1("%s\n",strerror(errno));
            linuxDly(2);
//            usleep(10);
            retry_times--;
            continue;
        }
    }
    if(retry_times==0)
    {
        zprintf1("can bus may be full!\n");
    }
    pthread_mutex_unlock(&send_mut);
    return nbytes;
}

int CanDriver::writeframe(const CANDATAFORM& f)
{
    return writeframe(prodata_to_lawdata(f));
}

int CanDriver::write_send_data(const CANDATAFORM  & Msg)
{
    int ret = -1;
    ret = writeframe(Msg);

    return ret;
}





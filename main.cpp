#include <QTextCodec>
#include <can_bus.h>
#include <stdio.h>
#include "e_poll.h"
#include "timers.h"
#include "pthread.h"
#include "can_protocol.h"
#include "pro_data.h"
#include "candata.h"
#include "tk200pro.h"
#include "zprint.h"
#include <QCoreApplication>
#include "netprint.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <string.h>
#include <sys/io.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

Can_Data* gCanInfo = NULL;
// Print_Server * netp = NULL;

void SignalFunc(int var)
{
    ::printf("<DeviceMng signal1 exit %d val!>\n", var);

    if (gCanInfo != NULL)
    {
        delete gCanInfo;
        delete debug_p;
        delete gNetP;
        gCanInfo = NULL;
    }
    exit(0);
}

O_Timer timr;
int     printf_time_callback(TEvent* tmpara)
{
    nprintf("info is!\n");
}

int main(int argc, char* argv[])
{
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");

    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QCoreApplication a(argc, argv);

#if 0
         argc = 9;
         argv[1] = "87654334";
         argv[2] = "87654335";
         argv[3] = "87654336";
         argv[4] = "87654337";
         argv[5] = "87654331";
//         argv[6] = "87654322";
         argv[6] = "1";
         argv[7] = "87654338";
#if 0
         argv[8] = "tk200";
#else
         argv[8] = "236_1";
#endif
//         argv[6] = "1";
//         argv[7] = "1030";
//         argv[7] = "tk200";
#endif
    if (argc < 9)
    {
        printf("driver init para error!\n");
        return -1;
    }

    string type = argv[8];

    //         string pr_file = "/opt/config/usr/Par/" + type;

    string path = "/opt/ptcan_log/";
    int    ret;
    if (access(path.c_str(), F_OK) == -1)
    {
        int flag = mkdir(path.c_str(), S_IRWXU);
        if (flag == 0)
        {
            // printf("file create success %s\r\n",path.c_str());
            ret = true;
        }
        else
        {
            // printf("file create error:%s\r\n",path.c_str());
            ret = false;
        }
    }
    else
    {
        // printf("file is exist\r\n");
        ret = true;
    }
    if (ret == true)
    {
        string pr_file = path + type;
        debug_p->printf_init(pr_file.c_str(), 0);
    }
    zprintf3("++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    zprintf1("\r\n");
    zprintf3("pt can version %d.%d.%d\r\n", PTCAN_VERSION_H, PTCAN_VERSION_M, PTCAN_VERSION_L);
    zprintf3("ptcan low version is %d\r\n", 0x00);
    zprintf3("add shake thread\r\n");
    zprintf3("ptcan low version is %d\r\n", 0x01);
    zprintf3("del mac_24_used \r\n");
    zprintf3("ptcan low version is %d\r\n", 0x02);
    zprintf3("del debug \r\n");
    zprintf3("ptcan low version is %d\r\n", 0x03);
    zprintf3("reset finish report add csrxcanp->reset_msg != RESET_SUCCESS condition \r\n");
    zprintf3("ptcan low version is %d\r\n", 0x04);
    zprintf3("break location report before break_location = 0\r\n");
    zprintf3("++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    signal(SIGINT, SignalFunc);
    signal(SIGTERM, SignalFunc);

    gCanInfo = new Can_Data();

    if (gCanInfo == NULL)
    {
        zprintf1("error: can data creat failed!\n");
        return -1;
    }

    int* point = &(gCanInfo->devkey.data_key);
    for (int i = 1; i < argc - 1; i++)
    {
        qDebug() << "main argv[" << i << "]: " << argv[i];
        *point++ = atoi(argv[i]);
    }

    if ((type == "1030") || (type == "236_1") || (type == "236_2"))
    {
        gNetP = new Print_Server;
    }
    else
    {
        gNetP = new Print_Server(0xf420, 0xf421);
    }

#ifdef ARM_PLATFORM
    string dir = "/opt/config/driver/device/" + type + ".xml";
    string dir1, dir2;
    if (type == "236_1")
    {
        dir1 = "/opt/config/driver/device/" + type + "_1" + ".xml";
        dir2 = "/opt/config/driver/device/" + type + "_2" + ".xml";
    }
    gCanInfo->creat_can_bus_pro();
    gCanInfo->can_read_xml(QString(QString::fromLocal8Bit(dir.c_str())), QString(QString::fromLocal8Bit(dir1.c_str())),
        QString(QString::fromLocal8Bit(dir2.c_str())));

    gCanInfo->can_app_init();

#else
    /*            string type = argv[8];
                string dir = "/opt/"+type +".xml";
                cout << dir <<endl;
                Driver_Para<can_bus_para,can_bus_para_t> read;
                read.read_xml_dri_para("/home/zty/devicemng/tk200.xml","BusPara");
                printf(" %d %d\n", read.dri_info.canid, read.dri_info.crctime)*/
    ;
//          gCanInfo.can_read_xml("/home/zty/devicemng/tk200.xml");
//          gCanInfo.can_app_init();

//            string dir = "/opt/config/driver/device/"+type +".xml";

//            gCanInfo.can_read_xml("/home/zty/max/xml/z1030.xml");
//            gCanInfo->can_read_xml("/home/zty/max/info/1030.xml");

//            gCanInfo.can_read_xml("/home/zty/max/xml/bt1030.xml");
//            gCanInfo->creat_can_bus_pro();
//            gCanInfo->can_read_xml("/home/only/Documents/tk200.xml");
//            gCanInfo->can_app_init();
#endif

    return a.exec();
}

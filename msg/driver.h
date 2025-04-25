#ifndef DRIVER_H
#define DRIVER_H

#include <QObject>
#include <semaphore.h>
// #include "msg.h"

#define TEST_DATAIN_CNT   16
#define TEST_DATAOUT_CNT  12

typedef struct
{
    uint16_t    TotalInCnt;
    uint16_t    TotalOutCnt;
    uint16_t    TotalStateCnt;
}sParamInfoType;

typedef enum
{
    COMSTATE_NORMAL =0,
    COMSTATE_ABNORMAL
}eComStateType;

typedef struct
{  
    int data_key;
    int ctrl_key;
    int sem_key;
    int recvmsg_key;
    int sendmsg_key;

    int driverid;
    int state_key;

}sDriverInfo;

class Key_Info
{
public:
    sDriverInfo devkey;
    Key_Info(){
        memset(&devkey, 0x00, sizeof(sDriverInfo));
    }

};

class driver
{
private:
    driver();

public:
    sDriverInfo DriverInfo;
    sParamInfoType ParamInfo;
    eComStateType  ComState;

    static driver * Getdriver(void)
    {
        static driver gdriver;
        return &gdriver;
    }
    ~driver();
    bool Init(void);
    bool CtrlDeal();
};

#endif // DRIVER_H

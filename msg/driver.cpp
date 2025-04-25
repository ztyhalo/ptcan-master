#include "driver.h"
#include "QDebug"

driver::driver():ComState(COMSTATE_NORMAL)
{
    memset(&DriverInfo, 0x00, sizeof(DriverInfo));
    memset(&ParamInfo, 0x00, sizeof(ParamInfo));

}

driver::~driver()
{
    qDebug()<<"**************~driver";
}

bool driver::Init()
{
    ComState = COMSTATE_NORMAL;
    ParamInfo.TotalInCnt =TEST_DATAIN_CNT;
    ParamInfo.TotalOutCnt =TEST_DATAOUT_CNT;

    return true;
}

bool driver::CtrlDeal()
{
    return true;
}

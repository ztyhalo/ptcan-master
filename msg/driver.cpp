#include "driver.h"
#include "MsgMng.h"

driver::driver()
{

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

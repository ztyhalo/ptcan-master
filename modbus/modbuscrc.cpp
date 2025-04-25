
#include "modbuscrc.h"


/***********************************************************************************
 * 函数名：Modbus_CRCCal
 * 功能：生成crc校验
 *
 ***********************************************************************************/
void Modbus_CRCCal(const uint8_t * dat,  uint8_t len, uint8_t *crc)
{
    uint8_t		i;
    uint8_t		j;
    uint8_t		tmp;
    uint16_t	crcVal = 0xffff;

    for(i = 0; i < len; i++)
    {
        crcVal ^= dat[i];
        for(j = 0; j < 8; j++)
        {
            tmp = crcVal & 0x01;
            crcVal >>= 1;
            if(tmp)
            {
                crcVal ^= 0xa001;
            }
        }
    }
    crc[0] = (uint8_t)(crcVal &  0xff);          //little-endian
    crc[1] = (uint8_t)(crcVal >> 0x08);
}

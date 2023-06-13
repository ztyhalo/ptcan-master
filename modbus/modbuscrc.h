#ifndef MODBUSCRC_H
#define MODBUSCRC_H

#include <stdint.h>

void Modbus_CRCCal(uint8_t * dat,  uint8_t len, uint8_t *crc);
#endif // MODBUSCRC_H


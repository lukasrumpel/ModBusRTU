/*
 * modbusRTU.h
 *
 *  Created on: 13.02.2023
 *      Author: lukas
 */

#ifndef INC_MODBUSRTU_H_
#define INC_MODBUSRTU_H_

#include <stdint.h>

typedef enum{
	modbusOK = 0,
	regOutOfBound = -1
}modbusErrCode;

extern uint16_t registerRead(uint8_t regAdr);
extern modbusErrCode registerWrite(uint8_t regAdr, uint16_t regData);
extern modbusErrCode setSlaveAddress(uint8_t address);
extern uint16_t modbusCRC(uint8_t *data, uint8_t len);
extern void modbusResponse(uint8_t *data, uint8_t len);

#endif /* INC_MODBUSRTU_H_ */

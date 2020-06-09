#ifndef MODBUS_H
#define MODBUS_H

#define MBdebug 

#define _ModbusTCP_port 502 
//
// MODBUS Functions Codes
// 
#define MB_FC_NONE 0
#define MB_FC_READ_REGISTERS 3 //implemented
#define MB_FC_WRITE_REGISTER 6 //implemented
#define MB_FC_WRITE_MULTIPLE_REGISTERS 16 //implemented
//
// MODBUS Error Codes
//
#define MB_EC_NONE 0
#define MB_EC_ILLEGAL_FUNCTION 1
#define MB_EC_ILLEGAL_DATA_ADDRESS 2
#define MB_EC_ILLEGAL_DATA_VALUE 3
#define MB_EC_SLAVE_DEVICE_FAILURE 4
//
// MODBUS MBAP offsets
//
#define MB_TCP_TID 0
#define MB_TCP_PID 2
#define MB_TCP_LEN 4
#define MB_TCP_UID 6
#define MB_TCP_FUNC 7
#define MB_TCP_REGISTER_START 8
#define MB_TCP_REGISTER_NUMBER 10
 
#include "Arduino.h"
// Holding Registers [0...maxHoldReg]
#define maxHoldReg 20
#define BufferSize 260 //for 20 regs

// Read Write Holding Registers -  HoldReg 
boolean MBtransaction(void);

void MBserverBegin(void); 
#endif

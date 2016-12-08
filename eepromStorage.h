/*
 * eepromStorage.h
 *
 * Created: 13-11-2016 11:21:50
 *  Author: Paul
 */ 


#ifndef EEPROMSTORAGE_H_
#define EEPROMSTORAGE_H_

#include "clocks.h"
#include "applicationInterface.h"

#define EEPROM_OWN_ADDRESS			(uint16_t*)0X0
#define EEPROM_HOP_COUNT			(uint8_t*)0X4
#define EEPROM_MAX_RETRY			(uint8_t*)0X5
#define EEPROM_MAX_UPDU_LENGTH		(uint16_t*)0X6

#define EEPROM_START_INTF_OBJECTS	0x0020
#define EEPROM_END_OF_LIST			0xFFFF

typedef struct {
	int8_t						enable;
	intptr_t					endOfIntfObjs;
	A_InterfaceHandle			AI_Hndl;
	
} eepromStorage_Obj;

typedef eepromStorage_Obj *eepromStorage_Handle;

eepromStorage_Handle eepromStorage_Init(void *pMemory, uint16_t numBytes);

int8_t eepromStorage_Setup(eepromStorage_Handle handle, A_InterfaceHandle AI_Hndl);

int16_t eepromStorage_appendInterfaceObject(eepromStorage_Handle handle, interfaceProperty *prop);

interfaceProperty eepromStorage_fetchNextInterfaceObject(eepromStorage_Handle handle);

int8_t eepromStorage_closeInterfaceObject(eepromStorage_Handle handle);

int16_t eepromStorage_saveAllIntfObjects(eepromStorage_Handle handle);

int16_t eepromStorage_restoreAllIntfObjects(eepromStorage_Handle handle);

int8_t eepromStorage_resetInterfaceObject(eepromStorage_Handle handle);

int16_t eepromStorage_locateEndOfIntfObjects(eepromStorage_Handle handle);

#endif /* EEPROMSTORAGE_H_ */
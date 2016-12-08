/*
 * eepromStorage.c
 *
 * Created: 13-11-2016 11:22:10
 *  Author: Paul
 */ 

#include "eepromStorage.h"
#include "avr/eeprom.h"

eepromStorage_Handle eepromStorage_Init(void *pMemory, uint16_t numBytes) {
	eepromStorage_Handle eeprom_handle;
	if (numBytes < sizeof(eepromStorage_Obj))
	return ((eepromStorage_Handle)NULL);
	eeprom_handle = (eepromStorage_Handle)pMemory;
	return(eeprom_handle);
}

int8_t eepromStorage_Setup(eepromStorage_Handle handle, A_InterfaceHandle AI_Hndl) {
	eepromStorage_Obj *eeprom;
	eeprom = (eepromStorage_Obj *)handle;
	
	eeprom->AI_Hndl = AI_Hndl;
	eeprom->endOfIntfObjs = EEPROM_START_INTF_OBJECTS;
	eeprom->enable = 1;
	
	return 0;
}

int16_t eepromStorage_appendInterfaceObject(eepromStorage_Handle handle, interfaceProperty *prop) {
	eepromStorage_Obj *eeprom;
	eeprom = (eepromStorage_Obj *)handle;
	
	int16_t length = intfObj_SerializeElements(prop, NULL) + 5;
	uint8_t *eepromPtr = (uint8_t *)(eeprom->endOfIntfObjs);
	if ((eeprom->endOfIntfObjs + length) > EEPROM_SIZE)
		return 0;
	eeprom_update_word((uint16_t*)eepromPtr, length);
	eepromPtr += 2;
	eeprom_update_byte(eepromPtr++, prop->objectIndex);
	eeprom_update_word((uint16_t*)eepromPtr, prop->propertyId);
	eepromPtr += 2;
	intptr_t startPtr = 0;
	eeprom_update_block ((uint16_t*)startPtr, eepromPtr, intfObj_SerializeElements(prop, &startPtr));
	eeprom->endOfIntfObjs += length;
	return length;
}

interfaceProperty eepromStorage_fetchNextInterfaceObject(eepromStorage_Handle handle) {
	eepromStorage_Obj *eeprom;
	eeprom = (eepromStorage_Obj *)handle;
	
	uint16_t length;
	interfaceProperty prop;
	length = eeprom_read_word((uint16_t*)eeprom->endOfIntfObjs);
	if (!length || (eeprom->endOfIntfObjs + length) > EEPROM_SIZE)
		return prop;
	prop.objectIndex = eeprom_read_byte((uint8_t*)(eeprom->endOfIntfObjs + 2));
	prop.propertyId = eeprom_read_word((uint16_t*)(eeprom->endOfIntfObjs + 3));
	prop.elements = (void*)(MAPPED_EEPROM_START + eeprom->endOfIntfObjs + 5);
	eeprom->endOfIntfObjs += length;
	return prop;
}

int8_t eepromStorage_closeInterfaceObject(eepromStorage_Handle handle) {
	eepromStorage_Obj *eeprom;
	eeprom = (eepromStorage_Obj *)handle;	
	
	eeprom_update_word((uint16_t*)eeprom->endOfIntfObjs, EEPROM_END_OF_LIST);
	return 0;
}

int16_t eepromStorage_saveAllIntfObjects(eepromStorage_Handle handle) {
	eepromStorage_Obj *eeprom;
	eeprom = (eepromStorage_Obj *)handle;
	
	interfaceObjectServerHandle IO_Hndl	= eeprom->AI_Hndl->IO_Hndl;
	eepromStorage_resetInterfaceObject(handle);
	int16_t length = 0;
	uint8_t objIdxCnt = 0;
	uint8_t propIdxCnt = 1;
	interfaceProperty* prop;
	
	while(IntfObj_Srv_getObjByPropIndex(IO_Hndl, objIdxCnt, 0) != NULL) {
		for(;;) {
			prop = IntfObj_Srv_getObjByPropIndex(IO_Hndl, objIdxCnt, propIdxCnt++);
			if (prop == NULL)
				break;
			if ((prop->flags & 0x02) != 0)
			{
				length += eepromStorage_appendInterfaceObject(handle, prop);
			}
		}
		propIdxCnt = 1;
		objIdxCnt++;
	}
	
	eepromStorage_closeInterfaceObject(handle);
	
	return length;
}

int16_t eepromStorage_restoreAllIntfObjects(eepromStorage_Handle handle) {
	eepromStorage_Obj *eeprom;
	eeprom = (eepromStorage_Obj *)handle;
	
	interfaceObjectServerHandle IO_Hndl	= eeprom->AI_Hndl->IO_Hndl;
	eepromStorage_resetInterfaceObject(handle);

	int16_t endoflist = EEPROM_START_INTF_OBJECTS;
	int16_t length = 0;
	do {
		length = eeprom_read_word((uint16_t*)endoflist);
		if (length != EEPROM_END_OF_LIST) {
			interfaceProperty* prop;
			uint8_t objIdx = eeprom_read_byte((uint8_t*)(endoflist + 2));
			uint16_t propId = eeprom_read_word((uint16_t*)(endoflist + 3));
			prop = IntfObj_Srv_getObjByPropId(IO_Hndl, objIdx, propId);
			if (prop) {
				if (length == (intfObj_SerializeElements(prop, NULL) + 5)) {
					for(uint16_t i = 0; i < (length - 5); i++)
						((uint8_t*)(prop->elements))[i] =  ((uint8_t*)(MAPPED_EEPROM_START + endoflist + 5))[i];
				}
			}
			endoflist += length;
		}
	}
	while (length != EEPROM_END_OF_LIST);
	eeprom->endOfIntfObjs = endoflist;
	
	return endoflist;
}

int8_t eepromStorage_resetInterfaceObject(eepromStorage_Handle handle) {
	eepromStorage_Obj *eeprom;
	eeprom = (eepromStorage_Obj *)handle;
	
	eeprom->endOfIntfObjs = EEPROM_START_INTF_OBJECTS;
	return 0;
}

int16_t eepromStorage_locateEndOfIntfObjects(eepromStorage_Handle handle) {
	eepromStorage_Obj *eeprom;
	eeprom = (eepromStorage_Obj *)handle;
	
	int16_t endoflist = EEPROM_START_INTF_OBJECTS;
	int16_t length = 0;
	do {
		length = eeprom_read_word((uint16_t*)endoflist);
		endoflist += (length != EEPROM_END_OF_LIST) ? length : 0;
	}
	while (length != EEPROM_END_OF_LIST);
	eeprom->endOfIntfObjs = endoflist;
	return endoflist;
}

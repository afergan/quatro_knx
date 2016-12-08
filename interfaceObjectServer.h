/*
 * propertyObject.h
 *
 * Created: 17-8-2016 13:11:48
 *  Author: Paul
 */ 


#ifndef INTERFACEOBJECTSERVER_H_
#define INTERFACEOBJECTSERVER_H_

#include "clocks.h"
#include "interfaceObject.h"
#include "object_resources.h"
#include <avr/pgmspace.h>

#define PROP_FLAG_READ_ONLY				0
#define PROP_FLAG_WRITE_ENABLE			0x1
#define PROP_FLAG_SAVE_IN_EEPROM		0x2
#define PROP_FLAG_STORED_IN_PROGMEM		0x4
#define PROP_FLAG_USE_IN_CRC			0x8
	
#define STR_ELEMENT_PRGMEM(a)		const sGeneric (a ## _ELEMENT) PROGMEM = {.cnt = SWAP_UINT16((sizeof(a)) - 1), .value = (a)};
	
#define MAX_ELEMENTS_IS_ELEMENTS_IN_USE	0xFFFF

#define UINT32_ELEMENT(propId, storeflags, objectValue) {.propertyId = propId, .maxElements = 1, .readAccessLvl = 3, .writeAccessLvl = 2, .flags = (storeflags), .propertyDataType = PDT_UNSIGNED_LONG, .elements =  &(sLong){cnt : SWAP_UINT16(1), value : SWAP_UINT32(objectValue)}}
#define UINT16_ELEMENT(propId, storeflags, objectValue) {.propertyId = propId, .maxElements = 1, .readAccessLvl = 3, .writeAccessLvl = 2, .flags = (storeflags), .propertyDataType = PDT_UNSIGNED_INT, .elements =  &(sWord){cnt : SWAP_UINT16(1), value : SWAP_UINT16(objectValue)}}
#define UCHAR_ELEMENT(propId, storeflags, objectValue) {.propertyId = propId, .maxElements = 1, .readAccessLvl = 3, .writeAccessLvl = 2, .flags = (storeflags), .propertyDataType = PDT_UNSIGNED_CHAR, .elements = &(sByte){.cnt = SWAP_UINT16(1), .value = objectValue}}
#define CONTROL_ELEMENT(propId, storeflags, objectValue) {.propertyId = propId, .maxElements = 1, .readAccessLvl = 3, .writeAccessLvl = 2, .flags = (storeflags), .propertyDataType = PDT_CONTROL, .elements = &(sGeneric10){.cnt = SWAP_UINT16(1), .value = objectValue}}

#define DECSCR_UCHAR_ELEMENTS(propId, storeflags, objectValue) {.propertyId = propId, .maxElements = ((sizeof objectValue) - 1), .readAccessLvl = 3, .writeAccessLvl = 0, .flags = (storeflags), \
	.propertyDataType = PDT_UNSIGNED_CHAR, .elements = &(struct {uint16_t cnt; uint8_t value[(sizeof objectValue) - 1];}){.cnt = SWAP_UINT16((sizeof objectValue) - 1), .value = objectValue}}

#define GENERIC_ELEMENT(propId, size, readlvl, wrtlvl, storeflags, element) {.propertyId = propId, .maxElements = 1, .readAccessLvl = readlvl, .writeAccessLvl = wrtlvl, \
	.flags = storeflags, .propertyDataType = (PDT_VARIABLE_LENGTH + size), .elements = &(struct {uint16_t cnt; uint8_t value[size];}){.cnt = SWAP_UINT16(1), .value = element}}
#define HARDWARE_SERIAL_OBJECT(propId, readlvl, wrtlvl, storeflags, manuId, hardwSer) {.propertyId = propId, .maxElements = 1, .readAccessLvl = readlvl, .writeAccessLvl = wrtlvl, \
	.flags = storeflags, .propertyDataType = PDT_GENERIC06, .elements = &(struct {uint16_t cnt; uint16_t m_id; uint32_t hs;}){.cnt = SWAP_UINT16(1), .m_id = SWAP_UINT16(manuId), .hs=SWAP_UINT32(hardwSer)}}
#define VERSION_OBJECT(propId, storeflags, objectValue) {.propertyId = propId, .maxElements = 1, .readAccessLvl = 3, .writeAccessLvl = 0, .flags = (storeflags & 0xFE), \
	.propertyDataType = PDT_VERSION, .elements =  &(sGeneric02){.cnt = SWAP_UINT16(1), .value = objectValue}}
#define BITSET_OBJECT(propId, storeflags, objectValue) {.propertyId = propId, .maxElements = 1, .readAccessLvl = 3, .writeAccessLvl = 2, .flags = (storeflags), .propertyDataType = PDT_BITSET8, .elements = &(sByte){.cnt = SWAP_UINT16(1), .value = objectValue}}

#define TABLE_OBJECT(propType, storeflags, elementCnt) {.propertyId = PID_TABLE, .maxElements = elementCnt, .readAccessLvl = 3, .writeAccessLvl = 2, .flags = storeflags, .propertyDataType = propType, .elements = NULL}

#define PROPERTY_OBJECT(propId, propType, elementCnt, readlvl, wrtlvl, storeflags) {.propertyId = propId, .maxElements = elementCnt, .readAccessLvl = readlvl, .writeAccessLvl = wrtlvl, .flags = storeflags, .propertyDataType = propType, .elements = NULL}

#define DESCRIPTION_OBJECT_PRGMEM(propId, descrip, readlvl) {.propertyId = propId, .maxElements = sizeof(descrip) - 1, .readAccessLvl = readlvl, .writeAccessLvl = 0, .flags = PROP_FLAG_READ_ONLY | PROP_FLAG_STORED_IN_PROGMEM, .propertyDataType = PDT_UNSIGNED_CHAR, .elements = &(descrip ## _ELEMENT)}

	
#define END_OF_OBJECT {.propertyId = 0, .maxElements = 0, .readAccessLvl = 0, .writeAccessLvl = 0, .flags = 0, .propertyDataType = 0, .elements = NULL}


typedef struct {
	uint8_t				enable;
	interfaceProperty	**prop;
} interfaceObjectServerObj;

typedef interfaceObjectServerObj *interfaceObjectServerHandle;

interfaceObjectServerHandle IntfObj_Srv_Init(void *pMemory, uint16_t numBytes);

int8_t IntfObj_Srv_Setup(interfaceObjectServerHandle handle);

int8_t IntfObj_Srv_registerObject(interfaceObjectServerHandle handle, prop_obj_e objIdx, application_property_id_e propId, void *object);

int8_t IntfObj_Srv_registerTableObject(interfaceObjectServerHandle handle, prop_obj_e objIdx, PropertyId_e propId, void *object);

int8_t IntfObj_Srv_setMCTObject(interfaceObjectServerHandle handle, prop_obj_e objIdx);

interfaceProperty *IntfObj_Srv_getObjByPropId(interfaceObjectServerHandle handle, prop_obj_e objIdx, PropertyId_e propId);

interfaceProperty *IntfObj_Srv_getObjByPropIndex(interfaceObjectServerHandle handle, prop_obj_e objIdx, uint8_t propIndex);

uint8_t getMaxObjIdx();



#endif /* INTERFACEOBJECTSERVER_H_ */
/*
 * interfaceObject.h
 *
 * Created: 24-11-2016 18:38:08
 *  Author: Paul
 */ 


#ifndef INTERFACEOBJECT_H_
#define INTERFACEOBJECT_H_

#include "clocks.h"
#include "object_resources.h"

typedef struct {
	uint16_t	cnt;
	uint8_t		value;
} sByte;

typedef struct {
	uint16_t	cnt;
	uint16_t	value;
} sWord;

typedef struct {
	uint16_t	cnt;
	uint16_t	value[];
} sWordArray;

typedef struct {
	uint16_t	cnt;
	uint32_t	value;
} sLong;

typedef struct {
	uint16_t	cnt;
	uint8_t		value[];
} sGeneric;

typedef struct {
	uint16_t	cnt;
	uint8_t		value[1];
} sGeneric01;

typedef struct {
	uint16_t	cnt;
	uint8_t		value[2];
} sGeneric02;

typedef struct {
	uint16_t	cnt;
	uint8_t		value[4];
} sGeneric04;

typedef struct {
	uint16_t	cnt;
	uint8_t		value[5];
} sGeneric05;

typedef struct {
	uint16_t	cnt;
	uint8_t		value[6];
} sGeneric06;

typedef struct {
	uint16_t	cnt;
	uint8_t		value[8];
} sGeneric08;

typedef struct {
	uint16_t	cnt;
	uint8_t		value[10];
} sGeneric10;

typedef struct {
	uint16_t	cnt;
	uint8_t		value[][2];
} sGeneric02Array;

typedef struct {
	uint16_t	cnt;
	uint8_t		value[][4];
} sGeneric04Array;

typedef struct {
	uint16_t	cnt;
	uint8_t		value[][6];
} sGeneric06Array;

typedef struct {
	uint8_t						objectIndex;
	uint8_t						propertyIndex;
	PropertyId_e				propertyId;
	PDT_e						propertyDataType;
	uint8_t						readAccessLvl;
	uint8_t						writeAccessLvl;
	uint8_t						flags;			// bit 0 : write enable, bit 1 : save in eeprom, bit 2 : elements stored in progmem, bit 3 : use in CRC calculation
	uint16_t					maxElements;
	void						*elements;
} interfaceProperty;


uint8_t *IntfObj_getPropValue(interfaceProperty *prop, uint16_t startIndex);

uint8_t *IntfObj_getPropValue_P(interfaceProperty *prop, uint16_t startIndex);

int8_t IntfObj_setPropValue(interfaceProperty *prop, uint16_t startIndex, uint8_t count, uint8_t* value);

int8_t IntfObj_isWriteEnable(interfaceProperty *prop);

int8_t IntfObj_setElementsInUse(interfaceProperty *prop, uint16_t inUse);

uint16_t IntfObj_getElementsInUse(interfaceProperty *prop);

int16_t intfObj_SerializeElements(interfaceProperty *prop, intptr_t *startPointer);

int8_t intfObj_DeserializeElements(interfaceProperty *prop, intptr_t *startPointer, uint16_t Length);

int8_t intfObj_Clone(interfaceProperty *source, interfaceProperty *destination);

#endif /* INTERFACEOBJECT_H_ */
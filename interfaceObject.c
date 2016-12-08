/*
 * interfaceObject.c
 *
 * Created: 24-11-2016 18:37:54
 *  Author: Paul
 */ 

#include "interfaceObject.h"
#include <avr/pgmspace.h>

uint8_t *IntfObj_getPropValue(interfaceProperty *prop, uint16_t startIndex) {
	if (prop && prop->elements) {
		uint8_t elementSize = getPDTsize(prop->propertyDataType);
		if (startIndex <= SWAP_UINT16(*((uint16_t*)prop->elements))) {
			return (uint8_t*)(prop->elements + (startIndex - 1) * elementSize + 2);
		}
	}
	return NULL;
}

uint8_t *IntfObj_getPropValue_P(interfaceProperty *prop, uint16_t startIndex) {
	if (prop && prop->elements) {
		uint8_t elementSize = getPDTsize(prop->propertyDataType);
		if (startIndex <= SWAP_UINT16(pgm_read_dword(prop->elements))) {
			return (uint8_t*)(prop->elements + (startIndex - 1) * elementSize + 2);
		}
	}
	return NULL;
}

int8_t IntfObj_setPropValue(interfaceProperty *prop, uint16_t startIndex, uint8_t count, uint8_t* value) {
	if (prop && prop->elements) {
		if ((startIndex + count - 1) <= prop->maxElements) {
			uint8_t elementSize = getPDTsize(prop->propertyDataType);
			for (uint8_t i = 0; i < (elementSize * count); i++) {
				*((uint8_t*)prop->elements + 2 + (startIndex - 1) * elementSize + i) = *(value + i);
			}
			return (elementSize * count);
		}
	}
	return 0;
}

int8_t IntfObj_isWriteEnable(interfaceProperty *prop) {
	if (prop && prop->elements) {
		if ((prop->flags & 0x1) != 0)
		return 1;
	}
	return 0;
}

int8_t IntfObj_setElementsInUse(interfaceProperty *prop, uint16_t inUse) {
	if (prop && prop->elements && inUse > prop->maxElements)
	return -1;
	*((uint16_t*)prop->elements) = SWAP_UINT16(inUse);
	return 0;
}

uint16_t IntfObj_getElementsInUse(interfaceProperty *prop) {
	if (prop)
	return SWAP_UINT16(*((uint16_t*)prop->elements));
	return 0;
}

int16_t intfObj_SerializeElements(interfaceProperty *prop, intptr_t *startPointer) {
	if (prop)
	{
		if (startPointer)
		*startPointer = (intptr_t)prop->elements;
		return (prop->maxElements * getPDTsize(prop->propertyDataType)) + getPDTsize(PDT_UNSIGNED_INT);
	}
	return 0;
}

int8_t intfObj_DeserializeElements(interfaceProperty *prop, intptr_t *startPointer, uint16_t Length) {
	
	return 0;
}

int8_t intfObj_Clone(interfaceProperty *source, interfaceProperty *destination) {
	if (source != NULL && destination != NULL)
	{
		for (uint8_t i = 0; i < sizeof(interfaceProperty); i++)
			*((uint8_t*)destination + i) = *((uint8_t*)source + i); 
		return 0;
	}
	return -1;
}
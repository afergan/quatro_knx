/*
 * propertyObject.c
 *
 * Created: 17-8-2016 13:12:03
 *  Author: Paul
 */ 


#include "interfaceObjectServer.h"

// device descriptor
#define	DEVICE_DESCRIPTOR		0x07B0
// manufacturer id
#define MANUFACTURER_ID			0xabcd
// version
#define	VERSION_MAGIC			0
#define	VERSION_VERSION			1
#define VERSION_REVISION		2
#define VERSION					{(VERSION_MAGIC << 3) | (VERSION_VERSION >> 2), (VERSION_VERSION << 6) | (VERSION_REVISION & 0x3f)}
// app1 version
#define APP1_PROG_VERSION		0x00123401L
#define APP1_PROG_VERSION_MANU	{(uint8_t)(MANUFACTURER_ID >> 8), (uint8_t)MANUFACTURER_ID, (uint8_t)(APP1_PROG_VERSION >> 16), (uint8_t)(APP1_PROG_VERSION >> 8), (uint8_t)APP1_PROG_VERSION}
// app2 version
#define APP2_PROG_VERSION_MANU	{(uint8_t)(MANUFACTURER_ID >> 8), (uint8_t)MANUFACTURER_ID, 0x00,0x00,0x00}
// firmware revision
#define FIRMWARE_REVISION		1
// manufacturer data
#define MANUFACTURER_DATA		0xaaaaaaaaL
// serial number
#define SERIAL_NUMBER			0x12345678L
// hardware type
#define HARDWARE_TYPE			0xdeadbeefL
// order info
#define ORDER_INFO				"black mart"	// generic 10
// device description
#define DEVICE_DESCRIPTION		"- Quatro - nov. 2016"
STR_ELEMENT_PRGMEM(DEVICE_DESCRIPTION)

// device control.
// bit 1 : 1 means at least one frame received with source address same as this device
// bit 2 : verify mode
#define DEVICE_CONTROL			{0x00}
	
#define EMPTY_MCT				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

#define NUMBER_OF_OBJECTS		11

interfaceProperty deviceObject[] = {
	UINT16_ELEMENT(PID_OBJECT_TYPE, PROP_FLAG_READ_ONLY, DEVICE_OBJECT),
	UINT16_ELEMENT(PID_SERVICE_CONTROL, PROP_FLAG_WRITE_ENABLE, 0x0004), // bit 2 : 1 means individual address write enable.
	UCHAR_ELEMENT(PID_FIRMWARE_REVISION, PROP_FLAG_READ_ONLY, FIRMWARE_REVISION),
	HARDWARE_SERIAL_OBJECT(PID_SERIAL_NUMBER, 3, 2, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM, MANUFACTURER_ID, SERIAL_NUMBER),
	UINT16_ELEMENT(PID_MANUFACTURER_ID, PROP_FLAG_READ_ONLY, MANUFACTURER_ID),
	GENERIC_ELEMENT(PID_DEVICE_CONTROL, 1, 3, 2, PROP_FLAG_WRITE_ENABLE, DEVICE_CONTROL),
	GENERIC_ELEMENT(PID_ORDER_INFO, 10, 3, 0, PROP_FLAG_READ_ONLY, ORDER_INFO),
	UCHAR_ELEMENT(PID_PEI_TYPE, PROP_FLAG_READ_ONLY, 0),
	GENERIC_ELEMENT(PID_MANUFACTURER_DATA, 4, 3, 0, PROP_FLAG_READ_ONLY, UINT32_TO_BYTE_ARRAY(MANUFACTURER_DATA)),
	DESCRIPTION_OBJECT_PRGMEM(PID_DESCRIPTION, DEVICE_DESCRIPTION, 3),
	VERSION_OBJECT(PID_VERSION, PROP_FLAG_READ_ONLY, VERSION),
	UCHAR_ELEMENT(PID_OBJECT_INDEX, PROP_FLAG_READ_ONLY, 0),
	PROPERTY_OBJECT(PID_ROUTING_COUNT, PDT_UNSIGNED_CHAR, 1, 3, 2, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM),
	PROPERTY_OBJECT(PID_MAX_RETRY_COUNT, PDT_GENERIC01, 1, 3, 2, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM),
	PROPERTY_OBJECT(PID_PROGMODE, PDT_BITSET8, 1, 3, 2, PROP_FLAG_WRITE_ENABLE),
	PROPERTY_OBJECT(PID_MAX_APDULENGTH, PDT_UNSIGNED_INT, 1, 3, 2, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM),
	PROPERTY_OBJECT(PID_SUBNET_ADDR, PDT_UNSIGNED_CHAR, 1, 3, 0, PROP_FLAG_READ_ONLY),
	PROPERTY_OBJECT(PID_DEVICE_ADDR, PDT_UNSIGNED_CHAR, 1, 3, 0, PROP_FLAG_READ_ONLY),
	HARDWARE_SERIAL_OBJECT(PID_HARDWARE_TYPE, 3, 0, PROP_FLAG_READ_ONLY, MANUFACTURER_ID, HARDWARE_TYPE),
	GENERIC_ELEMENT(PID_DEVICE_DESCRIPTOR, 2, 3, 0, PROP_FLAG_READ_ONLY, UINT16_TO_BYTE_ARRAY(DEVICE_DESCRIPTOR)),
	UINT16_ELEMENT(PID_GROUP_TELEGR_RATE_LIMIT_TIME_BASE, PROP_FLAG_READ_ONLY, 17000),
	UINT16_ELEMENT(PID_GROUP_TELEGR_RATE_LIMIT_NO_OF_TELEGR, PROP_FLAG_READ_ONLY, 1234),
	END_OF_OBJECT
};

interfaceProperty addressTableObject[] = {
	UINT16_ELEMENT(PID_OBJECT_TYPE, PROP_FLAG_READ_ONLY, ADDRESS_TABLE_OBJECT),
	PROPERTY_OBJECT(PID_LOAD_STATE_CONTROL, PDT_CONTROL, 1, 3, 2, PROP_FLAG_WRITE_ENABLE),
	UINT32_ELEMENT(PID_TABLE_REFERENCE, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM | PROP_FLAG_USE_IN_CRC, 0L),
	TABLE_OBJECT(PDT_UNSIGNED_INT, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM | PROP_FLAG_USE_IN_CRC, MAX_GRP_OBJECTS),
	GENERIC_ELEMENT(PID_MCB_TABLE, 8, 3, 2, PROP_FLAG_READ_ONLY | PROP_FLAG_SAVE_IN_EEPROM, EMPTY_MCT),
	UCHAR_ELEMENT(PID_OBJECT_INDEX, PROP_FLAG_READ_ONLY, 1),
	END_OF_OBJECT
};
	
interfaceProperty associationTableObject[] = {
	UINT16_ELEMENT(PID_OBJECT_TYPE, PROP_FLAG_READ_ONLY, ASSOCIATION_TABLE_OBJECT),
	PROPERTY_OBJECT(PID_LOAD_STATE_CONTROL, PDT_CONTROL, 1, 3, 2, PROP_FLAG_WRITE_ENABLE),
	UINT32_ELEMENT(PID_TABLE_REFERENCE, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM | PROP_FLAG_USE_IN_CRC, 0L),
	TABLE_OBJECT(PDT_GENERIC04, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM | PROP_FLAG_USE_IN_CRC, MAX_GRP_OBJECTS),
	GENERIC_ELEMENT(PID_MCB_TABLE, 8, 3, 2, PROP_FLAG_READ_ONLY | PROP_FLAG_SAVE_IN_EEPROM, EMPTY_MCT),
	UCHAR_ELEMENT(PID_OBJECT_INDEX, PROP_FLAG_READ_ONLY, 2),
	END_OF_OBJECT
};

interfaceProperty groupObjectTableObject[] = {
	UINT16_ELEMENT(PID_OBJECT_TYPE, PROP_FLAG_READ_ONLY, GROUP_OBJECT_TABLE_OBJECT),
	PROPERTY_OBJECT(PID_LOAD_STATE_CONTROL, PDT_CONTROL, 1, 3, 2, PROP_FLAG_WRITE_ENABLE),
	UINT32_ELEMENT(PID_TABLE_REFERENCE, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM | PROP_FLAG_USE_IN_CRC, 0L),
	TABLE_OBJECT(PDT_GENERIC02, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM | PROP_FLAG_USE_IN_CRC, MAX_GRP_OBJECTS),
	GENERIC_ELEMENT(PID_MCB_TABLE, 8, 3, 2, PROP_FLAG_READ_ONLY | PROP_FLAG_SAVE_IN_EEPROM, EMPTY_MCT),
	UCHAR_ELEMENT(PID_OBJECT_INDEX, PROP_FLAG_READ_ONLY, 3),
	END_OF_OBJECT
};

interfaceProperty applicationObject1[] = {
	UINT16_ELEMENT(PID_OBJECT_TYPE, PROP_FLAG_READ_ONLY, APPLICATION_OBJECT_1),
	CONTROL_ELEMENT(PID_LOAD_STATE_CONTROL, PROP_FLAG_READ_ONLY, {(load_state_e)OBJECT_LOAD_STATE_LOADED}),
	CONTROL_ELEMENT(PID_RUN_STATE_CONTROL, PROP_FLAG_READ_ONLY,{(run_state_e)PROGRAM_RUNNING}),
	GENERIC_ELEMENT(PID_PROGRAM_VERSION, 5, 3, 0, PROP_FLAG_READ_ONLY, APP1_PROG_VERSION_MANU),
	UCHAR_ELEMENT(PID_PEI_TYPE, PROP_FLAG_READ_ONLY, 0),
	UINT32_ELEMENT(PID_TABLE_REFERENCE, PROP_FLAG_READ_ONLY, 0L),
	GENERIC_ELEMENT(PID_MCB_TABLE, 8, 3, 0, PROP_FLAG_READ_ONLY, EMPTY_MCT),
	UCHAR_ELEMENT(PID_OBJECT_INDEX, PROP_FLAG_READ_ONLY, 4),
	END_OF_OBJECT
};

interfaceProperty applicationObject2[] = {
	UINT16_ELEMENT(PID_OBJECT_TYPE, PROP_FLAG_READ_ONLY, APPLICATION_OBJECT_2),
	CONTROL_ELEMENT(PID_LOAD_STATE_CONTROL, PROP_FLAG_READ_ONLY, {(load_state_e)OBJECT_LOAD_STATE_UNLOADED}),
	CONTROL_ELEMENT(PID_RUN_STATE_CONTROL, PROP_FLAG_READ_ONLY, {(run_state_e)PROGRAM_HALTED}),
	GENERIC_ELEMENT(PID_PROGRAM_VERSION, 5, 3, 0, PROP_FLAG_READ_ONLY, APP2_PROG_VERSION_MANU),
	UCHAR_ELEMENT(PID_PEI_TYPE, PROP_FLAG_READ_ONLY, 0),
	UINT32_ELEMENT(PID_TABLE_REFERENCE, PROP_FLAG_READ_ONLY, 0L),
	GENERIC_ELEMENT(PID_MCB_TABLE, 8, 3, 0, PROP_FLAG_READ_ONLY, EMPTY_MCT),
	UCHAR_ELEMENT(PID_OBJECT_INDEX, PROP_FLAG_READ_ONLY, 5),	
	END_OF_OBJECT
};

// interface object touch pad

#define TOUCHPAD_DESCR	"touchpad layout configuration"
STR_ELEMENT_PRGMEM(TOUCHPAD_DESCR)

interfaceProperty touchpadCfgObject[] = {
	UINT16_ELEMENT(PID_OBJECT_TYPE, PROP_FLAG_READ_ONLY, TOUCHPAD_CFG_OBJECT),
	DESCRIPTION_OBJECT_PRGMEM(PID_OBJECT_NAME, TOUCHPAD_DESCR, 3),
	PROPERTY_OBJECT(PID_LOAD_STATE_CONTROL, PDT_CONTROL, 1, 3, 2, PROP_FLAG_WRITE_ENABLE),
	PROPERTY_OBJECT(PID_TOUCHPAD_CFG, PDT_GENERIC01, MAX_ELEMENTS_IS_ELEMENTS_IN_USE, 3, 2, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM | PROP_FLAG_USE_IN_CRC),
	UCHAR_ELEMENT(PID_OBJECT_INDEX, PROP_FLAG_READ_ONLY, 6),	
	END_OF_OBJECT
};

// interface object actions

#define ACTION_DESCR "touch action configuration"
STR_ELEMENT_PRGMEM(ACTION_DESCR)

interfaceProperty actionCfgObject[] = {
	UINT16_ELEMENT(PID_OBJECT_TYPE, PROP_FLAG_READ_ONLY, ACTION_CFG_OBJECT),
	DESCRIPTION_OBJECT_PRGMEM(PID_OBJECT_NAME, ACTION_DESCR, 3),
	PROPERTY_OBJECT(PID_LOAD_STATE_CONTROL, PDT_CONTROL, 1, 3, 2, PROP_FLAG_WRITE_ENABLE),
	PROPERTY_OBJECT(PID_TOUCH_ACTION_CFG, PDT_GENERIC06, MAX_ELEMENTS_IS_ELEMENTS_IN_USE, 3, 2, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM | PROP_FLAG_USE_IN_CRC),
	UCHAR_ELEMENT(PID_OBJECT_INDEX, PROP_FLAG_READ_ONLY, 7),
	END_OF_OBJECT
};

// interface object rgb feedback

#define RGB_DESCR "RGB led configuration"
STR_ELEMENT_PRGMEM(RGB_DESCR)

interfaceProperty rgbLegCfgObject[] = {
	UINT16_ELEMENT(PID_OBJECT_TYPE, PROP_FLAG_READ_ONLY, RGB_LED_CFG_OBJECT),
	DESCRIPTION_OBJECT_PRGMEM(PID_OBJECT_NAME, RGB_DESCR, 3),
	PROPERTY_OBJECT(PID_LOAD_STATE_CONTROL, PDT_CONTROL, 1, 3, 2, PROP_FLAG_WRITE_ENABLE),
	PROPERTY_OBJECT(PID_RGBLEG_CFG, PDT_GENERIC02, MAX_ELEMENTS_IS_ELEMENTS_IN_USE, 3, 2, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM | PROP_FLAG_USE_IN_CRC),
	UCHAR_ELEMENT(PID_OBJECT_INDEX, PROP_FLAG_READ_ONLY, 8),	
	END_OF_OBJECT
};

// interface object ambient sensors

#define AMBIENT_DESCR "Ambient sensor configuration"
STR_ELEMENT_PRGMEM(AMBIENT_DESCR)

interfaceProperty ambientSensorsCfgObject[] = {
	UINT16_ELEMENT(PID_OBJECT_TYPE, PROP_FLAG_READ_ONLY, AMBIENT_SENSOR_CFG_OBJECT),
	DESCRIPTION_OBJECT_PRGMEM(PID_OBJECT_NAME, AMBIENT_DESCR, 3),
	PROPERTY_OBJECT(PID_LOAD_STATE_CONTROL, PDT_CONTROL, 1, 3, 2, PROP_FLAG_WRITE_ENABLE),
	PROPERTY_OBJECT(PID_AMBIENT_SENSORS_CFG, PDT_UNSIGNED_INT, MAX_ELEMENTS_IS_ELEMENTS_IN_USE, 3, 2, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM | PROP_FLAG_USE_IN_CRC),
	PROPERTY_OBJECT(PID_AMBIENT_SENSORS_INTERVAL, PDT_UNSIGNED_INT, MAX_ELEMENTS_IS_ELEMENTS_IN_USE, 3, 2, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM | PROP_FLAG_USE_IN_CRC),
	PROPERTY_OBJECT(PID_AMBIENT_TEMPERATURE_OFFSET, PDT_EIB_FLOAT, MAX_ELEMENTS_IS_ELEMENTS_IN_USE, 3, 2, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM),
	PROPERTY_OBJECT(PID_AMBIENT_HUMIDITY_OFFSET, PDT_EIB_FLOAT, MAX_ELEMENTS_IS_ELEMENTS_IN_USE, 3, 2, PROP_FLAG_WRITE_ENABLE | PROP_FLAG_SAVE_IN_EEPROM),
	PROPERTY_OBJECT(PID_AMBIENT_TEMPERATURE, PDT_EIB_FLOAT, MAX_ELEMENTS_IS_ELEMENTS_IN_USE, 3, 0, PROP_FLAG_READ_ONLY),
	PROPERTY_OBJECT(PID_AMBIENT_HUMIDITY, PDT_EIB_FLOAT, MAX_ELEMENTS_IS_ELEMENTS_IN_USE, 3, 0, PROP_FLAG_READ_ONLY),
	UCHAR_ELEMENT(PID_OBJECT_INDEX, PROP_FLAG_READ_ONLY, 9),	
	END_OF_OBJECT
};


interfaceProperty endOfList[] = {
	END_OF_OBJECT
};

interfaceProperty *propertyObject[NUMBER_OF_OBJECTS] = {
	deviceObject,
	addressTableObject,
	associationTableObject,
	groupObjectTableObject,
	applicationObject1,
	applicationObject2,
	touchpadCfgObject,
	actionCfgObject,
	rgbLegCfgObject,
	ambientSensorsCfgObject,
	endOfList
};


interfaceObjectServerHandle IntfObj_Srv_Init(void *pMemory, uint16_t numBytes) {
	interfaceObjectServerHandle intfObjServ_handle;
	if (numBytes < sizeof(interfaceObjectServerObj))
		return ((interfaceObjectServerHandle)NULL);
	intfObjServ_handle = (interfaceObjectServerHandle)pMemory;
	return(intfObjServ_handle);
}

int8_t IntfObj_Srv_Setup(interfaceObjectServerHandle handle) {
	interfaceObjectServerObj *srv;
	srv = (interfaceObjectServerObj*) handle;

	// initialize objectIdx and PropertyIdx properties
	uint16_t oIdx = 0;
	uint16_t pIdx = 0;
	while ((propertyObject[oIdx])->propertyId)
	{
		while ((propertyObject[oIdx] + pIdx)->propertyId)
		{
			(propertyObject[oIdx] + pIdx)->objectIndex = oIdx;
			(propertyObject[oIdx] + pIdx)->propertyIndex = pIdx;
			pIdx++;
		}
		pIdx = 0;
		oIdx++;
	}
	
	srv->enable = 1;
	return 0;
}

int8_t IntfObj_Srv_registerObject(interfaceObjectServerHandle handle, prop_obj_e objIdx, application_property_id_e propId, void *object) {
	//interfaceObjectServerObj *srv;
	//srv = (interfaceObjectServerObj*) handle;
	
	if (objIdx >= NUMBER_OF_OBJECTS || (PropertyId_e)propId == (PropertyId_e)PID_TABLE)
		return -1;
	
	interfaceProperty *prop;
	prop = IntfObj_Srv_getObjByPropId(handle, objIdx, propId);
	if (prop)
	{
		prop->elements = object;
		if ((prop->maxElements == 0xffff) && *(uint16_t*)object != 0)
			prop->maxElements = SWAP_UINT16(*(uint16_t*)object);  // set max elements to elements in use
		return 0;
	}
	return -1;
}

int8_t IntfObj_Srv_registerTableObject(interfaceObjectServerHandle handle, prop_obj_e objIdx, PropertyId_e propId, void *object) {
	//interfaceObjectServerObj *srv;
	//srv = (interfaceObjectServerObj*) handle;
	
	if (objIdx >= NUMBER_OF_OBJECTS || propId != (PropertyId_e)PID_TABLE)
		return -1;
	
	interfaceProperty *prop;
	prop = IntfObj_Srv_getObjByPropId(handle, objIdx, propId);
	if (prop)
	{
		prop->elements = object;
		prop = IntfObj_Srv_getObjByPropId(handle, objIdx, PID_TABLE_REFERENCE);
		if (prop)
		{
			uint32_t ptr = 0;
			ptr = (intptr_t)object;
			((sLong*)(prop->elements))->value = SWAP_UINT32(ptr);
		}
		return 0;
	}
	return -1;
}

int8_t IntfObj_Srv_setMCTObject(interfaceObjectServerHandle handle, prop_obj_e objIdx) {
	if (objIdx >= NUMBER_OF_OBJECTS)
		return -1;
	
	interfaceProperty *prop;
	prop = IntfObj_Srv_getObjByPropId(handle, objIdx, PID_MCB_TABLE);
	if (prop == NULL)
		return -1;
	prop = IntfObj_Srv_getObjByPropId(handle, objIdx, PID_TABLE);
	uint8_t* elements = prop->elements;
	if (prop == NULL)
		return -1;
	uint16_t elementsInUse = IntfObj_getElementsInUse(prop);
	if (elementsInUse == 0)
		return -1;

	uint16_t segmentSize = (getPDTsize(prop->propertyDataType) * elementsInUse) + 2;

	CRC_t *crc;
	crc = &CRC;
	crc->CTRL = 0xC0; // reset

	crc->CHECKSUM1 = 0x1D; // initial value. documentation 03_05_01 page 34 reads 0xffffh, which is not correct.
	crc->CHECKSUM0 = 0x0F;
	
	crc->CTRL = 0x01; // activate CRC
	
	for (uint16_t i = 0; i < segmentSize; i++)
		crc->DATAIN = elements[i];
	
	prop = IntfObj_Srv_getObjByPropId(handle, objIdx, PID_MCB_TABLE);
	((sLong*)(prop->elements))->value = SWAP_UINT32((uint32_t)segmentSize);
	((sGeneric*)(prop->elements))->value[4] = 0; // bit 0: 0 CRC always valid, 1 CRC may become invalid
	((sGeneric*)(prop->elements))->value[5] = 0x30; // read level and write level
	((sGeneric*)(prop->elements))->value[6] = crc->CHECKSUM1;
	((sGeneric*)(prop->elements))->value[7] = crc->CHECKSUM0;
	return 0;
}

uint8_t getMaxObjIdx() { return (NUMBER_OF_OBJECTS); }
	
interfaceProperty *IntfObj_Srv_getObjByPropId(interfaceObjectServerHandle handle, prop_obj_e objIdx, PropertyId_e propId) {
	//interfaceObjectServerObj *srv;
	//srv = (interfaceObjectServerObj*) handle;
	
	if (objIdx >= NUMBER_OF_OBJECTS || !propId)
		return (interfaceProperty *)NULL;
		
	uint8_t i = 0;
	while ((propertyObject[objIdx] + i)->maxElements) {
		if ((propertyObject[objIdx] + i)->propertyId == propId)
			return (propertyObject[objIdx] + i);
		i++;
	}
	return (interfaceProperty *)NULL;
}

interfaceProperty *IntfObj_Srv_getObjByPropIndex(interfaceObjectServerHandle handle, prop_obj_e objIdx, uint8_t propIndex) {

	if (objIdx >= NUMBER_OF_OBJECTS)
		return (interfaceProperty *)NULL;
	
	uint16_t pIdx = 0;
	while ((propertyObject[objIdx] + pIdx)->maxElements)
	{
		if (!(propertyObject[objIdx] + pIdx)->propertyId)
			return (interfaceProperty*)NULL;
		if (pIdx == propIndex)
			return (propertyObject[objIdx] + propIndex);
		pIdx++;
	}
		
	return (interfaceProperty*)NULL;
}


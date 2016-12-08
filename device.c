/*
 * device.c
 *
 * Created: 21-8-2016 8:38:01
 *  Author: Paul
 */ 

#include "device.h"
#include "interfaceObjectServer.h"
#include "eepromStorage.h"
#include <avr/eeprom.h>

#define PROGRAM_LED_BUTTON_PORT		PORTD
#define PROGRAM_LED_PIN				0x02  // bit 1 = 0x02
#define PROGRAM_BUTTON_PIN			0x01  // bit 0 = 0x01

deviceHandle deviceInit(void *pMemory, uint16_t numBytes) {
	deviceHandle device_handle;
	if (numBytes < sizeof(deviceObj))
	return ((deviceHandle)NULL);
	device_handle = (deviceHandle)pMemory;
	return(device_handle);
}

int8_t deviceSetup(deviceHandle handle, A_InterfaceHandle AI_Hndl) {
	deviceObj *device;
	device = (deviceObj *)handle;
	
	device->AI_Hndl = AI_Hndl;
	
	// register callback functions
	
	AI_Hndl->deviceObjectHndl = handle;
	AI_Hndl->AI_deviceObject_ind = &devicePropInd;
	
	AI_Hndl->ownAddrIndHndl = handle;
	AI_Hndl->AI_ownAddr_ind = &ownAddressInd;
	
	// configure programming button and led port
	
	PROGRAM_LED_BUTTON_PORT.DIRSET = PROGRAM_LED_PIN; // configure programming led as output
	PROGRAM_LED_BUTTON_PORT.DIRCLR = PROGRAM_BUTTON_PIN;
	*(&(PROGRAM_LED_BUTTON_PORT.PIN0CTRL) + PROGRAM_BUTTON_PIN - 1) = _BV(PORT_OPC0_bp) | _BV(PORT_OPC1_bp); // configure pull-up for programming button.
	
	device->debounceCnt = 0;
	device->btnState = PROGRAM_LED_BUTTON_PORT.IN & PROGRAM_BUTTON_PIN;
	device->enable = 1;
	device->state = DEVICE_INIT;
	
	device->progmode.cnt = SWAP_UINT16(1);
	device->progmode.value = 0;
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_PROGMODE, &device->progmode);
	
	device->deviceAddr.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_DEVICE_ADDR, &device->deviceAddr);
	device->subNetAddr.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_SUBNET_ADDR, &device->subNetAddr);
	
	device->apduLength.cnt = SWAP_UINT16(1);
	device->apduLength.value = SWAP_UINT16(MAX_APDU_LENGTH);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_MAX_APDULENGTH, &device->apduLength);
	
	device->routingCount.cnt = SWAP_UINT16(1);
	device->routingCount.value = ROUTING_COUNT;
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_ROUTING_COUNT, &device->routingCount);
	
	device->maxRetry.cnt = SWAP_UINT16(1);
	device->maxRetry.value = MAX_RETRY_COUNT;
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_MAX_RETRY_COUNT, &device->maxRetry);
	
	N_set_hopCnt(AI_Hndl->A_Layer.T_layer.N_layerHnd, &device->routingCount.value); // set pointer to address of hop count property value
	
	T_layerSetMaxTDSULength(AI_Hndl->A_Layer.T_layerHnd, &device->apduLength.value); // set pointer to address of APDU length property value
	
	T_layerSetMaxRetry(AI_Hndl->A_Layer.T_layerHnd, &device->maxRetry.value); // set pointer to address of max retry property value
	
	N_set_ownAddress(AI_Hndl->A_Layer.T_layer.N_layerHnd, &device->ownAddress); // set pointer to address of own address field
	
	uint16_t ownAddr = eeprom_read_word(EEPROM_OWN_ADDRESS);
		
	if (!ownAddr || (ownAddr == 0xffff))
		ownAddr = 0x1407; // default address
	
	deviceSetOwnAddress(device, ownAddr);

	uint8_t maxRetry = eeprom_read_byte(EEPROM_MAX_RETRY);
	if (deviceSetMaxRetry(handle, maxRetry) < 0)
		deviceSetMaxRetry(handle, MAX_RETRY_COUNT);
			
	uint8_t routingCnt = eeprom_read_byte(EEPROM_HOP_COUNT);
	if (deviceSetRoutingCnt(handle, routingCnt) < 0)
		deviceSetRoutingCnt(handle, ROUTING_COUNT >> 4);

	uint16_t maxAPDU = eeprom_read_word(EEPROM_MAX_UPDU_LENGTH);
	if (deviceSetMaxAPDU(handle, maxAPDU))
		deviceSetMaxAPDU(handle, MAX_APDU_LENGTH);
	
	return 0;
}

int8_t deviceService(deviceHandle handle) {
	deviceObj *device;
	device = (deviceObj *)handle;
	if (!device->enable)
		return -1;
	
	if ((PROGRAM_LED_BUTTON_PORT.IN & PROGRAM_BUTTON_PIN) != device->btnState) {
		device->btnState = PROGRAM_LED_BUTTON_PORT.IN & PROGRAM_BUTTON_PIN;
		device->debounceCnt = 10;
	}
	if (device->debounceCnt) {
		device->debounceCnt--;
		if (!device->debounceCnt && !(device->btnState & PROGRAM_BUTTON_PIN)) {
			if (getDeviceState(handle) == DEVICE_PROGRAMMING) {
				setDeviceState(handle, DEVICE_NORMAL);
				device->progmode.value = 0;
			} else if (getDeviceState(handle) == DEVICE_NORMAL) {
				setDeviceState(handle, DEVICE_PROGRAMMING);
				device->progmode.value = 1;
			}
		}
	}
	
	if (getDeviceState(handle) == DEVICE_PROGRAMMING)
		PROGRAM_LED_BUTTON_PORT.OUTSET = PROGRAM_LED_PIN;
	else
		PROGRAM_LED_BUTTON_PORT.OUTCLR = PROGRAM_LED_PIN;
	
	return 0;	
}

int8_t setDeviceState(deviceHandle handle, device_state_e state) {
	deviceObj *device;
	device = (deviceObj *)handle;
	if (!device->enable)
		return -1;
	device->state = state;
	return 0;
}

device_state_e getDeviceState(deviceHandle handle) {
	deviceObj *device;
	device = (deviceObj *)handle;
	if (!device->enable)
		return -1;
	return device->state;
}

int16_t devicegGetOwnAddress(deviceHandle handle) {
	deviceObj *device;
	device = (deviceObj *)handle;
	if (!device->enable)
		return 0;
	return device->ownAddress;
}

int8_t deviceSetOwnAddress(deviceHandle handle, uint16_t addr) {
	deviceObj *device;
	device = (deviceObj *)handle;
	if (!device->enable)
		return -1;
	if (!addr || addr == 0xffff)
		return -1;
	device->ownAddress = addr;
	device->subNetAddr.value = addr >> 8;
	device->deviceAddr.value = addr;

	if (addr != eeprom_read_word(EEPROM_OWN_ADDRESS))
		eeprom_write_word(EEPROM_OWN_ADDRESS, addr);

	return 0;
}

int8_t deviceSetRoutingCnt(deviceHandle handle, uint8_t routingCnt) {
	deviceObj *device;
	device = (deviceObj *)handle;
	if ( !routingCnt || routingCnt > 7)
		return -1;
	device->routingCount.value = (routingCnt << 4);
	
	if (routingCnt != eeprom_read_byte(EEPROM_HOP_COUNT))
		eeprom_write_byte(EEPROM_HOP_COUNT, routingCnt);
	
	return 0;
}

int8_t deviceSetMaxRetry(deviceHandle handle, uint8_t maxRetry) {
	deviceObj *device;
	device = (deviceObj *)handle;
	if (maxRetry > 8)
		return -1;
	device->maxRetry.value = maxRetry;
	
	if (maxRetry != eeprom_read_byte(EEPROM_MAX_RETRY))
		eeprom_write_byte(EEPROM_MAX_RETRY, maxRetry);
	
	return 0;
}

int8_t deviceSetMaxAPDU(deviceHandle handle, uint16_t maxAPDU) {
	deviceObj *device;
	device = (deviceObj *)handle;
	if (maxAPDU < 15 || maxAPDU > MAX_APDU_LENGTH)
		return -1;
	device->apduLength.value = SWAP_UINT16(maxAPDU);
	
	if (maxAPDU != eeprom_read_word(EEPROM_MAX_UPDU_LENGTH))
		eeprom_write_word(EEPROM_MAX_UPDU_LENGTH, maxAPDU);
	
	return 0;
}

int8_t ownAddressInd(void *parentHandle, uint16_t newAddr) {
	deviceObj *device;
	device = (deviceObj *)parentHandle;

	return (deviceSetOwnAddress(device, newAddr) >= 0) ? 1 : 0;
}

int8_t devicePropInd(void *parentHandle, interfaceProperty *prop) {
	deviceObj *device;
	device = (deviceObj *)parentHandle;
	
	switch(prop->propertyId)
	{
		case PID_PROGMODE:
		{
			volatile uint8_t progMode = ((uint8_t*)prop->elements)[0] & 0x01;
			if (progMode == 1)
				setDeviceState(device, DEVICE_PROGRAMMING);
			if (progMode == 0 && getDeviceState(device) == DEVICE_PROGRAMMING)
				setDeviceState(device, DEVICE_NORMAL);
			return 1;
		}
		break;
		case PID_ROUTING_COUNT:
		{
			uint8_t hopCnt = ((uint8_t*)prop->elements)[0] >> 4;
			return (deviceSetMaxRetry(parentHandle, hopCnt) >= 0) ? 1 : 0;
		}
		break;
		case  PID_MAX_APDULENGTH:
		{
			uint16_t maxAPDULength = SWAP_UINT16(*(uint16_t*)prop->elements);
			return (deviceSetMaxAPDU(parentHandle, maxAPDULength) >= 0) ? 1 : 0;
		}
		break;
		case PID_MAX_RETRY_COUNT:
		{
			uint8_t maxRetry = ((uint8_t*)prop->elements)[0];
			return (deviceSetMaxRetry(parentHandle, maxRetry) >= 0) ? 1 : 0;
		}
		break;
		default:
		break;
	}
	return 1;
}
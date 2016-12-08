/*
 * device.h
 *
 * Created: 21-8-2016 8:37:50
 *  Author: Paul
 */ 


#ifndef DEVICE_H_
#define DEVICE_H_

#include "clocks.h"
#include "applicationInterface.h"

// max apdu length
#define MAX_APDU_LENGTH			56
// routing count
#define ROUTING_COUNT			0x60  // in accordance with specification. hop count in high nibble.
// max retry count
#define MAX_RETRY_COUNT			2

typedef enum {
	DEVICE_RESET,
	DEVICE_INIT,
	DEVICE_NORMAL,
	DEVICE_PROGRAMMING
} device_state_e;

typedef struct _deviceObj {
	uint8_t					enable;
	A_InterfaceHandle		AI_Hndl;
	device_state_e			state;
	uint16_t				ownAddress;
	uint8_t					btnState;
	uint8_t					debounceCnt;
	uint8_t					propertyChgd;
	
	// interface object
	
	sByte					progmode;
	sByte					deviceAddr;
	sByte					subNetAddr;
	sWord					apduLength;
	sByte					routingCount;
	sByte					maxRetry;
	
} deviceObj;

typedef struct _deviceObj *deviceHandle;


deviceHandle deviceInit(void *pMemory, uint16_t numBytes);

int8_t deviceSetup(deviceHandle handle, A_InterfaceHandle AI_Hndl);

int8_t deviceService(deviceHandle handle);

//void AI_Progmode_ind_cb(void *parent, uint8_t *value);

int8_t setDeviceState(deviceHandle handle, device_state_e state);

device_state_e getDeviceState(deviceHandle handle);

int16_t devicegGetOwnAddress(deviceHandle handle);

int8_t deviceSetOwnAddress(deviceHandle handle, uint16_t addr);

int8_t deviceSetRoutingCnt(deviceHandle handle, uint8_t routingCnt);

int8_t deviceSetMaxRetry(deviceHandle handle, uint8_t maxRetry);

int8_t deviceSetMaxAPDU(deviceHandle handle, uint16_t maxAPDU);

int8_t ownAddressInd(void *parentHandle, uint16_t newAddr);

int8_t devicePropInd(void *parentHandle, interfaceProperty *prop);

#endif /* DEVICE_H_ */
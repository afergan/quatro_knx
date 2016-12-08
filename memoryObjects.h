/*
 * memoryObjects.h
 *
 * Created: 1-8-2016 9:53:42
 *  Author: Paul
 */ 


#ifndef MEMORYOBJECTS_H_
#define MEMORYOBJECTS_H_

#include "clocks.h"
#include "interfaceObjectServer.h"


#define SYSTEM_STATE_ADDR		0x0060

#define SYSTEM_STATE_PROG_bp			0 // set means in programming mode
#define SYSTEM_STATE_LLM_bp				1 // set means normal operation
#define SYSTEM_STATE_TLE_bp				2 // set means transport layer active
#define SYSTEM_STATE_ALE_bp				3 // set means application layer active
#define SYSTEM_STATE_SE_bp				4 // set means serial interface is active
#define SYSTEM_STATE_UE_bp				5 // set means user running
#define SYSTEM_STATE_DM_bp				6 // set means download mode active
#define SYSTEM_STATE_PARITY_bp			7 // even parity for bits 0 - 6

typedef struct _byteObject {
	uint16_t		address;
	uint8_t			value;
} byteObject;

struct _statusBytes {
	byteObject		addrtable_load_status;
	byteObject		assoctable_load_status;		
	byteObject		application_load_status;
	byteObject		PeiprogLoadStatus;
	byteObject		application_run_state_control;
};

#define NUM_OF_CTRL_BYTES	sizeof(struct _statusBytes)/ sizeof(byteObject)

typedef struct _memObj{
	union {
		struct _statusBytes		descr;
		byteObject				arr[NUM_OF_CTRL_BYTES];
	} ctrlBytes;
} memObj;

typedef struct _memObj *memHandle;

memHandle memInit(void *pMemory, uint16_t numBytes);

int8_t memSetup(memHandle handle);

#endif /* MEMORYOBJECTS_H_ */
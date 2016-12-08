/*
 * eventFifo.h
 *
 * Created: 19-6-2016 7:53:06
 *  Author: Paul
 */ 


#ifndef EVENTFIFO_H_
#define EVENTFIFO_H_

#include "touchPad.h"

#define FIFO_TIMEOUT_MS		100

typedef enum {
	FB_STATE_FREE,
	FB_STATE_OCCUPIED
} event_fifo_state_e;

struct _eventFifoBuffer {
	event_fifo_state_e				state;
	struct _eventFifoBuffer			*next;
	uint32_t						timestamp;
	touch_event_e					tevent;
	touch_state_e					pad;
};


typedef struct _eventFifoObj {
	uint8_t						enabled;
	uint8_t						fifoLength;
	struct _eventFifoBuffer		*first;
	struct _eventFifoBuffer		*rbuffer;
} eventFifoObj;

typedef struct _eventFifoObj *eventFifoHandle;

eventFifoHandle eventFifoInit(void *pMemory, uint16_t numBytes);

int8_t eventFifoSetup(eventFifoHandle handle, struct _eventFifoBuffer *fb, uint8_t fifolen);

struct _eventFifoBuffer *allocNewEventFifoBuffer(eventFifoHandle handle);

int8_t freeEventFifoBuffer(eventFifoHandle handle, struct _eventFifoBuffer *bufP);

int8_t pushEventFifoBuffer(eventFifoHandle handle, touch_event_e tevent, touch_state_e pad);

int8_t popEventFifoBuffer(eventFifoHandle handle, touch_event_e *tevent, touch_state_e *pad);

/*
int8_t getNumberOfFilledEventFifoBuffers(eventFifoHandle handle);

int8_t getNumberOfFreeEventFifoBuffers(eventFifoHandle handle);*/

struct _eventFifoBuffer *getNextEventFifo(eventFifoHandle handle, struct _eventFifoBuffer *previous);

#endif /* EVENTFIFO_H_ */
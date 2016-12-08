/*
 * eventFifo.c
 *
 * Created: 19-6-2016 7:53:21
 *  Author: Paul
 */ 

#include "clocks.h"
#include "interrupt.h"

#include <string.h>
#include "eventFifo.h"

eventFifoHandle eventFifoInit(void *pMemory, uint16_t numBytes)
{
	eventFifoHandle eventFifo_handle;
	if (numBytes < sizeof(eventFifoObj))
	return ((eventFifoHandle)NULL);
	eventFifo_handle = (eventFifoHandle)pMemory;
	return(eventFifo_handle);
}

int8_t eventFifoSetup(eventFifoHandle handle, struct _eventFifoBuffer *fb, uint8_t fifolen)
{
	eventFifoObj *fifo;
	fifo = (eventFifoObj *) handle;
	
	fifo->rbuffer = fb;
	fifo->fifoLength = fifolen;
	for (int i = 0; i < fifo->fifoLength; i++) {
		fifo->rbuffer[i].state = FB_STATE_FREE;
		fifo->rbuffer[i].next = NULL;
	}

	fifo->enabled = 1;

	return 0;
}

struct _eventFifoBuffer *allocNewEventFifoBuffer(eventFifoHandle handle){
	eventFifoObj *fifo;
	fifo = (eventFifoObj *) handle;
	if (fifo->enabled != 1)
	return NULL;

	cli();
	struct _eventFifoBuffer *rb = NULL;
	for (int i=0; i < fifo->fifoLength; i++){
		if (fifo->rbuffer[i].state == FB_STATE_FREE)
		{
			rb = &(fifo->rbuffer[i]);
			break;
		}
	}
	if (rb != NULL) {
		if (fifo->first == NULL || fifo->first == rb)
		fifo->first = rb;
		else {
			struct _eventFifoBuffer *nxt = fifo->first;
			for (int i = 0; i < fifo->fifoLength; i++) {
				if (nxt->next != NULL)
				nxt = nxt->next;
				else {
					nxt->next = rb;
					break;
				}
			}
		}
	}
	sei();
	
	return rb;
}

int8_t freeEventFifoBuffer(eventFifoHandle handle, struct _eventFifoBuffer *bufP){
	eventFifoObj *fifo;
	fifo = (eventFifoObj *) handle;
	if (fifo->enabled != 1)
	return -1;

	cli();
	
	bufP->state = FB_STATE_FREE;
	if (bufP->next == NULL) {
		if (fifo->first == bufP)
		fifo->first = NULL;
		else {
			struct _eventFifoBuffer *nxt = fifo->first;
			for (int i = 0; i < fifo->fifoLength; i++) {
				if (nxt->next != bufP)
				nxt = nxt->next;
				else {
					nxt->next = NULL;
					break;
				}
			}
		}
		} else {
		if (fifo->first == bufP) {
			fifo->first = bufP->next;
			bufP->next = NULL;
		}
		else {
			struct _eventFifoBuffer *nxt = fifo->first;
			for (int i = 0; i < fifo->fifoLength; i++) {
				if (nxt->next != bufP)
				nxt = nxt->next;
				else {
					nxt->next = bufP->next;
					bufP->next = NULL;
					break;
				}
			}
		}
	}

	sei();
	
	return 0;
}

int8_t pushEventFifoBuffer(eventFifoHandle handle, touch_event_e tevent, touch_state_e pad) {
	eventFifoObj *fifo;
	fifo = (eventFifoObj *) handle;
	if (fifo->enabled != 1)
	return -1;
	int8_t result = 0;
	
	cli();
	
	struct _eventFifoBuffer *pb = allocNewEventFifoBuffer(handle);
	if (pb!= NULL) {
		pb->tevent = tevent;
		pb->pad = pad;
		pb->timestamp = getTimestamp_ms();
		pb->state = FB_STATE_OCCUPIED;
		result = 0;
	} else
	result = - 1;
	
	sei();
	
	return result;
}

int8_t popEventFifoBuffer(eventFifoHandle handle, touch_event_e *tevent, touch_state_e *pad) {
	volatile eventFifoObj *fifo;
	fifo = (eventFifoObj *) handle;
	if (fifo->enabled != 1)
		return -1;
	int8_t result = 0;
	
/*
	timeoutTmr timeout;
	setupTimeoutTmr(&timeout, FIFO_TIMEOUT_MS);*/

	cli();
	
/*
	while ((fifo->first == NULL || fifo->first->state != FB_STATE_OCCUPIED) && isNotTimedout(&timeout));
	if (isNotTimedout(&timeout)) {
		*tevent = fifo->first->tevent;
		*pad = fifo->first->pad;
		freeEventFifoBuffer(handle, fifo->first);
	} else {
		result = -1;
	}*/

	if ((fifo->first != NULL) && (fifo->first->state == FB_STATE_OCCUPIED))
	{
		*tevent = fifo->first->tevent;
		*pad = fifo->first->pad;
		freeEventFifoBuffer(handle, fifo->first);
		result = 1;		
	}
	sei();
	
	return result;
}

/*
int8_t getNumberOfFilledEventFifoBuffers(eventFifoHandle handle){
	eventFifoObj *fifo;
	fifo = (eventFifoObj *) handle;
	if (fifo->enabled != 1)
	return -1;
	uint8_t sum = 0;
	for (int i=0; i < fifo->fifoLength; i++){
		if (fifo->rbuffer[i].state == FB_STATE_OCCUPIED)
		sum++;
	}
	return sum;
}

int8_t getNumberOfFreeEventFifoBuffers(eventFifoHandle handle){
	eventFifoObj *fifo;
	fifo = (eventFifoObj *) handle;
	if (fifo->enabled != 1)
	return -1;
	uint8_t sum = 0;
	for (int i=0; i < fifo->fifoLength; i++){
		if (fifo->rbuffer[i].state == FB_STATE_FREE)
		sum++;
	}
	return sum;
}*/

struct _eventFifoBuffer *getNextEventFifo(eventFifoHandle handle, struct _eventFifoBuffer *previous) {
	eventFifoObj *fifo;
	fifo = (eventFifoObj *) handle;
	if (fifo->enabled != 1)
	return NULL;
	if (previous == NULL && fifo->first->state == FB_STATE_OCCUPIED)
	return fifo->first;
	if (previous == NULL && fifo->first->state != FB_STATE_OCCUPIED)
	return NULL;
	if (fifo->first == NULL)
	return NULL;
	
	struct _eventFifoBuffer *nxt = fifo->first;
	for (int i = 0; i <fifo->fifoLength; i++) {
		if (previous == nxt && nxt->next->state == FB_STATE_OCCUPIED)
		return nxt->next;
		if (nxt != NULL)
		nxt = nxt->next;
		else
		break;
	}
	return NULL;
}
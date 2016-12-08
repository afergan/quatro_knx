/*
 * fifo.c
 *
 * Created: 29-5-2016 11:59:56
 *  Author: Paul
 */ 

#include "clocks.h"
#include "interrupt.h"
#include "util\atomic.h"
#include <string.h>
#include "fifo.h"


fifoHandle fifoInit(void *pMemory, uint16_t numBytes)
{
	fifoHandle fifo_handle;
	if (numBytes < sizeof(fifoObj))
		return ((fifoHandle)NULL);
	fifo_handle = (fifoHandle)pMemory;
	return(fifo_handle);
}

int8_t fifoSetup(fifoHandle handle, fifoBuffer *fb, uint8_t fifolen)
{
	fifoObj *fifo;
	fifo = (fifoObj *) handle;
	
	fifo->rbuffer = fb;
	fifo->fifoLength = fifolen;
	for (int i = 0; i < fifo->fifoLength; i++) {
		fifo->rbuffer[i].state = STATE_FREE;
		fifo->rbuffer[i].next = NULL;
	}

	fifo->enabled = 1;

	return 0;
}

fifoBuffer *allocNewFifoBuffer(fifoHandle handle){
	fifoObj *fifo;
	fifo = (fifoObj *) handle;
	if (fifo->enabled != 1)
		return NULL;

	fifoBuffer *rb = NULL;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		for (int i=0; i < fifo->fifoLength; i++){
			if (fifo->rbuffer[i].state == STATE_FREE)
			{
				rb = &(fifo->rbuffer[i]);
				break;
			}
		}
		if (rb != NULL) {
			if (fifo->first == NULL || fifo->first == rb)
				fifo->first = rb;
			else {
				fifoBuffer *nxt = fifo->first;
				for (int i = 0; i < fifo->fifoLength; i++) {
					if (nxt->next != NULL)
					nxt = nxt->next;
					else {
						nxt->next = rb;
						break;
					}
				}
			}
			rb->state = STATE_FILLING;
		}
	}
	return rb;
}

int8_t freeFifoBuffer(fifoHandle handle, fifoBuffer *bufP){
	fifoObj *fifo;
	fifo = (fifoObj *) handle;
	if (fifo->enabled != 1 || !bufP)
		return -1;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		bufP->state = STATE_FREE;
		bufP->length = 0;
		if (bufP->next == NULL) {
			if (fifo->first == bufP)
			fifo->first = NULL;
			else {
				fifoBuffer *nxt = fifo->first;
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
				fifoBuffer *nxt = fifo->first;
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
	}
	return 0;
}

int8_t pushFifoBuffer(fifoHandle handle, uint8_t *data, uint16_t length) {
	fifoObj *fifo;
	fifo = (fifoObj *) handle;
	if (fifo->enabled != 1)
		return -1;
	int8_t result = 0;
	
	cli();
	
	fifoBuffer *pb = allocNewFifoBuffer(handle);
	if (pb!= NULL) {
		memcpy(pb->data, data, length);
		pb->length = length;
		pb->timestamp = getTimestamp_ms();
		pb->state = STATE_OCCUPIED;
		result = 0;
	} else
	result = - 1;
	
	sei();
	
	return result;
}

fifoBuffer *getFirstFifo(fifoHandle handle) {
	fifoObj *fifo;
	fifo = (fifoObj *) handle;
	uint8_t dataReady = 1;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		dataReady &= (fifo->enabled);
		dataReady &= (fifo->first != NULL);
		dataReady &= (fifo->first->state == STATE_OCCUPIED);
	}
	if (dataReady)
		return fifo->first;
	return NULL;
}

int8_t getNumberOfFilledFifoBuffers(fifoHandle handle){
	fifoObj *fifo;
	fifo = (fifoObj *) handle;
	if (fifo->enabled != 1)
		return -1;
	uint8_t sum = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		for (int i=0; i < fifo->fifoLength; i++){
			if (fifo->rbuffer[i].state == STATE_OCCUPIED)
			sum++;
		}
	}
	return sum;
}

int8_t getNumberOfFreeFifoBuffers(fifoHandle handle){
	fifoObj *fifo;
	fifo = (fifoObj *) handle;
	if (fifo->enabled != 1)
		return -1;
	uint8_t sum = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		for (int i=0; i < fifo->fifoLength; i++){
			if (fifo->rbuffer[i].state == STATE_FREE)
			sum++;
		}
	}
	return sum;
}

int8_t readFifoBufferWithMatch(fifoHandle handle, uint8_t *data, uint16_t length, uint8_t matchByte) {
	volatile fifoObj *fifo;
	fifo = (fifoObj *) handle;
	if (fifo->enabled != 1)
		return -1;

	if (length == 0 || length > FIFO_BUFFER_SIZE)
	return -1;
	
	int8_t result = 0;
	//uint32_t timeout = clock.timestamp + FIFO_TIMEOUT_MS;
	timeoutTmr timeout;
	setupTimeoutTmr(&timeout, FIFO_TIMEOUT_MS);
	
	fifoBuffer *matchFifo = NULL;
	uint8_t match = 0;
	
	do {
		matchFifo = getNextFifo(handle, matchFifo);
		if (matchFifo != NULL) {
			if (matchFifo->data[0] == matchByte) {
				match = 1;
			}
		}
	} while (!match && isNotTimedout(&timeout));
	
	cli();
	
	if (match) {
		memcpy(data, matchFifo->data, length);
		length = matchFifo->length;
		freeFifoBuffer(handle, matchFifo);
		result = length;
	} else
	result = -1;
	
	sei();
	
	return result;
}

fifoBuffer *getNextFifo(fifoHandle handle, fifoBuffer *previous) {
	fifoObj *fifo;
	fifo = (fifoObj *) handle;
	if (fifo->enabled != 1)
		return NULL;
	if (previous == NULL && fifo->first->state == STATE_OCCUPIED)
		return fifo->first;
	if (previous == NULL && fifo->first->state != STATE_OCCUPIED)
		return NULL;
	if (fifo->first == NULL)
		return NULL;
	
	fifoBuffer *nxt = fifo->first;
	for (int i = 0; i <fifo->fifoLength; i++) {
		if (previous == nxt && nxt->next->state == STATE_OCCUPIED)
			return nxt->next;
		if (nxt != NULL)
			nxt = nxt->next;
		else
			break;
	}
	return NULL;
}

int16_t popNumberOfBytes(fifoHandle handle){
	fifoObj *fifo;
	fifo = (fifoObj *) handle;
	if (fifo->enabled != 1)
	return -1;
	if (fifo->first->state == STATE_OCCUPIED)
		return fifo->first->length;
	else
		return -1;
}
/*
 * cmdFifo.c
 *
 * Created: 15-7-2016 19:46:42
 *  Author: Paul
 */ 

#include "clocks.h"
#include "interrupt.h"

#include <string.h>
#include "cmdHistory.h"


cmdHistoryHandle cmdHistInit(void *pMemory, uint16_t numBytes)
{
	cmdHistoryHandle hist_handle;
	if (numBytes < sizeof(cmdHistoryObj))
	return ((cmdHistoryHandle)NULL);
	hist_handle = (cmdHistoryHandle)pMemory;
	return(hist_handle);
}

int8_t cmdHistSetup(cmdHistoryHandle handle, struct _cmdBuffer *fb, uint8_t histLen)
{
	cmdHistoryObj *hist;
	hist = (cmdHistoryObj *) handle;
	
	hist->stack = fb;
	hist->historyLength = histLen;
	
		
	cmdHistReset(handle);

	return 0;
}

int8_t cmdHistReset(cmdHistoryHandle handle) {
	cmdHistoryObj *hist;
	hist = (cmdHistoryObj *) handle;
	for (int i = 0; i < hist->historyLength; i++) {
		hist->stack[i].state = BUF_FREE;
		hist->stack[i].next = NULL;
	}
	hist->first = NULL;
	hist->traverse =NULL;
	return 0;
}

int8_t cmdHistEnable(cmdHistoryHandle handle) {
	cmdHistoryObj *hist;
	hist = (cmdHistoryObj *) handle;

	hist->enabled = 1;
	return 0;
}

int8_t cmdHistDisable(cmdHistoryHandle handle) {
	cmdHistoryObj *hist;
	hist = (cmdHistoryObj *) handle;
	hist->enabled = 0;
	return 0;
}

struct _cmdBuffer *allocNewCmdBuffer(cmdHistoryHandle handle){
	cmdHistoryObj *hist;
	hist = (cmdHistoryObj *) handle;
	if (hist->enabled != 1)
	return NULL;
	// find new buffer location
	struct _cmdBuffer *rb = NULL;
	struct _cmdBuffer *lastAlloc = NULL;
	for (int i=0; i < hist->historyLength; i++){
		if (!hist->stack[i].next)
			lastAlloc = &(hist->stack[i]);
		if (hist->stack[i].state == BUF_FREE)
		{
			rb = &(hist->stack[i]);
			rb->state = BUF_FILLING;
			break;
		}
	}
	// chain new buffer location to previous
	if (rb) {
		if (hist->first == NULL || hist->first == rb)
		{
			hist->first = rb;
		}
		else
		{
			rb->next = hist->first;
			hist->first = rb;
		}
	}
	else // buffer full. shift out first
	{
		if (lastAlloc)
		{
			for (int i = 0; i < hist->historyLength; i++) {
				if (hist->stack[i].next == lastAlloc)
				{
					hist->stack[i].next = NULL;
					break;
				}
			}
			rb = lastAlloc;
			rb->next = hist->first;
			hist->first = rb;		
		}
	}
	return rb;
}


int8_t pushCmdBuffer(cmdHistoryHandle handle, uint8_t *data, int8_t length) {
	cmdHistoryObj *hist;
	hist = (cmdHistoryObj *) handle;
	if (hist->enabled != 1)
		return -1;
	int8_t result = 0;

	if (hist->traverse) // overwrite cli store
	{
		if (length)
		{
			memcpy(hist->first->data, data, length);
			hist->first->length = length;
		}
		else
		{
			struct _cmdBuffer *freeThis = hist->first; // do not store empty entry in history
			hist->first = freeThis->next;
			freeThis->next = NULL;
			freeThis->length = 0;
			freeThis->state = BUF_FREE;
		}
		hist->traverse = NULL;
	}
	else if (length)
	{
		struct _cmdBuffer *pb = allocNewCmdBuffer(handle);
		if (pb!= NULL) {
			memcpy(pb->data, data, length);
			pb->length = length;
			pb->state = BUF_OCCUPIED;
			result = 0;
		} else
			result = - 1;		
	}
	return result;
}

int8_t getNextCmdfromHistory(cmdHistoryHandle handle, uint8_t *data, uint8_t *length) {
	cmdHistoryObj *hist;
	hist = (cmdHistoryObj *) handle;
	if (hist->enabled != 1 || !hist->first)
		return -1;
	if (hist->traverse)
	{
		if (hist->traverse == hist->first)
		{
			memcpy(hist->first->data, data, *length);
			hist->first->length = *length;			
		}
		if (hist->traverse->next)
			hist->traverse = hist->traverse->next;
	}	
	else
	{
		hist->traverse = hist->first;
		struct _cmdBuffer *pb = allocNewCmdBuffer(handle); // store cli input before retrieving old stuff
		if (pb!= NULL) {
			memcpy(pb->data, data, *length);
			pb->length = *length;
			pb->state = BUF_OCCUPIED;
		}
	}
	if (hist->traverse->length <= CMD_LINE_LENGTH)
	{
		memcpy(data, hist->traverse->data, hist->traverse->length);
		*length = hist->traverse->length;
		return hist->traverse->length;
	}
	else
		return -1;
}

int8_t getPreviousCmdfromHistory(cmdHistoryHandle handle, uint8_t *data, uint8_t *length) {
	cmdHistoryObj *hist;
	hist = (cmdHistoryObj *) handle;
	if (hist->enabled != 1 || !hist->first)
		return -1;
	if (hist->traverse)
	{
		for (int i = 0; i < hist->historyLength; i++) {
			if (hist->stack[i].next == hist->traverse)
			{
				hist->traverse = &(hist->stack[i]);
				break;
			}
		}
		if (hist->traverse->length <= CMD_LINE_LENGTH)
		{
			memcpy(data, hist->traverse->data, hist->traverse->length);
			*length = hist->traverse->length;
			return hist->traverse->length;
		}
		else
			return -1;	
	}
	else
		return -1;
}


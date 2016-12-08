/*
 * fifo.h
 *
 * Created: 29-5-2016 11:59:41
 *  Author: Paul
 */ 


#ifndef FIFO_H_
#define FIFO_H_

#define FIFO_BUFFER_SIZE	64
#define FIFO_TIMEOUT_MS		100

typedef enum {
	STATE_FREE,
	STATE_FILLING,
	STATE_OCCUPIED
} fifo_state;

typedef struct _fifoBuffer{
	fifo_state						state;
	volatile uint16_t				length;
	struct _fifoBuffer				*next;
	uint32_t						timestamp;
	uint8_t							data[FIFO_BUFFER_SIZE];
} fifoBuffer;

typedef struct _fifoObj {
	uint8_t					enabled;
	uint8_t					fifoLength;
	fifoBuffer				*first;
	fifoBuffer				*rbuffer;
} fifoObj;

typedef struct _fifoObj *fifoHandle;

fifoHandle fifoInit(void *pMemory, uint16_t numBytes);

int8_t fifoSetup(fifoHandle handle, fifoBuffer *fb, uint8_t fifolen);

fifoBuffer *allocNewFifoBuffer(fifoHandle handle);

int8_t freeFifoBuffer(fifoHandle handle, fifoBuffer *bufP);

int8_t pushFifoBuffer(fifoHandle handle, uint8_t *data, uint16_t length);

fifoBuffer *getFirstFifo(fifoHandle handle);

int8_t getNumberOfFilledFifoBuffers(fifoHandle handle);

int8_t getNumberOfFreeFifoBuffers(fifoHandle handle);

int8_t readFifoBufferWithMatch(fifoHandle handle, uint8_t *data, uint16_t length, uint8_t matchByte);

fifoBuffer *getNextFifo(fifoHandle handle, fifoBuffer *previous);

int16_t popNumberOfBytes(fifoHandle handle);


#endif /* FIFO_H_ */
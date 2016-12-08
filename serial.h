/*
 * serial.h
 *
 * Created: 18-5-2016 7:19:18
 *  Author: Paul
 */ 


#ifndef SERIAL_H_
#define SERIAL_H_

#include "clocks.h"
#include "telegram.h"

#define SERIAL_BUFFER_SIZE			64
#define SERIAL_TIMEOUT_MS			100

#define  U_L_DATA_START				0x80
#define  U_L_DATA_CONTINUE			0x81
#define  U_L_DATA_END				0x40

typedef enum {
	PARITY_NONE,
	PARITY_EVEN,
	PARITY_ODD
	} parity_e;

typedef enum {
	STATE_WAITING,
	STATE_READING,
	STATE_WRITING,
	STATE_DATA_READY,
	STATE_ERROR
	} serial_state_e;

struct _readBuffer {
	volatile serial_state_e			state;
	volatile uint16_t				length;
	volatile uint16_t				idx;
	uint8_t							data[SERIAL_BUFFER_SIZE];
};

struct _writeBuffer {
	volatile serial_state_e			state;
	volatile uint8_t				L_data_ctrl;
	uint8_t							L_data_chksum;
	uint16_t						length;
	uint8_t							*data;
	timeoutTmr						timeout;
};

typedef struct _serialObj {
	uint8_t							enabled;
	void							*parent;
	USART_t							*uart;
	TC0_t							*idleTmr;
	void							(*dataReadyCb)(void *parent, uint8_t *data, uint16_t length);
	void							(*destAddrCb)(void *parent, address_type_e daf, uint16_t destAddr);
	struct _readBuffer				readingB;
	struct _writeBuffer				wbuffer;
} serialObj;

typedef struct _serialObj *serialHandle;

serialHandle serialInit(void *pMemory, uint16_t numBytes);

int8_t serialSetup(serialHandle handle, void *uart, uint32_t br, uint8_t bits, parity_e pr, void *tmr, uint16_t timeout_usec, void *parent, void *drcb, void* dacb);

int8_t serialEnable(serialHandle handle);

int8_t serialDisable(serialHandle handle);

int16_t serialWrite(serialHandle handle, uint8_t *data, uint16_t length);

int16_t serialWriteLdata(serialHandle handle, uint8_t *data, uint16_t length);

int16_t serialWriteNonBlocking(serialHandle handle, uint8_t *data, uint16_t length);

int8_t serialWriteSingleByte(serialHandle handle, uint8_t data);

int16_t serialRead(serialHandle handle, uint8_t *data, uint16_t length);

int16_t serialBytesRead(serialHandle handle);

void serialRxIsr(serialHandle handle);

void serialTxIsr(serialHandle handle);

void serialTimeOut(serialHandle handle);

#endif /* SERIAL_H_ */
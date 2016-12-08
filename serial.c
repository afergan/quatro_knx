/*
 * serial.c
 *
 * Created: 18-5-2016 7:19:01
 *  Author: Paul
 */ 


#include <string.h>
#include "serial.h"


serialHandle serialInit(void *pMemory, uint16_t numBytes)
{
	serialHandle serial_handle;
	if (numBytes < sizeof(serialObj))
		return ((serialHandle)NULL);
	serial_handle = (serialHandle)pMemory;
	return(serial_handle);
}

int8_t serialSetup(serialHandle handle, void *uart, uint32_t br, uint8_t bits, parity_e pr, void *tmr, uint16_t timeout_usec, void *parent, void *drcb, void *dacb)
{
	serialObj *serial;
	serial = (serialObj *) handle;
	
	uint8_t conf = 0;
	
	serial->uart = (USART_t*) uart;
		
	serial->uart->BAUDCTRLA = (F_CPU / (16 * br)) - 1;
	serial->uart->BAUDCTRLB = 0;

	if (bits == 9)
		conf = _BV(USART_CHSIZE0_bp) | _BV(USART_CHSIZE1_bp) | _BV(USART_CHSIZE2_bp);
	else
		conf = _BV(USART_CHSIZE0_bp) | _BV(USART_CHSIZE1_bp);
	
	if (pr == PARITY_EVEN) // even
		conf |= _BV(USART_PMODE1_bp);
	else if (pr == PARITY_ODD) // odd
		conf |= _BV(USART_PMODE0_bp) | _BV(USART_PMODE1_bp);
		
	serial->uart->CTRLC = conf;
	
	if (uart == &USARTC0) {
		PORTC.OUT |= 0b00001000;
		PORTC.DIR &= ~(0b00000100);
		PORTC.DIR |= 0b00001000;
	}
	
	if (uart == &USARTD0) {
		PORTD.OUT |= 0b00001000;
		PORTD.DIR &= ~(0b00000100);
		PORTD.DIR |= 0b00001000;
	}

#ifdef USARTE0
	if (uart == &USARTE0) {
		PORTE.OUT |= 0b00001000;
		PORTE.DIR &= ~(0b00000100);
		PORTE.DIR |= 0b00001000;
	}
#endif
		
	if (tmr != NULL){
		serial->idleTmr = (TC0_t*) tmr;
		serial->idleTmr->CTRLA = TC_CLKSEL_OFF_gc;
		serial->idleTmr->CTRLFSET = TC1_CMD_gm;
		serial->idleTmr->CTRLB = 0;
		serial->idleTmr->CTRLC = 0;
		serial->idleTmr->CTRLD = 0;
		serial->idleTmr->CTRLE = 0;
		serial->idleTmr->PERBUF = ((F_CPU * 1LL * timeout_usec) / (64 * 1000000LL));
		serial->idleTmr->CTRLFSET = TC0_CMD0_bm; // update
		
	}
	
	serial->readingB.state = STATE_WAITING;
	serial->readingB.length = 0;
	serial->readingB.idx = 0;
	
	serial->wbuffer.data = NULL;
	serial->wbuffer.length = 0;
	serial->wbuffer.L_data_ctrl = 0;
	
	serial->parent = parent;
	
	serial->dataReadyCb = drcb;
	serial->destAddrCb = dacb;
	
	return 0;
}



int8_t serialEnable(serialHandle handle){
	serialObj *serial;
	serial = (serialObj *) handle;

	serial->readingB.state = STATE_WAITING;
	serial->readingB.length = 0;
	serial->readingB.idx = 0;
	
	serial->wbuffer.data = NULL;
	serial->wbuffer.length = 0;
	serial->wbuffer.L_data_ctrl = 0;
	
	serial->enabled = 1;
	serial->uart->CTRLB = _BV(USART_RXEN_bp) | _BV(USART_TXEN_bp);
	serial->uart->CTRLA = _BV(USART_RXCINTLVL0_bp) | _BV(USART_RXCINTLVL1_bp);
	
	return 0;
}

int8_t serialDisable(serialHandle handle){
	serialObj *serial;
	serial = (serialObj *) handle;
		
	serial->uart->CTRLA &= 0;
	serial->uart->CTRLB &= ~(_BV(USART_RXEN_bp) | _BV(USART_TXEN_bp));
	serial->idleTmr->CTRLA = TC_CLKSEL_OFF_gc;
	serial->enabled = 0;
	return 0;
}

int16_t serialWrite(serialHandle handle, uint8_t *data, uint16_t length)
{
	volatile serialObj *serial;
	serial = (serialObj *) handle;
	if ((serial->enabled != 1) || (length == 0))// || (serial->wbuffer.state == STATE_WRITING))
		return -1;
	if (serial->wbuffer.state == STATE_WRITING)
	{
		while ((serial->wbuffer.state == STATE_WRITING) && isNotTimedout(&serial->wbuffer.timeout));  
		if (isTimedout(&serial->wbuffer.timeout))
		{
			serial->wbuffer.state = STATE_WAITING; // reset after time out
			return -1; // return without writing. Wait to finish
		}
	}
	serial->wbuffer.data = data;
	serial->wbuffer.length = length;
	serial->wbuffer.L_data_ctrl = 0;
	serial->wbuffer.state = STATE_WRITING;
	setupTimeoutTmr(&serial->wbuffer.timeout, SERIAL_TIMEOUT_MS);
	serial->uart->CTRLA |= USART_DREINTLVL1_bm; // (USART_DREINTLVL0_bm | USART_DREINTLVL1_bm);
	while (serial->wbuffer.state == STATE_WRITING && isNotTimedout(&serial->wbuffer.timeout));
	if (isTimedout(&serial->wbuffer.timeout))
		return -1;
	else
		return length;
}

// non blocking L_data frame write function

int16_t serialWriteLdata(serialHandle handle, uint8_t *data, uint16_t length)
{
	volatile serialObj *serial;
	serial = (serialObj *) handle;
	if ((serial->enabled != 1) || (length == 0) || (length > 63))
		return -1;
	if (serial->wbuffer.state == STATE_WRITING)
	{
		while ((serial->wbuffer.state == STATE_WRITING) && isNotTimedout(&serial->wbuffer.timeout));
		if (isTimedout(&serial->wbuffer.timeout))
		{
			serial->wbuffer.state = STATE_WAITING; // reset after time out
			return -1; // return without writing. Wait to finish
		}
	}
	serial->wbuffer.data = data;
	serial->wbuffer.length = length;
	serial->wbuffer.L_data_ctrl = U_L_DATA_START;
	serial->wbuffer.L_data_chksum = 0;
	serial->wbuffer.state = STATE_WRITING;
	setupTimeoutTmr(&serial->wbuffer.timeout, SERIAL_TIMEOUT_MS);
	serial->uart->CTRLA |= USART_DREINTLVL1_bm; // (USART_DREINTLVL0_bm | USART_DREINTLVL1_bm);
	return length;
}

// non blocking write. use a static array/pointer for data.

int16_t serialWriteNonBlocking(serialHandle handle, uint8_t *data, uint16_t length)
{
	volatile serialObj *serial;
	serial = (serialObj *) handle;
	if ((serial->enabled != 1) || (length == 0)) // || (serial->wbuffer.state == STATE_WRITING))
		return -1;
	if (serial->wbuffer.state == STATE_WRITING)
	{
		while ((serial->wbuffer.state == STATE_WRITING) && isNotTimedout(&serial->wbuffer.timeout));
		if (isTimedout(&serial->wbuffer.timeout))
		{
			serial->wbuffer.state = STATE_WAITING; // reset after time out
			return -1; // return without writing. Wait to finish
		}
	}
	serial->wbuffer.data = data;
	serial->wbuffer.length = length;
	serial->wbuffer.L_data_ctrl = 0;
	serial->wbuffer.state = STATE_WRITING;
	//serial->wbuffer.timeout = clock.timestamp + SERIAL_TIMEOUT_MS;
	setupTimeoutTmr(&serial->wbuffer.timeout, SERIAL_TIMEOUT_MS);
	serial->uart->CTRLA |= USART_DREINTLVL1_bm; // (USART_DREINTLVL0_bm | USART_DREINTLVL1_bm);
	return 0;
}

int8_t serialWriteSingleByte(serialHandle handle, uint8_t data) {
	volatile serialObj *serial;
	serial = (serialObj *) handle;
	if ((serial->enabled != 1))
		return -1;
	if (serial->wbuffer.state == STATE_WRITING)
	{
		while ((serial->wbuffer.state == STATE_WRITING) && isNotTimedout(&serial->wbuffer.timeout));
		if (isTimedout(&serial->wbuffer.timeout))
		{
			serial->wbuffer.state = STATE_WAITING; // reset after time out
			return -1; // return without writing. Wait to finish
		}
	}
	if (serial->uart->STATUS & USART_DREIF_bm)
	{
		serial->uart->DATA = data;
		return 1;
	}
	return 0;

}

int16_t serialRead(serialHandle handle, uint8_t *data, uint16_t length)
{
	volatile serialObj *serial;
	serial = (serialObj *) handle;
	if (serial->enabled != 1)
		return -1;
	if (length == 0 || length > SERIAL_BUFFER_SIZE)
		return -1;
	timeoutTmr timeout;
	setupTimeoutTmr(&timeout, SERIAL_TIMEOUT_MS);

	while (serial->readingB.idx < length && isNotTimedout(&timeout));
	if (isNotTimedout(&timeout)) {
		memcpy(data, (const void*)(serial->readingB.data), length);
		serial->readingB.idx = 0;
		serial->readingB.length = 0;
		serial->readingB.state = STATE_WAITING;
	} else {
		length = -1;
	}

	return length;
}

int16_t serialBytesRead(serialHandle handle)
{
	serialObj *serial;
	serial = (serialObj *) handle;
	if (serial->enabled != 1)
		return -1;

	return serial->readingB.idx;
}


void serialTxIsr(serialHandle handle){
	serialObj *serial;
	serial = (serialObj *) handle;
	if (serial->wbuffer.state == STATE_WRITING)
	{
		if (serial->wbuffer.L_data_ctrl != 0) // L_data send
		{
			if ((serial->wbuffer.L_data_ctrl & 0xC0) == U_L_DATA_START || (serial->wbuffer.L_data_ctrl & 0xC0) == U_L_DATA_END)
			{
				serial->uart->DATA = serial->wbuffer.L_data_ctrl; // send L_data control byte
				serial->wbuffer.L_data_ctrl++;
				serial->wbuffer.L_data_ctrl &= 0x3F;
			}
			else if ((serial->wbuffer.L_data_ctrl & 0xC0) == (U_L_DATA_START | U_L_DATA_END)) // frame send, including checksum.
			{
				serial->uart->CTRLA &= ~(USART_DREINTLVL0_bm | USART_DREINTLVL1_bm); // disable transmit interrupt
				serial->wbuffer.L_data_ctrl = 0;
				serial->wbuffer.state = STATE_WAITING;
			}
			else
			{
				if (serial->wbuffer.length != 0) // send packet byte
				{
					serial->wbuffer.length--;
					serial->uart->DATA = *(serial->wbuffer.data);
					serial->wbuffer.L_data_chksum ^= *(serial->wbuffer.data++);
					serial->wbuffer.L_data_ctrl |= (serial->wbuffer.length != 0) ? U_L_DATA_START : U_L_DATA_END;
				}
				else // finish with checksum
				{
					serial->uart->DATA = ~(serial->wbuffer.L_data_chksum);
					serial->wbuffer.L_data_ctrl = (U_L_DATA_START | U_L_DATA_END);
				}
			}
		}
		else // normal send
		{
			if (serial->wbuffer.length != 0) {
				serial->wbuffer.length--;
				serial->uart->DATA = *(serial->wbuffer.data++);
			} else {
				serial->uart->CTRLA &= ~(USART_DREINTLVL0_bm | USART_DREINTLVL1_bm); // disable transmit interrupt
				serial->wbuffer.state = STATE_WAITING;
			}
		}
	}
	else
		serial->uart->CTRLA &= ~(USART_DREINTLVL0_bm | USART_DREINTLVL1_bm); // disable transmit interrupt
}

// E981.03 receive interrupt ISR.
void serialRxIsr(serialHandle handle){
	serialObj *serial;
	serial = (serialObj *) handle;
	//uint8_t error = serial->uart->STATUS & (USART_PERR_bm | USART_BUFOVF_bm | USART_FERR_bm);
	uint8_t data_b8 = serial->uart->STATUS & (USART_RXB8_bm);
	uint8_t data = serial->uart->DATA;
	if (serial->enabled != 1)
		return;
	
	struct _readBuffer *readingBuffer = &(serial->readingB);
	
	if (readingBuffer != NULL) {
		// E981.03 extra bit signaling beginning of a new message. 	
		if (data_b8 && ((serial->uart->CTRLC & (USART_CHSIZE0_bm | USART_CHSIZE1_bm | USART_CHSIZE2_bm)) == USART_CHSIZE_gm))  // new service message
		{
			readingBuffer->state = STATE_READING;
			readingBuffer->idx = 0;

			uint8_t timeOutMask = ((data & 0xF0) == 0x90 || (data & 0xF0) == 0xB0 || (data & 0xF0) == 0x10 || (data & 0xF0) == 0x30 || data == 0xF0);
			uint8_t twoByeMask = (data ==  0xFE || data == 0xF1);
			// rest is single byte
		
			if (timeOutMask) {
				if (serial->idleTmr != NULL) {
				serial->idleTmr->CTRLFSET = (TC1_CMD1_bm); // reload timer 2.5 ms
				serial->idleTmr->CTRLA = TC_CLKSEL_DIV64_gc; // start time-out timer
				serial->idleTmr->INTCTRLA = TC_OVFINTLVL_HI_gc;
				}
				readingBuffer->length = 0xffff;
			} else if (twoByeMask) {
				readingBuffer->length = 2;
			} else {
				readingBuffer->length = 1;
			}

		} else if (readingBuffer->state == STATE_WAITING){
			readingBuffer->state = STATE_READING;
			readingBuffer->idx = 0;
			readingBuffer->length = 0;
		}
	
		// check right quantity of bytes (or buffer full). signal data ready.
		if (readingBuffer->state == STATE_READING) {
			if (readingBuffer->idx < SERIAL_BUFFER_SIZE)
				readingBuffer->data[readingBuffer->idx++] = data; //write data to buffer and increment pointer
			else if (readingBuffer->length != 0xffff)  {
					if (serial->dataReadyCb != NULL)
						(*(serial->dataReadyCb))(serial->parent, readingBuffer->data, readingBuffer->idx);
					readingBuffer->state = STATE_DATA_READY; // handle buffer overflow
			}
			
			if (readingBuffer->idx == readingBuffer->length) // finish reading fixed length message
			{
				if (serial->dataReadyCb != NULL)
					(*(serial->dataReadyCb))(serial->parent, readingBuffer->data, readingBuffer->idx);
				readingBuffer->state = STATE_DATA_READY; // finished reading set amount of bytes
			}

		}
		
		// check if message contains addr
		if (readingBuffer->length == 0xffff)
		{
			// standard frame length
			if ((readingBuffer->data[0] & 0x80) != 0 && readingBuffer->idx == 6) {
				address_type_e daf = (readingBuffer->data[5] & 0x80) ? KNX_GROUP_ADDRESS : KNX_PHYSICAL_ADDRESS;
				uint16_t destAddr = 0;
				destAddr = (uint16_t)(readingBuffer->data[3] << 8) | readingBuffer->data[4];
				if (serial->destAddrCb != NULL)
					(*(serial->destAddrCb))(serial->parent, daf, destAddr);
			}
			// extended frame length
			else if ((readingBuffer->data[0] & 0x80) == 0 && readingBuffer->idx == 7) {
				address_type_e daf = (readingBuffer->data[6] & 0x80) ? KNX_GROUP_ADDRESS : KNX_PHYSICAL_ADDRESS;
				uint16_t destAddr = 0;
				destAddr = (uint16_t)(readingBuffer->data[4] << 8) | readingBuffer->data[5];
				if (serial->destAddrCb != NULL)
					(*(serial->destAddrCb))(serial->parent, daf, destAddr);
			}
		}
	
		//reset timer after each byte.
		if (serial->idleTmr != NULL && ((serial->idleTmr->CTRLA & TC0_CLKSEL_gm) != TC_CLKSEL_OFF_gc)) {
			serial->idleTmr->CTRLFSET = (TC1_CMD1_bm); // reload timer 2.5 ms
		}	
	}
}

void serialTimeOut(serialHandle handle){
	serialObj *serial;
	serial = (serialObj *) handle;
	if (serial->idleTmr != NULL) {
		serial->idleTmr->INTCTRLA &= ~(TC_OVFINTLVL_HI_gc);
		serial->idleTmr->CTRLA = TC_CLKSEL_OFF_gc;
		serial->idleTmr->CTRLFSET = (TC1_CMD0_bm); // reload timer with initial value
		
	}
	if (serial->dataReadyCb != NULL)
		(*(serial->dataReadyCb))(serial->parent, serial->readingB.data, serial->readingB.idx); // time out signals message ready for unknown number of bytes
	serial->readingB.state = STATE_WAITING;
}



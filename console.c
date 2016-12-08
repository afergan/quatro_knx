/*
 * console.c
 *
 * Created: 5-7-2016 19:05:47
 *  Author: Paul
 */ 

#include "clocks.h"
#include "interrupt.h"
//#include "serial.h"
#include "console.h"
#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>

const char rstSpikeStr[] PROGMEM = "Spike Detection Reset\r\n";
const char rstSwStr[] PROGMEM = "Software reset\r\n";
const char rstDebugStr[] PROGMEM = "Debug reset\r\n";
const char rstWatchDogStr[] PROGMEM = "Watchdog reset\r\n";
const char rstBrownoutStr[] PROGMEM = "Brownout reset\r\n";
const char rstExternalStr[] PROGMEM = "External reset\r\n";
const char rstPoweronStr[] PROGMEM = "Power-on reset\r\n";

PGM_P const rstStringTable[] PROGMEM = {
	rstSpikeStr,
	rstSwStr,
	rstDebugStr,
	rstWatchDogStr,
	rstBrownoutStr,
	rstExternalStr,
	rstPoweronStr
};

const char initString1[] PROGMEM = "\n\r****************************************\n\r\n";
const char initString2[] PROGMEM = "Quatro touch sensor\r\n";
const char initString3[] PROGMEM = "19-11-2016\r\n";
const char initString4[] PROGMEM = "by Welpie\n\r\n";
const char initString5[] PROGMEM = "initialization...\n\r\n";
const char initString6[] PROGMEM = "\n1) rgb led initialize : ";
const char initString7[] PROGMEM = "2) touch pad initialize : ";
const char initString8[] PROGMEM = "3) room temperature / humidity initialize : ";
const char initString9[] PROGMEM = "4) own Address : ";
const char initString10[] PROGMEM = "\n\rinitialisation finished\n\r\n\r";

PGM_P const initStringTable[] PROGMEM = {
	initString1,
	initString2,
	initString3,
	initString4,
	initString5,
	initString6,
	initString7,
	initString8,
	initString9,
	initString10
};

consoleHandle consoleSerialHandle;

consoleHandle consoleInit(void *pMemory, uint16_t numBytes) {
	consoleHandle console_handle;
	if (numBytes < sizeof(consoleObj))
		return ((consoleHandle)NULL);
	console_handle = (consoleHandle)pMemory;
	return(console_handle);
}

int8_t consoleSetup(consoleHandle handle, void *uart, uint32_t br, uint8_t bits, parity_e pr) {
	consoleObj *console;
	console = (consoleObj *) handle;	
	
	consoleSerialHandle = handle;
	
	uint8_t conf = 0;
		
	console->uart = (USART_t*) uart;
	
	console->uart->BAUDCTRLA = (F_CPU / (16 * br)) - 1;
	console->uart->BAUDCTRLB = 0;

	if (bits == 9)
	conf = _BV(USART_CHSIZE0_bp) | _BV(USART_CHSIZE1_bp) | _BV(USART_CHSIZE2_bp);
	else
	conf = _BV(USART_CHSIZE0_bp) | _BV(USART_CHSIZE1_bp);
	
	if (pr == PARITY_EVEN) // even
	conf |= _BV(USART_PMODE1_bp);
	else if (pr == PARITY_ODD) // odd
	conf |= _BV(USART_PMODE0_bp) | _BV(USART_PMODE1_bp);
	
	console->uart->CTRLC = conf;
	
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
	
	console->enable = 1;
	
	console->uart->CTRLB = _BV(USART_RXEN_bp) | _BV(USART_TXEN_bp);
	console->uart->CTRLA = _BV(USART_RXCINTLVL0_bp) | _BV(USART_RXCINTLVL1_bp);
	return 0;
}

int8_t consoleReadByte(consoleHandle handle, uint8_t *data)
{
	consoleObj *console;
	console = (consoleObj *) handle;
	if (console->enable != 1)
		return -1;
	if (console->rBuf.start == console->rBuf.end)
		return 0;
	*data = console->rBuf.data[console->rBuf.start];
	console->rBuf.start = ringbufferIncr(console->rBuf.start);
	return 1;
}

int8_t consoleWriteByte(consoleHandle handle, uint8_t data)
{
	volatile consoleObj *console;
	console = (consoleObj *) handle;
	if (console->enable != 1)
		return -1;
	if (console->wBuf.state == STATE_WRITING)
	{
		while (console->wBuf.state == STATE_WRITING && isNotTimedout(&(console->wBuf.timeout)));
		if (isTimedout(&(console->wBuf.timeout)))
			return -1;
	}
	setupTimeoutTmr(&console->wBuf.timeout, CONSOLE_SERIAL_TIMEOUT);
	if (console->uart->STATUS & _BV(USART_DREIF_bp)) // transmit buffer empty?
	{
		console->uart->DATA = data;
			return 1;
	}
	return 0;
}

int16_t consoleWrite(consoleHandle handle, uint8_t *data, uint16_t length)
{
	volatile consoleObj *console;
	console = (consoleObj *) handle;
	if ((console->enable != 1) || (length == 0))
		return -1;
	if (console->wBuf.state == STATE_WRITING)
	{
		while (console->wBuf.state == STATE_WRITING && isNotTimedout(&(console->wBuf.timeout)));
		if (isTimedout(&console->wBuf.timeout))
			return -1;
	}
	console->wBuf.data = data;
	console->wBuf.prgmem = 0;
	console->wBuf.length = length;
	console->wBuf.state = STATE_WRITING;
	setupTimeoutTmr(&(console->wBuf.timeout), CONSOLE_SERIAL_TIMEOUT);
	console->uart->CTRLA |= USART_DREINTLVL1_bm; // enable interrupt to start writing
	return length;
}

int8_t consoleWrt_P(consoleHandle handle, const char *str) {
	volatile consoleObj *console;
	console = (consoleObj *) handle;
	if ((console->enable != 1))
		return -1;
	if (console->wBuf.state == STATE_WRITING)
	{
		while (console->wBuf.state == STATE_WRITING && isNotTimedout(&(console->wBuf.timeout)));
		if (isTimedout(&(console->wBuf.timeout)))
			return -1;
	}
	console->wBuf.data = (uint8_t*)str;
	console->wBuf.prgmem = 1;
	console->wBuf.length = strnlen_P((PGM_P)str, 255);
	console->wBuf.state = STATE_WRITING;
	setupTimeoutTmr(&console->wBuf.timeout, CONSOLE_SERIAL_TIMEOUT);
	console->uart->CTRLA |= USART_DREINTLVL1_bm; // enable interrupt to start writing
	return 0;	
}

uint8_t	consoleNewline(consoleHandle handle) {
	return consoleWrt_P(handle, PSTR("\n\r"));
}

uint8_t consoleClearScreen(consoleHandle handle) {
	consoleObj *console;
	console = (consoleObj *) handle;
	if (!console->enable)
		return -1;
	return writeAnsiCtrl(handle, ANSI_CLEAR_SCREEN ANSI_HOME_CURSOR);
}

int16_t consoleWriteHex(consoleHandle handle, uint8_t *hex, uint8_t length) {
	consoleObj *console;
	console = (consoleObj *) handle;
	if (!console->enable)
	return -1;
	char str[0xff];
	uint8_t j =0;
	for (uint8_t i=0; i < length; i++) {
		j += snprintf(str + j, 4, "%02x-", hex[i]);
		if (j > 245)
		{
			str[j++] = '.';
			str[j++] = '.';
			str[j++] = '.';
			str[j++] = '.';
			break;
		}
	}
	if (j > 0) j--;
	str[j++] = 0x0a;
	str[j++] = 0x0d;
	str[j++] = 0x00;
	return consoleWrite(handle, (uint8_t*)str, j);
}

int8_t consoleWrtInitStr(consoleHandle handle, initStr_e initStr) {
	consoleObj *console;
	console = (consoleObj *) handle;
	
	if (console->enable != 1)
		return -1;
	return consoleWrt_P(handle, (PGM_P)pgm_read_word(&(initStringTable[(uint8_t)initStr])));
}

int8_t consoleWrtResetCause(consoleHandle handle, rstCause_e cause) {
	consoleObj *console;
	console = (consoleObj *) handle;
	if (console->enable != 1)
		return -1;
	return consoleWrt_P(handle, (PGM_P)pgm_read_word(&(rstStringTable[(uint8_t)cause])));
}



int8_t consoleEchoTouch(consoleHandle handle, touch_state_e pad, touch_event_e tevent) {
	consoleObj *console;
	console = (consoleObj *) handle;
	if (console->enable != 1)
		return -1;	
	switch(pad) {
		case TOUCH_TOP:
			consoleWrtLiteral(console,"top ");
		break;
		case TOUCH_RIGHT:
			consoleWrtLiteral(console,"right ");
		break;
		case TOUCH_BOTTOM:
			consoleWrtLiteral(console,"bottom ");
		break;
		case TOUCH_LEFT:
			consoleWrtLiteral(console,"left ");
		break;
		case TOUCH_TOP_BOTTOM:
			consoleWrtLiteral(console,"top-bottom ");				
		break;
		case TOUCH_LEFT_RIGHT:
			consoleWrtLiteral(console,"left-right ");
		break;
		default:
			consoleWrtLiteral(console,"touch pad ");
		break;
	}
	switch (tevent) {
		case TOUCH_EVENT_DOWN:
			consoleWrtLiteral(console,"down\n\r");
		break;
		case TOUCH_EVENT_UP:
			consoleWrtLiteral(console,"up\n\r");
		break;
		case TOUCH_SHORT_PRESS_EVENT:
			consoleWrtLiteral(console,"short press\n\r");
		break;
		case TOUCH_DOUBLE_PRESS_EVENT:
			consoleWrtLiteral(console,"double press\n\r");
		break;
		case TOUCH_LONG_PRESS_DOWN_EVENT:
			consoleWrtLiteral(console,"long press down\n\r");
		break;
		case TOUCH_LONG_PRESS_UP_EVENT:
			consoleWrtLiteral(console,"long press up\n\r");
		break;
		case TOUCH_ERROR_EVENT:
			consoleWrtLiteral(console,"error event\n\r");
		break;
		case TOUCH_RESET_EVENT:
			consoleWrtLiteral(console,"reset event\n\r");
		break;
		case TOUCH_COMING_ALIVE_EVENT:
			consoleWrtLiteral(console,"coming alive\n\r");
		break;
		default:
			consoleWrtLiteral(console,"unspecified event..\n\r");
		break;
	}
	 return 0;
}

int8_t hex2Str(uint8_t *hex, char *str, uint8_t nibbles) {
	int j=0;
	for (int i=0; i < (nibbles >> 1); i++) {
		char highNibble = ((hex[i] & 0xF0) >> 4);
		char lowNibble = (hex[i] & 0x0F);
		str[j++] = (highNibble > 9) ? highNibble + 0x37 : highNibble + 0x30;
		str[j++] = (lowNibble > 9) ? lowNibble + 0x37 : lowNibble + 0x30;
	}
	return nibbles;
}

void consoleTxIsr(consoleHandle handle){
	consoleObj *console;
	console = (consoleObj *) handle;
	if (console->wBuf.state == STATE_WRITING && console->wBuf.length != 0) {
		console->wBuf.length--;
		if (console->wBuf.prgmem != 1)
			console->uart->DATA = *(console->wBuf.data++);
		else
			console->uart->DATA = pgm_read_byte(console->wBuf.data++);
		}
	else {
		console->uart->CTRLA &= ~(USART_DREINTLVL0_bm | USART_DREINTLVL1_bm); // disable transmit interrupt
		console->wBuf.state = STATE_WAITING;
		console->wBuf.prgmem = 0;
	}
}

void consoleRxIsr(consoleHandle handle){
	consoleObj *console;
	console = (consoleObj *) handle;
	uint8_t error = console->uart->STATUS & (USART_PERR_bm | USART_BUFOVF_bm | USART_FERR_bm);
	uint8_t data = console->uart->DATA;
	if (console->enable != 1 || error)
		return;
	console->rBuf.data[console->rBuf.end] = data;
	console->rBuf.end = ringbufferIncr(console->rBuf.end);
	if (console->rBuf.end == console->rBuf.start)
		console->rBuf.start = ringbufferIncr(console->rBuf.start);
}

uint8_t ringbufferIncr(uint8_t a) {
	a++;
	if (a == CONSOLE_RINGBUFFER_LENGTH)
	a = 0;
	return a;
}


ISR(USARTD0_RXC_vect) {
	consoleRxIsr(consoleSerialHandle);
}

ISR(USARTD0_DRE_vect){
	consoleTxIsr(consoleSerialHandle);
}

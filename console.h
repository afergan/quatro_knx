/*
 * console.h
 *
 * Created: 5-7-2016 19:06:01
 *  Author: Paul
 */ 


#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "serial.h"
#include "touchPad.h"
#include <avr/pgmspace.h>

#define	CONSOLE_RINGBUFFER_LENGTH	32

#define CONSOLE_SERIAL_TIMEOUT		500

#define CMD_LINE_LENGTH				32

#define ANSI_ERASE_TO_EOL			"\e[K"
#define ANSI_ERASE_LINE				"\e[2K"
#define ANSI_SAVE_CURSOR			"\e7"
#define ANSI_RESTORE_CURSOR			"\e8"
#define ANSI_MOVE_CURSOR_RIGHT		"\e[C"
#define ANSI_MOVE_CURSOR_LEFT		"\e[D"
#define ANSI_CLEAR_SCREEN			"\e[2J"
#define ANSI_HOME_CURSOR			"\e[f"
#define ANSI_RESET					"\e[c"

#define ESC							0x1b
#define CTRL_C						0x03
#define CR							"\r"
#define SPACE						0x20
#define NULLCHAR					0x00
#define DOT							0x2e
#define FORWARD_SLASH				0x2f

typedef enum {
	ANSI_BLACK = 0,
	ANSI_BLUE = 1,
	ANSI_GREEN = 2,
	ANSI_CYAN = 3,
	ANSI_RED = 4,
	ANSI_MAGENTA = 5,
	ANSI_BROWN = 6,
	ANSI_WHITE = 7,
	ANSI_GREY = 8,
	ANSI_LIGHT_BLUE = 9,
	ANSI_LIGHT_GREEN = 10,
	ANSI_LIGHT_CYAN = 11,
	ANSI_LIGHT_RED = 12,
	ANSI_LIGHT_MAGENTA = 13,
	ANSI_YELLOW = 14,
	ANSI_LIGHT_WHITE = 15,
	ANSI_DEFAULT_FOREGROUND_COLOR = 39
} ansi_color_e;

typedef enum {
	rstSpike,
	rstSw,
	rstDebug,
	rstWatchDog,
	rstBrownout,
	rstExternal,
	rstPoweron
} rstCause_e;

typedef enum {
	initLine1,
	initLine2,
	initLine3,
	initLine4,
	initLine5,
	initRgbStr,
	initTouchStr,
	initTempStr,
	initOwnAddrStr,
	initFinishStr
} initStr_e;

#define writeAnsiCtrl(a,b)			consoleWrite(a,(uint8_t*)b,sizeof(b))

#define consoleWrtLiteral(a,b)		consoleWrt_P(a, PSTR(b))

struct ringbuffer_s {
	volatile uint8_t				start;
	volatile uint8_t				end;
	uint8_t							data[CONSOLE_RINGBUFFER_LENGTH];
};

struct conWrtBuffer {
	volatile serial_state_e			state;
	uint8_t							prgmem;
	uint16_t						length;
	uint8_t							*data;
	timeoutTmr						timeout;
};

typedef struct _consoleObj {
	int8_t							enable;
	USART_t							*uart;
	volatile struct conWrtBuffer				wBuf;
	struct ringbuffer_s				rBuf;
} consoleObj;

typedef struct _consoleObj *consoleHandle;

consoleHandle consoleInit(void *pMemory, uint16_t numBytes);

int8_t consoleSetup(consoleHandle handle, void *uart, uint32_t br, uint8_t bits, parity_e pr);

int8_t consoleReadByte(consoleHandle handle, uint8_t *data);

int8_t consoleWriteByte(consoleHandle handle, uint8_t data);

int16_t consoleWrite(consoleHandle handle, uint8_t *data, uint16_t length);

uint8_t	consoleNewline(consoleHandle handle);

uint8_t consoleClearScreen(consoleHandle handle);

int16_t consoleWriteHex(consoleHandle handle, uint8_t *hex, uint8_t length);

int8_t consoleWrtInitStr(consoleHandle handle, initStr_e initStr);

int8_t consoleWrtResetCause(consoleHandle handle, rstCause_e cause);

int8_t consoleWrt_P(consoleHandle handle, const char *str);

int8_t consoleEchoTouch(consoleHandle handle, touch_state_e pad, touch_event_e tevent);

void consoleRxIsr(consoleHandle handle);

void consoleTxIsr(consoleHandle handle);

uint8_t ringbufferIncr(uint8_t a);

#endif /* CONSOLE_H_ */
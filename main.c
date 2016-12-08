/*
 * quatro firmware
 * 
 *
 * Created: 17-5-2016 18:15:15
 * Author : Paul
 */ 

#include "clocks.h"

#include "interrupt.h"
#include <stdlib.h>
#include "device.h"
#include "serial.h"

#include "applicationInterface.h"

#include "rgb_led.h"
#include "touchPad.h"
#include "eventFifo.h"
#include "action.h"
#include "console.h"
#include "cmdLineInterpreter.h"

#include "i2c.h"
#include "roomTempAndHumidity.h"
#include "colorFeedback.h"
#include "eepromStorage.h"
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>

#define KEYBOARD_FIFO_LENGTH		8


A_InterfaceObj AI_obj;
A_InterfaceHandle AI_Hndl;

deviceObj device_obj;
deviceHandle device_handle;

consoleObj console_obj;
consoleHandle console_handle;

clinterpreterObj cli_obj;
clinterpreterHandle cli_handle;

touchActionObj touchAction_obj;
touchActionHandle touchAction_handle;

rgbLedObj rgbLed_obj;
rgbLedHandle rgbLed_handle;

pad_port_config padPortCfg[TOUCHPAD_TOTAL];

touchPadObj keyboard_obj;
touchPadHandle keyboard_handle;

eventFifoObj keyboard_fifo;
struct _eventFifoBuffer keyboard_fifo_buffer[KEYBOARD_FIFO_LENGTH];
eventFifoHandle keyboard_fifo_handle;

i2cObj i2c_obj;
i2cHandle i2c_handle;

tempHum_Obj tempHum_obj;
tempHum_Handle tempHum_handle;

colorFeedback_Obj colorFbk_obj;
colorFeedback_Handle colorFbk_Hndl;

eepromStorage_Obj eeprom_obj;
eepromStorage_Handle eepromHndl;

void keyboard_event_callback(touch_event_e tevent, touch_state_e pad);

void everySecond();

void swReset();

uint8_t escapes = 0;

uint8_t everySecondTrigger = 0;

void echoTemperatures(char *str, uint8_t length);

#define USR_CMD_TBL \
	ENTRY("ownaddr,o", OWNADDR, "echo own individual address") \
	ENTRY("timestamp,t", TIMESTAMP, "echo timestamp") \
	ENTRY("ambient,temp,hum,h", AMBIENT, "echo temperature and humdity")


#define ENTRY(a,b,c)	void Cmd_ ## b(void *parentHandle);
	USR_CMD_TBL
#undef ENTRY

#define ENTRY(a,b,c)    const char CLI_CMD_STR_ ## b [] PROGMEM = a;
	USR_CMD_TBL
#undef ENTRY

#define ENTRY(a,b,c)    const char CLI_HLP_STR_ ## b [] PROGMEM = c;
	USR_CMD_TBL
#undef ENTRY

command usrCommands[] = {
	#define ENTRY(a,b,c) {.cmd=(CLI_CMD_STR_ ## b), .func = Cmd_ ## b, .helpTxt=(CLI_HLP_STR_ ## b)},
		USR_CMD_TBL
	#undef ENTRY
};

uint8_t USR_COMMANDS_N = sizeof (usrCommands)/ sizeof (command);

int main(void)
{
	setupClks(everySecond, 1024);

	
//	PORTE.OUT = 0x00;
	PORTE.DIR = 0x0c;

	AI_Hndl = A_InterfaceInit(&AI_obj, sizeof(AI_obj));
	A_InterfaceSetup(AI_Hndl, NULL);
	
	eepromHndl = eepromStorage_Init(&eeprom_obj, sizeof(eeprom_obj));
	eepromStorage_Setup(eepromHndl, AI_Hndl);

	device_handle = deviceInit(&device_obj, sizeof(device_obj));
	deviceSetup(device_handle, AI_Hndl);

	console_handle = consoleInit(&console_obj, sizeof(console_obj));
	consoleSetup(console_handle, &USARTD0, 115200, 8, PARITY_NONE);

	cli_handle = cmdLineInterpreterInit(&cli_obj, sizeof(cli_obj));
	cmdLineInterpreterSetup(cli_handle, console_handle);

	interruptEnable();
	
	for (uint8_t i = 0; i < 5; i++)
		consoleWrtInitStr(console_handle, (initStr_e)i);
	
	if (RST.STATUS & _BV(RST_SDRF_bp))
		consoleWrtResetCause(console_handle, rstSpike);
	if (RST.STATUS & _BV(RST_SRF_bp))
		consoleWrtResetCause(console_handle, rstSw);
	if (RST.STATUS & _BV(RST_PDIRF_bp))
		consoleWrtResetCause(console_handle, rstDebug);
	if (RST.STATUS & _BV(RST_WDRF_bp))
		consoleWrtResetCause(console_handle, rstWatchDog);
	if (RST.STATUS & _BV(RST_BORF_bp))
		consoleWrtResetCause(console_handle, rstBrownout);
	if (RST.STATUS & _BV(RST_EXTRF_bp))
		consoleWrtResetCause(console_handle, rstExternal);
	if (RST.STATUS & _BV(RST_PORF_bp))
		consoleWrtResetCause(console_handle, rstPoweron);
		
	RST.STATUS = 0xff;

	consoleWrtInitStr(console_handle, initRgbStr);
	
	rgbLed_handle = rgbLedInit(&rgbLed_obj, sizeof(rgbLed_obj));
	if (rgbLedSetup(rgbLed_handle, AI_Hndl, &TCD0, PWM_UPPER_NIBBLE) >= 0)
		consoleWrtLiteral(console_handle, "ok\n\r");
	else
		consoleWrtLiteral(console_handle, "failed\n\r");	
	
	rgbLedCfgState(rgbLed_handle, LED_CFG_IDLE, (led_s){.color = COLOR_BLEU, .intensity = 8});
	rgbLedCfgState(rgbLed_handle, LED_CFG_FEEDBACK, (led_s){.intensity = 16});
	rgbLedCfgState(rgbLed_handle, LED_CFG_TOUCH_SHORT, (led_s){.color = COLOR_GREEN, .intensity = 8});
	rgbLedCfgState(rgbLed_handle, LED_CFG_TOUCH_DOUBLE, (led_s){.color = COLOR_WHITE, .intensity = 48});
	rgbLedCfgState(rgbLed_handle, LED_CFG_TOUCH_LONG, (led_s){.color = COLOR_YELLOW, .intensity = 128});
	rgbLedCfgState(rgbLed_handle, LED_CFG_ALERT, (led_s){.color = COLOR_RED, .intensity = 200});
	rgbLedCfgState(rgbLed_handle, LED_CFG_BLINK, (led_s){.color = COLOR_BLEU, .intensity = 8});

	rgbLedEnable(rgbLed_handle);
	rgbLedsetIntensity(rgbLed_handle, 8);

	consoleWrtInitStr(console_handle, initTouchStr);

	keyboard_fifo_handle = eventFifoInit(&keyboard_fifo, sizeof(keyboard_fifo));
	int8_t ef_result = eventFifoSetup(keyboard_fifo_handle, keyboard_fifo_buffer, KEYBOARD_FIFO_LENGTH);
	
	keyboard_handle = touchPadInit(&keyboard_obj, sizeof(keyboard_obj));

	padPortCfg[TOUCH_TOP].inputPort = &PORTC;
	padPortCfg[TOUCH_TOP].bitNumber = 4;
	padPortCfg[TOUCH_BOTTOM].inputPort = &PORTC;
	padPortCfg[TOUCH_BOTTOM].bitNumber = 6;
	padPortCfg[TOUCH_LEFT].inputPort = &PORTC;
	padPortCfg[TOUCH_LEFT].bitNumber = 7;
	padPortCfg[TOUCH_RIGHT].inputPort = &PORTC;
	padPortCfg[TOUCH_RIGHT].bitNumber = 5;
	padPortCfg[TOUCH_CENTER].inputPort = &PORTD;
	padPortCfg[TOUCH_CENTER].bitNumber = 7;

	int8_t k_result = touchPadSetup(keyboard_handle, AI_Hndl, &PORTC, 0, 1, padPortCfg, (TC0_t*)&TCC1, keyboard_event_callback);

	touchAction_handle = touchActionInit(&touchAction_obj, sizeof(touchAction_obj));
	int8_t ta_result = touchActionSetup(touchAction_handle, AI_Hndl);		

	int16_t doublePress = 0; // double press enable. bit 0 is top, bit 1 is bottom according to enum 
	int16_t longPress = 0; //long press enable. gives long press down and long press up event. bit 0 is top, bit 1 is bottom according to enum 
	int16_t edgePress = 0; //press enable on the edges touch and release. gives down and up event. disables long press and double press. bit 0 is top, bit 1 is bottom according to enum 
	
	for (int i = 0; i < TOUCH_COMBINATIONS_TOTAL; i++)
	{
		doublePress |= ((touchAction_handle->sensor.value[i].doublePress != PRESS_NO_ACTION) ? 1 : 0) << i;
		longPress |= ((touchAction_handle->sensor.value[i].longPress != PRESS_NO_ACTION) ? 1 : 0) << i;	
		edgePress |= ((touchAction_handle->sensor.value[i].action == ACTION_EDGE) ? 1 : 0) << i;
	}
	
	int8_t cfg_result = touchPadConfigure(keyboard_handle, PAD_CFG_QUAD_ARROW, edgePress, doublePress, longPress);
	
	if (ta_result >=0 && ef_result >= 0 && k_result >= 0 && cfg_result >= 0)
	consoleWrtLiteral(console_handle, "ok\n\r");
	else
	consoleWrtLiteral(console_handle, "failed\n\r");
	
	
	i2c_handle = i2cInit(&i2c_obj, sizeof(i2c_obj));
	i2cSetup(i2c_handle, &TWIE, 100000);

	consoleWrtInitStr(console_handle, initTempStr);
	
	tempHum_handle = tempHum_Init(&tempHum_obj, sizeof(tempHum_obj));
	if (tempHum_Setup(tempHum_handle, i2c_handle, AI_Hndl, 0) >=0)
		consoleWrtLiteral(console_handle, "ok\n\r");
	else
		consoleWrtLiteral(console_handle, "failed\n\r");
		
	colorFbk_Hndl = colorFeedback_Init(&colorFbk_obj, sizeof(colorFbk_obj));
	colorFeedback_Setup(colorFbk_Hndl, AI_Hndl, rgbLed_handle);

	setDeviceState(device_handle, DEVICE_NORMAL);
	
	AI_Hndl->AI_Reset_ind = &swReset;

	consoleWrtInitStr(console_handle, initOwnAddrStr);

	uint16_t ownAddr = devicegGetOwnAddress(device_handle);

	uint8_t str[16];
	consoleWrite(console_handle, str, snprintf((char*)str, 16, "%d.%d.%d\n\r", ownAddr >> 12, (ownAddr >> 8) & 0xf, ownAddr & 0xff));
	
	consoleWrtInitStr(console_handle, initFinishStr);
	
	cmdLineRegisterUsrCmds(cli_handle, usrCommands, USR_COMMANDS_N);

	cmdLineEnable(cli_handle);

// ****************************************************		
//  main loop
// ****************************************************	
	PORTE.OUTCLR = 0x08;
    while (1) 
    {
		//PORTE.OUTTGL = 0x08;
		if (everySecondTrigger)
		{
			everySecondTrigger = 0;
			tempHum_everySecond(tempHum_handle);
		}
	
	
// ****************************************************		
//  control rgb led
// ****************************************************		
		touch_state_e state = 0;
		if (touchPadGetState(keyboard_handle, &state) >= 0) {
			switch(state) {
				case TOUCH_TOP:
				case TOUCH_BOTTOM:
				case TOUCH_LEFT:
				case TOUCH_RIGHT:
				case TOUCH_TOP_BOTTOM:
				case TOUCH_LEFT_RIGHT:
				{
					if (rgbLed_obj.state == LED_IDLE)
						rgbLedSetState(rgbLed_handle, LED_TOUCH);
				}
				break;
				case TOUCH_ERROR:
					rgbLedSetState(rgbLed_handle, LED_BLINK);
				break;
				case TOUCH_ALIVE:
					rgbLedSetState(rgbLed_handle, LED_IDLE);
				break;
				default:
					rgbLedSetState(rgbLed_handle, LED_IDLE);
				break;
			}
		}
// ****************************************************		
// handle touch events
// ****************************************************		
		touch_event_e tevent;
		touch_state_e pad;
		if (popEventFifoBuffer(keyboard_fifo_handle, &tevent, &pad) > 0)
		{
			if (pad < TOUCH_RESET)
				touchAction(touchAction_handle, pad, tevent);
			consoleEchoTouch(console_handle, pad, tevent);
			switch (tevent) {
				case TOUCH_DOUBLE_PRESS_EVENT:
					rgbLedSetState(rgbLed_handle, LED_TOUCH_DOUBLE);
				break;
				case TOUCH_LONG_PRESS_DOWN_EVENT:
					rgbLedSetState(rgbLed_handle, LED_TOUCH_LONG);
				break;
				default:
				break;
			}
		}
// ****************************************************
// handle keyboard input from host
// ****************************************************

	cmdLineService(cli_handle);

// ****************************************************
// KNX communication stack
// ****************************************************		
	
	AI_Service(AI_Hndl);

// ****************************************************
// handle programming button and device state
// ****************************************************

	deviceService(device_handle);

// ****************************************************
// temperature and humidity measurement
// ****************************************************

	tempHum_Service(tempHum_handle);

    } // while(1)
}

void Cmd_TIMESTAMP(void *parentHandle) {
	uint32_t ts = getTimestamp_ms();
	uint8_t str[32];
	uint16_t ts1 = ts >> 16;
	uint16_t ts2 = ts & 0xffff;
	consoleWrtLiteral(console_handle, "timestamp : ");
	consoleWrite(console_handle, str, snprintf((char*)str, 32, "%04x", ts1));
	consoleWrite(console_handle, str, snprintf((char*)str, 32, "%04x", ts2));
	consoleNewline(console_handle);
}

void Cmd_OWNADDR(void *parentHandle) {
	uint16_t ownAddr = eeprom_read_word((uint16_t*) 0);
	uint8_t str[32];
	consoleWrtLiteral(console_handle, "Own address : ");
	consoleWrite(console_handle, str, snprintf((char*)str, 32, "%d.%d.%d", ownAddr >> 12, (ownAddr >> 8) & 0xf, ownAddr & 0xff));
	consoleNewline(console_handle);
}

void Cmd_AMBIENT(void *parentHandle) {
	tempHumidityStartMeasurementToString(tempHum_handle, &echoTemperatures);
	cmdLineDisable(cli_handle);
}

void echoTemperatures(char *str, uint8_t length) {
	consoleWrite(console_handle, str, length);
	consoleNewline(console_handle);
	cmdLineEnable(cli_handle);
}

void swReset() {
	
	timeoutTmr	rstDelay;
	setupTimeoutTmr(&rstDelay, 40); // 40 milliseconds delay, to finish reply to client.
	while (isNotTimedout(&rstDelay));
	
	uint8_t *ccp;
	ccp = (uint8_t*) &CPU_CCP;
	*ccp = CCP_IOREG_gc;
	RST.CTRL = _BV(RST_SWRST_bp);
}

// function everySecond() is called from an interrupt serice routine. Trigger is set to take action in the main loop.

void everySecond() {
	everySecondTrigger = 1; 
}

void keyboard_event_callback(touch_event_e tevent, touch_state_e pad) {
	pushEventFifoBuffer(keyboard_fifo_handle, tevent, pad);
}

ISR(TCC1_OVF_vect) {
	touchPadIsr(keyboard_handle);
}

ISR(TCD0_OVF_vect) {
	rgbLedIsr(rgbLed_handle);
}

ISR(TWIE_TWIM_vect) {
	i2cMasterIsr(i2c_handle);
}
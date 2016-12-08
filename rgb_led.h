/*
 * rgb_led.h
 *
 * Created: 1-6-2016 7:25:09
 *  Author: Paul
 */ 


#ifndef RGB_LED_H_
#define RGB_LED_H_

#include "applicationInterface.h"

#define RGB_BLICK_ON	100
#define RGB_BLICK_OFF	500

#define LED_DEFAULT_INTENSITY	0xff

#define LED_CFG_CNT		7

typedef enum {
	LED_IDLE,
	LED_TOUCH,
	LED_TOUCH_DOUBLE,
	LED_TOUCH_LONG,
	LED_BLINK,
	LED_OFF
} led_state_e;

typedef enum {
	LED_IDLE_DEFAULT,
	LED_IDLE_FEEDBACK	
} led_idle_e;

typedef enum {
	LED_ALERT_NONE,
	LED_ALERT_CONTINUOUS,
	LED_ALERT_BLINK,
	LED_ALERT_FADE,
	LED_ALERT_FLASH
} led_alert_e;

typedef enum {
	COLOR_RED = 0x1,
	COLOR_GREEN = 0x2,
	COLOR_BLEU = 0x3,
	COLOR_YELLOW = 0x4,
	COLOR_ORANGE = 0x5,
	COLOR_MAGENTA = 0x6,
	COLOR_CYAN = 0x7,
	COLOR_WHITE = 0x8
} ledColor_e ;

typedef enum {
	PWM_UPPER_NIBBLE,
	PWM_LOWER_NIBBLE
} led_pwm_nibble_e;

typedef enum {
	LED_CFG_IDLE,
	LED_CFG_FEEDBACK,
	LED_CFG_TOUCH_SHORT,
	LED_CFG_TOUCH_DOUBLE,
	LED_CFG_TOUCH_LONG,
	LED_CFG_BLINK,
	LED_CFG_ALERT,
} led_cfg_e;

typedef struct {
	ledColor_e			color;
	uint8_t				intensity;
} led_s;

typedef struct _rgbLedObj {
	uint8_t				enable;
	A_InterfaceHandle	AI_Hndl;	
	
	led_state_e			state;
	led_idle_e			idleState;
	led_alert_e			alertState;
	TC0_t				*pwm;
	PORT_t				*port;
	uint8_t				red;
	uint8_t				green;
	uint8_t				blue;
	uint8_t				intensity;
	uint8_t				fadeIntensity;
	int16_t				blickCnt;

	// interface object
	
	// load control
	sGeneric10					loadCtrl;
	
	// PID_RGBLEG_CFG
	
	struct {
		uint16_t	cnt;
		led_s		value[LED_CFG_CNT];
		}				colorState;

} rgbLedObj;

typedef struct _rgbLedObj *rgbLedHandle;

rgbLedHandle rgbLedInit(void *pMemory, uint16_t numBytes);

int8_t rgbLedSetup(rgbLedHandle handle, A_InterfaceHandle AI_Hndl, void *pwmTmr, led_pwm_nibble_e nibble);

int8_t rgbLedPropInd(void *parentHandle, interfaceProperty *prop);

int8_t rgbLedEnable(rgbLedHandle handle);

int8_t rgbLedDisable(rgbLedHandle handle);

int8_t rgbLedOn(rgbLedHandle handle);

int8_t rgbLedOff(rgbLedHandle handle);

int8_t rgbLedsetIntensity(rgbLedHandle handle, uint8_t i);

int8_t rgbLedSetColorIntensity(rgbLedHandle handle, led_s ci);

int8_t rgbLedCfgState(rgbLedHandle handle, led_cfg_e state, led_s ci);

int8_t rgbLedSetIdleFeedbackColorIntensity(rgbLedHandle handle, ledColor_e color);

int8_t rgbLedSetPwms(rgbLedHandle handle);

int8_t rgbLedSetState(rgbLedHandle handle, led_state_e state);

int8_t rgbLedSetIdleState(rgbLedHandle handle, led_idle_e state);

int8_t rgbLedSetAlertState(rgbLedHandle handle, led_alert_e state);

void rgbLedIsr(rgbLedHandle handle);

#endif /* RGB_LED_H_ */
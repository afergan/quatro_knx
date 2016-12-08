/*
 * rgb_led.c
 *
 * Created: 1-6-2016 7:24:53
 *  Author: Paul
 */ 

#include "clocks.h"
#include "interrupt.h"
#include "rgb_led.h"

rgbLedHandle rgbLedInit(void *pMemory, uint16_t numBytes)
{
	rgbLedHandle rgbLed_handle;
	if (numBytes < sizeof(rgbLedObj))
	return ((rgbLedHandle)NULL);
	rgbLed_handle = (rgbLedHandle)pMemory;
	return(rgbLed_handle);
}

int8_t rgbLedSetup(rgbLedHandle handle, A_InterfaceHandle AI_Hndl, void *pwmTmr, led_pwm_nibble_e nibble) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;

	rgbLed->AI_Hndl = AI_Hndl;
	
	AI_Hndl->rgbLedCfgObjectHndl = handle;
	AI_Hndl->AI_rgbLedCfgObject_ind = &rgbLedPropInd;
	
	rgbLed->red = 0x00;
	rgbLed->green = 0x00;
	rgbLed->blue = 0x00;

	switch((intptr_t)pwmTmr) {
		case (intptr_t)&TCC0:
			rgbLed->port = &PORTC;
		break;
		case (intptr_t)&TCD0:
			rgbLed->port = &PORTD;		
		break;
		case (intptr_t)&TCE0:
			rgbLed->port = &PORTE;			
		break;
/*
		case (intptr_t)&TCF0:
			rgbLed->port = &PORTF;	
		break;*/
		default:
			rgbLed->port = NULL;
		break;
	}

	if (nibble == PWM_UPPER_NIBBLE) {
		rgbLed->port->PIN4CTRL |= _BV(PORT_INVEN_bp);
		rgbLed->port->PIN5CTRL |= _BV(PORT_INVEN_bp);
		rgbLed->port->PIN6CTRL |= _BV(PORT_INVEN_bp);
		rgbLed->port->OUT &= ~(0x70);
		rgbLed->port->DIR |= 0x70;
		rgbLed->port->REMAP = 0x07;
	} else {
		rgbLed->port->PIN0CTRL |= _BV(PORT_INVEN_bp);
		rgbLed->port->PIN1CTRL |= _BV(PORT_INVEN_bp);
		rgbLed->port->PIN2CTRL |= _BV(PORT_INVEN_bp);
		rgbLed->port->OUT &= ~(0x07);
		rgbLed->port->DIR |= 0x07;
		rgbLed->port->REMAP = 0x00;
	}
	
	rgbLed->pwm = (TC0_t *)pwmTmr;
	rgbLed->pwm->PERBUF = 1 << 13;
	rgbLed->pwm->CCA = 0;
	rgbLed->pwm->CCB = 0;
	rgbLed->pwm->CCC = 0;
	rgbLed->pwm->CTRLB = _BV(TC0_CCAEN_bp) | _BV(TC0_CCBEN_bp) | _BV(TC0_CCCEN_bp) | TC_WGMODE_SS_gc;
	rgbLed->pwm->CTRLC = 0x00;
	rgbLed->pwm->CTRLD = 0;
	rgbLed->pwm->CTRLE = 0;
	rgbLed->pwm->CTRLA = TC_CLKSEL_DIV1_gc;
	rgbLed->pwm->INTCTRLA = TC_OVFINTLVL_MED_gc;
	
	rgbLed->state = LED_OFF;
	rgbLed->alertState = LED_ALERT_NONE;
	
	rgbLed->loadCtrl.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, RGB_LED_CFG_OBJECT_IDX, PID_LOAD_STATE_CONTROL, &rgbLed->loadCtrl);
	rgbLed->colorState.cnt = SWAP_UINT16(LED_CFG_CNT);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, RGB_LED_CFG_OBJECT_IDX, PID_RGBLEG_CFG, &rgbLed->colorState);

	return 0;
}

int8_t rgbLedPropInd(void *parentHandle, interfaceProperty *prop) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) parentHandle;

	switch((application_property_id_e)prop->propertyId) {
		case PID_LOAD_STATE_CONTROL:
		
		break;
		case PID_RGBLEG_CFG:
		{

		}
		break;
		default:
		return -1;
	}	
	return 1;	
}

int8_t rgbLedEnable(rgbLedHandle handle) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;
	rgbLed->enable = 1;
	rgbLed->state = LED_IDLE;
	return rgbLedSetPwms(handle);
}

int8_t rgbLedDisable(rgbLedHandle handle) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;
	rgbLed->pwm->CCA = 0;
	rgbLed->pwm->CCB = 0;
	rgbLed->pwm->CCC = 0;
	rgbLed->enable = 0;
	rgbLed->state = LED_OFF;
	return 0;
}

int8_t rgbLedOn(rgbLedHandle handle) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;
	if (!rgbLed->enable)
		return -1;
	rgbLed->state = LED_IDLE;
	
	return rgbLedSetPwms(handle);	
}

int8_t rgbLedOff(rgbLedHandle handle) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;
	if (!rgbLed->enable)
		return -1;
	rgbLed->state = LED_OFF;
	return rgbLedSetPwms(handle);
}

int8_t rgbLedsetIntensity(rgbLedHandle handle, uint8_t i) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;
	if (!rgbLed->enable)
		return -1;
	rgbLed->intensity = i;
	return rgbLedSetPwms(handle);
}

int8_t rgbLedSetColorIntensity(rgbLedHandle handle, led_s ci) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;
	if (!rgbLed->enable)
		return -1;
	rgbLed->intensity = ci.intensity;
	switch(ci.color) {
		case COLOR_RED:
			rgbLed->red = 0xff;
			rgbLed->green = 0x00;
			rgbLed->blue = 0x00;
		break;
		case COLOR_GREEN:
			rgbLed->red = 0x00;
			rgbLed->green = 0xff;
			rgbLed->blue = 0x00;		
		break;
		case COLOR_BLEU:
			rgbLed->red = 0x00;
			rgbLed->green = 0x00;
			rgbLed->blue = 0xff;		
		break;
		case COLOR_YELLOW:
			rgbLed->red = 0xff;
			rgbLed->green = 0xff;
			rgbLed->blue = 0x00;		
		break;
		case COLOR_ORANGE:
			rgbLed->red = 0xff;
			rgbLed->green = 0xbf;
			rgbLed->blue = 0x00;		
		break;
		case COLOR_MAGENTA:
			rgbLed->red = 0xff;
			rgbLed->green = 0x00;
			rgbLed->blue = 0xff;		
		break;
		case COLOR_CYAN:
			rgbLed->red = 0x00;
			rgbLed->green = 0xff;
			rgbLed->blue = 0xff;		
		break;
		case COLOR_WHITE:
			rgbLed->red = 0xff;
			rgbLed->green = 0xff;
			rgbLed->blue = 0xff;		
		break;
		default:
			rgbLed->red = 0xff;
			rgbLed->green = 0xff;
			rgbLed->blue = 0xff;		
		break;
	}
	return rgbLedSetPwms(handle);	
}

int8_t rgbLedCfgState(rgbLedHandle handle, led_cfg_e state, led_s ci) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;
	
	rgbLed->colorState.value[state].color = ci.color;
	rgbLed->colorState.value[state].intensity = ci.intensity;
	
	return 0;
}

int8_t rgbLedSetIdleFeedbackColorIntensity(rgbLedHandle handle, ledColor_e color) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;
	rgbLed->colorState.value[LED_CFG_FEEDBACK].color= color;
	return 0;
}

int8_t rgbLedSetPwms(rgbLedHandle handle) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;
	if (!rgbLed->enable)
		return -1;
	if (rgbLed->state == LED_OFF) {
		rgbLed->pwm->CCABUF = 0;
		rgbLed->pwm->CCBBUF = 0;
		rgbLed->pwm->CCCBUF = 0;
		return 0;
	}
	rgbLed->pwm->CCABUF = (uint16_t)(rgbLed->red * rgbLed->intensity) >> 2;
	rgbLed->pwm->CCBBUF = (uint16_t)(rgbLed->green * rgbLed->intensity) >> 2;	
	rgbLed->pwm->CCCBUF = (uint16_t)(rgbLed->blue * rgbLed->intensity) >> 2;
	return 0;	
}

int8_t rgbLedSetState(rgbLedHandle handle, led_state_e state) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;
	if (!rgbLed->enable)
		return -1;
	rgbLed->state = state;
	
	return 0;	
}


int8_t rgbLedSetIdleState(rgbLedHandle handle, led_idle_e state) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;
	if (!rgbLed->enable)
		return -1;
	rgbLed->idleState = state;
	
	return 0;	
}

int8_t rgbLedSetAlertState(rgbLedHandle handle, led_alert_e state) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;
	if (!rgbLed->enable)
		return -1;
	rgbLed->alertState = state;
	
	return 0;	
}

void rgbLedIsr(rgbLedHandle handle) {
	rgbLedObj *rgbLed;
	rgbLed = (rgbLedObj *) handle;
	if (rgbLed->alertState == LED_ALERT_CONTINUOUS)
	{
		rgbLedSetColorIntensity(handle, rgbLed->colorState.value[LED_CFG_ALERT]);
	}
	else if (rgbLed->alertState == LED_ALERT_BLINK)
	{
		if (rgbLed->blickCnt < 0)
			rgbLed->blickCnt--;
		else
			rgbLed->blickCnt++;
		if (rgbLed->blickCnt > RGB_BLICK_ON) {
			rgbLed->blickCnt = -1;
			rgbLed->pwm->CCABUF	= 0;
			rgbLed->pwm->CCBBUF	= 0;
			rgbLed->pwm->CCCBUF	= 0;		
		} else if (rgbLed->blickCnt < (-1 * RGB_BLICK_OFF)) {
			rgbLed->blickCnt = 0;
			rgbLedSetColorIntensity(handle,rgbLed->colorState.value[LED_CFG_ALERT]);
		}		
	}
	else if (rgbLed->alertState == LED_ALERT_FLASH)
	{
		if (rgbLed->blickCnt < 0)
			rgbLed->blickCnt--;
		else
			rgbLed->blickCnt += 2;
		if (rgbLed->blickCnt > RGB_BLICK_ON) {
			rgbLed->blickCnt = -1;
			rgbLed->pwm->CCABUF	= 0;
			rgbLed->pwm->CCBBUF	= 0;
			rgbLed->pwm->CCCBUF	= 0;		
		} else if (rgbLed->blickCnt < (-1 * RGB_BLICK_OFF)) {
			rgbLed->blickCnt = 0;
			rgbLedSetColorIntensity(handle, rgbLed->colorState.value[LED_CFG_ALERT]);
		}			
	}
	else if (rgbLed->alertState == LED_ALERT_FADE)
	{
		if (rgbLed->blickCnt++ > RGB_BLICK_ON)
		{
			rgbLed->blickCnt = 0;
			rgbLed->fadeIntensity = rgbLed->colorState.value[LED_CFG_ALERT].intensity;	
		}
		else
			if (!rgbLed->fadeIntensity)
				rgbLed->fadeIntensity--;
		rgbLedSetColorIntensity(handle, rgbLed->colorState.value[LED_CFG_ALERT]);
	}	
	else  // alert none
	{
		if (rgbLed->state == LED_BLINK) {
			if (rgbLed->blickCnt < 0)
				rgbLed->blickCnt--;
			else
				rgbLed->blickCnt++;
			if (rgbLed->blickCnt > RGB_BLICK_ON) {
				rgbLed->blickCnt = -1;
				rgbLed->pwm->CCABUF	= 0;
				rgbLed->pwm->CCBBUF	= 0;
				rgbLed->pwm->CCCBUF	= 0;		
			} else if (rgbLed->blickCnt < (-1 * RGB_BLICK_OFF)) {
				rgbLed->blickCnt = 0;
				rgbLedSetColorIntensity(handle, rgbLed->colorState.value[LED_CFG_BLINK]);
			}
		}
		else if (rgbLed->state == LED_IDLE) {
			if (rgbLed->idleState == LED_IDLE_FEEDBACK)
				rgbLedSetColorIntensity(handle, rgbLed->colorState.value[LED_CFG_FEEDBACK]);
			else
				rgbLedSetColorIntensity(handle, rgbLed->colorState.value[LED_CFG_IDLE]);
			}
		else if (rgbLed->state == LED_TOUCH) {
			rgbLedSetColorIntensity(handle, rgbLed->colorState.value[LED_CFG_TOUCH_SHORT]);
		}
		else if (rgbLed->state == LED_TOUCH_DOUBLE) {
			rgbLedSetColorIntensity(handle, rgbLed->colorState.value[LED_CFG_TOUCH_DOUBLE]);
		}
		else if (rgbLed->state == LED_TOUCH_LONG) {
			rgbLedSetColorIntensity(handle, rgbLed->colorState.value[LED_CFG_TOUCH_LONG]);
		}
		else if (rgbLed->state == LED_OFF) {
			rgbLedOff(handle);
		}		
	}
}
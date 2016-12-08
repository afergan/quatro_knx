/*
 * ui.c
 *
 * Created: 8-6-2016 7:51:43
 *  Author: Paul
 */ 

#include "clocks.h"
#include "interrupt.h"
#include "touchPad.h"
#include "interfaceObjectServer.h"
#include <string.h>

touchPadHandle touchPadInit(void *pMemory, uint16_t numBytes) {
	touchPadHandle keyboard_handle;
	if (numBytes < sizeof(touchPadObj))
		return ((touchPadHandle)NULL);
	keyboard_handle = (touchPadHandle)pMemory;
	return(keyboard_handle);
}

int8_t touchPadSetup(touchPadHandle handle, A_InterfaceHandle AI_Hndl, PORT_t *cPort, uint8_t eb, uint8_t sb, pad_port_config *padPortCfg, TC0_t *tmr, event_cb_t eCb) {
	touchPadObj *kboard;
	kboard = (touchPadObj *) handle;
	
	kboard->AI_Hndl = AI_Hndl;
	
	AI_Hndl->touchpadCfgObjectHndl = handle;
	AI_Hndl->AI_touchpadCfgObject_ind = &touchPadPropInd;
	
	kboard->padPortCfg = padPortCfg;
	kboard->ctrlPort = cPort;
	kboard->syncTmr = tmr;
	kboard->enableBit = eb;
	kboard->syncBit = sb;
	kboard->ctrlPort->DIR |= _BV(kboard->enableBit) | _BV(kboard->syncBit);
	
	kboard->syncTmr->CTRLB = _BV(TC0_CCAEN_bp);

	kboard->syncTmr->CTRLC = 0;
	kboard->syncTmr->CTRLD = _BV(TC0_EVACT0_bp) | _BV(TC0_EVSEL3_bp);
	kboard->syncTmr->CTRLE = 0;
	kboard->syncTmr->INTCTRLA = 0;
	kboard->syncTmr->INTCTRLB = 0;
	kboard->syncTmr->PERBUF = ((F_CPU * 1LL * TOUCH_SCAN_PERIOD_USEC) / (64 * 1000000LL));
	kboard->syncTmr->CTRLFSET = TC0_CMD0_bm; // update
	kboard->syncTmr->CTRLA = TC_CLKSEL_DIV64_gc;

	kboard->healthChkPinCycle = 0;

	EVSYS.CH0CTRL = EVSYS_DIGFILT_4SAMPLES_gc;
	
	kboard->longPressEnableMask = 0;
	kboard->doublePressEnableMask = 0;
	kboard->edgePressEnableMask = 0;

	kboard->eventCb = eCb;
	
	kboard->state = TOUCH_RESET;
	
	//uint8_t loaded = OBJECT_LOAD_STATE_LOADED;
	
	//IntfObj_setPropValue(IntfObj_Srv_getObjByPropId(kboard->AI_Hndl->IO_Hndl, TOUCHPAD_CFG_OBJECT_IDX, PID_LOAD_STATE_CONTROL), 1, 1, &loaded);
	
	kboard->loadCtrl.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, TOUCHPAD_CFG_OBJECT_IDX, PID_LOAD_STATE_CONTROL, &kboard->loadCtrl);
	
	kboard->configuration.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(kboard->AI_Hndl->IO_Hndl, TOUCHPAD_CFG_OBJECT_IDX, PID_TOUCHPAD_CFG, &kboard->configuration);
	
	kboard->loadCtrl.value[0] = OBJECT_LOAD_STATE_LOADED;
	
	return 0;
}

int8_t touchPadConfigure(touchPadHandle handle,  pad_config_e padCfg, int16_t edgePressEnableMask, int16_t doublePressEnableMask, int16_t longPressEnableMask) {
	volatile touchPadObj *kboard;
	kboard = (touchPadObj *) handle;

	// set enabled bits according to configuration
	
	touchPadDisable(handle);

	kboard->configuration.value = padCfg;
	switch (padCfg){
		case PAD_CFG_QUAD_ARROW:
		kboard->shortPressEnableMask = (_BV(TOUCH_TOP) | _BV(TOUCH_BOTTOM) | _BV(TOUCH_LEFT) | _BV(TOUCH_RIGHT) | _BV(TOUCH_TOP_BOTTOM) | _BV(TOUCH_LEFT_RIGHT));
		break;
		
		// todo : code rest of the configurations
		
		default:
		break;
	}

	for (uint8_t i = 0; i < TOUCHPAD_TOTAL; i++)
	{
		kboard->padPortCfg[i].enabled = (kboard->shortPressEnableMask & (1 << i)) ? 1 : 0;
		
		uint8_t eventmask = 0;
		switch((intptr_t)kboard->padPortCfg[i].inputPort) {
			case (intptr_t)&PORTC:
				eventmask = 0b01100000;
			break;
			case (intptr_t)&PORTD:
				eventmask = 0b01101000;		
			break;
			case (intptr_t)&PORTE:
				eventmask = 0b01110000;			
			break;
	/*
			case (intptr_t)&PORTF:
				eventmask = 0b01111000;		
			break;*/
		}

		kboard->padPortCfg[i].eventMux = eventmask | (kboard->padPortCfg[i].bitNumber & 0x7);
	}

	kboard->healthChkPinCycle = 0;
	
	EVSYS.CH0MUX = kboard->padPortCfg[kboard->healthChkPinCycle].eventMux;

	kboard->longPressEnableMask = longPressEnableMask;
	kboard->doublePressEnableMask = doublePressEnableMask;
	kboard->edgePressEnableMask = edgePressEnableMask;
	
	touchPadReset(handle);
	
	timeoutTmr initTimeout;
	setupTimeoutTmr(&initTimeout, 500); // 0.5 sec to start. touchPad ISR is running.
	
	while ((kboard->healthIndication != ((1 << TOUCHPAD_TOTAL) - 1)) && isNotTimedout(&initTimeout)); // check capture on input bit 0
	
	if ((kboard->healthIndication == ((1 << TOUCHPAD_TOTAL) - 1)))
	{
		kboard->enabled = 1;
		kboard->state = TOUCH_ALIVE;
		return 0;
	}
	else
	{
		kboard->enabled = 1;
		return -1;		
	}	
		
}

int8_t touchPadEnable(touchPadHandle handle) {
	touchPadObj *kboard;
	kboard = (touchPadObj *) handle;
	kboard->enabled = 1;
	return touchPadReset(handle);
}

int8_t touchPadDisable(touchPadHandle handle) {
	touchPadObj *kboard;
	kboard = (touchPadObj *) handle;
	kboard->enabled = 0;
	kboard->syncTmr->INTCTRLA = 0;
	kboard->ctrlPort->OUT &= ~(_BV(kboard->enableBit) | _BV(kboard->syncBit)); // turn off power.
	return 0;	
}

int8_t touchPadReset(touchPadHandle handle) {
	touchPadObj *kboard;
	kboard = (touchPadObj *) handle;
	
	if (kboard->state != TOUCH_RESET)
		touchPadRaiseEvent(handle, TOUCH_RESET_EVENT, kboard->state); // reset event
	
	kboard->state = TOUCH_RESET;
	// reset touch pad IC's
	kboard->syncTmr->INTCTRLA = 0; // disable sync interrupt
	kboard->ctrlPort->OUT &= ~(_BV(kboard->enableBit) | _BV(kboard->syncBit)); // turn off power and wait for 100 msec's.
	_delay_ms(100);
	kboard->ctrlPort->OUT |= _BV(kboard->enableBit); // supply power to touch IC's and wait another 120 msec's.
	_delay_ms(120);

	kboard->healthChkBadCnt = 0;
	kboard->syncTmr->CTRLFSET = TC0_CMD1_bm; // restart sync generator
	kboard->syncTmr->INTCTRLA = TC_OVFINTLVL_MED_gc; // enable sync interrupt
	return 0;
}


void touchPadIsr(touchPadHandle handle) {
	touchPadObj *kboard;
	kboard = (touchPadObj *) handle;
	
	// Health check. change pull-up to pull-down on the input, depending on state of the input.
	
	for (int i = 0; i < TOUCHPAD_TOTAL; i++) {
		if (kboard->padPortCfg[i].inputPort->IN & _BV(kboard->padPortCfg[i].bitNumber))
			*(&(kboard->padPortCfg[i].inputPort->PIN0CTRL) + kboard->padPortCfg[i].bitNumber) = PORT_OPC_PULLDOWN_gc | PORT_ISC_RISING_gc;
		else
			*(&(kboard->padPortCfg[i].inputPort->PIN0CTRL) + kboard->padPortCfg[i].bitNumber) = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;
	}

	// debounce inputs
	
	int8_t edgeChange = 0;
	if (isTimedout(&kboard->debounce)) {
		// remap input bit to bit number according to enum
	
		uint8_t rawTouchState = 0;
	
		for (uint8_t i = 0; i < TOUCHPAD_TOTAL; i++)
			rawTouchState |= (((kboard->padPortCfg[i].inputPort->IN & _BV(kboard->padPortCfg[i].bitNumber) && kboard->padPortCfg[i].enabled) ? 1 : 0) << i);
			
		if (kboard->touchBitState != rawTouchState)
		{ // kboard->inputPort->IN & touchMask
			kboard->touchBitState = rawTouchState;
			setupTimeoutTmr(&kboard->debounce, TOUCH_DEBOUNCE_MS);
		}
		else
		{
			if (kboard->debouncedTouchBitState != kboard->touchBitState) {
				kboard->debouncedTouchBitState = kboard->touchBitState;
				edgeChange = 1;
			}
		}
	}

	// check touchpad health.
	if (kboard->padPortCfg[kboard->healthChkPinCycle].enabled)
	{
		if (kboard->syncTmr->CTRLGSET & _BV(TC0_CCABV_bp))
		{
			kboard->healthIndication |= (1 << kboard->healthChkPinCycle);
		}
		else
		{
			kboard->healthIndication &= ~(1 << kboard->healthChkPinCycle);
			kboard->healthChkBadCnt++;
		}
	}
	else
		kboard->healthIndication |= (1 << kboard->healthChkPinCycle); // when input disabled, write 1 (health ok).

	
	if (kboard->state == TOUCH_RESET && (kboard->healthIndication == ((1 << TOUCHPAD_TOTAL) - 1)))
	{
		kboard->state = TOUCH_ALIVE;
		touchPadRaiseEvent(handle, TOUCH_COMING_ALIVE_EVENT, kboard->state);
	}
	
	// check pins in round robin way. Every cycle a pad is checked.
	
	kboard->healthChkPinCycle++;
	if (kboard->healthChkPinCycle == TOUCHPAD_TOTAL)
		kboard->healthChkPinCycle = 0;	
	EVSYS.CH0MUX = kboard->padPortCfg[kboard->healthChkPinCycle].eventMux;

	kboard->syncTmr->INTFLAGS = 0xff; // reset all pending interrupts before activate
	
	if (kboard->healthChkBadCnt > TOUCH_BAD_CNT_THRESHOLD)
	{
		touchPadReset(handle);
	}
	
	if (kboard->state != TOUCH_RESET && kboard->enabled)
	{
		touchPadStateAndEvents(handle, edgeChange);
	}

	kboard->ctrlPort->OUTTGL = _BV(kboard->syncBit);  // toggle sync pin touch	

}

void touchPadStateAndEvents(touchPadHandle handle, uint8_t edgeChange) {
	touchPadObj *kboard;
	kboard = (touchPadObj *) handle;
	// sequence inputs in time
	
	touch_state_e tmpState = touchPadMapBitToState(handle, kboard->debouncedTouchBitState);

	if (edgeChange && kboard->sequence == TOUCH_SEQ_IDLE) { // start of a sequence
		if (tmpState == TOUCH_ALIVE) {
			kboard->state = tmpState;
			kboard->sequence = TOUCH_SEQ_IDLE;
		}
		else if (tmpState != TOUCH_ERROR)
		{
			kboard->state = tmpState;
			if (kboard->state < TOUCH_RESET && (kboard->edgePressEnableMask & (1 << kboard->state)))
			{
				touchPadRaiseEvent(handle, TOUCH_EVENT_DOWN, kboard->state); // touch down event
			}

			setupTimeoutTmr(&kboard->steadyState, STEADY_TOUCH_MS);
			setupTimeoutTmr(&kboard->timeout, TOUCH_TIMEOUT_MS);
			setupTimeoutTmr(&kboard->longTouch, LONG_TOUCH_MS);
			
			kboard->sequence = TOUCH_SEQ_1;
		} else
		{
			kboard->sequence = TOUCH_SEQ_ERROR;
		}
	}
	else if (kboard->sequence == TOUCH_SEQ_1) // wait for touch pad to be released within time limit, or signal long press.
	{
		if (isNotTimedout(&kboard->timeout))
		{
			if (tmpState == TOUCH_ALIVE)
			{
				if (kboard->state < TOUCH_RESET && (kboard->edgePressEnableMask & (1 << kboard->state))) // if edge press enabled --> raise up event.
				{
					touchPadRaiseEvent(handle, TOUCH_EVENT_UP, kboard->state); // touch up event
					setupTimeoutTmr(&kboard->steadyState, TOUCH_END_DELAY);
					setupTimeoutTmr(&kboard->timeout, TOUCH_TIMEOUT_MS);
					kboard->sequence = TOUCH_SEQ_4;
				}
				else
				{
					if (kboard->state < TOUCH_RESET && (kboard->doublePressEnableMask & (1 << kboard->state))) // when edge press disabled and double touch enabled, set time out for idle period between two touches
					{
						kboard->sequence = TOUCH_SEQ_2;
						setupTimeoutTmr(&kboard->timeout, TOUCH_DOUBLE_TIMEOUT_MS);
					}
					else
					{
						touchPadRaiseEvent(handle, TOUCH_SHORT_PRESS_EVENT, kboard->state); // when double touch disabled, raise short touch event and finish.
						setupTimeoutTmr(&kboard->steadyState, TOUCH_END_DELAY);
						setupTimeoutTmr(&kboard->timeout, TOUCH_TIMEOUT_MS);
						kboard->sequence = TOUCH_SEQ_4;
					}
				}
			}
			else if (edgeChange) // another pad is pressed. Check if multi touch
			{
				if (touchPadMultiTouchTransition(kboard->state , tmpState) == MULTI_TOUCH_PRESSED)
				{
					setupTimeoutTmr(&kboard->longTouch, LONG_TOUCH_MS);
					setupTimeoutTmr(&kboard->timeout, TOUCH_TIMEOUT_MS);
					setupTimeoutTmr(&kboard->steadyState, STEADY_TOUCH_MS);
					if (kboard->edgePressEnableMask & (1 << kboard->state))
						touchPadRaiseEvent(handle, TOUCH_EVENT_UP, kboard->state); // if edge press enabled, raise touch up event for single touch that is now becoming multi touch.
					kboard->state = tmpState; // multi touch
					if (kboard->state < TOUCH_RESET && (kboard->edgePressEnableMask & (1 << kboard->state)))
					{
						touchPadRaiseEvent(handle, TOUCH_EVENT_DOWN, kboard->state); // touch down event for multi-touch
					}
				}
				else
				{
					if (kboard->edgePressEnableMask & (1 << kboard->state))
						touchPadRaiseEvent(handle, TOUCH_EVENT_UP, kboard->state); // touch up event
					kboard->sequence = TOUCH_SEQ_ERROR;
				}
			} else if (isTimedout(&kboard->longTouch) && !(kboard->edgePressEnableMask & (1 << kboard->state))) // timer signaling we moving into long press. 
			{
				
				if (tmpState == kboard->state)
				{
					if (kboard->state < TOUCH_RESET && (kboard->longPressEnableMask & (1 << kboard->state)))
					{
						touchPadRaiseEvent(handle, TOUCH_LONG_PRESS_DOWN_EVENT, kboard->state); // long touch down
						setupTimeoutTmr(&kboard->timeout, TOUCH_TIMEOUT_MS);
						kboard->sequence = TOUCH_SEQ_5;
					}
				}
				else
				{
					kboard->sequence = TOUCH_SEQ_ERROR;
				}
			}
		}
		else // time is up and pad is not released. When edge press enabled raise up event and go to error.
		{
			if (kboard->edgePressEnableMask & (1 << kboard->state))
				touchPadRaiseEvent(handle, TOUCH_EVENT_UP, kboard->state); // touch up event
			kboard->sequence = TOUCH_SEQ_ERROR;
		}
	}
	else if (kboard->sequence == TOUCH_SEQ_2) // idle time between double touch
	{
		if (isNotTimedout(&kboard->timeout))
		{
			if (edgeChange)
			{
				if (isTimedout(&kboard->steadyState) && tmpState == kboard->state) {
					kboard->sequence = TOUCH_SEQ_3;
				} else
				{
					kboard->sequence = TOUCH_SEQ_ERROR;
				}
				setupTimeoutTmr(&kboard->steadyState, STEADY_TOUCH_MS);
				setupTimeoutTmr(&kboard->timeout, TOUCH_TIMEOUT_MS);
			}
		}
		else
		{
			touchPadRaiseEvent(handle, TOUCH_SHORT_PRESS_EVENT, kboard->state); // short touch
			kboard->state = TOUCH_ALIVE;
			kboard->sequence = TOUCH_SEQ_IDLE;
		}
	}
	else if (kboard->sequence == TOUCH_SEQ_3)  // 2nd touch of the double touch
	{
		if (isNotTimedout(&kboard->timeout))
		{
			if (isTimedout(&kboard->steadyState)) {
				if (tmpState == TOUCH_ALIVE) // double touch
				{
					touchPadRaiseEvent(handle, TOUCH_DOUBLE_PRESS_EVENT, kboard->state);
					setupTimeoutTmr(&kboard->steadyState, DOUBLE_TOUCH_STATE_DELAY);
					setupTimeoutTmr(&kboard->timeout, TOUCH_TIMEOUT_MS);
					kboard->sequence = TOUCH_SEQ_4;
				} else if (tmpState != kboard->state)
				{
					kboard->sequence = TOUCH_SEQ_ERROR;
				}
			}
		}
		else
		{
			kboard->sequence = TOUCH_SEQ_ERROR;
		}
	}
	else if (kboard->sequence == TOUCH_SEQ_4) // end of sequence. check no pads are pressed.
	{ 
		if (isNotTimedout(&kboard->timeout))
		{
			if (isTimedout(&kboard->steadyState))
			{
				kboard->state = TOUCH_ALIVE;
				if (tmpState == TOUCH_ALIVE)
				kboard->sequence = TOUCH_SEQ_IDLE;
			}
		}
		else
		{
			kboard->sequence = TOUCH_SEQ_ERROR;  // time is up. The pad(s) are not released in time.
		}
	}
	else if (kboard->sequence == TOUCH_SEQ_5)
	{ // long touch
		if (isNotTimedout(&kboard->timeout))
		{
			// check release of pad. Also multi touch. When one pad released of a multi touch combination, raise up event.
			if (tmpState == TOUCH_ALIVE || touchPadMultiTouchTransition(kboard->state , tmpState) == MULTI_TOUCH_RELEASED)
			{
				touchPadRaiseEvent(handle, TOUCH_LONG_PRESS_UP_EVENT, kboard->state);
				kboard->sequence = TOUCH_SEQ_4;
				setupTimeoutTmr(&kboard->steadyState, TOUCH_END_DELAY);
				setupTimeoutTmr(&kboard->timeout, TOUCH_TIMEOUT_MS);
			} else if (tmpState != kboard->state)
			{
				kboard->sequence = TOUCH_SEQ_ERROR; // another pad pressed, when we were waiting for all pads to be released.
			}
		}
		else
		{
			touchPadRaiseEvent(handle, TOUCH_LONG_PRESS_UP_EVENT, kboard->state);
			kboard->sequence = TOUCH_SEQ_ERROR; // long is not that long! Pad not released in time in a long press state.
		}
	}
	else if (kboard->sequence == TOUCH_SEQ_ERROR)
	{
		kboard->state = TOUCH_ERROR;
		kboard->sequence = TOUCH_SEQ_ERROR_END;
		setupTimeoutTmr(&kboard->timeout, TOUCH_ERROR_TIMEOUT_MS);
	}
	else if (kboard->sequence == TOUCH_SEQ_ERROR_END)
	{
		if (isNotTimedout(&kboard->timeout))
		{
			if (tmpState == TOUCH_ALIVE)
			{
				if (isTimedout(&kboard->steadyState))
				{
					kboard->state = TOUCH_ALIVE;
					kboard->sequence = TOUCH_SEQ_IDLE;
				}
			}
			else
			{
				setupTimeoutTmr(&kboard->steadyState, TOUCH_ERROR_END_DELAY);
			}
		}
		else
		{
			kboard->sequence = TOUCH_SEQ_RESET;
			setupTimeoutTmr(&kboard->timeout, TOUCH_RESET_TIME_MS);
			touchPadReset(handle);		
		}
	}
	else if (kboard->sequence == TOUCH_SEQ_RESET)
	{
		if (isNotTimedout(&kboard->timeout))
		{
			kboard->sequence = TOUCH_SEQ_ERROR_END;
			setupTimeoutTmr(&kboard->timeout, TOUCH_ERROR_TIMEOUT_MS);
		}
	}
}

int8_t touchPadRaiseEvent(touchPadHandle handle, touch_event_e tevent, touch_state_e state) {
	touchPadObj *kboard;
	kboard = (touchPadObj *) handle;
	if (kboard->eventCb != NULL) {
		(*(kboard->eventCb))(tevent, state);
		return 0;
	} else
		return -1;
}

touch_state_e touchPadMapBitToState(touchPadHandle handle, uint8_t touchBitState) {
	touchPadObj *kboard;
	kboard = (touchPadObj *) handle;
	
	touch_state_e state;
	
	if (kboard->state == TOUCH_RESET)
		return (touch_state_e)TOUCH_RESET;
	
	if (!(touchBitState ^ (1 << TOUCH_TOP)))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_TOP)) ? TOUCH_TOP : TOUCH_ALIVE;
	}
	else if (!(touchBitState ^ (1 << TOUCH_RIGHT)))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_RIGHT)) ? TOUCH_RIGHT : TOUCH_ALIVE;
	}
	else if (!(touchBitState ^ (1 << TOUCH_BOTTOM)))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_BOTTOM)) ? TOUCH_BOTTOM : TOUCH_ALIVE;
	}
	else if (!(touchBitState ^ (1 << TOUCH_LEFT)))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_LEFT)) ? TOUCH_LEFT : TOUCH_ALIVE;
	}
	else if (!(touchBitState ^ (1 << TOUCH_CENTER)))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_CENTER)) ? TOUCH_CENTER : TOUCH_ALIVE;
	}
	else if (!(touchBitState ^ ((1 << TOUCH_TOP) | (1 << TOUCH_BOTTOM))))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_TOP_BOTTOM)) ?  TOUCH_TOP_BOTTOM : TOUCH_ALIVE;
	}
	else if (!(touchBitState ^ ((1 << TOUCH_LEFT) | (1 << TOUCH_RIGHT))))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_LEFT_RIGHT)) ? TOUCH_LEFT_RIGHT : TOUCH_ALIVE;
	}
	else if (!(touchBitState ^ ((1 << TOUCH_TOP) | (1 << TOUCH_LEFT))))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_TOP_LEFT)) ? TOUCH_TOP_LEFT : TOUCH_ALIVE;
	}		
	else if (!(touchBitState ^ ((1 << TOUCH_TOP) | (1 << TOUCH_RIGHT))))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_TOP_RIGHT)) ? TOUCH_TOP_RIGHT : TOUCH_ALIVE;
	}
	else if (!(touchBitState ^ ((1 << TOUCH_BOTTOM) | (1 << TOUCH_LEFT))))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_BOTTOM_LEFT)) ? TOUCH_BOTTOM_LEFT : TOUCH_ALIVE;
	}
	else if (!(touchBitState ^ ((1 << TOUCH_BOTTOM) | (1 << TOUCH_RIGHT))))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_BOTTOM_RIGHT)) ? TOUCH_BOTTOM_RIGHT : TOUCH_ALIVE;
	}
	else if (!(touchBitState ^ ((1 << TOUCH_CENTER) | (1 << TOUCH_TOP))))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_CENTER_TOP)) ? TOUCH_CENTER_TOP : TOUCH_ALIVE;
	}
	else if (!(touchBitState ^ ((1 << TOUCH_CENTER) | (1 << TOUCH_BOTTOM))))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_CENTER_BOTTOM)) ? TOUCH_CENTER_BOTTOM : TOUCH_ALIVE;
	}
	else if (!(touchBitState ^ ((1 << TOUCH_CENTER) | (1 << TOUCH_LEFT))))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_CENTER_LEFT)) ? TOUCH_CENTER_LEFT : TOUCH_ALIVE;
	}
	else if (!(touchBitState ^ ((1 << TOUCH_CENTER) | (1 << TOUCH_RIGHT))))
	{
		state = (kboard->shortPressEnableMask & (1 << TOUCH_CENTER_RIGHT)) ? TOUCH_CENTER_RIGHT : TOUCH_ALIVE;
	}
	else if (touchBitState == 0) {
		state = TOUCH_ALIVE;
	} else
		state = TOUCH_ERROR;
	return state;
}

multi_touch_e touchPadMultiTouchTransition(touch_state_e before, touch_state_e now) {

	touch_state_e multiTouch = TOUCH_UNKNOWN;
	touch_state_e singleTouch = TOUCH_ERROR;
	multi_touch_e result = MULTI_TOUCH_ERROR;
	
	if (before > TOUCH_CENTER && before < TOUCH_RESET) // we are in a multi touch state
	{
		if (now < TOUCH_TOP_BOTTOM) // moving towards single touch
		{
			multiTouch = before;
			singleTouch = now;
			result = MULTI_TOUCH_RELEASED;
		}
		else
			return (multi_touch_e)MULTI_TOUCH_NONE;
	}
	else if (before < TOUCH_TOP_BOTTOM) // we are in a single touch state
	{
		if (now > TOUCH_CENTER && now < TOUCH_RESET) // moving towards multi touch
		{
			multiTouch = now;
			singleTouch = before;
			result = MULTI_TOUCH_PRESSED;
		}
			return (multi_touch_e)MULTI_TOUCH_NONE;
	}
	switch(multiTouch) // check valid multi to single transition (or visa versa)
	{
		case TOUCH_TOP_BOTTOM:
		return ((singleTouch == TOUCH_TOP || singleTouch == TOUCH_BOTTOM) ? result : (multi_touch_e)MULTI_TOUCH_ERROR);
		case TOUCH_LEFT_RIGHT:
		return ((singleTouch == TOUCH_LEFT || singleTouch == TOUCH_RIGHT) ? result : (multi_touch_e)MULTI_TOUCH_ERROR);
		case TOUCH_TOP_LEFT:
		return ((singleTouch == TOUCH_TOP || singleTouch == TOUCH_LEFT) ? result : (multi_touch_e)MULTI_TOUCH_ERROR);
		case TOUCH_TOP_RIGHT:
		return ((singleTouch == TOUCH_TOP || singleTouch == TOUCH_RIGHT) ? result : (multi_touch_e)MULTI_TOUCH_ERROR);
		case TOUCH_BOTTOM_LEFT:
		return ((singleTouch == TOUCH_BOTTOM || singleTouch == TOUCH_LEFT) ? result : (multi_touch_e)MULTI_TOUCH_ERROR);
		case TOUCH_BOTTOM_RIGHT:
		return ((singleTouch == TOUCH_BOTTOM || singleTouch == TOUCH_RIGHT) ? result : (multi_touch_e)MULTI_TOUCH_ERROR);
		case TOUCH_CENTER_TOP:
		return ((singleTouch == TOUCH_CENTER || singleTouch == TOUCH_TOP) ? result : (multi_touch_e)MULTI_TOUCH_ERROR);
		case TOUCH_CENTER_BOTTOM:
		return ((singleTouch == TOUCH_CENTER || singleTouch == TOUCH_BOTTOM) ? result : (multi_touch_e)MULTI_TOUCH_ERROR);
		case TOUCH_CENTER_LEFT:
		return ((singleTouch == TOUCH_CENTER || singleTouch == TOUCH_LEFT) ? result : (multi_touch_e)MULTI_TOUCH_ERROR);
		case TOUCH_CENTER_RIGHT:
		return ((singleTouch == TOUCH_CENTER || singleTouch == TOUCH_RIGHT) ? result : (multi_touch_e)MULTI_TOUCH_ERROR);
		default:
		break;
	}
	return result;
}


int8_t touchPadGetState(touchPadHandle handle, touch_state_e *state) {
	touchPadObj *kboard;
	kboard = (touchPadObj *) handle;
	if (kboard->enabled != 1)
		return -1;
	*state = kboard->state;
	return 0;
}

int8_t touchPadPropInd(void *parentHandle, interfaceProperty *prop) {
	touchPadObj *kboard;
	kboard = (touchPadObj *) parentHandle;

	switch((application_property_id_e)prop->propertyId) {
		case PID_LOAD_STATE_CONTROL:
		
		break;
		case PID_TOUCHPAD_CFG:
		{

		}
		break;
		default:
		return -1;
	}
	
	return 1;
}

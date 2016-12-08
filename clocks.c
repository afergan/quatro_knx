/*
 * clocks.c
 *
 * Created: 20-5-2016 7:18:41
 *  Author: Paul
 */ 

#include "clocks.h"
#include <avr/interrupt.h>
#include "util\atomic.h"

int8_t setupClks(timer_cb t_cb, int16_t cbIntervalms){
	
	OSC_t *osc;
	osc = (OSC_t*) &OSC_CTRL;
	
	osc->XOSCCTRL = (OSC_FRQRANGE_2TO9_gc | OSC_XOSCSEL_XTAL_16KCLK_gc);
	osc->CTRL = _BV(OSC_XOSCEN_bp) | _BV(OSC_RC2MEN_bp) | _BV(OSC_RC32KEN_bp);
	
	_delay_ms(2);
	
	uint8_t *ccp;
	ccp = (uint8_t*) &CPU_CCP;
	*ccp = CCP_IOREG_gc;
	
	CLK_t *clk;
	clk = (CLK_t*) &CLK_CTRL;
	
	clk->CTRL = (CLK_SCLKSEL_XOSC_gc);
	clk->RTCCTRL = CLK_RTCSRC_RCOSC_gc;   // 1024 Hz (roughly 1 ms period)
	
	clock.rtc = (RTC_t*) &RTC;

	clock.rtc->INTCTRL = RTC_OVFINTLVL_MED_gc;
	clock.rtc->PER = 0xffff; // 64 seconds before starting at zero.
	clock.rtc->CTRL = _BV(RTC_PRESCALER0_bp); // no prescaling
	
	if (t_cb != NULL && cbIntervalms > 0)
	{
		clock.timerCb = t_cb;
		clock.ms_per_interrupt = cbIntervalms;
		clock.rtc->COMP = cbIntervalms;
		clock.rtc->INTCTRL |= RTC_COMPINTLVL_MED_gc;
	}
	
	clock.rtc->CNT = 0; // reset clock

	clk->RTCCTRL |= _BV(CLK_RTCEN_bp); // enable RTC clock
	
	clock.enabled = 1;
	
	return 0;
}

uint32_t getTimestamp_ms() {
	uint32_t ts = clock.rtc->CNT;
	ts |= ((uint32_t)clock.timestamp_64s << 16);
	return ts;
}

int8_t setupTimeoutTmr(timeoutTmr *tmr, uint16_t time_ms) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		uint16_t ts = clock.rtc->CNT;
		if (ts > (ts + time_ms + 1000)) // an extra 1000 ms to prevent overflow
		{
			tmr->alt = TIMEOUT_B;
			tmr->timeout_ms = ts + 0x7fff + time_ms;
		}
		else
		{
			tmr->alt = TIMEOUT_A;
			tmr->timeout_ms = ts + time_ms;
		}		
	}
	return 0;
}

int8_t isTimedout(timeoutTmr *tmr) {
	uint8_t result = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		uint16_t ts = clock.rtc->CNT;
		if (tmr->alt == TIMEOUT_B)
			ts += 0x7fff;
		if (ts > tmr->timeout_ms || tmr->alt == TIMEDOUT)
		{
			result = 1;
			tmr->alt = TIMEDOUT; // retain state after time out
		}
		else
			result = 0;		
	}
	return result;
}

int8_t isNotTimedout(timeoutTmr *tmr) {
	uint8_t result = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		uint16_t ts = clock.rtc->CNT;
		if (tmr->alt == TIMEOUT_B)
			ts += 0x7fff;
		if (ts > tmr->timeout_ms || tmr->alt == TIMEDOUT)
		{
			result = 0;
			tmr->alt = TIMEDOUT; // retain state after time out
		}
		else
			result = 1;		
	}
	return result;
}

ISR(RTC_OVF_vect) {
	if (clock.enabled)
		clock.timestamp_64s++;
}

ISR(RTC_COMP_vect) {
	while(clock.rtc->STATUS & _BV(RTC_SYNCBUSY_bp));
	clock.rtc->COMP += clock.ms_per_interrupt;
	if (clock.timerCb) {
		clock.timerCb();
	}
}
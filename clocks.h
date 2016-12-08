/*
 * clocks.h
 *
 * Created: 20-5-2016 7:18:31
 *  Author: Paul
 */ 


#ifndef CLOCKS_H_
#define CLOCKS_H_

#define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

typedef void (*timer_cb)(void);

typedef enum {
	TIMEOUT_A,
	TIMEOUT_B,
	TIMEDOUT
} timeout_tmr_e;

typedef volatile struct {
	timeout_tmr_e		alt;
	uint16_t			timeout_ms;
} timeoutTmr;

struct clock_s {
	RTC_t				*rtc;
    uint16_t			timestamp_64s; // combined with counter word, forms a 32 bit millisecond counter.
    int16_t				ms_per_interrupt;
    unsigned char		enabled;
    timer_cb			timerCb;
};

struct clock_s clock;

int8_t setupClks(timer_cb t_cb, int16_t cbIntervalms);

uint32_t getTimestamp_ms();

int8_t setupTimeoutTmr(timeoutTmr *tmr, uint16_t time_ms);

int8_t isTimedout(timeoutTmr *tmr);

int8_t isNotTimedout(timeoutTmr *tmr);

#endif /* CLOCKS_H_ */
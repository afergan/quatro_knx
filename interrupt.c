/*
 * interrupt.c
 *
 * Created: 21-5-2016 7:01:53
 *  Author: Paul
 */ 

#include "clocks.h"
#include "interrupt.h"
#include <stdlib.h>

uint8_t interruptEnable(){
		
	PMIC_t *it;
	it = (PMIC_t*) &PMIC;
			
	it->CTRL |= (PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm);
			
	sei();
	
	return 0;
}

uint8_t interruptDisable(){
	
	cli();
	
	return 0;
}
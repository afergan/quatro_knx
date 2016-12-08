/*
 * colorFeedback.h
 *
 * Created: 20/09/16 14:11:58
 *  Author: PvR
 */ 


#ifndef COLORFEEDBACK_H_
#define COLORFEEDBACK_H_

#include "clocks.h"
#include "i2c.h"
#include "applicationInterface.h"
#include "rgb_led.h"

typedef struct {
	int8_t						enable;

	A_InterfaceHandle			AI_Hndl;
	
	rgbLedHandle				rgbLed_Hndl;

} colorFeedback_Obj;

typedef colorFeedback_Obj *colorFeedback_Handle;

colorFeedback_Handle colorFeedback_Init(void *pMemory, uint16_t numBytes);

int8_t colorFeedback_Setup(colorFeedback_Handle handle, A_InterfaceHandle AI_Hndl, rgbLedHandle	rgbLed_Hndl);

void rgbFeedback_Group_Write_cb(void *parentHndl, uint16_t grpObjIdx);

#endif /* COLORFEEDBACK_H_ */
/*
 * action.h
 *
 * Created: 5-7-2016 7:19:09
 *  Author: Paul
 */ 


#ifndef ACTION_H_
#define ACTION_H_

#include "touchPad.h"
#include "applicationInterface.h"

typedef enum {
	ACTION_NO_ACTION,
	ACTION_EDGE,
	ACTION_SWITCH,
	ACTION_DIM,
	ACTION_MOTOR_CONTROL,
	ACTION_SINGLE_PRESS_MOTOR_CONTROL,
	ACTION_SCENE,
	ACTION_ONE_BYTE,
	ACTION_TWO_BYTE,
	ACTION_SHIFT,
	ACTION_RGB_VALUE
} sensor_action_e;

typedef enum {
	PRESS_NO_ACTION,
	PRESS_ACTION_ZERO,
	PRESS_ACTION_ONE,
	PRESS_ACTION_TOGGLE,
	PRESS_ACTION_UP,
	PRESS_ACTION_DOWN,
	PRESS_ACTION_STOP,
	PRESS_ACTION_UP_DOWN_TOGGLE
} press_action_e;

typedef struct sensor_obj_s {
	sensor_action_e		action;
	press_action_e		shortPress;
	press_action_e		doublePress;
	press_action_e		longPress;
	press_action_e		pressDown;
	press_action_e		pressUp;
} sensor_obj;

typedef struct _touchActionObj {
	uint8_t				enable;
	A_InterfaceHandle	AI_Hndl;
	
	// interface object
	
	// load control
	sGeneric10			loadCtrl;
	
	struct {
		uint16_t	cnt;
		sensor_obj	value[TOUCH_COMBINATIONS_TOTAL];
		}				sensor;

} touchActionObj;

typedef struct _touchActionObj *touchActionHandle;

touchActionHandle touchActionInit(void *pMemory, uint16_t numBytes);

int8_t touchActionSetup(touchActionHandle handle, A_InterfaceHandle AI_Hndl);

int8_t sensor_action(touchActionHandle handle, press_action_e pressAction, group_obj *grObj);

int8_t touchAction(touchActionHandle handle, touch_state_e touchSensor, touch_event_e tevent);

int8_t touchActionPropInd(void *parentHandle, interfaceProperty *prop);

#endif /* ACTION_H_ */
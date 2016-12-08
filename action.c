/*
 * action.c
 *
 * Created: 5-7-2016 7:18:56
 *  Author: Paul
 */ 

#include "clocks.h"
#include "interrupt.h"
//#include "touchPad.h"
#include "applicationLayer.h"
#include "groupObjectServer.h"
#include "action.h"
#include "interfaceObjectServer.h"
#include <string.h>


touchActionHandle touchActionInit(void *pMemory, uint16_t numBytes) {
	touchActionHandle touchAction_handle;
	if (numBytes < sizeof(touchActionObj))
		return ((touchActionHandle)NULL);
	touchAction_handle = (touchActionHandle)pMemory;
	return(touchAction_handle);
}

int8_t touchActionSetup(touchActionHandle handle, A_InterfaceHandle AI_Hndl) {

	touchActionObj *grpAction;
	grpAction = (touchActionObj *) handle;
	
	grpAction->AI_Hndl = AI_Hndl;
	
	AI_Hndl->actionCfgObjectHndl = handle;
	AI_Hndl->AI_actionCfgObject_ind = &touchActionPropInd;

	//
	// Set up some working parameters for the group addresses, association table and group object flags
	// to do: store to, and retrieve from eeprom and configure through external software (device programming).
	//
	
	if (TOUCH_TOP < TOUCH_COMBINATIONS_TOTAL)
	{
		// initialize sensor actions
		grpAction->sensor.value[TOUCH_TOP].action = ACTION_SWITCH;
		grpAction->sensor.value[TOUCH_TOP].pressDown = PRESS_NO_ACTION;
		grpAction->sensor.value[TOUCH_TOP].pressUp = PRESS_NO_ACTION;
		grpAction->sensor.value[TOUCH_TOP].shortPress = PRESS_ACTION_TOGGLE;
		grpAction->sensor.value[TOUCH_TOP].longPress = PRESS_NO_ACTION;
		grpAction->sensor.value[TOUCH_TOP].doublePress = PRESS_NO_ACTION;
	}

	
	if (TOUCH_RIGHT < TOUCH_COMBINATIONS_TOTAL)
	{
		grpAction->sensor.value[TOUCH_RIGHT].action = ACTION_DIM;
		grpAction->sensor.value[TOUCH_RIGHT].shortPress = PRESS_ACTION_ONE;
		grpAction->sensor.value[TOUCH_RIGHT].longPress = PRESS_ACTION_UP;
		grpAction->sensor.value[TOUCH_RIGHT].doublePress = PRESS_NO_ACTION;
	}

	if (TOUCH_BOTTOM < TOUCH_COMBINATIONS_TOTAL)
	{
		grpAction->sensor.value[TOUCH_BOTTOM].action = ACTION_SWITCH;
		grpAction->sensor.value[TOUCH_BOTTOM].shortPress = PRESS_ACTION_ONE;
		grpAction->sensor.value[TOUCH_BOTTOM].longPress = PRESS_NO_ACTION;
		grpAction->sensor.value[TOUCH_BOTTOM].doublePress = PRESS_ACTION_ZERO;
	}

	if (TOUCH_LEFT < TOUCH_COMBINATIONS_TOTAL)
	{
		grpAction->sensor.value[TOUCH_LEFT].action = ACTION_DIM;
		grpAction->sensor.value[TOUCH_LEFT].shortPress = PRESS_ACTION_ZERO;
		grpAction->sensor.value[TOUCH_LEFT].longPress = PRESS_ACTION_DOWN;
		grpAction->sensor.value[TOUCH_LEFT].doublePress = PRESS_NO_ACTION;
	}

	if (TOUCH_TOP_BOTTOM < TOUCH_COMBINATIONS_TOTAL)
	{
		grpAction->sensor.value[TOUCH_TOP_BOTTOM].action = ACTION_SWITCH;
		grpAction->sensor.value[TOUCH_TOP_BOTTOM].shortPress = PRESS_ACTION_ZERO;
		grpAction->sensor.value[TOUCH_TOP_BOTTOM].longPress = PRESS_NO_ACTION;
		grpAction->sensor.value[TOUCH_TOP_BOTTOM].doublePress = PRESS_NO_ACTION;
	}
	
	grpAction->loadCtrl.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, ACTION_CFG_OBJECT_IDX, PID_LOAD_STATE_CONTROL, &grpAction->loadCtrl);
	grpAction->sensor.cnt = SWAP_UINT16(TOUCH_COMBINATIONS_TOTAL);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, ACTION_CFG_OBJECT_IDX, PID_TOUCH_ACTION_CFG, &grpAction->sensor);
	
	grpAction->loadCtrl.value[0] = OBJECT_LOAD_STATE_LOADED;
	
	return 0;
}

int8_t sensor_action(touchActionHandle handle, press_action_e pressAction, group_obj *grObj) {
	
	touchActionObj *grpAction;
	grpAction = (touchActionObj *) handle;

	if (pressAction != PRESS_NO_ACTION)  // && grAddr.addr != 0 
	{
		uint8_t value = 0;
		switch (pressAction) {
			case PRESS_ACTION_ONE:
				value = 1;
			break;
			case PRESS_ACTION_ZERO:
				value = 0;
			break;
			case PRESS_ACTION_TOGGLE:
				value = grObj->value ^ 0x01;
			break;
			case PRESS_ACTION_UP:
				value = 0x09;
			break;
			case PRESS_ACTION_DOWN:
				value = 0x01;
			break;
			case PRESS_ACTION_STOP:
				value = grObj->value & 0x08;
			break;
			case PRESS_ACTION_UP_DOWN_TOGGLE:
				value = (grObj->value ^ 0x08) | 0x01;
			break;
			default:
			break;
		}

		// search association table for match grpObj and retrieve group address (can be multiple).
		grObj->value = value;
		grObj->communicationFlags |= COMM_FLAG_WRITE_REQUEST;
		AI_Group_Write_Req(grpAction->AI_Hndl, grObj->grpObjIdx);

	}
	return 0;
}

int8_t touchAction(touchActionHandle handle, touch_state_e touchSensor, touch_event_e tevent) {
	touchActionObj *grpAction;
	grpAction = (touchActionObj *) handle;

	sensor_group_obj *sensorGrpObjs = GrpSrv_getGroupObjBySensor(grpAction->AI_Hndl->GO_Hndl, touchSensor);
	
	sensor_obj *sensor = &grpAction->sensor.value[touchSensor];

	switch(sensor->action) {
		case ACTION_NO_ACTION:
		break;
		case ACTION_EDGE:
		switch (tevent) {
			case TOUCH_EVENT_DOWN:
			sensor_action(handle, sensor->pressDown, &sensorGrpObjs->switchShortPress);
			break;
			case TOUCH_EVENT_UP:
			sensor_action(handle, sensor->pressUp, &sensorGrpObjs->switchShortPress);
			break;
			default:
			break;
		}
		break;
		case ACTION_SWITCH:
		switch (tevent) {
			case TOUCH_SHORT_PRESS_EVENT:
			sensor_action(handle, sensor->shortPress, &sensorGrpObjs->switchShortPress);
			break;
			case TOUCH_DOUBLE_PRESS_EVENT:
			sensor_action(handle, sensor->doublePress, &sensorGrpObjs->switchDoublePress);
			break;
			case TOUCH_LONG_PRESS_DOWN_EVENT:
			sensor_action(handle, sensor->longPress, &sensorGrpObjs->switchLongPress);
			break;
			default:
			break;
		}
		break;
		case ACTION_DIM:
		switch (tevent) {
			case TOUCH_SHORT_PRESS_EVENT:
			sensor_action(handle, sensor->shortPress, &sensorGrpObjs->dimSwitch);
			break;
			case TOUCH_LONG_PRESS_DOWN_EVENT:
			sensor_action(handle, sensor->longPress, &sensorGrpObjs->dimCommand);
			break;
			case TOUCH_LONG_PRESS_UP_EVENT:
			sensor_action(handle, PRESS_ACTION_STOP, &sensorGrpObjs->dimCommand);
			break;
			default:
			break;
		}
		break;
		case ACTION_MOTOR_CONTROL:
		break;
		case ACTION_SINGLE_PRESS_MOTOR_CONTROL:
		break;
		case ACTION_SCENE:
		break;
		case ACTION_ONE_BYTE:
		break;
		case ACTION_TWO_BYTE:
		break;
		case ACTION_SHIFT:
		break;
		case ACTION_RGB_VALUE:
		break;
	}
	return 0;
}

int8_t touchActionPropInd(void *parentHandle, interfaceProperty *prop) {
	touchActionObj *grpAction;
	grpAction = (touchActionObj *) parentHandle;
	
	switch((application_property_id_e)prop->propertyId) {
		case PID_LOAD_STATE_CONTROL:
		
		break;
		case PID_TOUCH_ACTION_CFG:
		{

		}
		break;
		default:
		return -1;
	}
	return 1;
}

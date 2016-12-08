/*
 * roomTempAndHumidity.c
 *
 * Created: 19/09/16 09:32:39
 *  Author: PvR
 */ 


#include "roomTempAndHumidity.h"
#include <string.h>
#include <stdio.h>

tempHum_Handle tempHum_Init(void *pMemory, uint16_t numBytes) {
	tempHum_Handle tempHum_handle;
	if (numBytes < sizeof(tempHum_Obj))
		return ((tempHum_Handle)NULL);
	tempHum_handle = (tempHum_Handle)pMemory;
	return tempHum_handle;	
}

int8_t tempHum_Setup(tempHum_Handle handle, i2cHandle i2c_handle, A_InterfaceHandle	AI_Hndl, uint16_t measurementIntervalSeconds) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)handle;
	
	th->toString_cb_func = NULL;
	
	th->i2c_handle = i2c_handle;
	th->AI_Hndl = AI_Hndl;
	
	AI_Hndl->ambientSensorCfgObjectHndl = handle;
	AI_Hndl->AI_ambientSensorCfgObject_ind = &tempHumPropInd;
	
	th->measurementIntervalSeconds.value = SWAP_UINT16(measurementIntervalSeconds);
	th->timeRemaining = 1;
	
	th->hih_handle = HIH_Init(&th->hih_obj, sizeof(th->hih_obj));
	int8_t hih_result =  HIH_Setup(th->hih_handle, i2c_handle, &PORTE, 2);

	th->mcp9808_handle = MCP9808_Init(&th->mcp9808_obj, sizeof(th->mcp9808_obj));
	int8_t mcp_result = MCP9808_Setup(th->mcp9808_handle, i2c_handle);
	
	th->loadCtrl.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, AMBIENT_SENSOR_CFG_OBJECT_IDX, PID_LOAD_STATE_CONTROL, &th->loadCtrl);
	th->sensorConfig.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, AMBIENT_SENSOR_CFG_OBJECT_IDX, PID_AMBIENT_SENSORS_CFG, &th->sensorConfig);
	th->measurementIntervalSeconds.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, AMBIENT_SENSOR_CFG_OBJECT_IDX, PID_AMBIENT_SENSORS_INTERVAL, &th->measurementIntervalSeconds);
	th->temperatureOffset.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, AMBIENT_SENSOR_CFG_OBJECT_IDX, PID_AMBIENT_TEMPERATURE_OFFSET, &th->temperatureOffset);
	th->humidityOffset.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, AMBIENT_SENSOR_CFG_OBJECT_IDX, PID_AMBIENT_HUMIDITY_OFFSET, &th->humidityOffset);
	th->temperatures.cnt = SWAP_UINT16( ROOM_TEMP_SENSOR_CNT);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, AMBIENT_SENSOR_CFG_OBJECT_IDX, PID_AMBIENT_TEMPERATURE, &th->temperatures);
	th->humidity.cnt = SWAP_UINT16( ROOM_HUMIDITY_SENSOR_CNT);
	IntfObj_Srv_registerObject(AI_Hndl->IO_Hndl, AMBIENT_SENSOR_CFG_OBJECT_IDX, PID_AMBIENT_HUMIDITY, &th->humidity);

	resetIntfObj(handle);

	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *)(AI_Hndl->GO_Hndl);
	
	GrpObj_register_Read_ind_cb(&(grpSrv->obj.ambient.temperature), &tempHumidityStartMeasurement_Group_Read, handle);
	GrpObj_register_Read_ind_cb(&(grpSrv->obj.ambient.humidity), &tempHumidityStartMeasurement_Group_Read, handle);
	
	th->loadCtrl.value[0] = 1;
	
	return (hih_result | mcp_result);
}

int8_t tempHumPropInd(void *parentHandle, interfaceProperty *prop) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)parentHandle;	
	
	switch((application_property_id_e)prop->propertyId) {
		case PID_LOAD_STATE_CONTROL:
		
		break;
		case PID_AMBIENT_SENSORS_CFG:
		{
			uint16_t cfg = *(uint16_t*)prop->elements;
		}
		break;
		case PID_AMBIENT_SENSORS_INTERVAL:
		{
			volatile uint16_t interval = SWAP_UINT16(*(uint16_t*)prop->elements);
			if (interval < 10 || interval > 3600)
				return -1;
		}
		break;
		case PID_AMBIENT_TEMPERATURE_OFFSET:
		
		break;
		case PID_AMBIENT_HUMIDITY_OFFSET:
		
		break;
		default:
		return -1;
	}
	return 1;
}

void tempHum_Service(tempHum_Handle handle) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)handle;	

	HIH_Service(th->hih_handle);
	MCP9808_Service(th->mcp9808_handle);
	
	if (th->measurementFlags != AMBIENT_MEAS_IDLE)
		if (isTimedout(&th->measurementTimeout))
			tempHumidityCompleted(handle);
}

void tempHum_everySecond(tempHum_Handle handle) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)handle;
	
	if (th->measurementIntervalSeconds.value != 0)
	{
		if (!--th->timeRemaining)
		{
			th->timeRemaining = SWAP_UINT16(th->measurementIntervalSeconds.value);
			tempHumidityStartMeasurement(handle, &tempHumidityGroupWrite);
		}
	}
}

int8_t tempHum_setMeasurementInterval(tempHum_Handle handle, uint16_t measurementIntervalSeconds) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)handle;
	
	th->measurementIntervalSeconds.value = SWAP_UINT16(measurementIntervalSeconds);
	return 0;	
}

void tempHumidityStartMeasurement(tempHum_Handle handle, void *completed_cb) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)handle;
	
	if (completed_cb == NULL || th->measurementFlags)
		return;
	th->completed_cb_func = completed_cb;
	setupTimeoutTmr(&th->measurementTimeout, AMBIENT_TIMEOUT);

	th->measurementFlags = AMBIENT_MEAS_STARTED;
	resetIntfObj(handle);
	HIH_StartMeas(th->hih_handle, &hih_finished_cb, handle);
	MCP9808_StartMeas(th->mcp9808_handle, &mcp_finished_cb, handle);
}

void hih_finished_cb(void *parentHandle, HIH_measurement_result *result) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)parentHandle;
	th->measurementFlags |= AMBIENT_MEAS_HIH_COMPLETED;
	
	hihUpdateIntfObj(parentHandle, result);
	
	if (th->measurementFlags == AMBIENT_MEAS_COMPLETED)
		 tempHumidityCompleted((tempHum_Handle)parentHandle);
}

void mcp_finished_cb(void *parentHandle, mcp9808_measurement_result *result) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)parentHandle;
	th->measurementFlags |= AMBIENT_MEAS_MCP_COMPLETED;
	
	mcpUpdateIntfObj(parentHandle, result);
	
	if (th->measurementFlags == AMBIENT_MEAS_COMPLETED)
		tempHumidityCompleted((tempHum_Handle)parentHandle);
	
}

void tempHumidityCompleted(tempHum_Handle handle) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)handle;	
	
	calculateAverage((uint16_t*)&(th->temperatures));
	// Add offset
	int32_t avg = knxFloatToUint32(SWAP_UINT16(*((uint16_t*)&(th->temperatures) + 1)));
	avg += knxFloatToUint32(SWAP_UINT16(*((uint16_t*)&(th->temperatureOffset) + 1)));
	*((uint16_t*)&(th->temperatures) + 1) = SWAP_UINT16(uint32ToKnxFloat(avg));
		
	calculateAverage((uint16_t*)&(th->humidity));
	// Add offset
	avg = knxFloatToUint32(SWAP_UINT16(*((uint16_t*)&(th->humidity) + 1)));
	avg += knxFloatToUint32(SWAP_UINT16(*((uint16_t*)&(th->humidityOffset) + 1)));
	*((uint16_t*)&(th->humidity) + 1) = SWAP_UINT16(uint32ToKnxFloat(avg));
			
	if (th->completed_cb_func)
		th->completed_cb_func(handle);
}


void tempHumidityStartMeasurement_Group_Read(void *parentHandle, uint16_t *grpObjIdx) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)parentHandle;

	tempHumidityStartMeasurement(parentHandle, &tempHumidityGroupReadResponse);
}


void tempHumidityGroupReadResponse(void *parentHandle) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)parentHandle;


	A_InterfaceObj *AI_Handle;
	AI_Handle = (A_InterfaceObj *)((tempHum_Obj *)parentHandle)->AI_Hndl;

	AI_Handle->GOsrv_obj.obj.ambient.temperature.value = SWAP_UINT16(th->temperatures.value[0]);
	AI_Group_Read_Res(AI_Handle, AI_Handle->GOsrv_obj.obj.ambient.temperature.grpObjIdx);
	
	AI_Handle->GOsrv_obj.obj.ambient.humidity.value = SWAP_UINT16(th->humidity.value[0]);
	AI_Group_Read_Res(AI_Handle, AI_Handle->GOsrv_obj.obj.ambient.humidity.grpObjIdx);
	
	th->measurementFlags = AMBIENT_MEAS_IDLE;
}


void tempHumidityStartMeasurementToString(tempHum_Handle handle, void *cb_func) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)handle;
	th->toString_cb_func = cb_func;
	tempHumidityStartMeasurement(handle, &tempHumidityToStringResponse);
}

void tempHumidityToStringResponse(tempHum_Handle handle) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)handle;


	uint8_t sz = 255;
	char str[sz];
	uint8_t strPtr = 0;

	strPtr += snprintf(str, sz, "temp : ");
	for (uint8_t i = 0; i < ROOM_TEMP_SENSOR_CNT; i++)
	{
		strPtr += knxFloatToString(SWAP_UINT16(th->temperatures.value[i]), (str + strPtr), (sz - strPtr));
		if ((sz - strPtr) == 0 || (sz - strPtr) > sz)
			break;
		if (i < (ROOM_TEMP_SENSOR_CNT - 1))
			strPtr += snprintf((str + strPtr), (sz - strPtr), " : ");
	}
	strPtr += snprintf((str + strPtr), (sz - strPtr), "\n\rhum  : ");
	for (uint8_t i = 0; i < ROOM_HUMIDITY_SENSOR_CNT; i++)
	{
		strPtr += knxFloatToString(SWAP_UINT16(th->humidity.value[i]), (str + strPtr), (sz - strPtr));
		if ((sz - strPtr) == 0 || (sz - strPtr) > sz)
			break;
		if (i < (ROOM_HUMIDITY_SENSOR_CNT - 1))
			strPtr += snprintf((str + strPtr), (sz - strPtr), " : ");
	}	

	if (th->toString_cb_func)
		th->toString_cb_func(str, strlen(str));
	th->toString_cb_func = NULL;

	th->measurementFlags = AMBIENT_MEAS_IDLE;
}


void tempHumidityGroupWrite(void *parentHandle) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)parentHandle;

	
	A_InterfaceObj *AI_Handle;
	AI_Handle = (A_InterfaceObj *)((tempHum_Obj *)parentHandle)->AI_Hndl;
	
	AI_Handle->GOsrv_obj.obj.ambient.temperature.value = SWAP_UINT16(th->temperatures.value[0]);
	AI_Handle->GOsrv_obj.obj.ambient.temperature.communicationFlags |= COMM_FLAG_WRITE_REQUEST;
	AI_Group_Write_Req(AI_Handle, AI_Handle->GOsrv_obj.obj.ambient.temperature.grpObjIdx);
	
	AI_Handle->GOsrv_obj.obj.ambient.humidity.value = SWAP_UINT16(th->humidity.value[0]);
	AI_Handle->GOsrv_obj.obj.ambient.humidity.communicationFlags |= COMM_FLAG_WRITE_REQUEST;
	AI_Group_Write_Req(AI_Handle, AI_Handle->GOsrv_obj.obj.ambient.humidity.grpObjIdx);
		
	th->measurementFlags = AMBIENT_MEAS_IDLE;
}

void hihUpdateIntfObj(void *parentHandle, HIH_measurement_result *result) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)parentHandle;
	for(uint8_t i = 0; i < 4; i++)
	{
		th->temperatures.value[i + 5] = SWAP_UINT16(result->temperature.knxFloat);
		th->humidity.value[i + 1]= SWAP_UINT16(result->humidity.knxFloat);
	}
}

void mcpUpdateIntfObj(void *parentHandle, mcp9808_measurement_result *result) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)parentHandle;
	for(uint8_t i = 0; i < 4; i++)
		th->temperatures.value[i + 1] = SWAP_UINT16(result[i].KNXFloatTemperature);
}

void resetIntfObj(void *parentHandle) {
	tempHum_Obj *th;
	th = (tempHum_Obj *)parentHandle;
	for(uint8_t i = 0; i < ROOM_TEMP_SENSOR_CNT; i++)
	th->temperatures.value[i] = SWAP_UINT16(0x7fff);
	for(uint8_t i = 0; i < ROOM_HUMIDITY_SENSOR_CNT; i++)
	th->humidity.value[i] = SWAP_UINT16(0x7fff);
}

uint16_t calculateAverage(uint16_t *measurement) {
	uint16_t measCnt = SWAP_UINT16(*(uint16_t*)measurement);
	int32_t sum = 0;
	uint16_t result = 0x7fff;
	int8_t denominator = 0;
	for(uint8_t i = 2; i <= measCnt; i++)
	{
		uint16_t value = SWAP_UINT16(*((uint16_t*)measurement + i));
		if (value != 0x7fff)
		{
			sum += knxFloatToUint32(value);
			denominator++;
		}
	}
	if (denominator > 0)
	{
		sum /= denominator;
		result = uint32ToKnxFloat(sum);
	}
	
	*(((uint16_t*)measurement + 1)) = SWAP_UINT16(result);
	
	return result;
}

uint8_t knxFloatToString(uint16_t knxFloat, char *str, uint8_t size) {
	if (knxFloat == 0x7fff)
		return snprintf(str, size, "?");
	uint32_t value = knxFloatToUint32(knxFloat);
	int len = snprintf(str, size - 1, "%ld", value);
	str[len] = str[len - 1];
	str[len - 1] = str[len - 2];
	str[len - 2] = '.';
	str[len + 1] = 0x00;	
	return strlen(str);
}

int32_t knxFloatToUint32(uint16_t knxFloat) {
	int32_t value = knxFloat;
	if (value != 0x7fff)
	{
		uint8_t exponent = (value & 0x7800) >> 11;
		if ((value & 0x8000) == 0)
			value &= 0x7ff;
		else
			value |= 0xfffff800;
		value <<= exponent;
	}
	return value;
}

uint16_t uint32ToKnxFloat(int32_t deciValue) {
	uint8_t exponent = 0;
	while (deciValue > (0x03ff)) {
		deciValue = deciValue >> 1;
		exponent++;
	}
	while (deciValue < (-0x03ff)) {
		deciValue = (deciValue >> 1) | 0x80000000;
		exponent++;
	}
	return ((uint16_t)exponent << 11) | (deciValue & 0x87ff);
}
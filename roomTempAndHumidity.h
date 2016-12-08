/*
 * roomTempAndHumidity.h
 *
 * Created: 19/09/16 09:33:31
 *  Author: PvR
 */ 


#ifndef ROOMTEMPANDHUMIDITY_H_
#define ROOMTEMPANDHUMIDITY_H_

#include "clocks.h"
#include "i2c.h"
#include "applicationInterface.h"
#include "interfaceObject.h"
#include "Humidicon_sensor.h"
#include "MCP9808_temp_sensor.h"

#define ROOM_TEMP_SENSOR_CNT		9
#define ROOM_HUMIDITY_SENSOR_CNT	5

#define AMBIENT_MEAS_IDLE			0
#define AMBIENT_MEAS_STARTED		0X1
#define AMBIENT_MEAS_HIH_COMPLETED	0X2
#define AMBIENT_MEAS_MCP_COMPLETED	0X4
#define AMBIENT_MEAS_COMPLETED		0x7

#define AMBIENT_TIMEOUT				1000

typedef struct {
	int8_t						enable;
	i2cHandle					i2c_handle;
	
	A_InterfaceHandle			AI_Hndl;
	
	HIH_Obj						hih_obj;
	HIH_Handle					hih_handle;

	MCP9808_Obj					mcp9808_obj;
	MCPC9808_Handle				mcp9808_handle;
	uint8_t						mcpStatus[4];
	
	uint8_t						measurementFlags;
	
	uint16_t					timeRemaining;

	timeoutTmr					measurementTimeout;
	
	void						(*completed_cb_func)(void *handle);
	
	void						(*toString_cb_func)(char *str, uint8_t length);
	
	// interface objects
	
	//ambient load control
	sGeneric10					loadCtrl;
	
	// ambient cfg
	sWord						sensorConfig;

	// ambient sensor interval
	sWord						measurementIntervalSeconds;
	
	// ambient temperature	
	struct						 {
		uint16_t cnt;
		uint16_t value[ROOM_TEMP_SENSOR_CNT];
	}							temperatures;
	struct 
	{
		uint16_t cnt;
		uint16_t value;
	}							temperatureOffset;

	// ambient humidity
	struct						 {
		uint16_t cnt;
		uint16_t value[ROOM_HUMIDITY_SENSOR_CNT];
	}							humidity;
	struct
	{
		uint16_t cnt;
		uint16_t value;
	}							humidityOffset;
} tempHum_Obj;

typedef tempHum_Obj *tempHum_Handle;

tempHum_Handle tempHum_Init(void *pMemory, uint16_t numBytes);

int8_t tempHum_Setup(tempHum_Handle handle, i2cHandle i2c_handle, A_InterfaceHandle AI_Hndl, uint16_t measurementIntervalSeconds);

int8_t tempHumPropInd(void *parentHandle, interfaceProperty *prop);

void tempHum_Service(tempHum_Handle handle);

void tempHum_everySecond(tempHum_Handle handle);

int8_t tempHum_setMeasurementInterval(tempHum_Handle handle, uint16_t measurementIntervalSeconds);

void tempHumidityStartMeasurement(tempHum_Handle handle, void *completed_cb);

void hih_finished_cb(void *parentHandle, HIH_measurement_result *result);

void mcp_finished_cb(void *parentHandle, mcp9808_measurement_result *result);

void tempHumidityCompleted(tempHum_Handle handle);

void tempHumidityStartMeasurement_Group_Read(void *parentHandle, uint16_t *grpObjIdx);

void tempHumidityGroupReadResponse(void *parentHandle);

void tempHumidityStartMeasurementToString(tempHum_Handle handle, void *cb_func);

void tempHumidityToStringResponse(tempHum_Handle handle);

void hihUpdateIntfObj(void *parentHandle, HIH_measurement_result *result);

void mcpUpdateIntfObj(void *parentHandle, mcp9808_measurement_result *result);

void tempHumidityGroupWrite(void *parentHandle);

void resetIntfObj(void *parentHandle);

uint16_t calculateAverage(uint16_t *measurement);

uint8_t knxFloatToString(uint16_t knxFloat, char *str, uint8_t size);

int32_t knxFloatToUint32(uint16_t knxFloat);

uint16_t uint32ToKnxFloat(int32_t deciValue);

#endif /* ROOMTEMPANDHUMIDITY_H_ */
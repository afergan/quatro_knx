/*
 * MCP9808_temp_sensor.h
 *
 * Created: 9-9-2016 7:17:17
 *  Author: Paul
 */ 

#ifndef MCP9808_TEMP_SENSOR_H_
#define MCP9808_TEMP_SENSOR_H_

#include "clocks.h"
#include "i2c.h"

#define MCP9808_I2C_ADDRESS_1			0x18
#define MCP9808_I2C_ADDRESS_2			0x19
#define MCP9808_I2C_ADDRESS_3			0x1A
#define MCP9808_I2C_ADDRESS_4			0x1B

#define MCP9808_SENSOR_COUNT			4

#define MCP9808_MEASUREMENT_TIME_MS		280  // in 0.0625 degrees precision, conversion time 250 ms
#define MCP9808_READ_TIMEOUT_MS			10
#define MCP9808_TIMEOUT_MS				10

typedef enum {
	MCP9808_STARTUP,
	MCP9808_IDLE,
	MCP9808_START_MEASUREMENT,
	MCP9808_MEASURING,
	MCP9808_READING,
	MCP9808_FINISH_READING,
	MCP9808_DATA_READY,
	MCP9808_SHUTDOWN,
	MCP9808_COMPLETED,
	MCP9808_ERROR
} mcp9808_state_e;

typedef enum {
	MCP9808_MEAS_VOID,
	MCP9808_MEAS_OK,
	MCP9808_MEAS_STALE,
	MCP9808_MEAS_ERROR
} mcp9808_meas_e;

typedef struct {
	uint16_t					raw;
	uint16_t					KNXFloatTemperature;
	mcp9808_meas_e				status;
} mcp9808_measurement_result;

typedef struct {
	int8_t						enable;
	i2cHandle					i2c_handle;
	volatile uint8_t			sensorSelect;
	uint8_t						sensorStatus;
	mcp9808_measurement_result	result[MCP9808_SENSOR_COUNT];
	volatile mcp9808_state_e	state;
	timeoutTmr					measurementTimeout;
	void						*parentHandle;
	void						(*measurement_finished_cb)(void *parentHandle, mcp9808_measurement_result *result);
} MCP9808_Obj;

typedef MCP9808_Obj *MCPC9808_Handle;

MCPC9808_Handle MCP9808_Init(void *pMemory, uint16_t numBytes);

int8_t MCP9808_Setup(MCPC9808_Handle handle, i2cHandle i2c_handle);

int8_t MCP9808_SensorSelect(MCPC9808_Handle handle, uint8_t sensor);

int8_t MCP9808_StartMeas(MCPC9808_Handle handle, void *measurement_finished_cb, void *parentHandle);

void MCP9808_i2cDataReady(void *parentHandle, i2c_status_e status);

void MCP9808_i2cWriteAwakeFinished(void *parentHandle, i2c_status_e status);

void MCP9808_i2cWriteSleepFinished(void *parentHandle, i2c_status_e status);

void MCP9808_Service(MCPC9808_Handle handle);

#endif /* MCP9808_TEMP_SENSOR_H_ */
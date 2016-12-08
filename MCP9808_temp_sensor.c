/*
 * MCP9808_temp_sensor.c
 *
 * Created: 9-9-2016 7:16:57
 *  Author: Paul
 */ 

#include "MCP9808_temp_sensor.h"
#include "object_resources.h"

uint8_t MCP9808SendToSleep[] = {0x01, 0x01, 0x00};
uint8_t MCP9808AwakeSensor[] = {0x01, 0x00, 0x00};

MCPC9808_Handle MCP9808_Init(void *pMemory, uint16_t numBytes) {
	MCPC9808_Handle MCP_handle;
	if (numBytes < sizeof(MCP9808_Obj))
		return ((MCPC9808_Handle)NULL);
	MCP_handle = (MCPC9808_Handle)pMemory;
		return MCP_handle;
}

int8_t MCP9808_Setup(MCPC9808_Handle handle, i2cHandle i2c_handle) {
	MCP9808_Obj *mcp;
	mcp = (MCP9808_Obj *)handle;
	
	mcp->i2c_handle = i2c_handle;
	mcp->measurement_finished_cb = NULL;
	mcp->state = MCP9808_IDLE;
	mcp->enable = 1;

	mcp->sensorStatus = 0;
	
	uint8_t config[] = {0x08, 0x03}; // set sensor to 0.0625 degrees resolution.
	
	for (uint8_t i = 0; i < MCP9808_SENSOR_COUNT; i++)
	{
		if (i2cWrite(mcp->i2c_handle, MCP9808_I2C_ADDRESS_1 + i, NULL, 0) >= 0)
		{
			mcp->sensorStatus |= (1 << i);
			mcp->result->status = MCP9808_MEAS_VOID;
			i2cWrite(mcp->i2c_handle, MCP9808_I2C_ADDRESS_1 + i, config, 2);
			i2cWrite(mcp->i2c_handle, MCP9808_I2C_ADDRESS_1 + i, MCP9808SendToSleep, 3);
		}
	}
	
	if (!mcp->sensorStatus)
		return -1;
	else
		return 0;
}

int8_t MCP9808_SensorSelect(MCPC9808_Handle handle, uint8_t sensor);

int8_t MCP9808_StartMeas(MCPC9808_Handle handle, void *measurement_finished_cb, void *parentHandle) {
	MCP9808_Obj *mcp;
	mcp = (MCP9808_Obj *)handle;
	
	if (mcp->enable != 1 || mcp->state != MCP9808_IDLE)
		return -1;
	
	mcp->parentHandle = parentHandle;
	mcp->measurement_finished_cb = measurement_finished_cb;
	mcp->sensorSelect = 0;
	setupTimeoutTmr(&mcp->measurementTimeout, MCP9808_TIMEOUT_MS);
	mcp->state = MCP9808_START_MEASUREMENT;
	return 0;
}

void MCP9808_i2cDataReady(void *parentHandle, i2c_status_e status) {
	MCP9808_Obj *mcp;
	mcp = (MCP9808_Obj *)parentHandle;	

	if (status == I2C_STATUS_ERROR)
		mcp->result[mcp->sensorSelect & 0x7f].status = MCP9808_MEAS_ERROR;
	else
		mcp->result[mcp->sensorSelect & 0x7f].status = MCP9808_MEAS_OK;
	
	mcp->sensorSelect++;
	mcp->sensorSelect &= 0x7f;
	if (mcp->sensorSelect == MCP9808_SENSOR_COUNT)
		mcp->state = MCP9808_FINISH_READING;
}

void MCP9808_i2cWriteAwakeFinished(void *parentHandle, i2c_status_e status) {
	MCP9808_Obj *mcp;
	mcp = (MCP9808_Obj *)parentHandle;

	mcp->sensorSelect++;
	mcp->sensorSelect &= 0x7f;
	
	if (mcp->sensorSelect == MCP9808_SENSOR_COUNT)
	{
		mcp->sensorSelect = 0;
		mcp->state = MCP9808_MEASURING;
		setupTimeoutTmr(&mcp->measurementTimeout, MCP9808_MEASUREMENT_TIME_MS);
	}		
}

void MCP9808_i2cWriteSleepFinished(void *parentHandle, i2c_status_e status) {
	MCP9808_Obj *mcp;
	mcp = (MCP9808_Obj *)parentHandle;

	mcp->sensorSelect++;
	mcp->sensorSelect &= 0x7f;

	if (mcp->sensorSelect == MCP9808_SENSOR_COUNT)
	{
		mcp->sensorSelect = 0;
		mcp->state = MCP9808_COMPLETED;
	}
}

void MCP9808_Service(MCPC9808_Handle handle) {
	MCP9808_Obj *mcp;
	mcp = (MCP9808_Obj *)handle;
	
	if (mcp->enable != 1)
		return;

	if (mcp->state == MCP9808_START_MEASUREMENT)
	{
		if ((mcp->sensorSelect & 0x80) == 0)
		{
			i2cWriteNonBlocking(mcp->i2c_handle, MCP9808_I2C_ADDRESS_1 + mcp->sensorSelect, MCP9808AwakeSensor, 3, MCP9808_i2cWriteAwakeFinished, handle);
			mcp->sensorSelect |= 0x80;
		}
		
		if (isTimedout(&mcp->measurementTimeout))
		{
			mcp->sensorSelect = 0;
			mcp->state = MCP9808_MEASURING;
			setupTimeoutTmr(&mcp->measurementTimeout, MCP9808_MEASUREMENT_TIME_MS);
		}
	}
	else if (mcp->state == MCP9808_MEASURING)
	{
		if (isTimedout(&mcp->measurementTimeout))
		{
			mcp->state = MCP9808_READING;
			setupTimeoutTmr(&mcp->measurementTimeout, MCP9808_READ_TIMEOUT_MS);
		}
	}
	else if (mcp->state == MCP9808_READING)
	{
		if ((mcp->sensorSelect & 0x80) == 0) 
		{
			uint8_t addressByte = 0x05; // write to this register address starts measurement.
			int8_t result = i2cWrite(mcp->i2c_handle, MCP9808_I2C_ADDRESS_1 + mcp->sensorSelect, &addressByte, 1);
			if (result >= 0)
			{
				i2cReadNonBlocking(mcp->i2c_handle, MCP9808_I2C_ADDRESS_1 + mcp->sensorSelect, (uint8_t*)(&mcp->result[mcp->sensorSelect].raw), 2, MCP9808_i2cDataReady, handle);
				mcp->sensorSelect |=0x80;
			}
			
		}
		if (isTimedout(&mcp->measurementTimeout)) // nack or timeout
		{
			mcp->result[mcp->sensorSelect & 0x7f].raw = 0xffff;
			mcp->result[mcp->sensorSelect & 0x7f].status = MCP9808_MEAS_ERROR;
			mcp->sensorSelect = 0;
		}
		
		if (mcp->sensorSelect == MCP9808_SENSOR_COUNT)
			mcp->state = MCP9808_FINISH_READING;
	}
	else if (mcp->state == MCP9808_FINISH_READING)
	{
		// do float conversion
		for (uint8_t i=0; i < MCP9808_SENSOR_COUNT; i++)
		{
			if (mcp->result[i].status == MCP9808_MEAS_OK)
			{
				uint8_t exponent = 0;
				int32_t tmp = SWAP_UINT16(mcp->result[i].raw) & 0x0fff;
				if ((SWAP_UINT16(mcp->result[i].raw) & 0x1000) != 0)
					tmp *= -1;
				tmp *= 25; // knx resolution is 0.01. Sensor resolution is 0.0625 degrees
				tmp = tmp >> 2;
				while (tmp > (0x03ff)) {
					tmp = tmp >> 1;
					exponent++;
				}
				while (tmp < (-0x03ff)) {
					tmp = (tmp >> 1) | 0x80000000;
					exponent++;
				}
				mcp->result[i].KNXFloatTemperature =((uint16_t)exponent << 11) | (tmp & 0x87ff);
			}
			else
			{
				mcp->result[i].KNXFloatTemperature = 0x7fff; // error condition
			}
		}
		mcp->state = MCP9808_DATA_READY;
	}
	else if (mcp->state == MCP9808_DATA_READY)
	{
		mcp->sensorSelect = 0;
		setupTimeoutTmr(&mcp->measurementTimeout, MCP9808_TIMEOUT_MS);
		mcp->state = MCP9808_SHUTDOWN;
	}
	else if (mcp->state == MCP9808_SHUTDOWN)
	{
		if ((mcp->sensorSelect & 0x80) == 0)
		{
			i2cWriteNonBlocking(mcp->i2c_handle, MCP9808_I2C_ADDRESS_1 + mcp->sensorSelect, MCP9808SendToSleep, 3, MCP9808_i2cWriteSleepFinished, handle);
			mcp->sensorSelect |= 0x80;
		}
		if (isTimedout(&mcp->measurementTimeout))
		{
			mcp->sensorSelect = 0;
			mcp->state = MCP9808_ERROR;
		}

	}
	else if (mcp->state == MCP9808_COMPLETED) 
	{
		if (mcp->measurement_finished_cb)
		{
			mcp->measurement_finished_cb(mcp->parentHandle, mcp->result);
			mcp->measurement_finished_cb = NULL;
		}
		mcp->state = MCP9808_IDLE;	
	}
	else if (mcp->state == MCP9808_ERROR)
	{
		for(uint8_t i = 0; i < MCP9808_SENSOR_COUNT; i++)
			mcp->result[i].KNXFloatTemperature = 0x7fff;
		if (mcp->measurement_finished_cb)
		{
			mcp->measurement_finished_cb(mcp->parentHandle, mcp->result);
			mcp->measurement_finished_cb = NULL;
		}
		mcp->state = MCP9808_IDLE;
	}
}
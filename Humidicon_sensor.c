/*
 * tempHum.c
 *
 * Created: 9-7-2016 7:43:56
 *  Author: Paul
 */ 


#include "Humidicon_sensor.h"

HIH_Handle HIH_Init(void *pMemory, uint16_t numBytes) {
	HIH_Handle tempHum_handle;
	if (numBytes < sizeof(HIH_Obj))
		return ((HIH_Handle)NULL);
	tempHum_handle = (HIH_Handle)pMemory;
	return tempHum_handle;
}

int8_t HIH_Setup(HIH_Handle handle, i2cHandle i2c_handle, PORT_t *pwrPort, uint8_t pwrBit) {
	HIH_Obj *tempHum;
	tempHum = (HIH_Obj *)handle;

	tempHum->enable = 1;
	tempHum->i2c_handle = i2c_handle;
	tempHum->state = TH_IDLE;
	tempHum->measurement_finished_cb = NULL;
	tempHum->pwrPort = pwrPort;
	tempHum->pwrBit = pwrBit;
	
	tempHum->pwrPort->DIRSET = _BV(tempHum->pwrBit);
	
	tempHum->pwrPort->OUTSET = _BV(tempHum->pwrBit);
	
	return i2cWrite(tempHum->i2c_handle, HIH_I2C_ADDRESS, NULL, 0);
}

int8_t HIH_Reset(HIH_Handle handle) {
	HIH_Obj *tempHum;
	tempHum = (HIH_Obj *)handle;
	if (!tempHum->enable)
	return -1;

	tempHum->pwrPort->OUT &= ~(_BV(tempHum->pwrBit)); // switch off HIH
	
	tempHum->i2c_handle->twi->MASTER.CTRLA &= ~(_BV(TWI_MASTER_ENABLE_bp));
	PORTE.OUTCLR = 0x03;
	PORTE.DIRSET = 0x03;
	
	timeoutTmr pwrOffDelay;
	setupTimeoutTmr(&pwrOffDelay, 200);
	while(isNotTimedout(&pwrOffDelay));
	
	PORTE.DIRCLR = 0x03;	

	tempHum->pwrPort->OUT |= _BV(tempHum->pwrBit); // switch on HIH

	i2cReset(tempHum->i2c_handle);
	
	//tempHum->measurementTimeout = clock.timestamp + HIH_STARTUP_TIME_MS;
	setupTimeoutTmr(&tempHum->measurementTimeout, HIH_STARTUP_TIME_MS);
	tempHum->state = TH_STARTUP;		

	return 0;
}

int8_t HIH_StartMeas(HIH_Handle handle, void *measurement_finished_cb, void *parentHandle) {
	HIH_Obj *tempHum;
	tempHum = (HIH_Obj *)handle;
	if (!tempHum->enable || (tempHum->state != TH_IDLE))
		return -1;

	if (i2cWrite(tempHum->i2c_handle, HIH_I2C_ADDRESS, NULL, 0) >= 0)
	{
		tempHum->parentHandle = parentHandle;
		tempHum->measurement_finished_cb = measurement_finished_cb;
		tempHum->state = TH_MEASURING;
		setupTimeoutTmr(&tempHum->measurementTimeout, HIH_MEASUREMENT_TIME_MS);
		return 0;
	}
	else
		return -2;
}


void HIH_Service(HIH_Handle handle) {
	HIH_Obj *tempHum;
	tempHum = (HIH_Obj *)handle;
	if (!tempHum->enable)
		return;
	switch(tempHum->state) {
		case TH_STARTUP:
			if (isTimedout(&tempHum->measurementTimeout))
			{
				tempHum->state = TH_IDLE;
			}
		break;
		case TH_IDLE:
			return;
		break;
		case TH_MEASURING:
			if (isTimedout(&tempHum->measurementTimeout))
			{
				if (i2cReadNonBlocking(tempHum->i2c_handle, HIH_I2C_ADDRESS, tempHum->raw, 4, &HIH_data_ready_callback, handle) >= 0)
				{
					tempHum->state = TH_READING;
					setupTimeoutTmr(&tempHum->measurementTimeout, HIH_READ_TIMEOUT_MS);
				}
			}
		break;
		case TH_READING:
			if (isTimedout(&tempHum->measurementTimeout)) // error condition
			{
				tempHum->state = TH_ERROR;
			}
		break;
		case TH_FINISH_READING:
		{
			uint8_t status = (tempHum->raw[0] & 0xc0) >> 6;
			if (status == 0 || status == 1)
			{
				tempHum->result[0].humidity.raw = (uint16_t)(tempHum->raw[0] & 0x3f) << 8 | tempHum->raw[1];
				tempHum->result[0].temperature.raw = ((uint16_t)(tempHum->raw[2]) << 8 | (tempHum->raw[3] & 0xfc)) >> 2;
				//knx 2 byte float conversion
				// convert humidity to knx float
				uint8_t exponent = 0;
				int32_t tmp = tempHum->result[0].humidity.raw;
				tmp *= (100 * 100); // 100% and knx multiplication by 0.01
				tmp = tmp >> 14;
				while (tmp > (0x03ff)) {
					tmp = tmp >> 1;
					exponent++;
				}
				tempHum->result[0].humidity.knxFloat = ((uint16_t)exponent << 11) | (tmp & 0x87ff);
				// convert temperature to humidity float
				exponent = 0;
				tmp = tempHum->result[0].temperature.raw;
				tmp *= (165 * 100); // 165 degrees and knx multiplication by 0.01
				tmp = tmp >> 14;
				tmp -= 4000; // start at -40 degrees in milli degrees
				while (tmp > (0x03ff)) {
					tmp = tmp >> 1;
					exponent++;
				}
				while (tmp < (-0x03ff)) {
					tmp = (tmp >> 1) | 0x80000000;
					exponent++;
				}
				tempHum->result[0].temperature.knxFloat = ((uint16_t)exponent << 11) | (tmp & 0x87ff);
				
				if (status == 0)
					tempHum->result[0].measState = HIH_MEAS_OK;
				else
					tempHum->result[0].measState = HIH_MEAS_STALE;
				
				tempHum->state = TH_DATA_READY;
			}
			else
			{
				tempHum->result[0].measState = HIH_MEAS_ERROR;
				tempHum->state = TH_ERROR;
			}
			
		}
		break;
		case TH_DATA_READY:
			if (tempHum->measurement_finished_cb)
			{
				tempHum->measurement_finished_cb(tempHum->parentHandle, tempHum->result);
				tempHum->measurement_finished_cb = NULL;
			}
			tempHum->state = TH_IDLE;
		break;
		case TH_ERROR:
			tempHum->result[0].humidity.raw = 0xffff;
			tempHum->result[0].temperature.raw = 0xffff;
			tempHum->result[0].humidity.knxFloat = 0x7fff; // knx float error value
			tempHum->result[0].temperature.knxFloat = 0x7fff;
			tempHum->result[0].measState = HIH_MEAS_ERROR;
			if (tempHum->measurement_finished_cb)
			{
				tempHum->measurement_finished_cb(tempHum->parentHandle, tempHum->result);
				tempHum->measurement_finished_cb = NULL;
			}		
			tempHum->state = TH_IDLE;
		break;
		default:
		break;
	}
}


void HIH_data_ready_callback(void *parentHandle, i2c_status_e status) {
	HIH_Obj *th;
	th = (HIH_Obj *)parentHandle;
	
	if (status == I2C_STATUS_OK)
		th->state = TH_FINISH_READING;
	else
		th->state = TH_ERROR;
}
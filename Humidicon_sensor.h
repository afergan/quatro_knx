/*
 * tempHum.h
 *
 * Created: 9-7-2016 7:44:13
 *  Author: Paul
 */ 


#ifndef HUMIDICON_SENSOR_H_
#define HUMIDICON_SENSOR_H_

#include "clocks.h"
#include "i2c.h"

#define HIH_I2C_ADDRESS				 0x27

typedef enum {
	HIH_SENSOR_PWR_OFF = 0,
	HIH_SENSOR_PWR_ON = 1
} hih_power_e;

#define HIH_STARTUP_TIME_MS			100
#define HIH_MEASUREMENT_TIME_MS		80
#define HIH_READ_TIMEOUT_MS			10

typedef enum {
	TH_STARTUP,
	TH_IDLE,
	TH_MEASURING,
	TH_READING,
	TH_FINISH_READING,
	TH_DATA_READY,
	TH_ERROR
} th_state_e;

typedef enum {
	HIH_MEAS_OK,
	HIH_MEAS_STALE,
	HIH_MEAS_ERROR
} hih_meas_e;

typedef struct knxFloatTwoByte_s {
	uint16_t		raw;
	uint16_t		knxFloat;			
}knxFloatTwoByte;

typedef struct {
	knxFloatTwoByte				temperature;
	knxFloatTwoByte				humidity;
	hih_meas_e					measState;
} HIH_measurement_result;

typedef struct {
	int8_t						enable;
	i2cHandle					i2c_handle;
	PORT_t						*pwrPort;
	uint8_t						pwrBit;
	uint8_t						raw[4];
	HIH_measurement_result		result[4];
	th_state_e					state;
	timeoutTmr					measurementTimeout;
	void						*parentHandle;
	void						(*measurement_finished_cb)(void *parentHandle, HIH_measurement_result *result);
} HIH_Obj;

typedef HIH_Obj *HIH_Handle;

HIH_Handle HIH_Init(void *pMemory, uint16_t numBytes);

int8_t HIH_Setup(HIH_Handle handle, i2cHandle i2c_handle, PORT_t *pwrPort, uint8_t pwrBit);

int8_t HIH_Reset(HIH_Handle handle);

int8_t HIH_StartMeas(HIH_Handle handle, void *measurement_finished_cb, void *parentHandle);

void HIH_Service(HIH_Handle handle);

void HIH_data_ready_callback(void *parentHandle, i2c_status_e status);

#endif /* HUMIDICON_SENSOR_H_ */
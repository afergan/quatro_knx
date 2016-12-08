/*
 * i2c.h
 *
 * Created: 9-7-2016 7:43:06
 *  Author: Paul
 */ 


#ifndef I2C_H_
#define I2C_H_

//#define I2C_ADDRESS			0x27

#define I2C_TIMEOUT_MS		100
#define I2C_MAX_LENGTH		8

typedef enum {
	I2C_STATE_WAITING,
	I2C_STATE_READING,
	I2C_STRETCH_CLK,
	I2C_STATE_WRITING,
	I2C_STATE_DATA_READY,
	I2C_STATE_NACK,
	I2C_STATE_ERROR
} i2c_state_e;

typedef enum {
	I2C_STATUS_OK,
	I2C_STATUS_ERROR
} i2c_status_e;

typedef struct _i2cObj {
	int8_t					enable;
	TWI_t					*twi;
	volatile i2c_state_e	state;
	uint8_t					*data;
	uint8_t					length;
	timeoutTmr				timeout;
	void					*parentHandle;
	void					(*data_ready_cb)(void *parentHandle, i2c_status_e status);
} i2cObj;

typedef struct _i2cObj *i2cHandle;

i2cHandle i2cInit(void *pMemory, uint16_t numBytes);

int8_t i2cSetup(i2cHandle handle, void *twi, uint32_t br);

int8_t i2cReset(i2cHandle handle);

int8_t i2cRead(i2cHandle handle, uint8_t address, uint8_t *data, uint8_t length);

int8_t i2cReadNonBlocking(i2cHandle handle, uint8_t address, uint8_t *data, uint8_t length, void *data_ready_cb, void *parentHandle);

int8_t i2cWrite(i2cHandle handle, uint8_t address,  uint8_t *data, uint8_t length);

int8_t i2cWriteNonBlocking(i2cHandle handle, uint8_t address, uint8_t *data, uint8_t length, void *data_ready_cb, void *parentHandle);

i2c_state_e i2cGetState(i2cHandle handle);

void i2cMasterIsr(i2cHandle handle);

#endif /* I2C_H_ */
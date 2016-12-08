/*
 * i2c.c
 *
 * Created: 9-7-2016 7:43:18
 *  Author: Paul
 */ 

#include "clocks.h"
#include "interrupt.h"
#include "i2c.h"

i2cHandle i2cInit(void *pMemory, uint16_t numBytes) {
	i2cHandle i2c_handle;
	if (numBytes < sizeof(i2cObj))
		return ((i2cHandle)NULL);
	i2c_handle = (i2cHandle)pMemory;
	return (i2c_handle);
}

int8_t i2cSetup(i2cHandle handle, void *twi, uint32_t br) {
	i2cObj *i2c;
	i2c = (i2cObj *)handle;
	
	i2c->twi = (TWI_t*)twi;

	i2c->state = I2C_STATE_WAITING;
	i2c->enable = 1;
	
	PORTE.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;
	PORTE.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc;
	
	i2c->twi->CTRL = 0;
	i2c->twi->MASTER.BAUD = (F_CPU / (2 * br)) - 5;
	i2c->twi->MASTER.CTRLC = 0;
	i2c->twi->MASTER.CTRLB = 0;// _BV(TWI_MASTER_SMEN_bp); // enable automatic ack.
	
	i2c->twi->MASTER.CTRLA = TWI_MASTER_INTLVL_MED_gc | _BV(TWI_MASTER_RIEN_bp) | _BV(TWI_MASTER_ENABLE_bp);
	i2c->twi->MASTER.STATUS = _BV(TWI_MASTER_BUSSTATE0_bp); // set bus state to idle

	return 0;
}

int8_t i2cReset(i2cHandle handle) {
	i2cObj *i2c;
	i2c = (i2cObj *)handle;
	if (!i2c->enable)
		return -1;
	volatile int8_t maxStp = 20;
	while (((i2c->twi->MASTER.STATUS & 0x3) !=  1) && (maxStp--))
	{
		i2c->twi->MASTER.CTRLC = _BV(TWI_MASTER_CMD0_bp) | _BV(TWI_MASTER_CMD1_bp); // generate i2c stop condition
		_delay_us(10);
	}
	i2c->twi->MASTER.CTRLA = TWI_MASTER_INTLVL_MED_gc | _BV(TWI_MASTER_RIEN_bp) | _BV(TWI_MASTER_ENABLE_bp);
	i2c->twi->MASTER.STATUS = _BV(TWI_MASTER_WIF_bp) | _BV(TWI_MASTER_RIF_bp) | _BV(TWI_MASTER_BUSSTATE0_bp); // set bus state to idle
	i2c->state = I2C_STATE_WAITING;
	
	return 0;	
}

int8_t i2cRead(i2cHandle handle, uint8_t address, uint8_t *data, uint8_t length) {
	volatile i2cObj *i2c;
	i2c = (i2cObj *)handle;
	if (!i2c->enable || length == 0)
		return -1;
	if (i2c->state == I2C_STATE_ERROR)
	{
		i2cReset(handle);
	}
	while (i2c->state == I2C_STATE_READING || i2c->state == I2C_STATE_WRITING)
		if (isTimedout(&i2c->timeout))
			i2cReset(handle);

	i2c->data_ready_cb = NULL;
	i2c->data = data;
	i2c->length = length;
	i2c->state = I2C_STATE_READING;
	setupTimeoutTmr(&i2c->timeout, I2C_TIMEOUT_MS);
	i2c->twi->MASTER.ADDR = (address << 1) | 0x01;
	while (i2c->state == I2C_STATE_READING) {
		if (isTimedout(&i2c->timeout))
			i2c->state = I2C_STATE_ERROR;
	}
	if (i2c->state == I2C_STATE_WAITING)
		return length;
	else
		return -1;
}

int8_t i2cReadNonBlocking(i2cHandle handle, uint8_t address, uint8_t *data, uint8_t length, void *data_ready_cb, void *parentHandle) {
	volatile i2cObj *i2c;
	i2c = (i2cObj *)handle;
	if (!i2c->enable || length == 0)
		return -1;
	if (i2c->state == I2C_STATE_ERROR)
	{
		i2cReset(handle);
	}
	while (i2c->state == I2C_STATE_READING || i2c->state == I2C_STATE_WRITING)
		if (isTimedout(&i2c->timeout))
			i2cReset(handle);

	i2c->parentHandle = parentHandle;
	i2c->data_ready_cb = data_ready_cb;
	i2c->data = data;
	i2c->length = length;
	i2c->state = I2C_STATE_READING;
	setupTimeoutTmr(&i2c->timeout, I2C_TIMEOUT_MS);
	i2c->twi->MASTER.ADDR = (address << 1) | 0x01;
	return 0;
}

int8_t i2cWrite(i2cHandle handle, uint8_t address, uint8_t *data, uint8_t length) {
	volatile i2cObj *i2c;
	i2c = (i2cObj *)handle;
	if (!i2c->enable)
		return -1;
	if (i2c->state == I2C_STATE_ERROR)
	{
		i2cReset(handle);
	}
	while (i2c->state == I2C_STATE_READING || i2c->state == I2C_STATE_WRITING)
		if (isTimedout(&i2c->timeout))
			i2cReset(handle);

	i2c->data_ready_cb = NULL;	
	i2c->data = data;
	i2c->length = length;
	i2c->state = I2C_STATE_WRITING;
	setupTimeoutTmr(&i2c->timeout, I2C_TIMEOUT_MS);
	i2c->twi->MASTER.ADDR = (address << 1) & 0xfe;
	i2c->twi->MASTER.CTRLA |= _BV(TWI_MASTER_WIEN_bp);
	while (i2c->state == I2C_STATE_WRITING)
		if (isTimedout(&i2c->timeout))
			i2c->state = I2C_STATE_ERROR;
	
	if (i2c->state == I2C_STATE_WAITING)
		return length;
	else if (i2c->state == I2C_STATE_NACK)
		return -2;
	else
		return -1;
}

int8_t i2cWriteNonBlocking(i2cHandle handle, uint8_t address, uint8_t *data, uint8_t length, void *data_ready_cb, void *parentHandle) {
	volatile i2cObj *i2c;
	i2c = (i2cObj *)handle;
	if (!i2c->enable)
		return -1;
	if (i2c->state == I2C_STATE_ERROR)
	{
		i2cReset(handle);
	}
	while (i2c->state == I2C_STATE_READING || i2c->state == I2C_STATE_WRITING)
		if (isTimedout(&i2c->timeout))
			i2cReset(handle);
	
	i2c->parentHandle = parentHandle;
	i2c->data_ready_cb = data_ready_cb;
	i2c->data = data;
	i2c->length = length;
	i2c->state = I2C_STATE_WRITING;
	setupTimeoutTmr(&i2c->timeout, I2C_TIMEOUT_MS);
	i2c->twi->MASTER.ADDR = (address << 1) & 0xfe;
	i2c->twi->MASTER.CTRLA |= _BV(TWI_MASTER_WIEN_bp);
	return 0;
}

i2c_state_e i2cGetState(i2cHandle handle) {
	volatile i2cObj *i2c;
	i2c = (i2cObj *)handle;
	return i2c->state;
}

void i2cMasterIsr(i2cHandle handle) {
	i2cObj *i2c;
	i2c = (i2cObj *)handle;

	if ((i2c->twi->MASTER.STATUS & (_BV(TWI_MASTER_ARBLOST_bp) | _BV(TWI_MASTER_BUSERR_bp) |_BV(TWI_MASTER_RXACK_bp))))
	{
		i2c->twi->MASTER.CTRLA &= ~(_BV(TWI_MASTER_WIEN_bp)); // disable write interrupt
		i2c->twi->MASTER.CTRLC = _BV(TWI_MASTER_CMD0_bp) | _BV(TWI_MASTER_CMD1_bp); // generate i2c stop condition
		i2c->twi->MASTER.STATUS = _BV(TWI_MASTER_BUSSTATE0_bp); // set bus state to idle
		if (i2c->data_ready_cb != NULL)
		{
			i2c->data_ready_cb(i2c->parentHandle, I2C_STATUS_ERROR);
			i2c->data_ready_cb = NULL; // reset call back
		}
		i2c->state = ((i2c->twi->MASTER.STATUS & _BV(TWI_MASTER_RXACK_bp)) != 0) ? I2C_STATE_NACK : I2C_STATE_ERROR;
	}
	else if ((i2c->twi->MASTER.STATUS) & _BV(TWI_MASTER_WIF_bp)) { // write interrupt

		if (i2c->state == I2C_STATE_WRITING)
		{
				if (i2c->length) {
					uint8_t next = i2c->data[0];
					i2c->data++;
					i2c->length--;
					i2c->twi->MASTER.DATA = next;
				}
				else
				{
					i2c->twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc; // generate i2c stop condition
					i2c->twi->MASTER.CTRLA &= ~(_BV(TWI_MASTER_WIEN_bp));
					if (i2c->data_ready_cb != NULL)
					{
						i2c->data_ready_cb(i2c->parentHandle, I2C_STATUS_OK);
						i2c->data_ready_cb = NULL; // reset call back
					}
					i2c->state = I2C_STATE_WAITING;
				}				
			}
			
	} 
	else if ((i2c->twi->MASTER.STATUS) & _BV(TWI_MASTER_RIF_bp)) { // read interrupt
		
		if (i2c->state == I2C_STATE_READING && i2c->length > 0)
		{
			
			*i2c->data++ = i2c->twi->MASTER.DATA;
			i2c->length--;
			if (i2c->length)
			{
				i2c->twi->MASTER.CTRLC = _BV(TWI_MASTER_CMD1_bp); // generate acknowledge
			}
			else
			{
				i2c->twi->MASTER.CTRLC = _BV(TWI_SLAVE_ACKACT_bp) | TWI_MASTER_CMD_STOP_gc; // when all bytes received, generate i2c nack and stop condition
				if (i2c->data_ready_cb != NULL)
				{
					i2c->data_ready_cb(i2c->parentHandle, I2C_STATUS_OK);
					i2c->data_ready_cb = NULL; // reset call back
				}
				i2c->state = I2C_STATE_WAITING;				
			}
		}
				
	} 
}
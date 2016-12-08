/*
 * tranceiver.h
 *
 *
 * resembles the link layer according to the OSI-model
 *
 * Created: 22-5-2016 16:16:41
 *  Author: Paul
 */ 


#ifndef LINK_LAYER_H_
#define LINK_LAYER_H_

#include "clocks.h"
#include "serial.h"
#include "fifo.h"
#include "telegram.h"

#define		TRANCEIVER_ADDRESSED_BP		0
#define		TRANCEIVER_BUSY_BP			1
#define		TRANCEIVER_NACK_BP			2

#define		L_DATA_TIMEOUT_MS			200

#define		TELEGRAM_IN_FIFO_LENGTH		4
#define		TELEGRAM_OUT_FIFO_LENGTH	4

// Host to Uart
#define  U_RESET_REQUEST				0x01
#define  U_STATE_REQUEST				0x02
#define  U_ACTIVATE_BUSMON				0x05
#define  U_ACK_INFORM_VOID				0x10
#define  U_ACK_INFORM_NACK				0x14
#define  U_ACK_INFORM_BUSY				0x12
#define  U_ACK_INFORM_ADDRESSED			0x11
#define  U_PRODUCT_ID_REQUEST			0x20
#define  U_ACTIVATE_BUSY_MODE			0x21
#define  U_REST_BUSY_MODE				0x22
#define  U_MX_RST_CNT					0x24
#define  U_ACTIVATE_CRC					0x25
#define  U_SET_KNX_ADDRESS				0x28
#define  U_SET_ALARM_TELEGRAM			0x29
#define  U_SET_TRIGGER_TELEGRAM			0x2A
#define  U_SET_TRIGGER_TELEGRAM_MASK	0x2B
#define  U_READ_REGISTER_REQUEST		0x2E
#define  U_WRITE_REGISTER				0x2F
#define  U_L_DATA_START					0x80
#define  U_L_DATA_CONTINUE				0x81
#define  U_L_DATA_END					0x40
#define  U_POLLING_STATE				0xE0
#define  U_L_LONG_DATA_CONTINUE			0xC0
#define  U_L_LONG_DATA_END				0xD0

// Uart to Host
#define  H_ACKNOWLEDGE					0x00		// only in busmon
#define  H_RESET_INDICATION				0x03
#define  H_L_DATA_NEGATIVE_CONFIRM		0x0B
#define  H_NEGATIVE_ACKNOWLEDGE			0x0C		// only in busmon
#define  H_L_DATA_TELEGRAM_EXTENDED		0x10		// extended frame
#define  H_L_DATA_POSITIVE_CONFIRM		0x8B
#define  H_L_DATA_TELEGRAM				0x90		// standard frame
#define  H_BUSY_ACKNOWLEDGE				0xC0		// only in busmon
#define  H_POSITIVE_ACKNOWLEDGE			0xCC		// only in busmon
#define  H_L_POLL_DATA_REQUEST			0xF0
#define  H_READ_REGISTER_RESPONSE		0xF1
#define  H_READ_PROD_ID_RESPONSE		0xFE
#define  H_STATE_INDICATION				0x07

// UART memory addresses
#define  E98103_CMODE					0x200
#define  E98103_RESET_CTRL				0x201
#define  E98103_BUSY_REG				0x202
#define  E98103_SPI_CTRL				0x205
#define  E98103_SPI_PINS				0x206
#define  E98103_UART_CTRL				0x208
#define  E98103_CLK_CTRL				0x209
#define  E98103_CLK_FAC0				0x20a
#define  E98103_CLK_FAC1				0x20b
#define  E98103_PS_CTRL					0x20e
#define  E98103_MAX_BUS_CURR			0x20f
#define  E98103_CURRENT_SLOPE			0x210
#define  E98103_AOUT_CTRL				0x211
#define  E98103_AOUT_SRC				0x212
#define  E98103_ALARM_STAT				0x213
#define  E98103_TRIGGER					0x214
#define  E98103_KNX_TR_BUF_STAT			0x215
#define  E98103_KNX_ADR_STAT			0x216
#define  E98103_MAX_RST_CNT				0x217
#define  E98103_KNX_TX_LEN1				0x218
#define  E98103_KNX_RX_LEN0				0x219
#define  E98103_ACK_HOST				0x21a
#define  E98103_POLL_CONF				0x21b
#define  E98103_UART_STAT				0x2a0
#define  E98103_UART_RX					0x2a3
#define  E98103_UART_TX					0x2a4
#define  E98103_DEVMODE					0x300
#define  E98103_RES_SOURCE				0x302
#define  E98103_PINS					0x306
#define  E98103_SPI_STAT				0x310
#define  E98103_PROD_ID					0x371
#define  E98103_ADC_VSTRES				0x397
#define  E98103_ADC_V20RES				0x398
#define  E98103_ADC_VCCRES				0x399
#define  E98103_ADC_VIORES				0x39a
#define  E98103_ADC_VBUSP_MEAN			0x39d
#define  E98103_ADC_TEMPRES				0x39e
#define  E98103_BUS_CURR_STAT			0x3bo
#define  E98103_PS_STAT					0x3bf
#define  E98103_ACK_KNXIC				0x3e9

typedef enum {
	L_DATA_READY,
	L_DATA_WAITING_FOR_ACK,
	L_DATA_NACK_BUSY
} l_data_status_e;

typedef enum {
	L_POSITIVE_CONFIRM,
	L_NEGATIVE_CONFIRM
} l_confirm_e;

typedef enum {
	L_CON_NOT_OK,
	L_CON_OK
} l_status_e;

typedef enum {
	L_SEND_ASYNC,
	L_SEND_SYNC
} l_send_sync;

typedef struct {
	uint8_t			ack_request;
	address_type_e	daf;
	uint16_t		source_addres;
	uint16_t		destination_address;
	uint8_t			frame_format;
	uint8_t			hop_count;
	uint8_t			octet_count;
	priority_e		priority;
	uint8_t			*lsdu;
	l_status_e		l_status;			
} L_Data_s;

typedef struct _tranceiverObj {
	uint8_t				enable;
	void				*parent_Hndl;
	uint8_t				pushTelegram;
	serialObj			E981_03_serial_obj;
	serialHandle		E981_03_serial_handle;
	fifoObj				E981_03_in_fifo_obj;
	fifoBuffer			E981_03_in_fifo_buffer[TELEGRAM_IN_FIFO_LENGTH];
	fifoHandle			E981_03_in_fifo_handle;
	fifoObj				E981_03_out_fifo_obj;
	fifoBuffer			E981_03_out_fifo_buffer[TELEGRAM_OUT_FIFO_LENGTH];
	fifoHandle			E981_03_out_fifo_handle;
	l_data_status_e		l_dataStatus;
	timeoutTmr			l_dataTimeout;
	fifoBuffer			*writeBufferP;
	void				(*L_Data_ind_cb)(void *parent_Hndl, L_Data_s *L_Data_ind);
	void				(*L_SystemBroadcast_ind_cb)(void *parent_Hndl, L_Data_s *L_Data_ind);
	void				(*L_Data_con_cb)(void *parent_Hndl, L_Data_s *L_Data_ind);
	void				(*L_SystemBroadcast_con_cb)(void *parent_Hndl, L_Data_s *L_Data_ind);
} tranceiverObj;

typedef tranceiverObj *tranceiverHandle;

tranceiverHandle tranceiverInit(void *pMemory, uint16_t numBytes);

//int8_t tranceiverSetup(tranceiverHandle handle, serialHandle sh, fifoHandle inFifo, fifoHandle outFifo);
int8_t tranceiverSetup(tranceiverHandle handle, void *parent_Hndl);

int8_t tranceiverEnable(tranceiverHandle handle);

int8_t tranceiverDisable(tranceiverHandle handle);

int16_t tranceiverGetProductId(tranceiverHandle handle, uint8_t *version); // 0x20

int16_t tranceiverReadReg(tranceiverHandle handle, uint16_t address, uint8_t *value); // 0x2E

int8_t tranceiverWriteReg(tranceiverHandle handle, uint16_t address, uint8_t value); // 0x2F

int16_t tranceiverStateReq(tranceiverHandle handle, uint8_t *state); // 0x02

int8_t tranceiver_Acknowledge_Information(tranceiverHandle handle, uint8_t ackMask); //0x10 - 0x17

int8_t tranceiverActivateBusmon(tranceiverHandle handle); // 0x05

int8_t tranceiverResetReq(tranceiverHandle handle); // 0x01

int8_t tranceiverActivateBusyMode(tranceiverHandle handle); // 0x21

int8_t tranceiverResetBusyMode(tranceiverHandle handle); // 0x22

int8_t tranceiverMxRstCnt(tranceiverHandle handle, uint8_t busyAckMask); // 0x24

int16_t tranceiver_Push_L_Data(tranceiverHandle handle, uint8_t *telegram, uint8_t length); // 0x80

int16_t tranceiver_Send_L_Data(tranceiverHandle handle, uint8_t *telegram, uint8_t length); // 0x80

void tranceiver_L_Data_confirm(tranceiverHandle handle, l_confirm_e confirm);

void L_data_Service(tranceiverHandle handle);

int8_t get_L_Data_from_telegram(uint8_t *telegram, L_Data_s *L_Data);

void L_Data_req(tranceiverHandle handle, L_Data_s *L_data_req, l_send_sync sync);

void L_SystemBroadcast_req(tranceiverHandle handle, L_Data_s *L_data_req, l_send_sync sync);

void tranceiver_data_ready_callback(tranceiverHandle handle, uint8_t *data, uint16_t length);

void tranceiver_message_type_callback(tranceiverHandle handle, address_type_e daf, uint16_t destAddr);

#endif /* LINK_LAYER_H_ */
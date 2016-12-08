/*
 * linkLayer.c
 *
 * resembles the link layer according to the OSI-model
 *
 * Created: 22-5-2016 16:16:28
 *  Author: Paul
 */ 

#include "linkLayer.h"
#include "applicationInterface.h"
#include "util\atomic.h"

serialHandle L_data_SerialHandle;

tranceiverHandle tranceiverInit(void *pMemory, uint16_t numBytes)
{
	tranceiverHandle tranceiver_handle;
	if (numBytes < sizeof(tranceiverObj))
		return ((tranceiverHandle)NULL);
	tranceiver_handle = (tranceiverHandle)pMemory;
		return(tranceiver_handle);
}

int8_t tranceiverSetup(tranceiverHandle handle, void *parent_Hndl) {
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	
	
	tranceiver->E981_03_serial_handle = serialInit(&tranceiver->E981_03_serial_obj, sizeof(tranceiver->E981_03_serial_obj));
	serialSetup(tranceiver->E981_03_serial_handle, &USARTC0, 115200, 9, PARITY_EVEN, &TCE0, 2500, tranceiver, tranceiver_data_ready_callback, tranceiver_message_type_callback);
	serialEnable(tranceiver->E981_03_serial_handle);

	tranceiver->E981_03_in_fifo_handle = fifoInit(&tranceiver->E981_03_in_fifo_obj, sizeof(tranceiver->E981_03_in_fifo_obj));
	fifoSetup(tranceiver->E981_03_in_fifo_handle, tranceiver->E981_03_in_fifo_buffer, TELEGRAM_IN_FIFO_LENGTH);

	tranceiver->E981_03_out_fifo_handle = fifoInit(&tranceiver->E981_03_out_fifo_obj, sizeof(tranceiver->E981_03_out_fifo_obj));
	fifoSetup(tranceiver->E981_03_out_fifo_handle, tranceiver->E981_03_out_fifo_buffer, TELEGRAM_OUT_FIFO_LENGTH);
	
	L_data_SerialHandle = tranceiver->E981_03_serial_handle;
	
	tranceiver->parent_Hndl = parent_Hndl;

	tranceiver->enable = 1;
	return 0;
}

int8_t tranceiverEnable(tranceiverHandle handle) {
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	tranceiver->enable = 1;
	return 0;
}

int8_t tranceiverdisable(tranceiverHandle handle) {
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	tranceiver->enable = 0;
	return 0;
}

int16_t tranceiverGetProductId(tranceiverHandle handle, uint8_t *version) {
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	if (!tranceiver->enable)
		return -1;
	uint8_t command[] = { 0x20};
	serialWrite(tranceiver->E981_03_serial_handle, command, 1);
	int16_t byteCnt = readFifoBufferWithMatch(tranceiver->E981_03_in_fifo_handle, version, 2, 0xFE);
	return byteCnt;
}

int16_t tranceiverReadReg(tranceiverHandle handle, uint16_t address, uint8_t *value) { // 0x2E
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	if (!tranceiver->enable)
		return -1;
	uint8_t command[3] = { 0x2E, 0x00, 0x00};
	command[1] = (uint8_t)((address & 0x0300) >> 8);
	command[2] = (uint8_t)(address & 0x00FF);
	serialWrite(tranceiver->E981_03_serial_handle, command, 3);
	int16_t byteCnt = readFifoBufferWithMatch(tranceiver->E981_03_in_fifo_handle, value, 2, 0xF1);
	return byteCnt;
}

int8_t tranceiverWriteReg(tranceiverHandle handle, uint16_t address, uint8_t value) { // 0x2F

	return 0;
}

int8_t tranceiverResetReq(tranceiverHandle handle) { // 0x01
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	if (!tranceiver->enable)
		return -1;

	uint8_t e981_message[1] = { 0x01 };
	serialWrite(tranceiver->E981_03_serial_handle, e981_message, 1);	
		
	return 0;
}


int16_t tranceiverStateReq(tranceiverHandle handle, uint8_t *state) { // 0x02
	
	return 0;
}

int8_t tranceiverActivateBusmon(tranceiverHandle handle) { // 0x05
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	if (!tranceiver->enable)
		return -1;

	uint8_t e981_message[1] = { 0x05 };
	serialWrite(tranceiver->E981_03_serial_handle, e981_message, 1);	
		
	return 0;
}

int16_t tranceiver_Push_L_Data(tranceiverHandle handle, uint8_t *telegram, uint8_t length) { // 0x80
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	if (!tranceiver->enable || length > 63)
		return -1;

	return pushFifoBuffer(tranceiver->E981_03_out_fifo_handle, telegram, length);
}


// blocking mode send
int16_t tranceiver_Send_L_Data(tranceiverHandle handle, uint8_t *telegram, uint8_t length) { // 0x80
	volatile tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	if (!tranceiver->enable || length > 63)
		return -1;
	
	fifoBuffer *firstMessage = getFirstFifo(handle->E981_03_out_fifo_handle); // retrieve first message, if available, to empty fifo
	
	while (firstMessage != NULL) // empty out buffer
	{
		while (tranceiver->l_dataStatus == L_DATA_WAITING_FOR_ACK)	// check if still busy with last telegram
			if (isTimedout(&tranceiver->l_dataTimeout))
				tranceiver_L_Data_confirm(handle, L_NEGATIVE_CONFIRM);
		
		if (firstMessage != NULL)
		{
			if (serialWriteLdata(tranceiver->E981_03_serial_handle, firstMessage->data, firstMessage->length) >= 0) // non-blocking write of L_data frame
			{
				tranceiver->writeBufferP = firstMessage;
				tranceiver->l_dataStatus = L_DATA_WAITING_FOR_ACK;
				setupTimeoutTmr(&tranceiver->l_dataTimeout, L_DATA_TIMEOUT_MS);
			}
			else
			{
				freeFifoBuffer(handle->E981_03_out_fifo_handle, tranceiver->writeBufferP);
				tranceiver->writeBufferP = NULL;
			}

		}
		firstMessage = getFirstFifo(handle->E981_03_out_fifo_handle);
	}

	pushFifoBuffer(tranceiver->E981_03_out_fifo_handle, telegram, length); // push telegram in the fifo buffer
	
	firstMessage = getFirstFifo(handle->E981_03_out_fifo_handle); // retrieve first message. (hopefully the one just pushed!)

	while (tranceiver->l_dataStatus == L_DATA_WAITING_FOR_ACK)	// check if still busy with last telegram
		if (isTimedout(&tranceiver->l_dataTimeout))
			tranceiver_L_Data_confirm(handle, L_NEGATIVE_CONFIRM);
		
	if (firstMessage) // send telegram
	{
		if (serialWriteLdata(tranceiver->E981_03_serial_handle, firstMessage->data, firstMessage->length) >= 0) // non-blocking write of L_data frame
		{
			tranceiver->writeBufferP = firstMessage;
			tranceiver->l_dataStatus = L_DATA_WAITING_FOR_ACK;
			setupTimeoutTmr(&tranceiver->l_dataTimeout, L_DATA_TIMEOUT_MS);
		}
		else
		{
			freeFifoBuffer(handle->E981_03_out_fifo_handle, tranceiver->writeBufferP);
			tranceiver->writeBufferP = NULL;
		}
	}
	return 0;	
}

void tranceiver_L_Data_confirm(tranceiverHandle handle, l_confirm_e confirm) {
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;

	fifoBuffer *telegram = tranceiver->writeBufferP;
	
	L_Data_s L_Data_con;
	
	get_L_Data_from_telegram(telegram->data, &L_Data_con);
	
	if (confirm == L_POSITIVE_CONFIRM)
		L_Data_con.l_status = L_CON_OK;
	else
		L_Data_con.l_status = L_CON_NOT_OK;

	if (L_Data_con.daf == KNX_GROUP_ADDRESS && L_Data_con.destination_address == 0)
	{
		if (tranceiver->L_SystemBroadcast_con_cb)
			tranceiver->L_SystemBroadcast_con_cb(tranceiver->parent_Hndl, &L_Data_con);
	}
	else if (L_Data_con.daf == KNX_GROUP_ADDRESS || (L_Data_con.daf == KNX_PHYSICAL_ADDRESS && L_Data_con.destination_address != 0))
	{
		if (tranceiver->L_Data_con_cb)
			tranceiver->L_Data_con_cb(tranceiver->parent_Hndl, &L_Data_con);
	}

	if (tranceiver->l_dataStatus == L_DATA_WAITING_FOR_ACK)
	{
		if (tranceiver->writeBufferP)
		{
			freeFifoBuffer(handle->E981_03_out_fifo_handle, tranceiver->writeBufferP);
			tranceiver->writeBufferP = NULL;
		}
	}
	if (confirm == L_POSITIVE_CONFIRM)
		tranceiver->l_dataStatus = L_DATA_READY;
	else
		tranceiver->l_dataStatus = L_DATA_NACK_BUSY;	
}

int8_t tranceiver_Acknowledge_Information(tranceiverHandle handle, uint8_t ackMask) {
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	if (!tranceiver->enable)
		return -1;
	
	int8_t result;
	result = serialWriteSingleByte(tranceiver->E981_03_serial_handle, U_ACK_INFORM_VOID | (ackMask & 0x07));
	return (result > 0) ? 0 : result;	
}

void L_data_Service(tranceiverHandle handle) {  // runs in the main loop
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	if (!tranceiver->enable)
	return;

	if (tranceiver->l_dataStatus == L_DATA_WAITING_FOR_ACK)	// check if still busy with last telegram
	{
		if (isTimedout(&tranceiver->l_dataTimeout))
			tranceiver_L_Data_confirm(handle, L_NEGATIVE_CONFIRM);
	}
	else
	{
		// handle outgoing messages
	
		fifoBuffer *firstMessage = getFirstFifo(handle->E981_03_out_fifo_handle);
		if (firstMessage) 
		{
			if (serialWriteLdata(tranceiver->E981_03_serial_handle, firstMessage->data, firstMessage->length) >= 0) // non-blocking write of L_data frame
			{
				tranceiver->writeBufferP = firstMessage;
				tranceiver->l_dataStatus = L_DATA_WAITING_FOR_ACK;
				setupTimeoutTmr(&tranceiver->l_dataTimeout, L_DATA_TIMEOUT_MS);
			}
			else
			{
				freeFifoBuffer(handle->E981_03_out_fifo_handle, tranceiver->writeBufferP);
				tranceiver->writeBufferP = NULL;
			}

		}
	}

	// handle incoming messages from the fifo buffer.
	// raise proper call back function.
	
	fifoBuffer *firstMessage = getFirstFifo(handle->E981_03_in_fifo_handle);
	if (firstMessage) {
		if (firstMessage->length > 6)
		{
			// check checksum
			uint8_t chksum = 0;
			for (uint8_t i = 0; i < (firstMessage->length - 1); i++) {
				chksum ^= firstMessage->data[i];
			}
			if (*(firstMessage->data + firstMessage->length - 1) == (~chksum & 0xff))
			{	
				L_Data_s L_Data_ind;
				
				get_L_Data_from_telegram(firstMessage->data, &L_Data_ind);
				
				if (L_Data_ind.daf == KNX_GROUP_ADDRESS && L_Data_ind.destination_address == 0)
				{
					if (tranceiver->L_SystemBroadcast_ind_cb)
						tranceiver->L_SystemBroadcast_ind_cb(tranceiver->parent_Hndl, &L_Data_ind);
				}
				else if (L_Data_ind.daf == KNX_GROUP_ADDRESS || (L_Data_ind.daf == KNX_PHYSICAL_ADDRESS && L_Data_ind.destination_address != 0))
				{
					if (tranceiver->L_Data_ind_cb)
						tranceiver->L_Data_ind_cb(tranceiver->parent_Hndl, &L_Data_ind);
				}				
			}
		}
		freeFifoBuffer(handle->E981_03_in_fifo_handle, firstMessage);
	}
}

int8_t get_L_Data_from_telegram(uint8_t *telegram, L_Data_s *L_Data) {
	
	uint8_t controlField = telegram[0];

	if ((controlField & 0xD3) == 0x90 || (controlField & 0xD3) == 0xB0) // standard frame length. APDU <= 15 octets
	{
				
		L_Data->frame_format = 0x00;
		L_Data->priority = (priority_e)((controlField & 0x0C) >> 2);
		L_Data->source_addres = (uint16_t)(telegram[1]) << 8 | telegram[2];
		L_Data->destination_address = (uint16_t)(telegram[3]) << 8 | telegram[4];
		uint8_t npci = telegram[5];
		L_Data->daf = (address_type_e)((npci & 0x80) >> 7);
		L_Data->octet_count = npci & 0x0F;
		L_Data->lsdu = telegram + 6;
		L_Data->l_status = L_CON_OK;
		L_Data->ack_request = 0;
	}
	else if ((controlField & 0xD3) == 0x10 || (controlField & 0xD3) == 0xC0) // extended frame length. APDU > 15 octets
	{
					
		L_Data->priority = (priority_e)((controlField & 0x0C) >> 2);
		uint8_t control_E_field = telegram[1];
		L_Data->frame_format = 0x80 | (control_E_field & 0x0f);
		L_Data->daf = (address_type_e)((control_E_field & 0x80) >> 7);
		L_Data->source_addres = (uint16_t)(telegram[2]) << 8 | telegram[3];
		L_Data->destination_address = (uint16_t)(telegram[4]) << 8 | telegram[5];
		L_Data->octet_count = telegram[6];
		L_Data->lsdu = telegram + 7;
		L_Data->l_status = L_CON_OK;
		L_Data->ack_request = 0;
	}
	else if ((controlField & 0xF3) == 0xF0) // poll
	{
				
		L_Data->frame_format = 0x00;
		L_Data->source_addres = (uint16_t)(telegram[1]) << 8 | telegram[2];
		L_Data->destination_address = (uint16_t)(telegram[3]) << 8 | telegram[4];
		// not finished
	}
	
	return 0;	
}

void L_Data_req(tranceiverHandle handle, L_Data_s *L_data_req, l_send_sync sync) {
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	if (!tranceiver->enable)
		return;
		
	uint8_t idx = 0;
	uint8_t data[FIFO_BUFFER_SIZE];
	
	if (L_data_req->frame_format & 0x80)
	{
		if (L_data_req->octet_count > (FIFO_BUFFER_SIZE - 7))
			return;
		// extended frame
		data[idx++] = 0x30 | ((L_data_req->priority) << 2);
		data[idx++] = (L_data_req->daf << 7) |  (L_data_req->hop_count << 4);
		data[idx++] = L_data_req->source_addres >> 8;
		data[idx++] = L_data_req->source_addres & 0xff;
		data[idx++] = L_data_req->destination_address >> 8;
		data[idx++] = L_data_req->destination_address & 0xff;
		data[idx++] = L_data_req->octet_count;
	}
	else
	{
		// standard frame length <15
		if (L_data_req->octet_count > 0xf)
			return;
		data[idx++] = 0xb0 | ((L_data_req->priority) << 2);
		data[idx++] = L_data_req->source_addres >> 8;
		data[idx++] = L_data_req->source_addres & 0xff;
		data[idx++] = L_data_req->destination_address >> 8;
		data[idx++] = L_data_req->destination_address & 0xff;
		data[idx++] = (L_data_req->daf << 7) | (L_data_req->hop_count << 4) | (L_data_req->octet_count & 0xf);
		L_data_req->octet_count &= 0xf;
	}
	for (uint8_t i = 0; i < (L_data_req->octet_count + 1); i++)
	{
		data[idx++] = *(L_data_req->lsdu + i);
	}
	if (sync == L_SEND_SYNC)
		tranceiver_Send_L_Data(handle, data, idx);
	else
		tranceiver_Push_L_Data(handle, data, idx);
}

void L_SystemBroadcast_req(tranceiverHandle handle, L_Data_s *L_data_req, l_send_sync sync) {
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	if (!tranceiver->enable)
	return;
	
	uint8_t idx = 0;
	uint8_t data[FIFO_BUFFER_SIZE];
	
	if (L_data_req->frame_format & 0x80)
	{
		if (L_data_req->octet_count > (FIFO_BUFFER_SIZE - 7))
			return;
		// extended frame
		data[idx++] = 0x30 | ((L_data_req->priority) << 2);
		data[idx++] = (KNX_GROUP_ADDRESS << 7) |  (L_data_req->hop_count << 4);
		data[idx++] = L_data_req->source_addres >> 8;
		data[idx++] = L_data_req->source_addres & 0xff;
		data[idx++] = 0;
		data[idx++] = 0;
		data[idx++] = L_data_req->octet_count;

	}
	else
	{
		if (L_data_req->octet_count > 0xf)
			return;
		// standard frame length <15
		data[idx++] = 0xb0 | ((L_data_req->priority) << 2);
		data[idx++] = L_data_req->source_addres >> 8;
		data[idx++] = L_data_req->source_addres & 0xff;
		data[idx++] = 0;
		data[idx++] = 0;
		data[idx++] = (KNX_GROUP_ADDRESS << 7) | (L_data_req->hop_count << 4) | (L_data_req->octet_count & 0xf);
		L_data_req->octet_count &= 0xf;
	}
	for (uint8_t i = 0; i < L_data_req->octet_count + 1; i++)
	{
		data[idx++] = *(L_data_req->lsdu + i);
	}
	if (sync == L_SEND_SYNC)
		tranceiver_Send_L_Data(handle, data, idx);
	else
		tranceiver_Push_L_Data(handle, data, idx);
}


// full message received. Can be a telegram or any other reply from the uart (service message).
// Lets keep this function short as it runs in the serial ISR.

void tranceiver_data_ready_callback(tranceiverHandle handle, uint8_t *data, uint16_t length) {
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	
	
	
	if (tranceiver->pushTelegram && length > 6 && length <= 64) // push knx telegrams with matching destination addressing in fifo. Process in the main loop.
	{
		tranceiver->pushTelegram = 0;
		pushFifoBuffer(tranceiver->E981_03_in_fifo_handle, data, length);
	}
	else if (data[0] == H_L_DATA_POSITIVE_CONFIRM && length == 1)
	{
		tranceiver_L_Data_confirm(handle, L_POSITIVE_CONFIRM); // notify link layer of succes in transmitting L-data
	}
	else if (data[0] == H_L_DATA_NEGATIVE_CONFIRM && length == 1)
	{
		tranceiver_L_Data_confirm(handle, L_NEGATIVE_CONFIRM); // notify link layer of fail in transmitting L-data
	}
	else if (length < 4)
	{
		pushFifoBuffer(tranceiver->E981_03_in_fifo_handle, data, length); // push uart returns in fifo. Process in the main loop.
	}
}

// This function gets called when only part of the telegram is received. Immediately after the destination address, this callback function
// notifies the tranceiver if we are addressed or not (full duplex). The pushTelegram bit is set when a telegram is applicable to this device
// the data_ready_callback checks this bit and pushes the telegram in the receive fifo buffer.
// Lets keep this function short as it runs in the serial ISR.

void tranceiver_message_type_callback(tranceiverHandle handle, address_type_e daf, uint16_t destAddr) {
	tranceiverObj *tranceiver;
	tranceiver = (tranceiverObj *) handle;
	
	if (daf == KNX_PHYSICAL_ADDRESS && destAddr == N_get_ownAddress(tranceiver->parent_Hndl)) // match destination to own physical address. Unicast message
	{
		tranceiver->pushTelegram = 1;
		tranceiver_Acknowledge_Information(handle, U_ACK_INFORM_ADDRESSED);
	}
	else if (daf == KNX_GROUP_ADDRESS && destAddr == 0) // Broadcast message
	{
		tranceiver->pushTelegram = 1;
		tranceiver_Acknowledge_Information(handle, U_ACK_INFORM_ADDRESSED);
	}
	else if (daf == KNX_GROUP_ADDRESS && GrpSrv_ChkKnownGrpAddr(((A_InterfaceObj*)((A_LayerObj*)((T_layerObj*)((N_layerObj*)tranceiver->parent_Hndl)->parent_Hndl)->parent_Hndl)->parent_Hndl)->GO_Hndl ,destAddr)) // match destination address to list of known addresses. Multicast message
	{
		tranceiver->pushTelegram = 1;
		tranceiver_Acknowledge_Information(handle, U_ACK_INFORM_ADDRESSED);

	} else
		tranceiver_Acknowledge_Information(handle, U_ACK_INFORM_VOID);
}

ISR(USARTC0_RXC_vect) {
	serialRxIsr(L_data_SerialHandle);
}

ISR(USARTC0_DRE_vect){
	serialTxIsr(L_data_SerialHandle);
}

ISR(TCE0_OVF_vect){
	serialTimeOut(L_data_SerialHandle);
}

/*
 * networkLayer.c
 *
 * Created: 25/08/16 15:16:48
 *  Author: PvR
 */ 

#include "networkLayer.h"


N_layerHandle N_layerInit(void *pMemory, uint16_t numBytes)
{
	N_layerHandle N_layer_handle;
	if (numBytes < sizeof(N_layerObj))
		return ((N_layerHandle)NULL);
	N_layer_handle = (N_layerHandle)pMemory;
	return(N_layer_handle);
}

int8_t N_layerSetup(N_layerHandle handle, void *parent_Hndl) {
	N_layerObj *N_layer;
	N_layer = (N_layerObj *) handle;
	
	N_layer->tranceiver_handle = tranceiverInit(&N_layer->tranceiver, sizeof(N_layer->tranceiver));
	tranceiverSetup(N_layer->tranceiver_handle, N_layer);
	N_layer->tranceiver.L_Data_ind_cb = &L_Data_ind_cb;
	N_layer->tranceiver.L_SystemBroadcast_ind_cb = &L_SystemBroadcast_ind_cb;
	N_layer->tranceiver.L_Data_con_cb = &L_Data_con_cb;
	N_layer->tranceiver.L_SystemBroadcast_con_cb = &L_SystemBroadcast_con_cb;
	tranceiverEnable(N_layer->tranceiver_handle);	
	
	N_layer->parent_Hndl = parent_Hndl;
	N_layer->enable = 1;
	
	//N_layer->ownAddress = 0x1407;
	
	return 0;
}

void N_set_ownAddress(N_layerHandle handle, uint16_t *ownAddr) {
	N_layerObj *N_layer;
	N_layer = (N_layerObj *) handle;
	if (!N_layer->enable || !ownAddr)
		return;
	N_layer->ownAddress = ownAddr;
}

uint16_t N_get_ownAddress(N_layerHandle handle) {
	N_layerObj *N_layer;
	N_layer = (N_layerObj *) handle;
	if (!N_layer->enable)
		return 0;
	return *N_layer->ownAddress;
}

int8_t N_set_hopCnt(N_layerHandle handle, uint8_t *hopCnt) {
	N_layerObj *N_layer;
	N_layer = (N_layerObj *) handle;
	N_layer->hopCnt = hopCnt;
	return 0;
}

void N_service(N_layerHandle handle) {
	N_layerObj *N_layer;
	N_layer = (N_layerObj *) handle;
	if (!N_layer->enable)
		return;
	L_data_Service(N_layer->tranceiver_handle);	
}

void N_Data_Individual_req(N_layerHandle handle, N_Data_s *N_Data_req) {
	N_layerObj *N_layer;
	N_layer = (N_layerObj *) handle;	
	
	L_Data_s L_Data_rq;
	
	map_N_Data_to_L_Data(N_layer, &L_Data_rq, N_Data_req);
	L_Data_rq.daf = KNX_PHYSICAL_ADDRESS;
	//L_Data_rq.source_addres = N_layer->ownAddress;	
	
	L_Data_req(N_layer->tranceiver_handle, &L_Data_rq, L_SEND_ASYNC);  // push in fifo and send from main loop
}

void N_Data_Individual_ack_req(N_layerHandle handle, N_Data_s *N_Data_req) {
	N_layerObj *N_layer;
	N_layer = (N_layerObj *) handle;
	
	L_Data_s L_Data_rq;
	
	map_N_Data_to_L_Data(N_layer, &L_Data_rq, N_Data_req);
	L_Data_rq.daf = KNX_PHYSICAL_ADDRESS;
	
	L_Data_req(N_layer->tranceiver_handle, &L_Data_rq, L_SEND_SYNC);  // send in blocking mode
}

void N_Data_Group_req(N_layerHandle handle, N_Data_s *N_Data_req) {
	N_layerObj *N_layer;
	N_layer = (N_layerObj *) handle;	
	
	L_Data_s L_Data_rq;
	map_N_Data_to_L_Data(N_layer, &L_Data_rq, N_Data_req);
	//L_Data_rq.source_addres = N_layer->ownAddress;
	
	L_Data_req(N_layer->tranceiver_handle, &L_Data_rq, L_SEND_ASYNC); // push in fifo and send from main loop
}

void N_Data_Broadcast_req(N_layerHandle handle, N_Data_s *N_Data_req) {
	N_layerObj *N_layer;
	N_layer = (N_layerObj *) handle;

	L_Data_s L_Data_rq;
	map_N_Data_to_L_Data(N_layer, &L_Data_rq, N_Data_req);
	//L_Data_rq.source_addres = N_layer->ownAddress;
	L_Data_rq.destination_address = 0;
	
	L_SystemBroadcast_req(N_layer->tranceiver_handle, &L_Data_rq, L_SEND_ASYNC); // push in fifo and send from main loop
}

void N_Data_SystemBroadcast_req(N_layerHandle handle, N_Data_s *N_Data_req) {
	N_layerObj *N_layer;
	N_layer = (N_layerObj *) handle;
	
	L_Data_s L_Data_rq;
	map_N_Data_to_L_Data(N_layer, &L_Data_rq, N_Data_req);
	//L_Data_rq.source_addres = N_layer->ownAddress;
	L_Data_rq.destination_address = 0;
	
	L_SystemBroadcast_req(N_layer->tranceiver_handle, &L_Data_rq, L_SEND_ASYNC); // push in fifo and send from main loop
}

void L_Data_ind_cb(void *parent_Hndl, L_Data_s *L_Data_ind) {
	N_layerObj *N_layer;
	N_layer = parent_Hndl;
	
	N_Data_s N_Data_ind;

	map_L_Data_to_N_Data(&N_Data_ind, L_Data_ind);
	
	if (L_Data_ind->daf == KNX_GROUP_ADDRESS)
	{
		if (L_Data_ind->source_addres == N_get_ownAddress(parent_Hndl))
		{
			if (N_layer->N_Data_Group_con_cb)
				N_layer->N_Data_Group_con_cb(N_layer->parent_Hndl, &N_Data_ind);
		}
		else
		{
			if (N_layer->N_Data_Group_ind_cb)
				N_layer->N_Data_Group_ind_cb(N_layer->parent_Hndl, &N_Data_ind);			
		}
	}
	else if (L_Data_ind->daf == KNX_PHYSICAL_ADDRESS)
	{
		if (N_layer->N_Data_Individual_ind_cb)
			N_layer->N_Data_Individual_ind_cb(N_layer->parent_Hndl, &N_Data_ind);
	}
}

void L_SystemBroadcast_ind_cb(void *parent_Hndl, L_Data_s *L_Data_ind) {
	N_layerObj *N_layer;
	N_layer = parent_Hndl;
	
	N_Data_s N_Data_ind;
	
	map_L_Data_to_N_Data(&N_Data_ind, L_Data_ind);
		
	if (N_layer->N_Data_Broadcast_ind_cb)
		N_layer->N_Data_Broadcast_ind_cb(N_layer->parent_Hndl, &N_Data_ind);
}

void L_Data_con_cb(void *parent_Hndl, L_Data_s *L_Data_con) {
	N_layerObj *N_layer;
	N_layer = parent_Hndl;
	
	N_Data_s N_Data_con;

	map_L_Data_to_N_Data(&N_Data_con, L_Data_con);	
	
	if (L_Data_con->daf == KNX_GROUP_ADDRESS)
	{
/*
		if (N_layer->N_Data_Group_con_cb)
			N_layer->N_Data_Group_con_cb(N_layer->parent_Hndl, &N_Data_con);*/
	}
	else if (L_Data_con->daf == KNX_PHYSICAL_ADDRESS)
	{
		if (N_layer->N_Data_Individual_con_cb)
			N_layer->N_Data_Individual_con_cb(N_layer->parent_Hndl, &N_Data_con);
	}
}

void L_SystemBroadcast_con_cb(void *parent_Hndl, L_Data_s *L_Data_con) {
	N_layerObj *N_layer;
	N_layer = parent_Hndl;
	
	N_Data_s N_Data_con;
	
	map_L_Data_to_N_Data(&N_Data_con, L_Data_con);
		
	if (N_layer->N_Data_Broadcast_con_cb)
		N_layer->N_Data_Broadcast_con_cb(N_layer->parent_Hndl, &N_Data_con);
}

int8_t map_L_Data_to_N_Data(N_Data_s *N_Data, L_Data_s *L_Data) {
	
	N_Data->source_address = L_Data->source_addres;
	N_Data->destination_address = L_Data->destination_address;
	N_Data->priority = L_Data->priority;
	N_Data->octet_count = L_Data->octet_count;
	N_Data->n_status = L_Data->l_status;
	N_Data->ack_request = L_Data->ack_request;
	N_Data->nsdu = L_Data->lsdu;
	if (L_Data->frame_format & 0x80) // extended frame
		N_Data->hop_count_type = (*(L_Data->lsdu - 6) & 0x70) >> 4;
	else
		N_Data->hop_count_type = (*(L_Data->lsdu - 1) & 0x70) >> 4;	
	return 0;
}

int8_t map_N_Data_to_L_Data(N_layerObj *N_layer, L_Data_s *L_Data, N_Data_s *N_Data) {
	L_Data->ack_request = N_Data->ack_request;
	L_Data->l_status = N_Data->n_status;
	//L_Data->source_addres = N_Data->source_address;
	L_Data->source_addres = *N_layer->ownAddress;	
	L_Data->destination_address = N_Data->destination_address;
	L_Data->daf = KNX_GROUP_ADDRESS;
	L_Data->priority = N_Data->priority;
	L_Data->octet_count = N_Data->octet_count;
	L_Data->lsdu = N_Data->nsdu;
	uint8_t hopCnt = (*(N_layer->hopCnt) > 0) ? (*(N_layer->hopCnt) >> 4): HOP_COUNT_DEFAULT;
	L_Data->hop_count =(N_Data->hop_count_type == 7) ? 7 : hopCnt;
	if (N_Data->octet_count < 16)
		L_Data->frame_format = 0x00;
	else
		L_Data->frame_format = 0x80;	
	return 0;
}
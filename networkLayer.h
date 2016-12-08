/*
 * networkLayer.h
 *
 * Created: 25/08/16 15:17:08
 *  Author: PvR
 */ 


#ifndef NETWORKLAYER_H_
#define NETWORKLAYER_H_

#include "linkLayer.h"

#define HOP_COUNT_DEFAULT		6

typedef enum {
	N_CON_NOT_OK,
	N_CON_OK
} n_status_e;

typedef struct {
	uint8_t			ack_request;
	uint8_t			hop_count_type;
	uint16_t		source_address;
	uint16_t		destination_address;
	uint8_t			*nsdu;
	uint8_t			octet_count;
	priority_e		priority;
	n_status_e		n_status;
	} N_Data_s;

typedef struct {
	uint8_t				enable;
	uint16_t			*ownAddress;
	uint8_t				*hopCnt;
	void				*parent_Hndl;
	tranceiverObj		tranceiver;
	tranceiverHandle	tranceiver_handle;

	void				(*N_Data_Individual_ind_cb)(void *parent_Hndl, N_Data_s *N_Data_ind);
	void				(*N_Data_Group_ind_cb)(void *parent_Hndl, N_Data_s *N_Data_ind);
	void				(*N_Data_Broadcast_ind_cb)(void *parent_Hndl, N_Data_s *N_Data_ind);
	void				(*N_Data_SystemBroadcast_ind_cb)(void *parent_Hndl, N_Data_s *N_Data_ind);
	
	void				(*N_Data_Individual_con_cb)(void *parent_Hndl, N_Data_s *N_Data_con);
	void				(*N_Data_Group_con_cb)(void *parent_Hndl, N_Data_s *N_Data_con);
	void				(*N_Data_Broadcast_con_cb)(void *parent_Hndl, N_Data_s *N_Data_con);
	void				(*N_Data_SystemBroadcast_con_cb)(void *parent_Hndl, N_Data_s *N_Data_con);
	void				*ownAddressParentHndl;
	void				(*N_Own_Address_Chgd_cb)(void *ownAddressParentHndl, uint16_t ownAddress);
} N_layerObj;

typedef N_layerObj *N_layerHandle;

N_layerHandle N_layerInit(void *pMemory, uint16_t numBytes);

int8_t N_layerSetup(N_layerHandle handle, void *parent_Hndl);

void N_set_ownAddress(N_layerHandle handle, uint16_t *ownAddr);

int8_t N_set_hopCnt(N_layerHandle handle, uint8_t *hopCnt);

uint16_t N_get_ownAddress(N_layerHandle handle);

void N_service(N_layerHandle handle);

void N_Data_Individual_req(N_layerHandle handle, N_Data_s *N_Data_req);

void N_Data_Individual_ack_req(N_layerHandle handle, N_Data_s *N_Data_req);

void N_Data_Group_req(N_layerHandle handle, N_Data_s *N_Data_req);

void N_Data_Broadcast_req(N_layerHandle handle, N_Data_s *N_Data_req);

void N_Data_SystemBroadcast_req(N_layerHandle handle, N_Data_s *N_Data_req);

void L_Data_ind_cb(void *parent_Hndl, L_Data_s *L_Data_ind);

void L_SystemBroadcast_ind_cb(void *parent_Hndl, L_Data_s *L_Data_ind);

void L_Data_con_cb(void *parent_Hndl, L_Data_s *L_Data_con);

void L_SystemBroadcast_con_cb(void *parent_Hndl, L_Data_s *L_Data_con);

int8_t map_L_Data_to_N_Data(N_Data_s *N_Data, L_Data_s *L_Data);

int8_t map_N_Data_to_L_Data(N_layerObj *N_layer, L_Data_s *L_Data, N_Data_s *N_Data);

#endif /* NETWORKLAYER_H_ */
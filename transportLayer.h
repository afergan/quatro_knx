/*
 * transportLayer.h
 *
 * Created: 25/08/16 15:17:56
 *  Author: PvR
 */ 


#ifndef TRANSPORTLAYER_H_
#define TRANSPORTLAYER_H_

#include "networkLayer.h"

#define MAX_TSDU_LENGTH					(64 - 8)

#define CONNECTION_TIMEOUT_MS			6000
#define ACKNOWLEDGEMENT_TIMEOUT_MS		3000
#define MAX_REP_COUNT					3

#define CONNECTED_ACK					0xC2
#define CONNECTED_NEGATIVE_ACK			0xC3

typedef enum {
	T_CON_NOT_OK,
	T_CON_OK
} t_status_e;

typedef enum {
	TL_CONN_CLOSED,
	TL_CONN_OPEN_IDLE,
	TL_CONN_OPEN_WAIT,
	TL_CONN_CONNECTING
} tl_conn_state_e;

typedef enum {
	TL_CONNECTIONLESS,
	TL_CONNECTION_ORIENTED
} tl_conn_type_e;

typedef struct {
	uint8_t				ack_request;
	uint8_t				hop_count_type;
	uint16_t			source_address;
	uint16_t			destination_address;
	tl_conn_type_e		connType;
	uint8_t				*tsdu;
	uint8_t				octet_count;
	priority_e			priority;
	uint16_t			TSAP;
	t_status_e			t_status;
} T_Data_s;

typedef struct {
	// variables according to Chapter 5 of the transport layer system documentation
	tl_conn_state_e		conn_state;
	uint16_t			connection_address;
	uint8_t				SeqNoSend;
	uint8_t				SeqNoRcv;
	uint8_t				rep_count;
	timeoutTmr			connection_timeout_timer;
	timeoutTmr			acknowledgement_timeout_timer;
	T_Data_s			T_Data_store;
	uint8_t				tsdu_store[64];	
} T_session_s;

typedef struct {
	uint8_t				enable;
	void				*parent_Hndl;
	N_layerObj			N_layer;
	N_layerHandle		N_layerHnd;
	T_session_s			session;
	
	uint16_t			*maxTDSULength;
	uint8_t				*maxRetry;

// Transport layer indication call back functions
	void				(*T_Data_Individual_ind_cb)(void *parent_Hndl, T_Data_s *T_Data_ind);
	void				(*T_Data_Group_ind_cb)(void *parent_Hndl, T_Data_s *T_Data_ind);
	void				(*T_Data_Tag_Group_ind_cb)(void *parent_Hndl, T_Data_s *T_Data_ind);
	void				(*T_Data_Broadcast_ind_cb)(void *parent_Hndl, T_Data_s *T_Data_ind);
	void				(*T_Data_systemBroadcast_ind_cb)(void *parent_Hndl, T_Data_s *T_Data_ind);
	void				(*T_Data_Connected_ind_cb)(void *parent_Hndl, T_Data_s *T_Data_ind);
	void				(*T_Connect_ind_cb)(void *parent_Hndl, uint16_t TSAP);
	void				(*T_Disconnect_ind_cb)(void *parent_Hndl, uint16_t TSAP);
	
// Transport layer confirmation call back functions
	void				(*T_Data_Individual_con_cb)(void *parent_Hndl, T_Data_s *T_Data_con);
	void				(*T_Data_Group_con_cb)(void *parent_Hndl, T_Data_s *T_Data_con);
	void				(*T_Data_Tag_Group_con_cb)(void *parent_Hndl, T_Data_s *T_Data_con);
	void				(*T_Data_Broadcast_con_cb)(void *parent_Hndl, T_Data_s *T_Data_con);
	void				(*T_Data_systemBroadcast_con_cb)(void *parent_Hndl, T_Data_s *T_Data_con);
	void				(*T_Data_Connected_con_cb)(void *parent_Hndl, uint16_t TSAP);
	void				(*T_Connect_con_cb)(void *parent_Hndl, uint16_t destination_address, uint16_t TSAP, t_status_e status);
	void				(*T_Disconnect_con_cb)(void *parent_Hndl, uint16_t TSAP, priority_e priority, t_status_e status);
} T_layerObj;

typedef T_layerObj *T_layerHandle;

T_layerHandle T_layerInit(void *pMemory, uint16_t numBytes);

int8_t T_layerSetup(T_layerHandle handle, void *parent_Hndl);

int8_t T_layerSetMaxRetry(T_layerHandle handle, uint8_t *maxRetry);

int8_t T_layerSetMaxTDSULength(T_layerHandle handle, uint16_t *maxTDSULength);

//int8_t T_layerGetMaxTDSULength(T_layerHandle handle);

void T_service(T_layerHandle handle);

void T_Data_acknowlegde_req(T_layerHandle handle, uint8_t ack, uint8_t seqNo);

p2p_e get_individual_message_type(N_Data_s *N_Data_ind);

void T_Data_Individual_req(T_layerHandle handle, T_Data_s *T_Data_req);

void T_Data_Group_req(T_layerHandle handle, T_Data_s *T_Data_req);

void T_Data_Tag_Group_req(T_layerHandle handle, T_Data_s *T_Data_req);

void T_Data_Broadcast_req(T_layerHandle handle, T_Data_s *T_Data_req);

void T_Data_SystemBroadcast_req(T_layerHandle handle, T_Data_s *T_Data_req);

void T_Connect_req(T_layerHandle handle, uint16_t destination_address, priority_e priority);

void T_Disconnect_req(T_layerHandle handle, uint16_t TSAP);

void T_Data_Connected_req(T_layerHandle handle, T_Data_s *T_Data_req);

void N_Data_Individual_ind_cb(void *parent_Hndl, N_Data_s *N_Data_ind);

void N_Data_Group_ind_cb(void *parent_Hndl, N_Data_s *N_Data_ind);

void N_Data_Broadcast_ind_cb(void *parent_Hndl, N_Data_s *N_Data_ind);

void N_Data_SystemBroadcast_ind_cb(void *parent_Hndl, N_Data_s *N_Data_ind);

void N_Data_Individual_con_cb(void *parent_Hndl, N_Data_s *N_Data_con);

void N_Data_Group_con_cb(void *parent_Hndl, N_Data_s *N_Data_con);

void N_Data_Broadcast_con_cb(void *parent_Hndl, N_Data_s *N_Data_con);

void N_Data_SystemBroadcast_con_cb(void *parent_Hndl, N_Data_s *N_Data_con);

int8_t map_N_Data_to_T_Data(T_Data_s *T_Data, N_Data_s *N_Data);

#endif /* TRANSPORTLAYER_H_ */
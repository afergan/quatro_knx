/*
 * transportLayer.c
 *
 * Created: 25/08/16 15:17:26
 *  Author: PvR
 */ 

#include "transportLayer.h"
#include "applicationInterface.h"


T_layerHandle T_layerInit(void *pMemory, uint16_t numBytes)
{
	T_layerHandle T_layer_handle;
	if (numBytes < sizeof(N_layerObj))
	return ((T_layerHandle)NULL);
	T_layer_handle = (T_layerHandle)pMemory;
	return(T_layer_handle);
}

int8_t T_layerSetup(T_layerHandle handle, void *parent_Hndl) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) handle;
	
	T_layer->N_layerHnd = N_layerInit(&T_layer->N_layer, sizeof(T_layer->N_layer));
	N_layerSetup(T_layer->N_layerHnd, T_layer);
	
	// register all indication call back functions
	T_layer->N_layer.N_Data_Group_ind_cb = &N_Data_Group_ind_cb;
	T_layer->N_layer.N_Data_Individual_ind_cb = &N_Data_Individual_ind_cb;
	T_layer->N_layer.N_Data_Broadcast_ind_cb = &N_Data_Broadcast_ind_cb;
	T_layer->N_layer.N_Data_SystemBroadcast_ind_cb = &N_Data_SystemBroadcast_ind_cb;
	
		// register all indication call back functions
	T_layer->N_layer.N_Data_Group_con_cb = &N_Data_Group_con_cb;
	T_layer->N_layer.N_Data_Individual_con_cb = &N_Data_Individual_con_cb;
	T_layer->N_layer.N_Data_Broadcast_con_cb = &N_Data_Broadcast_con_cb;
	T_layer->N_layer.N_Data_SystemBroadcast_con_cb = &N_Data_SystemBroadcast_con_cb;

	T_layer->parent_Hndl = parent_Hndl;
	*T_layer->maxRetry = 3;
	*T_layer->maxTDSULength = 15;
	T_layer->enable = 1;
	return 0;
}

int8_t T_layerSetMaxRetry(T_layerHandle handle, uint8_t *maxRetry) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) handle;
	//if (maxRetry > 8)
	//	return -1;
	T_layer->maxRetry = maxRetry;
	return 0;
}

int8_t T_layerSetMaxTDSULength(T_layerHandle handle, uint16_t *maxTDSULength) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) handle;
	
	//if (maxTDSULength < 15 || maxTDSULength > MAX_TSDU_LENGTH)
	//	return -1;
	T_layer->maxTDSULength = maxTDSULength;
	return 0;
}

void T_service(T_layerHandle handle) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) handle;
	if (!T_layer->enable)
		return;
	if (T_layer->session.conn_state == TL_CONN_OPEN_IDLE || T_layer->session.conn_state == TL_CONN_OPEN_WAIT)
	{
		if (isTimedout(&T_layer->session.connection_timeout_timer))
		{
			// E16 --> A6
			T_Disconnect_req(T_layer, T_layer->session.connection_address);
			if (T_layer->T_Disconnect_ind_cb)
				T_layer->T_Disconnect_ind_cb(T_layer->parent_Hndl, T_layer->session.connection_address);	
		}
		
		if (T_layer->session.conn_state == TL_CONN_OPEN_WAIT && isTimedout(&T_layer->session.acknowledgement_timeout_timer))
		{
			if (T_layer->session.rep_count < *T_layer->maxRetry)
			{
				// E17 --> A9
				N_Data_s N_Data_req;
				N_Data_req.ack_request = 0;
				N_Data_req.n_status = N_CON_OK;
				N_Data_req.priority =T_layer->session.T_Data_store.priority;
				N_Data_req.destination_address = T_layer->session.connection_address;
				N_Data_req.hop_count_type = T_layer->session.T_Data_store.hop_count_type;
				N_Data_req.octet_count = T_layer->session.T_Data_store.octet_count;
				N_Data_req.nsdu = T_layer->session.tsdu_store;
				N_Data_req.nsdu[0] |= (0x40 | ((T_layer->session.SeqNoSend & 0xf) << 2));
			
				N_Data_Individual_req(T_layer->N_layerHnd, &N_Data_req);

				setupTimeoutTmr(&T_layer->session.acknowledgement_timeout_timer, ACKNOWLEDGEMENT_TIMEOUT_MS);
				setupTimeoutTmr(&T_layer->session.connection_timeout_timer, CONNECTION_TIMEOUT_MS);

				T_layer->session.rep_count++;				
			}
			else
			{
				// E18 --> A6
				T_Disconnect_req(T_layer, T_layer->session.connection_address);
				if (T_layer->T_Disconnect_ind_cb)
					T_layer->T_Disconnect_ind_cb(T_layer->parent_Hndl, T_layer->session.connection_address);			
			}

		}
	}
		
	N_service(T_layer->N_layerHnd);
}

p2p_e get_individual_message_type(N_Data_s *N_Data_ind) {
		
	p2p_e P2P_controlField = 0;
	if ((*(N_Data_ind->nsdu) &0xFC) == 0x00)
		P2P_controlField = P2P_CONNECTIONLESS;
	else if ((*(N_Data_ind->nsdu) &0xC0) == 0x40)
		P2P_controlField = P2P_CONNECTED;
	else if (*(N_Data_ind->nsdu) == 0x80)
		P2P_controlField = P2P_CONNECT_REQ;
	else if (*(N_Data_ind->nsdu) == 0x81)
		P2P_controlField = P2P_DISCONNECT_REQ;
	else if ((*(N_Data_ind->nsdu) & 0xC3) == 0xC2)
		P2P_controlField = ACK_NUMBERED_TELEGRAM;
	else if ((*(N_Data_ind->nsdu) & 0xC3) == 0xC3)
		P2P_controlField = NACK_NUMBERED_TELEGRAM;
		
	return P2P_controlField;
}


void N_Data_Individual_ind_cb(void *parent_Hndl, N_Data_s *N_Data_ind) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) parent_Hndl;
	
	p2p_e P2P_controlField = get_individual_message_type(N_Data_ind);
	
	uint8_t seqNo = (*(N_Data_ind->nsdu) >> 2) & 0x0F;
	
	// connectionless individual indication
	if (P2P_controlField == P2P_CONNECTIONLESS)
	{
		if (T_layer->T_Data_Individual_ind_cb) {
		
			T_Data_s T_Data_ind;
								
			map_N_Data_to_T_Data(&T_Data_ind, N_Data_ind);

			T_Data_ind.TSAP = N_Data_ind->source_address;  // map source to TSAP
			T_Data_ind.connType = TL_CONNECTIONLESS;
			
			T_layer->T_Data_Individual_ind_cb(T_layer->parent_Hndl, &T_Data_ind);
		}
	}
	// Event E00 and E01
	else if (P2P_controlField == P2P_CONNECT_REQ) {
		if (T_layer->session.conn_state == TL_CONN_OPEN_IDLE || T_layer->session.conn_state == TL_CONN_OPEN_WAIT)
		{
			if (N_Data_ind->source_address == T_layer->session.connection_address)
			{
				// A6 terminate connection with source address T_layer->connection_address
				T_Disconnect_req(T_layer, T_layer->session.connection_address);
				if (T_layer->T_Disconnect_ind_cb)
					T_layer->T_Disconnect_ind_cb(T_layer->parent_Hndl, N_Data_ind->source_address);
			}
			else if (N_Data_ind->source_address != T_layer->session.connection_address)
			{
				// A10 terminate with source address from T_layer
				T_layer->session.conn_state = TL_CONN_CLOSED;
				T_Disconnect_req(T_layer, N_Data_ind->source_address);
			}
			T_layer->session.conn_state = TL_CONN_CLOSED;
		}
		else if (T_layer->session.conn_state == TL_CONN_CLOSED)
		{
			// A1 open connection. Send a T_Connect to the user
			T_layer->session.connection_address = N_Data_ind->source_address;
			T_layer->session.SeqNoRcv = 0;
			T_layer->session.SeqNoSend = 0;
			T_layer->session.conn_state = TL_CONN_OPEN_IDLE;
			setupTimeoutTmr(&T_layer->session.connection_timeout_timer, CONNECTION_TIMEOUT_MS);
			if (T_layer->T_Connect_ind_cb)
					T_layer->T_Connect_ind_cb(T_layer->parent_Hndl, N_Data_ind->source_address);
			
		}
	}
	// Event E02 and E03
	else if (P2P_controlField == P2P_DISCONNECT_REQ) {
		if (N_Data_ind->source_address == T_layer->session.connection_address && (T_layer->session.conn_state == TL_CONN_OPEN_IDLE || T_layer->session.conn_state == TL_CONN_OPEN_WAIT)) {
			// A5 send disconnect to the user
			T_layer->session.conn_state = TL_CONN_CLOSED;
			if (T_layer->T_Disconnect_ind_cb)
				T_layer->T_Disconnect_ind_cb(T_layer->parent_Hndl, N_Data_ind->source_address);
		}
	}
	// Event E04, E05, E06 and E07
	else if (P2P_controlField == P2P_CONNECTED) {
		if (T_layer->session.conn_state == TL_CONN_CLOSED) {
			// A10 terminate with source address from T_layer
			T_Disconnect_req(T_layer, N_Data_ind->source_address);
		}
		else if (T_layer->session.conn_state == TL_CONN_OPEN_IDLE || T_layer->session.conn_state == TL_CONN_OPEN_WAIT)
		{
			if (N_Data_ind->source_address == T_layer->session.connection_address)
			{
				if (seqNo == T_layer->session.SeqNoRcv)
				{
					// E04 --> A2 send acknowledge to the network layer and send receive buffer to application layer
					setupTimeoutTmr(&T_layer->session.connection_timeout_timer, CONNECTION_TIMEOUT_MS);
					T_Data_acknowlegde_req(parent_Hndl, CONNECTED_ACK, T_layer->session.SeqNoRcv);
					
					T_layer->session.SeqNoRcv++;
					T_layer->session.SeqNoRcv &= 0xf;
					
					if (T_layer->T_Data_Connected_ind_cb)
					{
						T_Data_s T_Data_ind;
						
						map_N_Data_to_T_Data(&T_Data_ind, N_Data_ind);

						T_Data_ind.TSAP = N_Data_ind->source_address;  // map source to TSAP
						T_Data_ind.connType = TL_CONNECTION_ORIENTED;
						
						T_layer->T_Data_Connected_ind_cb(T_layer->parent_Hndl, &T_Data_ind);
					}
				}
				else if (seqNo == ((T_layer->session.SeqNoRcv -1) & 0xf))
				{
					// E05 --> A3
					setupTimeoutTmr(&T_layer->session.connection_timeout_timer, CONNECTION_TIMEOUT_MS);
					T_Data_acknowlegde_req(parent_Hndl, CONNECTED_ACK, seqNo);
				}
				else
				{
					// E06 --> A4
					setupTimeoutTmr(&T_layer->session.connection_timeout_timer, CONNECTION_TIMEOUT_MS);
					T_Data_acknowlegde_req(parent_Hndl, CONNECTED_NEGATIVE_ACK, seqNo);
				}
			}
			else
			{
				// E07 --> A10
				T_layer->session.conn_state = TL_CONN_CLOSED;
				T_Disconnect_req(T_layer, N_Data_ind->source_address);
			}
		}
	}
	// Event E08, E09 and E10
	else if (P2P_controlField == ACK_NUMBERED_TELEGRAM)
	{
		if (N_Data_ind->source_address == T_layer->session.connection_address)
		{
			if (seqNo == T_layer->session.SeqNoSend)
			{
				// E08
				if (T_layer->session.conn_state == TL_CONN_OPEN_IDLE)
				{
					// A6
					T_Disconnect_req(T_layer, T_layer->session.connection_address);
					if (T_layer->T_Disconnect_ind_cb)
						T_layer->T_Disconnect_ind_cb(T_layer->parent_Hndl, N_Data_ind->source_address);
				}
				else if (T_layer->session.conn_state == TL_CONN_OPEN_WAIT)
				{
					// A8
					T_layer->session.SeqNoSend++;
					T_layer->session.SeqNoSend &= 0xf;
					T_layer->session.conn_state = TL_CONN_OPEN_IDLE;
					//T_layer->session.connection_timeout_timer = clock.timestamp + CONNECTION_TIMEOUT_MS;
					setupTimeoutTmr(&T_layer->session.connection_timeout_timer, CONNECTION_TIMEOUT_MS);
				}
			}
			else
			{
				// E09 --> A6
				T_Disconnect_req(T_layer, T_layer->session.connection_address);
				if (T_layer->T_Disconnect_ind_cb)
					T_layer->T_Disconnect_ind_cb(T_layer->parent_Hndl, N_Data_ind->source_address);
			}
		}
		else
		{
			// E10 --> A10
			T_layer->session.conn_state = TL_CONN_CLOSED;
			T_Disconnect_req(T_layer, N_Data_ind->source_address);
		}
	}
	// Event E11, E12, E13 and E14
	else if (P2P_controlField == NACK_NUMBERED_TELEGRAM)
	{
		if (N_Data_ind->source_address == T_layer->session.connection_address)
		{
			if (seqNo == T_layer->session.SeqNoSend)
			{
				if (T_layer->session.rep_count < *T_layer->maxRetry)
				{
					// E12
					if (T_layer->session.conn_state == TL_CONN_OPEN_IDLE)
					{
						// A6
						T_Disconnect_req(T_layer, T_layer->session.connection_address);
						if (T_layer->T_Disconnect_ind_cb)
							T_layer->T_Disconnect_ind_cb(T_layer->parent_Hndl, N_Data_ind->source_address);
					}
					else if (T_layer->session.conn_state == TL_CONN_OPEN_WAIT)
					{
						// A9 Send stored message as a N_Data_individual_req

						N_Data_s N_Data_req;
						N_Data_req.ack_request = 0;
						N_Data_req.n_status = N_CON_OK;
						N_Data_req.priority =T_layer->session.T_Data_store.priority;
						N_Data_req.destination_address = T_layer->session.connection_address;
						N_Data_req.hop_count_type = T_layer->session.T_Data_store.hop_count_type;
						N_Data_req.octet_count = T_layer->session.T_Data_store.octet_count;
						N_Data_req.nsdu = T_layer->session.tsdu_store;
						N_Data_req.nsdu[0] |= (0x40 | ((T_layer->session.SeqNoSend & 0xf) << 2));
	
						N_Data_Individual_req(T_layer->N_layerHnd, &N_Data_req);
	
						//T_layer->session.acknowledgement_timeout_timer = clock.timestamp + ACKNOWLEDGEMENT_TIMEOUT_MS;
						//T_layer->session.connection_timeout_timer = clock.timestamp + CONNECTION_TIMEOUT_MS;
						setupTimeoutTmr(&T_layer->session.acknowledgement_timeout_timer, ACKNOWLEDGEMENT_TIMEOUT_MS);
						setupTimeoutTmr(&T_layer->session.connection_timeout_timer, CONNECTION_TIMEOUT_MS);
						
						T_layer->session.rep_count++;

					}
				}
				else
				{
					// E13 --> A6
					T_Disconnect_req(T_layer, T_layer->session.connection_address);
					if (T_layer->T_Disconnect_ind_cb)
						T_layer->T_Disconnect_ind_cb(T_layer->parent_Hndl, N_Data_ind->source_address);
				}
			}
			else
			{
				// E11 --> A6
				T_Disconnect_req(T_layer, T_layer->session.connection_address);
				if (T_layer->T_Disconnect_ind_cb)
						T_layer->T_Disconnect_ind_cb(T_layer->parent_Hndl, N_Data_ind->source_address);
			}
		}
		else
		{
			// E14 --> A10
			T_layer->session.conn_state = TL_CONN_CLOSED;
			T_Disconnect_req(T_layer, N_Data_ind->source_address);
		}
	}
}

void T_Data_acknowlegde_req(T_layerHandle handle, uint8_t ack, uint8_t seqNo) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) handle;
	
	N_Data_s N_Data_req;
	N_Data_req.ack_request = 0;
	N_Data_req.n_status = N_CON_OK;
	N_Data_req.destination_address = T_layer->session.connection_address;
	N_Data_req.priority = KNX_SYSTEM;
	N_Data_req.hop_count_type = 6;
	N_Data_req.octet_count = 0;
	N_Data_req.nsdu = (uint8_t[]) { (0xC3 & ack) | ((seqNo & 0xf) << 2)};
					
	N_Data_Individual_ack_req(T_layer->N_layerHnd, &N_Data_req); // send acknowledge or negative acknowledge
}

void T_Data_Individual_req(T_layerHandle handle, T_Data_s *T_Data_req) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) handle;
	
	N_Data_s N_Data_req;
	N_Data_req.ack_request = T_Data_req->ack_request;
	N_Data_req.n_status = T_Data_req->t_status;		
	N_Data_req.priority = T_Data_req->priority;
	N_Data_req.destination_address = T_Data_req->TSAP;
	N_Data_req.hop_count_type = T_Data_req->hop_count_type;
	N_Data_req.octet_count = T_Data_req->octet_count;
	N_Data_req.nsdu = T_Data_req->tsdu;
		
		
	N_Data_Individual_req(T_layer->N_layerHnd, &N_Data_req);
}

void T_Data_Group_req(T_layerHandle handle, T_Data_s *T_Data_req) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) handle;
	
	N_Data_s N_Data_req;
	N_Data_req.ack_request = T_Data_req->ack_request;
	N_Data_req.n_status = T_Data_req->t_status;	
	N_Data_req.priority = T_Data_req->priority;
	
	N_Data_req.destination_address = GrpSrv_getGroupAddrByAddrTblIdx(((A_InterfaceHandle)((A_LayerHandle)T_layer->parent_Hndl)->parent_Hndl)->GO_Hndl, T_Data_req->TSAP);
	
	N_Data_req.hop_count_type = T_Data_req->hop_count_type;
	N_Data_req.octet_count = T_Data_req->octet_count;
	N_Data_req.nsdu = T_Data_req->tsdu;
	
	N_Data_Group_req(T_layer->N_layerHnd, &N_Data_req);
}

void T_Data_Broadcast_req(T_layerHandle handle, T_Data_s *T_Data_req) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) handle;

	N_Data_s N_Data_req;
	N_Data_req.ack_request = T_Data_req->ack_request;
	N_Data_req.n_status = T_Data_req->t_status;
	N_Data_req.priority = T_Data_req->priority;
	N_Data_req.destination_address = 0;
	N_Data_req.hop_count_type = T_Data_req->hop_count_type;
	N_Data_req.octet_count = T_Data_req->octet_count;
	N_Data_req.nsdu = T_Data_req->tsdu;	
	
	N_Data_Broadcast_req(T_layer->N_layerHnd, &N_Data_req);
}

void T_Data_SystemBroadcast_req(T_layerHandle handle, T_Data_s *T_Data_req) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) handle;
	
	N_Data_s N_Data_req;
	N_Data_req.ack_request = T_Data_req->ack_request;
	N_Data_req.n_status = T_Data_req->t_status;
	N_Data_req.priority = T_Data_req->priority;
	N_Data_req.destination_address = 0;
	N_Data_req.hop_count_type = T_Data_req->hop_count_type;
	N_Data_req.octet_count = T_Data_req->octet_count;
	N_Data_req.nsdu = T_Data_req->tsdu;
	
	N_Data_SystemBroadcast_req(T_layer->N_layerHnd, &N_Data_req);
}

void T_Connect_req(T_layerHandle handle, uint16_t destination_address, priority_e priority) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) handle;
	
	if (T_layer->session.conn_state == TL_CONN_OPEN_IDLE || T_layer->session.conn_state == TL_CONN_OPEN_WAIT)
	{
		// A6
		T_Disconnect_req(T_layer, T_layer->session.connection_address);
		if (T_layer->T_Disconnect_ind_cb)
			T_layer->T_Disconnect_ind_cb(T_layer->parent_Hndl, T_layer->session.connection_address);
	}
	else if (T_layer->session.conn_state == TL_CONN_CLOSED)
	{
		N_Data_s N_Data_req;
		N_Data_req.ack_request = 0;
		N_Data_req.n_status = N_CON_OK;
		N_Data_req.destination_address = destination_address;
		N_Data_req.priority = priority;
		N_Data_req.hop_count_type = 6;
		N_Data_req.octet_count = 0;
		uint8_t		nsdu[] = { 0x80};
		N_Data_req.nsdu = nsdu;
		
		N_Data_Individual_req(T_layer->N_layerHnd, &N_Data_req); // open P2P connection
		
		T_layer->session.conn_state = TL_CONN_OPEN_IDLE;	
	}
	

}

void T_Disconnect_req(T_layerHandle handle, uint16_t TSAP) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) handle;
	
	if (T_layer->session.conn_state != TL_CONN_CLOSED)
	{
		N_Data_s N_Data_req;
		N_Data_req.ack_request = 0;
		N_Data_req.n_status = N_CON_OK;
		N_Data_req.destination_address = TSAP;
		N_Data_req.priority = KNX_SYSTEM;
		N_Data_req.hop_count_type = 6;
		N_Data_req.octet_count = 0;
		uint8_t		nsdu[] = { 0x81};
		N_Data_req.nsdu = nsdu;
		
		N_Data_Individual_req(T_layer->N_layerHnd, &N_Data_req);	
	}
	
	T_layer->session.conn_state = TL_CONN_CLOSED;
	if (T_layer->T_Disconnect_ind_cb)
		T_layer->T_Disconnect_ind_cb(T_layer->parent_Hndl, TSAP);
}

void T_Data_Connected_req(T_layerHandle handle, T_Data_s *T_Data_req) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) handle;
	
	if (T_layer->session.conn_state == TL_CONN_OPEN_WAIT)
	{
		// A6
		T_Disconnect_req(T_layer, T_layer->session.connection_address);
	}
	if (T_layer->session.conn_state == TL_CONN_CLOSED)
	{
		if (T_layer->T_Disconnect_ind_cb)
			T_layer->T_Disconnect_ind_cb(T_layer->parent_Hndl, T_Data_req->TSAP);
		return;
	}
	
	// A7
	// store T_Data_Connected_req for possible resending after failure
	
	T_layer->session.T_Data_store.ack_request = T_Data_req->ack_request;
	T_layer->session.T_Data_store.destination_address = T_Data_req->destination_address;
	T_layer->session.T_Data_store.hop_count_type = T_Data_req->hop_count_type;
	T_layer->session.T_Data_store.octet_count = T_Data_req->octet_count;
	T_layer->session.T_Data_store.priority = T_Data_req->priority;
	T_layer->session.T_Data_store.source_address = T_Data_req->source_address;
	T_layer->session.T_Data_store.t_status = T_Data_req->t_status;
	T_layer->session.T_Data_store.TSAP = T_Data_req->TSAP;
	for (uint8_t i = 0; i < T_Data_req->octet_count + 1; i++)
	{
		*(T_layer->session.tsdu_store + i) = *(T_Data_req->tsdu + i);
	}
	
	
	N_Data_s N_Data_req;
	N_Data_req.ack_request = T_Data_req->ack_request;
	N_Data_req.n_status = T_Data_req->t_status;
	N_Data_req.priority = T_Data_req->priority;
	N_Data_req.destination_address = T_layer->session.connection_address;
	N_Data_req.hop_count_type = T_Data_req->hop_count_type;
	N_Data_req.octet_count = T_Data_req->octet_count;
	N_Data_req.nsdu = T_Data_req->tsdu;
	N_Data_req.nsdu[0] |= (0x40 | ((T_layer->session.SeqNoSend & 0xf) << 2));
	
	N_Data_Individual_req(T_layer->N_layerHnd, &N_Data_req);
	
	setupTimeoutTmr(&T_layer->session.acknowledgement_timeout_timer, ACKNOWLEDGEMENT_TIMEOUT_MS);
	setupTimeoutTmr(&T_layer->session.connection_timeout_timer, CONNECTION_TIMEOUT_MS);
	T_layer->session.rep_count = 0;
	T_layer->session.conn_state = TL_CONN_OPEN_WAIT;
}


void N_Data_Group_ind_cb(void *parent_Hndl, N_Data_s *N_Data_ind) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) parent_Hndl;	
	
	T_Data_s T_Data_ind;
	
	map_N_Data_to_T_Data(&T_Data_ind, N_Data_ind);
	
	// map group address to TSAP
	
	T_Data_ind.TSAP = GrpSrv_getAddrTblIdxByGroupAddress(((A_InterfaceHandle)((A_LayerHandle)T_layer->parent_Hndl)->parent_Hndl)->GO_Hndl, N_Data_ind->destination_address);

	if ((*(N_Data_ind->nsdu) & 0xFC) == 0x04)
	{
		// T_Data_Tag_Group_PDU
		if (T_layer->T_Data_Tag_Group_ind_cb)
			T_layer->T_Data_Tag_Group_ind_cb(T_layer->parent_Hndl, &T_Data_ind);
	}
	else
	{
		if (T_layer->T_Data_Group_ind_cb)
			T_layer->T_Data_Group_ind_cb(T_layer->parent_Hndl, &T_Data_ind);
	}
	
}

void N_Data_Broadcast_ind_cb(void *parent_Hndl, N_Data_s *N_Data_ind) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) parent_Hndl;
	
	T_Data_s T_Data_ind;
	
	map_N_Data_to_T_Data(&T_Data_ind, N_Data_ind);
	
	if (T_layer->T_Data_Broadcast_ind_cb)
		T_layer->T_Data_Broadcast_ind_cb(T_layer->parent_Hndl, &T_Data_ind);
	
}

void N_Data_SystemBroadcast_ind_cb(void *parent_Hndl, N_Data_s *N_Data_ind) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) parent_Hndl;	
	
	T_Data_s T_Data_ind;
	
	map_N_Data_to_T_Data(&T_Data_ind, N_Data_ind);
	
	if (T_layer->T_Data_systemBroadcast_ind_cb)
		T_layer->T_Data_systemBroadcast_ind_cb(T_layer->parent_Hndl, &T_Data_ind);	
		
}

void N_Data_Individual_con_cb(void *parent_Hndl, N_Data_s *N_Data_con) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) parent_Hndl;

	T_Data_s T_Data_con;
	
	map_N_Data_to_T_Data(&T_Data_con, N_Data_con);
	
	p2p_e P2P_controlField = get_individual_message_type(N_Data_con);

	if (P2P_controlField == P2P_CONNECT_REQ) {
		if (T_layer->T_Connect_con_cb)
			T_layer->T_Connect_con_cb(T_layer->parent_Hndl, T_Data_con.destination_address, T_Data_con.TSAP, T_Data_con.t_status);
		
	}
	else if (P2P_controlField == P2P_DISCONNECT_REQ) {
		if (T_layer->T_Disconnect_con_cb)
			T_layer->T_Disconnect_con_cb(T_layer->parent_Hndl, T_Data_con.TSAP, T_Data_con.priority, T_Data_con.t_status);
		
	}
	else if (P2P_controlField == P2P_CONNECTED) {
		if (T_layer->T_Data_Connected_con_cb)
			T_layer->T_Data_Connected_con_cb(T_layer->parent_Hndl, T_Data_con.TSAP);
	}
}

void N_Data_Group_con_cb(void *parent_Hndl, N_Data_s *N_Data_con) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) parent_Hndl;	
	
	T_Data_s T_Data_con;
	
	map_N_Data_to_T_Data(&T_Data_con, N_Data_con);	
	
	// map group address to TSAP
		
	T_Data_con.TSAP = GrpSrv_getAddrTblIdxByGroupAddress(((A_InterfaceHandle)((A_LayerHandle)T_layer->parent_Hndl)->parent_Hndl)->GO_Hndl, N_Data_con->destination_address);
		
	if ((*(N_Data_con->nsdu) & 0xFC) == 0x04)
	{
		// T_Data_Tag_Group_PDU
		if (T_layer->T_Data_Tag_Group_con_cb)
			T_layer->T_Data_Tag_Group_con_cb(T_layer->parent_Hndl, &T_Data_con);
	}
	else
	{
		if (T_layer->T_Data_Group_con_cb)
			T_layer->T_Data_Group_con_cb(T_layer->parent_Hndl, &T_Data_con);
	}
}

void N_Data_Broadcast_con_cb(void *parent_Hndl, N_Data_s *N_Data_con) {
		T_layerObj *T_layer;
	T_layer = (T_layerObj *) parent_Hndl;	
	
	T_Data_s T_Data_con;
	
	map_N_Data_to_T_Data(&T_Data_con, N_Data_con);
	
	if (T_layer->T_Data_Broadcast_con_cb)
		T_layer->T_Data_Broadcast_con_cb(T_layer->parent_Hndl, &T_Data_con);
}

void N_Data_SystemBroadcast_con_cb(void *parent_Hndl, N_Data_s *N_Data_con) {
	T_layerObj *T_layer;
	T_layer = (T_layerObj *) parent_Hndl;	
	
	T_Data_s T_Data_con;
	
	map_N_Data_to_T_Data(&T_Data_con, N_Data_con);
	
	if (T_layer->T_Data_systemBroadcast_con_cb)
		T_layer->T_Data_systemBroadcast_con_cb(T_layer->parent_Hndl, &T_Data_con);	
}

int8_t map_N_Data_to_T_Data(T_Data_s *T_Data, N_Data_s *N_Data) {
	T_Data->source_address = N_Data->source_address;
	T_Data->TSAP = N_Data->destination_address;
	T_Data->destination_address = N_Data->destination_address;
	T_Data->priority = N_Data->priority;
	T_Data->octet_count = N_Data->octet_count;
	T_Data->tsdu = N_Data->nsdu;
	T_Data->hop_count_type = N_Data->hop_count_type;
	T_Data->t_status = N_Data->n_status;
	T_Data->ack_request = N_Data->ack_request;
	return 0;
}
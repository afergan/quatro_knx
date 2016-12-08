/*
 * applicationLayer.c
 *
 * Created: 25/08/16 15:18:29
 *  Author: PvR
 */ 

#include "applicationLayer.h"
#include "applicationInterface.h"
#include "interfaceObjectServer.h"
#include "groupObjectServer.h"

#include "console.h"
#include <string.h>
#include <stdio.h>

A_LayerHandle A_LayerInit(void *pMemory, uint16_t numBytes) {
	A_LayerHandle A_Layer_handle;
	if (numBytes < sizeof(A_LayerObj))
		return ((A_LayerHandle)NULL);
	A_Layer_handle = (A_LayerHandle)pMemory;
	return(A_Layer_handle);
}

int8_t A_LayerSetup(A_LayerHandle handle, groupObjectServerHandle grpSrv_handle, void *parent_Hndl) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	A_Layer->parent_Hndl = parent_Hndl;
	A_Layer->grpSrv_handle = grpSrv_handle;
	
	A_Layer->T_layerHnd = T_layerInit(&A_Layer->T_layer, sizeof(A_Layer->T_layer));
	T_layerSetup(A_Layer->T_layerHnd, A_Layer);

	// register all indication call back functions
	
	A_Layer->T_layer.T_Data_Broadcast_ind_cb = &T_Data_Broadcast_ind_cb;
	A_Layer->T_layer.T_Data_systemBroadcast_ind_cb = &T_Data_systemBroadcast_ind_cb;
	A_Layer->T_layer.T_Data_Individual_ind_cb = &T_Data_Individual_ind_cb;
	A_Layer->T_layer.T_Data_Connected_ind_cb = &T_Data_Connected_ind_cb;
	A_Layer->T_layer.T_Data_Group_ind_cb = &T_Data_Group_ind_cb;
	A_Layer->T_layer.T_Data_Tag_Group_ind_cb = &T_Data_Tag_Group_ind_cb;
	A_Layer->T_layer.T_Connect_ind_cb = &T_Connect_ind_cb;
	A_Layer->T_layer.T_Disconnect_ind_cb = &T_Disconnect_ind_cb;

	// register all confirmation call back functions
	
	A_Layer->T_layer.T_Data_Broadcast_con_cb = &T_Data_Broadcast_con_cb;
	A_Layer->T_layer.T_Data_systemBroadcast_con_cb = &T_Data_systemBroadcast_con_cb;
	A_Layer->T_layer.T_Data_Individual_con_cb = &T_Data_Individual_con_cb;
	A_Layer->T_layer.T_Data_Connected_con_cb = &T_Data_Connected_con_cb;
	A_Layer->T_layer.T_Data_Group_con_cb = &T_Data_Group_con_cb;
	A_Layer->T_layer.T_Data_Tag_Group_con_cb = &T_Data_Tag_Group_con_cb;
	A_Layer->T_layer.T_Connect_con_cb = &T_Connect_con_cb;
	A_Layer->T_layer.T_Disconnect_con_cb = &T_Disconnect_con_cb;
	
	// register group object server call backs
	
	
	
	A_Layer->enable = 1;
	return 0;
}

// Restart functions

void A_Restart_req(A_LayerHandle handle, A_Restart_s *A_Restart_req);

void A_Restart_res(A_LayerHandle handle, A_Restart_s *A_Restart_res) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
		return;
	
	T_Data_s T_Data_res;
	
	T_Data_res.ack_request = A_Restart_res->ack_request;
	T_Data_res.t_status = A_Restart_res->a_status;
	T_Data_res.hop_count_type = A_Restart_res->hop_count_type;
	T_Data_res.priority = A_Restart_res->priority;
	T_Data_res.connType = A_Restart_res->connType;
	T_Data_res.TSAP = A_Restart_res->ASAP;
	
	uint8_t tsdu[16];
	T_Data_res.tsdu = tsdu;
	
	*(uint16_t*)tsdu = SWAP_UINT16((uint16_t)(KNX_RESTART << 6) | 0x21); // master reset
	
	tsdu[2] = A_Restart_res->errorCode;
	tsdu[3] = A_Restart_res->processTime >> 8;
	tsdu[4] = A_Restart_res->processTime & 0xff;
	
	T_Data_res.octet_count = 4;

	if (A_Restart_res->connType == TL_CONNECTION_ORIENTED)
		T_Data_Connected_req(A_Layer->T_layerHnd , &T_Data_res);
	else
		T_Data_Individual_req(A_Layer->T_layerHnd , &T_Data_res);	
}

// Memory read / write functions
	
void A_Memory_Read_req(A_LayerHandle handle, A_Memory_s *A_Memory_req);

void A_Memory_Read_res(A_LayerHandle handle, A_Memory_s *A_Memory_res) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
		return;
	
	T_Data_s T_Data_res;

	T_Data_res.ack_request = A_Memory_res->ack_request;
	T_Data_res.t_status = A_Memory_res->a_status;
	T_Data_res.hop_count_type = A_Memory_res->hop_count_type;
	T_Data_res.priority = A_Memory_res->priority;
	T_Data_res.connType = TL_CONNECTION_ORIENTED;
	T_Data_res.TSAP = A_Memory_res->ASAP;
	uint8_t tsdu[16];
	T_Data_res.tsdu = tsdu;
	
	*(uint16_t*)tsdu = SWAP_UINT16((uint16_t)(KNX_MEMORY_RESPONSE << 6));	
	
	tsdu[1] |= (A_Memory_res->number & 0x3f);
	
	tsdu[2] = A_Memory_res->memoryAddress >> 8;
	tsdu[3] = A_Memory_res->memoryAddress & 0xff;
	
	for (uint8_t i = 0; i < A_Memory_res->number; i++)
	{
		*(tsdu + 4 + i) = *(A_Memory_res->data + i);
	}

	T_Data_res.octet_count = 3 + A_Memory_res->number;

	T_Data_Connected_req(A_Layer->T_layerHnd , &T_Data_res);	
}

void A_Memory_Write_req(A_LayerHandle handle, A_Memory_s *A_Memory_req);

void A_Memory_Write_res(A_LayerHandle handle, A_Memory_s *A_Memory_res) {
	
}

// ADC read functions

void A_ADC_Read_req(A_LayerHandle handle, A_ADC_read_s *A_ADC_read_req);

void A_ADC_Read_res(A_LayerHandle handle, A_ADC_read_s *A_ADC_read_res) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
		return;
	
	T_Data_s T_Data_res;

	T_Data_res.ack_request = A_ADC_read_res->ack_request;
	T_Data_res.t_status = A_ADC_read_res->a_status;
	T_Data_res.hop_count_type = A_ADC_read_res->hop_count_type;
	T_Data_res.priority = A_ADC_read_res->priority;
	T_Data_res.connType = TL_CONNECTION_ORIENTED;
	T_Data_res.TSAP = A_ADC_read_res->ASAP;
	uint8_t tsdu[5];
	T_Data_res.tsdu = tsdu;
	
	*(uint16_t*)tsdu = SWAP_UINT16((uint16_t)(KNX_ADC_RESPONSE << 6));
	
	tsdu[1] |= (A_ADC_read_res->channel & 0x3f);
	tsdu[2] = A_ADC_read_res->readCount;
	tsdu[3] = A_ADC_read_res->sum >> 8;
	tsdu[4] = A_ADC_read_res->sum & 0xff;
		
	T_Data_res.octet_count = 4;

	T_Data_Connected_req(A_Layer->T_layerHnd , &T_Data_res);	
}


// authorization functions

void A_Authorize_Request_req(A_LayerHandle handle, A_Authorisation_s *A_Authorize_req);

void A_Authorize_Request_res(A_LayerHandle handle, A_Authorisation_s *A_Authorize_res) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
		return;
	
	T_Data_s T_Data_res;

	T_Data_res.ack_request = A_Authorize_res->ack_request;
	T_Data_res.t_status = A_Authorize_res->a_status;
	T_Data_res.hop_count_type = A_Authorize_res->hop_count_type;
	T_Data_res.priority = A_Authorize_res->priority;
	T_Data_res.connType = TL_CONNECTION_ORIENTED;
	T_Data_res.TSAP = A_Authorize_res->ASAP;
	uint8_t tsdu[3];
	T_Data_res.tsdu = tsdu;
	
	*(uint16_t*)tsdu = SWAP_UINT16((uint16_t)(KNX_ESCAPE << 6) | KNX_ESCAPE_AUTHORIZE_RESPONSE);
	
	tsdu[2] = A_Authorize_res->level;
	
	T_Data_res.octet_count = 2;

	T_Data_Connected_req(A_Layer->T_layerHnd , &T_Data_res);		
}


// Individual Address functions

void A_IndividualAddress_Write_req(A_LayerHandle handle, A_IndividualAddress_s *A_IndividualAddr_req);

void A_IndividualAddress_Read_req(A_LayerHandle handle, A_IndividualAddress_s *A_IndividualAddr_req);

void A_IndividualAddress_Read_res(A_LayerHandle handle, A_IndividualAddress_s *A_IndividualAddr_res) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
		return;
	
	T_Data_s T_Data_res;
	
	T_Data_res.ack_request = A_IndividualAddr_res->ack_request;
	T_Data_res.t_status = A_IndividualAddr_res->a_status;
	T_Data_res.priority = A_IndividualAddr_res->priority;
	T_Data_res.hop_count_type = A_IndividualAddr_res->hop_count_type;
	uint8_t	tsdu[] = {0,0};
	T_Data_res.tsdu = tsdu;
	*(uint16_t*)tsdu = SWAP_UINT16((uint16_t)(KNX_INDIVIDUAL_ADDR_RESPONSE << 6));
	T_Data_res.octet_count = 1;
	
	 T_Data_Broadcast_req(A_Layer->T_layerHnd, &T_Data_res);
}

void A_IndividualAddressSerialNumber_Write_req(A_LayerHandle handle, A_IndividualAddress_s *A_IndividualAddr_req);

void A_IndividualAddressSerialNumber_Read_req(A_LayerHandle handle, A_IndividualAddress_s *A_IndividualAddr_req);

void A_IndividualAddressSerialNumber_Read_res(A_LayerHandle handle, A_IndividualAddress_s *A_IndividualAddr_res) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
	return;
	
	T_Data_s T_Data_res;
	
	T_Data_res.ack_request = A_IndividualAddr_res->ack_request;
	T_Data_res.t_status = A_IndividualAddr_res->a_status;
	T_Data_res.priority = A_IndividualAddr_res->priority;
	T_Data_res.hop_count_type = A_IndividualAddr_res->hop_count_type;
	uint8_t	tsdu[12] = {0,0};
	T_Data_res.tsdu = tsdu;
	*(uint16_t*)tsdu = SWAP_UINT16((uint16_t)(KNX_ESCAPE << 6) | KNX_ESCAPE_INDIVIDUAL_ADDRESS_SERIAL_NUMBER_RESPONSE);
	
	for (uint8_t i = 0; i < 6; i++)
	{
		tsdu[2 + i] = A_IndividualAddr_res->serialNumber[i];
	}
	tsdu[8] = A_IndividualAddr_res->domainAddress >> 8;
	tsdu[9] = A_IndividualAddr_res->domainAddress & 0xff;
	tsdu[10] = 0;
	tsdu[11] = 0;
	
	T_Data_res.octet_count = 11;
	
	T_Data_Broadcast_req(A_Layer->T_layerHnd, &T_Data_res);	
}


// Group value functions

void A_GroupValue_Read_req(A_LayerHandle handle, A_Data_Group_s *A_Data_req) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
		return;
	
	if (((GrpSrv_getGrpObjConfigFlags(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, A_Data_req->ASAP) & (FLAG_COMMUNICATE | FLAG_TRANSMIT)) == (FLAG_COMMUNICATE | FLAG_TRANSMIT)))
	{
		T_Data_s T_data_req;
		
		T_data_req.ack_request = A_Data_req->ack_request;
		T_data_req.t_status = A_Data_req->a_status;
		T_data_req.hop_count_type = A_Data_req->hop_count_type;
		T_data_req.priority = A_Data_req->priority;
		
		T_data_req.TSAP = GrpSrv_getAddrTblIdxByGrpObjTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, A_Data_req->ASAP);
		
		uint8_t tsdu[2] = {0,0};
		
		*(uint16_t*)tsdu = SWAP_UINT16((KNX_GROUP_VALUE_READ << 6));
		T_data_req.tsdu = tsdu;
		
		T_data_req.octet_count = 1;
		
		T_Data_Group_req(A_Layer->T_layerHnd, &T_data_req);
	}
}

void A_GroupValue_Read_res(A_LayerHandle handle, A_Data_Group_s *A_Data_res) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
		return;	
		
	if (((GrpSrv_getGrpObjConfigFlags(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, A_Data_res->ASAP) & (FLAG_COMMUNICATE | FLAG_READ | FLAG_TRANSMIT)) == (FLAG_COMMUNICATE | FLAG_READ | FLAG_TRANSMIT)))
	{	
		T_Data_s T_data_res;
		
		T_data_res.ack_request = A_Data_res->ack_request;
		T_data_res.t_status = A_Data_res->a_status;
		T_data_res.hop_count_type = A_Data_res->hop_count_type;
		T_data_res.priority = A_Data_res->priority;

		T_data_res.TSAP = GrpSrv_getAddrTblIdxByGrpObjTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, A_Data_res->ASAP);

		uint8_t tsdu[16];

		*(uint16_t*)tsdu = SWAP_UINT16((KNX_GROUP_VALUE_RESPONSE << 6));
		T_data_res.tsdu = tsdu;

		// get group object data type
		grpObjSz_e size = GrpSrv_getGrpObjSize(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, A_Data_res->ASAP);

		uint8_t byteCnt = 0;

		switch (size) {
			case GRP_OBJ_SZ_1_BIT:
			case GRP_OBJ_SZ_2_BITS:
			case GRP_OBJ_SZ_3_BITS:
			case GRP_OBJ_SZ_4_BITS:
			case GRP_OBJ_SZ_5_BITS:
			case GRP_OBJ_SZ_6_BITS:
			{
				byteCnt = 0;
				tsdu[1] |= (A_Data_res->asdu[0] & 0x3f);
			}
			break;
			case GRP_OBJ_SZ_7_BITS:
			case GRP_OBJ_SZ_1_BYTE:
			byteCnt = 1;
			break;
			case GRP_OBJ_SZ_2_BYTES:
			byteCnt = 2;
			break;
			case GRP_OBJ_SZ_3_BYTES:
			byteCnt = 3;
			break;
			case GRP_OBJ_SZ_4_BYTES:
			byteCnt = 4;
			break;
			case GRP_OBJ_SZ_6_BYTES:
			byteCnt = 6;
			break;
			case GRP_OBJ_SZ_8_BYTES:
			byteCnt = 8;
			break;
			case GRP_OBJ_SZ_10_BYTES:
			byteCnt = 10;
			break;
			case GRP_OBJ_SZ_14_BYTES:
			byteCnt = 14;
			break;
			case GRP_OBJ_SZ_5_BYTES:
			byteCnt = 5;
			break;
			case GRP_OBJ_SZ_7_BYTES:
			byteCnt = 7;
			break;
			case GRP_OBJ_SZ_9_BYTES:
			byteCnt = 9;
			break;
			case GRP_OBJ_SZ_11_BYTES:
			byteCnt = 11;
			break;
			case GRP_OBJ_SZ_12_BYTES:
			byteCnt = 12;
			break;
			case GRP_OBJ_SZ_13_BYTES:
			byteCnt = 13;
			break;
			case GRP_OBJ_SZ_15_BYTES:
			byteCnt = 15;
			break;
			default:
			break;
		}

		if (byteCnt) {
			for (uint8_t i = 0; i < byteCnt; i++)
			{
				tsdu[i + 2] = A_Data_res->asdu[i];
			}
		}

		T_data_res.octet_count = byteCnt + 1;

		T_Data_Group_req(A_Layer->T_layerHnd, &T_data_res);
	
	}
}

void A_GroupValue_Write_req(A_LayerHandle handle, A_Data_Group_s *A_Data_req) {	// not finished
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
		return;

	if (((GrpSrv_getGrpObjConfigFlags(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, A_Data_req->ASAP) & (FLAG_COMMUNICATE |FLAG_TRANSMIT)) == (FLAG_COMMUNICATE | FLAG_TRANSMIT)))
	{
		T_Data_s T_data_req;
		
		T_data_req.ack_request = A_Data_req->ack_request;
		T_data_req.t_status = A_Data_req->a_status;
		T_data_req.hop_count_type = A_Data_req->hop_count_type;
		T_data_req.priority = A_Data_req->priority;
		
		T_data_req.TSAP = GrpSrv_getAddrTblIdxByGrpObjTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, A_Data_req->ASAP);

		uint8_t tsdu[16];
	
		*(uint16_t*)tsdu = SWAP_UINT16((KNX_GROUP_VALUE_WRITE << 6));
		T_data_req.tsdu = tsdu;
	
		// get group object data type
		grpObjSz_e size = GrpSrv_getGrpObjSize(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, A_Data_req->ASAP);
	
		uint8_t byteCnt = 0;
	
		switch (size) {
			case GRP_OBJ_SZ_1_BIT:
			case GRP_OBJ_SZ_2_BITS:
			case GRP_OBJ_SZ_3_BITS:
			case GRP_OBJ_SZ_4_BITS:
			case GRP_OBJ_SZ_5_BITS:
			case GRP_OBJ_SZ_6_BITS:
			{
				byteCnt = 0;
				tsdu[1] |= (A_Data_req->asdu[0] & 0x3f);
			}
			break;
			case GRP_OBJ_SZ_7_BITS:
			case GRP_OBJ_SZ_1_BYTE:
			byteCnt = 1;
			break;
			case GRP_OBJ_SZ_2_BYTES:
			byteCnt = 2;
			break;
			case GRP_OBJ_SZ_3_BYTES:
			byteCnt = 3;
			break;
			case GRP_OBJ_SZ_4_BYTES:
			byteCnt = 4;
			break;
			case GRP_OBJ_SZ_6_BYTES:
			byteCnt = 6;
			break;
			case GRP_OBJ_SZ_8_BYTES:
			byteCnt = 8;
			break;
			case GRP_OBJ_SZ_10_BYTES:
			byteCnt = 10;
			break;
			case GRP_OBJ_SZ_14_BYTES:
			byteCnt = 14;
			break;
			case GRP_OBJ_SZ_5_BYTES:
			byteCnt = 5;
			break;
			case GRP_OBJ_SZ_7_BYTES:
			byteCnt = 7;
			break;
			case GRP_OBJ_SZ_9_BYTES:
			byteCnt = 9;
			break;
			case GRP_OBJ_SZ_11_BYTES:
			byteCnt = 11;
			break;
			case GRP_OBJ_SZ_12_BYTES:
			byteCnt = 12;
			break;
			case GRP_OBJ_SZ_13_BYTES:
			byteCnt = 13;
			break;
			case GRP_OBJ_SZ_15_BYTES:
			byteCnt = 15;
			break;
			default:
			break;
		}
	
		if (byteCnt) {
			for (uint8_t i = 0; i < byteCnt; i++)
			{
				tsdu[i + 2] = A_Data_req->asdu[i];
			}
		}
	
		T_data_req.octet_count = byteCnt + 1;

		T_Data_Group_req(A_Layer->T_layerHnd, &T_data_req);
	}
}

// device descriptor functions

void A_Device_Descriptor_res(A_LayerHandle handle, A_Device_Descriptor_s *A_Dev_Descr_res) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
		return;
	
	T_Data_s T_Data_req;
	uint8_t tsdu[16] = {0,0,0,0};	
	T_Data_req.tsdu = tsdu;
	T_Data_req.ack_request = A_Dev_Descr_res->ack_request;
	T_Data_req.t_status = A_Dev_Descr_res->a_status;
	T_Data_req.priority = A_Dev_Descr_res->priority;
	T_Data_req.connType = A_Dev_Descr_res->connType;
	T_Data_req.hop_count_type = A_Dev_Descr_res->hop_count_type;
	T_Data_req.destination_address = A_Dev_Descr_res->ASAP;
	T_Data_req.TSAP = A_Dev_Descr_res->ASAP;
	
	*(uint16_t*)tsdu = SWAP_UINT16((uint16_t)(KNX_MASK_VERSION_RESPONSE << 6));
	
	if (A_Dev_Descr_res->device_descriptor_octet_count == 1)
	{
		T_Data_req.octet_count = 1;
		tsdu[1] |= A_Dev_Descr_res->device_descriptor[0];		
	}
	else
	{
		for (uint8_t i = 0; i < A_Dev_Descr_res->device_descriptor_octet_count; i++)
		{
			tsdu[2 + i] = A_Dev_Descr_res->device_descriptor[i];
		}
		T_Data_req.octet_count = A_Dev_Descr_res->device_descriptor_octet_count + 1;
	}

	if (A_Dev_Descr_res->connType == TL_CONNECTION_ORIENTED)
		T_Data_Connected_req(A_Layer->T_layerHnd , &T_Data_req);
	else
		T_Data_Individual_req(A_Layer->T_layerHnd , &T_Data_req);
}

void A_Device_Descriptor_req(A_LayerHandle handle, A_Device_Descriptor_s *A_Dev_Descr_req) { // not finished
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
		return;
		
}

// property value read / write functions

void A_Property_Value_Read_req(A_LayerHandle handle, A_Property_Value_s *A_Property_val_req) {
	
}

void A_Property_Value_Read_res(A_LayerHandle handle, A_Property_Value_s *A_Property_val_res) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
		return;
	
	T_Data_s T_Data_req;
	uint8_t tsdu[MAX_TSDU_LENGTH];
	T_Data_req.tsdu = tsdu;
	T_Data_req.ack_request = A_Property_val_res->ack_request;
	T_Data_req.t_status = A_Property_val_res->a_status;
	T_Data_req.priority = A_Property_val_res->priority;
	T_Data_req.connType = A_Property_val_res->connType;
	T_Data_req.hop_count_type = A_Property_val_res->hop_count_type;
	T_Data_req.destination_address = A_Property_val_res->ASAP;
	T_Data_req.TSAP = A_Property_val_res->ASAP;
	
	*(uint16_t*)tsdu = SWAP_UINT16((uint16_t)(KNX_ESCAPE << 6) | KNX_ESCAPE_PROPERTY_RESPONSE);
	
	T_Data_req.octet_count = 2;
	
	tsdu[T_Data_req.octet_count++] = A_Property_val_res->objectIndex;
	tsdu[T_Data_req.octet_count++] = A_Property_val_res->propertyId;
	tsdu[T_Data_req.octet_count++] = (A_Property_val_res->startIndex >> 8) & 0xf;
	tsdu[T_Data_req.octet_count++] = A_Property_val_res->startIndex & 0xff;
	
	if (A_Property_val_res->count > 0 && A_Property_val_res->data_byteCnt > 0)
	{
		for (uint8_t i = 0; ((i < A_Property_val_res->data_byteCnt) && (T_Data_req.octet_count <= SWAP_UINT16(*(A_Layer->T_layer.maxTDSULength)))); i++)  // && (T_Data_req.octet_count <= SWAP_UINT16(*(A_Layer->T_layer.maxTDSULength)))
			tsdu[T_Data_req.octet_count++] = *(A_Property_val_res->data + i);
		tsdu[4] |= (A_Property_val_res->count << 4);
	}
	
	T_Data_req.octet_count--;  // decrease count by one. TPCI byte does not count
	
	if (A_Property_val_res->connType == TL_CONNECTION_ORIENTED)
		T_Data_Connected_req(A_Layer->T_layerHnd , &T_Data_req);
	else
		T_Data_Individual_req(A_Layer->T_layerHnd , &T_Data_req);	
}

void A_Property_Value_Write_req(A_LayerHandle handle, A_Property_Value_s *A_Property_val_req) {

}

// property description read functions

void A_Property_Description_Read_req(A_LayerHandle handle, A_Property_Description_s *A_property_descr_req) {
	
	
}

void A_Property_Description_Read_res(A_LayerHandle handle, A_Property_Description_s *A_property_descr_res) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
	return;
	
	T_Data_s T_Data_req;
	uint8_t tsdu[9];
	T_Data_req.tsdu = tsdu;
	T_Data_req.octet_count = 8;
	T_Data_req.ack_request = A_property_descr_res->ack_request;
	T_Data_req.t_status = A_property_descr_res->a_status;
	T_Data_req.priority = A_property_descr_res->priority;
	T_Data_req.connType = A_property_descr_res->connType;
	T_Data_req.hop_count_type = A_property_descr_res->hop_count_type;
	T_Data_req.destination_address = A_property_descr_res->ASAP;
	T_Data_req.TSAP = A_property_descr_res->ASAP;
	
	*(uint16_t*)tsdu = SWAP_UINT16((uint16_t)(KNX_ESCAPE << 6) | KNX_ESCAPE_PROPERTY_DESCR_RESPONSE);
	
	tsdu[2] = A_property_descr_res->objectIndex;
	tsdu[3] = A_property_descr_res->propertyId;
	tsdu[4] = A_property_descr_res->propertyIndex;
	
	if (A_property_descr_res->maxElements)
	{
		tsdu[5] = (A_property_descr_res->DataType & 0x3f) | (((A_property_descr_res->writeEnable & 0x01) != 0) ? 0x80 : 0);
		tsdu[6] = (A_property_descr_res->maxElements >> 8) & 0xf;
		tsdu[7] = A_property_descr_res->maxElements & 0xff;
		tsdu[8] = A_property_descr_res->accessLvl;
	}
	else
	{
		tsdu[5] = 0;
		tsdu[6] = 0;
		tsdu[7] = 0;
		tsdu[8] = 0;
	}
	
	if (A_property_descr_res->connType == TL_CONNECTION_ORIENTED)
		T_Data_Connected_req(A_Layer->T_layerHnd , &T_Data_req);
	else
		T_Data_Individual_req(A_Layer->T_layerHnd , &T_Data_req);	
}

// main loop service routine

void A_service(A_LayerHandle handle) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)handle;
	if (!A_Layer->enable)
		return;	
	T_service(handle->T_layerHnd);
}


void A_P2P_common_services(void *parent_Hndl, T_Data_s *T_Data_ind) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)parent_Hndl;
	
	uint16_t apci_full = SWAP_UINT16(*((uint16_t*)T_Data_ind->tsdu));
	apci_e apci = (apci_full >> 6) & 0xf;
	escape_apdu_e apci_escape = apci_full & 0x3f;
	user_apdu_e apci_user = apci_full & 0x3f;

	// connection oriented and connectionless common services.
	
	switch (apci) {
		case KNX_MASK_VERSION_READ:
		{ // answer mask version read request
			A_Device_Descriptor_s A_Dev_Descr_res;
			A_Dev_Descr_res.ack_request = T_Data_ind->ack_request;
			A_Dev_Descr_res.a_status = T_Data_ind->t_status;
			A_Dev_Descr_res.priority = T_Data_ind->priority;
			A_Dev_Descr_res.connType = T_Data_ind->connType;
			A_Dev_Descr_res.hop_count_type = T_Data_ind->hop_count_type;
			A_Dev_Descr_res.ASAP = T_Data_ind->TSAP;
			A_Dev_Descr_res.deviceDescriptorType = *(T_Data_ind->tsdu + 1) & 0x3f;
			if (A_Layer->A_Device_Descriptor_Read_ind_cb)
				A_Layer->A_Device_Descriptor_Read_ind_cb(A_Layer->parent_Hndl, &A_Dev_Descr_res);
		}
		break;
		case KNX_MASK_VERSION_RESPONSE:
		break;
		case KNX_RESTART:
		{
			if (!(T_Data_ind->tsdu[1] & 0x1e)) // check for the reserved bit fields to be zero.
			{
				A_Restart_s A_Restart_ind;
				A_Restart_ind.ack_request = T_Data_ind->ack_request;
				A_Restart_ind.a_status = T_Data_ind->t_status;
				A_Restart_ind.priority = T_Data_ind->priority;
				A_Restart_ind.connType = T_Data_ind->connType;
				A_Restart_ind.hop_count_type = T_Data_ind->hop_count_type;
				A_Restart_ind.ASAP = T_Data_ind->TSAP;
				
				A_Restart_ind.restartType = T_Data_ind->tsdu[1] & 0x1;
				if (A_Restart_ind.restartType)
				{
					A_Restart_ind.eraseCode = T_Data_ind->tsdu[2];
					A_Restart_ind.channelNumber = T_Data_ind->tsdu[3];
				}
				
				if (A_Layer->A_Restart_ind_cb)
					A_Layer->A_Restart_ind_cb(A_Layer->parent_Hndl, &A_Restart_ind);
			}
		}
		break;
		case KNX_ESCAPE:
		{
			switch (apci_escape) {
				case KNX_ESCAPE_PROPERTY_DESCR_REQ:
				{
					A_Property_Description_s A_Prop_Descr;
					A_Prop_Descr.ack_request = T_Data_ind->ack_request;
					A_Prop_Descr.a_status = T_Data_ind->t_status;
					A_Prop_Descr.priority = T_Data_ind->priority;
					A_Prop_Descr.connType = T_Data_ind->connType;
					A_Prop_Descr.hop_count_type = T_Data_ind->hop_count_type;
					A_Prop_Descr.ASAP = T_Data_ind->TSAP;
					
					A_Prop_Descr.objectIndex = *(T_Data_ind->tsdu + 2);
					A_Prop_Descr.propertyId = *(T_Data_ind->tsdu + 3);
					A_Prop_Descr.propertyIndex = *(T_Data_ind->tsdu + 4);
					
					if (A_Layer->A_Property_Description_Read_ind_cb)
						A_Layer->A_Property_Description_Read_ind_cb(A_Layer->parent_Hndl, &A_Prop_Descr);
				}
				break;
				case KNX_ESCAPE_PROPERTY_DESCR_RESPONSE:
				break;
				case KNX_ESCAPE_PROPERTY_REQ:
				{
					A_Property_Value_s A_Prop_val;
					A_Prop_val.ack_request = T_Data_ind->ack_request;
					A_Prop_val.a_status = T_Data_ind->t_status;
					A_Prop_val.priority = T_Data_ind->priority;
					A_Prop_val.connType = T_Data_ind->connType;
					A_Prop_val.hop_count_type = T_Data_ind->hop_count_type;
					A_Prop_val.ASAP = T_Data_ind->TSAP;
					
					A_Prop_val.objectIndex = *(T_Data_ind->tsdu + 2);
					A_Prop_val.propertyId = *(T_Data_ind->tsdu + 3);
					A_Prop_val.count = *(T_Data_ind->tsdu + 4) >> 4;
					A_Prop_val.startIndex = (((uint16_t)*(T_Data_ind->tsdu + 4) & 0xf) << 8) | *(T_Data_ind->tsdu + 5);
					
					if (A_Layer->A_Property_Value_Read_ind_cb)
						A_Layer->A_Property_Value_Read_ind_cb(A_Layer->parent_Hndl, &A_Prop_val);
				}
				break;
				case KNX_ESCAPE_PROPERTY_RESPONSE:
				break;
				case KNX_ESCAPE_PROPERTY_WRITE:
				{
					A_Property_Value_s A_Prop_val;
					A_Prop_val.ack_request = T_Data_ind->ack_request;
					A_Prop_val.a_status = T_Data_ind->t_status;
					A_Prop_val.priority = T_Data_ind->priority;
					A_Prop_val.connType = T_Data_ind->connType;
					A_Prop_val.hop_count_type = T_Data_ind->hop_count_type;
					A_Prop_val.ASAP = T_Data_ind->TSAP;
					
					A_Prop_val.objectIndex = *(T_Data_ind->tsdu + 2);
					A_Prop_val.propertyId = *(T_Data_ind->tsdu + 3);
					A_Prop_val.count = *(T_Data_ind->tsdu + 4) >> 4;
					A_Prop_val.startIndex = (((uint16_t)*(T_Data_ind->tsdu + 4) & 0xf) << 8) | *(T_Data_ind->tsdu + 5);
					A_Prop_val.data = T_Data_ind->tsdu + 6;
					A_Prop_val.data_byteCnt = T_Data_ind->octet_count - 5;
					
					if (A_Layer->A_Property_Value_Write_ind_cb)
						A_Layer->A_Property_Value_Write_ind_cb(A_Layer->parent_Hndl, &A_Prop_val);
				}
				break;
				case KNX_ESCAPE_LINK_READ:
				break;
				case KNX_ESCAPE_LINK_RESPONSE:
				break;
				case KNX_ESCAPE_LINK_WRITE:
				break;
				default:
				break;
			}
		}
		break;
		case KNX_USER_MESSAGE:
		{
			switch (apci_user) {
				case KNX_USER_FUNCTION_PROPERTY_COMMAND:
				break;
				case KNX_USER_FUNCTION_PROPERTY_STATE_READ:
				break;
				case KNX_USER_FUNCTION_PROPERTY_STATE_RESPONSE:
				break;
				default:
				break;
			}
		}
		default:
		break;
	}	
}

void A_P2P_connection_oriented_services(void *parent_Hndl, T_Data_s *T_Data_ind) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)parent_Hndl;
	
	uint16_t apci_full = SWAP_UINT16(*((uint16_t*)T_Data_ind->tsdu));
	apci_e apci = (apci_full >> 6) & 0xf;
	escape_apdu_e apci_escape = apci_full & 0x3f;
	user_apdu_e apci_user = apci_full & 0x3f;
	
	switch (apci) {
		case KNX_ADC_READ:
		{
			A_ADC_read_s A_ADC_read_ind;
			
			A_ADC_read_ind.a_status = T_Data_ind->t_status;
			A_ADC_read_ind.ack_request = T_Data_ind->ack_request;
			A_ADC_read_ind.ASAP = T_Data_ind->TSAP;
			A_ADC_read_ind.hop_count_type = T_Data_ind->hop_count_type;
			A_ADC_read_ind.priority = T_Data_ind->priority;
			A_ADC_read_ind.connType = TL_CONNECTION_ORIENTED;
			A_ADC_read_ind.channel = T_Data_ind->tsdu[1] & 0x3f;
			A_ADC_read_ind.readCount = T_Data_ind->tsdu[2];
			
			if (A_Layer->A_ADC_Read_ind_cb)
				A_Layer->A_ADC_Read_ind_cb(A_Layer->parent_Hndl, &A_ADC_read_ind);
		}
		break;
		case KNX_ADC_RESPONSE:
		break;
		case KNX_MEMORY_READ:
		{
			A_Memory_s A_Memory_read_ind;

			A_Memory_read_ind.a_status = T_Data_ind->t_status;
			A_Memory_read_ind.ack_request = T_Data_ind->ack_request;
			A_Memory_read_ind.ASAP = T_Data_ind->TSAP;
			A_Memory_read_ind.hop_count_type = T_Data_ind->hop_count_type;
			A_Memory_read_ind.priority = T_Data_ind->priority;			
			A_Memory_read_ind.connType = TL_CONNECTION_ORIENTED;
			A_Memory_read_ind.number = T_Data_ind->tsdu[1] & 0x3f;
			A_Memory_read_ind.memoryAddress = (uint16_t)T_Data_ind->tsdu[2] << 8 | (T_Data_ind->tsdu[3] & 0xff);
			
			if (A_Layer->A_Memory_Read_ind_cb)
				A_Layer->A_Memory_Read_ind_cb(A_Layer->parent_Hndl, &A_Memory_read_ind);
		}
		break;
		case KNX_MEMORY_RESPONSE:
		break;
		case KNX_MEMORY_WRITE:
		break;
		case KNX_ESCAPE:
		{
			switch(apci_escape) {
				case KNX_ESCAPE_BIT_WRITE:
				break;
				case KNX_ESCAPE_AUTHORIZE_REQ:
				{
					A_Authorisation_s	A_Authorisation_req_ind;
					
					A_Authorisation_req_ind.a_status = T_Data_ind->t_status;
					A_Authorisation_req_ind.ack_request = T_Data_ind->ack_request;
					A_Authorisation_req_ind.hop_count_type = T_Data_ind->hop_count_type;
					A_Authorisation_req_ind.priority = T_Data_ind->priority;
					A_Authorisation_req_ind.connType = TL_CONNECTION_ORIENTED;
					A_Authorisation_req_ind.ASAP = T_Data_ind->TSAP;
					for (uint8_t i = 0; i < 4; i++)
						A_Authorisation_req_ind.key[i] = *(T_Data_ind->tsdu + 3 + i);
					
					if (A_Layer->A_Authorize_Request_ind_cb) // check if call back function registered
						A_Layer->A_Authorize_Request_ind_cb(A_Layer->parent_Hndl, &A_Authorisation_req_ind);
				}
				break;
				case KNX_ESCAPE_AUTHORIZE_RESPONSE:
				break;
				case KNX_ESCAPE_SET_KEY_REQ:
				break;
				case KNX_ESCAPE_SET_KEY_RESPONSE:
				break;
				default:
				break;
			}	
		}
		break;
		case KNX_USER_MESSAGE:
		{
			switch(apci_user) {
				case KNX_USER_MEMORY_READ:
				break;
				case KNX_USER_MEMORY_RESPONSE:
				break;
				case KNX_USER_MEMORY_WRITE:
				break;
				case KNX_USER_MANUFACTURER_REQ:
				break;
				case KNX_USER_MANUFACTURER_RESPONSE:
				break;
				default:
				break;
			}
		}
		break;
		default:
		break;
	}
}

void A_Broadcast_services(void *parent_Hndl, T_Data_s *T_Data_ind) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)parent_Hndl;
	
	uint16_t apci_full = SWAP_UINT16(*((uint16_t*)T_Data_ind->tsdu));
	apci_e apci = (apci_full >> 6) & 0xf;
	escape_apdu_e apci_escape = apci_full & 0x3f;
	user_apdu_e apci_user = apci_full & 0x3f;
	
	switch (apci) {
		case KNX_INDIVIDUAL_ADDR_WRITE:
		{
			A_IndividualAddress_s A_individualAddr;
			A_individualAddr.newAddress = (uint16_t)T_Data_ind->tsdu[2] << 8 | T_Data_ind->tsdu[3];
			
			if (A_Layer->A_IndividualAddress_Write_ind_cb)
				A_Layer->A_IndividualAddress_Write_ind_cb(A_Layer->parent_Hndl, &A_individualAddr);
		}
		break;
		case KNX_INDIVIDUAL_ADDR_REQ:
		{
			A_IndividualAddress_s A_individualAddr;
			A_individualAddr.hop_count_type = 6;
			A_individualAddr.priority = KNX_SYSTEM;
			
			if (A_Layer->A_IndividualAddress_Read_ind_cb)
				A_Layer->A_IndividualAddress_Read_ind_cb(A_Layer->parent_Hndl, &A_individualAddr);

		}
		break;
		case KNX_INDIVIDUAL_ADDR_RESPONSE:

		break;
		case KNX_ESCAPE:
		{
			switch(apci_escape) {
				case KNX_ESCAPE_INDIVIDUAL_ADDRESS_SERIAL_NUMBER_WRITE:
				{
					A_IndividualAddress_s A_individualAddr;
					A_individualAddr.hop_count_type = 6;
					A_individualAddr.priority = KNX_SYSTEM;
					for (uint8_t i = 0; i < 6; i++)
					{
						A_individualAddr.serialNumber[i] = T_Data_ind->tsdu[2 + i];
					}
					A_individualAddr.newAddress = (uint16_t)T_Data_ind->tsdu[8] << 8 | T_Data_ind->tsdu[9];
					
					if (A_Layer->A_IndividualAddressSerialNumber_Write_ind_cb)
					A_Layer->A_IndividualAddressSerialNumber_Write_ind_cb(A_Layer->parent_Hndl, &A_individualAddr);
				}				
				break;
				case KNX_ESCAPE_INDIVIDUAL_ADDRESS_SERIAL_NUMBER_READ:
				{
					A_IndividualAddress_s A_individualAddr;
					A_individualAddr.hop_count_type = 6;
					A_individualAddr.priority = KNX_SYSTEM;
					for (uint8_t i = 0; i < 6; i++)
					{
						A_individualAddr.serialNumber[i] = T_Data_ind->tsdu[2 + i];
					}
					
					if (A_Layer->A_IndividualAddressSerialNumber_Read_ind_cb)
						A_Layer->A_IndividualAddressSerialNumber_Read_ind_cb(A_Layer->parent_Hndl, &A_individualAddr);
				}
				break;
				case KNX_ESCAPE_INDIVIDUAL_ADDRESS_SERIAL_NUMBER_RESPONSE:
				break;
				case KNX_ESCAPE_NETWORK_PARAMETER_READ:
				break;
				case KNX_ESCAPE_NETWORK_PARAMETER_WRITE:
				break;
				case KNX_ESCAPE_NETWORK_PARAMETER_INFO_REPORT:
				break;	
				default:
				break;
			}
			
		}
		break;
		case KNX_USER_MESSAGE:
		{
			switch(apci_user) {
				default:
				break;
			}
		}
		break;
		default:
		break;
	}	
}

void A_SystemBroadcast_services(void *parent_Hndl, T_Data_s *T_Data_ind) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;
	
	uint16_t apci_full = SWAP_UINT16(*((uint16_t*)T_Data_ind->tsdu));
	apci_e apci = (apci_full >> 6) & 0xf;
	escape_apdu_e apci_escape = apci_full & 0x3f;
	user_apdu_e apci_user = apci_full & 0x3f;
	
	switch (apci) {
		case KNX_MASK_VERSION_RESPONSE: // same service as device descriptor response.
		break;
		case KNX_ESCAPE:
		{
			switch(apci_escape) {
				case KNX_ESCAPE_DOMAIN_ADDRESS_WRITE:
				break;
				case KNX_ESCAPE_DOMAIN_ADDRESS_REQ:
				break;
				case KNX_ESCAPE_DOMAIN_ADDRESS_RESPONSE:
				break;
				case KNX_ESCAPE_DOMAIN_ADDRESS_SELECTIVE_READ:
				break;
				case KNX_ESCAPE_DOMAIN_ADDRESS_SERIAL_NUMBER_WRITE:
				break;
				case KNX_ESCAPE_DOMAIN_ADDRESS_SERIAL_NUMBER_READ:
				break;
				case KNX_ESCAPE_DOMAIN_ADDRESS_SERIAL_NUMBER_RESPONSE:
				break;
				case KNX_ESCAPE_NETWORK_PARAMETER_WRITE:
				break;
				case KNX_ESCAPE_NETWORK_PARAMETER_READ:
				break;
				case KNX_ESCAPE_NETWORK_PARAMETER_INFO_REPORT:
				break;
				default:
				break;
			}
		}
		case KNX_USER_MESSAGE:
		{
			switch(apci_user) {
				default:
				break;
			}
		}
		break;
		default:
		break;
	}	
}

// T Data indication call backs

void T_Data_Connected_ind_cb(void *parent_Hndl, T_Data_s *T_Data_ind) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;
	
	A_P2P_common_services(parent_Hndl, T_Data_ind);
	
	A_P2P_connection_oriented_services(parent_Hndl, T_Data_ind);
}

void T_Data_Individual_ind_cb(void *parent_Hndl, T_Data_s *T_Data_ind) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;
	
	A_P2P_common_services(parent_Hndl, T_Data_ind);
}

void T_Data_Group_ind_cb(void *parent_Hndl, T_Data_s *T_Data_ind) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)parent_Hndl;
	
	uint16_t apci_full = SWAP_UINT16(*((uint16_t*)T_Data_ind->tsdu));
	apci_e apci = (apci_full >> 6) & 0xf;
	escape_apdu_e apci_escape = apci_full & 0x3f;
	user_apdu_e apci_user = apci_full & 0x3f;

	switch (apci) {
		case KNX_GROUP_VALUE_READ:
		if (A_Layer->A_GroupValue_Read_ind_cb) // check if call back function registered
		{
			A_Data_Group_s A_Data_ind;
				
			map_T_Data_to_A_Group_Data(&A_Data_ind, T_Data_ind);
			
			uint16_t assocTblIdx = GrpSrv_getFirstAssocTblIdxByAddrTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, T_Data_ind->TSAP);
				
			while (assocTblIdx)
			{
				// lookup group object index by address table index
				uint16_t ASAP = GrpSrv_getGrpObjIdxByAssocTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, assocTblIdx);
				
				if (ASAP)
				{
					if (((GrpSrv_getGrpObjConfigFlags(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, ASAP) & (FLAG_COMMUNICATE |FLAG_READ)) == (FLAG_COMMUNICATE | FLAG_READ)))
					{
						A_Data_ind.ASAP = ASAP;
					
						A_Layer->A_GroupValue_Read_ind_cb(A_Layer->parent_Hndl, &A_Data_ind);
						assocTblIdx = 0; // end loop						
					}
					else
						assocTblIdx = GrpSrv_getNextAssocTblIdxByAddrTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, assocTblIdx);
				}
			}
		}
		break;
		case KNX_GROUP_VALUE_WRITE:
		{
			if (A_Layer->A_GroupValue_Write_ind_cb) // check if call back function registered
			{
				A_Data_Group_s A_Data_ind;
		
				map_T_Data_to_A_Group_Data(&A_Data_ind, T_Data_ind);

				uint16_t assocTblIdx = GrpSrv_getFirstAssocTblIdxByAddrTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, T_Data_ind->TSAP);
		
				while (assocTblIdx)
				{
			
					// lookup group object index by address table index
					uint16_t ASAP = GrpSrv_getGrpObjIdxByAssocTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, assocTblIdx);
			
					if (ASAP)
					{
						if (((GrpSrv_getGrpObjConfigFlags(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, ASAP) & (FLAG_COMMUNICATE |FLAG_WRITE)) == (FLAG_COMMUNICATE | FLAG_WRITE)))
						{
							A_Data_ind.ASAP = ASAP;
								
							A_Layer->A_GroupValue_Write_ind_cb(A_Layer->parent_Hndl, &A_Data_ind);
						}
					}
					assocTblIdx = GrpSrv_getNextAssocTblIdxByAddrTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, assocTblIdx);
				}
			}
		}
		break;
		case KNX_GROUP_VALUE_RESPONSE:
		{
			if (A_Layer->A_GroupValue_Read_Acon_cb) // check if call back function registered
			{
				A_Data_Group_s A_Data_con;
						
				map_T_Data_to_A_Group_Data(&A_Data_con, T_Data_ind);
						
				uint16_t assocTblIdx = GrpSrv_getFirstAssocTblIdxByAddrTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, T_Data_ind->TSAP);
						
				while (assocTblIdx)
				{
							
					// lookup group object index by address table index
					uint16_t ASAP = GrpSrv_getGrpObjIdxByAssocTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, assocTblIdx);
							
					if (ASAP)
					{
						if (((GrpSrv_getGrpObjConfigFlags(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, ASAP) & (FLAG_COMMUNICATE |FLAG_UPDATE)) == (FLAG_COMMUNICATE | FLAG_UPDATE)))
						{
							A_Data_con.ASAP = ASAP;

							A_Layer->A_GroupValue_Read_Acon_cb(A_Layer->parent_Hndl, &A_Data_con);
						}
					}
					assocTblIdx = GrpSrv_getNextAssocTblIdxByAddrTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, assocTblIdx);
				}
			}			
		}
		break;
		case KNX_ESCAPE:
		{
			switch (apci_escape) {
				case KNX_ESCAPE_GROUP_PROP_INFO_REPORT:
				
				break;
				case KNX_ESCAPE_GROUP_PROP_VALUE_READ:
				
				break;
				case KNX_ESCAPE_GROUP_PROP_VALUE_RESPONSE:
				
				break;
				case KNX_ESCAPE_GROUP_PROP_VALUE_WRITE:
				
				break;
				default:
				break;
			}
		}
		case KNX_USER_MESSAGE:
		{
			switch(apci_user) {
				default:
				break;
			}
		}
		break;
		default:
		break;
	}
		
		
}

void T_Data_Broadcast_ind_cb(void *parent_Hndl, T_Data_s *T_Data_ind) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;
	
	A_Broadcast_services(parent_Hndl, T_Data_ind);
}


void T_Data_Tag_Group_ind_cb(void *parent_Hndl, T_Data_s *T_Data_ind) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;
	
}

void T_Data_systemBroadcast_ind_cb(void *parent_Hndl, T_Data_s *T_Data_ind) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;
	
	A_SystemBroadcast_services(parent_Hndl, T_Data_ind);
}

void T_Connect_ind_cb(void *parent_Hndl, uint16_t TSAP) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;	
}

void T_Disconnect_ind_cb(void *parent_Hndl, uint16_t TSAP) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;	
}

// T Data confirmation call backs

void T_Data_Individual_con_cb(void *parent_Hndl, T_Data_s *T_Data_con) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;	
}

void T_Data_Group_con_cb(void *parent_Hndl, T_Data_s *T_Data_con) {
	A_LayerObj *A_Layer;
	A_Layer = (A_LayerObj *)parent_Hndl;	

	uint16_t apci_full = SWAP_UINT16(*((uint16_t*)T_Data_con->tsdu));
	apci_e apci = (apci_full >> 6) & 0xf;
	escape_apdu_e apci_escape = apci_full & 0x3f;
	user_apdu_e apci_user = apci_full & 0x3f;

	switch (apci) {
		case KNX_GROUP_VALUE_READ:
		if (A_Layer->A_GroupValue_Read_Lcon_cb) // check if call back function registered
		{
			A_Data_Group_s A_Data_con;
							
			map_T_Data_to_A_Group_Data(&A_Data_con, T_Data_con);
			
			// lookup group object index by address table index
			uint16_t ASAP = GrpSrv_getGrpObjTblIdxByAddrTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, T_Data_con->TSAP);
				
			if (ASAP)
			{

				A_Data_con.ASAP = ASAP;
					
				A_Layer->A_GroupValue_Read_Lcon_cb(A_Layer->parent_Hndl, &A_Data_con);
			}
		}
		break;
		case KNX_GROUP_VALUE_WRITE:
		if (A_Layer->A_GroupValue_Write_Lcon_cb) // check if call back function registered
		{
			A_Data_Group_s A_Data_con;
			
			map_T_Data_to_A_Group_Data(&A_Data_con, T_Data_con);
			
			uint16_t assocTblIdx = GrpSrv_getFirstAssocTblIdxByAddrTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, T_Data_con->TSAP);
			
			while (assocTblIdx)
			{
				// lookup group object index by address table index
				uint16_t ASAP = GrpSrv_getGrpObjIdxByAssocTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, assocTblIdx);
				
				if (ASAP)
				{
					A_Data_con.ASAP = ASAP;
					
					A_Layer->A_GroupValue_Write_Lcon_cb(A_Layer->parent_Hndl, &A_Data_con);
				}
				assocTblIdx = GrpSrv_getNextAssocTblIdxByAddrTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, assocTblIdx);
			}


		}
		break;
		case KNX_GROUP_VALUE_RESPONSE:
		if (A_Layer->A_GroupValue_Read_Rcon_cb) // check if call back function registered
		{
			A_Data_Group_s A_Data_con;
							
			map_T_Data_to_A_Group_Data(&A_Data_con, T_Data_con);
			
			// lookup group object index by address table index
			uint16_t ASAP = GrpSrv_getGrpObjTblIdxByAddrTblIdx(((A_InterfaceHandle)A_Layer->parent_Hndl)->GO_Hndl, T_Data_con->TSAP);
				
			if (ASAP)
			{

				A_Data_con.ASAP = ASAP;
					
				A_Layer->A_GroupValue_Read_Rcon_cb(A_Layer->parent_Hndl, &A_Data_con);
			}
		}		
		break;
		case KNX_ESCAPE:
		{
			switch (apci_escape) {
				case KNX_ESCAPE_GROUP_PROP_INFO_REPORT:
					
				break;
				case KNX_ESCAPE_GROUP_PROP_VALUE_READ:
					
				break;
				case KNX_ESCAPE_GROUP_PROP_VALUE_RESPONSE:
					
				break;
				case KNX_ESCAPE_GROUP_PROP_VALUE_WRITE:
					
				break;
				default:
				break;
			}
		}
		case KNX_USER_MESSAGE:
		{
			switch(apci_user) {
				default:
				break;
			}
		}
		break;
		default:
		break;
	}
	
}

void T_Data_Tag_Group_con_cb(void *parent_Hndl, T_Data_s *T_Data_con) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;	
}

void T_Data_Broadcast_con_cb(void *parent_Hndl, T_Data_s *T_Data_con) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;	
}

void T_Data_systemBroadcast_con_cb(void *parent_Hndl, T_Data_s *T_Data_con) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;	
}

void T_Data_Connected_con_cb(void *parent_Hndl, uint16_t TSAP) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;	
}

void T_Connect_con_cb(void *parent_Hndl, uint16_t destination_address, uint16_t TSAP, t_status_e status) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;	
}

void T_Disconnect_con_cb(void *parent_Hndl, uint16_t TSAP, priority_e priority, t_status_e status) {
	//A_LayerObj *A_Layer;
	//A_Layer = (A_LayerObj *)parent_Hndl;	
}

int8_t map_T_Data_to_A_Group_Data(A_Data_Group_s *A_Group_Data, T_Data_s *T_Data) {
	A_Group_Data->hop_count_type = T_Data->hop_count_type;
	A_Group_Data->priority = T_Data->priority;
	A_Group_Data->a_status = T_Data->t_status;
	A_Group_Data->ack_request = T_Data->ack_request;
	A_Group_Data->asdu = T_Data->tsdu + 1;
	A_Group_Data->asdu_byteCnt = T_Data->octet_count;
	return 0;
}
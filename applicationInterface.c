/*
 * applicationInterface.c
 *
 * Created: 30-8-2016 17:30:27
 *  Author: Paul
 */ 

#include "applicationInterface.h"
#include <avr/pgmspace.h>

A_InterfaceHandle A_InterfaceInit(void *pMemory, uint16_t numBytes) {
	A_InterfaceHandle A_Interface_handle;
	if (numBytes < sizeof(A_InterfaceObj))
		return ((A_InterfaceHandle)NULL);
	A_Interface_handle = (A_InterfaceHandle)pMemory;
	return(A_Interface_handle);	
}

int8_t A_InterfaceSetup(A_InterfaceHandle handle, void *parent_Hndl) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)handle;

	AI_Hndl->parent_Hndl = parent_Hndl;

	AI_Hndl->IO_Hndl = IntfObj_Srv_Init(&AI_Hndl->IOsrv_obj, sizeof(AI_Hndl->IOsrv_obj));
	IntfObj_Srv_Setup(AI_Hndl->IO_Hndl);
	
	AI_Hndl->GO_Hndl = GrpSrv_Init(&AI_Hndl->GOsrv_obj, sizeof(AI_Hndl->GOsrv_obj));
	GrpSrv_Setup(AI_Hndl->GO_Hndl, AI_Hndl->IO_Hndl);
	
	AI_Hndl->addressTableObjectHndl = AI_Hndl->GO_Hndl;
	AI_Hndl->AI_addressTableObject_ind = &GrpSrv_GrpAddrTblPropInd;
	
	AI_Hndl->associationTableObjectHndl = AI_Hndl->GO_Hndl;
	AI_Hndl->AI_associationTableObject_ind = &GrpSrv_AssocTblPropInd;
	
	AI_Hndl->groupObjectTableObjectHndl = AI_Hndl->GO_Hndl;
	AI_Hndl->AI_groupObjectTableObject_ind = &GrpSrv_GrpObjTblPropInd;

	AI_Hndl->AL_Hndl = A_LayerInit(&AI_Hndl->A_Layer, sizeof(AI_Hndl->A_Layer));
	A_LayerSetup(AI_Hndl->AL_Hndl, AI_Hndl->GO_Hndl, AI_Hndl);
	
//	AI_Hndl->A_Layer.T_layer.N_layer.ownAddressParentHndl = AI_Hndl;
//	AI_Hndl->A_Layer.T_layer.N_layer.N_Own_Address_Chgd_cb = &N_OwnAddress_ind;
	
	// group value read call back functions
	AI_Hndl->AL_Hndl->A_GroupValue_Read_Lcon_cb = &AI_GroupValue_Read_Lcon_cb;
	AI_Hndl->AL_Hndl->A_GroupValue_Read_ind_cb = &AI_GroupValue_Read_ind_cb;
	AI_Hndl->AL_Hndl->A_GroupValue_Read_Rcon_cb = &AI_GroupValue_Read_Rcon_cb;
	AI_Hndl->AL_Hndl->A_GroupValue_Read_Acon_cb = &AI_GroupValue_Read_Acon_cb;
	
	// group value write call back functions
	AI_Hndl->AL_Hndl->A_GroupValue_Write_ind_cb = &AI_GroupValue_Write_ind_cb;
	AI_Hndl->AL_Hndl->A_GroupValue_Write_Lcon_cb = &AI_GroupValue_Write_Lcon_cb;
	
	// device descriptor call back functions
	AI_Hndl->AL_Hndl->A_Device_Descriptor_Read_Lcon_cb = &AI_Device_Descriptor_Read_Lcon_cb;
	AI_Hndl->AL_Hndl->A_Device_Descriptor_Read_ind_cb = &AI_Device_Descriptor_Read_ind_cb;
	AI_Hndl->AL_Hndl->A_Device_Descriptor_Read_Acon_cb = &AI_Device_Descriptor_Read_Acon_cb;
	
	// property value call back functions
	AI_Hndl->AL_Hndl->A_Property_Value_Read_Lcon_cb = &AI_Property_Value_Read_Lcon_cb;
	AI_Hndl->AL_Hndl->A_Property_Value_Read_ind_cb = &AI_Property_Value_Read_ind_cb;
	AI_Hndl->AL_Hndl->A_Property_Value_Read_Acon_cb = &AI_Property_Value_Read_Acon_cb;
	AI_Hndl->AL_Hndl->A_Property_Value_Write_Lcon_cb = &AI_Property_Value_Write_Lcon_cb;
	AI_Hndl->AL_Hndl->A_Property_Value_Write_ind_cb = &AI_Property_Value_Write_ind_cb;
	
	// property description call back functions
	AI_Hndl->AL_Hndl->A_Property_Description_Read_Lcon_cb = &AI_Property_Description_Read_Lcon_cb;
	AI_Hndl->AL_Hndl->A_Property_Description_Read_ind_cb = &AI_Property_Description_Read_ind_cb;
	AI_Hndl->AL_Hndl->A_Property_Description_Read_Acon_cb = &AI_Property_Description_Read_Acon_cb;
	
	// Memory read / write functions
	AI_Hndl->AL_Hndl->A_Memory_Read_Lcon_cb = &AI_Memory_Read_Lcon_cb;
	AI_Hndl->AL_Hndl->A_Memory_Read_ind_cb = &AI_Memory_Read_ind_cb;
	AI_Hndl->AL_Hndl->A_Memory_Write_Lcon_cb = &AI_Memory_Write_Lcon_cb;
	AI_Hndl->AL_Hndl->A_Memory_Write_Acon_cb = &AI_Memory_Write_Acon_cb;
	AI_Hndl->AL_Hndl->A_Memory_Write_ind_cb = &AI_Memory_Write_ind_cb;
		
	// ADC read functions
	AI_Hndl->AL_Hndl->A_ADC_Read_Lcon_cb =&AI_ADC_Read_Lcon_cb;
	AI_Hndl->AL_Hndl->A_ADC_Read_Acon_cb = &AI_ADC_Read_Acon_cb;
	AI_Hndl->AL_Hndl->A_ADC_Read_ind_cb = &AI_ADC_Read_ind_cb;

	// authorize service call back functions
	AI_Hndl->AL_Hndl->A_Authorize_Request_ind_cb = &AI_Authorize_Request_ind_cb;
	AI_Hndl->AL_Hndl->A_Authorize_Request_Acon_cb = &AI_Authorize_Request_Acon_cb;
	
	// individual address write call back functions
	AI_Hndl->AL_Hndl->A_IndividualAddress_Write_Lcon_cb = &AI_IndividualAddress_Write_Lcon_cb;
	AI_Hndl->AL_Hndl->A_IndividualAddress_Write_ind_cb = &AI_IndividualAddress_Write_ind_cb;
	AI_Hndl->AL_Hndl->A_IndividualAddress_Read_Lcon_cb = &AI_IndividualAddress_Read_Lcon_cb;
	AI_Hndl->AL_Hndl->A_IndividualAddress_Read_Acon_cb = &AI_IndividualAddress_Read_Acon_cb;
	AI_Hndl->AL_Hndl->A_IndividualAddress_Read_Rcon_cb = &AI_IndividualAddress_Read_Rcon_cb;
	AI_Hndl->AL_Hndl->A_IndividualAddress_Read_ind_cb = &AI_IndividualAddress_Read_ind_cb;
		
	AI_Hndl->AL_Hndl->A_IndividualAddressSerialNumber_Write_Lcon_cb = &AI_IndividualAddressSerialNumber_Write_Lcon_cb;
	AI_Hndl->AL_Hndl->A_IndividualAddressSerialNumber_Write_ind_cb = &AI_IndividualAddressSerialNumber_Write_ind_cb;
	AI_Hndl->AL_Hndl->A_IndividualAddressSerialNumber_Read_Lcon_cb = &AI_IndividualAddressSerialNumber_Read_Lcon_cb;
	AI_Hndl->AL_Hndl->A_IndividualAddressSerialNumber_Read_Acon_cb = &AI_IndividualAddressSerialNumber_Read_Acon_cb;
	AI_Hndl->AL_Hndl->A_IndividualAddressSerialNumber_Read_Rcon_cb = &AI_IndividualAddressSerialNumber_Read_Rcon_cb;
	AI_Hndl->AL_Hndl->A_IndividualAddressSerialNumber_Read_ind_cb = &AI_IndividualAddressSerialNumber_Read_ind_cb;
	
	// Restart call back functions
	AI_Hndl->AL_Hndl->A_Restart_ind_cb = &AI_Restart_ind_cb;
	
	AI_Hndl->enable = 1;
	return 0;
}

void AI_Service(A_InterfaceHandle handle) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)handle;
	if (!AI_Hndl->enable)
		return;
	A_service(AI_Hndl->AL_Hndl);
}
/*
void AI_OwnAddress_set(A_InterfaceHandle handle, uint16_t ownAddress) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)handle;
	if (!AI_Hndl->enable || !ownAddress)
		return;
	if (AI_Hndl->A_Layer.T_layer.session.conn_state != TL_CONN_CLOSED)
		return;
	N_set_ownAddress(AI_Hndl->A_Layer.T_layer.N_layerHnd, ownAddress);
}

void N_OwnAddress_ind(void *parentHndl, uint16_t ownAddress) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parentHndl;

	IntfObj_setPropValue(IntfObj_Srv_getObjByPropId(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_SUBNET_ADDR), 1, 1, (uint8_t*)&(ownAddress) + 1);
	IntfObj_setPropValue(IntfObj_Srv_getObjByPropId(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_DEVICE_ADDR), 1, 1, (uint8_t*)&(ownAddress));

	if (AI_Hndl->AI_OwnAddress_ind)
		AI_Hndl->AI_OwnAddress_ind(AI_Hndl->AI_OwnAddressHndl, ownAddress);
}
*/
void AI_Group_Write_Req(A_InterfaceHandle handle, uint16_t grpObjIdx) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)handle;
	if (!AI_Hndl->enable)
		return;	
		
	group_obj *grpObj = grpSrv_getGroupObject(AI_Hndl->GO_Hndl, grpObjIdx);
	
	if (GrpObj_getCommFlags(grpObj) & COMM_FLAG_WRITE_REQUEST)
	{
		GrpObj_clrCommFlags(grpObj, COMM_FLAG_WRITE_REQUEST);
		GrpObj_setCommFlags(grpObj, COMM_FLAG_TRANSMITTING);
		
		A_Data_Group_s A_Data_Group_req;
		
		A_Data_Group_req.ack_request = 1;
		A_Data_Group_req.a_status = A_OK;
		A_Data_Group_req.ASAP = grpObjIdx;
		A_Data_Group_req.hop_count_type = 6;
		A_Data_Group_req.priority = GrpSrv_getGrpObjPriority(AI_Hndl->GO_Hndl, grpObjIdx);
		
		// get group object data type
		grpObjSz_e size = GrpSrv_getGrpObjSize(AI_Hndl->GO_Hndl, grpObjIdx);

		uint16_t value;
		GrpObj_GetValue(grpObj, &value);

		uint8_t data[16];
			
		if (size < GRP_OBJ_SZ_7_BITS)
		{
			data[0] = value & 0x3f;
		}
		else if (size > GRP_OBJ_SZ_6_BITS && size < GRP_OBJ_SZ_2_BYTES)
		{
			data[0] = value & 0xff;
		}
		else
		{
			data[0] = value >> 8;
			data[1] = value & 0xff;
		}
			
		A_Data_Group_req.asdu = data;
		
		A_GroupValue_Write_req(AI_Hndl->AL_Hndl, &A_Data_Group_req);
	}
}

void AI_Group_Read_Req(A_InterfaceHandle handle, uint16_t grpObjIdx) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)handle;
	if (!AI_Hndl->enable)
		return;
	
	group_obj *grpObj = grpSrv_getGroupObject(AI_Hndl->GO_Hndl, grpObjIdx);
	
	if (GrpObj_getCommFlags(grpObj) & COMM_FLAG_READ_REQUEST)
	{	
		GrpObj_clrCommFlags(grpObj, COMM_FLAG_READ_REQUEST);
		GrpObj_setCommFlags(grpObj, COMM_FLAG_TRANSMITTING);
		
		A_Data_Group_s A_Data_Group_req;
		
		A_Data_Group_req.ack_request = 1;
		A_Data_Group_req.a_status = A_OK;
		A_Data_Group_req.ASAP = grpObjIdx;
		A_Data_Group_req.hop_count_type = 6;
		A_Data_Group_req.priority = GrpSrv_getGrpObjPriority(AI_Hndl->GO_Hndl, grpObjIdx);
		
		A_GroupValue_Read_req(AI_Hndl->AL_Hndl, &A_Data_Group_req);	
	}
}

void AI_Group_Read_Res(A_InterfaceHandle handle, uint16_t grpObjIdx) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)handle;
	if (!AI_Hndl->enable)
		return;
		
	group_obj *grpObj = grpSrv_getGroupObject(AI_Hndl->GO_Hndl, grpObjIdx);
	
	if (GrpObj_getCommFlags(grpObj) & COMM_FLAG_READ_RESPONSE)
	{
		GrpObj_clrCommFlags(grpObj, COMM_FLAG_READ_RESPONSE);
		
		A_Data_Group_s A_Data_Group_res;
		
		A_Data_Group_res.ack_request = 1;
		A_Data_Group_res.a_status = A_OK;
		A_Data_Group_res.ASAP = grpObjIdx;
		A_Data_Group_res.hop_count_type = 6;
		A_Data_Group_res.priority = GrpSrv_getGrpObjPriority(AI_Hndl->GO_Hndl, grpObjIdx);
		
		// get group object data type
		grpObjSz_e size = GrpSrv_getGrpObjSize(AI_Hndl->GO_Hndl, grpObjIdx);

		uint16_t value;
		GrpObj_GetValue(grpObj, &value);

		uint8_t data[16];
		
		if (size < GRP_OBJ_SZ_7_BITS)
		{
			data[0] = value & 0x3f;
		}
		else if (size > GRP_OBJ_SZ_6_BITS && size < GRP_OBJ_SZ_2_BYTES)
		{
			data[0] = value & 0xff;
		}
		else
		{
			data[0] = value >> 8;
			data[1] = value & 0xff;
		}
		
		A_Data_Group_res.asdu = data;
		
		A_GroupValue_Read_res(AI_Hndl->AL_Hndl, &A_Data_Group_res);
	}
}



// group value read call back functions
void AI_GroupValue_Read_Lcon_cb(void *parent_Hndl, A_Data_Group_s *A_Data_Lcon) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;
	
	group_obj *grpObj = grpSrv_getGroupObject(AI_Hndl->GO_Hndl, A_Data_Lcon->ASAP);
	
	GrpObj_clrCommFlags(grpObj, COMM_FLAG_TRANSMITTING);
	GrpObj_setCommFlags(grpObj, COMM_FLAG_OK);
}

void AI_GroupValue_Read_ind_cb(void *parent_Hndl, A_Data_Group_s *A_Data_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;
	
	if (A_Data_ind->ASAP)
	{
		// get group object flags
		uint8_t configFlags = GrpSrv_getGrpObjConfigFlags(AI_Hndl->GO_Hndl, A_Data_ind->ASAP);
		
		if ((configFlags & (FLAG_COMMUNICATE | FLAG_READ)) == (FLAG_COMMUNICATE | FLAG_READ))
		{
			group_obj *grpObj = grpSrv_getGroupObject(AI_Hndl->GO_Hndl, A_Data_ind->ASAP);
			if (GrpObj_is_Read_ind_cb_registered(grpObj))
			{
				GrpObj_setCommFlags(grpObj, COMM_FLAG_READ_RESPONSE);
				GrpObj_Call_Read_ind_cb(grpObj);
			}
			else
			{
				// get group object data type
				grpObjSz_e size = GrpSrv_getGrpObjSize(AI_Hndl->GO_Hndl, A_Data_ind->ASAP);

				uint16_t value;
				GrpObj_GetValue(grpObj, &value);

				uint8_t data[16];
			
				if (size < GRP_OBJ_SZ_7_BITS)
				{
					data[0] = value & 0x3f;
				}
				else if (size > GRP_OBJ_SZ_6_BITS && size < GRP_OBJ_SZ_2_BYTES)
				{
					data[0] = value & 0xff;
				}
				else
				{
					data[0] = value >> 8;
					data[1] = value & 0xff;
				}
			
				A_Data_ind->asdu = data;

				A_GroupValue_Read_res(AI_Hndl->AL_Hndl, A_Data_ind);				
			}
		}
	}
}

void AI_GroupValue_Read_Rcon_cb(void *parent_Hndl, A_Data_Group_s *A_Data_Rcon) {
	
}

void AI_GroupValue_Read_Acon_cb(void *parent_Hndl, A_Data_Group_s *A_Data_Acon) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;
	
	if (A_Data_Acon->ASAP)
	{
		group_obj *grpObj = grpSrv_getGroupObject(AI_Hndl->GO_Hndl, A_Data_Acon->ASAP);
		
		GrpObj_clrCommFlags(grpObj, COMM_FLAG_TRANSMITTING);
		
		if (A_Data_Acon->a_status == A_OK)
		{
			// get group object data type
			grpObjSz_e size = GrpSrv_getGrpObjSize(AI_Hndl->GO_Hndl, A_Data_Acon->ASAP);

			uint8_t value[16] = {0,0};

			switch (size) {
				case GRP_OBJ_SZ_1_BIT:
				case GRP_OBJ_SZ_2_BITS:
				case GRP_OBJ_SZ_3_BITS:
				case GRP_OBJ_SZ_4_BITS:
				case GRP_OBJ_SZ_5_BITS:
				case GRP_OBJ_SZ_6_BITS:
				if (A_Data_Acon->asdu_byteCnt == 1)
					value[0] = (A_Data_Acon->asdu[0] & 0x3f);
				break;
				case GRP_OBJ_SZ_7_BITS:
				case GRP_OBJ_SZ_1_BYTE:
				if (A_Data_Acon->asdu_byteCnt == 1)
					value[0] = A_Data_Acon->asdu[1];
				break;
				case GRP_OBJ_SZ_2_BYTES:
				if (A_Data_Acon->asdu_byteCnt == 2)
				{
					value[0] = A_Data_Acon->asdu[2];
					value[1] = A_Data_Acon->asdu[1];
				}
				break;
				default:
				break;
			}

			if (GrpObj_UpdateValue(grpObj, *(((uint16_t*)value)), size) >= 0 )
				GrpObj_setCommFlags(grpObj, COMM_FLAG_OK);
			else
				GrpObj_setCommFlags(grpObj, COMM_FLAG_ERROR);
				
			if (GrpObj_is_Write_ind_cb_registered(grpObj))
			{
				GrpObj_setCommFlags(grpObj, COMM_FLAG_UPDATE);
				GrpObj_Call_Write_ind_cb(grpObj);
			}

		}
		else
		{
			GrpObj_setCommFlags(grpObj, COMM_FLAG_ERROR);		
		}

	}	
}

// group value write call back functions
void AI_GroupValue_Write_Lcon_cb(void *parent_Hndl, A_Data_Group_s *A_Data_Lcon) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;
	
	if (A_Data_Lcon->ASAP)
	{
		group_obj *grpObj = grpSrv_getGroupObject(AI_Hndl->GO_Hndl, A_Data_Lcon->ASAP);
		
		GrpObj_clrCommFlags(grpObj, COMM_FLAG_TRANSMITTING);
		
		if (A_Data_Lcon->a_status == A_OK)
		{
			// get group object data type
			grpObjSz_e size = GrpSrv_getGrpObjSize(AI_Hndl->GO_Hndl, A_Data_Lcon->ASAP);

			uint8_t value[16] = {0,0};

			switch (size) {
				case GRP_OBJ_SZ_1_BIT:
				case GRP_OBJ_SZ_2_BITS:
				case GRP_OBJ_SZ_3_BITS:
				case GRP_OBJ_SZ_4_BITS:
				case GRP_OBJ_SZ_5_BITS:
				case GRP_OBJ_SZ_6_BITS:
				if (A_Data_Lcon->asdu_byteCnt == 1)
					value[0] = (A_Data_Lcon->asdu[0] & 0x3f);
				break;
				case GRP_OBJ_SZ_7_BITS:
				case GRP_OBJ_SZ_1_BYTE:
				if (A_Data_Lcon->asdu_byteCnt == 1)
					value[0] = A_Data_Lcon->asdu[0];
				break;
				case GRP_OBJ_SZ_2_BYTES:
				if (A_Data_Lcon->asdu_byteCnt == 2)
				{
					value[0] = A_Data_Lcon->asdu[2];
					value[1] = A_Data_Lcon->asdu[1];
				}
				break;
				default:
				break;
			}

			if (GrpObj_UpdateValue(grpObj, *(((uint16_t*)value)), size) >= 0)
				GrpObj_setCommFlags(grpObj, COMM_FLAG_OK);			
			else
				GrpObj_setCommFlags(grpObj, COMM_FLAG_ERROR);
		}
		else
		{
			GrpObj_setCommFlags(grpObj, COMM_FLAG_ERROR);		
		}

	}
}

void AI_GroupValue_Write_ind_cb(void *parent_Hndl, A_Data_Group_s *A_Data_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;

	if (A_Data_ind->ASAP)
	{
		// get group object flags
		uint8_t configFlags = GrpSrv_getGrpObjConfigFlags(AI_Hndl->GO_Hndl, A_Data_ind->ASAP);
		
		if ((configFlags & (FLAG_WRITE | FLAG_COMMUNICATE)) == (FLAG_WRITE | FLAG_COMMUNICATE))
		{
			group_obj *grpObj = grpSrv_getGroupObject(AI_Hndl->GO_Hndl, A_Data_ind->ASAP);
			
			if (A_Data_ind->a_status == A_OK)
			{
				// get group object data type
				grpObjSz_e size = GrpSrv_getGrpObjSize(AI_Hndl->GO_Hndl, A_Data_ind->ASAP);

				uint8_t value[16] = {0,0};

				switch (size) {
					case GRP_OBJ_SZ_1_BIT:
					case GRP_OBJ_SZ_2_BITS:
					case GRP_OBJ_SZ_3_BITS:
					case GRP_OBJ_SZ_4_BITS:
					case GRP_OBJ_SZ_5_BITS:
					case GRP_OBJ_SZ_6_BITS:
					if (A_Data_ind->asdu_byteCnt == 1)
						value[0] = (A_Data_ind->asdu[0] & 0x3f);
					break;
					case GRP_OBJ_SZ_7_BITS:
					case GRP_OBJ_SZ_1_BYTE:
					if (A_Data_ind->asdu_byteCnt == 1)
						value[0] = A_Data_ind->asdu[0];
					break;
					case GRP_OBJ_SZ_2_BYTES:
					if (A_Data_ind->asdu_byteCnt == 2)
					{
						value[0] = A_Data_ind->asdu[2];
						value[1] = A_Data_ind->asdu[1];
					}
					break;
					default:
					break;
				}
			
				GrpObj_UpdateValue(grpObj, *(((uint16_t*)value)), size);

				if (GrpObj_is_Write_ind_cb_registered(grpObj))
				{
					GrpObj_setCommFlags(grpObj, COMM_FLAG_UPDATE);
					GrpObj_Call_Write_ind_cb(grpObj);
				}

			
				
			}
			/*else
			{
				GrpSrv_setGrpObjCommFlags(AI_Hndl->GO_Hndl, A_Data_ind->ASAP, COMM_FLAG_ERROR);
			}*/
		}

	}
}


// device descriptor call back functions
void AI_Device_Descriptor_Read_Lcon_cb(void *parent_Hndl, A_Device_Descriptor_s *A_Dev_Descr_Lcon) {
	
}
void AI_Device_Descriptor_Read_ind_cb(void *parent_Hndl, A_Device_Descriptor_s *A_Dev_Descr_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;	

	uint8_t deviceDescriptor[2];
	A_Dev_Descr_ind->device_descriptor = deviceDescriptor;

	if (A_Dev_Descr_ind->deviceDescriptorType == 0) {
		interfaceProperty *propStored = IntfObj_Srv_getObjByPropId(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_DEVICE_DESCRIPTOR); 
		
		A_Dev_Descr_ind->device_descriptor_octet_count = 2;
		*(uint16_t*)(&deviceDescriptor) = *((uint16_t*)propStored->elements + 1);
		
	}
	else
	{
		A_Dev_Descr_ind->device_descriptor_octet_count = 1;
		deviceDescriptor[0] = 0x3f; // descriptor type not equal to 0 are not supported.
	}
	
	A_Dev_Descr_ind->priority = KNX_SYSTEM;
	
	A_Device_Descriptor_res(AI_Hndl->AL_Hndl, A_Dev_Descr_ind);
}


void AI_Device_Descriptor_Read_Acon_cb(void *parent_Hndl, A_Device_Descriptor_s *A_Dev_Descr_Acon) {
	
}
// property value call back functions
void AI_Property_Value_Read_Lcon_cb(void *parent_Hndl, A_Property_Value_s *A_Property_val_Lcon) {
	
}

void AI_Property_Value_Read_ind_cb(void *parent_Hndl, A_Property_Value_s *A_Property_val_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;	

	uint8_t data[MAX_TSDU_LENGTH];
	A_Property_val_ind->data = data;

	interfaceProperty *propStored = IntfObj_Srv_getObjByPropId(AI_Hndl->IO_Hndl, A_Property_val_ind->objectIndex, A_Property_val_ind->propertyId);
	if (propStored)
	{
		if (propStored->elements && propStored->propertyDataType != PDT_FUNCTION)
		{
			if (!A_Property_val_ind->startIndex) // start index 0 gets the number of elements in  use
			{
				if ((propStored->flags & PROP_FLAG_STORED_IN_PROGMEM) != 0)
				{
					*(uint16_t*)data = pgm_read_dword(propStored->elements);
				}
				else
				{
					*(uint16_t*)data = *(uint16_t*)propStored->elements;	
				}
				A_Property_val_ind->data_byteCnt = 2;
			}
			else
			{
				uint16_t start = A_Property_val_ind->startIndex - 1;
				if (propStored->maxElements >= (start + A_Property_val_ind->count))
				{
					uint8_t elementSize = (propStored->propertyDataType == PDT_CONTROL) ? 1 : getPDTsize(propStored->propertyDataType);
					uint16_t ms = (SWAP_UINT16(*(AI_Hndl->AL_Hndl->T_layerHnd->maxTDSULength)) - 6);
					if ((A_Property_val_ind->count * elementSize) <= ms)
					{
						
						uint8_t cnt = 0;
						if ((propStored->flags & PROP_FLAG_STORED_IN_PROGMEM) != 0)
						{
							uint8_t *element = IntfObj_getPropValue_P(propStored, A_Property_val_ind->startIndex);
							for (; cnt < (A_Property_val_ind->count * elementSize); cnt++)
								data[cnt] = pgm_read_byte(element + cnt);
						}
						else
						{
							uint8_t *element = IntfObj_getPropValue(propStored, A_Property_val_ind->startIndex);
							for (; cnt < (A_Property_val_ind->count * elementSize); cnt++)
								data[cnt] = *(element + cnt);
						}

						A_Property_val_ind->data_byteCnt = cnt;
					}
					else
						A_Property_val_ind->count = 0; // error : too many elements requested. Will not fit in available bytes for one message
				}
				else
					A_Property_val_ind->count = 0; // error : requested number of elements is more then maximum elements in property object
			}
		}
		else
		{
			if (!A_Property_val_ind->startIndex) // start index 0 gets the number of elements in use
			{
				*(uint16_t*)data = 0; // no elements are available. return current element count 0
				A_Property_val_ind->data_byteCnt = 2;
			}
			else
				A_Property_val_ind->count = 0; // error: no elements are available
		}
	}
	else
		A_Property_val_ind->count = 0; // error: invalid object index and or property id

	A_Property_Value_Read_res(AI_Hndl->AL_Hndl, A_Property_val_ind);
}

void AI_Property_Value_Read_Acon_cb(void *parent_Hndl, A_Property_Value_s *A_Property_val_Acon) {
	
}

void AI_Property_Value_Write_Lcon_cb(void *parent_Hndl, A_Property_Value_s *A_Property_val_Lcon) {
	//A_InterfaceObj *AI_Hndl;
	//AI_Hndl = (A_InterfaceObj *)parent_Hndl;
		
}

void AI_Property_Value_Write_ind_cb(void *parent_Hndl, A_Property_Value_s *A_Property_val_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;	

	PDT_e propDataType = getPDTfromPropType(A_Property_val_ind->propertyId);
	if (propDataType == PDT_UNKNOWN)
	{
		interfaceProperty *prop = IntfObj_Srv_getObjByPropId(AI_Hndl->IO_Hndl, A_Property_val_ind->objectIndex, A_Property_val_ind->propertyId);
		if (prop != NULL)
			propDataType = prop->propertyDataType;
	}

	uint16_t expectedByteCnt = getPDTsize(propDataType) * A_Property_val_ind->count;
	interfaceProperty *prop = IntfObj_Srv_getObjByPropId(AI_Hndl->IO_Hndl, A_Property_val_ind->objectIndex, A_Property_val_ind->propertyId);
	
	if (IntfObj_isWriteEnable(prop) && expectedByteCnt == A_Property_val_ind->data_byteCnt)
	{
		interfaceProperty propInd;
		intfObj_Clone(prop, &propInd);
		propInd.elements = A_Property_val_ind->data;
		uint8_t validPropValue = 1;

		switch(A_Property_val_ind->objectIndex)
		{
			case DEVICE_OBJECT_IDX:
			if (AI_Hndl->AI_deviceObject_ind)
				validPropValue = AI_Hndl->AI_deviceObject_ind(AI_Hndl->deviceObjectHndl, &propInd);
			break;
			case ADDRESS_TABLE_OBJECT_IDX:
			if (AI_Hndl->AI_addressTableObject_ind)
				validPropValue = AI_Hndl->AI_addressTableObject_ind(AI_Hndl->addressTableObjectHndl, &propInd);
			break;
			case ASSOCIATION_TABLE_OBJECT_IDX:
			if (AI_Hndl->AI_associationTableObject_ind)
					validPropValue = AI_Hndl->AI_associationTableObject_ind(AI_Hndl->associationTableObjectHndl, &propInd);
			break;
			case GROUP_OBJECT_TABLE_OBJECT_IDX:
				if (AI_Hndl->AI_groupObjectTableObject_ind)
					validPropValue = AI_Hndl->AI_groupObjectTableObject_ind(AI_Hndl->groupObjectTableObjectHndl, &propInd);
			break;
			case APPLICATION_1_OBJECT_IDX:
				if (AI_Hndl->AI_application1Object_ind)
					validPropValue = AI_Hndl->AI_application1Object_ind(AI_Hndl->application1ObjectHndl, &propInd);
			break;
			case APPLICATION_2_OBJECT_IDX:
				if (AI_Hndl->AI_application2Object_ind)
					validPropValue = AI_Hndl->AI_application2Object_ind(AI_Hndl->application2ObjectHndl, &propInd);
			break;
			case TOUCHPAD_CFG_OBJECT_IDX:
				if (AI_Hndl->AI_touchpadCfgObject_ind)
					validPropValue = AI_Hndl->AI_touchpadCfgObject_ind(AI_Hndl->touchpadCfgObjectHndl, &propInd);
			break;
			case ACTION_CFG_OBJECT_IDX:
				if (AI_Hndl->AI_actionCfgObject_ind)
					validPropValue = AI_Hndl->AI_actionCfgObject_ind(AI_Hndl->actionCfgObjectHndl, &propInd);
			break;
			case RGB_LED_CFG_OBJECT_IDX:
				if (AI_Hndl->AI_rgbLedCfgObject_ind)
					validPropValue = AI_Hndl->AI_rgbLedCfgObject_ind(AI_Hndl->rgbLedCfgObjectHndl, &propInd);
			break;
			case AMBIENT_SENSOR_CFG_OBJECT_IDX:
				if (AI_Hndl->AI_ambientSensorCfgObject_ind)
					validPropValue = AI_Hndl->AI_ambientSensorCfgObject_ind(AI_Hndl->ambientSensorCfgObjectHndl, &propInd);
			break;
			default:
			break;
		}
		
		if (validPropValue == 1)
			A_Property_val_ind->data_byteCnt = IntfObj_setPropValue(prop, A_Property_val_ind->startIndex, A_Property_val_ind->count, A_Property_val_ind->data);
		else
			A_Property_val_ind->count = 0;	
	}
	else
		A_Property_val_ind->count = 0;	


	A_Property_Value_Read_res(AI_Hndl->AL_Hndl, A_Property_val_ind); // after write reply with property response (auto confirmation).
}

// property description call back functions
void AI_Property_Description_Read_Lcon_cb(void *parent_Hndl, A_Property_Description_s *A_property_descr_Lcon) {
	
}

void AI_Property_Description_Read_ind_cb(void *parent_Hndl, A_Property_Description_s *A_property_descr_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;	
	
	interfaceProperty *propStored = NULL;

	if (A_property_descr_ind->propertyId)
		propStored = IntfObj_Srv_getObjByPropId(AI_Hndl->IO_Hndl, A_property_descr_ind->objectIndex, A_property_descr_ind->propertyId);
	else
		propStored = IntfObj_Srv_getObjByPropIndex(AI_Hndl->IO_Hndl, A_property_descr_ind->objectIndex, A_property_descr_ind->propertyIndex);
	
	if (propStored)
	{
		A_property_descr_ind->propertyId = propStored->propertyId;
		A_property_descr_ind->propertyIndex = propStored->propertyIndex;
		A_property_descr_ind->writeEnable = propStored->flags;
		A_property_descr_ind->DataType = propStored->propertyDataType;
		A_property_descr_ind->maxElements = propStored->maxElements;
		A_property_descr_ind->accessLvl = propStored->readAccessLvl << 4 | (propStored->writeAccessLvl & 0xf);
	}
	else
	{
		A_property_descr_ind->maxElements = 0;
	}

	A_Property_Description_Read_res(AI_Hndl->AL_Hndl, A_property_descr_ind);	
}

void AI_Property_Description_Read_Acon_cb(void *parent_Hndl, A_Property_Description_s *A_property_descr_Acon) {
	
}

// Memory read / write functions
void AI_Memory_Read_Lcon_cb(void *parent_Hndl, A_Memory_s *A_Memory_Lcon) {
	
}
void AI_Memory_Read_ind_cb(void *parent_Hndl, A_Memory_s *A_Memory_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;	
	
	A_Memory_ind->data = (uint8_t *)A_Memory_ind->memoryAddress;
	A_Memory_Read_res(AI_Hndl->AL_Hndl, A_Memory_ind);
}

void AI_Memory_Write_Lcon_cb(void *parent_Hndl, A_Memory_s *A_Memory_Lcon) {
	
}
void AI_Memory_Write_Acon_cb(void *parent_Hndl, A_Memory_s *A_Memory_Acon) {
	
}
void AI_Memory_Write_ind_cb(void *parent_Hndl, A_Memory_s *A_Memory_ind) {

}
	
	
// ADC read functions
void AI_ADC_Read_Lcon_cb(void *parent_Hndl, A_ADC_read_s *A_ADC_read_Lcon) {
	
}
void AI_ADC_Read_Acon_cb(void *parent_Hndl, A_ADC_read_s *A_ADC_read_Acon) {
	
}
void AI_ADC_Read_ind_cb(void *parent_Hndl, A_ADC_read_s *A_ADC_read_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;

	uint16_t vbus = 0;
	if (A_ADC_read_ind->channel == 1) { // channel for getting the bus voltage. All other channels return 0.

		tranceiverReadReg(AI_Hndl->A_Layer.T_layer.N_layer.tranceiver_handle, E98103_ADC_VBUSP_MEAN, (uint8_t*)&vbus);
						
		vbus = SWAP_UINT16(vbus) & 0xff;
		vbus *= 165;
		vbus >>= 8;
						
		uint8_t shift = A_ADC_read_ind->readCount;
		while (shift) {
			shift >>= 1;
			vbus <<= 1;
		}
		A_ADC_read_ind->sum = vbus;
	}
					
	A_ADC_Read_res(AI_Hndl->AL_Hndl, A_ADC_read_ind);
}

// authorize service call back functions
void AI_Authorize_Request_ind_cb(void *parent_Hndl, A_Authorisation_s *A_Authorize_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;	


	A_Authorize_ind->hop_count_type = 6;
	A_Authorize_ind->level = 0;
	A_Authorize_Request_res(AI_Hndl->AL_Hndl, A_Authorize_ind);
}

void AI_Authorize_Request_Acon_cb(void *parent_Hndl, A_Authorisation_s *A_Authorize_Acon) {
	
}

// individual address write call back functions
void AI_IndividualAddress_Write_Lcon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Lcon) {
	
}
void AI_IndividualAddress_Write_ind_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;

	uint8_t *progmode = IntfObj_getPropValue(IntfObj_Srv_getObjByPropId(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_PROGMODE), 1);
	
	if (*progmode)
	{
		if (AI_Hndl->AI_ownAddr_ind)
			AI_Hndl->AI_ownAddr_ind(AI_Hndl->ownAddrIndHndl, A_IndividualAddr_ind->newAddress);
	}
}
void AI_IndividualAddress_Read_Lcon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Lcon) {
	
}
void AI_IndividualAddress_Read_Acon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Acon) {
	
}
void AI_IndividualAddress_Read_Rcon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Rcon) {
	
}
void AI_IndividualAddress_Read_ind_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;

	uint8_t *progmode = IntfObj_getPropValue(IntfObj_Srv_getObjByPropId(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_PROGMODE), 1);

	if (*progmode)
	{
		A_IndividualAddr_ind->individualAddress = N_get_ownAddress(AI_Hndl->A_Layer.T_layer.N_layerHnd);
		A_IndividualAddress_Read_res(AI_Hndl->AL_Hndl, A_IndividualAddr_ind);
	}	
}

void AI_IndividualAddressSerialNumber_Write_Lcon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Lcon) {
	
}
void AI_IndividualAddressSerialNumber_Write_ind_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;
	
	uint8_t *serial = IntfObj_getPropValue(IntfObj_Srv_getObjByPropId(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_SERIAL_NUMBER), 1);
	uint8_t match = 1;
	for (uint8_t i = 0; i < 6; i++)
	{
		if (A_IndividualAddr_ind->serialNumber[i] != serial[i])
			match = 0;
	}
	if (match)
	{
		if (AI_Hndl->AI_ownAddr_ind)
			AI_Hndl->AI_ownAddr_ind(AI_Hndl->ownAddrIndHndl, A_IndividualAddr_ind->newAddress);
	}
}

void AI_IndividualAddressSerialNumber_Read_Lcon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Lcon) {
	
}
void AI_IndividualAddressSerialNumber_Read_Acon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Acon) {
	
}
void AI_IndividualAddressSerialNumber_Read_Rcon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Rcon) {
	
}

void AI_IndividualAddressSerialNumber_Read_ind_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;
	uint8_t *serial = IntfObj_getPropValue(IntfObj_Srv_getObjByPropId(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_SERIAL_NUMBER), 1);
	uint8_t match = 1;
	for (uint8_t i = 0; i < 6; i++)
	{
		if (A_IndividualAddr_ind->serialNumber[i] != serial[i])
		match = 0;
	}
	if (match)
	{
		A_IndividualAddr_ind->individualAddress = N_get_ownAddress(AI_Hndl->A_Layer.T_layer.N_layerHnd);
		A_IndividualAddr_ind->domainAddress = 0;
		A_IndividualAddressSerialNumber_Read_res(AI_Hndl->AL_Hndl, A_IndividualAddr_ind);
	}	
}

void AI_Restart_ind_cb(void *parent_Hndl, A_Restart_s *A_Restart_ind) {
	A_InterfaceObj *AI_Hndl;
	AI_Hndl = (A_InterfaceObj *)parent_Hndl;
	
	if (A_Restart_ind->restartType)  // master reset
	{
		// todo : reset codes to be handled.
		
		A_Restart_ind->errorCode = 0;
		A_Restart_ind->processTime = 0;
		
		A_Restart_res(AI_Hndl->AL_Hndl, A_Restart_ind);	
	}
	else
	{
		uint8_t progmode = 0;
		IntfObj_setPropValue(IntfObj_Srv_getObjByPropId(AI_Hndl->IO_Hndl, DEVICE_OBJECT_IDX, PID_PROGMODE), 1, 1,&progmode);
		if (AI_Hndl->AI_Reset_ind)
			AI_Hndl->AI_Reset_ind();
	}
		
}
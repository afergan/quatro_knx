/*
 * applicationLayer.h
 *
 * Created: 25/08/16 15:18:13
 *  Author: PvR
 */ 


#ifndef APPLICATIONLAYER_H_
#define APPLICATIONLAYER_H_


#include "clocks.h"
#include "linkLayer.h"
#include "networkLayer.h"
#include "telegram.h"
#include "memoryObjects.h"
#include "transportLayer.h"
#include "groupObjectServer.h"

#define DEFAULT_ROUTE_CNT			6

typedef enum {
	A_NOT_OK,
	A_OK
} a_status_e;

typedef struct {
	uint8_t			ack_request;
	a_status_e		a_status;
	uint16_t		ASAP;
	priority_e		priority;
	uint8_t			hop_count_type;
	uint8_t			*asdu;
	uint8_t			asdu_byteCnt;
	} A_Data_Group_s;

typedef struct {
	uint8_t			ack_request;
	a_status_e		a_status;
	uint16_t		ASAP;
	priority_e		priority;
	uint8_t			hop_count_type;
	tl_conn_type_e	connType;
	uint8_t			deviceDescriptorType;
	uint8_t			*device_descriptor;
	uint8_t			device_descriptor_octet_count;	
	} A_Device_Descriptor_s;

typedef struct  {
	uint8_t			ack_request;
	a_status_e		a_status;
	uint16_t		ASAP;
	priority_e		priority;
	uint8_t			hop_count_type;
	tl_conn_type_e	connType;
	uint8_t			objectIndex;
	uint8_t			propertyId;
	uint16_t		startIndex;
	uint8_t			count;
	uint8_t			*data;
	uint8_t			data_byteCnt;
} A_Property_Value_s;

typedef struct  {
	uint8_t			ack_request;
	a_status_e		a_status;
	uint16_t		ASAP;
	priority_e		priority;
	uint8_t			hop_count_type;
	tl_conn_type_e	connType;
	uint8_t			objectIndex;
	uint8_t			propertyId;
	uint16_t		propertyIndex;
	uint8_t			DataType;
	uint8_t			accessLvl;
	uint8_t			writeEnable;
	uint16_t		maxElements;
} A_Property_Description_s;

typedef struct {
	uint8_t			ack_request;
	a_status_e		a_status;
	priority_e		priority;
	uint8_t			hop_count_type;
	uint16_t		newAddress;
	uint16_t		individualAddress;
	uint16_t		domainAddress;
	uint8_t			serialNumber[6];
} A_IndividualAddress_s;

typedef struct {
	uint8_t			ack_request;
	a_status_e		a_status;
	priority_e		priority;
	uint8_t			hop_count_type;
	tl_conn_type_e	connType;
	uint16_t		ASAP;
	uint16_t		level;
	uint8_t			key[4];
} A_Authorisation_s;

typedef struct {
	uint8_t			ack_request;
	a_status_e		a_status;
	priority_e		priority;
	uint8_t			hop_count_type;
	tl_conn_type_e	connType;
	uint16_t		ASAP;
	uint8_t			channel;
	uint8_t			readCount;
	uint16_t		sum;
} A_ADC_read_s;

typedef struct {
	uint8_t			ack_request;
	a_status_e		a_status;
	priority_e		priority;
	uint8_t			hop_count_type;
	tl_conn_type_e	connType;
	uint16_t		ASAP;
	uint8_t			number;
	uint16_t		memoryAddress;
	uint8_t			*data;
} A_Memory_s;

typedef struct {
	uint8_t			ack_request;
	a_status_e		a_status;
	priority_e		priority;
	uint8_t			hop_count_type;
	tl_conn_type_e	connType;
	uint16_t		ASAP;
	uint8_t			restartType;
	uint8_t			eraseCode;
	uint8_t			channelNumber;
	uint8_t			errorCode;
	uint16_t		processTime;
} A_Restart_s;

typedef struct {
	uint8_t						enable;
	void						*parent_Hndl;
	T_layerObj					T_layer;
	T_layerHandle				T_layerHnd;
	groupObjectServerHandle		grpSrv_handle;
	
	// Memory read / write functions
	void						(*A_Memory_Read_Lcon_cb)(void *parent_Hndl, A_Memory_s *A_Memory_Lcon);
	void						(*A_Memory_Read_ind_cb)(void *parent_Hndl, A_Memory_s *A_Memory_ind);
	void						(*A_Memory_Write_Lcon_cb)(void *parent_Hndl, A_Memory_s *A_Memory_Lcon);
	void						(*A_Memory_Write_Acon_cb)(void *parent_Hndl, A_Memory_s *A_Memory_Acon);
	void						(*A_Memory_Write_ind_cb)(void *parent_Hndl, A_Memory_s *A_Memory_ind);
	
	// ADC read functions
	void						(*A_ADC_Read_Lcon_cb)(void *parent_Hndl, A_ADC_read_s *A_ADC_read_Lcon);
	void						(*A_ADC_Read_Acon_cb)(void *parent_Hndl, A_ADC_read_s *A_ADC_read_Acon);
	void						(*A_ADC_Read_ind_cb)(void *parent_Hndl, A_ADC_read_s *A_ADC_read_ind);

	// authorize service call back functions
	void						(*A_Authorize_Request_ind_cb)(void *parent_Hndl, A_Authorisation_s *A_Authorize_ind);
	void						(*A_Authorize_Request_Acon_cb)(void *parent_Hndl, A_Authorisation_s *A_Authorize_Acon);
	
	// individual address write call back functions
	void						(*A_IndividualAddress_Write_Lcon_cb)(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Lcon);
	void						(*A_IndividualAddress_Write_ind_cb)(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_ind);
	void						(*A_IndividualAddress_Read_Lcon_cb)(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Lcon);
	void						(*A_IndividualAddress_Read_Acon_cb)(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Acon);
	void						(*A_IndividualAddress_Read_Rcon_cb)(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Rcon);
	void						(*A_IndividualAddress_Read_ind_cb)(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_ind);
	
	void						(*A_IndividualAddressSerialNumber_Write_Lcon_cb)(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Lcon);
	void						(*A_IndividualAddressSerialNumber_Write_ind_cb)(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_ind);
	void						(*A_IndividualAddressSerialNumber_Read_Lcon_cb)(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Lcon);
	void						(*A_IndividualAddressSerialNumber_Read_Acon_cb)(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Acon);
	void						(*A_IndividualAddressSerialNumber_Read_Rcon_cb)(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Rcon);
	void						(*A_IndividualAddressSerialNumber_Read_ind_cb)(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_ind);
	
	// group value read call back functions
	void						(*A_GroupValue_Read_Lcon_cb)(void *parent_Hndl, A_Data_Group_s *A_Data_Lcon);
	void						(*A_GroupValue_Read_ind_cb)(void *parent_Hndl, A_Data_Group_s *A_Data_ind);
	void						(*A_GroupValue_Read_Rcon_cb)(void *parent_Hndl, A_Data_Group_s *A_Data_Rcon);
	void						(*A_GroupValue_Read_Acon_cb)(void *parent_Hndl, A_Data_Group_s *A_Data_Acon);
	// group value write call back functions
	void						(*A_GroupValue_Write_Lcon_cb)(void *parent_Hndl, A_Data_Group_s *A_Data_Lcon);
	void						(*A_GroupValue_Write_ind_cb)(void *parent_Hndl, A_Data_Group_s *A_Data_ind);
	// device descriptor call back functions
	void						(*A_Device_Descriptor_Read_Lcon_cb)(void *parent_Hndl, A_Device_Descriptor_s *A_Dev_Decr_Lcon);
	void						(*A_Device_Descriptor_Read_ind_cb)(void *parent_Hndl, A_Device_Descriptor_s *A_Dev_Decr_ind);
	void						(*A_Device_Descriptor_Read_Acon_cb)(void *parent_Hndl, A_Device_Descriptor_s *A_Dev_Decr_Acon);
	// property value call back functions
	void						(*A_Property_Value_Read_Lcon_cb)(void *parent_Hndl, A_Property_Value_s *A_Property_val_Lcon);
	void						(*A_Property_Value_Read_ind_cb)(void *parent_Hndl, A_Property_Value_s *A_Property_val_ind);
	void						(*A_Property_Value_Read_Acon_cb)(void *parent_Hndl, A_Property_Value_s *A_Property_val_Acon);
	void						(*A_Property_Value_Write_Lcon_cb)(void *parent_Hndl, A_Property_Value_s *A_Property_val_Lcon);
	void						(*A_Property_Value_Write_ind_cb)(void *parent_Hndl, A_Property_Value_s *A_Property_val_ind);
	// property description call back functions
	void						(*A_Property_Description_Read_Lcon_cb)(void *parent_Hndl, A_Property_Description_s *A_property_descr_Lcon);
	void						(*A_Property_Description_Read_ind_cb)(void *parent_Hndl, A_Property_Description_s *A_property_descr_ind);
	void						(*A_Property_Description_Read_Acon_cb)(void *parent_Hndl, A_Property_Description_s *A_property_descr_Acon);
	// restart call back functions
	void						(*A_Restart_ind_cb)(void *parent_Hndl, A_Restart_s *A_Restart_ind);
} A_LayerObj;

typedef A_LayerObj *A_LayerHandle;


A_LayerHandle A_LayerInit(void *pMemory, uint16_t numBytes);

int8_t A_LayerSetup(A_LayerHandle handle, groupObjectServerHandle grpSrv_handle, void *parent_Hndl);

// Memory read / write functions
	
void A_Memory_Read_req(A_LayerHandle handle, A_Memory_s *A_Memory_req);

void A_Memory_Read_res(A_LayerHandle handle, A_Memory_s *A_Memory_res);

void A_Memory_Write_req(A_LayerHandle handle, A_Memory_s *A_Memory_req);

void A_Memory_Write_res(A_LayerHandle handle, A_Memory_s *A_Memory_res);

// ADC read functions

void A_ADC_Read_req(A_LayerHandle handle, A_ADC_read_s *A_ADC_read_req);

void A_ADC_Read_res(A_LayerHandle handle, A_ADC_read_s *A_ADC_read_res);

// authorization functions

void A_Authorize_Request_req(A_LayerHandle handle, A_Authorisation_s *A_Authorize_req);

void A_Authorize_Request_res(A_LayerHandle handle, A_Authorisation_s *A_Authorize_res);

// Individual Address functions

void A_IndividualAddress_Write_req(A_LayerHandle handle, A_IndividualAddress_s *A_IndividualAddr_req);

void A_IndividualAddress_Read_req(A_LayerHandle handle, A_IndividualAddress_s *A_IndividualAddr_req);

void A_IndividualAddress_Read_res(A_LayerHandle handle, A_IndividualAddress_s *A_IndividualAddr_req);

void A_IndividualAddressSerialNumber_Write_req(A_LayerHandle handle, A_IndividualAddress_s *A_IndividualAddr_req);

void A_IndividualAddressSerialNumber_Read_req(A_LayerHandle handle, A_IndividualAddress_s *A_IndividualAddr_req);

void A_IndividualAddressSerialNumber_Read_res(A_LayerHandle handle, A_IndividualAddress_s *A_IndividualAddr_res);

// Group value functions

void A_GroupValue_Read_req(A_LayerHandle handle, A_Data_Group_s *A_Data_req);		// read request

void A_GroupValue_Read_res(A_LayerHandle handle, A_Data_Group_s *A_Data_res);		// read response

void A_GroupValue_Write_req(A_LayerHandle handle, A_Data_Group_s *A_Data_req);	// write request

// device descriptor functions

void A_Device_Descriptor_res(A_LayerHandle handle, A_Device_Descriptor_s *A_Dev_Descr_res);	// device descriptor response

void A_Device_Descriptor_req(A_LayerHandle handle, A_Device_Descriptor_s *A_Dev_Descr_req);	// device descriptor request

// property value read / write functions

void A_Property_Value_Read_req(A_LayerHandle handle, A_Property_Value_s *A_Property_val_req);	// property read request

void A_Property_Value_Read_res(A_LayerHandle handle, A_Property_Value_s *A_Property_val_res);	// property read response

void A_Property_Value_Write_req(A_LayerHandle handle, A_Property_Value_s *A_Property_val_req);	// property write request

// property description read functions

void A_Property_Description_Read_req(A_LayerHandle handle, A_Property_Description_s *A_property_descr_req); // property description request

void A_Property_Description_Read_res(A_LayerHandle handle, A_Property_Description_s *A_property_descr_res); // property description response

// Restart functions

void A_Restart_req(A_LayerHandle handle, A_Restart_s *A_Restart_req);

void A_Restart_res(A_LayerHandle handle, A_Restart_s *A_Restart_res);


void A_service(A_LayerHandle handle);  // main loop service routine

void A_P2P_common_services(void *parent_Hndl, T_Data_s *T_Data_ind); // connection oriented and connection less

void A_P2P_connection_oriented_services(void *parent_Hndl, T_Data_s *T_Data_ind); // connection oriented

void A_Broadcast_services(void *parent_Hndl, T_Data_s *T_Data_ind);

void A_SystemBroadcast_services(void *parent_Hndl, T_Data_s *T_Data_ind);

// T Data indication call backs

void T_Data_Connected_ind_cb(void *parent_Hndl, T_Data_s *T_Data_ind);

void T_Data_Group_ind_cb(void *parent_Hndl, T_Data_s *T_Data_ind);

void T_Data_Broadcast_ind_cb(void *parent_Hndl, T_Data_s *T_Data_ind);

void T_Data_Individual_ind_cb(void *parent_Hndl, T_Data_s *T_Data_ind);

void T_Data_Tag_Group_ind_cb(void *parent_Hndl, T_Data_s *T_Data_ind);

void T_Data_systemBroadcast_ind_cb(void *parent_Hndl, T_Data_s *T_Data_ind);

void T_Connect_ind_cb(void *parent_Hndl, uint16_t TSAP);

void T_Disconnect_ind_cb(void *parent_Hndl, uint16_t TSAP);

// T Data confirmation call backs

void T_Data_Individual_con_cb(void *parent_Hndl, T_Data_s *T_Data_con);

void T_Data_Group_con_cb(void *parent_Hndl, T_Data_s *T_Data_con);

void T_Data_Tag_Group_con_cb(void *parent_Hndl, T_Data_s *T_Data_con);

void T_Data_Broadcast_con_cb(void *parent_Hndl, T_Data_s *T_Data_con);

void T_Data_systemBroadcast_con_cb(void *parent_Hndl, T_Data_s *T_Data_con);

void T_Data_Connected_con_cb(void *parent_Hndl, uint16_t TSAP);

void T_Connect_con_cb(void *parent_Hndl, uint16_t destination_address, uint16_t TSAP, t_status_e status);

void T_Disconnect_con_cb(void *parent_Hndl, uint16_t TSAP, priority_e priority, t_status_e status);

int8_t map_T_Data_to_A_Group_Data(A_Data_Group_s *A_Group_Data, T_Data_s *T_Data);


#endif /* APPLICATIONLAYER_H_ */
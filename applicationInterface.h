/*
 * applicationInterface.h
 *
 * Created: 30-8-2016 17:30:44
 *  Author: Paul
 */ 


#ifndef APPLICATIONINTERFACE_H_
#define APPLICATIONINTERFACE_H_

#include "clocks.h"
#include "telegram.h"
#include "applicationLayer.h"
#include "groupObjectServer.h"
#include "interfaceObjectServer.h"

typedef struct {
	uint8_t							enable;
	void							*parent_Hndl;
	interfaceObjectServerObj		IOsrv_obj;
	interfaceObjectServerHandle		IO_Hndl;
	groupObjectServerObj			GOsrv_obj;
	groupObjectServerHandle			GO_Hndl;
	A_LayerObj						A_Layer;
	A_LayerHandle					AL_Hndl;
	
// call back functions
	void							(*AI_Reset_ind)();
	
	void							*deviceObjectHndl;
	void							*addressTableObjectHndl;
	void							*associationTableObjectHndl;
	void							*groupObjectTableObjectHndl;
	void							*application1ObjectHndl;
	void							*application2ObjectHndl;
	void							*touchpadCfgObjectHndl;
	void							*actionCfgObjectHndl;
	void							*rgbLedCfgObjectHndl;
	void							*ambientSensorCfgObjectHndl;
	
	
	int8_t							(*AI_deviceObject_ind)(void *deviceObjectHndl, interfaceProperty *prop);
	int8_t							(*AI_addressTableObject_ind)(void *addressTableObjectHndl, interfaceProperty *prop);
	int8_t							(*AI_associationTableObject_ind)(void *associationTableObjectHndl, interfaceProperty *prop);
	int8_t							(*AI_groupObjectTableObject_ind)(void *groupObjectTableObjectHndl, interfaceProperty *prop);
	int8_t							(*AI_application1Object_ind)(void *application1ObjectHndl, interfaceProperty *prop);
	int8_t							(*AI_application2Object_ind)(void *application2ObjectHndl, interfaceProperty *prop);
	int8_t							(*AI_touchpadCfgObject_ind)(void *touchpadCfgObjectHndl, interfaceProperty *prop);
	int8_t							(*AI_actionCfgObject_ind)(void *actionCfgObjectHndl, interfaceProperty *prop);
	int8_t							(*AI_rgbLedCfgObject_ind)(void *rgbLedCfgObjectHndl, interfaceProperty *prop);
	int8_t							(*AI_ambientSensorCfgObject_ind)(void *ambientSensorCfgObjectHndl, interfaceProperty *prop);
	
	void							*ownAddrIndHndl;
	int8_t							(*AI_ownAddr_ind)(void *ownAddrIndHndl, uint16_t newAddr);

} A_InterfaceObj;

typedef A_InterfaceObj *A_InterfaceHandle;

A_InterfaceHandle A_InterfaceInit(void *pMemory, uint16_t numBytes);

int8_t A_InterfaceSetup(A_InterfaceHandle handle, void *parent_Hndl);

void AI_Service(A_InterfaceHandle handle);

// lets create some function for the application to interact with the interface
// remember the following primitives:
//
// res : response
// req : request
// con : confirm
// ind : indication
//
// they should translate into structs for data, functions and call back functions
//
// direct memory access (without interaction to the outside world), use get and set
//

//void AI_OwnAddress_set(A_InterfaceHandle handle, uint16_t ownAddress);

//void N_OwnAddress_ind(void *parentHndl, uint16_t ownAddress);

void AI_Group_Write_Req(A_InterfaceHandle handle, uint16_t grpObjIdx);

void AI_Group_Read_Req(A_InterfaceHandle handle, uint16_t grpObjIdx);

void AI_Group_Read_Res(A_InterfaceHandle handle, uint16_t grpObjIdx);




// group value read call back functions
void AI_GroupValue_Read_Lcon_cb(void *parent_Hndl, A_Data_Group_s *A_Data_Lcon);
void AI_GroupValue_Read_ind_cb(void *parent_Hndl, A_Data_Group_s *A_Data_ind);
void AI_GroupValue_Read_Rcon_cb(void *parent_Hndl, A_Data_Group_s *A_Data_Rcon);
void AI_GroupValue_Read_Acon_cb(void *parent_Hndl, A_Data_Group_s *A_Data_Acon);
	// group value write call back functions
void AI_GroupValue_Write_Lcon_cb(void *parent_Hndl, A_Data_Group_s *A_Data_Lcon);
void AI_GroupValue_Write_ind_cb(void *parent_Hndl, A_Data_Group_s *A_Data_ind);
// device descriptor call back functions
void AI_Device_Descriptor_Read_Lcon_cb(void *parent_Hndl, A_Device_Descriptor_s *A_Dev_Descr_Lcon);
void AI_Device_Descriptor_Read_ind_cb(void *parent_Hndl, A_Device_Descriptor_s *A_Dev_Descr_ind);
void AI_Device_Descriptor_Read_Acon_cb(void *parent_Hndl, A_Device_Descriptor_s *A_Dev_Descr_Acon);
// property value call back functions
void AI_Property_Value_Read_Lcon_cb(void *parent_Hndl, A_Property_Value_s *A_Property_val_Lcon);
void AI_Property_Value_Read_ind_cb(void *parent_Hndl, A_Property_Value_s *A_Property_val_ind);
void AI_Property_Value_Read_Acon_cb(void *parent_Hndl, A_Property_Value_s *A_Property_val_Acon);
void AI_Property_Value_Write_Lcon_cb(void *parent_Hndl, A_Property_Value_s *A_Property_val_Lcon);
void AI_Property_Value_Write_ind_cb(void *parent_Hndl, A_Property_Value_s *A_Property_val_ind);
// property description call back functions
void AI_Property_Description_Read_Lcon_cb(void *parent_Hndl, A_Property_Description_s *A_property_descr_Lcon);
void AI_Property_Description_Read_ind_cb(void *parent_Hndl, A_Property_Description_s *A_property_descr_ind);
void AI_Property_Description_Read_Acon_cb(void *parent_Hndl, A_Property_Description_s *A_property_descr_Acon);

// Memory read / write functions
void AI_Memory_Read_Lcon_cb(void *parent_Hndl, A_Memory_s *A_Memory_Lcon);
void AI_Memory_Read_ind_cb(void *parent_Hndl, A_Memory_s *A_Memory_ind);
void AI_Memory_Write_Lcon_cb(void *parent_Hndl, A_Memory_s *A_Memory_Lcon);
void AI_Memory_Write_Acon_cb(void *parent_Hndl, A_Memory_s *A_Memory_Acon);
void AI_Memory_Write_ind_cb(void *parent_Hndl, A_Memory_s *A_Memory_ind);

// ADC read functions
void AI_ADC_Read_Lcon_cb(void *parent_Hndl, A_ADC_read_s *A_ADC_read_Lcon);
void AI_ADC_Read_Acon_cb(void *parent_Hndl, A_ADC_read_s *A_ADC_read_Acon);
void AI_ADC_Read_ind_cb(void *parent_Hndl, A_ADC_read_s *A_ADC_read_ind);

// authorize service call back functions
void AI_Authorize_Request_ind_cb(void *parent_Hndl, A_Authorisation_s *A_Authorize_ind);
void AI_Authorize_Request_Acon_cb(void *parent_Hndl, A_Authorisation_s *A_Authorize_Acon);

// individual address write call back functions
void AI_IndividualAddress_Write_Lcon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Lcon);
void AI_IndividualAddress_Write_ind_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_ind);
void AI_IndividualAddress_Read_Lcon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Lcon);
void AI_IndividualAddress_Read_Acon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Acon);
void AI_IndividualAddress_Read_Rcon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Rcon);
void AI_IndividualAddress_Read_ind_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_ind);
	
void AI_IndividualAddressSerialNumber_Write_Lcon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Lcon);
void AI_IndividualAddressSerialNumber_Write_ind_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_ind);
void AI_IndividualAddressSerialNumber_Read_Lcon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Lcon);
void AI_IndividualAddressSerialNumber_Read_Acon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Acon);
void AI_IndividualAddressSerialNumber_Read_Rcon_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_Rcon);
void AI_IndividualAddressSerialNumber_Read_ind_cb(void *parent_Hndl, A_IndividualAddress_s *A_IndividualAddr_ind);

// Restart call back function
void AI_Restart_ind_cb(void *parent_Hndl, A_Restart_s *A_Restart_ind);

#endif /* APPLICATIONINTERFACE_H_ */
/*
 * groupAddr.h
 *
 * Created: 5-7-2016 17:49:22
 *  Author: Paul
 */ 


#ifndef GROUPOBJECTSERVER_H_
#define GROUPOBJECTSERVER_H_

#include "groupObject.h"
#include "interfaceObjectServer.h"
#include "object_resources.h"

typedef struct {
	group_obj					switchShortPress;
	group_obj					switchDoublePress;
	group_obj					switchLongPress;
	group_obj					dimSwitch;
	group_obj					dimCommand;
	group_obj					motorCommandShortPress; // open/close
	group_obj					motorCommandLongPress; // open/close
	group_obj					motorCommandOpenClose; // single press
	group_obj					motorCommandStep; // single press
	group_obj					sceneNumber;
	group_obj					sceneBlocking;
	group_obj					byteOutput;
	group_obj					twoByteOutput;
	group_obj					RedValue;
	group_obj					GreenValue;
	group_obj					BlueValue;
} sensor_group_obj;

typedef struct {
	group_obj					switchShortPress;
	group_obj					switchDoublePress;
	group_obj					switchLongPress;
	group_obj					multitouchRegister;
	group_obj					byteOutput;
	group_obj					multiTouchRoomToggle;
	group_obj					roomToggle; // input
} sensor_multi_group_obj;

typedef struct {
	group_obj					red;
	group_obj					green;
	group_obj					blue;
	group_obj					yellow;
	group_obj					magenta;
	group_obj					cyan;
} feedbackColor_group_obj;

typedef struct {
	group_obj					temperature;
	group_obj					humidity;
} room_ambient_group_obj;

typedef struct {
	group_obj					testObj1;
	group_obj					testObj2;
	group_obj					testObj3;
	group_obj					testObj4;
} test_obj;

#define TOTAL_GROUP_OBJECTS		(sizeof(sensor_group_obj) * 5\
								+ sizeof(sensor_multi_group_obj) * 2\
								+ sizeof(feedbackColor_group_obj)\
								+ sizeof(room_ambient_group_obj)\
								+ sizeof(test_obj)\
								)/sizeof(group_obj)

typedef struct {
	uint8_t						configFlags;
	grpObjSz_e					size;
} groupObjectConf;

typedef struct {
	uint16_t					addressObjIdx;
	uint16_t					groupObjIdx;
} addressGroupAssoc;

typedef struct {
	uint16_t					nextAssocTblIdx[MAX_GRP_OBJECTS];
} chainLinkAssocTbl;

typedef struct {
	uint8_t						enable;
	interfaceObjectServerHandle	IO_Hndl;
	chainLinkAssocTbl			assocLinkTbl;
	union {
		struct 	{
			sensor_group_obj			sensor[5];
			sensor_multi_group_obj		sensorMulti[2];
			feedbackColor_group_obj		colorFeedback;
			room_ambient_group_obj		ambient;
			test_obj					testObj;
		}						obj;
		group_obj				idx[TOTAL_GROUP_OBJECTS];
	};
	
	uint8_t						sz;	
	// interface object
	// group address table
	sGeneric10					grpAddrLoadCtrl;
	struct  {
		uint16_t					cnt;
		uint16_t					value[MAX_GRP_OBJECTS];
	}							grpAddrTbl;
	// group association table
	sGeneric10					addrGrpAssocLoadCtrl;
	struct {
		uint16_t					cnt;
		addressGroupAssoc			value[MAX_GRP_OBJECTS];
	}							addrGrpAssocTbl;
	// group object table
	sGeneric10					grpObjLoadCtrl;
	struct {
		uint16_t					cnt;
		groupObjectConf				value[MAX_GRP_OBJECTS];
	}							grpObjTbl;
} groupObjectServerObj;

typedef struct groupObjectServerObj *groupObjectServerHandle;

groupObjectServerHandle GrpSrv_Init(void *pMemory, uint16_t numBytes);

int8_t GrpSrv_Setup(groupObjectServerHandle handle, interfaceObjectServerHandle IO_Hndl);

int8_t grpSrv_WorkingDefaults(groupObjectServerHandle handle);

int8_t grpSrv_createLinkTbl(groupObjectServerHandle handle);

int8_t GrpSrv_setRecordAddrTbl(groupObjectServerHandle handle, uint16_t addrTblIdx, uint16_t grpAddr);

int8_t GrpSrv_setRecordAssocTbl(groupObjectServerHandle handle, uint16_t assocTblIdx, uint16_t addrTblIdx, uint16_t grpObjTblIdx);

int8_t GrpSrv_setRecordGrpObjTbl(groupObjectServerHandle handle, uint16_t grpObjTblIdx, uint8_t flags, grpObjSz_e size);

sensor_group_obj *GrpSrv_getGroupObjBySensor(groupObjectServerHandle handle, int8_t touchSensor);

int8_t GrpSrv_ChkKnownGrpAddr(groupObjectServerHandle handle, uint16_t ga);

uint16_t GrpSrv_getGrpAddrByGrpObjIdx(groupObjectServerHandle handle, uint16_t grpObjIdx);

int8_t GrpSrv_getGrpObjIdxbyGrpAddr(groupObjectServerHandle handle, uint16_t grpAddr, uint16_t *grpObjIdx);

uint16_t GrpSrv_getGroupAddrByAddrTblIdx(groupObjectServerHandle handle, uint16_t grpAddrTblIdx);

uint16_t GrpSrv_getAddrTblIdxByGroupAddress(groupObjectServerHandle handle, uint16_t grpAddr);

uint16_t GrpSrv_getGrpObjTblIdxByAddrTblIdx(groupObjectServerHandle handle, uint16_t AddrTblIdx);

uint16_t GrpSrv_getAddrTblIdxByGrpObjTblIdx(groupObjectServerHandle handle, uint16_t grpObjIdx);

uint16_t GrpSrv_getGrpObjIdxByAssocTblIdx(groupObjectServerHandle handle, uint16_t assocTblIdx);

int8_t GrpSrv_setGrpObjConfigFlags(groupObjectServerHandle handle, uint16_t grpObjTblIdx, uint8_t flags);

int8_t GrpSrv_clrGrpObjConfigFlags(groupObjectServerHandle handle, uint16_t grpObjTblIdx, uint8_t flags);

int8_t GrpSrv_getGrpObjConfigFlags(groupObjectServerHandle handle, uint16_t grpObjIdx);

group_obj *grpSrv_getGroupObject(groupObjectServerHandle handle, uint16_t grpObjIdx);

uint8_t GrpSrv_getGrpObjPriority(groupObjectServerHandle handle, uint16_t grpObjIdx);

uint16_t GrpSrv_getFirstAssocTblIdxByAddrTblIdx(groupObjectServerHandle handle, uint16_t AddrTblIdx);

uint16_t GrpSrv_getNextAssocTblIdxByAddrTblIdx(groupObjectServerHandle handle, uint16_t assocTblIdx);

uint16_t GrpSrv_getFirstReadEnabledAssocTblIdxByGrpObjIdx(groupObjectServerHandle handle, uint16_t grpObjIdx);

grpObjSz_e GrpSrv_getGrpObjSize(groupObjectServerHandle handle, uint16_t grpObjIdx);

int8_t GrpSrv_GrpObjTblPropInd(void *parentHandle, interfaceProperty *prop);

int8_t GrpSrv_GrpAddrTblPropInd(void *parentHandle, interfaceProperty *prop);

int8_t GrpSrv_AssocTblPropInd(void *parentHandle, interfaceProperty *prop);

#endif /* GROUPOBJECTSERVER_H_ */
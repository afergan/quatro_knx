/*
 * groupAddr.c
 *
 * Created: 5-7-2016 17:49:36
 *  Author: Paul
 */ 

#include "clocks.h"
#include "interrupt.h"
#include "touchPad.h"
#include "telegram.h"
#include "groupObjectServer.h"
#include "applicationInterface.h"

#define GRP_1_0_1	1
#define GRP_1_1_0	2
#define GRP_1_1_1	3
#define GRP_2_1_5	4
#define GRP_2_1_6	5
#define GRP_1_2_0	6
#define GRP_1_2_1	7
#define GRP_1_2_2	8
#define GRP_1_2_3	9
#define GRP_1_2_4	10
#define GRP_1_2_5	11
#define GRP_1_2_6	12
#define GRP_1_2_7	13
#define GRP_1_2_8	14
#define GRP_2_1_4	15
#define GRP_2_1_7	16
#define GRP_2_1_8	17

groupObjectServerHandle GrpSrv_Init(void *pMemory, uint16_t numBytes) {
	groupObjectServerHandle grpObjServ_handle;
	if (numBytes < sizeof(groupObjectServerObj))
		return ((groupObjectServerHandle)NULL);
	grpObjServ_handle = (groupObjectServerHandle)pMemory;
	return(grpObjServ_handle);
}

int8_t GrpSrv_Setup(groupObjectServerHandle handle, interfaceObjectServerHandle IO_Hndl) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;
	
	grpSrv->IO_Hndl = IO_Hndl;
	
	grpSrv->sz = (TOTAL_GROUP_OBJECTS);
		
	// add index to dynamic group object table
	
	for (uint16_t i = 0; i < TOTAL_GROUP_OBJECTS; i ++) {
		grpSrv->idx[i].grpObjIdx = i + 1; // group object table start with index 1 
	}

	// setup static values for group object table
	
	for (uint8_t i = 0; i < 4; i++) {
		sensor_group_obj *sensor = grpSrv->obj.sensor + i;
		GrpSrv_setRecordGrpObjTbl(handle, sensor->switchShortPress.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->switchDoublePress.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->switchLongPress.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->dimSwitch.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->dimCommand.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_4_BITS);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->motorCommandShortPress.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->motorCommandLongPress.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->motorCommandOpenClose.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->motorCommandStep.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->sceneNumber.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BYTE);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->sceneBlocking.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->byteOutput.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BYTE);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->twoByteOutput.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_2_BYTES);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->RedValue.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BYTE);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->GreenValue.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BYTE);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->BlueValue.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BYTE);
	}

	for (uint8_t i = 0; i < 2; i++) {
		sensor_multi_group_obj *sensor = grpSrv->obj.sensorMulti + i;
		GrpSrv_setRecordGrpObjTbl(handle, sensor->switchShortPress.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->switchDoublePress.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->switchLongPress.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->multitouchRegister.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BYTE);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->byteOutput.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BYTE);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->multiTouchRoomToggle.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BYTE);
		GrpSrv_setRecordGrpObjTbl(handle, sensor->roomToggle.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BYTE);
	}

	feedbackColor_group_obj *feedbackColor = &(grpSrv->obj.colorFeedback);
	GrpSrv_setRecordGrpObjTbl(handle, feedbackColor->red.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
	GrpSrv_setRecordGrpObjTbl(handle, feedbackColor->green.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
	GrpSrv_setRecordGrpObjTbl(handle, feedbackColor->blue.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
	GrpSrv_setRecordGrpObjTbl(handle, feedbackColor->yellow.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
	GrpSrv_setRecordGrpObjTbl(handle, feedbackColor->magenta.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);
	GrpSrv_setRecordGrpObjTbl(handle, feedbackColor->cyan.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_1_BIT);	
	
	room_ambient_group_obj *temp = &(grpSrv->obj.ambient);
	GrpSrv_setRecordGrpObjTbl(handle, temp->temperature.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_2_BYTES);
	GrpSrv_setRecordGrpObjTbl(handle, temp->humidity.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_2_BYTES);
	
	test_obj *test = &(grpSrv->obj.testObj);
	GrpSrv_setRecordGrpObjTbl(handle, test->testObj1.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_2_BYTES);
	GrpSrv_setRecordGrpObjTbl(handle, test->testObj2.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_2_BYTES);
	GrpSrv_setRecordGrpObjTbl(handle, test->testObj3.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_2_BYTES);
	GrpSrv_setRecordGrpObjTbl(handle, test->testObj4.grpObjIdx, KNX_LOW, GRP_OBJ_SZ_2_BYTES);

	grpSrv->grpObjLoadCtrl.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(IO_Hndl, GROUP_OBJECT_TABLE_OBJECT_IDX, PID_LOAD_STATE_CONTROL, &grpSrv->grpObjLoadCtrl);
	IntfObj_Srv_registerTableObject(IO_Hndl, GROUP_OBJECT_TABLE_OBJECT_IDX, PID_TABLE, &grpSrv->grpObjTbl);
	IntfObj_setElementsInUse(IntfObj_Srv_getObjByPropId(IO_Hndl, GROUP_OBJECT_TABLE_OBJECT_IDX, PID_TABLE), TOTAL_GROUP_OBJECTS);
	IntfObj_Srv_setMCTObject(IO_Hndl, GROUP_OBJECT_TABLE_OBJECT_IDX);
	
	grpSrv->grpAddrLoadCtrl.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(IO_Hndl, ADDRESS_TABLE_OBJECT_IDX, PID_LOAD_STATE_CONTROL, &grpSrv->grpAddrLoadCtrl);
	IntfObj_Srv_registerTableObject(IO_Hndl, ADDRESS_TABLE_OBJECT_IDX, PID_TABLE, &grpSrv->grpAddrTbl);

	grpSrv->addrGrpAssocLoadCtrl.cnt = SWAP_UINT16(1);
	IntfObj_Srv_registerObject(IO_Hndl, ASSOCIATION_TABLE_OBJECT_IDX, PID_LOAD_STATE_CONTROL, &grpSrv->addrGrpAssocLoadCtrl);	
	IntfObj_Srv_registerTableObject(IO_Hndl, ASSOCIATION_TABLE_OBJECT_IDX, PID_TABLE, &grpSrv->addrGrpAssocTbl);

	uint8_t loaded = OBJECT_LOAD_STATE_LOADED;
	
	IntfObj_setPropValue(IntfObj_Srv_getObjByPropId(IO_Hndl, GROUP_OBJECT_TABLE_OBJECT_IDX, PID_LOAD_STATE_CONTROL), 1, 1, &loaded);
	
	grpSrv_WorkingDefaults(handle);

	return 0;	
}

int8_t grpSrv_WorkingDefaults(groupObjectServerHandle handle) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;
	
	// initialize address table with some working values
	
	GrpSrv_setRecordAddrTbl(handle, GRP_1_0_1, 0x0801);
	GrpSrv_setRecordAddrTbl(handle, GRP_1_1_0, 0x0900);
	GrpSrv_setRecordAddrTbl(handle, GRP_1_1_1, 0x0901);
	GrpSrv_setRecordAddrTbl(handle, GRP_2_1_5, 0x1105);
	GrpSrv_setRecordAddrTbl(handle, GRP_2_1_6, 0x1106);
	GrpSrv_setRecordAddrTbl(handle, GRP_1_2_0, 0x0A00);
	GrpSrv_setRecordAddrTbl(handle, GRP_1_2_1, 0x0A01);
	GrpSrv_setRecordAddrTbl(handle, GRP_1_2_2, 0x0A02);
	GrpSrv_setRecordAddrTbl(handle, GRP_1_2_3, 0x0A03);
	GrpSrv_setRecordAddrTbl(handle, GRP_1_2_4, 0x0A04);
	GrpSrv_setRecordAddrTbl(handle, GRP_1_2_5, 0x0A05);
	GrpSrv_setRecordAddrTbl(handle, GRP_2_1_4, 0x1104);
	GrpSrv_setRecordAddrTbl(handle, GRP_2_1_7, 0x1107);
	GrpSrv_setRecordAddrTbl(handle, GRP_2_1_8, 0x1108);
	
	IntfObj_setElementsInUse(IntfObj_Srv_getObjByPropId(grpSrv->IO_Hndl, ADDRESS_TABLE_OBJECT_IDX, PID_TABLE), GRP_2_1_8);
	IntfObj_Srv_setMCTObject(grpSrv->IO_Hndl, ADDRESS_TABLE_OBJECT_IDX);
	
	uint8_t loaded = OBJECT_LOAD_STATE_LOADED;
	
	IntfObj_setPropValue(IntfObj_Srv_getObjByPropId(grpSrv->IO_Hndl, ADDRESS_TABLE_OBJECT_IDX, PID_LOAD_STATE_CONTROL), 1, 1, &loaded);
	
	// associations table
			
	uint16_t cnt = 1;
			
	sensor_group_obj *sensorGrpObjs = GrpSrv_getGroupObjBySensor(handle, TOUCH_TOP);
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_0_1, sensorGrpObjs->switchShortPress.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, sensorGrpObjs->switchShortPress.grpObjIdx, (FLAG_COMMUNICATE | FLAG_TRANSMIT | FLAG_WRITE | FLAG_UPDATE));
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_0_1,sensorGrpObjs->switchDoublePress.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, sensorGrpObjs->switchDoublePress.grpObjIdx, (FLAG_COMMUNICATE | FLAG_TRANSMIT | FLAG_WRITE | FLAG_UPDATE));
	
	sensorGrpObjs = GrpSrv_getGroupObjBySensor(handle, TOUCH_RIGHT);
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_1_0, sensorGrpObjs->dimSwitch.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, sensorGrpObjs->dimSwitch.grpObjIdx, (FLAG_COMMUNICATE | FLAG_TRANSMIT | FLAG_WRITE | FLAG_UPDATE));
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_1_1, sensorGrpObjs->dimCommand.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, sensorGrpObjs->dimCommand.grpObjIdx, (FLAG_COMMUNICATE | FLAG_TRANSMIT | FLAG_WRITE | FLAG_UPDATE));
	
	
	sensorGrpObjs = GrpSrv_getGroupObjBySensor(handle, TOUCH_BOTTOM);
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_0_1, sensorGrpObjs->switchShortPress.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, sensorGrpObjs->switchShortPress.grpObjIdx, (FLAG_COMMUNICATE | FLAG_TRANSMIT | FLAG_WRITE | FLAG_UPDATE));
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_0_1, sensorGrpObjs->switchDoublePress.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, sensorGrpObjs->switchDoublePress.grpObjIdx, (FLAG_COMMUNICATE | FLAG_TRANSMIT | FLAG_WRITE | FLAG_UPDATE));
	

	sensorGrpObjs = GrpSrv_getGroupObjBySensor(handle, TOUCH_LEFT);
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_1_0, sensorGrpObjs->dimSwitch.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, sensorGrpObjs->dimSwitch.grpObjIdx, (FLAG_COMMUNICATE | FLAG_TRANSMIT | FLAG_WRITE | FLAG_UPDATE));
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_1_1, sensorGrpObjs->dimCommand.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, sensorGrpObjs->dimCommand.grpObjIdx, (FLAG_COMMUNICATE | FLAG_TRANSMIT | FLAG_WRITE | FLAG_UPDATE));

	sensorGrpObjs = GrpSrv_getGroupObjBySensor(handle, TOUCH_TOP_BOTTOM);
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_0_1, sensorGrpObjs->switchShortPress.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, sensorGrpObjs->switchShortPress.grpObjIdx, (FLAG_COMMUNICATE | FLAG_TRANSMIT | FLAG_WRITE | FLAG_UPDATE));
	
	// temperature and humidity sensors
	
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_2_1_5, grpSrv->obj.ambient.temperature.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, grpSrv->obj.ambient.temperature.grpObjIdx, (FLAG_COMMUNICATE | FLAG_TRANSMIT | FLAG_READ));
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_2_1_6, grpSrv->obj.ambient.humidity.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, grpSrv->obj.ambient.humidity.grpObjIdx, (FLAG_COMMUNICATE | FLAG_TRANSMIT | FLAG_READ));

	// set RGB led group addresses.
	
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_2_0, grpSrv->obj.colorFeedback.red.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, grpSrv->obj.colorFeedback.red.grpObjIdx, (FLAG_COMMUNICATE | FLAG_WRITE | FLAG_UPDATE));
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_2_1, grpSrv->obj.colorFeedback.green.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, grpSrv->obj.colorFeedback.green.grpObjIdx, (FLAG_COMMUNICATE | FLAG_WRITE | FLAG_UPDATE));
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_2_2, grpSrv->obj.colorFeedback.blue.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, grpSrv->obj.colorFeedback.blue.grpObjIdx, (FLAG_COMMUNICATE | FLAG_WRITE | FLAG_UPDATE));
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_2_3, grpSrv->obj.colorFeedback.yellow.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, grpSrv->obj.colorFeedback.yellow.grpObjIdx, (FLAG_COMMUNICATE | FLAG_WRITE | FLAG_UPDATE));
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_2_4, grpSrv->obj.colorFeedback.magenta.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, grpSrv->obj.colorFeedback.magenta.grpObjIdx, (FLAG_COMMUNICATE | FLAG_WRITE | FLAG_UPDATE));
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_1_2_5, grpSrv->obj.colorFeedback.cyan.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, grpSrv->obj.colorFeedback.cyan.grpObjIdx, (FLAG_COMMUNICATE | FLAG_WRITE | FLAG_UPDATE));
	
	// Test objecten
	
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_2_1_4, grpSrv->obj.testObj.testObj1.grpObjIdx);
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_2_1_8, grpSrv->obj.testObj.testObj1.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, grpSrv->obj.testObj.testObj1.grpObjIdx, (FLAG_COMMUNICATE | FLAG_TRANSMIT | FLAG_READ | FLAG_WRITE));
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_2_1_4, grpSrv->obj.testObj.testObj2.grpObjIdx);
	GrpSrv_setRecordAssocTbl(handle, cnt++, GRP_2_1_7, grpSrv->obj.testObj.testObj2.grpObjIdx);
	GrpSrv_setGrpObjConfigFlags(handle, grpSrv->obj.testObj.testObj2.grpObjIdx, (FLAG_COMMUNICATE | FLAG_TRANSMIT | FLAG_READ | FLAG_UPDATE));
	
	IntfObj_setElementsInUse(IntfObj_Srv_getObjByPropId(grpSrv->IO_Hndl, ASSOCIATION_TABLE_OBJECT_IDX, PID_TABLE), cnt - 1);
	IntfObj_Srv_setMCTObject(grpSrv->IO_Hndl, ASSOCIATION_TABLE_OBJECT_IDX);
	
	grpSrv_createLinkTbl(handle);
	
	IntfObj_setPropValue(IntfObj_Srv_getObjByPropId(grpSrv->IO_Hndl, ASSOCIATION_TABLE_OBJECT_IDX, PID_LOAD_STATE_CONTROL), 1, 1, &loaded);
	
	return 0;
}

int8_t grpSrv_createLinkTbl(groupObjectServerHandle handle) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;
	
	uint16_t assocTblLength = SWAP_UINT16(grpSrv->addrGrpAssocTbl.cnt);
	
	if (assocTblLength > 0)
	{
		uint16_t tblIdx = 0;
		while (tblIdx < assocTblLength)
		{
			uint16_t innerIdx = tblIdx;
			uint16_t addrTblIdx = grpSrv->addrGrpAssocTbl.value[innerIdx].addressObjIdx;
			innerIdx++;
			while (innerIdx < assocTblLength)
			{
				if (grpSrv->addrGrpAssocTbl.value[innerIdx].addressObjIdx == addrTblIdx)
				{
					grpSrv->assocLinkTbl.nextAssocTblIdx[tblIdx] = innerIdx + 1;
					break;
				}
				innerIdx++;
			}
			tblIdx++;
		}
	}
	//grpSrv->assocLinkTbl.nextAssocTblIdx[assocTblLength] = 0xbeef;
	return 0;
}



int8_t GrpSrv_setRecordAddrTbl(groupObjectServerHandle handle, uint16_t addrTblIdx, uint16_t grpAddr) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;

	if (addrTblIdx <= MAX_GRP_OBJECTS && addrTblIdx && grpAddr)
	{
		grpSrv->grpAddrTbl.value[addrTblIdx - 1] = SWAP_UINT16(grpAddr);
		return 0;
	}
	
	return -1;
}

int8_t GrpSrv_setRecordAssocTbl(groupObjectServerHandle handle, uint16_t assocTblIdx, uint16_t addrTblIdx, uint16_t grpObjTblIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;

	if (assocTblIdx <= MAX_GRP_OBJECTS && assocTblIdx && assocTblIdx && grpObjTblIdx)
	{
		grpSrv->addrGrpAssocTbl.value[assocTblIdx - 1].addressObjIdx = SWAP_UINT16(addrTblIdx);
		grpSrv->addrGrpAssocTbl.value[assocTblIdx - 1].groupObjIdx = SWAP_UINT16(grpObjTblIdx);
		return 0;
	}
	
	return -1;
}

int8_t GrpSrv_setRecordGrpObjTbl(groupObjectServerHandle handle, uint16_t grpObjTblIdx, uint8_t flags, grpObjSz_e size) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;
	
	if (grpObjTblIdx <= MAX_GRP_OBJECTS && grpObjTblIdx)
	{
		grpSrv->grpObjTbl.value[grpObjTblIdx - 1].configFlags = flags;
		grpSrv->grpObjTbl.value[grpObjTblIdx - 1].size = size;
		return 0;
	}
	return -1;
}

int8_t GrpSrv_ChkKnownGrpAddr(groupObjectServerHandle handle, uint16_t ga) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;
	
	uint16_t tblEntries = 0;
	tblEntries = SWAP_UINT16( grpSrv->grpAddrTbl.cnt);

	for (uint16_t i = 0; i < tblEntries; i++) {
		if (ga == GrpSrv_getGroupAddrByAddrTblIdx(handle, i + 1))
			return 1;
	}
	return 0;
}

sensor_group_obj *GrpSrv_getGroupObjBySensor(groupObjectServerHandle handle, int8_t touchSensor) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;
	
	if (touchSensor < TOUCH_RESET)
		return (sensor_group_obj*)(&grpSrv->obj.sensor[touchSensor]);
	return (sensor_group_obj*)NULL;
	
}

uint16_t GrpSrv_getGrpAddrByGrpObjIdx(groupObjectServerHandle handle, uint16_t grpObjIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;
		
	uint16_t grpAddrTblIdx = 0;
	for (uint16_t i = 0; i < SWAP_UINT16(grpSrv->addrGrpAssocTbl.cnt); i++) {
		if (SWAP_UINT16(grpSrv->addrGrpAssocTbl.value[i].groupObjIdx) == grpObjIdx) {
			grpAddrTblIdx = SWAP_UINT16(grpSrv->addrGrpAssocTbl.value[i].addressObjIdx);
			break;
		}
	}
	if (grpAddrTblIdx && grpAddrTblIdx <= SWAP_UINT16(grpSrv->grpAddrTbl.cnt)) {
		return SWAP_UINT16(grpSrv->grpAddrTbl.value[grpAddrTblIdx - 1]);
	}
	return 0;
}

int8_t GrpSrv_getGrpObjIdxbyGrpAddr(groupObjectServerHandle handle, uint16_t grpAddr, uint16_t *grpObjIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;
	
	uint16_t assocTblIdx = 0;
	uint16_t addrTblIdx = 0;
	uint16_t i = 0;
	// Look up group address in group address table
	for (; i < SWAP_UINT16(grpSrv->grpAddrTbl.cnt); i++) {
		if (SWAP_UINT16(grpSrv->grpAddrTbl.value[i]) == grpAddr) {
			addrTblIdx = i + 1;
			break;
		}
	}
	if (!addrTblIdx)
		return 0;
	// look up address table index in the association table
	for (i = 0; i < SWAP_UINT16(grpSrv->addrGrpAssocTbl.cnt); i++) {
		if (SWAP_UINT16(grpSrv->addrGrpAssocTbl.value[i].addressObjIdx) == addrTblIdx) {
			assocTblIdx = i + 1;
			break;
		}
	}
	// if entry found, pass the associated group object index
	if (assocTblIdx) {
		*grpObjIdx = SWAP_UINT16(grpSrv->addrGrpAssocTbl.value[assocTblIdx - 1].groupObjIdx);
		return 1;
	}
	return 0;
}

uint16_t GrpSrv_getGroupAddrByAddrTblIdx(groupObjectServerHandle handle, uint16_t grpAddrTblIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;
	
	if (grpAddrTblIdx > SWAP_UINT16(grpSrv->grpAddrTbl.cnt) || !grpAddrTblIdx)
		return 0;
	return SWAP_UINT16(grpSrv->grpAddrTbl.value[grpAddrTblIdx - 1]);
}

uint16_t GrpSrv_getAddrTblIdxByGroupAddress(groupObjectServerHandle handle, uint16_t grpAddr) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;

	uint16_t i = 0;
	// Look up group address in group address table
	for (; i < SWAP_UINT16(grpSrv->grpAddrTbl.cnt); i++) {
		if (SWAP_UINT16(grpSrv->grpAddrTbl.value[i]) == grpAddr) 
			return (i + 1);
	}
	return 0;
}

// look up group object index by address table index in association table

uint16_t GrpSrv_getGrpObjTblIdxByAddrTblIdx(groupObjectServerHandle handle, uint16_t AddrTblIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;

	uint16_t i = 0;
	uint16_t assocTblIdx = 0;
	// look up address table index in the association table
	for (i = 0; i < SWAP_UINT16(grpSrv->addrGrpAssocTbl.cnt); i++) {
		volatile uint16_t addrIdx = SWAP_UINT16(grpSrv->addrGrpAssocTbl.value[i].addressObjIdx);
		if (addrIdx == AddrTblIdx) {
			assocTblIdx = i + 1;
			break;
		}
	}
	// if entry found, pass the associated group object index
	if (assocTblIdx) {
		return SWAP_UINT16(grpSrv->addrGrpAssocTbl.value[assocTblIdx - 1].groupObjIdx);
	}
	return 0;
}


uint16_t GrpSrv_getFirstAssocTblIdxByAddrTblIdx(groupObjectServerHandle handle, uint16_t AddrTblIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;

	uint16_t i = 0;
	// look up address table index in the association table
	for (i = 0; i < SWAP_UINT16(grpSrv->addrGrpAssocTbl.cnt); i++) {
		volatile uint16_t addrIdx = SWAP_UINT16(grpSrv->addrGrpAssocTbl.value[i].addressObjIdx);
		if (addrIdx == AddrTblIdx) {
			return (i + 1);
		}
	}
	return 0;
}

uint16_t GrpSrv_getNextAssocTblIdxByAddrTblIdx(groupObjectServerHandle handle, uint16_t assocTblIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;

	if (assocTblIdx > SWAP_UINT16(grpSrv->addrGrpAssocTbl.cnt) || !assocTblIdx)
		return 0;
	else
		return (grpSrv->assocLinkTbl.nextAssocTblIdx[assocTblIdx - 1]);
}

uint16_t GrpSrv_getAddrTblIdxByGrpObjTblIdx(groupObjectServerHandle handle, uint16_t grpObjIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;

	uint16_t i = 0;
	uint16_t assocTblIdx = 0;
	
	// look up address table index in the association table
	for (i = 0; i < SWAP_UINT16(grpSrv->addrGrpAssocTbl.cnt); i++) {
		volatile uint16_t addrIdx = SWAP_UINT16(grpSrv->addrGrpAssocTbl.value[i].groupObjIdx);
		if (addrIdx == grpObjIdx) {
			assocTblIdx = i + 1;
			break;
		}
	}
	if (assocTblIdx) {
		return SWAP_UINT16(grpSrv->addrGrpAssocTbl.value[assocTblIdx - 1].addressObjIdx);
	}
	return 0;
}

uint16_t GrpSrv_getGrpObjIdxByAssocTblIdx(groupObjectServerHandle handle, uint16_t assocTblIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;
	
	if (assocTblIdx > SWAP_UINT16(grpSrv->addrGrpAssocTbl.cnt) || !assocTblIdx)
		return 0;
	return SWAP_UINT16(grpSrv->addrGrpAssocTbl.value[assocTblIdx - 1].groupObjIdx);
}

group_obj *grpSrv_getGroupObject(groupObjectServerHandle handle, uint16_t grpObjIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;
	
	if (grpObjIdx <= grpSrv->sz)
		return grpSrv->idx + (grpObjIdx - 1);
	return NULL;	
}

int8_t GrpSrv_setGrpObjConfigFlags(groupObjectServerHandle handle, uint16_t grpObjTblIdx, uint8_t flags) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;
	
	if (grpObjTblIdx <= MAX_GRP_OBJECTS && grpObjTblIdx)
	{
		grpSrv->grpObjTbl.value[grpObjTblIdx - 1].configFlags |= flags;
		return 0;
	}
	
	return -1;
}

int8_t GrpSrv_clrGrpObjConfigFlags(groupObjectServerHandle handle, uint16_t grpObjTblIdx, uint8_t flags) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;
	
	if (grpObjTblIdx <= MAX_GRP_OBJECTS && grpObjTblIdx)
	{
		grpSrv->grpObjTbl.value[grpObjTblIdx - 1].configFlags &= ~(flags);
		return 0;
	}
	
	return -1;
}

int8_t GrpSrv_getGrpObjConfigFlags(groupObjectServerHandle handle, uint16_t grpObjIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;

	uint16_t tblEntries = 0;
	tblEntries = SWAP_UINT16( grpSrv->grpObjTbl.cnt);
	
	if (grpObjIdx <= tblEntries)
		return grpSrv->grpObjTbl.value[grpObjIdx - 1].configFlags;	
	return -1;	
}

uint8_t GrpSrv_getGrpObjPriority(groupObjectServerHandle handle, uint16_t grpObjIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;

	uint16_t tblEntries = 0;
	tblEntries = SWAP_UINT16( grpSrv->grpObjTbl.cnt);
	
	if (grpObjIdx <= tblEntries)
		return (grpSrv->grpObjTbl.value[grpObjIdx - 1].configFlags & 0x3);
	return -1;	
		
}

grpObjSz_e GrpSrv_getGrpObjSize(groupObjectServerHandle handle, uint16_t grpObjIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;

	uint16_t tblEntries = 0;
	tblEntries = SWAP_UINT16( grpSrv->grpObjTbl.cnt);
	
	if (grpObjIdx <= tblEntries)
		return grpSrv->grpObjTbl.value[grpObjIdx - 1].size;
	return -1;
}

uint16_t GrpSrv_getFirstReadEnabledAssocTblIdxByGrpObjIdx(groupObjectServerHandle handle, uint16_t grpObjIdx) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) handle;

	uint16_t i = 0;
	// look up address table index in the association table
	for (i = 0; i < SWAP_UINT16(grpSrv->addrGrpAssocTbl.cnt); i++) {
		uint16_t grpObjIdx_read_enabled = SWAP_UINT16(grpSrv->addrGrpAssocTbl.value[i].groupObjIdx);
		if ((grpObjIdx == grpObjIdx_read_enabled) && (GrpSrv_getGrpObjConfigFlags(handle, grpObjIdx) & FLAG_READ)){
			return (i + 1);
		}
	}
	return 0;	
}

int8_t GrpSrv_GrpObjTblPropInd(void *parentHandle, interfaceProperty *prop) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) parentHandle;
	
	switch((PropertyId_e)prop->propertyId) {
		case PID_LOAD_STATE_CONTROL:
		
		break;
		case PID_TABLE:

		break;
		default:
		return -1;
	}
	return 1;
}

int8_t GrpSrv_GrpAddrTblPropInd(void *parentHandle, interfaceProperty *prop) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) parentHandle;
	
	switch((PropertyId_e)prop->propertyId) {
		case PID_LOAD_STATE_CONTROL:
		
		break;
		case PID_TABLE:

		break;
		default:
		return -1;
	}	
	return 1;	
}

int8_t GrpSrv_AssocTblPropInd(void *parentHandle, interfaceProperty *prop) {
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *) parentHandle;
	
	switch((PropertyId_e)prop->propertyId) {
		case PID_LOAD_STATE_CONTROL:
		
		break;
		case PID_TABLE:

		break;
		default:
		return -1;
	}	
	return 1;	
}
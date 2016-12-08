/*
 * groupObject.h
 *
 * Created: 20-11-2016 7:26:14
 *  Author: Paul
 */ 


#ifndef GROUPOBJECT_H_
#define GROUPOBJECT_H_

#include "clocks.h"
#include "object_resources.h"

typedef struct {
	uint16_t					grpObjIdx;
	uint8_t						communicationFlags;
	void						*parentObj_Read_ind;
	void						(*grpObj_Read_ind_cb)(void *parent, uint16_t grpObjIdx);
	void						*parentObj_Write_ind;
	void						(*grpObj_Write_ind_cb)(void *parent, uint16_t grpObjIdx);
	uint16_t					value;
} group_obj;

int8_t GrpObj_UpdateValue(group_obj *grpObj, uint16_t val, grpObjSz_e size);

int8_t GrpObj_GetValue(group_obj *grpObj, uint16_t *val);

int8_t GrpObj_getCommFlags(group_obj *grpObj);

int8_t GrpObj_setCommFlags(group_obj *grpObj, uint8_t flags);

int8_t GrpObj_clrCommFlags(group_obj *grpObj, uint8_t flags);

int8_t GrpObj_register_Read_ind_cb(group_obj *grpObj, void *grpVal_Read_cb, void *parentHndl);

int8_t GrpObj_register_Write_ind_cb(group_obj *grpObj, void *grpVal_write_cb, void *parentHndl);

int8_t GrpObj_is_Read_ind_cb_registered(group_obj *grpObj);

int8_t GrpObj_is_Write_ind_cb_registered(group_obj *grpObj);

void GrpObj_Call_Read_ind_cb(group_obj *grpObj);

void GrpObj_Call_Write_ind_cb(group_obj *grpObj);

#endif /* GROUPOBJECT_H_ */
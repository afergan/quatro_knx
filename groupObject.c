/*
 * groupObject.c
 *
 * Created: 20-11-2016 7:26:33
 *  Author: Paul
 */ 

#include "groupObject.h"

int8_t GrpObj_UpdateValue(group_obj *grpObj, uint16_t val, grpObjSz_e size) {
	uint16_t trimmedValue = val;

	if (grpObj != NULL)
	{
		switch (size) {
		case GRP_OBJ_SZ_1_BIT:
			trimmedValue = (val & 0x1);
			break;
		case GRP_OBJ_SZ_2_BITS:
			trimmedValue = (val & 0x3);
			break;
		case GRP_OBJ_SZ_3_BITS:
			trimmedValue = (val & 0x7);
			break;
		case GRP_OBJ_SZ_4_BITS:
			trimmedValue = (val & 0xf);
			break;
		case GRP_OBJ_SZ_5_BITS:
			trimmedValue = (val & 0x1f);
			break;
		case GRP_OBJ_SZ_6_BITS:
			trimmedValue = (val & 0x3f);
			break;
		case GRP_OBJ_SZ_7_BITS:
			trimmedValue = (val & 0x7f);
			break;
		case GRP_OBJ_SZ_1_BYTE:
			trimmedValue = (val & 0xff);
			break;
		case GRP_OBJ_SZ_2_BYTES:
			trimmedValue = val;
			break;
		default:
			break;
		}
		if (trimmedValue != val)
		{
			grpObj->value = 0;
			return -1;
		}
		grpObj->value = val;
		return 0;		
	}
	return -1;	
}

int8_t GrpObj_GetValue(group_obj *grpObj, uint16_t *val) {
	if (grpObj != NULL) {
		*val = grpObj->value;
		return 0;
	}	
	return -1;	
}


int8_t GrpObj_getCommFlags(group_obj *grpObj) {
	return (grpObj != NULL) ? grpObj->communicationFlags : -1;
}

int8_t GrpObj_setCommFlags(group_obj *grpObj, uint8_t flags) {
	if (grpObj != NULL)
	{
		grpObj->communicationFlags |= flags;
		return 0;
	}
	return -1;
}

int8_t GrpObj_clrCommFlags(group_obj *grpObj, uint8_t flags) {
	if (grpObj != NULL)
	{
		grpObj->communicationFlags &= ~(flags);
		return 0;
	}
	return -1;	
}

int8_t GrpObj_register_Read_ind_cb(group_obj *grpObj, void *grpVal_Read_cb, void *parentHndl) {

	grpObj->grpObj_Read_ind_cb = grpVal_Read_cb;
	grpObj->parentObj_Read_ind = parentHndl;

	return 0;
}

int8_t GrpObj_register_Write_ind_cb(group_obj *grpObj, void *grpVal_write_cb, void *parentHndl) {

	grpObj->grpObj_Write_ind_cb = grpVal_write_cb;
	grpObj->parentObj_Write_ind = parentHndl;

	return 0;
}

int8_t GrpObj_is_Read_ind_cb_registered(group_obj *grpObj) {
	return ((grpObj != NULL) && (grpObj->grpObj_Read_ind_cb != NULL)) ? 1 : 0;
}

int8_t GrpObj_is_Write_ind_cb_registered(group_obj *grpObj) {
	return ((grpObj != NULL) && (grpObj->grpObj_Write_ind_cb != NULL)) ? 1 : 0;
}

void GrpObj_Call_Read_ind_cb(group_obj *grpObj) {
	if ((grpObj != NULL) && (grpObj->grpObj_Read_ind_cb != NULL))
	{
		grpObj->grpObj_Read_ind_cb(grpObj->parentObj_Read_ind, grpObj->grpObjIdx);
	}
}

void GrpObj_Call_Write_ind_cb(group_obj *grpObj) {
	if ((grpObj != NULL) && (grpObj->grpObj_Write_ind_cb != NULL))
	{
		grpObj->grpObj_Write_ind_cb(grpObj->parentObj_Write_ind, grpObj->grpObjIdx);
	}	
}


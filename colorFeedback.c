/*
 * colorFeedback.c
 *
 * Created: 20/09/16 14:11:43
 *  Author: PvR
 */ 

#include "colorFeedback.h"

colorFeedback_Handle colorFeedback_Init(void *pMemory, uint16_t numBytes) {
	colorFeedback_Handle colorFbk_handle;
	if (numBytes < sizeof(colorFeedback_Obj))
	return ((colorFeedback_Handle)NULL);
	colorFbk_handle = (colorFeedback_Handle)pMemory;
	return colorFbk_handle;	
}

int8_t colorFeedback_Setup(colorFeedback_Handle handle, A_InterfaceHandle AI_Hndl, rgbLedHandle	rgbLed_Hndl) {
	colorFeedback_Obj *colorFbk_hndl;
	colorFbk_hndl = (colorFeedback_Obj*)handle;
	
	colorFbk_hndl->AI_Hndl = AI_Hndl;
	colorFbk_hndl->rgbLed_Hndl = rgbLed_Hndl;
	
	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *)(AI_Hndl->GO_Hndl);
		
	GrpObj_register_Write_ind_cb(&(grpSrv->obj.colorFeedback.red), &rgbFeedback_Group_Write_cb, handle);
	GrpObj_register_Write_ind_cb(&(grpSrv->obj.colorFeedback.green), &rgbFeedback_Group_Write_cb, handle);
	GrpObj_register_Write_ind_cb(&(grpSrv->obj.colorFeedback.blue), &rgbFeedback_Group_Write_cb, handle);
	GrpObj_register_Write_ind_cb(&(grpSrv->obj.colorFeedback.yellow), &rgbFeedback_Group_Write_cb, handle);
	GrpObj_register_Write_ind_cb(&(grpSrv->obj.colorFeedback.magenta), &rgbFeedback_Group_Write_cb, handle);
	GrpObj_register_Write_ind_cb(&(grpSrv->obj.colorFeedback.cyan), &rgbFeedback_Group_Write_cb, handle);

	return 0;
}


void rgbFeedback_Group_Write_cb(void *parentHndl, uint16_t grpObjIdx) {
	colorFeedback_Obj *colorFbk_Hndl;
	colorFbk_Hndl = (colorFeedback_Obj*)parentHndl;

	groupObjectServerObj *grpSrv;
	grpSrv = (groupObjectServerObj *)((colorFeedback_Obj *)parentHndl)->AI_Hndl->GO_Hndl;	
	
	feedbackColor_group_obj *colorFbk_GrpObj = &(grpSrv->obj.colorFeedback);
	
	group_obj *grpObj = grpSrv_getGroupObject((groupObjectServerHandle)grpSrv, grpObjIdx); 
	
	uint16_t value;
	GrpObj_GetValue(grpObj, &value);
	GrpObj_clrCommFlags(grpObj, COMM_FLAG_UPDATE);
	
	if (colorFbk_GrpObj->red.grpObjIdx == grpObjIdx)
	{
		if (colorFbk_GrpObj->red.value)
		{
			rgbLedSetIdleFeedbackColorIntensity(colorFbk_Hndl->rgbLed_Hndl, COLOR_RED);
			rgbLedSetIdleState(colorFbk_Hndl->rgbLed_Hndl, LED_IDLE_FEEDBACK);
		}
		else
			rgbLedSetIdleState(colorFbk_Hndl->rgbLed_Hndl, LED_IDLE_DEFAULT);
	}
	else
		colorFbk_GrpObj->red.value = 0;
	if (colorFbk_GrpObj->green.grpObjIdx == grpObjIdx)
	{
		if (colorFbk_GrpObj->green.value)
		{
			rgbLedSetIdleFeedbackColorIntensity(colorFbk_Hndl->rgbLed_Hndl, COLOR_GREEN);
			rgbLedSetIdleState(colorFbk_Hndl->rgbLed_Hndl, LED_IDLE_FEEDBACK);
		}
		else
			rgbLedSetIdleState(colorFbk_Hndl->rgbLed_Hndl, LED_IDLE_DEFAULT);
	}
	else
		colorFbk_GrpObj->green.value = 0;
	if (colorFbk_GrpObj->blue.grpObjIdx == grpObjIdx)
	{
		if (colorFbk_GrpObj->blue.value)
		{
			rgbLedSetIdleFeedbackColorIntensity(colorFbk_Hndl->rgbLed_Hndl, COLOR_BLEU);
			rgbLedSetIdleState(colorFbk_Hndl->rgbLed_Hndl, LED_IDLE_FEEDBACK);
		}
		else
			rgbLedSetIdleState(colorFbk_Hndl->rgbLed_Hndl, LED_IDLE_DEFAULT);
	}
	else
		colorFbk_GrpObj->blue.value = 0;
	if (colorFbk_GrpObj->yellow.grpObjIdx == grpObjIdx)
	{
		if (colorFbk_GrpObj->yellow.value)
		{
			rgbLedSetIdleFeedbackColorIntensity(colorFbk_Hndl->rgbLed_Hndl, COLOR_YELLOW);
			rgbLedSetIdleState(colorFbk_Hndl->rgbLed_Hndl, LED_IDLE_FEEDBACK);
		}
		else
			rgbLedSetIdleState(colorFbk_Hndl->rgbLed_Hndl, LED_IDLE_DEFAULT);
	}
	else
		colorFbk_GrpObj->yellow.value = 0;
	if (colorFbk_GrpObj->magenta.grpObjIdx == grpObjIdx)
	{
		if (colorFbk_GrpObj->magenta.value)
		{
			rgbLedSetIdleFeedbackColorIntensity(colorFbk_Hndl->rgbLed_Hndl, COLOR_MAGENTA);
			rgbLedSetIdleState(colorFbk_Hndl->rgbLed_Hndl, LED_IDLE_FEEDBACK);
		}
		else
			rgbLedSetIdleState(colorFbk_Hndl->rgbLed_Hndl, LED_IDLE_DEFAULT);
	}
	else
		colorFbk_GrpObj->magenta.value = 0;
	if (colorFbk_GrpObj->cyan.grpObjIdx == grpObjIdx)
	{
		if (colorFbk_GrpObj->cyan.value)
		{
			rgbLedSetIdleFeedbackColorIntensity(colorFbk_Hndl->rgbLed_Hndl, COLOR_CYAN);
			rgbLedSetIdleState(colorFbk_Hndl->rgbLed_Hndl, LED_IDLE_FEEDBACK);
		}
		else
		rgbLedSetIdleState(colorFbk_Hndl->rgbLed_Hndl, LED_IDLE_DEFAULT);
	}
	else
		colorFbk_GrpObj->cyan.value = 0;
	
}

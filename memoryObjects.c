/*
 * memoryObjects.c
 *
 * Created: 1-8-2016 9:53:26
 *  Author: Paul
 */ 

#include "memoryObjects.h"



memHandle memInit(void *pMemory, uint16_t numBytes) {
	memHandle mem_handle;
	if (numBytes < sizeof(memObj))
	return ((memHandle)NULL);
	mem_handle = (memHandle)pMemory;
	return(mem_handle);
}

int8_t memSetup(memHandle handle) {
	memObj *mem;
	mem = (memObj *) handle;
		
	mem->ctrlBytes.descr.addrtable_load_status.address = 0xB6EA;
	mem->ctrlBytes.descr.assoctable_load_status.address = 0xB6EB;
	mem->ctrlBytes.descr.application_load_status.address = 0xB6EC;
	mem->ctrlBytes.descr.PeiprogLoadStatus.address = 0xB6ED;
	mem->ctrlBytes.descr.application_run_state_control.address = 0x0060;
	
	mem->ctrlBytes.descr.addrtable_load_status.value = 1;
	mem->ctrlBytes.descr.assoctable_load_status.value = 1;
	mem->ctrlBytes.descr.application_load_status.value = 1;
	mem->ctrlBytes.descr.PeiprogLoadStatus.value = 1;
	mem->ctrlBytes.descr.application_run_state_control.value = 0;


	
	return 0;
}
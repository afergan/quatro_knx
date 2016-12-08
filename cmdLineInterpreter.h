/*
 * cmdLineInterpreter.h
 *
 * Created: 27-11-2016 8:11:00
 *  Author: Paul
 */ 


#ifndef CMDLINEINTERPRETER_H_
#define CMDLINEINTERPRETER_H_

#include "cmdLineEditor.h"
#include "cmdHistory.h"
#include "console.h"
#include <avr/pgmspace.h>

#define COMMAND_FIFO_LENGTH			8

typedef struct {
	PGM_P	cmd;
	void	(*func)(void* parentHandle);
	PGM_P	helpTxt;
} command;


typedef struct {
	int8_t				enable;
	consoleHandle		console_handle;
	cleditorObj			cmdLine_obj;
	cleditorHandle		cmdLine_handle;
	cmdHistoryObj		cmdHistory_obj;
	struct _cmdBuffer	cmdHistory_buffer[COMMAND_FIFO_LENGTH];
	cmdHistoryHandle	cmdHistory_handle;
	command				*systemCommands;
	command				*userCommands;
	uint8_t				sysCmdCnt;
	uint8_t				usrCmdCnt;
} clinterpreterObj;

typedef struct clinterpreterObj *clinterpreterHandle;

clinterpreterHandle cmdLineInterpreterInit(void *pMemory, uint16_t numBytes);

int8_t cmdLineInterpreterSetup(clinterpreterHandle handle, consoleHandle console);

int8_t cmdLineRegisterUsrCmds(clinterpreterHandle handle, command *usrCmds, uint8_t usrCmdCnt);

int8_t cmdLineEnable(clinterpreterHandle handle);

int8_t cmdLineDisable(clinterpreterHandle handle);

int8_t cmdLineService(clinterpreterHandle handle);


#endif /* CMDLINEINTERPRETER_H_ */
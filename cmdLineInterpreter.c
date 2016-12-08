/*
 * cmdLineInterpreter.c
 *
 * Created: 27-11-2016 8:11:16
 *  Author: Paul
 */ 

#include "cmdLineInterpreter.h"
#include <string.h>
#include <stdio.h>

#define SYS_CMD_TBL \
        ENTRY("cls", CLS, "clear screen") \
        ENTRY("help,?", HELP, "help on commands") \
        ENTRY("info", INFO, "information")

#define ENTRY(a,b,c)	void Cmd_ ## b(void *parentHandle);
	SYS_CMD_TBL
#undef ENTRY

#define ENTRY(a,b,c)    const char CLI_CMD_STR_ ## b [] PROGMEM = a;
	SYS_CMD_TBL
#undef ENTRY

#define ENTRY(a,b,c)    const char CLI_HLP_STR_ ## b [] PROGMEM = c;
	SYS_CMD_TBL
#undef ENTRY

command sysCommands[] = {
#define ENTRY(a,b,c) {.cmd=(CLI_CMD_STR_ ## b), .func = Cmd_ ## b, .helpTxt=(CLI_HLP_STR_ ## b)},
    SYS_CMD_TBL
#undef ENTRY
};

/*
command sysCommands[] PROGMEM = {
	{"cls",		Cmd_clearScreen},
	{"help",	Cmd_HELP},
	{"h",		Cmd_HELP},
	{"?",		Cmd_HELP},
	{"/",		Cmd_HELP},
	{"info",	Cmd_INFO}
};
*/
uint8_t SYS_COMMANDS_N = sizeof (sysCommands)/ sizeof (command);

clinterpreterHandle cmdLineInterpreterInit(void *pMemory, uint16_t numBytes) {
	clinterpreterHandle cli_handle;
	if (numBytes < sizeof(clinterpreterObj))
		return ((clinterpreterHandle)NULL);
	cli_handle = (clinterpreterHandle)pMemory;
	return(cli_handle);		
}

int8_t cmdLineInterpreterSetup(clinterpreterHandle handle, consoleHandle console) {
	clinterpreterObj *cmd;
	cmd = (clinterpreterObj *) handle;
	
	cmd->console_handle = console;
	
	cmd->cmdHistory_handle = cmdHistInit(&cmd->cmdHistory_obj, sizeof(cmd->cmdHistory_obj));
	cmdHistSetup(cmd->cmdHistory_handle, cmd->cmdHistory_buffer, COMMAND_FIFO_LENGTH);
	
	cmd->cmdLine_handle = cleInit(&cmd->cmdLine_obj, sizeof(cmd->cmdLine_obj));
	cleSetup(cmd->cmdLine_handle, cmd->console_handle, cmd->cmdHistory_handle);
	
	cmd->systemCommands = sysCommands;

	cmd->sysCmdCnt = SYS_COMMANDS_N;
	
	cmd->usrCmdCnt = 0;
		
	return 0;
}

int8_t cmdLineRegisterUsrCmds(clinterpreterHandle handle, command *usrCmds, uint8_t usrCmdCnt) {
	clinterpreterObj *cmd;
	cmd = (clinterpreterObj *) handle;
	
	cmd->userCommands = usrCmds;
	cmd->usrCmdCnt = usrCmdCnt;
	
	return 0;
}

int8_t cmdLineEnable(clinterpreterHandle handle) {
	clinterpreterObj *cmd;
	cmd = (clinterpreterObj *) handle;
	
	cmd->enable = 1;
	
	cmdHistEnable(cmd->cmdHistory_handle);
	cleEnable(cmd->cmdLine_handle);
	
	return 0;
}

int8_t cmdLineDisable(clinterpreterHandle handle) {
	clinterpreterObj *cmd;
	cmd = (clinterpreterObj *) handle;
	
	cmd->enable = 0;
	
	cmdHistDisable(cmd->cmdHistory_handle);
	cleDisable(cmd->cmdLine_handle);
	
	return 0;
}

int8_t cmdLineService(clinterpreterHandle handle) {
	clinterpreterObj *cli;
	cli = (clinterpreterObj *) handle;
	
	if (cli->enable != 1)
		return -1;
		
	if (cleService(cli->cmdLine_handle) > 0)
	{

		for (uint8_t cmd = 0; cmd < cli->sysCmdCnt; cmd++)
		{	
			char cmdTokens[32];
			char del[2] = ",";
			strcpy_P(cmdTokens, cli->systemCommands[cmd].cmd);
			char *cmdToken = strtok(cmdTokens, del);
			
			while (cmdToken != NULL)
			{
				if (strcmp(cmdToken, (char*)(((cleditorObj*)(cli->cmdLine_handle))->command)) == 0)
				{
					if (cli->systemCommands[cmd].func)
						cli->systemCommands[cmd].func(handle);
					return 0;
				}
				cmdToken = strtok(NULL, del);
			}			
		}
		for (uint8_t cmd = 0; cmd < cli->usrCmdCnt; cmd++)
		{	
			char cmdTokens[32];
			char del[2] = ",";
			strcpy_P(cmdTokens, cli->userCommands[cmd].cmd);
			char *cmdToken = strtok(cmdTokens, del);
			
			while (cmdToken != NULL)
			{
				if (strcmp(cmdToken, (char*)(((cleditorObj*)(cli->cmdLine_handle))->command)) == 0)
				{
					if (cli->userCommands[cmd].func)
						cli->userCommands[cmd].func(handle);
					return 0;
				}
				cmdToken = strtok(NULL, del);
			}			
		}
		consoleWrtLiteral(cli->console_handle, "? for help\n\r");
	}
	
	return 0;
}

void Cmd_CLS(void *parentHandle) {
	clinterpreterObj *cli;
	cli = (clinterpreterObj *) parentHandle;	
	
	consoleClearScreen(cli->console_handle);
}

void Cmd_HELP(void *parentHandle) {
	clinterpreterObj *cli;
	cli = (clinterpreterObj *) parentHandle;
	
	consoleWrtLiteral(cli->console_handle, "**** help ***\n\r");
	for (uint8_t cmd = 0; cmd < cli->sysCmdCnt; cmd++)
	{
		consoleWrt_P(cli->console_handle, cli->systemCommands[cmd].cmd);
		consoleWrtLiteral(cli->console_handle, " : ");
		consoleWrt_P(cli->console_handle, cli->systemCommands[cmd].helpTxt);
		consoleNewline(cli->console_handle);
	}
	for (uint8_t cmd = 0; cmd < cli->usrCmdCnt; cmd++)
	{
		consoleWrt_P(cli->console_handle, cli->userCommands[cmd].cmd);
		consoleWrtLiteral(cli->console_handle, " : ");
		consoleWrt_P(cli->console_handle, cli->userCommands[cmd].helpTxt);
		consoleNewline(cli->console_handle);
	}
}

void Cmd_INFO(void *parentHandle) {
	clinterpreterObj *cli;
	cli = (clinterpreterObj *) parentHandle;
	
	consoleWrtLiteral(cli->console_handle, "**** info ***\n\r");
}
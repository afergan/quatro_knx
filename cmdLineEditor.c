/*
 * cmdLineEditor.c
 *
 * Created: 16-7-2016 10:41:58
 *  Author: Paul
 */ 

#include "clocks.h"
#include "interrupt.h"
#include <stdio.h>
#include <string.h>
#include "cmdLineEditor.h"

cleditorHandle cleInit(void *pMemory, uint16_t numBytes) {
	cleditorHandle cli_handle;
	if (numBytes < sizeof(cleditorObj))
		return ((cleditorHandle)NULL);
	cli_handle = (cleditorHandle)pMemory;
	return(cli_handle);	
}

int8_t cleSetup(cleditorHandle handle, consoleHandle console, cmdHistoryHandle cmdHist) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	cle->console_handle = console;
	cle->cmdHist_handle = cmdHist;
	cle->prompt = CLI_PROMPT_NOT_READY;
	return 0;
}

int8_t cleEnable(cleditorHandle handle) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (cle->console_handle)
	{
		cle->enable = 1;
		return 0;
	}
	else
		return -1;
}

int8_t cleDisable(cleditorHandle handle) {
	cleditorObj *cle;
	cle = (cleditorObj*) handle;	
	cle->enable = 0;
	return 0;
}

int8_t cleService(cleditorHandle handle) {
	cleditorObj *cle;
	cle = (cleditorObj*) handle;
	if (!cle->enable)
		return -1;
	
	int8_t result = 0;
	
	if (cle->cmdline.EscSeqState == CLI_ESC_PENDING)
	{
		if (isTimedout(&cle->cmdline.escTimeout)) // (clock.timestamp > cle->cmdline.escTimeout)
		{
			cle->cmdline.EscSeqState = CLI_ESC_TIMED_OUT; // most probably esc key pushed
			cle->cmdline.escSeqIdx = 0;
			uint8_t str[] = "^C";
			consoleWrite(cle->console_handle, (uint8_t*)str, sizeof(str));
			cle->cmdline.EscSeqState = CLI_ESC_NONE;
			if (cle->cmdHist_handle)
				pushCmdBuffer(cle->cmdHist_handle, NULL, 0);
			consoleNewline(cle->console_handle);
			cle->prompt = CLI_PROMPT_NOT_READY;
		}
	}
	
	if (cle->prompt == CLI_PROMPT_NOT_READY) // initialize command prompt
	{
		cle->cmdline.cursor = 0;
		cle->cmdline.length = 0;
		cle->prompt = CLI_PROMPT_READY;
		writeAnsiCtrl(cle->console_handle, ANSI_ERASE_LINE CR PROMPT_STR);
	}
	
	uint8_t byteRead;
	if (consoleReadByte(cle->console_handle, &byteRead) > 0)
	{
		switch(cleAnsiEscSeq(handle, byteRead)) {
			case CLI_ESC_NONE:
			{
				if (byteRead == 0x7f || byteRead == 0x08) // backspace
				{
					uint8_t len = 0;
					uint8_t *rebuildLine =cleBackspace(handle, &len);
					if (rebuildLine)
					{
						writeAnsiCtrl(cle->console_handle, ANSI_MOVE_CURSOR_LEFT ANSI_ERASE_TO_EOL ANSI_SAVE_CURSOR);
						consoleWrite(cle->console_handle, rebuildLine, len);
						writeAnsiCtrl(cle->console_handle, ANSI_RESTORE_CURSOR);
					}
				}
				else if (byteRead == 0x0d) // enter
				{
					cle->prompt = CLI_PROMPT_NOT_READY; // end command prompt
					if (cle->cmdHist_handle)
							pushCmdBuffer(cle->cmdHist_handle, cle->cmdline.line, cle->cmdline.length);
					extractArguments(handle);
					consoleNewline(cle->console_handle);
					result = 1;
				}
				else
				{
					uint8_t len = 0;
					uint8_t *rebuildLine =cleAddChar(handle, byteRead, &len);
					if (rebuildLine)
					{
						if (!len)
						consoleWriteByte(cle->console_handle, byteRead);
						else
						{
							writeAnsiCtrl(cle->console_handle, ANSI_ERASE_TO_EOL ANSI_SAVE_CURSOR);
							consoleWrite(cle->console_handle, rebuildLine, len);
							writeAnsiCtrl(cle->console_handle, ANSI_RESTORE_CURSOR ANSI_MOVE_CURSOR_RIGHT);
						}
					}
				}
			}
			break;
			case CLI_ESC_SEQ_ENDED:
			{
				switch (cle->cmdline.escSeq[cle->cmdline.escSeqIdx - 1]) {
					case 'A': // arrow up
					{
						if (cle->cmdHist_handle)
						{
							if (getNextCmdfromHistory(cle->cmdHist_handle, cle->cmdline.line, &(cle->cmdline.length)) >= 0)
							{
								cle->cmdline.cursor = cle->cmdline.length;
								writeAnsiCtrl(cle->console_handle, ANSI_ERASE_LINE CR PROMPT_STR);
								consoleWrite(cle->console_handle, cle->cmdline.line, cle->cmdline.length);
							}						
						}
					}
					break;
					case 'B': // arrow down
					{
						if (cle->cmdHist_handle)
						{
							if (getPreviousCmdfromHistory(cle->cmdHist_handle, cle->cmdline.line, &(cle->cmdline.length)) >= 0)
							{
								cle->cmdline.cursor = cle->cmdline.length;
								writeAnsiCtrl(cle->console_handle, ANSI_ERASE_LINE CR PROMPT_STR);
								consoleWrite(cle->console_handle, cle->cmdline.line, cle->cmdline.length);
							}						
						}
					}
					break;
					case 'C': // arrow right
					if (cleArrowRight(handle))
						consoleWrite(cle->console_handle, cle->cmdline.escSeq, cle->cmdline.escSeqIdx);
					break;
					case 'D': // arrow left
					if (cleArrowLeft(handle))
						consoleWrite(cle->console_handle, cle->cmdline.escSeq, cle->cmdline.escSeqIdx);
					break;
					case 0x7e: // tilde
					{
						switch(cle->cmdline.escSeq[cle->cmdline.escSeqIdx - 2]) {
							case '1': // home-key
							if (cleHome(handle))
							{
								writeAnsiCtrl(cle->console_handle, CR ANSI_MOVE_CURSOR_RIGHT);
							}
							break;
							case '2': // insert-key
							if (cleInsertKey(handle))
								consoleWrite(cle->console_handle, cle->cmdline.escSeq, cle->cmdline.escSeqIdx);
							break;
							case '3': // delete-key
							{
								uint8_t len = 0;
								uint8_t *rebuildLine = cleDeleteKey(handle, &len);
								if (rebuildLine)
								{
									writeAnsiCtrl(cle->console_handle, ANSI_ERASE_TO_EOL ANSI_SAVE_CURSOR);
									consoleWrite(cle->console_handle, rebuildLine, len);
									writeAnsiCtrl(cle->console_handle, ANSI_RESTORE_CURSOR);
								}
							}
							break;
							case '4': // end-key
							if (cleEnd(handle))
							{
								writeAnsiCtrl(cle->console_handle, ANSI_ERASE_LINE CR PROMPT_STR);
								consoleWrite(cle->console_handle, cle->cmdline.line, cle->cmdline.length);
							}
							break;
							default:
							{
								consoleWriteHex(cle->console_handle, cle->cmdline.escSeq, cle->cmdline.escSeqIdx);
							}
							break;
						}
						break;
					}
					case 'c':
						// return form a reset. [?6c means VT102
					break;
					default:
					{
						consoleWriteHex(cle->console_handle, cle->cmdline.escSeq, cle->cmdline.escSeqIdx);
					}
					break;
				}
			}
			break;
			default:
			break;
		}
	}
	return result;	
}

int8_t extractArguments(cleditorHandle handle) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	
	uint8_t *cmdPtr;
	cmdPtr = cle->cmdline.line;
					
	*(cmdPtr + cle->cmdline.length) = NULLCHAR;
	
	cle->command = cle->cmdline.line;
	cle->argCnt = 0;				
	
	// split command and params. replace space with null chars
	while (*(cmdPtr++) != NULLCHAR)
	{
		while (*(cmdPtr) > SPACE) cmdPtr++;
		while (*(cmdPtr) == SPACE) *(cmdPtr++) = NULLCHAR;
		if (*(cmdPtr) > SPACE  && cle->argCnt < MAX_PARAMS) cle->argVal[cle->argCnt++] = cmdPtr;
	}
	return 0;
}

int8_t cleEnter(cleditorHandle handle) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
		return -1;
	
	return 0;
}

uint8_t *cleAddChar(cleditorHandle handle, uint8_t asciiChar, uint8_t *length) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
	return NULL;
	if (cle->cmdline.length >= CMD_LINE_LENGTH - 1)
	return NULL;
	if (asciiChar >= 0x20 && asciiChar <= 126)
	{
		if (cle->cmdline.cursor == cle->cmdline.length)
		{
			cle->cmdline.line[cle->cmdline.length++] = asciiChar; // place at the end
			cle->cmdline.cursor = cle->cmdline.length;
			*length = 0;
		}
		else
		{
			for (int8_t i = cle->cmdline.length; i >= cle->cmdline.cursor; i--)
			{
				cle->cmdline.line[i+1] = cle->cmdline.line[i];
			}
			cle->cmdline.length++;
			cle->cmdline.line[cle->cmdline.cursor++] = asciiChar;
			*length = (cle->cmdline.length - cle->cmdline.cursor) + 1;
		}
		uint8_t *tmp = cle->cmdline.line + cle->cmdline.cursor - 1;
		return tmp;
	}
	return NULL;
}

uint8_t *cleBackspace(cleditorHandle handle, uint8_t *length) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
	return NULL;
	if (cle->cmdline.cursor > 0)
	{
		cle->cmdline.cursor--;
		if (cle->cmdline.cursor != cle->cmdline.length)
		{
			for (int8_t i = cle->cmdline.cursor; i < cle->cmdline.length; i++)
			{
				cle->cmdline.line[i] = cle->cmdline.line[i+1];
			}
		}
		cle->cmdline.length--;
		*length = cle->cmdline.length - cle->cmdline.cursor;
		return (cle->cmdline.line + cle->cmdline.cursor);
	}
	return NULL;
}

int8_t cleDelChar(cleditorHandle handle) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
	return -1;
	return 0;
}

cli_esc_seq_e cleAnsiEscSeq(cleditorHandle handle, uint8_t asciiChar) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
	return -1;
	if (cle->cmdline.EscSeqState == CLI_ESC_NONE && cle->cmdline.escSeqIdx > 0)
	{
		cle->cmdline.escSeqIdx = 0;
	}

	if (cle->cmdline.EscSeqState == CLI_ESC_SEQ_ENDED)
		cle->cmdline.EscSeqState = CLI_ESC_NONE;

	if (asciiChar == 0x1b)
	{
		cle->cmdline.EscSeqState = CLI_ESC_PENDING;
		cle->cmdline.escSeqIdx = 0;
		setupTimeoutTmr(&cle->cmdline.escTimeout, CMD_LINE_ESC_TIMEOUT);
		//cle->cmdline.escTimeout = clock.timestamp + CMD_LINE_ESC_TIMEOUT;
	}
	if (cle->cmdline.EscSeqState == CLI_ESC_PENDING)
	{
		cle->cmdline.escSeq[cle->cmdline.escSeqIdx++] = asciiChar;
		if (((asciiChar >= 64 && asciiChar <= 126) && !(asciiChar == '[')))
		{
			cle->cmdline.EscSeqState = CLI_ESC_SEQ_ENDED;
		}

	}
	return cle->cmdline.EscSeqState;
}

int8_t cleEscape(cleditorHandle handle, cli_esc_seq_e *escState) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
	return -1;
	if (cle->cmdline.EscSeqState == CLI_ESC_PENDING)
	{
		if (isTimedout(&cle->cmdline.escTimeout)) // (clock.timestamp > cle->cmdline.escTimeout)
		{
			cle->cmdline.EscSeqState = CLI_ESC_TIMED_OUT;
			cle->cmdline.escSeqIdx = 0;
		}
	}
	*escState = cle->cmdline.EscSeqState;
	return 0;
}

int8_t cleArrowRight(cleditorHandle handle) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
	return -1;
	if (cle->cmdline.cursor < cle->cmdline.length)
	{
		cle->cmdline.cursor++;
		return 1;
	}
	else
	return 0;
}

int8_t cleArrowLeft(cleditorHandle handle) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
	return -1;
	if (cle->cmdline.cursor > 0)
	{
		cle->cmdline.cursor--;
		return 1;
	}
	else
	return 0;
}

int8_t cleArrowUp(cleditorHandle handle) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
	return -1;
	
	return 0;
}

int8_t cleArrowDown(cleditorHandle handle) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
	return -1;
	return 0;
}

int8_t cleHome(cleditorHandle handle) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
	return -1;
	cle->cmdline.cursor = 0;
	return 1;
}

int8_t cleEnd(cleditorHandle handle) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
	return -1;
	cle->cmdline.cursor = cle->cmdline.length;
	return 1;
}

int8_t cleInsertKey(cleditorHandle handle) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
	return -1;
	return 0;
}

uint8_t *cleDeleteKey(cleditorHandle handle, uint8_t *length) {
	cleditorObj *cle;
	cle = (cleditorObj *) handle;
	if (!cle->enable)
	return NULL;
	if (cle->cmdline.cursor != cle->cmdline.length)
	{
		for (int8_t i = cle->cmdline.cursor; i < cle->cmdline.length; i++)
		{
			cle->cmdline.line[i] = cle->cmdline.line[i+1];
		}
		cle->cmdline.length--;
		*length = cle->cmdline.length - cle->cmdline.cursor;

		return (cle->cmdline.line + cle->cmdline.cursor);
	}
	return NULL;
}
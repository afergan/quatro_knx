/*
 * cli.h
 *
 * Created: 16-7-2016 10:42:11
 *  Author: Paul
 */ 


#ifndef CMDLINEEDITOR_H_
#define CMDLINEEDITOR_H_

#include "console.h"
#include "cmdHistory.h"


#define MAX_PARAMS					5

#define CMD_LINE_ESC_TIMEOUT		100
#define PROMPT_CHAR					'>'
#define PROMPT_STR					">"

typedef enum {
	CLI_ESC_NONE,
	CLI_ESC_PENDING,
	CLI_ESC_TIMED_OUT,
	CLI_ESC_SEQ_ENDED
} cli_esc_seq_e;

typedef enum {
	CLI_PROMPT_NOT_READY,
	CLI_PROMPT_READY	
} cli_prompt_state_e;
	
	
struct cmdline_s {
	uint8_t							line[CMD_LINE_LENGTH];
	uint8_t							length;
	uint8_t							cursor;
	cli_esc_seq_e					EscSeqState;
	uint8_t							escSeq[8];
	uint8_t							escSeqIdx;
	timeoutTmr						escTimeout;
};

typedef struct {
	int8_t							enable;
	consoleHandle					console_handle;
	cmdHistoryHandle				cmdHist_handle;
	struct cmdline_s				cmdline;
	uint8_t							*command;
	uint8_t							*argVal[5];
	uint8_t							argCnt;
	cli_prompt_state_e				prompt;
} cleditorObj;

typedef struct cleditorObj *cleditorHandle;

cleditorHandle cleInit(void *pMemory, uint16_t numBytes);

int8_t cleSetup(cleditorHandle handle, consoleHandle console, cmdHistoryHandle cmdHist);

int8_t cleEnable(cleditorHandle handle);

int8_t cleDisable(cleditorHandle handle);

int8_t cleService(cleditorHandle handle);

int8_t extractArguments(cleditorHandle handle);

int8_t cleEnter(cleditorHandle handle);

uint8_t *cleAddChar(cleditorHandle handle, uint8_t asciiChar, uint8_t *length);

uint8_t *cleBackspace(cleditorHandle handle, uint8_t *length);

int8_t cleDelChar(cleditorHandle handle);

cli_esc_seq_e cleAnsiEscSeq(cleditorHandle handle, uint8_t asciiChar);

int8_t cleEscape(cleditorHandle handle, cli_esc_seq_e *escState);

int8_t cleArrowRight(cleditorHandle handle);

int8_t cleArrowLeft(cleditorHandle handle);

int8_t cleArrowUp(cleditorHandle handle);

int8_t cleArrowDown(cleditorHandle handle);

int8_t cleHome(cleditorHandle handle);

int8_t cleEnd(cleditorHandle handle);

int8_t cleInsertKey(cleditorHandle handle);

uint8_t *cleDeleteKey(cleditorHandle handle, uint8_t *length);

#endif /* CMDLINEEDITOR_H_ */
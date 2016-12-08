/*
 * cmdFifo.h
 *
 * Created: 15-7-2016 19:46:58
 *  Author: Paul
 */ 


#ifndef CMDHISTORY_H_
#define CMDHISTORY_H_


#define CMD_LINE_LENGTH				32

typedef enum {
	BUF_FREE,
	BUF_FILLING,
	BUF_OCCUPIED
} buf_state_e;

struct _cmdBuffer {
	buf_state_e					state;
	volatile uint8_t			length;
	struct _cmdBuffer			*next;
	uint8_t						data[CMD_LINE_LENGTH];
};

typedef struct _cmdHistoryObj {
	uint8_t					enabled;
	uint8_t					historyLength;
	struct _cmdBuffer		*first;
	struct _cmdBuffer		*traverse;
	struct _cmdBuffer		*stack;
} cmdHistoryObj;

typedef struct _cmdHistoryObj *cmdHistoryHandle;

cmdHistoryHandle cmdHistInit(void *pMemory, uint16_t numBytes);

int8_t cmdHistSetup(cmdHistoryHandle handle, struct _cmdBuffer *fb, uint8_t histLen);

int8_t cmdHistReset(cmdHistoryHandle handle);

int8_t cmdHistEnable(cmdHistoryHandle handle);

int8_t cmdHistDisable(cmdHistoryHandle handle);

struct _cmdBuffer *allocNewCmdBuffer(cmdHistoryHandle handle);

int8_t pushCmdBuffer(cmdHistoryHandle handle, uint8_t *data, int8_t length);

int8_t getNextCmdfromHistory(cmdHistoryHandle handle, uint8_t *data, uint8_t *length);

int8_t getPreviousCmdfromHistory(cmdHistoryHandle handle, uint8_t *data, uint8_t *length);

#endif /* CMDHISTORY_H_ */
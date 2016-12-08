/*
 * touchPad.h
 *
 * Created: 8-6-2016 7:51:33
 *  Author: Paul
 */ 


#ifndef TOUCHPAD_H_
#define TOUCHPAD_H_

#include "applicationInterface.h"

#define TOUCHPAD_TOTAL				5
#define TOUCH_COMBINATIONS_TOTAL	16

#define TOUCH_BAD_CNT_THRESHOLD		10

#define TOUCH_SCAN_PERIOD_USEC		20000LL

#define TOUCH_DEBOUNCE_MS			30L
#define MULTI_TOUCH_TIMEOUT_MS		300L
#define LONG_TOUCH_MS				1000L
#define STEADY_TOUCH_MS				100L
#define TOUCH_DOUBLE_TIMEOUT_MS		800L
#define TOUCH_TIMEOUT_MS			3000L
#define LONG_TOUCH_TIMEOUT_MS		3000L
#define DOUBLE_TOUCH_STATE_DELAY	500L
#define TOUCH_ERROR_END_DELAY		1500L
#define TOUCH_END_DELAY				200L
#define TOUCH_ERROR_TIMEOUT_MS		3000L
#define TOUCH_RESET_TIME_MS			500L

typedef enum {
	TOUCH_TOP = 0,
	TOUCH_BOTTOM = 1,
	TOUCH_LEFT = 2,
	TOUCH_RIGHT = 3,
	TOUCH_CENTER = 4,
	TOUCH_TOP_BOTTOM = 5,
	TOUCH_LEFT_RIGHT = 6,
	TOUCH_TOP_LEFT = 7,
	TOUCH_TOP_RIGHT = 8,
	TOUCH_BOTTOM_LEFT = 9,
	TOUCH_BOTTOM_RIGHT = 10,
	TOUCH_CENTER_TOP = 11,
	TOUCH_CENTER_BOTTOM = 12,
	TOUCH_CENTER_LEFT = 14,
	TOUCH_CENTER_RIGHT = 15,  // do not go past 15, as this enum is also used as a bit position in unsigned int16
	TOUCH_RESET = 16,
	TOUCH_ALIVE = 17,
	TOUCH_PAD_CLEANING = 18,
	TOUCH_ERROR = 19,
	TOUCH_UNKNOWN = 20
} touch_state_e;

typedef enum {
	TOUCH_SEQ_IDLE,
	TOUCH_SEQ_1,
	TOUCH_SEQ_2,
	TOUCH_SEQ_3,
	TOUCH_SEQ_4,
	TOUCH_SEQ_5,
	TOUCH_SEQ_ERROR,
	TOUCH_SEQ_ERROR_END,
	TOUCH_SEQ_RESET
} touch_seq_e;


typedef enum {
	TOUCH_NO_EVENT,
	TOUCH_UNKNOWN_EVENT,
	TOUCH_ERROR_EVENT,
	TOUCH_RESET_EVENT,
	TOUCH_COMING_ALIVE_EVENT,
	TOUCH_EVENT_DOWN,
	TOUCH_EVENT_UP,
	TOUCH_SHORT_PRESS_EVENT,
	TOUCH_LONG_PRESS_DOWN_EVENT,
	TOUCH_LONG_PRESS_UP_EVENT,
	TOUCH_DOUBLE_PRESS_EVENT
} touch_event_e;

typedef enum {
	MULTI_TOUCH_NONE,
	MULTI_TOUCH_PRESSED,
	MULTI_TOUCH_RELEASED,
	MULTI_TOUCH_ERROR
} multi_touch_e;

// Touch pad layout
//
// maximum 5 simultaneous pads (5 input pins). In total 9 configurations.
// pads can be used in single touch or multi touch.

//
// Configuration "QuadArrow"
//
// +-----------------------------+
// |\                           /|
// |   \         TOP         /   |
// |      \               /      |
// |         \         /         |
// |            \   /            |
// |   LEFT       o     RIGHT    |
// |            /   \            |
// |         /         \         |
// |      /               \      |
// |   /       BOTTOM        \   |
// |/                           \|
// +-----------------------------+
//
// Configuration "Quad"
//
// +--------------+--------------+
// |              |              |
// |              |              |
// |     TOP      |     RIGHT    |
// |              |              |
// |              |              |
// +--------------+--------------+
// |              |              |
// |              |              |
// |     LEFT     |    BOTTOM    |
// |              |              |
// |              |              |
// +--------------+--------------+
//
// Configuration "DualVertical"
//
// +--------------+--------------+
// |              |              |
// |              |              |
// |              |              |
// |              |              |
// |              |              |
// |    LEFT      |    RIGHT     |
// |              |              |
// |              |              |
// |              |              |
// |              |              |
// |              |              |
// +--------------+--------------+
//
// Configuration "DualHorizontal"
//
// +-----------------------------+
// |                             |
// |                             |
// |             TOP             |
// |                             |
// |                             |
// +-----------------------------+
// |                             |
// |                             |
// |           BOTTOM            |
// |                             |
// |                             |
// +-----------------------------+
//
// Configuration "Single"
//
// +-----------------------------+
// |                             |
// |                             |
// |                             |
// |                             |
// |                             |
// |           CENTER            |
// |                             |
// |                             |
// |                             |
// |                             |
// |                             |
// +-----------------------------+
//
// Configuration "TripleHorizontal"
//
// +-----------------------------+
// |                             |
// |            TOP              |
// |                             |
// +-----------------------------+
// |                             |
// |           CENTER            |
// |                             |
// +-----------------------------+
// |                             |
// |           BOTTOM            |
// |                             |
// +-----------------------------+
//
// Configuration "TripleVertical"
//
// +---------+---------+---------+
// |         |         |         |
// |         |         |         |
// |         |         |         |
// |         |         |         |
// |         |         |         |
// |  LEFT   | CENTER  |  RIGHT  |
// |         |         |         |
// |         |         |         |
// |         |         |         |
// |         |         |         |
// |         |         |         |
// +---------+---------+---------+
//
// Configuration "ArrowCenter"
//
// +-----------------------------+
// | \                         / |
// |    \        TOP        /    |
// |       \             /       |
// |         +---------+         |
// |         |         |         |
// |  LEFT   | CENTER  |  RIGHT  |
// |         |         |         |
// |         +---------+         |
// |       /             \       |
// |    /      BOTTOM       \    |
// | /                         \ |
// +-----------------------------+
//
// Configuration "Bricks"
//
// +-------------------+---------+
// |                   |         |
// |        TOP        |    R    |
// |                   |    I    |
// +---------+---------+    G    |
// |         |         |    H    |
// |         | CENTER  |    T    |
// |    L    |         |         |
// |    E    +---------+---------+
// |    F    |                   |
// |    T    |       BOTTOM      |
// |         |                   |
// +---------+-------------------+


typedef enum {
	PAD_CFG_UNKNOWN,
	PAD_CFG_QUAD_ARROW,
	PAD_CFG_QUAD,
	PAD_CFG_DUAL_VERTICAL,
	PAD_CFG_DUAL_HORIZONTAL,
	PAD_CFG_SINGLE,
	PAD_CFG_TRIPLE_HORIZONTAL,
	PAD_CFG_TRIPLE_VERTICAL,
	PAD_CFG_ARROW_CENTER,
	PAD_CFG_BRICKS
} pad_config_e;

typedef struct {
	uint8_t					enabled;	
	PORT_t					*inputPort;
	uint8_t					bitNumber;
	uint8_t					eventMux; // do not fill in.
} pad_port_config;

typedef void (*event_cb_t)(touch_event_e tevent, touch_state_e pad);

typedef struct {
	uint8_t					enabled;
	A_InterfaceHandle		AI_Hndl;
	pad_port_config			*padPortCfg;
	PORT_t					*ctrlPort;
	uint8_t					enableBit;
	uint8_t					syncBit;
	TC0_t					*syncTmr;
	touch_state_e			state;
	touch_event_e			touchEvent;
	event_cb_t				eventCb;
	int16_t					doublePressEnableMask; // bit 0 is top, bit 1 is bottom according to enum
	int16_t					longPressEnableMask;
	int16_t					edgePressEnableMask;
	int16_t					shortPressEnableMask;
	touch_seq_e				sequence;
	
	timeoutTmr				debounce;
	timeoutTmr				steadyState;
	timeoutTmr				timeout;
	timeoutTmr				longTouch;
	timeoutTmr				multiTouchTime;	
	
	uint8_t					touchBitState;
	uint8_t					debouncedTouchBitState;
	uint8_t					healthIndication;
	uint8_t					healthChkPinCycle;
	uint8_t					healthChkBadCnt;
	
	// interface object
	
	// load control
	sGeneric10				loadCtrl;
	
	// PID_TOUCHPAD_CFG
	struct {
		uint16_t		cnt;
		pad_config_e	value;
		}					configuration;
		
} touchPadObj;

typedef touchPadObj *touchPadHandle;


touchPadHandle touchPadInit(void *pMemory, uint16_t numBytes);

int8_t touchPadSetup(touchPadHandle handle, A_InterfaceHandle AI_Hndl, PORT_t *cPort, uint8_t eb, uint8_t sb, pad_port_config *padPortCfg, TC0_t *tmr, event_cb_t eCb);

int8_t touchPadConfigure(touchPadHandle handle,  pad_config_e padCfg, int16_t edgePressEnableMask, int16_t doublePressEnableMask, int16_t longPressEnableMask);

int8_t touchPadEnable(touchPadHandle handle);

int8_t touchPadDisable(touchPadHandle handle);

int8_t touchPadReset(touchPadHandle handle);

void touchPadIsr(touchPadHandle handle);

void touchPadStateAndEvents(touchPadHandle handle, uint8_t edgeChange);

int8_t touchPadRaiseEvent(touchPadHandle handle, touch_event_e tevent, touch_state_e state);

int8_t touchPadGetState(touchPadHandle handle, touch_state_e *state);

touch_state_e touchPadMapBitToState(touchPadHandle handle, uint8_t touchBitState);

multi_touch_e touchPadMultiTouchTransition(touch_state_e before, touch_state_e now);

int8_t touchPadPropInd(void *parentHandle, interfaceProperty *prop);

#endif /* TOUCHPAD_H_ */
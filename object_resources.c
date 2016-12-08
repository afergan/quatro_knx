/*
 * object_resources.c
 *
 * Created: 20-11-2016 9:05:10
 *  Author: Paul
 */ 

#include "object_resources.h"


uint8_t getPDTsize(PDT_e pdt) {
	switch (pdt) {
		case PDT_CONTROL:
		return 10;
		case PDT_CHAR:
		return 1;
		case PDT_UNSIGNED_CHAR:
		return 1;
		case PDT_INT:
		return 2;
		case PDT_UNSIGNED_INT:
		return 2;
		case PDT_EIB_FLOAT:
		return 2;
		case PDT_DATE:
		return 3;
		case PDT_TIME:
		return 3;
		case PDT_LONG:
		return 4;
		case PDT_UNSIGNED_LONG:
		return 4;
		case PDT_FLOAT:
		return 4;
		case PDT_DOUBLE:
		return 4;
		case PDT_CHAR_BLOCK:
		return 10;
		case PDT_POLL_GROUP_SETTING:
		return 3;
		case PDT_SHORT_CHAR_BLOCK:
		return 5;
		case PDT_DATE_TIME:
		return 8;
		case PDT_VARIABLE_LENGTH:
		return 0; // variable length
		case PDT_GENERIC01:
		return 1;
		case PDT_GENERIC02:
		return 2;
		case PDT_GENERIC03:
		return 3;
		case PDT_GENERIC04:
		return 4;
		case PDT_GENERIC05:
		return 5;
		case PDT_GENERIC06:
		return 6;
		case PDT_GENERIC07:
		return 7;
		case PDT_GENERIC08:
		return 8;
		case PDT_GENERIC09:
		return 9;
		case PDT_GENERIC10:
		return 10;
		case PDT_GENERIC11:
		return 11;
		case PDT_GENERIC12:
		return 12;
		case PDT_GENERIC13:
		return 13;
		case PDT_GENERIC14:
		return 14;
		case PDT_GENERIC15:
		return 15;
		case PDT_GENERIC16:
		return 16;
		case PDT_GENERIC17:
		return 17;
		case PDT_GENERIC18:
		return 18;
		case PDT_GENERIC19:
		return 19;
		case PDT_GENERIC20:
		return 20;
		case PDT_VERSION:
		return 2;
		case PDT_ALARM_INFO:
		return 6;
		case PDT_BINARY_INFORMATION:
		return 1;
		case PDT_BITSET8:
		return 1;
		case PDT_BITSET16:
		return 2;
		case PDT_ENUM8:
		return 1;
		case PDT_SCALING:
		return 1;
		case PDT_NE_VL:
		return 0; // undefined
		case PDT_NE_FL:
		return 0; // undefined
		case PDT_FUNCTION:
		return 0; // undefined
		default:
		return 0;
	}
}

PDT_e getPDTfromPropType(PropertyId_e propId) {
	switch(propId) {
		case PID_OBJECT_TYPE:
		return PDT_UNSIGNED_INT;
		case PID_OBJECT_NAME:
		return PDT_CHAR;
		case PID_LOAD_STATE_CONTROL:
		return PDT_CONTROL;
		case PID_RUN_STATE_CONTROL:
		return PDT_CONTROL;
		case PID_TABLE_REFERENCE:
		return PDT_UNSIGNED_LONG;
		case PID_SERVICE_CONTROL:
		return PDT_UNSIGNED_INT;
		case PID_FIRMWARE_REVISION:
		return PDT_UNSIGNED_CHAR;
		case PID_SERIAL_NUMBER:
		return PDT_GENERIC06;
		case PID_MANUFACTURER_ID:
		return PDT_UNSIGNED_INT;
		case PID_PROGRAM_VERSION:
		return PDT_GENERIC05;
		case PID_DEVICE_CONTROL:
		return PDT_BITSET8;
		case PID_ORDER_INFO:
		return PDT_GENERIC10;
		case PID_PEI_TYPE:
		return PDT_UNSIGNED_CHAR;
		case PID_PORT_CONFIGURATION:
		return PDT_UNSIGNED_CHAR;
		case PID_POLL_GROUP_SETTINGS:
		return PDT_POLL_GROUP_SETTING;
		case PID_MANUFACTURER_DATA:
		return PDT_GENERIC04;
		case PID_DESCRIPTION:
		return PDT_UNSIGNED_CHAR;
		case PID_ENROL:
		return PDT_FUNCTION;
		case PID_VERSION:
		return PDT_VERSION;
		case PID_MCB_TABLE:
		return PDT_GENERIC08;
		case PID_ERROR_CODE:
		return PDT_ENUM8;
		case PID_OBJECT_INDEX:
		return PDT_UNSIGNED_CHAR;
		case PID_DOWNLOAD_COUNTER:
		return PDT_UNSIGNED_INT;
		case PID_ROUTING_COUNT:		// object type 0
		return PDT_UNSIGNED_CHAR;
		case PID_MAX_RETRY_COUNT:
		return PDT_GENERIC01;
		case PID_ERROR_FLAGS:
		return PDT_UNSIGNED_CHAR;
		case PID_PROGMODE:
		return PDT_BITSET8;
		case PID_PRODUCT_ID:
		return PDT_GENERIC10;
		case PID_MAX_APDULENGTH:
		return PDT_UNSIGNED_INT;
		case PID_SUBNET_ADDR:
		return PDT_UNSIGNED_CHAR;
		case PID_DEVICE_ADDR:
		return PDT_UNSIGNED_CHAR;
		case PID_PB_CONFIG:
		return PDT_GENERIC04;
		case PID_ADDR_REPORT:
		return PDT_GENERIC06;
		case PID_ADDR_CHECK:
		return PDT_GENERIC01;
		case PID_OBJECT_VALUE:
		return PDT_FUNCTION;
		case PID_OBJECTLINK:
		return PDT_FUNCTION;
		case PID_APPLICATION:
		return PDT_FUNCTION;
		case PID_PARAMETER:
		return PDT_FUNCTION;
		case PID_OBJECTADDRESS:
		return PDT_FUNCTION;
		case PID_PSU_TYPE:
		return PDT_UNSIGNED_INT;
		case PID_PSU_STATUS:
		return PDT_BINARY_INFORMATION;
		case PID_PSU_ENABLE:
		return PDT_ENUM8;
		case PID_DOMAIN_ADDRESS:
		return PDT_UNSIGNED_INT;
		case PID_IO_LIST:
		return PDT_UNSIGNED_INT;
		case PID_MGT_DESCRIPTOR_01:
		return PDT_GENERIC10;
		case PID_PL110_PARAM:
		return PDT_GENERIC01;
		case PID_RECEIVE_BLOCK_TABLE:
		return PDT_UNSIGNED_CHAR; // array of 16
		case PID_RANDOM_PAUSE_TABLE:
		return PDT_UNSIGNED_CHAR; // array of 12
		case PID_RECEIVE_BLOCK_NR:
		return PDT_UNSIGNED_CHAR;
		case PID_HARDWARE_TYPE:
		return PDT_GENERIC06;
		case PID_RETRANSMITTER_NUMBER:
		return PDT_UNSIGNED_CHAR;
		case PID_SERIAL_NR_TABLE:
		return PDT_GENERIC06; // array
		case PID_BIBATMASTER_ADDRESS:
		return PDT_UNSIGNED_INT;
		case PID_RF_DOMAIN_ADDRESS:
		return PDT_GENERIC06;
		case PID_DEVICE_DESCRIPTOR:
		return PDT_GENERIC02;
		case PID_GROUP_TELEGR_RATE_LIMIT_TIME_BASE:
		return PDT_UNSIGNED_INT;
		case PID_GROUP_TELEGR_RATE_LIMIT_NO_OF_TELEGR:
		return PDT_UNSIGNED_INT;
		default:
		return PDT_UNKNOWN;
	}
}

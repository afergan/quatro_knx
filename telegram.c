/*
 * telegram.c
 *
 * Created: 18-8-2016 14:33:47
 *  Author: Paul
  

#include "telegram.h"
#include "console.h"
#include <string.h>
#include <stdio.h>

telegramHandle telegramInit(void *pMemory, uint16_t numBytes) {
	telegramHandle telegram_handle;
	if (numBytes < sizeof(telegramObj))
	return ((telegramHandle)NULL);
	telegram_handle = (telegramHandle)pMemory;
	return(telegram_handle);
}

int8_t tgSetControlByte(uint8_t *telegram, repeat_e repeat, priority_e priority) {
	*telegram = 0x90 | ((uint8_t)(repeat) << 5) | ((uint8_t)priority << 2);
	return 0;
}

int8_t tgSetNPCI_Byte(uint8_t *telegram, address_type_e addressType, uint8_t routeCount, uint8_t length) {
	*(telegram + 5) = addressType << 7 | (routeCount & 0x07) << 4 | (length &0x0f);
	return 0;
}

int8_t tgSetTPDU_p2pCtrl(uint8_t *telegram, tpci_e transportType, uint8_t packetCnt, p2p_e control) {
	*(telegram + 6) = transportType << 6 | (packetCnt & 0x0f) << 2 | control;
	return 0;
}

int8_t tgSetTPDU_data(uint8_t *telegram, tpci_e transportType, uint8_t packetCnt, apci_e service) {
	*(telegram + 6)= transportType << 6 | (packetCnt & 0x0f) << 2 | (service >> 2);
	*(telegram + 7) = (uint8_t)service << 6;
	return 0;
}

int8_t tgSetTPDU_escape(uint8_t *telegram, escape_apdu_e escService) {
	*(telegram + 7) |= (uint8_t)escService;
	return 0;
}

int8_t tgSetSrcAddress(uint8_t *telegram, uint16_t address) {
	*(telegram + 1) = address >> 8;
	*(telegram + 2) = address & 0xff;
	return 0;
}

int8_t tgSetDstAddress(uint8_t *telegram, uint16_t address) {
	*(telegram + 3) = address >> 8;
	*(telegram + 4) = address & 0xff;
	return 0;
}

int8_t tgSetSixBitValue(uint8_t *telegram, uint8_t value) {
	*(telegram + 7) |= value & 0x3f;
	return 0;
}

int8_t tgSetBytesValue(uint8_t *telegram, uint8_t *value, uint8_t byteCnt) {
	for (uint8_t i = 0; i < byteCnt; i++)
		*(telegram + 8 + i) = value[i];
	return 0;
}

int8_t tgSetObjectIdx(uint8_t *telegram, uint8_t objIdx) {
	*(telegram + 8) = objIdx;
	return 0;
}

int8_t tgSetPropId(uint8_t *telegram, uint8_t propId) {
	*(telegram + 9) = propId;
	return 0;
}

int8_t tgSetPropIdx(uint8_t *telegram, uint8_t propIdx) {
	*(telegram + 10) = propIdx;
	return 0;
}

int8_t tgSetPropDataType(uint8_t *telegram, PDT_e pdt) {
	*(telegram + 11) &= ~(0x3f);
	*(telegram + 11) |= (pdt & 0x3f);
	return 0;
}

int8_t tgSetPropWriteable(uint8_t *telegram, uint8_t wrtEnable) {
	*(telegram + 11) &= ~(0x80);
	if (wrtEnable)
		*(telegram + 11) |= 0x80;
	return 0;
}

int8_t tgSetMemoryStartAddrCnt(uint8_t * telegram, uint16_t startAddr, uint8_t Cnt) {
	*(telegram + 7) |= Cnt & 0x0F;
	*(telegram + 8) = startAddr >> 8;
	*(telegram + 9) = startAddr & 0xff;	
	return 0;
}

int8_t tgSetPropMaxElements(uint8_t * telegram, uint16_t maxElements) {
	*(telegram + 12) = maxElements >> 8;
	*(telegram + 13) = maxElements & 0xff;
	return 0;
}

int8_t tgSetPropAccessLvl(uint8_t *telegram, uint8_t readAccessLvl, uint8_t writeAccessLvl) {
	*(telegram + 14) = readAccessLvl << 4 | (writeAccessLvl & 0x0f);
	return 0;
}

int8_t tgSetPropStartCntElement(uint8_t *telegram, uint16_t startElement, uint8_t elementCnt) {
	*(telegram + 10) = (elementCnt << 4) | ((startElement >> 8) & 0x0F);
	*(telegram + 11) = startElement & 0xFF;	
	return 0;
}

int8_t tgSetMaskVersion(uint8_t *telegram, uint16_t maskVersion) {
	*(telegram + 8) = maskVersion >> 8;
	*(telegram + 9) = maskVersion & 0xff;
	return 0;
}

int8_t tgSetADCresult(uint8_t *telegram, uint8_t channel, uint8_t numberOfConversions, uint16_t summedResult) {
	*(telegram + 7) |= channel & 0x3F;
	*(telegram + 8) = numberOfConversions;
	*(telegram + 9) = summedResult >> 8;
	*(telegram + 10) = summedResult & 0xFF;	
	return 0;
}

int8_t telegramDecode(telegramHandle handle, uint8_t *telegramRaw, int16_t length) {
	telegramObj *telegram;
	telegram = (telegramObj *)handle;
	
	if (length < 8)
	return -1;
	
	// check checksum
	uint8_t chksum = 0;
	for (uint8_t i = 0; i < (length - 1); i++) {
		chksum ^= telegramRaw[i];
	}
	if (*(telegramRaw + length - 1) != (~chksum & 0xff))
		return -1;
	
	uint8_t controleByte = telegramRaw[0];
	uint8_t telegramMask = ((controleByte & 0xF0) == 0x90 || (controleByte & 0xF0) == 0xB0 || (controleByte & 0xF0) == 0x10 || (controleByte & 0xF0) == 0x30 || controleByte == 0xF0);
	if (!telegramMask)
		return -1;
	telegram->repeat = (controleByte & 0x20) ? KNX_FIRST : KNX_REPEAT;
	telegram->priority = (priority_e)((controleByte & 0x0C) >> 2);
	telegram->sourceAddr = (uint16_t)(telegramRaw[1]) << 8 | telegramRaw[2];
	telegram->destAddr = (uint16_t)(telegramRaw[3]) << 8 | telegramRaw[4];
	uint8_t npci = telegramRaw[5];
	telegram->daf = (address_type_e)((npci & 0x80) >> 7);
	telegram->routingCnt = (npci & 0x70) >> 4;
	telegram->length = npci & 0x0F;
	uint16_t t_pdu = (uint16_t)(telegramRaw[6]) << 8 | telegramRaw[7];
	telegram->tpci = (tpci_e)(t_pdu >> 14);
	telegram->packetCnt = (uint8_t)((t_pdu >> 10) & 0x0F);
	telegram->apci = (telegram->length == 0) ? 0 : (apci_e)((t_pdu >> 6) & 0x0F);
	telegram->p2p = (telegram->length == 0) ? (p2p_e)((t_pdu >> 8) & 0x03) : 0;
	telegram->escapeCode = (telegram->length == 0) ? 0 : (escape_apdu_e)(t_pdu & 0x3F);
	telegram->userCode = (telegram->length == 0) ? 0 : (user_apdu_e)(t_pdu & 0x3F);
	telegram->checksum = telegramRaw[7 + telegram->length];
	uint8_t idx = 8;
	if (telegram->apci == KNX_MASK_VERSION_READ) {
		telegram->maskVersionIdx = telegramRaw[7] & 0x3f;
	} else if (telegram->apci == KNX_MEMORY_READ || telegram->apci == KNX_MEMORY_WRITE || telegram->apci == KNX_MEMORY_RESPONSE) {
		telegram->memoryAcces.byteCnt = (uint8_t)(t_pdu & 0x0F);
		telegram->memoryAcces.startMemAddr = (uint16_t)(telegramRaw[idx++]) << 8;
		telegram->memoryAcces.startMemAddr |= telegramRaw[idx++];
		if (telegram->apci == KNX_MEMORY_WRITE || telegram->apci == KNX_MEMORY_RESPONSE) {
			memcpy(telegram->memoryAcces.values, telegramRaw + 10, telegram->memoryAcces.byteCnt);
			idx += telegram->memoryAcces.byteCnt;
		}
	} else if (telegram->apci == KNX_ADC_READ || telegram->apci == KNX_ADC_RESPONSE) {
		telegram->adcRead.channel = (uint8_t)(t_pdu & 0x3F);
		telegram->adcRead.numberOfConversions = (uint8_t)(telegramRaw[idx++]);
		if (telegram->apci == KNX_ADC_RESPONSE)
		{
			telegram->adcRead.summedResult = (uint16_t)(telegramRaw[idx++]) << 8;
			telegram->adcRead.summedResult |= telegramRaw[idx++];
		}
	} else if (telegram->apci == KNX_ESCAPE) {
		if (telegram->escapeCode == KNX_ESCAPE_PROPERTY_REQ || telegram->escapeCode == KNX_ESCAPE_PROPERTY_WRITE || telegram->escapeCode == KNX_ESCAPE_PROPERTY_RESPONSE) {
			telegram->propertyAcces.objectIndex = telegramRaw[idx++];
			telegram->propertyAcces.propertyId = telegramRaw[idx++];
			telegram->propertyAcces.count = telegramRaw[idx] >> 4;
			telegram->propertyAcces.startIndex = (uint16_t)(telegramRaw[idx++] & 0x0F) << 8;
			telegram->propertyAcces.startIndex |= telegramRaw[idx++];
			if (telegram->escapeCode == KNX_ESCAPE_PROPERTY_WRITE || telegram->escapeCode == KNX_ESCAPE_PROPERTY_RESPONSE) {
				telegram->propertyAcces.byteCnt = telegram->length - 5;
				if (telegram->propertyAcces.byteCnt > 0 && telegram->propertyAcces.byteCnt < 11) {
					memcpy(telegram->propertyAcces.values, telegramRaw + 12, telegram->propertyAcces.byteCnt);
					idx += telegram->propertyAcces.byteCnt;
					} else {
					memset(telegram->propertyAcces.values, 0, 10);
					telegram->propertyAcces.byteCnt = 0;
				}
			}
		}
		else if (telegram->escapeCode == KNX_ESCAPE_PROPERTY_DESCR_REQ || telegram->escapeCode == KNX_ESCAPE_PROPERTY_DESCR_RESPONSE) {
			telegram->propertyAcces.objectIndex = telegramRaw[idx++];
			telegram->propertyAcces.propertyId = telegramRaw[idx++];
			telegram->propertyAcces.propertyIndex = telegramRaw[idx++];
			if (telegram->escapeCode == KNX_ESCAPE_PROPERTY_DESCR_RESPONSE) {
				telegram->propertyAcces.propertyDataType = telegramRaw[idx++] & 0x3f;
				telegram->propertyAcces.maxElements = (uint16_t)(telegramRaw[idx++]) << 8;
				telegram->propertyAcces.maxElements |= telegramRaw[idx++];
				telegram->propertyAcces.readAccessLvl = (telegramRaw[idx] & 0xf0) >> 4 ;
				telegram->propertyAcces.writeAccessLvl = telegramRaw[idx++] & 0x0f;
			}
		}
	}
	telegram->payloadLength = 0;
	if (telegram->length == 1) {
		telegram->payload[0] = telegramRaw[7] & 0x3F;
		telegram->payloadLength = 1;
	}
	else
	{
		idx -= 8;
		if (telegram->length > 1 && (telegram->length - idx) > 0) {
			memcpy(telegram->payload, telegramRaw + 8, (telegram->length - idx) - 1);
			telegram->payloadLength = (telegram->length - idx) - 1;
		}
	}
	if (telegram->daf == KNX_GROUP_ADDRESS) {
		if (telegram->destAddr == 0)
			telegram->messageType = MESSAGE_BROADCAST;
		else
			telegram->messageType = MESSAGE_MULTICAST;
	}
	else
		telegram->messageType = MESSAGE_UNICAST;

	return 0;
}

message_type_e tgGetMessageType(telegramHandle handle) {
	telegramObj *telegram;
	telegram = (telegramObj *)handle;
	return telegram->messageType;
}

int8_t tgGetGroupAddr(telegramHandle handle, uint16_t *groupAddress) {
	telegramObj *telegram;
	telegram = (telegramObj *)handle;
	if (telegram->daf != KNX_GROUP_ADDRESS)
	return -1;
	*groupAddress = telegram->destAddr;
	return 0;
}

int8_t tgGetPhysicalAddr(telegramHandle handle, uint16_t *physicalAddress) {
	telegramObj *telegram;
	telegram = (telegramObj *)handle;
	if (telegram->daf != KNX_PHYSICAL_ADDRESS)
	return -1;
	*physicalAddress = telegram->destAddr;
	return 0;
}

int8_t tgGetValueMaxTwoByte(telegramHandle handle, uint16_t *value) {
	telegramObj *telegram;
	telegram = (telegramObj *)handle;
	if (telegram->payloadLength == 0 || telegram->payloadLength > 2)
	return -1;
	if (telegram->payloadLength == 1)
	{
		*value = telegram->payload[0];
		return 1;
	}
	if (telegram->payloadLength == 2)
	{
		*value = (uint16_t)(telegram->payload[1] << 8) || telegram->payload[0];
		return 2;
	}
	return 0;
}
*/

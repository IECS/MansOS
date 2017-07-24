#include "serialCommunication.h"

void addCrc(SerialPacket *sp) {
	sp->crc = 0;

	sp->crc = crc8Add(sp->crc, sp->len);
	sp->crc = crc8Add(sp->crc, sp->nr);
	sp->crc = crc8Add(sp->crc, sp->cmd);

	for (counter = 0; counter < sp->len; counter++) {
		sp->crc = crc8Add(sp->crc, sp->data[counter]);
	}
}

void sendResponse(uint8_t cmd, uint8_t nr) {
	struct SerialPacket p;

	p.cmd = cmd;
	p.len = 0;
	p.nr = nr;
	addCrc(&p);

	sendResponsePacket(&p);
}

void sendResponseByte(uint8_t cmd, uint8_t nr, uint8_t dataByte) {
	struct SerialPacket p;

	p.cmd = cmd;
	p.len = 1;
	p.nr = nr;
	p.data = &dataByte;
	addCrc(&p);

	sendResponsePacket(&p);
}

void sendResponseData(uint8_t cmd, uint8_t nr, uint8_t len, uint8_t* data) {
	struct SerialPacket p;

	p.cmd = cmd;
	p.len = len;
	p.nr = nr;
	p.data = data;
	addCrc(&p);

	sendResponsePacket(&p);
}

void sendResponsePacket(SerialPacket *sp) {
	addCrc(sp);

	mutexLock(&uartMutex);

	serialSendByte(PRINTF_SERIAL_ID, PACKET_START);
	serialSendByte(PRINTF_SERIAL_ID, sp->len);
	serialSendByte(PRINTF_SERIAL_ID, sp->nr);
	serialSendByte(PRINTF_SERIAL_ID, sp->cmd);

	for (counter = 0; counter < sp->len; counter++)
	{
		serialSendByte(PRINTF_SERIAL_ID, sp->data[counter]);
	}

	serialSendByte(PRINTF_SERIAL_ID, sp->crc);
	serialSendByte(PRINTF_SERIAL_ID, PACKET_END);

	mutexUnlock(&uartMutex);
}

void sendDebugResponse(char *data) {

	struct SerialPacket p;

	p.cmd = DEBUG_MESSAGE_CMD;
	p.nr = 0;

	p.data = (uint8_t *)data;
	p.len = strlen(data);

	addCrc(&p);

	sendResponsePacket(&p);
}

void sendDataDump(uint8_t *data, uint8_t len) {

	SerialPacket p;

	p.cmd = DATA_DUMP_CMD;
	p.nr = 0;

	p.data = data;
	p.len = len;

	addCrc(&p);

	sendResponsePacket(&p);
}

// After the callback returns buffer is reset and reception restarts.
void serialByteReceive(uint8_t byte) {
	RINGFIFO_WR(byte);
	
	if (byte == PACKET_END)
	{
		parseSerialFifo();
	}
}

/**
Packet contents:
Delimiter		- 1 byte('$' aka 0x24)
Size			- 1 byte
Packet number	- 1 byte
Command			- 1 byte
Data			- Size byte
CRC				- 1 byte
Delimiter		- 1 byte('\n' aka 0xB)
**/
void parseSerialFifo(void) {
	// Search the beginning of packet
	while (!RINGFIFO_EMPTY() && RINGFIFO_PEEK(0) != PACKET_START)
	{
		RINGFIFO_SKIP();
	}
		
	if (RINGFIFO_COUNT() < MIN_PACKET_LEN)
	{
		return;
	}

	parseHeaderFromFifo(rp);

	// All packet not in fifo
	if (RINGFIFO_COUNT() < rp->len + MIN_PACKET_LEN)
	{
		return;
	}

	// Packet end is OK?
	if (RINGFIFO_PEEK(rp->len + MIN_PACKET_LEN - 1) == PACKET_END)
	{
		if (parseDataFromFifo(rp))
		{
			packetRecvHandle(rp);
		}
		else
		{
			sendResponse(WRONG_CRC_CMD, rp->nr);
		}
	}
	else
	{
		sendResponse(NO_END_SYMBOL_CMD, rp->nr);
	}

	RINGFIFO_DROP(rp->len + MIN_PACKET_LEN - 1);

	emptySerialPacket(rp);
}

void parseSerialFifoBlocking(void)
{
	for (;;)
	{
		// Search the beginning of packet
		while (RINGFIFO_EMPTY());

		if (RINGFIFO_PEEK(0) != PACKET_START)
		{
			RINGFIFO_SKIP();
			continue;
		}

		while (RINGFIFO_COUNT() < MIN_PACKET_LEN);

		rp->len = RINGFIFO_PEEK(1);
		rp->nr = RINGFIFO_PEEK(2);
		rp->cmd = RINGFIFO_PEEK(3);

		if (rp->len > 100)
		{
			RINGFIFO_SKIP();
			continue;
		}

		// All packet not in fifo
		while (RINGFIFO_COUNT() < rp->len + MIN_PACKET_LEN);

		// Packet end is OK?
		if (RINGFIFO_PEEK(rp->len + MIN_PACKET_LEN - 1) == PACKET_END)
		{
			if (rp->len != 0)
			{
				// Need to respect boundaries of ring fifo
				if (gUartFifo.rdIdx + rp->len > RINGFIFO_SIZE)
				{
					// Take all until end
					memcpy(rp->data, (const void *)&RINGFIFO_PEEK(4), RINGFIFO_SIZE - gUartFifo.rdIdx);
					// Take rest from beginning
					memcpy(&rp->data[RINGFIFO_SIZE - gUartFifo.rdIdx], (const void *)&gUartFifo.data[0], rp->len - (RINGFIFO_SIZE - gUartFifo.rdIdx));
				}
				else
				{
					memcpy(rp->data, (const void *)&RINGFIFO_PEEK(4), rp->len);
				}
			}

			addCrc(rp);

			if (rp->crc == RINGFIFO_PEEK(rp->len + MIN_PACKET_LEN - 2))
			{
				packetRecvHandle(rp);
			}
			else
			{
				sendResponse(WRONG_CRC_CMD, rp->nr);
			}
		}
		else
		{
			sendResponse(NO_END_SYMBOL_CMD, rp->nr);
		}

		RINGFIFO_DROP(rp->len + MIN_PACKET_LEN - 1);
	}
}

void parseHeaderFromFifo(SerialPacket *rp)
{
	rp->len = RINGFIFO_PEEK(1);
	rp->nr = RINGFIFO_PEEK(2);
	rp->cmd = RINGFIFO_PEEK(3);
}

bool parseDataFromFifo(SerialPacket *rp)
{
	if (rp->len != 0)
	{
		memcpy(rp->data, (const void *)&RINGFIFO_PEEK(4), rp->len);
	}

	addCrc(rp);

	return rp->crc == RINGFIFO_PEEK(rp->len + MIN_PACKET_LEN - 2);
}

void emptySerialPacket(SerialPacket *p)
{
	p->cmd = 0;
	p->crc = 0;
	p->len = 0;
	p->nr = 0;
}

void setPacketRecvHandle(packetRecvHandleType packetRecvCB)
{
	packetRecvHandle = packetRecvCB;
}
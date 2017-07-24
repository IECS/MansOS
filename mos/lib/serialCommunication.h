#include "stdmansos.h"
#include "codec.h"
#include "mutex.h"

#ifndef SERIAL_COMMUNICATION_H
#define SERIAL_COMMUNICATION_H

#define PACKET_START 0x24
#define PACKET_END 0x0A
#define MIN_PACKET_LEN 6

#define WRONG_CRC_CMD 0xA
#define NO_END_SYMBOL_CMD 0xB
#define DROPPING_SYMBOL_CMD 0xC
#define RECIEVED_SYMBOL_CMD 0xD
#define DEBUG_MESSAGE_CMD 0xE
#define DATA_DUMP_CMD 0xF
#define UNKNOWN_PACKET_CMD 0x10
#define TEST_PACKET_CMD 0x11
#define BANDWIDTH_TEST_CMD 0x12
#define NO_DATA_CMD 0x13

typedef struct SerialPacket
{
	uint8_t len;
	uint8_t nr;
	uint8_t cmd;
	uint8_t *data;
	uint8_t crc;
} SerialPacket;

Mutex_t uartMutex;

/*** RING FIFO ***/
#define RINGFIFO_SIZE (256)               /* serial buffer in bytes (power 2)   */
#define RINGFIFO_MASK (RINGFIFO_SIZE-1ul) /* buffer size mask                   */

/* Buffer read / write macros                                                 */
#define RINGFIFO_RESET()      	{gUartFifo.rdIdx = gUartFifo.wrIdx = 0;}
#define RINGFIFO_WR(dataIn)   	{gUartFifo.data[RINGFIFO_MASK & gUartFifo.wrIdx++] = (dataIn);}
#define RINGFIFO_RD(dataOut)  	{gUartFifo.rdIdx++; dataOut = gUartFifo.data[RINGFIFO_MASK & (gUartFifo.rdIdx - 1)];}
#define RINGFIFO_EMPTY()      	(gUartFifo.rdIdx == gUartFifo.wrIdx)
#define RINGFIFO_FULL()       	((RINGFIFO_MASK & gUartFifo.rdIdx) == (RINGFIFO_MASK & (gUartFifo.wrIdx + 1)))
#define RINGFIFO_COUNT()      	(RINGFIFO_MASK & (gUartFifo.wrIdx - gUartFifo.rdIdx))

#define RINGFIFO_PEEK(offset) 	(gUartFifo.data[RINGFIFO_MASK & (gUartFifo.rdIdx + offset)])
#define RINGFIFO_SKIP()			(gUartFifo.rdIdx++)
#define RINGFIFO_DROP(offset)	(gUartFifo.rdIdx += offset)

/* buffer type                                                                */
typedef struct {
	uint8_t wrIdx;
	uint8_t rdIdx;
	uint8_t data[RINGFIFO_SIZE];
} RingFifo_t;

volatile RingFifo_t gUartFifo;

uint8_t *dummy;
SerialPacket *rp;
SerialPacket *sp;
char *printfData;

uint16_t counter;

typedef void (*packetRecvHandleType)(SerialPacket *);
packetRecvHandleType packetRecvHandle;

void setPacketRecvHandle(packetRecvHandleType packetRecvCB);

void addCrc(SerialPacket *sp);
void sendResponse(uint8_t cmd, uint8_t nr);
void sendResponseByte(uint8_t cmd, uint8_t nr, uint8_t dataByte);
void sendResponseData(uint8_t cmd, uint8_t nr, uint8_t len, uint8_t* data);
void sendResponsePacket(SerialPacket *sp);
void sendDebugResponse(char *data);
void sendDataDump(uint8_t *data, uint8_t len);
void serialByteReceive(uint8_t byte);
void parseSerialFifo(void);
void parseSerialFifoBlocking(void);
void parseHeaderFromFifo(SerialPacket *rp);
bool parseDataFromFifo(SerialPacket *rp);
void emptySerialPacket(SerialPacket *p);
void parseCommand(SerialPacket *rp);

// Anyone knows correct include?
extern int sprintf(const char *str, const char *format, ...);

// To avoid compile warnings
#undef PRINTF
#define PRINTF(format, ...) {\
	sprintf(printfData, format, ##__VA_ARGS__);\
	sendDebugResponse(printfData); \
}

#endif //SERIAL_COMMUNICATION_H
#include "amb8420.h"
#include <lib/dprint.h>
#include <hil/udelay.h>
#include <string.h>
#include <kernel/threads/threads.h>

#include <hil/leds.h>

#if DEBUG
#define RADIO_DEBUG 1
#endif

#if RADIO_DEBUG
#include "dprint.h"
#define RPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define RPRINTF(...) do {} while (0)
#endif

static AMB8420RxHandle rxHandle;

static bool isOn;

static uint8_t rxBuffer[AMB8420_MAX_PACKET_LEN + 1];
static uint8_t rxCursor;

static uint8_t amb8420OperationMode; // TODO: get rid of

static int8_t lastRssi;

static volatile bool queryRssi;

static enum {
    STATE_READ_DELIMITER,
    STATE_READ_COMMAND,
    STATE_READ_LENGTH,
    STATE_READ_DATA,
    STATE_READ_CS,
} recvState = STATE_READ_DELIMITER;

static uint8_t recvCommand;
static uint8_t recvLength;

static void rxPacket(uint8_t checksum)
{
    int i;
    // PRINTF("got command %#02x, len %d\n", recvCommand, recvLength);
    recvState = STATE_READ_DELIMITER;
    // verify checksum
    checksum ^= AMB8420_START_DELIMITER;
    checksum ^= recvCommand;
    checksum ^= recvLength;

    for (i = 0; i < rxCursor; ++i) {
        checksum ^= rxBuffer[i];
    }
    if (checksum) {
        PRINTF("incorrect checksum %#02x!\n", checksum);
        return;
    }

    // if data received, notify the user
    switch (recvCommand) {
    case AMB8420_CMD_DATAEX_IND:
        if (recvLength >= 1) {
            // the last byte of data is RSSI value
            recvLength--;
            lastRssi = rxBuffer[recvLength];
        } else {
            lastRssi = 0;
            return;
        }
        break;
    case (AMB8420_CMD_RSSI_REQ | AMB8420_REPLY_FLAG):
        lastRssi = rxBuffer[0];
        queryRssi = false;
        return;
    default:
        return;
    }

#if USE_THREADS
    // disable Rx interrupts
    IE1 &= ~URXIE0;
    processFlags.bits.radioProcess = true;
#else
    if (rxHandle) {
        rxHandle();
    } else {
        amb8420Discard();
    }
#endif
}

static void usartReceive(uint8_t byte) {
    // USARTSendByte(1, '0' + (byte >> 4) );
    // USARTSendByte(1, '0' + (byte & 0xf) );
    // USARTSendByte(1, '\n');
    // greenLedToggle();

    switch (recvState) {
    case STATE_READ_DELIMITER:
//        PRINT("delim\n");
        if (byte == AMB8420_START_DELIMITER) {
            recvState = STATE_READ_COMMAND;
        }
        break;

    case STATE_READ_COMMAND:
        recvCommand = byte;
//        PRINTF("command = %d\n", command);
        recvState = STATE_READ_LENGTH;
        break;

    case STATE_READ_LENGTH:
        recvLength = byte;
        // PRINTF("length = %d\n", recvLength);
        if (recvLength) {
//            PRINT("len -> data\n");
            recvState = STATE_READ_DATA;
            rxCursor = 0;
            if (recvLength > sizeof(rxBuffer)) {
                recvLength = sizeof(rxBuffer);
            }
        } else {
//            PRINT("len -> cs\n");
            recvState = STATE_READ_CS;
        }
        break;

    case STATE_READ_DATA:
        rxBuffer[rxCursor++] = byte;
//        PRINTF("rxCursor = %d\n", rxCursor);
        if (rxCursor == recvLength || rxCursor == sizeof(rxBuffer)) {
//            PRINT("data -> cs\n");
            recvState = STATE_READ_CS;
        }
        break;

    case STATE_READ_CS:
        rxPacket(byte);
        break;
    }
}

void amb8420InitUsart(void)
{
    USARTInit(AMB8420_UART_ID, AMB8420_SERIAL_BAUDRATE, 0);
    USARTEnableTX(AMB8420_UART_ID);
    USARTSetReceiveHandle(AMB8420_UART_ID, usartReceive);
}

void amb8420Reset(void)
{
    bool ok;
    pinClear(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    mdelay(10); // wait for some time
    pinSet(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    mdelay(2);
    // wait for initialization to complete
    AMB8420_WAIT_FOR_RTS_READY(ok);

    pinSet(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    amb8420InitUsart();
    mdelay(100);

    amb8420OperationMode = AMB8420_TRANSPARENT_MODE;
}

void amb8420Init(void)
{
    bool ok;
    RPRINTF("amb8420Init...\n");

    amb8420InitUsart();

    pinAsOutput(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    pinAsOutput(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);
    pinAsOutput(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);
    pinAsOutput(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);
    pinAsOutput(AMB8420_DATA_REQUEST_PORT, AMB8420_DATA_REQUEST_PIN);
    pinAsInput(AMB8420_RTS_PORT, AMB8420_RTS_PIN);
    pinAsInput(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);

    pinSet(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);
    pinClear(AMB8420_DATA_REQUEST_PORT, AMB8420_DATA_REQUEST_PIN);

    // TODO: go to sleep
    // AMB8420_ENTER_SLEEP_MODE();
    AMB8420_ENTER_ACTIVE_MODE();

    // pinIntRising(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);
    // pinEnableInt(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);

    // put the system in reset
    amb8420Reset();

    // Switch to command mode (generate falling front)
    pinClear(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);
    mdelay(1);
    pinSet(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);

    RPRINTF("..done\n");
}

void amb8420On(void)
{
    RPRINTF("amb842On\n");
    if (!isOn) {
        isOn = true;
        AMB8420_ENTER_ACTIVE_MODE();
        // if the waiting fails, will retry next time when "on" is called
        AMB8420_WAIT_FOR_RTS_READY(isOn);
        // TODO: switch to command mode
    }
}

void amb8420Off(void)
{
    RPRINTF("amb842Off\n");
    if (isOn) {
        isOn = false;
        AMB8420_ENTER_SLEEP_MODE();
        mdelay(1);
    }
}

int amb8420Read(void *buf, uint16_t bufsize)
{
    RPRINTF("amb8420Read\n");
    if (recvLength > bufsize) {
        recvLength = bufsize;
    }
    memcpy(buf, rxBuffer, recvLength);
    int len = recvLength;
    rxCursor = 0;

#if USE_THREADS
    // enable Rx interrupts back
    IE1 |= URXIE0;
#endif

    return len;
}

int amb8420Send(const void *header_, uint16_t headerLen,
                const void *data_, uint16_t dataLen)
{
    RPRINTF("amb8420Send\n");

    int result = -1;

    bool wasOn = isOn;
    if (!wasOn) amb8420On();

    const uint8_t *header = header_;
    const uint8_t *data = data_;
    uint8_t totalLen = headerLen + dataLen;
    bool ok;

    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) goto end;

    uint8_t cs = AMB8420_START_DELIMITER ^ AMB8420_CMD_DATA_REQ;
    cs ^= totalLen;

    // start delimiter
    USARTSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER);
    // command
    USARTSendByte(AMB8420_UART_ID, AMB8420_CMD_DATA_REQ);
    // data length
    USARTSendByte(AMB8420_UART_ID, totalLen);

    // data
    uint16_t i;
    for (i = 0; i < headerLen; ++i) {
        cs ^= header[i];
        USARTSendByte(AMB8420_UART_ID, header[i]);
    }
    for (i = 0; i < dataLen; ++i) {
        cs ^= data[i];
        USARTSendByte(AMB8420_UART_ID, data[i]);
    }
    // checksum
    USARTSendByte(AMB8420_UART_ID, cs);

    if (!wasOn) amb8420Off();

    result = 0;
  end:
    return result;
}

// void amb8420SetCb()
// {
//     //0x02 0x49 0x01 < status > < CS >
//     PRINTF("amb8420 set answer: ");
//     debugHexdump(rxBuffer, rxCursor);
//     rxCursor = 0;
// }

// int amb8420Set(uint8_t len, uint8_t* data, uint8_t position)
// {
//     if (amb8420OperationMode == AMB8420_TRANSPARENT_MODE) {
//         return -1;
//     }
//     bool ok;
//     AMB8420RxHandle old = amb8420SetReceiver(amb8420SetCb);
//     uint8_t crc = 0x02 ^ 0x09, i;
//     // Wait for device to become ready
//     AMB8420_WAIT_FOR_RTS_READY(ok);
//     if (!ok) goto end;
//     USARTSendByte(AMB8420_UART_ID, 0x02);
//     USARTSendByte(AMB8420_UART_ID, 0x09);
//     USARTSendByte(AMB8420_UART_ID, len + 2);
//     USARTSendByte(AMB8420_UART_ID, position);
//     USARTSendByte(AMB8420_UART_ID, len);
//     crc ^= (len + 2) ^ position ^ len;
//     for (i = 0; i < len; i++)
//     {
//         USARTSendByte(AMB8420_UART_ID, data[i]);
//         crc ^= data[i];
//     }
//     USARTSendByte(AMB8420_UART_ID, crc);

//     // Wait for device to become ready
//     AMB8420_WAIT_FOR_RTS_READY(ok);
//   end:
//     amb8420SetReceiver(old);
//     return ok ? 0 : -1;
// }

// void amb8420GetCb(void)
// {
//     PRINTF("amb8420 set answer: ");
//     //0x02 0x4A < number of bytes + 2 > < memory position > < number of bytes >
//     //< parameter > < CS >
//     debugHexdump(rxBuffer, rxCursor);
//     rxCursor = 0;
// }

// int amb8420Get(uint8_t memoryPosition, uint8_t numberOfBytes)
// {
//     if (amb8420OperationMode == AMB8420_TRANSPARENT_MODE) {
//         return -1;
//     }
//     bool ok;
//     AMB8420RxHandle old = amb8420SetReceiver(amb8420GetCb);
//     // Wait for device to become ready
//     AMB8420_WAIT_FOR_RTS_READY(ok);
//     if (!ok) goto end;
//     USARTSendByte(AMB8420_UART_ID, 0x02);
//     USARTSendByte(AMB8420_UART_ID, 0x0A);
//     USARTSendByte(AMB8420_UART_ID, 0x02);
//     USARTSendByte(AMB8420_UART_ID, memoryPosition);
//     USARTSendByte(AMB8420_UART_ID, numberOfBytes);
//     USARTSendByte(AMB8420_UART_ID, 0x02 ^ 0x0A ^ 0x02 ^ memoryPosition ^ numberOfBytes);

//     // Wait for device to become ready
//     AMB8420_WAIT_FOR_RTS_READY(ok);
//   end:
//     amb8420SetReceiver(old);
//     return 0;
// }

void amb8420Discard(void)
{
    rxCursor = 0;
}

AMB8420RxHandle amb8420SetReceiver(AMB8420RxHandle handle)
{
    AMB8420RxHandle old = rxHandle;
    rxHandle = handle;
    return old;
}

void amb8420SetChannel(int channel)
{
}

void amb8420SetTxPower(uint8_t power)
{
}

int8_t amb8420GetLastRSSI(void)
{
    return lastRssi;
}

uint8_t amb8420GetLastLQI(void)
{
    return 0;
}

int amb8420GetRSSI(void)
{
    int result = 0;
    bool ok;

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) goto end;

    queryRssi = true;

    USARTSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER);
    USARTSendByte(AMB8420_UART_ID, AMB8420_CMD_RSSI_REQ);
    USARTSendByte(AMB8420_UART_ID, 0x00);
    USARTSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER ^ AMB8420_CMD_RSSI_REQ);

     // Wait for device to become ready
     AMB8420_WAIT_FOR_RTS_READY(ok);
     if (!ok) goto end;

     ENABLE_INTS(); // nested interupts

     while (queryRssi) {
         // PRINT("wait for rssi...\n");
         mdelay(10);
     }

     result = lastRssi;

  end:
    return result;
}

bool amb8420IsChannelClear(void)
{
    return 0;
}


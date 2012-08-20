#include "amb8420.h"
#include <lib/dprint.h>
#include <hil/udelay.h>
#include <string.h>
#include <kernel/threads/threads.h>

#include <hil/leds.h>

#if DEBUG
#define RADIO_DEBUG 1
#endif

//#define RADIO_DEBUG 1

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

static int8_t lastRssi;

static volatile uint8_t commandInProgress;

static AMB8420AddrMode_t currentAddressMode;

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
    RPRINTF("AMB: command %#02x, len %d\n", recvCommand, recvLength);
    recvState = STATE_READ_DELIMITER;
    // verify checksum
    checksum ^= AMB8420_START_DELIMITER;
    checksum ^= recvCommand;
    checksum ^= recvLength;

    for (i = 0; i < rxCursor; ++i) {
        checksum ^= rxBuffer[i];
    }
    if (checksum) {
        RPRINTF("incorrect checksum %#02x!\n", checksum);
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
        if (commandInProgress == AMB8420_CMD_RSSI_REQ) {
            lastRssi = rxBuffer[0];
            commandInProgress = 0;
        }
        return;
    case (AMB8420_CMD_SET_REQ |  AMB8420_REPLY_FLAG):
    case (AMB8420_CMD_SET_DESTADDR_REQ | AMB8420_REPLY_FLAG):
    case (AMB8420_CMD_SET_DESTNETID_REQ | AMB8420_REPLY_FLAG):
    case (AMB8420_CMD_SET_CHANNEL_REQ | AMB8420_REPLY_FLAG):
        if (commandInProgress == (recvCommand & ~AMB8420_REPLY_FLAG)) {
            commandInProgress = 0;
        }
        return;
    default:
        return;
    }

#if USE_THREADS
    // disable Rx interrupts
    USARTDisableRX(AMB8420_UART_ID);
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

    bool wasOn = isOn;
    if (!wasOn) amb8420On();

    pinClear(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    mdelay(10); // wait for some time
    pinSet(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    mdelay(2);
    // wait for initialization to complete
    AMB8420_WAIT_FOR_RTS_READY(ok);

    pinSet(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    amb8420InitUsart();
    mdelay(100);

    // Switch to command mode (generate falling front)
    pinClear(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);
    mdelay(1);
    pinSet(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);

    // go to sleep
    if (!wasOn) amb8420Off();
}

void amb8420Init(void)
{
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

    // in case interrupts are used (for non-command mode):
    //pinIntRising(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);
    //pinEnableInt(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);

    // put the system in reset
    amb8420Reset();
 
    RPRINTF("..done\n");
}

void amb8420On(void)
{
    if (!isOn) {
        RPRINTF("amb842On\n");
        isOn = true;
        AMB8420_ENTER_ACTIVE_MODE();
        // if the waiting fails, will retry next time when "on" is called
        AMB8420_WAIT_FOR_RTS_READY(isOn);
    }
}

void amb8420Off(void)
{
    if (isOn) {
        RPRINTF("amb842Off\n");
        isOn = false;
        AMB8420_ENTER_SLEEP_MODE();
    }
}

int amb8420Read(void *buf, uint16_t bufsize)
{
    RPRINTF("amb8420Read\n");
    uint8_t *src = rxBuffer;
    int len = recvLength;
    switch (currentAddressMode) {
    case AMB8420_ADDR_MODE_NONE:
        break;
    case AMB8420_ADDR_MODE_ADDR:
        len--; src++;
        break;
    case AMB8420_ADDR_MODE_ADDRNET:
        len -= 2; src += 2;
        break;
    }

    if (len > bufsize) len = bufsize;
    else if (len <= 0) return -1;

    memcpy(buf, src, len);
    rxCursor = 0;

#if USE_THREADS
    // enable Rx interrupts back
    USARTEnableRX(AMB8420_UART_ID);
#endif

    return len;
}

int amb8420Send(const void *header_, uint16_t headerLen,
                const void *data_, uint16_t dataLen)
{
    RPRINTF("amb8420Send, size=%u\n", headerLen + dataLen);

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

static int amb8420Set(uint8_t position, uint8_t len, uint8_t *data)
{
    bool ok;
    uint8_t crc, i;
    Handle_t handle;

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) goto end;

    commandInProgress = AMB8420_CMD_SET_REQ;

    USARTSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER);
    USARTSendByte(AMB8420_UART_ID, AMB8420_CMD_SET_REQ);
    USARTSendByte(AMB8420_UART_ID, len + 2);
    USARTSendByte(AMB8420_UART_ID, position);
    USARTSendByte(AMB8420_UART_ID, len);
    crc = AMB8420_START_DELIMITER ^ AMB8420_CMD_SET_REQ
            ^ (len + 2) ^ position ^ len;
    for (i = 0; i < len; i++) {
        USARTSendByte(AMB8420_UART_ID, data[i]);
        crc ^= data[i];
    }
    USARTSendByte(AMB8420_UART_ID, crc);

    // wait for reply
    INTERRUPT_ENABLED_START(handle);
    while (commandInProgress);
    INTERRUPT_ENABLED_END(handle);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
  end:
    return ok ? 0 : -1;
}

static inline int amb8420Set1b(uint8_t position, uint8_t variable)
{
    return amb8420Set(position, 1, &variable);
}

void amb8420SetChannel(int channel)
{
    bool ok;
    Handle_t handle;

    channel &= 0xff; // one byte channels only
    RPRINTF("set channel to %d\n", channel);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) goto end;

    commandInProgress = AMB8420_CMD_SET_CHANNEL_REQ;

    USARTSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER);
    USARTSendByte(AMB8420_UART_ID, AMB8420_CMD_SET_CHANNEL_REQ);
    USARTSendByte(AMB8420_UART_ID, 0x1);
    USARTSendByte(AMB8420_UART_ID, channel);
    USARTSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER
            ^ AMB8420_CMD_SET_CHANNEL_REQ ^ 0x1 ^ channel);

    // wait for reply
    INTERRUPT_ENABLED_START(handle);
    while (commandInProgress);
    INTERRUPT_ENABLED_END(handle);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
  end:
    return;
}

void amb8420SetTxPower(uint8_t power)
{
    RPRINTF("set tx power to %d\n", power);
    amb8420Set1b(PHY_DEFAULT_CHANNEL_POS, power);
    amb8420Reset();
}

int8_t amb8420GetLastRSSI(void)
{
    return lastRssi;
}

uint8_t amb8420GetLastLQI(void)
{
    // not implemented in hardware
    return 0;
}

int amb8420GetRSSI(void)
{
    int result = 0;
    bool ok;
    Handle_t handle;

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) goto end;

    commandInProgress = AMB8420_CMD_RSSI_REQ;

    USARTSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER);
    USARTSendByte(AMB8420_UART_ID, AMB8420_CMD_RSSI_REQ);
    USARTSendByte(AMB8420_UART_ID, 0x00);
    USARTSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER ^ AMB8420_CMD_RSSI_REQ);

    // if this code is executed with interrupts disabled,
    // the rx callback will never be called. so make sure
    // interrupts are on.
    INTERRUPT_ENABLED_START(handle);

    // wait for reply
    while (commandInProgress);
    result = lastRssi;

    // disable interrups, if required
    INTERRUPT_ENABLED_END(handle);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
  end:
    return result;
}

bool amb8420IsChannelClear(void)
{
    return true;
}

int amb8420EnterAddressingMode(AMB8420AddrMode_t mode, uint8_t srcAddress)
{
    RPRINTF("set src addr to %#02x\n", srcAddress);

    amb8420Set1b(MAC_ADDR_MODE_POS, mode);
    switch (mode) {
    case AMB8420_ADDR_MODE_NONE:
        amb8420Set1b(MAC_NUM_RETRYS_POS, 0);
        break;
    case AMB8420_ADDR_MODE_ADDR:
        amb8420Set1b(MAC_NUM_RETRYS_POS, 2);
        amb8420Set1b(MAC_SRC_ADDR_POS, srcAddress);
        break;
    case AMB8420_ADDR_MODE_ADDRNET:
        break; // not yet
    }

    amb8420Set1b(MAC_DST_ADDR_POS, 0xff); // XXX

    amb8420Reset();
    currentAddressMode = mode;
    return 0;
}

int amb8420SetDstAddress(uint8_t dstAddress)
{
    bool ok;
    Handle_t handle;

    RPRINTF("set dst addr to %#02x\n", dstAddress);

    // better be safe than sorry - treat 0 as broadcast too
    if (dstAddress == 0) dstAddress = 0xff;

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) goto end;

    commandInProgress = AMB8420_CMD_SET_DESTADDR_REQ;

    USARTSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER);
    USARTSendByte(AMB8420_UART_ID, AMB8420_CMD_SET_DESTADDR_REQ);
    USARTSendByte(AMB8420_UART_ID, 0x1);
    USARTSendByte(AMB8420_UART_ID, dstAddress);
    USARTSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER
            ^ AMB8420_CMD_SET_DESTADDR_REQ ^ 0x1 ^ dstAddress);

    // wait for reply
    INTERRUPT_ENABLED_START(handle);
    while (commandInProgress);
    INTERRUPT_ENABLED_END(handle);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
  end:
    return ok ? 0 : -1;
}

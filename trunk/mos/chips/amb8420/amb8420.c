#include "amb8420.h"
#include <print.h>
#include <delay.h>
#include <string.h>
#include <kernel/threads/threads.h>

#include <leds.h>

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

#ifndef RADIO_BUFFER_SIZE
#define RADIO_BUFFER_SIZE AMB8420_MAX_PACKET_LEN
#endif

static uint8_t rxBuffer[RADIO_BUFFER_SIZE + 1];
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

//
// Enable/disable flash access to the USART
// Busy waiting loop us not used, as the code is executed with ints off.
//
#define AMB8420_SERIAL_CAPTURE()    \
    serial[AMB8420_UART_ID].busy = true;                             \
    if (serial[AMB8420_UART_ID].function != SERIAL_FUNCTION_RADIO) { \
        amb8420InitSerial();                                         \
    }                                                                \

#define AMB8420_SERIAL_FREE()   \
    serial[AMB8420_UART_ID].busy = false;

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
    case (AMB8420_CMD_GET_REQ |  AMB8420_REPLY_FLAG):
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
    serialDisableRX(AMB8420_UART_ID); // TODO: is this ok?
    processFlags.bits.radioProcess = true;
    // wake up the kernel thread
    // XXX: done in ISR because it cannot be done here!
    // EXIT_SLEEP_MODE();
#else
    if (rxHandle) {
        rxHandle();
    } else {
        amb8420Discard();
    }
#endif
}

static void serialReceive(uint8_t byte) {
    // serialSendByte(AMB8420_UART_ID ^ 1, '0' + (byte >> 4) );
    // serialSendByte(AMB8420_UART_ID ^ 1, '0' + (byte & 0xf) );
    // serialSendByte(AMB8420_UART_ID ^ 1, '\n');
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

void amb8420InitSerial(void)
{
    serialInit(AMB8420_UART_ID, AMB8420_SERIAL_BAUDRATE, 0);
    serialEnableTX(AMB8420_UART_ID);
    serialSetReceiveHandle(AMB8420_UART_ID, serialReceive);
    serial[AMB8420_UART_ID].function = SERIAL_FUNCTION_RADIO;
}

void amb8420Reset(void)
{
    bool ok;

    RPRINTF("amb8420Reset\n");

    AMB8420ModeContext_t ctx;
    // AMB8420_ENTER_STANDBY_MODE(ctx);
    AMB8420_ENTER_ACTIVE_MODE(ctx);

    pinClear(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    mdelay(10); // wait for some time
    pinSet(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    mdelay(2);
    // wait for initialization to complete
    AMB8420_WAIT_FOR_RTS_READY(ok);

    pinSet(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    amb8420InitSerial();
    mdelay(100);

    // Switch to command mode (generate falling front)
    pinClear(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);
    mdelay(1);
    pinSet(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);

    // go to sleep
    AMB8420_RESTORE_MODE(ctx);

    // long delay, maybe helps for the RTS problem
    mdelay(500);
}

void amb8420Init(void)
{
    RPRINTF("amb8420Init...\n");

    amb8420InitSerial();

    pinAsOutput(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    pinAsOutput(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);
    pinAsOutput(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);
    pinAsOutput(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);
    // pinAsOutput(AMB8420_DATA_REQUEST_PORT, AMB8420_DATA_REQUEST_PIN);
    pinAsInput(AMB8420_RTS_PORT, AMB8420_RTS_PIN);
    // pinAsInput(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);

    pinSet(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);
    // pinClear(AMB8420_DATA_REQUEST_PORT, AMB8420_DATA_REQUEST_PIN);

    // in case interrupts are used (for non-command mode):
    //pinIntRising(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);
    //pinEnableInt(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);

    // make sure low power mode is enabled
    pinSet(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);
    pinSet(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);

    // put the system in reset
    amb8420Reset();
 
    RPRINTF("..done\n");
}

void amb8420On(void)
{
    if (!isOn) {
        RPRINTF("amb842On\n");
        isOn = true;
        pinClear(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);
        pinClear(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);
        // if the waiting fails, will retry next time when "on" is called
        AMB8420_WAIT_FOR_RTS_READY(isOn);
    }
}

void amb8420Off(void)
{
    if (isOn) {
        RPRINTF("amb842Off\n");
        isOn = false;
        pinSet(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);
        pinSet(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);
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
    serialEnableRX(AMB8420_UART_ID);
#endif

    return len;
}

int amb8420Send(const void *header_, uint16_t headerLen,
                const void *data_, uint16_t dataLen)
{
    RPRINTF("amb8420Send, size=%u\n", headerLen + dataLen);

    int result = -1;

    AMB8420ModeContext_t ctx;
    AMB8420_ENTER_ACTIVE_MODE(ctx);

    const uint8_t *header = header_;
    const uint8_t *data = data_;
    uint8_t totalLen = headerLen + dataLen;
    bool ok;
//    Handle_t h;

    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) {
        amb8420Reset(); // XXX: reset the radio in hope it will help
        greenLedOn();
        goto end;
    }

    uint8_t cs = AMB8420_START_DELIMITER ^ AMB8420_CMD_DATA_REQ;
    cs ^= totalLen;

//    PRINTF("send, tar=%u\n", TAR);
//    ATOMIC_START_TIMESAVE(h);
    AMB8420_SERIAL_CAPTURE();

    // start delimiter
    serialSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER);
    // command
    serialSendByte(AMB8420_UART_ID, AMB8420_CMD_DATA_REQ);
    // data length
    serialSendByte(AMB8420_UART_ID, totalLen);

    // data
    uint16_t i;
    for (i = 0; i < headerLen; ++i) {
        cs ^= header[i];
        serialSendByte(AMB8420_UART_ID, header[i]);
    }
    for (i = 0; i < dataLen; ++i) {
        cs ^= data[i];
        serialSendByte(AMB8420_UART_ID, data[i]);
    }
    // checksum
    serialSendByte(AMB8420_UART_ID, cs);

    AMB8420_SERIAL_FREE();
//    ATOMIC_END_TIMESAVE(h);
    // PRINTF("end send, tar=%u\n", TAR);

    AMB8420_RESTORE_MODE(ctx);

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

    // ATOMIC_START(handle);
    AMB8420_SERIAL_CAPTURE();

    commandInProgress = AMB8420_CMD_SET_REQ;

    serialSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER);
    serialSendByte(AMB8420_UART_ID, AMB8420_CMD_SET_REQ);
    serialSendByte(AMB8420_UART_ID, len + 2);
    serialSendByte(AMB8420_UART_ID, position);
    serialSendByte(AMB8420_UART_ID, len);
    crc = AMB8420_START_DELIMITER ^ AMB8420_CMD_SET_REQ
            ^ (len + 2) ^ position ^ len;
    for (i = 0; i < len; i++) {
        serialSendByte(AMB8420_UART_ID, data[i]);
        crc ^= data[i];
    }
    serialSendByte(AMB8420_UART_ID, crc);

    AMB8420_SERIAL_FREE();
    // ATOMIC_END(handle);

    // wait for reply.
    // if someone grabs access to serial during this waiting,
    // then it will fail, but that's ok, as the caller will retry.
    INTERRUPT_ENABLED_START(handle);
#ifndef PLATFORM_ARDUINO
    BUSYWAIT_UNTIL(!commandInProgress, TIMER_100_MS, ok);
#else
    mdelay(20);
#endif
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

// for debugging
//static 
int amb8420GetAll(void)
{
    bool ok;
    uint8_t crc;
    Handle_t handle;

    const uint8_t position = 0;
    const uint8_t len = 0x80;

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) goto end;

    // ATOMIC_START(handle);
    AMB8420_SERIAL_CAPTURE();

    commandInProgress = AMB8420_CMD_GET_REQ;

    serialSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER);
    serialSendByte(AMB8420_UART_ID, AMB8420_CMD_GET_REQ);
    serialSendByte(AMB8420_UART_ID, 2);
    serialSendByte(AMB8420_UART_ID, position);
    serialSendByte(AMB8420_UART_ID, len);
    crc = AMB8420_START_DELIMITER ^ AMB8420_CMD_GET_REQ
            ^ 2 ^ position ^ len;
    serialSendByte(AMB8420_UART_ID, crc);

    AMB8420_SERIAL_FREE();
    // ATOMIC_END(handle);

    // wait for reply
    // if someone grabs access to serial during this waiting,
    // then it will fail, but that's ok, as the caller will retry.
    INTERRUPT_ENABLED_START(handle);
#ifndef PLATFORM_ARDUINO
    BUSYWAIT_UNTIL(!commandInProgress, TIMER_100_MS, ok);
#else
    mdelay(20);
#endif
    INTERRUPT_ENABLED_END(handle);

#if RADIO_DEBUG
    PRINTF("recvLength=%u\n", recvLength);
    debugHexdump(rxBuffer, recvLength);
#endif

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
  end:
    return ok ? 0 : -1;
}

void amb8420SetChannel(int channel)
{
    bool ok;
    Handle_t handle;

    AMB8420ModeContext_t ctx;
    AMB8420_ENTER_STANDBY_MODE(ctx);

    channel &= 0xff; // one byte channels only
    RPRINTF("set channel to %d\n", channel);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) goto end;

    // ATOMIC_START(handle);
    AMB8420_SERIAL_CAPTURE();

    commandInProgress = AMB8420_CMD_SET_CHANNEL_REQ;

    serialSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER);
    serialSendByte(AMB8420_UART_ID, AMB8420_CMD_SET_CHANNEL_REQ);
    serialSendByte(AMB8420_UART_ID, 0x1);
    serialSendByte(AMB8420_UART_ID, channel);
    serialSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER
            ^ AMB8420_CMD_SET_CHANNEL_REQ ^ 0x1 ^ channel);

    AMB8420_SERIAL_FREE();
    // ATOMIC_END(handle);

    // wait for reply
    INTERRUPT_ENABLED_START(handle);
#ifndef PLATFORM_ARDUINO
    BUSYWAIT_UNTIL(!commandInProgress, TIMER_100_MS, ok);
#else
    mdelay(20);
#endif
    INTERRUPT_ENABLED_END(handle);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
  end:
    AMB8420_RESTORE_MODE(ctx);
    return;
}

void amb8420SetTxPower(uint8_t power)
{
    RPRINTF("set tx power to %d\n", power);
    AMB8420ModeContext_t ctx;
    AMB8420_ENTER_STANDBY_MODE(ctx);
    while (amb8420Set1b(PHY_PA_POWER_POS, power));
    amb8420Reset();
    AMB8420_RESTORE_MODE(ctx);
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

    AMB8420ModeContext_t ctx;
    AMB8420_ENTER_STANDBY_MODE(ctx);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) goto end;

    // ATOMIC_START(handle);
    AMB8420_SERIAL_CAPTURE();

    commandInProgress = AMB8420_CMD_RSSI_REQ;

    serialSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER);
    serialSendByte(AMB8420_UART_ID, AMB8420_CMD_RSSI_REQ);
    serialSendByte(AMB8420_UART_ID, 0x00);
    serialSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER ^ AMB8420_CMD_RSSI_REQ);

    AMB8420_SERIAL_FREE();
    // ATOMIC_END(handle);

    // if this code was executed with interrupts disabled,
    // the rx callback will never be called. so make sure
    // interrupts are on.
    INTERRUPT_ENABLED_START(handle);

    // wait for reply
#ifndef PLATFORM_ARDUINO // just. not. possible
    BUSYWAIT_UNTIL_NORET(!commandInProgress, TIMER_SECOND);
#else
    mdelay(100);
#endif
    result = lastRssi;

    // disable interrups, if required
    INTERRUPT_ENABLED_END(handle);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
  end:
    AMB8420_RESTORE_MODE(ctx);
    return result;
}

bool amb8420IsChannelClear(void)
{
    return true;
}

int amb8420EnterAddressingMode(AMB8420AddrMode_t mode, uint8_t srcAddress)
{
    RPRINTF("set src addr to %#02x\n", srcAddress);

    // disable radio rx/tx
    AMB8420ModeContext_t ctx;
    AMB8420_ENTER_STANDBY_MODE(ctx);

    // retry setting until succeed
    while (amb8420Set1b(MAC_ADDR_MODE_POS, mode));

    switch (mode) {
    case AMB8420_ADDR_MODE_NONE:
        while (amb8420Set1b(MAC_NUM_RETRYS_POS, 0));
        break;
    case AMB8420_ADDR_MODE_ADDR:
        while (amb8420Set1b(MAC_NUM_RETRYS_POS, 2));
        while (amb8420Set1b(MAC_SRC_ADDR_POS, srcAddress));
        break;
    case AMB8420_ADDR_MODE_ADDRNET:
        break; // not yet
    }

    while (amb8420Set1b(MAC_DST_ADDR_POS, 0xff)); // XXX

    amb8420Reset();
    currentAddressMode = mode;
    AMB8420_RESTORE_MODE(ctx);
    return 0;
}

bool amb8420SetDstAddress(uint8_t dstAddress)
{
    bool ok;
    // Handle_t handle;

    // PRINTF("set dst addr to %#02x\n", dstAddress);

    // disable radio rx/tx
    AMB8420ModeContext_t ctx;
    AMB8420_ENTER_STANDBY_MODE(ctx);

    // better be safe than sorry - treat 0 as broadcast too
    if (dstAddress == 0) dstAddress = 0xff;

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) goto end;

    // PRINTF("set dst, tar=%u\n", TAR);
    // ATOMIC_START_TIMESAVE(handle);
    AMB8420_SERIAL_CAPTURE();

    commandInProgress = AMB8420_CMD_SET_DESTADDR_REQ;

    serialSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER);
    serialSendByte(AMB8420_UART_ID, AMB8420_CMD_SET_DESTADDR_REQ);
    serialSendByte(AMB8420_UART_ID, 0x1);
    serialSendByte(AMB8420_UART_ID, dstAddress);
    serialSendByte(AMB8420_UART_ID, AMB8420_START_DELIMITER
            ^ AMB8420_CMD_SET_DESTADDR_REQ ^ 0x1 ^ dstAddress);

    AMB8420_SERIAL_FREE();
    // ATOMIC_END_TIMESAVE(handle);
    // PRINTF("end set dst, tar=%u\n", TAR);

    // wait for reply
    // INTERRUPT_ENABLED_START(handle);
    // BUSYWAIT_UNTIL(!commandInProgress, TIMER_100_MS, ok);
    // if (commandInProgress) PRINTF("command wait fail\n");
    // INTERRUPT_ENABLED_END(handle);
    // if (!ok) goto end;

    // Wait for device to become ready
    // AMB8420_WAIT_FOR_RTS_READY(ok);
  end:
    AMB8420_RESTORE_MODE(ctx);
    // PRINTF(" ok = %d\n", (int)ok);
    // if (!ok) amb8420Reset();
    return ok;
}

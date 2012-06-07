#include "amb8420.h"
#include <lib/dprint.h>
#include <hil/udelay.h>
#include <string.h>

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

static uint8_t rxBuffer[AMB8420_MAX_PACKET_LEN];
static uint16_t rxCursor;

static void usartReceive(uint8_t byte) {
    if (rxCursor < AMB8420_MAX_PACKET_LEN) {
        rxBuffer[rxCursor++] = byte;
    }
}

void amb8420Init(void)
{
    RPRINTF("amb8420Init...\n");

    USARTInit(AMB8420_UART_ID, AMB8420_SERIAL_BAUDRATE, 0);
    USARTEnableTX(AMB8420_UART_ID);
    USARTSetReceiveHandle(AMB8420_UART_ID, usartReceive);

    pinAsOutput(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    pinAsOutput(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);
    pinAsOutput(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);
    pinAsOutput(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);
    pinAsOutput(AMB8420_DATA_REQUEST_PORT, AMB8420_DATA_REQUEST_PIN);
    pinAsInput(AMB8420_RTS_PORT, AMB8420_RTS_PIN);
    pinAsInput(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);

    pinSet(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);
    pinClear(AMB8420_DATA_REQUEST_PORT, AMB8420_DATA_REQUEST_PIN);
    // disable transmissions via serial port
    pinSet(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);
    // and enter sleep mode
    pinSet(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);

    // pinIntFalling(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);
    // pinEnableInt(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);

    // put the system in reset
    pinClear(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    mdelay(10); // wait for some time
    pinSet(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    mdelay(2);
    // wait for initialization to complete
    while (pinRead(AMB8420_RTS_PORT, AMB8420_RTS_PIN)) {
        udelay(100);
    }

    RPRINTF("..done\n");
}

void amb8420On(void)
{
    RPRINTF("amb842On\n");
    if (!isOn) {
        pinClear(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);
        pinClear(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);
        mdelay(2); // found empirically
        isOn = true;
    }
}

void amb8420Off(void)
{
    RPRINTF("amb842Off\n");
    if (isOn) {
        pinSet(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);
        pinSet(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);
        isOn = false;
    }
}

int amb8420Read(void *buf, uint16_t bufsize)
{
    RPRINTF("amb8420Read\n");
    if (rxCursor > bufsize) {
        rxCursor = bufsize;
    }
    memcpy(buf, rxBuffer, rxCursor);
    int len = rxCursor;
    rxCursor = 0;
    return len;
}

int amb8420Send(const void *header_, uint16_t headerLen,
                const void *data_, uint16_t dataLen)
{
    RPRINTF("amb8420Send\n");

    bool wasOn = isOn;
    if (!wasOn) amb8420On();

    const uint8_t *header = header_;
    const uint8_t *data = data_;

    uint16_t i;
    for (i = 0; i < headerLen; ++i) {
        USARTSendByte(AMB8420_UART_ID, header[i]);
    }
    for (i = 0; i < dataLen; ++i) {
        USARTSendByte(AMB8420_UART_ID, data[i]);
    }

    // generate send signal
    pinSet(AMB8420_DATA_REQUEST_PORT, AMB8420_DATA_REQUEST_PIN);
    udelay(10);
    pinClear(AMB8420_DATA_REQUEST_PORT, AMB8420_DATA_REQUEST_PIN);

    if (!wasOn) amb8420Off();

    return 0;
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

void amb8420SetChannel(int channel)
{
}

void amb8420SetTxPower(uint8_t power)
{
}

int8_t amb8420GetLastRSSI(void)
{
    return 0;
}

uint8_t amb8420GetLastLQI(void)
{
    return 0;
}

int amb8420GetRSSI(void)
{
    return 0;
}

bool amb8420IsChannelClear(void)
{
    return 0;
}

// XISR(AMB8420_DATA_INDICATE_PORT, amb8420Interrupt)
// {
//     if (!pinReadIntFlag(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN)) {
//         RPRINTF("got some other port1 interrupt!\n");
//         return;
//     }
//     RPRINTF("****************** got radio interrupt!\n");
//     pinClearIntFlag(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);

// #if USE_THREADS
//     processFlags.bits.radioProcess = true;
// #else
//     if (rxHandle) {
//         rxHandle();
//     } else {
//         amb8420Discard();
//     }
// #endif
// }

void amb8420PollForPacket(void)
{
    static bool wasLow;

    bool isLow = !pinRead(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);
    if (isLow == wasLow) return;
    wasLow = isLow;

    // falling edge indicates radio packet rx
    if (isLow) {
        return;
    }
    // rising edge indicates no more data on serial
#if USE_THREADS
    processFlags.bits.radioProcess = true;
#else
    if (rxHandle) {
        rxHandle();
    } else {
        amb8420Discard();
    }
#endif
}

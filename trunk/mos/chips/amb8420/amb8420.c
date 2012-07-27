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
    // USARTSendByte(1, '0' + (byte >> 4) );
    // USARTSendByte(1, '0' + (byte & 0xf) );
    // USARTSendByte(1, '\n');
    if (rxCursor < AMB8420_MAX_PACKET_LEN) {
        rxBuffer[rxCursor++] = byte;
    }
}

void amb8420InitUsart(void)
{
    USARTInit(AMB8420_UART_ID, AMB8420_SERIAL_BAUDRATE, 0);
    USARTEnableTX(AMB8420_UART_ID);
    USARTSetReceiveHandle(AMB8420_UART_ID, usartReceive);
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

    AMB8420_ENTER_SLEEP_MODE();

    pinIntRising(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);
    pinEnableInt(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);

    // put the system in reset
    pinClear(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    mdelay(10); // wait for some time
    pinSet(AMB8420_RESET_PORT, AMB8420_RESET_PIN);
    mdelay(2);
    // wait for initialization to complete
    AMB8420_WAIT_FOR_RTS_READY(ok);

    amb8420OperationMode = AMB8420_TRANSPARENT_MODE;
    RPRINTF("..done\n");
}

void amb8420On(void)
{
    RPRINTF("amb842On\n");

    amb8420InitUsart(); // XXX

    if (!isOn) {
        isOn = true;
        AMB8420_ENTER_ACTIVE_MODE();
        // if the waiting fails, will retry next time when "on" is called
        AMB8420_WAIT_FOR_RTS_READY(isOn);
    }
}

void amb8420Off(void)
{
    RPRINTF("amb842Off\n");
    if (isOn) {
        isOn = false;
        AMB8420_ENTER_SLEEP_MODE();
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

uint8_t amb8420GetXor(uint8_t len, uint8_t* data)
{
    uint8_t crc = 0;
    while(len != 255) // -1 == 255
    {
        crc ^= data[len--];
    }
    return crc;
}

void amb8420ChangeModeCb()
{
    debugHexdump(rxBuffer, rxCursor);
    if (rxBuffer[4] == amb8420GetXor(3, (uint8_t*)rxBuffer))
    {
        // 0x02 0x44 0x01 < newly configured operating mode > < CS >
        amb8420OperationMode = rxBuffer[3];
    }
    rxCursor = 0;
}

void amb8420ChangeMode()
{
    bool ok;
    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) return;

    AMB8420RxHandle old = amb8420SetReceiver(amb8420ChangeModeCb);
    // Generate falling front
    pinClear(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);
    mdelay(1);
    pinSet(AMB8420_CONFIG_PORT, AMB8420_CONFIG_PIN);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
    amb8420SetReceiver(old);
}

void amb8420SetCb()
{
    //0x02 0x49 0x01 < status > < CS >
    PRINTF("amb8420 set answer: ");
    debugHexdump(rxBuffer, rxCursor);
    rxCursor = 0;
}

int amb8420Set(uint8_t len, uint8_t* data, uint8_t position)
{
    if (amb8420OperationMode == AMB8420_TRANSPARENT_MODE) {
        return -1;
    }
    bool ok;
    AMB8420RxHandle old = amb8420SetReceiver(amb8420SetCb);
    uint8_t crc = 0x02 ^ 0x09, i;
    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) goto end;
    USARTSendByte(AMB8420_UART_ID, 0x02);
    USARTSendByte(AMB8420_UART_ID, 0x09);
    USARTSendByte(AMB8420_UART_ID, len +2 );
    USARTSendByte(AMB8420_UART_ID, position);
    USARTSendByte(AMB8420_UART_ID, len);
    crc ^= (len + 2) ^ position ^ len;
    for (i = 0; i < len; i++)
    {
        USARTSendByte(AMB8420_UART_ID, data[i]);
        crc ^= data[i];
    }
    USARTSendByte(AMB8420_UART_ID, crc);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
  end:
    amb8420SetReceiver(old);
    return ok ? 0 : -1;
}

void amb8420GetCb()
{
    PRINTF("amb8420 set answer: ");
    //0x02 0x4A < number of bytes + 2 > < memory position > < number of bytes >
    //< parameter > < CS >
    debugHexdump(rxBuffer, rxCursor);
    rxCursor = 0;
}

int amb8420Get(uint8_t memoryPosition, uint8_t numberOfBytes)
{
    if (amb8420OperationMode == AMB8420_TRANSPARENT_MODE) {
        return -1;
    }
    bool ok;
    AMB8420RxHandle old = amb8420SetReceiver(amb8420GetCb);
    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
    if (!ok) goto end;
    USARTSendByte(AMB8420_UART_ID, 0x02);
    USARTSendByte(AMB8420_UART_ID, 0x0A);
    USARTSendByte(AMB8420_UART_ID, 0x02);
    USARTSendByte(AMB8420_UART_ID, memoryPosition);
    USARTSendByte(AMB8420_UART_ID, numberOfBytes);
    USARTSendByte(AMB8420_UART_ID, 0x02 ^ 0x0A ^ 0x02 ^ memoryPosition ^ numberOfBytes);

    // Wait for device to become ready
    AMB8420_WAIT_FOR_RTS_READY(ok);
  end:
    amb8420SetReceiver(old);
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

XISR(AMB8420_DATA_INDICATE_PORT, amb8420Interrupt)
{
    if (!pinReadIntFlag(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN)) {
        RPRINTF("got some other port1 interrupt!\n");
        return;
    }
    // RPRINTF("****************** got radio interrupt!\n");
    RPRINTF("int!\n");
    pinClearIntFlag(AMB8420_DATA_INDICATE_PORT, AMB8420_DATA_INDICATE_PIN);

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

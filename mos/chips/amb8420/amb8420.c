#include "amb8420.h"

AMB8420RxHandle rxHandle;

void amb8420Init(void)
{
}

void amb8420On(void)
{
}

void amb8420Off(void)
{
}

int amb8420Read(void *buf, uint16_t bufsize)
{
    return 0;
}

int amb8420Send(const void *header, uint16_t headerLen,
                 const void *data, uint16_t dataLen)
{
    return 0;
}

void amb8420Discard(void)
{
}

AMB8420RxHandle amb8420SetReceiver(AMB8420RxHandle handle) {
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


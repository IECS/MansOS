/*
 * Pagaidaam viss, ko shii programma dara, ir sanjmem laiku no routera, noparsee to, un suuta to atpakalj!
 */

#define SERIAL_PORT 0

void setup()
{
   USB.begin();
}

static inline uint16_t crc16Add(uint16_t acc, uint8_t byte) {
    acc ^= byte;
    acc  = (acc >> 8) | (acc << 8);
    acc ^= (acc & 0xff00) << 4;
    acc ^= (acc >> 8) >> 4;
    acc ^= (acc & 0xff00) >> 5;
    return acc;
}

uint16_t crc16(const uint8_t *data, uint16_t len) {
    uint16_t i, crc = 0;
    for (i = 0; i < len; ++i) {
        crc = crc16Add(crc, *data++);
    }
    return crc;
}

#define DELIMITER '$'
char buffer[200];

void onUsartDataRecvd(uint8_t *data) {
    struct {
        unsigned char delimiter1;
        unsigned char delimiter2;
        unsigned short crc;
        unsigned long time;
    } s;

    memcpy(&s, data, sizeof(s));
    if (s.delimiter1 != DELIMITER || s.delimiter2 != 0) {
        USB.print("onUsartDataRecvd: wrong delimiters!\n");
        return;
    }
    uint16_t crc = crc16((uint8_t *) &s.time, sizeof(s.time));
    if (crc != s.crc) {
       sprintf(buffer, "onUsartDataRecvd: wrong crc (%#x vs %#x)\n", crc, s.crc);
       USB.print(buffer);
       return;
    }

    sprintf(buffer, "rx time %lu via serial\n", s.time);
    USB.print(buffer);
}

void usartReceive(uint8_t byte) {
    static uint8_t recvBuffer[8];
    static int bytesAfterDelimiter = -1;
    USB.print("usartReceive ");
    USB.print(byte);
    USB.print('\n');
    if (bytesAfterDelimiter == -1) {
        if (byte == DELIMITER) bytesAfterDelimiter = 0;
        else return;
    }
    recvBuffer[bytesAfterDelimiter++] = byte;
    if (bytesAfterDelimiter > sizeof(recvBuffer)) {
        bytesAfterDelimiter = -1;
        onUsartDataRecvd(recvBuffer);
    }
}

void loop()
{
  Utils.blinkLEDs(1000);
  USB.println("in loop");

  while(serialAvailable(SERIAL_PORT)>0) {
    int rb = serialRead(SERIAL_PORT);
    if (rb == -1) break;
    usartReceive(rb);
  }

  delay(2000);
}

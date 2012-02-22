/**
 * Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of  conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>

#include <map>
#include <iterator>
using namespace std;

#define PROMPT "$ "

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

#define ARRAYLEN(x) (sizeof(x) / sizeof(x[0]))

int serialFD = -1;

bool commandInProgress;
unsigned commandIssuedTime;
bool commandReplyReceived;

uint16_t dstAddress;

#define COMMAND_TIMEOUT 2 // seconds
#define WAIT_TIMEOUT    1 // seconds

#define MAX_CMD_SIZE 32

const char *serialDevice = "/dev/ttyUSB0";
const tcflag_t BAUDRATE = B38400;

static void hexdump(uint8_t *data, unsigned len);

// starting from 0; used to calculate address on external flash
uint16_t imageNumber;
// a random number used to identify a particular connection;
// non zero if currently uploading image
uint16_t imageId;
void sendCodeFileStart(CodeType ct);
void sendCodeFileContinue(void);
bool isCodeFileBeingSent(void) {
    return imageId != 0;
}

// -----------------------------------------------

struct Oid_t
{
    uint8_t oid[MAX_OID_LEN];
    unsigned length;

    Oid_t() : length(0) {
        memset(oid, 0, sizeof(oid));
    }
};

// -----------------------------------------------

typedef void (*PrettyPrintFunction)(uint32_t value);

void printMoteType(uint32_t value);
void printOnOff(uint32_t value);
void printTSR(uint32_t value);
void printPAR(uint32_t value);
void printVoltage(uint32_t value);
void printRelativeHumidity(uint32_t value);
void printTemperature(uint32_t value);

typedef struct {
    uint8_t oid[MAX_OID_LEN];
    uint8_t len;
    const char *name;
    PrettyPrintFunction printFn;
} KnownOid;

static KnownOid knownOids[] = {
    {{SMP_MOS, SMP_RES_TYPE}, 2, "Mote type", printMoteType},
    {{SMP_MOS, SMP_RES_ADDRESS}, 2, "PAN address", NULL},
    {{SMP_MOS, SMP_RES_IEEE_ADDRESS}, 2, "IEEE address", NULL},
    {{SMP_MOS, SMP_RES_LED, SMP_LED_RED}, 3, "Red LED", printOnOff},
    {{SMP_MOS, SMP_RES_LED, SMP_LED_GREEN}, 3, "Green LED", printOnOff},
    {{SMP_MOS, SMP_RES_LED, SMP_LED_BLUE}, 3, "Blue LED", printOnOff},
    {{SMP_MOS, SMP_RES_SENSOR, SMP_SENSOR_TSR}, 3, "Total solar radiation", printTSR},
    {{SMP_MOS, SMP_RES_SENSOR, SMP_SENSOR_PAR}, 3, "Photosynthetically active radiation", printPAR},
    {{SMP_MOS, SMP_RES_SENSOR, SMP_SENSOR_VOLTAGE}, 3, "Voltage", printVoltage},
    {{SMP_MOS, SMP_RES_SENSOR, SMP_SENSOR_HUMIDITY}, 3, "Relative humidity", printRelativeHumidity},
    {{SMP_MOS, SMP_RES_SENSOR, SMP_SENSOR_TEMPERATURE}, 3, "Temperature", printTemperature},
};

KnownOid *matchKnownOid(uint8_t *oid, uint8_t oidLen) {
    for (uint8_t i = 0; i < ARRAYLEN(knownOids); ++i) {
        KnownOid *ko = &knownOids[i];
        if (ko->len == oidLen
                && !memcmp(ko->oid, oid, oidLen)) {
            return ko;
        }
    }
    return NULL;
}

void printMoteType(uint32_t value) {
    (void) value;
    printf("Tmote Sky"); // TODO
}

void printOnOff(uint32_t value) {
    printf(value ? "on" : "off");
}

static inline double current(unsigned vSensor) {
#define VREF 2.5
    return (vSensor / 4096. * VREF) / 100000.;
}

void printTSR(uint32_t value) {
    double dvalue = 0.769 * 1e5 * current(value) * 1000;
    printf("%0.2f lx", dvalue);
}

void printPAR(uint32_t value) {
    double dvalue = 0.625 * 1e6 * current(value) * 1000;
    printf("%0.2f lx", dvalue);
}

void printVoltage(uint32_t value) {
    double dvalue = value / 4096. * 3.0;
    printf("%0.2f V", dvalue);
}

void printRelativeHumidity(uint32_t value) {
    double dvalue = -4.0 + 0.0405 * value - 0.0000028 * value * value;
    printf("%.2f %%", dvalue);
}

void printTemperature(uint32_t value) {
    double dvalue = -39.6 + 0.01 * (double) value;
    printf("%.2f C", dvalue);
}

// -----------------------------------------------

void writeSerialPort(const void *data, uint16_t len, uint16_t dstAddress) {
    // fprintf(stderr, "shell: writeSerialPort %d bytes to 0x%04x\n", len, dstAddress);
    // hexdump((uint8_t *)data, len);

    len += 2; // inlude destination address as well

    uint8_t header[5];
    header[0] = SERIAL_PACKET_DELIMITER; // always start with this symbol
    header[1] = len >> 8;
    header[2] = len & 0xff;
    header[3] = dstAddress >> 8;
    header[4] = dstAddress & 0xff;

    if (write(serialFD, header, sizeof(header)) != sizeof(header)) {
        // TODO: handle EAGAIN
        perror("write header serial port");
        return;
    }
    if (write(serialFD, data, len) != len) {
        // TODO: handle EAGAIN
        perror("write data serial port");
    }
}

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|0|  length (7) |G|O|
+---------------------------------------------------------------+

                          <-- 8 bits -->
  +---------------------------------------------------------------+
1 | 0x0 - single byte length (1 bit) |    0x6 - length (7 bits)   |
  +---------------------------------------------------------------+
2 |             0x0 - SSMP GET command (8 bits)                   |
  +---------------------------------------------------------------+
3 | 0x0 - SSMP GET command (2 bits)  |      unused (6 bits)       |
  +---------------------------------------------------------------+
4 | 0x1 - SSMP OID (2 bits)          |  0x2 - OID length (6 bits) |
  +---------------------------------------------------------------+
5 |               0x1 - OID 'MANSOS' (8 bits)                     |
  +---------------------------------------------------------------+
6 |                0x3 - OID 'LEDs' (8 bits)                      |
  +---------------------------------------------------------------+
7 |            0x0 - OID 'LED code RED' (8 bits)                  |
  +---------------------------------------------------------------+

*/
void smpSend(SmpCommand_e cmd, Oid_t *oid, unsigned numOid, SmpVariant_t *arg) {
    uint8_t packetBuffer[400];
    uint8_t *p = packetBuffer;

    commandInProgress = true;
    commandIssuedTime = time(NULL);
    commandReplyReceived = false;

    p++; // leave place for length
    *p++ = SMP_PACKET_REQUEST;

    // Mote type (always Tmote Sky)
    *p++ = SMP_COMMAND_GET;
    *p++ = SMP_ELEM_OID | 2;
    *p++ = SMP_MOS;
    *p++ = SMP_RES_TYPE;

    // Mote PAN address
    *p++ = SMP_COMMAND_GET;
    *p++ = SMP_ELEM_OID | 2;
    *p++ = SMP_MOS;
    *p++ = SMP_RES_ADDRESS;

    for (unsigned i = 0; i < numOid; ++i) {
        if (oid->length) {
            *p++ = cmd;
            *p++ = SMP_ELEM_OID | oid->length;
            memcpy(p, oid->oid, oid->length);
            p += oid->length;
        }
        ++oid;
    }
    if (arg && arg->type != 0xFF) {
        uint16_t maxLen = sizeof(packetBuffer) - (p - packetBuffer);
        encodeVariant(&p, &maxLen, arg);
    }

    packetBuffer[0] = p - packetBuffer;

    writeSerialPort(packetBuffer, p - packetBuffer, dstAddress);
}

void smpSend(void *binaryPacket, uint8_t binaryPacketLen) {
    uint8_t packetBuffer[400];
    uint8_t *p = packetBuffer;

    commandInProgress = true;
    commandIssuedTime = time(NULL);
    commandReplyReceived = false;

    p++; // leave place for length
    *p++ = SMP_PACKET_REQUEST;

    // Binary packet
    *p++ = SMP_COMMAND_SET;
    *p++ = SMP_ELEM_OID | 2;
    *p++ = SMP_MOS;
    *p++ = SMP_RES_BINARY_PACKET;

    uint16_t maxLen = sizeof(packetBuffer) - (p - packetBuffer);
    encodeBinary(&p, &maxLen, binaryPacket, binaryPacketLen);

    packetBuffer[0] = p - packetBuffer;

    writeSerialPort(packetBuffer, p - packetBuffer, dstAddress);
}

void printOid(uint8_t *oid, uint8_t oidLen) {
    printf(" ");

    KnownOid *ko = matchKnownOid(oid, oidLen);
    if (ko) {
        fputs(ko->name, stdout);
        printf(": ");
        return;
    }

    printf("OID ");
    for (uint8_t i = 0; i < oidLen; ++i) {
        printf("%d.", oid[i]);
    }
    printf(": ");
}

void printValue(SmpVariant_t *value, uint8_t *oid, uint8_t oidLen) {
    uint32_t valueAsInt = 0;
    fputc('"', stdout);
    switch (value->type) {
    case ST_OCTET:
        printf("%u", value->u.uint8);
        valueAsInt = value->u.uint8;
        break;
    case ST_UINTEGER16:
        printf("0x%04x", value->u.uint16);
        valueAsInt = value->u.uint16;
        break;
    case ST_INTEGER:
        printf("%d", value->u.int32);
        valueAsInt = value->u.int32;
        break;
    case ST_UINTEGER:
        printf("%u", value->u.uint32);
        valueAsInt = value->u.int32;
        break;
    case ST_UINTEGER64: {
        uint8_t data[8];
        memcpy(data, &value->u.uint64, 8);
        printf("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                data[0], data[1], data[2], data[3],
                data[4], data[5], data[6], data[7]);
    }
        break;
    default:
        printf("?");
        break;
    }
    fputc('"', stdout);

    KnownOid *ko = matchKnownOid(oid, oidLen);
    if (ko && ko->printFn) {
        printf(" (");
        ko->printFn(valueAsInt);
        printf(")");
    }
}

void smpRecv(uint8_t *data, uint16_t recvLen) {
    int32_t packetLen;
    uint8_t oid[MAX_OID_LEN];
    uint8_t oidLen = 0;
    uint8_t oidPrefixLen = 0;

    SmpVariant_t arg;
    arg.type = 0xFF;

    uint8_t *p = data;

    // TODO: wait for a specific/multiple responses!
    commandReplyReceived = true;

    packetLen = *p++;

    // check packet length
    if (packetLen > recvLen) {
        fprintf(stderr, "shell: packetLen is longer than physical length (%d vs %d)\n",
                packetLen, recvLen);
        return;
    }

    // check packet type
    switch (*p) {
    case SMP_PACKET_REQUEST:
        fprintf(stderr, "shell: ignoring SMP request packet\n");
        return;
    case SMP_PACKET_RESPONSE:
        break;
    default:
        fprintf(stderr, "shell: unrecognized SMP packet type %d\n", *p);
        return;
    }

    if (isCodeFileBeingSent()) {
        // TODO FIXME: abort on an error!
        if (image.blockId >= image.numBlocksToSend) {
            // file sent.
            printf("File uploaded.");
            imageId = 0;
        } else if (image.blockId == 0) {
            printf("Uploading started...");
        } else {
            printf("A chunk uploaded...");
        }
        return;
    }

    // parse the received packet
    printf("A mote with:\n");
    ++p;
    for (; p < data + packetLen; ++p) {
        uint8_t len;
        uint16_t maxLen = packetLen;

        switch (*p & 0xc0) {
        case SMP_ELEM_COMMAND:
            // fprintf(stderr, "SMP_ELEM_COMMAND\n");
            break;

        case SMP_ELEM_OID:
        case SMP_ELEM_OID_PREFIX:
            // fprintf(stderr, "SMP_ELEM_OID\n");
            len = *p & 0x3F;
            if (maxLen < len + 1) {
                fprintf(stderr, "element length too large %d, "
                        "maxLen=%d\n", len, maxLen);
                return;
            }
            if ((*p & 0xc0) == SMP_ELEM_OID) {
                // whole OID
                if (oidPrefixLen + oidLen > MAX_OID_LEN) {
                    fprintf(stderr, "OID too long");
                    return;
                }
                memcpy(oid + oidPrefixLen, p + 1, len);
                oidLen = len + oidPrefixLen;

                printOid(oid, oidLen);
            } else {
                // OID prefix
                oidPrefixLen = len;
                memcpy(oid, p + 1, oidPrefixLen);
            }

            p += len;
            break;

        case SMP_ELEM_VALUE:
            // fprintf(stderr, "SMP_ELEM_VALUE\n");
            if (decodeVariant(&p, &maxLen, &arg)) {
                return;
            }
            printValue(&arg, oid, oidLen);
            fputc('\n', stdout);
            --p; // XXX
            break;
        }
    }
}

char *parseOid(char *p, Oid_t *oid) {
    uint8_t *r = oid->oid;
    uint8_t acc = 0;
    for ( ; ; ++p) {
        if (isdigit(*p)) {
            acc = acc * 10 + *p - '0';
        }
        else if (*p == '.') {
            *r++ = acc;
            acc = 0;
        }
        else {
            *r++ = acc;
            break;
        }
    }
    oid->length = r - oid->oid;
    return p; 
}

void parseValue(char *p, SmpVariant_t *result) {
    while (isspace(*p)) ++p;
    switch (*p) {
    case 'o': // octet
        result->type = ST_OCTET;
        result->u.uint8 = atoi(p + 1);
        break;
    case 'i':
    case 'u':
    default:
        // TODO: support more types
        break;
    }
}

void showHelp() {
    fprintf(stderr, "available commands:\n"
            "ls                              -- list all motes\n"
            "led (red|green|blue) [on|off]   -- control LEDs\n"
            "sense                           -- read sensor values\n"
            "get <OID>                       -- get a specific OID value from all motes\n"
            "set <OID> <type> <value>        -- set a specific OID to <value>\n"
            "select [<address>]              -- select a specific mote (no args for broadcast)\n"
            "load [<file>]                   -- load an ihex file (no args for clear existing)\n"
            "program [<address>]             -- upload code (from ihex file) on a specific mote\n"
            "reboot                          -- reboot mote\n"
            "quit                            -- exit program\n"
            "help                            -- show this help\n");
    fflush(stderr);
}

typedef void (*CmdHandleFunction)(char *args);

struct Command_t {
    const char *name;
    CmdHandleFunction handle;
};

void handleSetCommand(char *args);
void handleGetCommand(char *args);
void handleLsCommand(char *args);
void handleLedCommand(char *args);
void handleSenseCommand(char *args);
void handleQuitCommand(char *args);
void handleHelpCommand(char *args);
void handleSelectAddressCommand(char *args);
void handleLoadFileCommand(char *args);
void handleProgramCommand(char *args);
void handleRebootCommand(char *args);

typedef map<string, Command_t> CommandMap;
CommandMap commands;

static char *trim(char *buffer)
{
    char *start = buffer;
    while (isspace(*start)) ++start;

    char *end = start;
    while (*end != '\0') ++end;
    while (end >= start && (isspace(*end) || *end == 0)) --end;
    if (*end != '\0') { 
        ++end;
        *end = '\0';
    }

    return start;
}

static void parseCmdAndArgs(char *cmdWithArgs, char *cmd, char *&args)
{
    char *src = cmdWithArgs;
    char *dst = cmd;
    while (isspace(*src)) ++src;
    while (*src && !isspace(*src)) {
        if (src - cmdWithArgs >= MAX_CMD_SIZE) {
            break;
        }
        *dst++ = *src++;
    }
    *dst = 0;

    while (isspace(*src)) ++src;
    if (*src) args = src;
    else args = NULL;
}

// is 'p' prefix of 's'?
static bool prefixMatch(const char *p, const char *s) {
    while (*p && *s) {
        if (*p != *s) return false;
        ++p;
        ++s;
    }
    if (*p && !*s) return false;
    return true;
}

static bool parseOnOff(char *args, bool &success) {
    success = true;
    while (isspace(*args)) ++args;

    char *end;
    unsigned u = strtoul(args, &end, 10);
    if (end != args) {
        return (bool) u;
    }

    if (!strcasecmp(args, "on") || !strcasecmp(args, "true")) {
        return true;
    }
    if (!strcasecmp(args, "off") || !strcasecmp(args, "false")) {
        return false;
    }

    success = false;
    return false;
}

void processCommand(char *cmdWithArgs) {
    if (*cmdWithArgs == '\0') return; // do nothing on empty input

    char cmd[MAX_CMD_SIZE + 1];
    char *args;
    parseCmdAndArgs(cmdWithArgs, cmd, args);

    // cout << "got cmd = " << cmd << " args=" << (args ? : "null") << endl;

    CommandMap::iterator it = commands.lower_bound(cmd);
    if (it == commands.end()
            || !prefixMatch(cmd, it->first.c_str())) {
        fprintf(stderr, "unrecognized input \"%s\"\n", cmd);
        showHelp();
        return;
    }

    CommandMap::iterator it1 = it;
    ++it1;
    if (it1 != commands.end()) {
        if (prefixMatch(cmd, it1->first.c_str())) {
            fprintf(stderr, "more than one command matches input \"%s\"\n", cmd);
            showHelp();
            return;
        }
    }

    // cout << "handle " << it->first << " command" << endl;
    it->second.handle(args);
}

void registerCommand(const char *name, CmdHandleFunction fn) {
    Command_t cmd;
    cmd.name = name;
    cmd.handle = fn;
    commands.insert(make_pair(string(name), cmd));
}

// ------------------------------------------------------- commands

void handleSetCommand(char *args) {
    Oid_t oid;
    if (!args) {
        fprintf(stderr, "SMP set command: arguments expected!\n");
        return;
    }
    char *p = parseOid(args, &oid);
    SmpVariant_t arg;
    arg.type = 0xFF;
    parseValue(p, &arg);
    smpSend(SMP_COMMAND_SET, &oid, 1, &arg);
}

void handleGetCommand(char *args) {
    Oid_t oid;
    if (!args) {
        fprintf(stderr, "SMP get command: arguments expected!\n");
        return;
    }
    parseOid(args, &oid);
    smpSend(SMP_COMMAND_GET, &oid, 1, NULL);
}

void handleLedCommand(char *args) {
    if (!args) {
        fprintf(stderr, "LED command: arguments expected!\n");
        return;
    }

    char *end;
    unsigned led = strtoul(args, &end, 10);
    // printf("shell: handleLedCommand, args=\"%s\" end=\"%s\"\n", args, end);
    if (args == end) {
        if (prefixMatch("red", args)) {
            led = SMP_LED_RED;
            args += 3;
        } else if (prefixMatch("green", args)) {
            led = SMP_LED_GREEN;
            args += 5;
        } else if (prefixMatch("blue", args)) {
            led = SMP_LED_BLUE;
            args += 4;
        } else {
            fprintf(stderr, "LED command: invalid argument!\n");
            return;
        }
    } else {
        args = end;
    }

    Oid_t oid;
    oid.oid[0] = SMP_MOS;
    oid.oid[1] = SMP_RES_LED;
    oid.oid[2] = led;
    oid.length = 3;

    bool success;
    bool onOff = parseOnOff(args, success);
    if (success) {
        // set command
        SmpVariant_t arg;
        arg.type = ST_OCTET;
        arg.u.uint8 = onOff;

        //printf("shell: set led %d to %d\n", led, onOff);
        smpSend(SMP_COMMAND_SET, &oid, 1, &arg);
    } else {
        // get command
        smpSend(SMP_COMMAND_GET, &oid, 1, NULL);
    }
}

void handleSenseCommand(char *args) {
    (void) args;
    Oid_t oids[5];
    for (unsigned i = 0; i < 5; ++i) {
        oids[i].oid[0] = SMP_MOS;
        oids[i].oid[1] = SMP_RES_SENSOR;
        oids[i].length = 3;
    }
    oids[0].oid[2] = SMP_SENSOR_TSR;
    oids[1].oid[2] = SMP_SENSOR_PAR;
    oids[2].oid[2] = SMP_SENSOR_VOLTAGE;
    oids[3].oid[2] = SMP_SENSOR_HUMIDITY;
    oids[4].oid[2] = SMP_SENSOR_TEMPERATURE;
    smpSend(SMP_COMMAND_GET, oids, 5, NULL);
}

void handleLsCommand(char *args) {
    (void) args;
    printf("Listing all motes...\n");

    // Mote IEEE address
    Oid_t oid;
    oid.oid[0] = SMP_MOS;
    oid.oid[1] = SMP_RES_IEEE_ADDRESS;
    oid.length = 2;

    smpSend(SMP_COMMAND_GET, &oid, 1, NULL);
}

void handleQuitCommand(char *args) {
    (void) args;
    exit(0);
}

void handleHelpCommand(char *args) {
    (void) args;
    showHelp();
}

void handleSelectAddressCommand(char *args) {
    if (!args) {
        fprintf(stderr, "Select address: selected broadcast address\n");
        dstAddress = 0;
        return;
    }
    char *end;
    unsigned address = strtoul(args, &end, 0);
    if (end == args) {
        fprintf(stderr, "Select address: a number expected!\n");
        return;
    }
    dstAddress = address;
    fprintf(stderr, "Select address: selected address 0x%04x!\n", dstAddress);
}

void handleLoadFileCommand(char *args) {
    if (!args) {
        fprintf(stderr, "shell: clearing existing code file\n");
        image.clear();
        return;
    }

    if (loadIntelHexFile(args) < 0) {
        fprintf(stderr, "shell: failed to load code file\n");
    } else {
        image.toChunks();
        fprintf(stderr, "shell: code file %s loaded\n", args);
    }
}

void handleProgramCommand(char *args) {
    CodeType ct = CT_BOTH;
    if (args) {
        if (!strcmp("user", args)) {
            ct = CT_USER_CODE;
        } else if (!strcmp("system", args)) {
            ct = CT_SYSTEM_CODE;
        } else {
            fprintf(stderr, "shell: expected 'user' or 'system'!\n");
            return;
        }
    }

    if (image.empty()) {
        fprintf(stderr, "shell: code file not loaded!\n");
        return;
    }

    // select a random image id, to be used during this whole command
    imageId = rand() + 1;

    sendCodeFileStart(ct);
}

void handleRebootCommand(char *args) {
    (void) args; // TODO: allow to specify args...

    RebootCommandPacket_t pck;
    pck.type = RPROG_PACKET_REBOOT;
    pck.doReprogram = !image.empty();
    if (pck.doReprogram) {
        pck.intFlashAddress = image.imageStartAddress;
        pck.extFlashAddress = DEFAULT_EXTERNAL_FLASH_ADDRESS
                + imageNumber * MAX_IMAGE_SIZE;
    } else {
        pck.intFlashAddress = 0;
        pck.extFlashAddress = 0;
    }

    smpSend(&pck, sizeof(pck));

    // this is a special command: do not wait for reply
    commandInProgress = false;
}

// -------------------------------------------------------

void sendCodeFileStart(CodeType ct) {
    ReprogrammingStartPacket_t pck;

    pck.type = RPROG_PACKET_START;
    pck.extFlashAddress = DEFAULT_EXTERNAL_FLASH_ADDRESS
            + imageNumber * MAX_IMAGE_SIZE;
    pck.imageId = imageId;
    pck.imageBlockCount = image.numBlocksToSend = image.blockCount(ct);
    // pck.destinationAddress = RA_DST_LOCAL;

    // start sending hex file
    smpSend(&pck, sizeof(pck));
}

void sendCodeFileContinue(void) {
    if (image.blockId >= image.numBlocksToSend) {
        return;
    }

    ReprogrammingContinuePacket_t pck;
    pck.type = RPROG_PACKET_CONTINUE;

    const Chunk &chunk = image.chunks[image.currentChunk];

    pck.imageId = imageId;
    pck.blockId = image.blockId;
    // address = internal flash address, possibly masked with 0x01
    pck.address = chunk.address;
    memcpy(pck.data, &chunk.data[0], REPROGRAMMING_DATA_CHUNK_SIZE);
    pck.crc = crc16((uint8_t *)&pck.address, 2 + REPROGRAMMING_DATA_CHUNK_SIZE);

    // send next hex file chunk
    smpSend(&pck, sizeof(pck));

    ++image.currentChunk;
    ++image.blockId;
}

// -------------------------------------------------------

int readInput() {
    char buffer[100];
    ssize_t ret = read(0, buffer, sizeof(buffer) - 1);
    if (ret < 0) {
        perror("readInput");
        return ret;
    }

    if (ret == 0) {
        // fprintf(stderr, "EOF on stdin\n");
        return 0;
    }
    if (commandInProgress) {
        // ignore user input while commands are in progress
        return ret;
    }

    buffer[ret] = 0;
    processCommand(trim(buffer));
    return ret;
}

__attribute__((unused))
static void hexdump(uint8_t *data, unsigned len) {
    static const char digits[] = "0123456789abcdef";
    unsigned i;
    char line[3 * 16 + 1] = {0};
    char *p = line;
    for (i = 0; i < len; ++i) {
        *p++ = digits[data[i] >> 4];
        *p++ = digits[data[i] & 0xF];
        *p++ = ' ';
        if ((i & 15) == 15) {
            fputs(line, stdout);
            putchar('\n');
            p = line;
        }
    }
    if (i & 15) {
        *p = '\0';
        fputs(line, stdout);
        putchar('\n');
    }
}

void handleDebugPacket(uint8_t *data, uint16_t len) {
    printf("debug: %s", (char *)data);
    if (len > 0 && data[len - 1] != '\n') putchar('\n');
    fflush(stdout);
}

void handleSmpPacket(uint8_t *data, uint16_t len) {
    // printf("read %d bytes from serial port:\n", len);
    // hexdump(data, len);
    smpRecv(data, len);
    printf("\n");
    fflush(stdout);
}

int readData() {
    static uint8_t recvBuf[400];
    static uint8_t protocol;
    static uint16_t symbolsRead;
    static uint16_t expectedLen;
    static enum {
        READ_DELIMITER,
        READ_PROTOCOL,
        READ_LEN_BYTE1,
        READ_LEN_BYTE2,
        READ_DATA
    } state = READ_DELIMITER;

    uint8_t tmpBuf[400];
    ssize_t len = read(serialFD, tmpBuf, sizeof(tmpBuf) - 1);
    uint8_t *input = tmpBuf;

    if (len < 0) {
        perror("readData");
        return len;
    }
    if (len == 0) {
        fprintf(stderr, "EOF on serial port\n");
        return 0;
    }

    // fprintf(stderr, "readData: read %d bytes\n", len);
    // hexdump(tmpBuf, len);

    while (input < tmpBuf + len) {
        switch (state) {
        case READ_DELIMITER:
            if (*input++ == SERIAL_PACKET_DELIMITER) {
                state = READ_PROTOCOL;
            }
            break;
        case READ_PROTOCOL:
            protocol = *input++;
            if (protocol == PROTOCOL_SMP || protocol == PROTOCOL_DEBUG) {
                state = READ_LEN_BYTE1;
            } else if (protocol != SERIAL_PACKET_DELIMITER) {
                state = READ_DELIMITER;
            }
            break;
        case READ_LEN_BYTE1:
            expectedLen = (*input++) << 8;
            state = READ_LEN_BYTE2;
            break;
        case READ_LEN_BYTE2:
            expectedLen = expectedLen | (*input++);
            if (expectedLen == 0 || expectedLen > sizeof(recvBuf) - 1) {
                fprintf(stderr, "shell: invalid packet length "
                        "on serial port: %d\n", expectedLen);
                expectedLen = 0;
                state = READ_DELIMITER;
            } else {
                state = READ_DATA;
            }
            break;
        case READ_DATA:
            // TODO: make more efficient
            recvBuf[symbolsRead++] = *input++;
            if (symbolsRead == expectedLen) {
                switch (protocol) {
                case PROTOCOL_DEBUG:
                    recvBuf[symbolsRead] = '\0';
                    handleDebugPacket(recvBuf, symbolsRead);
                    break;
                case PROTOCOL_SMP:
                    handleSmpPacket(recvBuf, symbolsRead);
                    break;
                default:
                    fprintf(stderr, "unknown data recvd\n");
                    break;
                }
                state = READ_DELIMITER;
                symbolsRead = 0;
            }
            break;
        }
    }
    return len;
}

// usage: ./shell [serial-port]
int main(int argc, char *argv[]) {
    struct termios newtio;

    printf("MansOS command shell; version %d.%d (built %s)\n\n",
            VERSION_MAJOR, VERSION_MINOR, __DATE__);
            
    // Backwards compability
    if (argc == 2) {
        serialDevice = *++argv;
    }
    else {
        while (*++argv != NULL) {
            if  (!strcmp(*argv, "-l")) {
                handleLoadFileCommand(*++argv);
            }
            else if (!strcmp(*argv, "-s")) {
                 handleSelectAddressCommand(*++argv);
            }
            else if (!strcmp(*argv, "-d")) {
                serialDevice = *++argv;
            }
            else
                printf("Unrecognized parameter: %s\n", *argv);
        }
    }
    // fprintf(stderr, "using serial port %s\n", serialDevice);

    serialFD = open(serialDevice, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serialFD < 0) {
        perror("serial open");
        return -1;
    }

    /* Serial port setting */
    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag = CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | IGNBRK;
    cfsetispeed(&newtio, BAUDRATE);
    cfsetospeed(&newtio, BAUDRATE);

    if (tcflush(serialFD, TCIFLUSH) < 0) {
        perror("tcflush");
        return -1;
    }
    if (tcsetattr(serialFD, TCSANOW, &newtio) < 0) {
        perror("tcsetattr");
        return -1;
    }

    registerCommand("ls", handleLsCommand);
    registerCommand("led", handleLedCommand);
    registerCommand("sense", handleSenseCommand);
    registerCommand("set", handleSetCommand);
    registerCommand("get", handleGetCommand);
    registerCommand("quit", handleQuitCommand);
    registerCommand("help", handleHelpCommand);
    registerCommand("?", handleHelpCommand);
    registerCommand("select", handleSelectAddressCommand);
    registerCommand("load", handleLoadFileCommand);
    registerCommand("program", handleProgramCommand);
    registerCommand("reboot", handleRebootCommand);

    fputs(PROMPT, stdout);
    fflush(stdout);

    bool eofOnStdin = false;
    for (;;) {
        fd_set rfds;
        struct timeval tv;
        int ret;

        FD_ZERO(&rfds);
        if (!eofOnStdin) FD_SET(0, &rfds);
        FD_SET(serialFD, &rfds);

        tv.tv_sec = WAIT_TIMEOUT;
        tv.tv_usec = 0;

        ret = select(serialFD + 1, &rfds, NULL, NULL, &tv);
        if (ret == -1) {
            perror("select");
            return -1;
        }

        bool printPrompt = false;

        if (FD_ISSET(serialFD, &rfds)) {
            if (readData() < 0) return -1;
        }
        if (FD_ISSET(0, &rfds)) {
            int ret = readInput();
            if (ret < 0) return -1;
            if (ret == 0) eofOnStdin = true;
            else if (!commandInProgress) {
                printPrompt = true;
            }
        }

        unsigned now = time(NULL);
        if (commandInProgress && now >= commandIssuedTime + COMMAND_TIMEOUT) {
            if (!commandReplyReceived) {
                printf("..timeout.\n");
                imageId = 0;
                commandInProgress = false;
                printPrompt = true;
            } else if (!isCodeFileBeingSent()) {
                commandInProgress = false;
                printPrompt = true;
            }
        }

        if (eofOnStdin && !commandInProgress) {
            // printf("EOF\n");
            fputc('\n', stdout);
            return 0;
        }

        if (printPrompt) {
            fputs(PROMPT, stdout);
            fflush(stdout);
        }

        if (isCodeFileBeingSent() && commandReplyReceived) {
            sendCodeFileContinue();
        }
    }
}

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

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>

#include "../sense.h"
#define PLATFORM_PC
#include "byteorder.h"

void readConfigFromFile(const char *fname, struct in_addr *ip, uint16_t *port, const char **outFile);

int getData(struct in_addr ip, uint16_t port, const char *outFile);

int readPacket(int fd, SadPacket_t *);
void parsePacket(SadPacket_t *, const char *outFile);

#define SAD_PORT 10000

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        return -1;
    }
    
    struct in_addr ip;
    if (!inet_aton(argv[1], &ip)) {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        return -1;
    }

    uint16_t port = argc > 2 ? atoi(argv[2]) : SAD_PORT;
    if (!port) {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        return -1;
    }

    const char *outFile = NULL;
    readConfigFromFile("/var/pckg/client.config", &ip, &port, &outFile);

    for (;;) {
        getData(ip, port, outFile);
        sleep(20);
    }
}

int getData(struct in_addr ip, uint16_t port, const char *outFile) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr = ip;

    fprintf(stderr, "connecting to %s %d\n", inet_ntoa(ip), port);

    if (connect(fd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }

#define BUFFER_SIZE 0x100
    char msg[BUFFER_SIZE + 1] = {0};
    uint16_t msgPos = 0;

    uint8_t currByte, prevByte;

    currByte = 0;
    for (;;) {
        ssize_t len;
        SadPacket_t packet;

        prevByte = currByte;
        len = read(fd, &currByte, 1);
        if (len < 0) {
            perror("read");
            close(fd);
            return -1;
        }
        if (len == 0) {
            fprintf(stderr, "EOF on TCP connection\n");
            close(fd);
            return -1;
        }

        uint16_t stamp = (currByte << 8) + prevByte;

        if (stamp == SAD_STAMP) {
            if (msgPos > 1) fprintf(stderr, "got message: \"%s\"\n", msg);
            memset(msg, 0, msgPos);
            msgPos = 0;

            packet.id = stamp;
            if (readPacket(fd, &packet) < 0) {
                close(fd);
                return -1;
            }

            parsePacket(&packet, outFile);
        }
        else if (isascii(currByte)) {
            if (msgPos == BUFFER_SIZE) {
                fprintf(stderr, "got message: \"%s\"\n", msg);
                memset(msg, 0, msgPos);
                msgPos = 0;
            }
            msg[msgPos++] = currByte;
        }
    }
}

int readPacket(int fd, SadPacket_t *p) {
    uint16_t bytesRead = 2;

    do {
        ssize_t len = read(fd, (char *) p + bytesRead, sizeof(SadPacket_t) - bytesRead);
        if (len < 0) {
            perror("read");
            return -1;
        }
        if (len == 0) {
            fprintf(stderr, "EOF on TCP connection\n");
            return -1;
        }
        bytesRead += len;
    } while (bytesRead < sizeof(SadPacket_t));

    return 0;
}

void parsePacket(SadPacket_t *p, const char *outFile) {
    uint16_t calcCrc = tole16(crc16_data((uint8_t *) p, sizeof(*p) - 2, 0));
    if (p->crc != calcCrc) {
        fprintf(stderr, "%u: got packet with bad CRC 0x%04x, expected 0x%04x\n",
                (unsigned) time(NULL), p->crc, calcCrc);
        return;
    }

    time_t now = time(NULL);
    fprintf(stdout, "%u,%u,", (unsigned) now, p->address);

    Measurement_t *data = &p->data;
    uint16_t i;
    for (i = 0; i < RETRIES; ++i) {
        fprintf(stdout, "%u,", data->light[i]);
    }
    for (i = 0; i < RETRIES; ++i) {
        fprintf(stdout, "%u,", data->psaLight[i]); 
    }
    fprintf(stdout, "%u\n", p->data.voltage);
    fflush(stdout);

    if (outFile) {
        int outFd = open(outFile, O_APPEND | O_WRONLY | O_CREAT, 0644);
        if (outFd == -1) {
            perror("open file");
        } else {
            SadFilePacket_t fp;
            fp.timestamp = now;
            fp.address = p->address;
            memcpy(&fp.data, &p->data, sizeof(fp.data));

            int ret = write(outFd, &fp, sizeof(fp));
            if (ret == -1) {
                perror("write to file");
                close(open("/var/pckg/client.error", O_APPEND | O_WRONLY | O_CREAT, 0644));
            } else if (ret < sizeof(fp)) {
                fprintf(stderr, "write to file: not all data written!\n");
                close(open("/var/pckg/client.error", O_APPEND | O_WRONLY | O_CREAT, 0644));
            }
            close(outFd);
        }
    }
}

void readConfigFromFile(const char *fname, struct in_addr *ip, uint16_t *port, const char **outFile) {
    char ipstr[100], outfstr[200];
    FILE *f;
    int ret;
    struct in_addr newIp;
    int newPort;

    f = fopen(fname, "rb");    
    if (!f) return;
    ret = fscanf(f, "%s %d %s", ipstr, &newPort, outfstr);
    fclose(f);

    if (ret != 2 && ret != 3) {
        fprintf(stderr, "fscanf config file\n"); 
        return;
    }

    if (!inet_aton(ipstr, &newIp)) {
        fprintf(stderr, "inet_aton from config file\n"); 
        return;
    }

    if (newPort < 0 || newPort > 0xffff) {
        fprintf(stderr, "port in config file: %d\n", newPort); 
        return;
    }

    ip->s_addr = newIp.s_addr;
    *port = newPort;
    if (ret > 2) {
        *outFile = strdup(outfstr);
    }
}

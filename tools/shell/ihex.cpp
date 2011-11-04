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

/*
 * This file implements a simple ihex file parser
 * for extracting code segments from a file in Intel Hex format.
*/

#include "common.h"
#include <algorithm>
using namespace std;

Image image;

#define DUMP_BINARY 0

static inline unsigned toHex(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return c;
}

unsigned parseHexInt(const char *buffer, unsigned start, unsigned end)
{
    unsigned result = 0;
    for (unsigned i = start; i < end; ++i) {
        result <<= 4;
        result |= toHex(buffer[i]);
    }
    return result;
}

unsigned parseHexByte(char *&p)
{
    unsigned result = 0;
    result += toHex(*p++) << 4;
    result += toHex(*p++);
    return result;
}

uint16_t crc16(const uint8_t *data, uint16_t len) {
    uint16_t i, crc = 0;

    for (i = 0; i < len; ++i) {
        crc = crc16Add(crc, *data++);
    }

    return crc;
}

int loadIntelHexFile(const char *fname)
{
    image.clear(); // clear any old data

    unsigned currentAddress = 0;
    Section section;

    // read and parse the ihex file
    char line[100];
    FILE *file = fopen(fname, "r");
    if (!file) {
        perror("fopen");
        return -1;
    }

    while (fgets(line, sizeof(line), file)) {
        if (line[0] != ':') {
            cout << "ihex file format error!" << endl;
            return -1;
        }
        unsigned length = parseHexInt(line, 1, 3);
        unsigned address = parseHexInt(line, 3, 7);
        unsigned type = parseHexInt(line, 7, 9);

        if (type >= 1 && type <= 5) {
            continue;
        }
        if (type != 0) {
            cout << "ignoring unknown type field in ihex file: type=" << type << endl;
            continue;
        }

        if (currentAddress != address) {
            if (currentAddress != 0) {
                // a section is completed
                image.sections.push_back(section);
            }
            section.address = currentAddress = address;
            section.data.clear();
        }
        char *p = line + 9;
        char *end = p + length * 2;
        while (p < end) {
            section.data.push_back(parseHexByte(p));
        }
        currentAddress += length;
    }
    if (!section.data.empty()) {
        // also insert the last section
        image.sections.push_back(section);
    }
    fclose(file);

    vector<Section>::iterator it = image.sections.begin();
    for (; it != image.sections.end(); ++it) {
        printf("Segment 0x%x - 0x%x (%u bytes)\n", it->address, it->address + it->data.size(),
                it->data.size());
#if DUMP_BINARY
        unsigned i = 0;
        RawVector::iterator rit = it->data.begin();
        for (; rit != it->data.end(); ++rit) {
            printf("0x%02x,", *rit);
            if (++i % 16 == 0) {
                putc('\n', stdout);
            } else {
                putc(' ', stdout);
            }
        }
        printf("\n");
#endif
    }

    return 0;
}

void Image::toChunks() {
    if (sections.empty()) return;

    sortSections();

    vector<Section>::iterator it = sections.begin();
    for (; it != sections.end(); ++it) {
        const Section &s = *it;
        unsigned address = ALIGN_DOWN(s.address, REPROGRAMMING_DATA_CHUNK_SIZE);
        unsigned end = s.address + s.data.size();

        // do not include sections that want to override bootloader code!
        if (address < DEFAULT_INTERNAL_FLASH_ADDRESS) {
            fprintf(stderr, "ihex: ignoring a section, address 0x%04x is in bootloader code!\n",
                    address);
            continue;
        }

        for (; address < end; address += REPROGRAMMING_DATA_CHUNK_SIZE) {
            Chunk c;
            c.address = address;
            memset(c.data, 0xff, sizeof(c.data)); // fill with 0xff's
            if (address >= s.address) {
                // normal operation
                memcpy(c.data, &s.data[address - s.address], 
                        min(end - address, REPROGRAMMING_DATA_CHUNK_SIZE));
            } else {
                // pad beginning
                memcpy(c.data + (s.address - address), &s.data[0], 
                        REPROGRAMMING_DATA_CHUNK_SIZE - (s.address - address));
            }
            chunks.push_back(c);
        }
    }

    imageStartAddress = chunks[0].address;

    // set a special flag '0x01' bit for each block that is the first one in a flash segment
    uint16_t lastAddress = (uint16_t)~0u;
    vector<Chunk>::iterator cit = chunks.begin();
    for (; cit != chunks.end(); ++cit) {
        uint16_t a = ALIGN_DOWN(cit->address, INT_FLASH_SEGMENT_SIZE);
        if (a != lastAddress) {
            cit->address |= 0x1; // assume address is always aligned to 2
            lastAddress = a;
        }
    }
}

void Image::sortSections() {
    sort(sections.begin(), sections.end());
}

void Image::clear() {
    sections.clear();
    chunks.clear();
    currentChunk = 0;
    imageStartAddress = 0;
    blockId = 0;
    numBlocksToSend = 0;
}

unsigned Image::blockCount(CodeType ct)
{
    blockId = 0;

    if (ct == CT_BOTH) {
        currentChunk = 0;
        return chunks.size();
    }

    unsigned startAddress, endAddress;
    if (ct == CT_SYSTEM_CODE) {
        startAddress = SYSTEM_CODE_START;
        endAddress = startAddress + MAX_SYSTEM_CODE_SIZE;
    } else {
        startAddress = USER_CODE_START;
        endAddress = startAddress + MAX_USER_CODE_SIZE;
    }

    vector<Chunk>::iterator cit = chunks.begin();
    for (; cit != chunks.end(); ++cit) {
        if (cit->address >= startAddress && cit->address < endAddress) {
            break;
        }
    }

    if (cit == chunks.end()) return 0;

    currentChunk = cit - chunks.begin();

    unsigned result = 0;
    for (; cit != chunks.end(); ++cit) {
        if (cit->address >= endAddress) break;
        ++result;
    }

    return result;
}

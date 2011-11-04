#ifndef COMMON_H
#define COMMON_H

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
using namespace std;

// TODO: platform (msp430) specific
#include <hal/platforms/telosb/intflash_hal.h>
#include <hal/platforms/telosb/extflash_hal.h>

#include <kernel/reprogramming.h>
#include <smp/smp.h>
#include <lib/codec/crc.h>

enum CodeType {
    CT_USER_CODE,
    CT_SYSTEM_CODE,
    CT_BOTH,
};

typedef vector<unsigned char> RawVector;

struct Section {
    unsigned address;
    RawVector data;
};

static inline bool operator < (const Section &s1, const Section &s2) {
    return s1.address < s2.address;
}

struct Chunk {
    unsigned address;
    uint8_t data[REPROGRAMMING_DATA_CHUNK_SIZE];
};

struct Image {
    vector<Section> sections;
    vector<Chunk> chunks;
    unsigned currentChunk;
    unsigned imageStartAddress;
    unsigned blockId;
    unsigned numBlocksToSend;

    Image() : currentChunk(0), imageStartAddress(0), blockId(0), numBlocksToSend(0) {}

    void sortSections();
    void toChunks();
    void clear();
    bool empty() {
        return sections.empty();
    }
    unsigned blockCount(CodeType ct);
};

extern Image image;

int loadIntelHexFile(const char *fname);

#endif

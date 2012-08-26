/**
 * Copyright (c) 2008-2012 the MansOS team. All rights reserved.
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

#ifndef MANSOS_FSTREAM_H
#define MANSOS_FSTREAM_H

//
// Flash stream module interface
//

#include <kernel/defines.h>

//
// Initialize empty filesystem on flash disk
//
void flashStreamClear(void);

//
// Position stream pointer for later reading.
// The minimal address accepted is DATA_START;
// if lower address is passed as argument, DATA_START is used instead.
//
void flashStreamSeek(uint32_t addr);

//
// Position stream pointer for later reading,
// relative to current position.
//
void flashStreamSeekToNewBlock(void);

//
// Read data. Reading is done at the position specified by flashStreamSeek(),
// the position left after previous flashStreamRead(), or from the start
// of the recorded stream, if neither flashStreamSeek() nor flashStreamRead()
// has been called before.
// Passed 'dataLen' value should be the size of the buffer;
// it is filled with byte count that was read.
// If CRC of the stored data is wrong, 'false' is returned.
//
bool flashStreamRead(void *buffer, uint16_t *dataLen);

//
// Write new data in flash.
// Data is always appended at the end of the flash stream.
//
bool flashStreamWrite(const uint8_t *data, uint16_t dataSize);

//
// Flush the flash stream.
// If this function is not called, last data written may be lost
// when the mote is restarted.
//
bool flashStreamFlush(void);

//
// Verify checksums for used flash blocks.
//
bool flashStreamVerifyChecksums(void);


#endif

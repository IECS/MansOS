#ifndef MANSOS_SDSTREAM_H
#define MANSOS_SDSTREAM_H

//
// SD card stream module interface
//

#include <kernel/defines.h>

//
// Initialize empty filesystem on flash disk
//
void sdcardStreamClear(void);

//
// Position stream pointer for later reading.
// The minimal address accepted is DATA_START;
// if lower address is passed as argument, DATA_START is used instead.
//
void sdcardStreamSeek(uint32_t addr);

//
// Position stream pointer for later reading,
// relative to current position.
//
void sdcardStreamSeekToNewBlock(void);

//
// Read data. Reading is done at the position specified by sdcardStreamSeek(),
// the position left after previous sdcardStreamRead(), or from the start
// of the recorded stream, if neither sdcardStreamSeek() nor sdcardStreamRead()
// has been called before.
// Passed 'dataLen' value should be the size of the buffer;
// it is filled with byte count that was read.
// If CRC of the stored data is wrong, 'false' is returned.
//
bool sdcardStreamRead(void *buffer, uint16_t *dataLen);

//
// Write new data in sdcard.
// Data is always appended at the end of the sdcard stream.
//
bool sdcardStreamWrite(const uint8_t *data, uint16_t dataSize);

//
// Flush the sdcard stream.
// If this function is not called, last data written may be lost
// when the mote is restarted.
//
bool sdcardStreamFlush(void);

//
// Verify checksums for used sdcard blocks.
//
bool sdcardStreamVerifyChecksums(void);


#endif

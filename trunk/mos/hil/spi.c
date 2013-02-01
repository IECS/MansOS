/*
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

//==============================================================================
// Hardware controlled SPI, master mode only
// The actual implementation is in HPL. HAL layer provides macros which
// call the real code
//==============================================================================


#include <spi.h>
#include <digital.h>

/*
 * Writes a string to SPI
 * @param   busId     SPI bus ID
 * @param   buf     the buffer containing the string
 * @param   len     buffer length in bytes
 * @return  0       on success, error code otherwise
 */
void spiWrite(uint8_t busId, const void *buf_, uint16_t len) {
     uint8_t *buf = (uint8_t *) buf_;
     uint8_t *end = buf + len;
     while (buf < end) spiWriteByte(busId, *buf++);
}

/*
 * Reads a message into buffer from SPI (in general - performs n byte reads)
 * @param   busId     SPI bus ID
 * @param   buf      the buffer to store the message
 * @param   len      buffer length in bytes
 * @return  received byte count
 */
void spiRead(uint8_t busId, void *buf_, uint16_t len) {
    uint8_t *buf = (uint8_t *) buf_;
    uint8_t *end = buf + len;
    while (buf < end) *buf++ = spiReadByte(busId);
}

/*
 * Reads a message, discards it (without storing anywhere)
 * @param   busId     SPI bus ID
 * @param   len     discarded message length in bytes
 * @return  received byte count
 */
uint8_t spiReadAndDiscard(uint8_t busId, uint16_t len) {
    uint16_t i;
    for (i = 0; i < len; ++i) {
        // no possibility to detect errors
        spiReadByte(busId);
    }
    return len;
}

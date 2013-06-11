/*
 * Copyright (c) 2013 the MansOS team. All rights reserved.
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

#ifndef MANSOS_CODEC_H
#define MANSOS_CODEC_H

/// \file
/// Routines for data encoding and decoding. Includes checksum calculations
///

#include <defines.h>

//! Calculate 16-bit checksum of data using ^16 + ^12 + ^5 + 1 CITT polynomial
uint16_t crc16(const uint8_t *data, uint16_t len);

//! Calculate 8-bit checksum of data using ^8 + ^5 + ^4 + 1 polynomial
uint8_t crc8(const uint8_t *data, uint16_t len);

///
/// Encode a stream of 'length' bytes bufIn to a HDLC frame bufOut.
///   @return a pointer to the next byte after the last encoded
///
uint8_t *frameEncode(int length, uint8_t* bufIn, uint8_t* bufOut);

///
/// Decode a HDLC frame bufIn to a stream of bytes bufOut.
///
/// At most 'length' bytes are decoded.
/// The 'bufOut' may coincide with 'bufIn' since the result is never longer. 
/// Quits once the frame flag encountered at the end of frame.
///   @return a pointer to the next byte after the last encoded,
///           or NULL if the frame was aborted
///
uint8_t *frameDecode(int length, uint8_t* bufIn, uint8_t* bufOut);

//! Encode a nibble (half of a byte) in twice longer hamming code
static inline uint8_t hammingEncode(uint8_t nibble);

/// Decode Hamming-encoded byte
/// @return false on failure, tru on success
static inline bool hammingDecode(uint8_t byte, uint8_t *result);

/// Decode Hamming-encoded buffer in place
/// @return the length of the decoded buffer
static inline uint16_t hammingDecodeInplace(uint8_t *data, uint16_t length);


// implementation
#include <lib/codec/crc.h>
#include <lib/codec/framer.h>
#include <lib/codec/hamming.h>

#endif

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

//-------------------------------------------------------------------
//      Framer.
// Used to frame packets, e.g for transmission over a serial port
// Based on HDLC protocol (High-level Data Link Control)
//      The ABORT flag is optional due to faster and smaller code. 
//-------------------------------------------------------------------

#ifndef MANSOS_FRAMER_H
#define MANSOS_FRAMER_H

//#define FRAME_ENABLE_ABORT    // Uncomment or define this to enable the abort 

#define FRAME_FLAG_BYTE  0x7e   // Frame start and stop
#define FRAME_ABORT_BYTE 0x7f   // Optional. See the FRAME_ENABLE_ABORT flag
#define FRAME_ESCAPE_BYTE 0x7d
#define FRAME_XOR_BYTE 0x20


//-------------------------------------------------------------------
// Encode a stream of 'length' bytes bufIn to a HDLC frame bufOut.
// Returns a pointer to the next byte after the last encoded
//-------------------------------------------------------------------
uint8_t *frameEncode(int length, uint8_t* bufIn, uint8_t* bufOut);

//-------------------------------------------------------------------
// Decode a HDLC frame bufIn to a stream of bytes bufOut.
//      At most 'length' bytes are decoded.
// The bufOut may coincide with bufIn since the result is never longer. 
//      Quits once the frame flag encountered at the end of frame.
// Returns a pointer to the next byte after the last encoded
//      Returns NULL if frame aborted.
//-------------------------------------------------------------------
uint8_t *frameDecode(int length, uint8_t* bufIn, uint8_t* bufOut);

#endif

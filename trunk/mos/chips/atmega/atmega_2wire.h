
/*
  twi.h - TWI/I2C library for Wiring & Arduino
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*
 * atmega_2wire.h and atmega_2wire.c is a modified version of 2-wire library
 * from Wiring (twi.h and twi.c). Modifications by MansOS team.
 * Modification made:
 * - Naming
 * - Static transmission buffer, only pointer for reception buffer - write data
 *     directly into user's buffer without copying it
 * - CPU_FREQ changed to CPU_HZ for compatibility
 * - TWI_FREQ changed to TWI_SPEED for compatibility
 * - Some comments added
 * - Slave operation removed, only master mode support left
 * - Some indentation and whitespace changes
 * - Error codes changed - HIL layer constants used
 * - Using void pointers for data buffers
 * - On/Off functions added (init does not enable the module implicitly)
 */

#ifndef ATMEGA_2WIRE_H
#define ATMEGA_2WIRE_H

#include "atmega_timers.h"

#ifndef TWI_SPEED
#define TWI_SPEED 100000L
#endif

#ifndef TWI_BUFFER_LENGTH
#define TWI_BUFFER_LENGTH 32
#endif

// bus states
#define TWI_READY 0
#define TWI_MRX   1
#define TWI_MTX   2
//  #define TWI_SRX   3
//  #define TWI_STX   4

void twiInit(void);
void twiOn(); // Enable the I2C bus
void twiOff(); // Disable the I2C bus
//void twiSetAddr(uint8_t addr);
uint8_t twiRead(uint8_t addr, void *data, uint8_t len);
uint8_t twiWrite(uint8_t addr, const void *data, uint8_t len,
        uint8_t wait);
//uint8_t twiTransmit(const void *data, uint8_t len);
//void twiAttachSlaveRxEvent(void (*cb)(uint8_t*, int) );
//void twiAttachSlaveTxEvent(void (*cb)(void) );
void twiReply(uint8_t ack);
void twiStop();
void twiReleaseBus();

#endif // !ATMEGA_2WIRE_H


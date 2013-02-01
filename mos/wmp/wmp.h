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

#ifndef MANSOS_WMP_H
#define MANSOS_WMP_H

/// \file
/// MansOS Web Management Protocol (WMP) public API
///

#include <defines.h>
#include <alarms.h>

//! All WMP-accessible sensors must be listed here
enum WmpSensorType_e {
     WMP_SENSOR_LIGHT = 1,
     WMP_SENSOR_HUMIDITY,
     WMP_SENSOR_BATTERY,
     WMP_SENSOR_ADC0,
     WMP_SENSOR_ADC1,
     WMP_SENSOR_ADC2,
     WMP_SENSOR_ADC3,
     WMP_SENSOR_ADC4,
     WMP_SENSOR_ADC5,
     WMP_SENSOR_ADC6,
     WMP_SENSOR_ADC7,
} PACKED;
typedef enum WmpSensorType_e WmpSensorType_t;

//! All WMP-configurable system outputs must be listed here
enum WmpOutputType_e {
     WMP_OUTPUT_SERIAL = 1,
     WMP_OUTPUT_SDCARD,
     WMP_OUTPUT_FILE,
} PACKED;
typedef enum WmpOutputType_e WmpOutputType_t;

//! These commands are handled on mote-side of the protocol
enum WmpCommandType_e {
    //! set LED state
    WMP_CMD_SET_LED = 1,
    //! get LED state
    WMP_CMD_GET_LED,
    //! set sensor reading period
    WMP_CMD_SET_SENSOR,
    //! get sensor reading period
    WMP_CMD_GET_SENSOR,
    //! enable/disable output (e.g. to serial, file etc.)
    WMP_CMD_SET_OUTPUT,
    //! get output status (enabled/disabled)
    WMP_CMD_GET_OUTPUT,
    //! set local network address
    WMP_CMD_SET_ADDR,
    //! get local network address
    WMP_CMD_GET_ADDR,
    //! set file name (to use for data logging)
    WMP_CMD_SET_FILENAME,
    //! get file name (used for data logging)
    WMP_CMD_GET_FILENAME,
    //! get list of all files on FAT filesystem
    WMP_CMD_GET_FILELIST,
    //! get contents of a file
    WMP_CMD_GET_FILE,
    //! set DAC channel value
    WMP_CMD_SET_DAC,
    //! get DAC channel value
    WMP_CMD_GET_DAC,
} PACKED;
typedef enum WmpCommandType_e WmpCommandType_t;

//! This bit is set in replies to commands
#define WMP_CMD_REPLY_FLAG  0x80

//! WMP return code: success
#define WMP_SUCCESS 0x0
//! WMP return code: error occured
#define WMP_ERROR   0xFF

//! Start character of all WMP binary packets
#define WMP_START_CHARACTER   '$'

// initialization (setups serial receiver)
void wmpInit(void);


//
// EEPROM addresses. Config is stored there.
//
#define WMP_EEPROM_SENSOR_BASE   0   // 20 sensors, 4 bytes for each
#define WMP_EEPROM_OUTPUT_BASE   80  // 10 outputs (on/off)
#define WMP_EEPROM_LED_BASE      90  // 10 leds (on/off)
#define WMP_EEPROM_FILE_BASE     100 // 12 byte filename
#define WMP_EEPROM_NETADDR_BASE  112 // 2 byte network address
#define WMP_EEPROM_NETADDR8_BASE 114 // 8 byte long network address

#define WMP_EEPROM_MAGIC_KEY     0xab116688


#endif

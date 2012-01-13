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

#ifndef MANSOS_AT25DF_H
#define MANSOS_AT25DF_H

#include <kernel/stdtypes.h>

// minimal unit that can be erased
#define AT25DF_SECTOR_SIZE    (4 * 1024)  // sector size is 64kb, but 4kb blocks can be erased

// maximal unit that can be written
#define AT25DF_PAGE_SIZE      256

#define AT25DF_SECTOR_COUNT   512

// initialize pin directions and SPI in general. Enter low power mode afterwards
void at25df_init();
// Enter low power mode (wait for last instruction to complete)
void at25df_sleep();
// Exit low power mode
void at25df_wake();
// Read a block of data from addr
void at25df_read(uint32_t addr, void* buffer, uint16_t len);
// Write len bytes (len <= 256) to flash at addr
// Block can split over multiple sectors/pages
void at25df_write(uint32_t addr, const void *buf, uint16_t len);
// Erase the entire flash
void at25df_bulkErase();
// Erase on sector, containing address addr. Addr is not the number of sector,
// rather an address (any) inside the sector
void at25df_eraseSector(uint32_t addr);


// Wrappers, borrowed from WSN430 library
// http://senstools.gforge.inria.fr

/**
 * \brief Save a whole page of data to the memory.
 * \param page the memory page number to write to.
 * \param buffer a pointer to the data to copy
 */
#define at25df_savePage(page, buffer) \
    at25df_page_program(((uint32_t) (page)) << 8, (buffer), 256)

/**
 * \brief Read data from the memory
 * \param page the memory page number to read from.
 * \param buffer a pointer to the buffer to store the data to.
 */
#define at25df_loadPage(page, buffer) \
    at25df_read(((uint32_t) (page)) << 8, (buffer), 256)


#endif

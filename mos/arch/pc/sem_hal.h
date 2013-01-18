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

#ifndef _SEM_HAL_H_
#define _SEM_HAL_H_

#include <kernel/stdtypes.h>

#ifndef __APPLE__

#include <semaphore.h>

#else // __APPLE defined

#include <mach/semaphore.h>
typedef semaphore_t sem_t;

#endif 


/** @brief Succeeded in decrementing semaphore */
#define SEM_SUCCESS 0
/** @brief Failed in decrementing semaphore */
#define SEM_FAIL 1


/** @brief Initialize a semaphore.
 * @param s A pointer to the semaphore to initialize.
 * @param value Initial value to give the semaphore.
 * Usually zero.
 */
void mos_sem_init(sem_t *s, int8_t value);

/** @brief Destroy a semaphore.
 */
void mos_sem_destroy(sem_t *s);

/** @brief Test the semaphore and decrement if possible, otherwise return.
 * @param s Semaphore
 * @return SEM_SUCCESS or SEM_FAIL
 */
uint8_t mos_sem_try_wait(sem_t *s);

/** @brief Wait on a semaphore.
 * Decrements the interal value of the semaphore if it
 * is non-zero.  If the value is zero, blocks until it
 * isn't zero.
 * @param s A pointer to the semaphore to wait on.
 */
void mos_sem_wait(sem_t *s);

/** @brief Post to the semaphore.
 * Increments the internal value of the semaphore,
 * @param s A pointer to the semaphore to post.
 */
void mos_sem_post(sem_t *s);

/** @brief get value of semaphore. non blocking, does not decrement value
 * @param s Semaphore
 * @return semaphore value
 */
int8_t mos_sem_get_val(sem_t *s);


#endif

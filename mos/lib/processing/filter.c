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

#include "filter.h"

// Initialize Filter_t
Filter_t filterInit(enum Comparators comp, uint16_t treshold) {
    Filter_t result;
    result.comparator = comp;
    result.treshold = treshold;
    return result;
}

bool addFilter(Filter_t *filter, uint16_t *val) {
    switch (filter->comparator) {
    case NOT_EQUAL:
        return (*val != filter->treshold) ? (filter->value = *val) || true : false;
        break;
    case EQUAL:
        return (*val == filter->treshold) ? (filter->value = *val) || true : false;
        break;
    case LESS:
        return (*val < filter->treshold) ? (filter->value = *val) || true : false;
        break;
    case LESS_OR_EQUAL:
        return (*val <= filter->treshold) ? (filter->value = *val) || true : false;
        break;
    case MORE:
        return (*val > filter->treshold) ? (filter->value = *val) || true : false;
        break;
    case MORE_OR_EQUAL:
        return (*val >= filter->treshold) ? (filter->value = *val) || true : false;
        break;
    }
#ifdef DEBUG
    PRINTF("%d is not supported comparator!\n", filter->comparator);
#endif //DEBUG
    return false;
}

uint16_t getFilterValue(Filter_t *filter) {
    return filter->value;
}

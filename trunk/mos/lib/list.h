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

//
// list classes
//

#ifndef MANSOS_LIST_H
#define MANSOS_LIST_H

// -- this comes from sys/queue.h

/*
 * Singly-linked List definitions.
 */
#define SLIST_HEAD(name, type)                      \
    struct name {                                   \
        struct type *slh_first; /* first element */ \
    }

#define SLIST_HEAD_INITIALIZER(head)            \
    { NULL }

#define SLIST_ENTRY(type)                           \
    struct {                                        \
        struct type *sle_next;  /* next element */  \
    }

/*
 * Singly-linked List functions.
 */
#define SLIST_INIT(head) do {                   \
        (head)->slh_first = NULL;               \
    } while (/*CONSTCOND*/0)

#define SLIST_INSERT_AFTER(slistelm, elm, field) do {       \
        (elm)->field.sle_next = (slistelm)->field.sle_next; \
        (slistelm)->field.sle_next = (elm);                 \
    } while (/*CONSTCOND*/0)

#define SLIST_INSERT_HEAD(head, elm, field) do {    \
        (elm)->field.sle_next = (head)->slh_first;  \
        (head)->slh_first = (elm);                  \
    } while (/*CONSTCOND*/0)

#define SLIST_INSERT(prev, elm, field) do {       \
        (elm)->field.sle_next = *prev;            \
        *prev = (elm);                            \
    } while (/*CONSTCOND*/0)

#define SLIST_REMOVE_HEAD(head, field) do {                     \
        (head)->slh_first = (head)->slh_first->field.sle_next;  \
    } while (/*CONSTCOND*/0)

#define SLIST_REMOVE(head, elm, type, field) do {           \
        if ((head)->slh_first == (elm)) {                   \
            SLIST_REMOVE_HEAD((head), field);               \
        }                                                   \
        else {                                              \
            struct type *curelm = (head)->slh_first;        \
            while(curelm->field.sle_next != (elm))          \
                curelm = curelm->field.sle_next;            \
            curelm->field.sle_next =                        \
                    curelm->field.sle_next->field.sle_next; \
        }                                                   \
    } while (/*CONSTCOND*/0)

// safe as SLIST_REMOVE, but don't crash if the element is not present
#define SLIST_REMOVE_SAFE(head, elm, type, field) do {      \
        if ((head)->slh_first == (elm)) {                   \
            SLIST_REMOVE_HEAD((head), field);               \
        }                                                   \
        else {                                              \
            struct type *curelm = (head)->slh_first;        \
            while (curelm && curelm->field.sle_next != (elm)) \
                curelm = curelm->field.sle_next;            \
            if (curelm) {                                   \
                curelm->field.sle_next =                    \
                        curelm->field.sle_next->field.sle_next; \
            }                                                   \
        }                                                   \
    } while (/*CONSTCOND*/0)

#define SLIST_FOREACH(var, head, field)                                 \
    for((var) = (head)->slh_first; (var); (var) = (var)->field.sle_next)

/*
 * Singly-linked List access methods.
 */
#define SLIST_EMPTY(head)   ((head)->slh_first == NULL)
#define SLIST_FIRST(head)   ((head)->slh_first)
#define SLIST_NEXT(elm, field)  ((elm)->field.sle_next)


/*
 * Singly-linked Tail queue declarations.
 */
#define STAILQ_HEAD(name, type)                                 \
    struct name {                                               \
        struct type *stqh_first;/* first element */             \
        struct type **stqh_last;/* addr of last next element */ \
    }

#define STAILQ_HEAD_INITIALIZER(head)           \
    { NULL, &(head).stqh_first }

#define STAILQ_ENTRY(type)                          \
    struct {                                        \
        struct type *stqe_next; /* next element */  \
    }

/*
 * Singly-linked Tail queue functions.
 */
#define STAILQ_EMPTY(head)      ((head)->stqh_first == NULL)

#define STAILQ_FIRST(head)      ((head)->stqh_first)

#define STAILQ_FOREACH(var, head, field)        \
    for((var) = STAILQ_FIRST((head));           \
        (var);                                  \
        (var) = STAILQ_NEXT((var), field))

#define STAILQ_INIT(head) do {                      \
        STAILQ_FIRST((head)) = NULL;                \
        (head)->stqh_last = &STAILQ_FIRST((head));  \
    } while (0)

#define STAILQ_INSERT_AFTER(head, tqelm, elm, field) do {               \
        if ((STAILQ_NEXT((elm), field) = STAILQ_NEXT((tqelm), field)) == NULL) \
            (head)->stqh_last = &STAILQ_NEXT((elm), field);             \
        STAILQ_NEXT((tqelm), field) = (elm);                            \
    } while (0)

#define STAILQ_INSERT_HEAD(head, elm, field) do {                       \
        if ((STAILQ_NEXT((elm), field) = STAILQ_FIRST((head))) == NULL) \
            (head)->stqh_last = &STAILQ_NEXT((elm), field);             \
        STAILQ_FIRST((head)) = (elm);                                   \
    } while (0)

#define STAILQ_INSERT_TAIL(head, elm, field) do {       \
        STAILQ_NEXT((elm), field) = NULL;               \
        *(head)->stqh_last = (elm);                     \
        (head)->stqh_last = &STAILQ_NEXT((elm), field); \
    } while (0)

#define STAILQ_LAST(head, type, field)                                  \
    (STAILQ_EMPTY(head) ?                                               \
            NULL :                                                      \
            ((struct type *)                                            \
                    ((char *)((head)->stqh_last) - __offsetof(struct type, field))))

#define STAILQ_NEXT(elm, field) ((elm)->field.stqe_next)

#define STAILQ_REMOVE(head, elm, type, field) do {                      \
        if (STAILQ_FIRST((head)) == (elm)) {                            \
            STAILQ_REMOVE_HEAD(head, field);                            \
        }                                                               \
        else {                                                          \
            struct type *curelm = STAILQ_FIRST((head));                 \
            while (STAILQ_NEXT(curelm, field) != (elm))                 \
                curelm = STAILQ_NEXT(curelm, field);                    \
            if ((STAILQ_NEXT(curelm, field) =                           \
                            STAILQ_NEXT(STAILQ_NEXT(curelm, field), field)) == NULL) \
                (head)->stqh_last = &STAILQ_NEXT((curelm), field);      \
        }                                                               \
    } while (0)

#define STAILQ_REMOVE_HEAD(head, field) do {                            \
        if ((STAILQ_FIRST((head)) =                                     \
                        STAILQ_NEXT(STAILQ_FIRST((head)), field)) == NULL) \
            (head)->stqh_last = &STAILQ_FIRST((head));                  \
    } while (0)

#define STAILQ_REMOVE_HEAD_UNTIL(head, elm, field) do {                 \
        if ((STAILQ_FIRST((head)) = STAILQ_NEXT((elm), field)) == NULL) \
            (head)->stqh_last = &STAILQ_FIRST((head));                  \
    } while (0)


// -- the following are mine - Atis

#define STAILQ_COUNT(head, field)               \
    ({                                          \
        unsigned __c = 0;                       \
        typeof(((head)->stqh_first)) __p;       \
        STAILQ_FOREACH(__p, head, field) ++__c; \
        __c;                                    \
    })

#define STAILQ_MERGE(head1, head2) do {                 \
        if (!STAILQ_EMPTY(head2)) {                     \
            *(head1)->stqh_last = STAILQ_FIRST(head2);  \
            (head1)->stqh_last = (head2)->stqh_last;    \
        }                                               \
    } while (0)

#define STAILQ_REMOVE_IF(head, ptr, field, condition) do {      \
        typeof(ptr) *__p = &(head)->stqh_first;                 \
        ptr = NULL;                                             \
        while (*__p) {                                          \
            typeof(ptr) __t = *__p;                             \
            if (condition) {                                    \
                *__p = __t->field.stqe_next;                    \
                __t->field.stqe_next = NULL;                    \
                /* update queue end pointer in case needed */   \
                if ((head)->stqh_last == &__t->field.stqe_next) \
                    (head)->stqh_last = __p;                    \
                ptr = __t;                                      \
                break;                                          \
            }                                                   \
            __p = &__t->field.stqe_next;                        \
        }                                                       \
    } while (0)
   

#endif

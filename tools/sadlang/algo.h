#ifndef PARSER_ALGO_H
#define PARSER_ALGO_H

#include <algorithm>

#define foreach(type, container, action)                 \
    do {                                                 \
        for (type::iterator it = (container).begin();    \
             it != (container).end(); ++it) { action; }  \
    } while (false) 

#define foreach_const(type, container, action)              \
    do {                                                    \
        for (type::const_iterator it = (container).begin(); \
             it != (container).end(); ++it) { action; }     \
    } while (false) 

#define for_each_safe(type, container, action)              \
    do {                                                    \
        type::iterator it = (container).begin();            \
        type::iterator next;                                \
        for (; it != (container).end(); it = next) {        \
            next = it;                                      \
            ++next;                                         \
            action;                                         \
        }                                                   \
    } while (false)


template<class Collection, class Elem>
static inline bool has(const Collection &c, const Elem &e) {
    return std::find(c.begin(), c.end(), e) != c.end();
}

#endif

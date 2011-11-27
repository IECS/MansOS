#ifndef SADLANG_COMMON_H
#define SADLANG_COMMON_H

#include "y.tab.h"
#include "algo.h"
#include "parser.h"
#include "objects.h"
#include "condition.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <map>
using namespace std;

extern "C" int yyparse(void);

#define PROGRAM_VERSION  "0.1"

#define IS_SPECIFIED(value)   ((value) != ~0u)
#define IS_UNSPECIFIED(value) ((value) == ~0u)

void userError(const char *fmt, ...) __attribute__((noreturn, format (printf, 1, 2)));
void sysError(const char *fmt, ...) __attribute__((noreturn, format (printf, 1, 2)));

#define SEPARATOR "//-----------------------------\n\n"

struct Parameter_s {
    Parameter_t *next;
    const char *name;
    Value_t* value;
    Condition *condition;

    Parameter_s() : next(), name(), value(), condition() {}
};

struct ParameterList_s {
    Parameter_t *first;

    ParameterList_s() : first() {}

    struct iterator {
        Parameter_t *p;
        iterator(Parameter_t *x) : p(x) {}

        void operator ++() {
            if (p) p = p->next;
        }
        operator Parameter_t *() {
            return p;
        }
        Parameter_t * operator->() {
            return p;
        }
    };

    iterator begin() { return iterator(first); }
    iterator end() { return iterator(NULL); }
};

struct ComponentUseCase_s {
    string name;
    ParameterList_t *params;
    ComponentUseCase_t *next;

    ComponentUseCase_s() : params(), next() {}
};

struct ComponentUseCaseList_s {
    ComponentUseCase_t *first;

    ComponentUseCaseList_s() : first() {}

    struct iterator {
        ComponentUseCase_t *p;
        iterator(ComponentUseCase_t *x) : p(x) {}

        void operator ++() {
            if (p) p = p->next;
        }
        operator ComponentUseCase_t *() {
            return p;
        }
        ComponentUseCase_t * operator->() {
            return p;
        }
    };

    iterator begin() { return iterator(first); }
    iterator end() { return iterator(NULL); }
};

enum ValueType {
    VALUE_BOOL,
    VALUE_INTEGER,
    VALUE_SYMBOLIC,
};

struct Value_s {
    ValueType type;
    union {
        bool boolean;
        int integer;
        const char *string;
    } u;
};

struct StringPair_s
{
    string first, second;
};

string toLowerCase(const string &);
string toUpperCase(const string &);
string toCamelCase(const string &);

void generateCode(void);
void generateConfigFile(void);
void generateMakefile(void);

void initComponentMap(void);

#endif

#ifndef SEAL_CONDITION_H
#define SEAL_CONDITION_H

#include <map>
#include <string>
using namespace std;
#include "y.tab.h"

struct Component;
struct ComponentParameter;

enum ConditionType {
    COND_ALARM_BASED, // checked based on alarm
    COND_PERIODIC,    // checked periodically
    COND_CONSTANT,    // always true/false
};

struct Condition
{
    enum Op {
         // test a boolean value; "number" is ignored in this case
        OP_TEST = 0,
        OP_EQ = EQ_TOKEN,
        OP_NEQ = NEQ_TOKEN,
        OP_GR = GR_TOKEN,
        OP_LE = LE_TOKEN,
        OP_GEQ = GEQ_TOKEN,
        OP_LEQ = LEQ_TOKEN
    };

    ComponentParameter *componentParameter;
    Op op;
    int number;
    bool numberSpecified;
    bool orderReversed;
    bool invert;

    bool generatedAlarm;
    bool isDefault;
    int id;

    Condition() :
            componentParameter(),
            op(OP_TEST),
            number(0),
            numberSpecified(false),
            orderReversed(false),
            invert(false),
            generatedAlarm(false),
            isDefault(false),
            id(~0u) {
    }

    static Condition *makeCondition(const string &component,
            const string &variable, Op op, int number, bool reversed, bool invert);

    bool generateConstants();
    bool generateVariables();
    bool generateAppMainCode();
};

struct ComponentParameter {
    Component *parent;
    string name;
    string function;
    ConditionType type;

    ComponentParameter(struct Component *c, string n, string f, ConditionType t) :
            parent(c), name(n), function(f), type(t) {}
};

struct Component {
    string name;
    map<string, ComponentParameter> params;

    void addValue(const string &name, const string &function, ConditionType);

    ComponentParameter *findParam(const string &name) {
        map<string, ComponentParameter>::iterator it = params.find(name);
        return it == params.end() ? NULL : &it->second;
    }
};

typedef map<string, Component> ComponentMap;
extern ComponentMap allComponents;

typedef vector<Condition *> ConditionCollection;
extern ConditionCollection allConditions;

void initComponentMap(void);

#endif

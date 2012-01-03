#include "common.h"

ConditionCollection allConditions;

Condition *Condition::makeCondition(const string &component,
        const string &variable, Op op, int number, bool reversed, bool invert)
{
    static int id;
    string componentSC = toLowerCase(component);
    string variableSC = toLowerCase(variable);

    ComponentMap::iterator it = allComponents.find(componentSC);
    if (it == allComponents.end()) {
        userError("no such component %s", component.c_str());
        return NULL;
    }

    ComponentParameter *param = it->second.findParam(variableSC);
    if (!param) {
        userError("component %s has no parameter %s", 
                component.c_str(), variable.c_str());
        return NULL;
    }

    Condition *c = new Condition();
    c->componentParameter = param;
    c->op = op;
    c->number = number;
    c->orderReversed = reversed;
    c->invert = invert;
    c->id = ++id;
    return c;
}

void Component::addValue(const string &name, const string &function, ConditionType type)
{
    params.insert(make_pair(name,
                    ComponentParameter(this, name, function, type)));
}

void initComponentMap(void)
{
    {
        Component c;
        c.name = "system";
        c.addValue("time", "getJiffies()", COND_ALARM_BASED);
        allComponents.insert(make_pair(c.name, c));
    }
    // ... //
}

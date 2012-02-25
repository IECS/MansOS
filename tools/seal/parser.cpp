#include "common.h"

static void addParameters(Object *, ParameterList_t *, Condition *condition);
static void addPacketFields(Object *, FieldList_t *);

extern "C" ParameterList_t *parameterListAdd(ParameterList_t *params, Parameter_t *param)
{
    param->next = params->first;
    params->first = param;
    return params;
}

extern "C" ParameterList_t *makeEmptyParameterList(void)
{
    return new ParameterList_t();
}

extern "C" Parameter_t *makeParameter(const char *name, Value_t *value)
{
    Parameter_t *result = new Parameter_t();
    result->name = strdup(name);
    result->value = value;
    return result;
}

extern "C" Parameter_t *makeConditionParameter(Condition_t *condition)
{
    Parameter_t *result = new Parameter_t();
    result->name = strdup("when");
    result->condition = condition;
    return result;
}

extern "C" Value_t *makeBoolValue(bool val)
{
    Value_t *result = new Value_t();
    result->type = VALUE_BOOL;
    result->u.boolean = val;
    return result;
}

extern "C" Value_t *makeNumericValue(unsigned val)
{
    Value_t *result = new Value_t();
    result->type = VALUE_INTEGER;
    result->u.integer = val;
    return result;
}

extern "C" Value_t *makeSymbolicValue(const char *val)
{
    Value_t *result = new Value_t();
    result->type = VALUE_SYMBOLIC;
    result->u.string = val;
    return result;
}


extern "C" FieldList_t *fieldListAdd(FieldList_t *params, Field_t *param)
{
    param->next = params->first;
    params->first = param;
    return params;
}

extern "C" FieldList_t *fieldListMake(Field_t *field)
{
    FieldList_t *result = new FieldList_t();
    result->first = field;
    return result;
}

extern "C" Field_t *makePacketField(const char *name)
{
    Field_t *result = new Field_t();
    result->name = strdup(name);
    return result;
}

extern "C" Field_t *makeConstPacketField(const char *name, Value_t *value)
{
    Field_t *result = new Field_t();
    result->name = strdup(name);
    result->value = value;
    return result;
}

// extern "C" void parseUseToken(const char *component, ParameterList_t *params)
// {
//     printf("use %s\n", component);
//     Object *o = objectsUsed.findOrCreate(component);
//     addParameters(o, params);
// }

// extern "C" void parseReadToken(const char *component, ParameterList_t *params)
// {
//     printf("read %s\n", component);
//     Object *o = objectsUsed.findOrCreate(component);
//     addParameters(o, params);
// }

// extern "C" void parseSinkToken(const char *component, ParameterList_t *params)
// {
//     printf("sink %s\n", component);
//     Object *o = objectsUsed.findOrCreate(component);
//     addParameters(o, params);
// }

extern "C" ComponentUseCase_t *makeComponentUseCase(const char *component,
        ParameterList_t *params) 
{
    ComponentUseCase_t *cu = new ComponentUseCase_t();
    cu->name = component;
    cu->params = params;
    return cu;
}

ComponentUseCase_t *makeComponentUseCaseWithFields(const char *component, ParameterList_t *params, FieldList_t *fields)
{
    ComponentUseCase_t *cu = new ComponentUseCase_t();
    cu->name = component;
    cu->fields = fields;
    cu->params = params;
    return cu;
}

extern "C" void addComponentWithCondition(ComponentUseCase_t *component,
        Condition *cond, bool elseCase)
{
    // cout << "addComponentWithCondition: " << component->name.c_str() << endl;
    Object *o = objectsUsed.findOrCreate(component->name.c_str());
    addParameters(o, component->params, cond);
    if (component->fields) {
        addPacketFields(o, component->fields);
    }
}

extern "C" void addComponentsWithCondition(ComponentUseCaseList_t *list, Condition *c,
        bool elseCase)
{
    ComponentUseCase_t *component = list->first;
    while (component) {
        addComponentWithCondition(component, c, elseCase);
        component = component->next;
    }
}

extern "C" ComponentUseCaseList_t *componentListAdd(
        ComponentUseCaseList_t *components, ComponentUseCase_t *component)
{
    if (component) {
        component->next = components->first;
        components->first = component;
    }
    return components;
}

extern "C" ComponentUseCaseList_t *makeEmptyComponentList(void)
{
    return new ComponentUseCaseList_t();
}

extern "C" StringPair_t *makeStringPair(const char *s1, const char *s2)
{
    StringPair_t *p = new StringPair_t();
    p->first = s1;
    p->second = s2;
    return p;
}

extern "C" Condition *makeCondition(StringPair_t *compvar,
        int op, int number, bool reversed, bool invert)
{
    return Condition::makeCondition(compvar->first, compvar->second,
            (Condition::Op) op, number, reversed, invert);
}

extern "C" Condition *makeBoolCondition(StringPair_t *compvar, bool invert)
{
    return Condition::makeCondition(compvar->first, compvar->second,
            Condition::OP_TEST, 0, false, invert);
}

extern "C" void parseCodeBlock(const char *place, const char* block) 
{
    printf("code block @ %s:\n", place);
    printf("%s\n", block);
}

static void addParameters(Object *object, ParameterList_t *params, Condition *condition)
{
// TODO:
//    std::sort(params->begin(), params->end());
// check for duplicates

    foreach(ParameterList_t, *params,
            if (!object->addParameter(it, condition)) {
                userError("Unknown/unsupported parameter %s for component %s",
                        it->name, object->getName());
            });
}

static void addPacketFields(Object *object, FieldList_t *params)
{
    if (object->type != Object::SINK) sysError("Packet fields specified for object %s: not a sink\n", object->getName());

    if (!params) return;

    Sink *sink = (Sink *) object;
    foreach(FieldList_t, *params,
            sink->addPacketField(it));
}

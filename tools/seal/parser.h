#ifndef SEAL_PARSER_H
#define SEAL_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ParameterList_s ParameterList_t;
typedef struct Parameter_s Parameter_t;
typedef struct FieldList_s FieldList_t;
typedef struct Field_s Field_t;
typedef struct Value_s Value_t;
typedef struct StringPair_s StringPair_t;
typedef struct Condition Condition_t;
typedef struct ComponentUseCase_s ComponentUseCase_t;
typedef struct ComponentUseCaseList_s ComponentUseCaseList_t;

Parameter_t *makeParameter(const char *name, Value_t *value);
Parameter_t *makeConditionParameter(Condition_t *condition);

ParameterList_t *parameterListAdd(ParameterList_t *, Parameter_t *);
ParameterList_t *makeEmptyParameterList(void);

Field_t *makePacketField(const char *name);
Field_t *makeConstPacketField(const char *name, Value_t *value);

FieldList_t *fieldListAdd(FieldList_t *, Field_t *);
FieldList_t *fieldListMake(Field_t *);

Value_t *makeBoolValue(bool val);
Value_t *makeNumericValue(unsigned val);
Value_t *makeSymbolicValue(const char *val);

ComponentUseCase_t *makeComponentUseCase(const char *component, ParameterList_t *params);
ComponentUseCase_t *makeComponentUseCaseWithFields(const char *component, ParameterList_t *params, FieldList_t *fields);

ComponentUseCaseList_t *componentListAdd(ComponentUseCaseList_t *, ComponentUseCase_t *);
ComponentUseCaseList_t *makeEmptyComponentList(void);

void addComponentWithCondition(ComponentUseCase_t *, Condition_t *, bool elseCase);
void addComponentsWithCondition(ComponentUseCaseList_t *, Condition_t *, bool elseCase);

StringPair_t *makeStringPair(const char *s1, const char *s2);

Condition_t *makeCondition(StringPair_t *compvar, int op, int number,
        bool reversed, bool invert);
Condition_t *makeBoolCondition(StringPair_t *compvar, bool invert);

void parseCodeBlock(const char *place, const char* block);

void yyerror(char *s);
int yylex(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

#ifndef SADLANG_PARSER_H
#define SADLANG_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ParameterList_s ParameterList_t;
typedef struct Parameter_s Parameter_t;
typedef struct Value_s Value_t;
typedef struct StringPair_s StringPair_t;
typedef struct Condition Condition_t;
typedef struct ComponentUseCase_s ComponentUseCase_t;
typedef struct ComponentUseCaseList_s ComponentUseCaseList_t;

Parameter_t *makeParameter(const char *name, Value_t *value);
Parameter_t *makeConditionParameter(Condition_t *condition);

ParameterList_t *parameterListAdd(ParameterList_t *, Parameter_t *);
ParameterList_t *makeEmptyParameterList(void);

Value_t *makeBoolValue(bool val);
Value_t *makeNumericValue(unsigned val);
Value_t *makeSymbolicValue(const char *val);

ComponentUseCase_t *makeComponentUseCase(const char *component, ParameterList_t *params);

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

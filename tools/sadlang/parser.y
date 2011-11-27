%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "parser.h"
%}

%union {
    int boolean;
    int integer;
    char *string;
    struct Parameter_s *param;
    struct ParameterList_s *params;
    struct ComponentUseCase_s *component;
    struct ComponentUseCaseList_s *components;
    struct Value_s *value;
    struct StringPair_s *stringPair;
    struct Condition *condition;
};

%token <integer> INTEGER_TOKEN
%token <integer> SECONDS_TOKEN
%token <integer> MILLISECONDS_TOKEN
%token <string> IDENTIFIER_TOKEN CODE_BLOCK
%token PARAMETER_TOKEN
%token USE_TOKEN
%token READ_TOKEN
%token SINK_TOKEN
%token WHEN_TOKEN
%token ELSE_TOKEN
%token ELSEWHEN_TOKEN
%token END_TOKEN
%token CODE_TOKEN
%token TRUE_TOKEN FALSE_TOKEN NOT_TOKEN
%token EQ_TOKEN NEQ_TOKEN GR_TOKEN LE_TOKEN GEQ_TOKEN LEQ_TOKEN

%type <param> parameter
%type <params> parameter_list
%type <component> declaration
%type <components> declaration_list
%type <value> value
%type <boolean> boolean_value
%type <integer> qualified_number
%type <stringPair> class_parameter
%type <condition> condition
%type <integer> op

%%

program:
        declaration_list                  { addComponentsWithCondition($1, NULL, false); }
        ;

declaration_list:
        declaration_list declaration      { $$ = componentListAdd($1, $2); }
        |  /* NULL */                     { $$ = makeEmptyComponentList(); }
        ;

declaration:
        USE_TOKEN IDENTIFIER_TOKEN parameter_list ';'
                   { $$ = makeComponentUseCase($2, $3); }
        | READ_TOKEN IDENTIFIER_TOKEN parameter_list ';'
                   { $$ = makeComponentUseCase($2, $3); }
        | SINK_TOKEN IDENTIFIER_TOKEN packet_field_specifier parameter_list ';'
                   { $$ = makeComponentUseCase($2, $4); }
        | when_block
                   { printf("when block\n"); $$ = NULL; }
        | code_block
                   { printf("code block\n"); $$ = NULL; }
        ;

when_block:
        WHEN_TOKEN condition ':' declaration
            { addComponentWithCondition($4, $2, false); }
        | WHEN_TOKEN condition ':' declaration_list elsewhen_block END_TOKEN
            { addComponentsWithCondition($4, $2, false); }
        ;

elsewhen_block:
        ELSEWHEN_TOKEN condition ':' declaration_list elsewhen_block
            { addComponentsWithCondition($4, $2, false); }
        | ELSE_TOKEN ':' declaration_list
            { addComponentsWithCondition($3, NULL, false); /*TODO*/ } 
        | /* NULL */
        ;

code_block:
        CODE_TOKEN IDENTIFIER_TOKEN CODE_BLOCK    { parseCodeBlock($2, $3); }
        ;

packet_field_specifier:
        '{' identifier_list '}'
        |  /* NULL */
        ;

parameter_list:
        parameter_list ',' parameter                      { $$ = parameterListAdd($1, $3); }
        |  /* NULL */                                     { $$ = makeEmptyParameterList(); }
        ;

parameter:
        IDENTIFIER_TOKEN                                  { $$ = makeParameter($1, NULL); }
        | IDENTIFIER_TOKEN value                          { $$ = makeParameter($1, $2); }
        | WHEN_TOKEN condition                            { $$ = makeConditionParameter($2); }
        ;

identifier_list:
        identifier_list ',' IDENTIFIER_TOKEN
        | IDENTIFIER_TOKEN
        ;

value:
        boolean_value            { $$ = makeBoolValue($1); }
        | qualified_number       { $$ = makeNumericValue($1); }
        | IDENTIFIER_TOKEN       { $$ = makeSymbolicValue($1); }
        ;

boolean_value:
        TRUE_TOKEN               { $$ = true; }
        | FALSE_TOKEN            { $$ = false; }
        ;

qualified_number:
        INTEGER_TOKEN
        | SECONDS_TOKEN
        | MILLISECONDS_TOKEN
        ;

condition:
        class_parameter                          { $$ = makeBoolCondition($1, false); }
        | NOT_TOKEN class_parameter              { $$ = makeBoolCondition($2, true); }
        | class_parameter op qualified_number    { $$ = makeCondition($1, $2, $3, false, false); }
        | qualified_number op class_parameter    { $$ = makeCondition($3, $2, $1, true, false); }
        | NOT_TOKEN class_parameter op qualified_number    { $$ = makeCondition($2, $3, $4, false, true); }
        | NOT_TOKEN qualified_number op class_parameter    { $$ = makeCondition($4, $3, $2, true, true); }
        ;

class_parameter:
        IDENTIFIER_TOKEN '.' IDENTIFIER_TOKEN    { $$ = makeStringPair($1, $3); }
        ;

op:
        EQ_TOKEN         { $$ = EQ_TOKEN; }
        | NEQ_TOKEN      { $$ = NEQ_TOKEN; }
        | GR_TOKEN       { $$ = GR_TOKEN; }
        | LE_TOKEN       { $$ = LE_TOKEN; }
        | GEQ_TOKEN      { $$ = GEQ_TOKEN; }
        | LEQ_TOKEN      { $$ = LEQ_TOKEN; }
        ;

%%

void yyerror(char *s) {
    fprintf(stderr, "error: %s\n", s);
}

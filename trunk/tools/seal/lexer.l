%{
    #include "y.tab.h"
    #include <stdlib.h>
    void yyerror(char *);
%}

%%

\/\/.*\n    ;       /* skip comments */

"=="        { return EQ_TOKEN; }
"!="        { return NEQ_TOKEN; }
">"         { return GR_TOKEN; }
"<"         { return LE_TOKEN; }
">="        { return GEQ_TOKEN; }
"<="        { return LEQ_TOKEN; }

"\"\"\".*\"\"\""    { return CODE_BLOCK; }

"parameter" { return PARAMETER_TOKEN; }
"use"       { return USE_TOKEN; }
"read"      { return READ_TOKEN; }
"output"    { return OUTPUT_TOKEN; }
"when"      { return WHEN_TOKEN; }
"else"      { return ELSE_TOKEN; }
"elsewhen"  { return ELSEWHEN_TOKEN; }
"code"      { return CODE_TOKEN; }

"true"      { return TRUE_TOKEN; }
"false"     { return FALSE_TOKEN; }
"not"       { return NOT_TOKEN; }

[a-zA-Z_][0-9a-zA-Z_]*           {
                yylval.string = strdup(yytext);
                return IDENTIFIER_TOKEN;
            }

[0-9]+"s"   {
                yylval.integer = atoi(yytext) * 1000;
                return SECONDS_TOKEN;
            }

[0-9]+"ms"  {
                yylval.integer = atoi(yytext);
                return MILLISECONDS_TOKEN;
            }

[0-9]+      {
                yylval.integer = atoi(yytext);
                return INTEGER_TOKEN;
            }

[.,:;{}]    { return *yytext; }

[ \t]       ;       /* skip whitespace */

.           { fprintf(stderr, "Unknown character %c\n", *yytext); exit(1); }

%%

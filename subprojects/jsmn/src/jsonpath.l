%option noyywrap noinput nounput never-interactive prefix="jsonpath_" reentrant bison-bridge

%{
    #include <stdio.h>

    #include "jqpath.h"
    #include "path_parse_context.h"
    #include "path_parse_state.h"
    #include "jsonpath.tab.h"

    typedef void *yyscan_t;
    #define YYSTYPE JSONPATH_STYPE

    #define YY_DECL int jsonpath_lex(JSONPATH_STYPE *yylval_param, \
                                     yyscan_t yyscanner, \
                                     struct path_parse_context *context)
    #define YY_USER_INIT (void)yylval_param;
%}

%%

\"([^\\\n\"]|\\.)*\"        { path_parse_state_capture_string(&context->state, yytext); return STRING; }
\'([^\\\n\']|\\.)*\'        { path_parse_state_capture_string(&context->state, yytext); return STRING; }
\$                          { return ROOT; }
\.                          { context->state.path.depth++; return DOT; }
\!\=                        { context->state.path.op = JQ_NOT_EQUALS; return NOT_EQUALS; }
\=\=                        { context->state.path.op = JQ_EQUALS; return EQUALS; }
\[                          { return OPEN_ARR; }
\*                          { return STAR; }
\]                          { context->state.path.depth++; return CLOSE_ARR; }
[0-9]+\.[0-9]+              { path_parse_state_capture_float(&context->state, yytext); return FLOAT; }
[0-9]+                      { path_parse_state_capture_int(&context->state, yytext); return INT; }
[a-zA-Z_][a-zA-Z0-9_]*      { path_parse_state_add_label(&context->state, yytext); return LABEL; }
[ \t\r\n]                   { }
.                           { return INVALID; }

%%

int jsonpath_lexer_init(struct path_parse_context *context, const char *in) {
    if (context == NULL || in == NULL) {
        return -1;
    }

    if (jsonpath_lex_init((yyscan_t *)&context->scanner) != 0) {
        return -1;
    }

    context->buffer = jsonpath__scan_string(in, (yyscan_t)context->scanner);
    return 0;
}

void jsonpath_lexer_destroy(struct path_parse_context *context) {
    if (context == NULL || context->scanner == NULL) {
        return;
    }

    if (context->buffer != NULL) {
        jsonpath__delete_buffer((YY_BUFFER_STATE)context->buffer,
                                (yyscan_t)context->scanner);
        context->buffer = NULL;
    }

    jsonpath_lex_destroy((yyscan_t)context->scanner);
    context->scanner = NULL;
}

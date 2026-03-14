%define api.prefix {jsonpath_}
%define api.pure full

%code requires {
    #include "path_parse_context.h"
}

%code provides {
    typedef void *yyscan_t;
    int jsonpath_lex(JSONPATH_STYPE *yylval_param, yyscan_t yyscanner,
                     struct path_parse_context *context);
}

%{
    #include <stdio.h>
    #include <stdlib.h>

    #include "jqpath.h"
    #include "path_parse_context.h"
    #include "path_parse_state.h"
    #include "string_funcs.h"

    int jsonpath_error(void *scanner, struct path_parse_context *context,
                       const char *er);
    int jsonpath_lexer_init(struct path_parse_context *context, const char *in);
    void jsonpath_lexer_destroy(struct path_parse_context *context);
%}

%parse-param {void *scanner}
%parse-param {struct path_parse_context *context}
%lex-param {void *scanner}
%lex-param {struct path_parse_context *context}

%token  STRING
%token  DOT
%token  EQUALS
%token  NOT_EQUALS
%token  OPEN_ARR
%token  CLOSE_ARR
%token  FLOAT
%token  INT
%token  LABEL
%token  ROOT
%token  STAR
%token  INVALID

%%

path:
    ROOT complete_path expression
  | ROOT complete_path
  | ROOT expression
  | ROOT
;

complete_path:
    segment
  | segment complete_path
;

segment:
    DOT LABEL
  | OPEN_ARR STRING CLOSE_ARR {
        path_parse_state_add_quoted_member(&context->state, context->state.strval);
    }
  | OPEN_ARR INT CLOSE_ARR {
        path_parse_state_add_index(&context->state, context->state.intval,
                                   context->state.last_int_len);
    }
  | OPEN_ARR STAR CLOSE_ARR {
        path_parse_state_add_wildcard(&context->state);
    }
;

expression:
    EQUALS INT {
        context->state.path.value.type = JQ_INT_VAL;
        context->state.path.value.value.int_val = context->state.intval;
        context->state.path.rendered = true;
    }
  | EQUALS FLOAT {
        context->state.path.value.type = JQ_FLOAT_VAL;
        context->state.path.value.value.float_val = context->state.floatval;
        context->state.path.rendered = true;
    }
  | EQUALS STRING {
        context->state.path.value.type = JQ_STRING_VAL;
        context->state.path.value.value.string_val = copy_string(context->state.strval);
        context->state.path.rendered = true;
    }
  | NOT_EQUALS INT {
        context->state.path.value.type = JQ_INT_VAL;
        context->state.path.value.value.int_val = context->state.intval;
        context->state.path.rendered = true;
    }
  | NOT_EQUALS FLOAT {
        context->state.path.value.type = JQ_FLOAT_VAL;
        context->state.path.value.value.float_val = context->state.floatval;
        context->state.path.rendered = true;
    }
  | NOT_EQUALS STRING {
        context->state.path.value.type = JQ_STRING_VAL;
        context->state.path.value.value.string_val = copy_string(context->state.strval);
        context->state.path.rendered = true;
    }
;

%%

int jsonpath_error(void *scanner, struct path_parse_context *context,
                   const char *er) {
    (void)scanner;
    (void)context;
    printf("Error: %s\n", er);
    return -1;
}

struct jqpath *jsonpath_parse_string(const char *in) {
    struct path_parse_context *context = path_parse_context_create();
    if (context == NULL) {
        return NULL;
    }

    if (jsonpath_lexer_init(context, in) != 0) {
        path_parse_context_destroy(context);
        return NULL;
    }

    int rv = jsonpath_parse(context->scanner, context);
    jsonpath_lexer_destroy(context);
    if (rv != 0) {
        path_parse_context_destroy(context);
        return NULL;
    }

    struct jqpath *path = path_parse_state_replicate(&context->state);
    path_parse_context_destroy(context);
    return path;
}

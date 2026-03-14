#ifndef PATH_PARSE_CONTEXT_H
#define PATH_PARSE_CONTEXT_H

#include "path_parse_state.h"

struct path_parse_context {
    struct path_parse_state state;
    void *scanner;
    void *buffer;
};

struct path_parse_context *path_parse_context_create(void);
void path_parse_context_destroy(struct path_parse_context *context);

#endif

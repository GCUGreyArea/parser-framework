#include <stdlib.h>

#include "path_parse_context.h"

struct path_parse_context *path_parse_context_create(void) {
    struct path_parse_context *context = malloc(sizeof(struct path_parse_context));
    if (context == NULL) {
        return NULL;
    }

    path_parse_state_reset(&context->state);
    context->scanner = NULL;
    context->buffer = NULL;
    return context;
}

void path_parse_context_destroy(struct path_parse_context *context) {
    if (context == NULL) {
        return;
    }

    free(context);
}

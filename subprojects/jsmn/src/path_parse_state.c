#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "jqpath.h"
#include "path_parse_state.h"
#include "string_funcs.h"

void path_parse_state_reset(struct path_parse_state *state) {
    if (state == NULL) {
        return;
    }

    state->intval = 0;
    state->last_int_len = 0;
    state->floatval = 0.0f;
    state->strval = NULL;
    memset(&state->path, 0, sizeof(state->path));
    state->path.op = JQ_NOT_SET;
    state->path.value.type = JQ_NO_VAL;
}

void path_parse_state_capture_int(struct path_parse_state *state, char *value) {
    if (state == NULL || value == NULL) {
        return;
    }

    state->intval = atoi(value);
    state->last_int_len = strlen(value);
}

void path_parse_state_capture_float(struct path_parse_state *state, char *value) {
    if (state == NULL || value == NULL) {
        return;
    }

    state->floatval = atof(value);
}

void path_parse_state_capture_string(struct path_parse_state *state, char *value) {
    if (state == NULL) {
        return;
    }

    state->strval = value;
}

void path_parse_state_add_label(struct path_parse_state *state, const char *label) {
    if (state == NULL || label == NULL) {
        return;
    }

    state->path.hash =
        merge_hash(state->path.hash, hash(label, strlen(label)));
}

void path_parse_state_add_index(struct path_parse_state *state, int idx,
                                int idx_len) {
    if (state == NULL) {
        return;
    }

    char *value = malloc((size_t)idx_len + 3);
    if (value == NULL) {
        printf("Memory allocation failure\n");
        abort();
    }

    sprintf(value, "[%i]", idx);
    state->path.hash =
        merge_hash(state->path.hash, hash(value, strlen(value)));
    free(value);
}

void path_parse_state_add_wildcard(struct path_parse_state *state) {
    if (state == NULL) {
        return;
    }

    state->path.hash = merge_hash(state->path.hash, hash("[]", 2));
}

void path_parse_state_add_quoted_member(struct path_parse_state *state,
                                        const char *value) {
    if (state == NULL || value == NULL) {
        return;
    }

    const char quote = value[0];
    if (quote != '"' && quote != '\'') {
        return;
    }

    const char *start = value + 1;
    const char *end = start;
    while (*end != '\0' && *end != quote) {
        if (*end == '\\' && *(end + 1) != '\0') {
            end += 2;
        } else {
            end++;
        }
    }

    state->path.hash = merge_hash(state->path.hash, hash(start, end - start));
}

struct jqpath *path_parse_state_replicate(struct path_parse_state *state) {
    if (state == NULL) {
        return NULL;
    }

    struct jqpath *copy = malloc(sizeof(struct jqpath));
    if (copy == NULL) {
        printf("Memory allocation failure\n");
        abort();
    }

    *copy = state->path;
    copy->rendered = false;

    if (copy->op == JQ_NOT_SET) {
        return copy;
    }

    switch (copy->value.type) {
    case JQ_STRING_VAL:
        copy->value.value.string_val = state->path.value.value.string_val;
        state->path.value.value.string_val = NULL;
        copy->rendered = true;
        break;
    case JQ_FLOAT_VAL:
        copy->value.value.float_val = state->path.value.value.float_val;
        copy->rendered = true;
        break;
    case JQ_INT_VAL:
        copy->value.value.int_val = state->path.value.value.int_val;
        copy->rendered = true;
        break;
    default:
        printf("Error: Corrupted value\n");
        abort();
    }

    return copy;
}

void jqpath_close_path(struct jqpath *path) {
    if (path == NULL) {
        return;
    }

    if (path->value.type == JQ_STRING_VAL) {
        kill_string(path->value.value.string_val);
    }

    free(path);
}

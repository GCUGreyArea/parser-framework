#ifndef PATH_PARSE_STATE_H
#define PATH_PARSE_STATE_H

#include "jqpath.h"

struct path_parse_state {
    int intval;
    int last_int_len;
    float floatval;
    char *strval;
    struct jqpath path;
};

void path_parse_state_reset(struct path_parse_state *state);
void path_parse_state_capture_int(struct path_parse_state *state, char *value);
void path_parse_state_capture_float(struct path_parse_state *state, char *value);
void path_parse_state_capture_string(struct path_parse_state *state, char *value);
void path_parse_state_add_label(struct path_parse_state *state, const char *label);
void path_parse_state_add_index(struct path_parse_state *state, int idx,
                                int idx_len);
void path_parse_state_add_wildcard(struct path_parse_state *state);
void path_parse_state_add_quoted_member(struct path_parse_state *state,
                                        const char *value);
struct jqpath *path_parse_state_replicate(struct path_parse_state *state);

#endif

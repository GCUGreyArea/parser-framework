#include <ctype.h>

#include "jqpath.h"

struct jqpath *path_parse_string(const char *str) {
    if (str == NULL) {
        return NULL;
    }

    while (*str != '\0' && isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == '$') {
        return jsonpath_parse_string(str);
    }

    return jqpath_parse_string(str);
}

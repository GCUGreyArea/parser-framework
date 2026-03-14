#include <errno.h>
#include <jsmn.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * An example of reading JSON from stdin and printing its content to stdout.
 * The output looks like YAML, but I'm not sure if it's really compatible.
 */

static int dump(const char *m_js, jsmntok_t *t, size_t count, int indent) {
    int i, j, k;
    jsmntok_t *key;
    if (count == 0) {
        return 0;
    }
    if (t->type == JSMN_PRIMITIVE) {
        printf("%.*s", t->end - t->start, m_js + t->start);
        return 1;
    } else if (t->type == JSMN_STRING) {
        printf("'%.*s'", t->end - t->start, m_js + t->start);
        return 1;
    } else if (t->type == JSMN_OBJECT) {
        printf("\n");
        j = 0;
        for (i = 0; i < t->size; i++) {
            for (k = 0; k < indent; k++) {
                printf("  ");
            }
            key = t + 1 + j;
            j += dump(m_js, key, count - j, indent + 1);
            if (key->size > 0) {
                printf(": ");
                j += dump(m_js, t + 1 + j, count - j, indent + 1);
            }
            printf("\n");
        }
        return j + 1;
    } else if (t->type == JSMN_ARRAY) {
        j = 0;
        printf("\n");
        for (i = 0; i < t->size; i++) {
            for (k = 0; k < indent - 1; k++) {
                printf("  ");
            }
            printf("   - ");
            j += dump(m_js, t + 1 + j, count - j, indent + 1);
            printf("\n");
        }
        return j + 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1)
        return -1;

    int r;

    jsmn_parser p(argv[1]);
    size_t tokcount = 2;

    r = p.parse();
    jsmntok_t *tok = p.get_tokens();

    dump(argv[1], tok, p.parsed_tokens(), 0);

    return EXIT_SUCCESS;
}

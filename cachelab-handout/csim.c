#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HELP    (1 << 0)
#define VERBOSE (1 << 1)

static int mask = 0;
static int s = 0, E = 0, b = 0;
static FILE *fd = NULL;


typedef struct Line {
    unsigned long long *tag;
    int valid;
    struct Line *prev, *next;
} Line;
typedef struct {
    Line *head;
    Line *tail;
} Set;
typedef struct {
    Set *sets;
} Cache;

Cache new_cache() {
    Cache cache;

    int S = 1 << s;
    Set *sets = malloc(S * sizeof(Set));
    for (int i = 0; i < S; i++) {
        Line *lines = calloc(E, sizeof(Line));
        for (int j = 0; j < E; j++) {
            lines[j].prev = lines + j - 1;
            lines[j].next = lines + j + 1;
        }
        lines[0].prev = NULL;
        lines[E- 1].next = NULL;

        sets[i].head = lines;
        sets[i].tail = lines;
    }

    cache.sets = sets;
    return cache;
}

void init(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            mask |= HELP;
            break;
        } else if (strcmp(argv[i], "-v") == 0) {
            mask |= VERBOSE;
        } else if (strcmp(argv[i], "-s") == 0) {
            if (++i < argc) {
                s = atoi(argv[i]);
            }
        } else if (strcmp(argv[i], "-E") == 0) {
            if (++i < argc) {
                E = atoi(argv[i]);
            }
        } else if (strcmp(argv[i], "-b") == 0) {
            if (++i < argc) {
                b = atoi(argv[i]);
            }
        } else if (strcmp(argv[i], "-t") == 0) {
            if (++i < argc) {
                fd = fopen(argv[i], "r");
            }
        } else {
            mask |= HELP;
            break;
        }
    }
}

#ifndef TEST
int main(int argc, char *argv[]) {
    init(argc, argv);

    printSummary(0, 0, 0);
    return 0;
}
#endif

#ifdef TEST
int main(int argc, char *argv[]) {
    init(argc, argv);
    printf("s = %d, E = %d, b = %d, mask = %x\n", s, E, b, mask);
    if (fd != NULL) {
        char *buf = NULL;
        size_t n = 0;
        ssize_t pos;
        while ((pos = getline(&buf, &n, fd)) > 1) {
            if (buf[pos - 1] == '\n') {
                buf[pos - 1] = 0;
            }

            printf("%s", buf);
        }
    } else {
        printf("cannot open file\n");
        exit(1);
    }

}
#endif
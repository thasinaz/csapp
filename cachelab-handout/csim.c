#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HELP 0x1
#define VERBOSE 0x2
#define TEST

int mask = 0;
int S = 0, E = 0, b = 0;
FILE *fd = NULL;


typedef struct Line {
    int tag;
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
                S = 1 << atoi(argv[i]);
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
    printf("S = %d, E = %d, b = %d, mask = %x\n", S, E, b, mask);
    if (fd != NULL) {
        char *buf = NULL;
        size_t n = 0;
        while (getline(&buf, &n, fd) != -1) {
            printf("%s", buf);
        }
    } else {
        printf("cannot open file\n");
        exit(1);
    }
    return 0;
}
#endif
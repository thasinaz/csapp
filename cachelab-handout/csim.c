#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HELP    (1 << 0)
#define VERBOSE (1 << 1)

typedef struct Line {
    unsigned long long tag;
    struct Line *prev, *next;
} Line;
typedef struct {
    Line *head;
    Line *tail;
} Set;
typedef struct {
    int s, E, b;
    Set *sets;
} Cache;

static int mask = 0;
static Cache cache = { 0, 0, 0, NULL };
static FILE *fd = NULL;
static char oper = 0;


int set_parameter(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            mask |= HELP;
            break;
        } else if (strcmp(argv[i], "-v") == 0) {
            mask |= VERBOSE;
        } else if (strcmp(argv[i], "-s") == 0) {
            if (++i < argc) {
                cache.s = atoi(argv[i]);
            }
        } else if (strcmp(argv[i], "-E") == 0) {
            if (++i < argc) {
                cache.E = atoi(argv[i]);
            }
        } else if (strcmp(argv[i], "-b") == 0) {
            if (++i < argc) {
                cache.b = atoi(argv[i]);
            }
        } else if (strcmp(argv[i], "-t") == 0) {
            if (++i < argc) {
                fd = fopen(argv[i], "r");
            }
        } else {
            return -1;
        }
    }
    return 0;
}

int alloc_cache() {
    int S = 1 << cache.s;
    int E = cache.E;
    if (E == 0) {
        return -1;
    }

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
    return 0;
}

int init(int argc, char *argv[]) {
    if (set_parameter(argc, argv) == -1) {
        return -1;
    }
    if ((mask & HELP) != 0) {
        return 0;
    }

    return alloc_cache();
}


unsigned long long parse(char *line) {
    if (line == NULL || line[0] != ' ') {
        oper = 0;
        return 0;
    }

    unsigned long long addr;
    sscanf(line, " %c %llx", &oper, &addr);

    return addr;
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
    if (init(argc, argv) == -1) {
        mask |= HELP;
    }
    if ((mask & HELP) != 0) {
        //print_help();
        return 0;
    }

    printf("s = %d, E = %d, b = %d, mask = %x\n", cache.s, cache.E, cache.b, mask);
    if (fd != NULL) {
        char *buf = NULL;
        size_t n = 0;
        ssize_t pos;
        while ((pos = getline(&buf, &n, fd)) > 1) {
            if (buf[pos - 1] == '\n') {
                buf[pos - 1] = 0;
            }

            printf("%s", buf);
            printf("oper = %c(%d), addr = %llx\n", oper, oper, parse(buf));
        }
    } else {
        printf("cannot open file\n");
        exit(1);
    }

}
#endif

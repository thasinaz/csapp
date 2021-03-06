#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#define BUF_LEN 64

#define HELP    (1 << 0)
#define VERBOSE (1 << 1)

typedef struct Line {
    unsigned long long tag;
    struct Line *prev, *next;
} Line;
typedef struct {
    Line *nil;
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
static int hits = 0, misses = 0, evictions = 0;


int parse_args(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
        case 'h':
            mask |= HELP;
            return 0;
        case 'v':
            mask |= VERBOSE;
            break;
        case 's':
            cache.s = atoi(optarg);
            break;
        case 'E':
            cache.E = atoi(optarg);
            break;
        case 'b':
            cache.b = atoi(optarg);
            break;
        case 't':
            fd = fopen(optarg, "r");
            break;
        default:
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
        Line *lines = malloc((E + 1) * sizeof(Line));
        for (int j = 0; j <= E; j++) {
            lines[j].prev = lines + j - 1;
            lines[j].next = lines + j + 1;
        }
        lines[0].prev = lines + E;
        lines[E].next = lines;

        sets[i].nil = lines;
        sets[i].tail = lines + 1;
    }

    cache.sets = sets;
    return 0;
}

int init(int argc, char *argv[]) {
    if (parse_args(argc, argv) == -1) {
        return -1;
    }
    if ((mask & HELP) != 0) {
        return 0;
    }

    return alloc_cache();
}


void print_help(char *path) {
    printf("Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>", path);
}

void print_verbose(char *s) {
    if ((mask & VERBOSE) != 0) {
        printf("%s", s);
    }
}

void increase_hits() {
    hits++;
    print_verbose("hit");
}

void increase_misses() {
    misses++;
    print_verbose("miss");
}

void increase_evictions() {
    evictions++;
    print_verbose("eviction");
}


unsigned long long parse(char *line) {
    if (line == NULL || line[0] != ' ') {
        oper = 0;
        return 0;
    }

    unsigned long long addr;
    sscanf(line, " %c %llx", &oper, &addr);
    print_verbose(line + 1);

    return addr;
}


int parse_addr(unsigned long long addr,
               size_t *index, unsigned long long *tag) {
    int s = cache.s;
    int b = cache.b;
    *index = (addr >> b) % (1 << s);
    *tag = addr >> (s + b);
    return 0;
}

Line *move_to_front(Set *set, Line *line) {
    Line *head = set->nil->next;

    if (head == line) {
        return line;
    }

    line->prev->next = line->next;
    line->next->prev = line->prev;
    line->prev = set->nil;
    line->next = head;
    head->prev = line;
    set->nil->next = line;
    return line;
}

Line *find(size_t index, unsigned long long tag) {
    Set *set = cache.sets + index;

    Line *line_ptr = set->nil->next;
    Line *tail = set->tail;
    while (line_ptr != tail) {
        if (line_ptr->tag == tag) {
            increase_hits();
            return move_to_front(set, line_ptr);
        }
        line_ptr = line_ptr->next;
    }
    increase_misses();
    return NULL;
}

Line *fetch(size_t index, unsigned long long tag) {
    Set *set = cache.sets + index;
    Line *nil = set->nil;
    Line *tail = set->tail;

    if (tail == nil) {
        tail->prev->tag = tag;
        print_verbose(" ");
        increase_evictions();
        return move_to_front(set, tail->prev);
    } else {
        tail->tag = tag;
        set->tail = tail->next;
        return move_to_front(set, tail);
    }
}

Line *get_cache_line(unsigned long long addr) {
    size_t index;
    unsigned long long tag;
    parse_addr(addr, &index, &tag);
    Line *line;

    if ((line = find(index, tag)) == NULL) {
        return fetch(index, tag);
    }
    return line;
}

void load(unsigned long long addr) {
    get_cache_line(addr);
}

void store(unsigned long long addr) {
    get_cache_line(addr);
}

int access_cache(unsigned long long addr) {
    switch (oper) {
    case 'L':
        load(addr);
        break;
    case 'M':
        load(addr);
        print_verbose(" ");
    case 'S':
        store(addr);
        break;
    default:
        break;
    }

    return 0;
}


#ifndef TEST
int main(int argc, char *argv[]) {
    if (init(argc, argv) == -1) {
        mask |= HELP;
    }
    if ((mask & HELP) != 0) {
        print_help(argv[0]);
        putchar('\n');
        return 0;
    }
    if (fd == NULL) {
        fprintf(stderr, "cannot open file\n");
        exit(1);
    }

    char buf[BUF_LEN];
    while (fgets(buf, BUF_LEN, fd) != NULL) {
        char *ch;
        if ((ch = strchr(buf, '\n')) != NULL) {
            *ch = 0;
        }

        unsigned long long addr = parse(buf);
        print_verbose(" ");
        access_cache(addr);
        print_verbose("\n");
    }

    printSummary(hits, misses, evictions);
    return 0;
}
#endif

#ifdef TEST
int main(int argc, char *argv[]) {
    if (init(argc, argv) == -1) {
        mask |= HELP;
    }
    if ((mask & HELP) != 0) {
        print_help(argv[0]);
        putchar('\n');
        return 0;
    }
    if (fd == NULL) {
        fprintf(stderr, "cannot open file\n");
        exit(1);
    }

    printf("s = %d, E = %d, b = %d, mask = %x\n", cache.s, cache.E, cache.b, mask);

    char buf[BUF_LEN];
    while (fgets(buf, BUF_LEN, fd) != NULL) {
        char *ch;
        if ((ch = strchr(buf, '\n')) != NULL) {
            *ch = 0;
        }

        printf("%s\n", buf);
        unsigned long long addr;
        printf("\noper = %c(%d), addr = %llx\n", oper, oper, (addr = parse(buf)));
        print_verbose(" ");
        access_cache(addr);
        print_verbose("\n");
    }

    printSummary(hits, misses, evictions);
    return 0;
}
#endif

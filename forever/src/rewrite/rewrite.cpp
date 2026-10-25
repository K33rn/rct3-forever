#include "rewrite.h"

namespace rewrite {

static Entry *g_list;

Entry::Entry(const char *name, uintptr_t at, const uint8_t *original, size_t len, void *with, volatile long *calls)
    : name(name), at(at), original(original), original_len(len), with(with), calls(calls), next(g_list) {
    g_list = this;
}

const Entry *list() { return g_list; }

void install_all() {
    int n = 0, done = 0;
    for (Entry *e = g_list; e; e = e->next, n++)
        if (replace_function(e->name, e->at, e->original, e->original_len, e->with)) done++;
    flog("rewritten functions in use: %d of %d", done, n);
}

void report() {
    static long last[256];
    int i = 0;
    for (Entry *e = g_list; e && i < 256; e = e->next, i++) {
        long c = *e->calls;
        if (c != last[i]) flog("%s: %ld calls", e->name, c);
        last[i] = c;
    }
}

}

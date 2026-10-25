#include "rewrite.h"
#include <string.h>

namespace rewrite {

static Entry *g_list;

Entry::Entry(const char *name, uintptr_t at, const uint8_t *original, size_t len, void *with, volatile long *calls)
    : name(name), at(at), original(original), original_len(len), with(with), calls(calls), next(g_list) {
    g_list = this;
}

Copies::Copies(const char *name, const uint32_t *at, size_t n, const uint8_t *original, size_t len, void *with,
               volatile long *calls) {
    for (size_t i = 0; i < n; i++) new Entry(name, at[i], original, len, with, calls);
}

const Entry *list() { return g_list; }

void install_all() {
    int n = 0, done = 0;
    // one log line per rewrite, however many copies of the function it replaces
    for (Entry *e = g_list; e; e = e->next) {
        bool first = true;
        for (Entry *p = g_list; p != e; p = p->next)
            if (!strcmp(p->name, e->name)) first = false;
        int copies = 0, replaced = 0;
        for (Entry *c = e; c; c = c->next) {
            if (strcmp(c->name, e->name)) continue;
            if (first) {
                copies++;
                if (replace_function(c->name, c->at, c->original, c->original_len, c->with)) replaced++;
            }
        }
        if (!first) continue;
        n += copies;
        done += replaced;
        if (copies == 1) flog("replaced: %s at 0x%x", e->name, (unsigned)e->at);
        else flog("replaced: %s, %d of %d copies", e->name, replaced, copies);
    }
    flog("rewritten functions in use: %d of %d", done, n);
}

void report() {
    // calls are counted per rewrite (shared by its copies)
    static const volatile long *seen[256];
    static long last[256];
    int k = 0;
    for (Entry *e = g_list; e && k < 256; e = e->next) {
        bool dup = false;
        for (int i = 0; i < k; i++)
            if (seen[i] == e->calls) dup = true;
        if (dup) continue;
        long c = *e->calls;
        if (c != last[k]) flog("%s: %ld calls", e->name, c);
        seen[k] = e->calls;
        last[k++] = c;
    }
}

}

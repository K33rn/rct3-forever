#include "rewrite.h"
#include <stdlib.h>
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

// ---- Keeping ecx and edx --------------------------------------------------------------------------------------------------
// The game was built with whole-program optimisation: where its compiler knew a function leaves ecx or edx alone, its
// callers keep values in them across the call (0x7CE52D keeps ecx across Value::Band2, and a guest walking into a sandbox
// park then crashed). A rewrite built the usual way may change both. tools/kept_regs.py lists the originals that keep
// one or both, with the argument bytes each takes off the stack; their calls go through a stub:
//   push ecx / push edx (the ones kept), the arguments again (or 64 bytes of the caller's stack when the original takes
//   none off: the rewrite then leaves them too, and the stub drops the copy), call the rewrite, pop them back, return as
//   the original did.
// What the rewrite gives back (eax, st(0), and edx when that isn't kept) passes through.
struct Kept {
    uint32_t at;
    uint8_t keep;    // 1 ecx, 2 edx
    uint8_t bytes;   // argument bytes the original takes off the stack
};
static const Kept g_kept[] = {
#include "kept_regs.inc"
};

static const Kept *kept_for(uintptr_t at) {
    size_t lo = 0, hi = sizeof g_kept / sizeof g_kept[0];
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        if (g_kept[mid].at < at) lo = mid + 1;
        else hi = mid;
    }
    return lo < sizeof g_kept / sizeof g_kept[0] && g_kept[lo].at == at ? &g_kept[lo] : nullptr;
}

static uint8_t *stub_room(size_t n) {
    static uint8_t *room;
    static size_t left;
    if (left < n) {
        room = (uint8_t *)VirtualAlloc(nullptr, 0x10000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        left = room ? 0x10000 : 0;
        if (!room) return nullptr;
    }
    uint8_t *p = room;
    room += n, left -= n;
    return p;
}

void *callable(const Entry *e) {
    if (e->kept) return e->kept;
    const Kept *k = kept_for(e->at);
    if (!k) return e->with;
    uint8_t *const start = stub_room(128), *p = start;
    if (!start) return e->with;
    const int saved = ((k->keep & 1) ? 4 : 0) + ((k->keep & 2) ? 4 : 0);
    const int copy = k->bytes ? k->bytes : 64;
    if (k->keep & 1) *p++ = 0x51;   // push ecx
    if (k->keep & 2) *p++ = 0x52;   // push edx
    for (int i = 0; i < copy; i += 4) *p++ = 0xff, *p++ = 0x74, *p++ = 0x24, *p++ = (uint8_t)(saved + copy);   // push [esp+n]
    *p = 0xe8;   // call with
    *(int32_t *)(p + 1) = (int32_t)((uintptr_t)e->with - ((uintptr_t)p + 5));
    p += 5;
    if (!k->bytes) *p++ = 0x83, *p++ = 0xc4, *p++ = 0x40;   // add esp, 64
    if (k->keep & 2) *p++ = 0x5a;   // pop edx
    if (k->keep & 1) *p++ = 0x59;   // pop ecx
    if (k->bytes) *p++ = 0xc2, *p++ = k->bytes, *p++ = 0;   // ret n
    else *p++ = 0xc3;
    FlushInstructionCache(GetCurrentProcess(), start, (SIZE_T)(p - start));
    e->kept = start;
    return start;
}

// For tracking a fault down: FOREVER_SKIP=a|b leaves out the rewrites with one of those in their name.
static bool skipped(const char *name) {
    const char *list = getenv("FOREVER_SKIP");
    if (!list || !*list) return false;
    for (const char *p = list;;) {
        const char *end = strchr(p, '|');
        size_t n = end ? (size_t)(end - p) : strlen(p);
        for (const char *c = name; *c; c++)
            if (!strncmp(c, p, n)) return true;
        if (!end) return false;
        p = end + 1;
    }
}

void install_all() {
    int n = 0, done = 0, stubbed = 0;
    // one log line per rewrite, however many copies of the function it replaces
    for (Entry *e = g_list; e; e = e->next) {
        bool first = true;
        for (Entry *p = g_list; p != e; p = p->next)
            if (!strcmp(p->name, e->name)) first = false;
        int copies = 0, replaced = 0;
        if (!first) continue;
        const bool skip = skipped(e->name);
        for (Entry *c = e; c; c = c->next) {
            if (strcmp(c->name, e->name)) continue;
            copies++;
            if (skip) continue;
            void *to = callable(c);
            if (replace_function(c->name, c->at, c->original, c->original_len, to)) replaced++, stubbed += to != c->with;
        }
        n += copies;
        done += replaced;
        if (skip) flog("skipped (FOREVER_SKIP): %s, %d %s", e->name, copies, copies == 1 ? "copy" : "copies");
        else if (copies == 1) flog("replaced: %s at 0x%x", e->name, (unsigned)e->at);
        else flog("replaced: %s, %d of %d copies", e->name, replaced, copies);
    }
    flog("rewritten functions in use: %d of %d (%d behind a stub keeping ecx/edx, as their originals do)", done, n, stubbed);
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

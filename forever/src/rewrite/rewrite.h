#pragma once
// A rewritten game function: new code, written from how the function behaves, that takes over every call of the
// original in RCT3plus.exe. Each rewrite/*.cpp registers its functions with REWRITE(...).
#include "../forever.h"

namespace rewrite {
struct Entry {
    const char *name;
    uintptr_t at;              // the original function in RCT3plus.exe
    const uint8_t *original;   // its first bytes, checked before replacing
    size_t original_len;
    void *with;
    volatile long *calls;      // counted by the rewrite, for the log
    Entry *next;
    Entry(const char *name, uintptr_t at, const uint8_t *original, size_t len, void *with, volatile long *calls);
};
}

#define REWRITE_CAT2(a, b) a##b
#define REWRITE_CAT(a, b) REWRITE_CAT2(a, b)
#define REWRITE(name, at, original, fn, calls) \
    static rewrite::Entry REWRITE_CAT(rewrite_entry_, __LINE__)(name, at, original, sizeof original, (void *)fn, calls)

#include "forever.h"
#include <string.h>

bool replace_function(const char *name, uintptr_t at, const uint8_t *expect, size_t n, void *with) {
    uint8_t *p = (uint8_t *)at;
    if (n < 5 || memcmp(p, expect, n)) {
        flog("not replaced: %s at 0x%x (its code isn't the known version)", name, (unsigned)at);
        return false;
    }
    DWORD old;
    VirtualProtect(p, 5, PAGE_EXECUTE_READWRITE, &old);
    p[0] = 0xe9;   // jmp rel32
    *(int32_t *)(p + 1) = (int32_t)((uintptr_t)with - (at + 5));
    VirtualProtect(p, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), p, 5);
    flog("replaced: %s at 0x%x", name, (unsigned)at);
    return true;
}

void *replace_import(uintptr_t slot, void *with) {
    void **p = (void **)slot;
    DWORD old;
    VirtualProtect(p, 4, PAGE_READWRITE, &old);
    void *was = *p;
    *p = with;
    VirtualProtect(p, 4, old, &old);
    return was;
}

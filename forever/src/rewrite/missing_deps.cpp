// PathTypeDatabaseEntry::MissingDependency, QueueTypeDatabaseEntry::MissingDependency and
// PoolTypeDatabaseEntry::MissingDependency (one function written out three times; tools/family.py: missing_deps.inc;
// they differ in where the entry keeps its data, +0x8C or +0x80, and the message): when the entry has data, report
// "Missing dependency for PathType" (or QueueType, PoolType) about it.
//   - The message printed with the game's sprintf into 1 KB (no values), made a wide text (0x10 new bytes,
//     WideString from C; none without memory) and given to the report (0xC211C0: fastcall, the data and the text,
//     then 3); the text let go.
// CIDDatabaseEntry::MissingDependency (0xA2C7B0) is the same without the check: it reports "Missing dependency for
// Carriedtem" (sic) about its +0x7C whether or not it has data. Its row is written here.
// (The game's stack cookie check guards only its own frame.)
#include "rewrite.h"

namespace {
struct MissingDep {
    uint32_t at, data, message;
    bool checked = true;
};
#define MISSING_DEPS(at, data, message, ...) {at, data, message},
constexpr MissingDep missing_deps[] = {
#include "missing_deps.inc"
};
#undef MISSING_DEPS
#define MISSING_DEPS(at, data, message, ...) {__VA_ARGS__},
const uint8_t missing_deps_first[][7] = {
#include "missing_deps.inc"
};
#undef MISSING_DEPS
typedef void(__cdecl *Free)(void *);

volatile long g_missing_deps;
void missing_dep(uint8_t *self, const MissingDep &kind) {
    g_missing_deps++;
    if (kind.checked && !*(void *volatile *)(self + kind.data)) return;
    char message[0x400];
    ((int(__cdecl *)(char *, const char *, ...))0xddcdb5)(message, (const char *)(uintptr_t)kind.message);
    void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
    int32_t *volatile text = memory ? ((int32_t * (__thiscall *)(void *, const char *))0xbcf850)(memory, message) : nullptr;
    ((void(__fastcall *)(void *, void *, uint32_t))0xc211c0)(*(void **)(self + kind.data), (void *)&text, 3);
    --*text;
    int32_t *again = text;   // read again, as the game does
    if (*(volatile int32_t *)again == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)again[3]);
        ((Free)0xbce000)(again);
    }
}

template <int K> void __fastcall missing_dep_copy(uint8_t *self) { missing_dep(self, missing_deps[K]); }
constexpr MissingDep carried_item = {0xa2c7b0, 0x7c, 0xf89320, false};
const uint8_t carried_item_first[] = {0x6a, 0xff, 0x68, 0x39, 0xc7, 0xe7, 0x00};
void __fastcall missing_dep_carried(uint8_t *self) { missing_dep(self, carried_item); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(missing_deps[0].at == 0x7313c0 && missing_deps[1].at == 0x7396e0 && missing_deps[2].at == 0xac3420, "missing_deps.inc changed");
REWRITE("Database::MissingDependency", 0x7313c0, missing_deps_first[0], (&missing_dep_copy<0>), &g_missing_deps);
REWRITE("Database::MissingDependency", 0x7396e0, missing_deps_first[1], (&missing_dep_copy<1>), &g_missing_deps);
REWRITE("Database::MissingDependency", 0xac3420, missing_deps_first[2], (&missing_dep_copy<2>), &g_missing_deps);
REWRITE("Database::MissingDependency", 0xa2c7b0, carried_item_first, &missing_dep_carried, &g_missing_deps);

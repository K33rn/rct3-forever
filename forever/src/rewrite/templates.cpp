// Template rewrites: functions the compiler made hundreds of times, the same apart from one address. Each copy is
// listed in generated.inc (made by tools/gen_rewrites.py from your RCT3plus.exe: addresses and first bytes only).
#include "rewrite.h"

static void(__cdecl *const memory_free)(void *) = (void(__cdecl *)(void *))0xbce000;

// ---- deleting destructors --------------------------------------------------------------------------------------
// What `delete object` calls through a class's virtual function table (slot 0): runs the class's destructor, then
// frees the object's memory when bit 0 of the flag is set; returns the object.

static volatile long g_deleting;

template <uint32_t DTOR> static void *__fastcall deleting(void *self, void *, uint32_t flags) {
    g_deleting++;
    ((void(__fastcall *)(void *))DTOR)(self);
    if (flags & 1) memory_free(self);
    return self;
}

// ---- QueryInterface --------------------------------------------------------------------------------------------
// Asks an object's class (virtual slot 1) for the interface whose id is kept in a global: the class keeps a table of
// its interfaces by id (+0x14, count +0x18), each giving the interface's offset in the object (+0x10).

struct InterfaceEntry {
    char unknown[0x10];
    int32_t offset;
};

struct ClassInterfaces {
    char unknown[0x14];
    InterfaceEntry **by_id;   // +0x14
    int32_t count;            // +0x18
};

static volatile long g_queries;

template <uint32_t ID> static void *__fastcall query(void *object) {
    g_queries++;
    if (!object) return nullptr;
    ClassInterfaces *c = ((ClassInterfaces * (__fastcall *)(void *))(*(void ***)object)[1])(object);
    if (!c) return nullptr;
    int32_t id = *(int32_t *)ID;
    if (id >= c->count) return nullptr;
    InterfaceEntry *e = c->by_id[id];
    if (!e) return nullptr;
    return (uint8_t *)object + e->offset;
}

// ---- module globals ---------------------------------------------------------------------------------------------
// A global object built before main from the start-up of a list of modules: `start` (0xBCE1A0, 0x805AB0, ...)
// runs the list's initialisers in turn (undoing them if one fails) and gives {table, error}; the global's
// constructor (0xBCDF40) throws that error unless it is -1; the global's destructor is registered with atexit.

struct StartResult {
    void *table;
    int32_t error;
};

static volatile long g_module_globals;
static int(__cdecl *const game_atexit)(void(__cdecl *)()) = (int(__cdecl *)(void(__cdecl *)()))0xddc870;
typedef void(__fastcall *GlobalFromStart)(void *global, void *, StartResult result);   // 0xBCDF40: ret 8

template <uint32_t START, uint32_t GLOBAL, uint32_t DTOR> static void __cdecl module_global() {
    g_module_globals++;
    StartResult r;
    ((void(__fastcall *)(StartResult *))START)(&r);
    ((GlobalFromStart)0xbcdf40)((void *)GLOBAL, nullptr, r);
    game_atexit((void(__cdecl *)())DTOR);
}

#define DELETING(at, dtor, freefn, ...)                                                                          \
    static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                        \
    static rewrite::Entry REWRITE_CAT(entry_, at)("DeletingDestructor", at, REWRITE_CAT(original_, at), 8,     \
                                                  (void *)&deleting<dtor>, &g_deleting);
#define QUERY(at, id, ...)                                                                                       \
    static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                        \
    static rewrite::Entry REWRITE_CAT(entry_, at)("QueryInterface", at, REWRITE_CAT(original_, at), 8,         \
                                                  (void *)&query<id>, &g_queries);
#define MODULE(at, start, global, dtor, ...)                                                                         static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                            static rewrite::Entry REWRITE_CAT(entry_, at)("ModuleGlobal", at, REWRITE_CAT(original_, at), 8,                                                             (void *)&module_global<start, global, dtor>, &g_module_globals);
#include "generated.inc"

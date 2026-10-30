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

// ---- cached interfaces ---------------------------------------------------------------------------------------------
// The interface (as QueryInterface) of the object a reference held at +REF points to (the handle's object at +OBJ),
// looked up the first time only: the result (or null) is kept at +CACHE and the byte at +FLAG set.

static volatile long g_cached;

template <uint32_t FLAG, uint32_t REF, uint32_t OBJ, uint32_t ID, uint32_t CACHE>
static void *__fastcall cached_interface(uint8_t *self) {
    g_cached++;
    if (!self[FLAG]) {
        void *object = *(void **)(*(uint8_t **)(self + REF) + OBJ);
        void *found = nullptr;
        if (object) {
            ClassInterfaces *c = ((ClassInterfaces * (__fastcall *)(void *))(*(void ***)object)[1])(object);
            if (c) {
                int32_t id = *(int32_t *)ID;
                if (id < c->count && c->by_id[id]) found = (uint8_t *)object + c->by_id[id]->offset;
            }
        }
        *(void **)(self + CACHE) = found;
        self[FLAG] = 1;
    }
    return *(void **)(self + CACHE);
}

// ---- loading arrays of structs ---------------------------------------------------------------------------------
// The counterpart of StructArrayField::Save (saved_arrays.cpp) for another field class: Load(object, stream,
// loader). The array {data, count} sits in the object at the field's description's offset (+4 -> +0x28); it reads
// the saved length (unused) and count, has the array resized by its item type's own function, then has every
// field (+0xC, count +0x10, virtual slot 1) load itself into every struct.

struct LoadStream {
    void **vtable;
    int32_t position;       // +0x4
    int32_t buffer_start;   // +0x8
    int32_t buffer_end;     // +0xc
    char unknown[4];
    uint8_t buffer[4];      // +0x14 (longer)
};

struct LoadField {
    void **vtable;          // [1] Load(item, stream, loader)
};

struct StructArrayLoader {
    void **vtable;
    uint8_t *info;          // +0x4: the field's description, its offset at +0x28
    char unknown[4];
    LoadField **fields;     // +0xc
    int32_t field_count;    // +0x10
};

static void read4(LoadStream *s, void *into) {
    int32_t at = s->position;
    if (at < s->buffer_start || at + 4 > s->buffer_end) {
        ((void(__fastcall *)(LoadStream *, void *, void *, int32_t))0x809210)(s, nullptr, into, 4);
    } else {
        *(uint32_t *)into = *(uint32_t *)(s->buffer + (at - s->buffer_start));
        s->position = at + 4;
    }
}

static volatile long g_array_loads;

template <uint32_t STRIDE, uint32_t RESIZE>
static void __fastcall struct_array_load(StructArrayLoader *f, void *, uint8_t *object, LoadStream *s, void *loader) {
    g_array_loads++;
    uint8_t **array = (uint8_t **)(object + *(int32_t *)(f->info + 0x28));
    int32_t length, count;
    read4(s, &length);
    read4(s, &count);
    ((void(__fastcall *)(void *, void *, int32_t))RESIZE)(array, nullptr, count);
    for (int32_t i = 0; i < count; i++)
        for (int32_t j = 0; j < f->field_count; j++) {
            LoadField *x = f->fields[j];
            ((void(__fastcall *)(LoadField *, void *, uint8_t *, LoadStream *, void *))x->vtable[1])(
                x, nullptr, *array + i * STRIDE, s, loader);
        }
}

// ---- global clean-ups -----------------------------------------------------------------------------------------
// Deletes the object a global points to, if any (its deleting destructor, virtual slot 0, with 1: destroy and free),
// and clears the global. Run at exit for the globals made on first use.

static volatile long g_cleanups;

template <uint32_t GLOBAL> static void __cdecl global_cleanup() {
    g_cleanups++;
    void *object = *(void **)GLOBAL;
    if (object) ((void(__fastcall *)(void *, void *, uint32_t))(*(void ***)object)[0])(object, nullptr, 1);
    *(void **)GLOBAL = nullptr;
}

// ---- reference constructors ----------------------------------------------------------------------------------
// A reference class built on the reference base (table 0xF26AE0): +4 the handle referred to, +8 its node in the
// handle's list of references (the list's head at handle +0x20; new nodes go first), +0x10 the reference itself,
// +0x14 the value, +0x18 a flag set. Either refers to nothing (the null handle, 0x805750) with the value cleared, or
// to a given object (its handle from 0x805700), the object also kept as the value. Then the class's own table.

static void join_list(uint32_t *self, uint8_t *handle) {
    self[1] = (uint32_t)(uintptr_t)handle;
    uint32_t **head = *(uint32_t ***)(handle + 0x20);
    uint32_t *node = self + 2;
    node[0] = (uint32_t)(uintptr_t)head[0];
    head[0][1] = (uint32_t)(uintptr_t)node;
    node[1] = (uint32_t)(uintptr_t)head;
    head[0] = node;
}

static volatile long g_references;

template <uint32_t TABLE> static void *__fastcall reference_null(uint32_t *self) {
    g_references++;
    self[0] = 0xf26ae0;
    self[4] = (uint32_t)(uintptr_t)self;
    join_list(self, ((uint8_t * (__fastcall *)(void *))0x805750)(self));
    self[0] = TABLE;
    self[5] = 0;
    ((uint8_t *)self)[0x18] = 1;
    return self;
}

template <uint32_t TABLE> static void *__fastcall reference_to(uint32_t *self, void *, void *object) {
    g_references++;
    self[0] = 0xf26ae0;
    self[4] = (uint32_t)(uintptr_t)self;
    join_list(self, ((uint8_t * (__fastcall *)(void *))0x805700)(object));
    self[5] = (uint32_t)(uintptr_t)object;
    self[0] = TABLE;
    ((uint8_t *)self)[0x18] = 1;
    return self;
}

// ---- freeing arrays of objects -------------------------------------------------------------------------------
// An array of objects made with new[]: the count is kept just in front of the items. Destroys every item (the
// runtime's vector destructor iterator, 0xDDCF39), frees the block (aligned free) and clears the pointer.

static volatile long g_array_frees;

template <uint32_t SIZE, uint32_t DTOR> static void __fastcall object_array_free(uint8_t **array) {
    g_array_frees++;
    uint8_t *items = *array;
    if (!items) return;
    ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0xddcf39)(items, SIZE, *(int32_t *)(items - 4),
                                                                     (void *)DTOR);
    ((void(__cdecl *)(void *))0xbce010)(items - 4);
    *array = nullptr;
}

// ---- table setters ---------------------------------------------------------------------------------------------
// Sets the object's virtual function table: what's left of a destructor with nothing else to undo.

static volatile long g_table_sets;

template <uint32_t TABLE> static void __fastcall set_table(uint32_t *self) {
    g_table_sets++;
    self[0] = TABLE;
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

// ---- start-up names, class descriptions and sections ---------------------------------------------------------
// Initialisers run before main (forever.dll is in place by then: the launcher loads it at the game's entry point).
// The originals keep an exception frame for their temporary; these don't. An exception at start-up ends the game
// either way, so all that is lost is the clean-up of that temporary on the way out.

static volatile long g_names, g_class_descriptions, g_sections;

// A name registered from C text (Name::Register, 0xBECA70, through a temporary String: 0xBEC4B0, freed by
// 0xBEC5E0), the name object kept in a global.
template <uint32_t TEXT, uint32_t GLOBAL> static void __cdecl registered_name() {
    g_names++;
    alignas(4) uint8_t text[12];
    void *s = ((void *(__fastcall *)(void *, void *, const char *, int32_t))0xbec4b0)(text, nullptr,
                                                                                      (const char *)TEXT, 0);
    *(uint32_t *)GLOBAL = ((uint32_t(__fastcall *)(void *))0xbeca70)(s);
    ((void(__fastcall *)(void *))0xbec5e0)(text);
}

// A class description (0x3C bytes, made by 0x805FD0 with the class's name and factory) kept in a global; null
// when there was no memory for it.
template <uint32_t NAME, uint32_t FACTORY, uint32_t GLOBAL> static void __cdecl class_description() {
    g_class_descriptions++;
    void *d = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x3c);
    if (d)
        d = ((void *(__fastcall *)(void *, void *, int32_t, const char *, void *))0x805fd0)(
            d, nullptr, 0, (const char *)NAME, (void *)FACTORY);
    *(void **)GLOBAL = d;
}

// A named section (Section::Section, 0x46FAD0) of the given kind, in a global. Its name is a wide string made for it
// (0xBCF850; 0x10 bytes {references, ..., +0xC text}); the section takes a reference to it, and the one held here
// is dropped after (the last one frees it). The section's destructor is registered with atexit. With no memory for
// the name the original crashes on the null string, and so does this.
template <uint32_t TEXT, uint32_t KIND, uint32_t GLOBAL, uint32_t DTOR> static void __cdecl named_section() {
    g_sections++;
    int32_t *name = (int32_t *)((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
    if (name)
        name = ((int32_t * (__fastcall *)(void *, void *, const char *))0xbcf850)(name, nullptr, (const char *)TEXT);
    int32_t *volatile held = name;   // the section is given its address
    ((void(__fastcall *)(void *, void *, int32_t *volatile *, uint32_t))0x46fad0)((void *)GLOBAL, nullptr, &held,
                                                                                 KIND);
    name = held;
    if (--name[0] == 0) {
        ((void(__cdecl *)(void *))0xbce010)(*(void **)(name + 3));
        memory_free(name);
    }
    game_atexit((void(__cdecl *)())DTOR);
}

// ---- texts by name ---------------------------------------------------------------------------------------------
// A shared text: {references, length, capacity, +0xC its characters (an aligned block)}; the last reference frees
// both. As with the start-up ones above, the originals' exception frames (clean-up of their temporaries when a
// called function throws) are left out.

typedef int32_t *Text;

static void release_text(Text t) {
    if (--t[0] == 0) {
        ((void(__cdecl *)(void *))0xbce010)(*(void **)(t + 3));
        memory_free(t);
    }
}

// String::FromC (0xBCFB10) in memory of its own, or the shared empty text (0xBD0520) for none; null when there
// was no memory (the original goes on with it and crashes when it lets go of it, and so does this)
static Text text_from_c(const char *c) {
    if (!c) return ((Text(__fastcall *)(char))0xbd0520)(1);
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
    return p ? ((Text(__fastcall *)(void *, void *, const char *))0xbcfb10)(p, nullptr, c) : nullptr;
}

static volatile long g_named_text_refs, g_game_text_copies;

// NamedTextRef: a reference to the text of a texts table (TextRef::TextRef, 0xBD15A0) by its name, made from C
// text; with `with_suffix`, the name with SUFFIX added (0x403E10). Its class tables are set in turn.
template <uint32_t SUFFIX, uint32_t TABLE, uint32_t CLASS_TABLE>
static void *__fastcall named_text_ref(uint32_t *self, void *, const char *name, void *texts, uint32_t with_suffix) {
    g_named_text_refs++;
    typedef void(__fastcall * TextRefMake)(void *, void *, Text volatile *, void *);   // ret 8
    if (with_suffix & 0xff) {
        Text volatile whole = text_from_c(name);
        Text volatile named;
        Text volatile *r = ((Text volatile * (__fastcall *)(Text volatile *, void *, Text volatile *, const char *))
                                0x403e10)(&whole, nullptr, &named, (const char *)SUFFIX);
        ((TextRefMake)0xbd15a0)(self, nullptr, r, texts);
        self[0] = TABLE;
        release_text(named);
        release_text(whole);
    } else {
        Text volatile t = text_from_c(name);
        ((TextRefMake)0xbd15a0)(self, nullptr, &t, texts);
        self[0] = TABLE;
        release_text(t);
    }
    self[0] = CLASS_TABLE;
    return self;
}

// GameText::Copy: the game text NAME (a NamedTextRef in the texts table 0x1303CFC, with the suffix) copied into a
// wide string of its own (0xBCF7D0), or the empty wide string (0xBD0490) when it has no text; returns `out`.
template <uint32_t NAME> static Text *__stdcall game_text_copy(Text *out) {
    g_game_text_copies++;
    alignas(4) uint8_t ref[0x18];
    uint8_t *r = ((uint8_t * (__fastcall *)(void *, void *, const char *, void *, uint32_t))0x4040f0)(
        ref, nullptr, (const char *)NAME, (void *)0x1303cfc, 1);
    const wchar_t *text = *(const wchar_t **)(r + 8);
    Text t;
    if (text) {
        void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
        t = p ? ((Text(__fastcall *)(void *, void *, const wchar_t *))0xbcf7d0)(p, nullptr, text) : nullptr;
    } else {
        t = ((Text(__fastcall *)(char))0xbd0490)(1);
    }
    *out = t;
    ((void(__fastcall *)(void *))0xbd1880)(ref);
    return out;
}

// The same from a text the caller has (not C text): with `with_suffix` it refers to that text with SUFFIX added
// (0x403E10), the one made for it let go of after.
static volatile long g_text_refs_from_text;
template <uint32_t SUFFIX, uint32_t TABLE, uint32_t CLASS_TABLE>
static void *__fastcall named_text_ref_from_text(uint32_t *self, void *, Text volatile *name, void *texts,
                                                 uint32_t with_suffix) {
    g_text_refs_from_text++;
    typedef void(__fastcall * TextRefMake)(void *, void *, Text volatile *, void *);   // ret 8
    if (with_suffix & 0xff) {
        Text volatile named;
        Text volatile *r = ((Text volatile * (__fastcall *)(Text volatile *, void *, Text volatile *, const char *))
                                0x403e10)(name, nullptr, &named, (const char *)SUFFIX);
        ((TextRefMake)0xbd15a0)(self, nullptr, r, texts);
        self[0] = TABLE;
        release_text(named);
    } else {
        ((TextRefMake)0xbd15a0)(self, nullptr, name, texts);
        self[0] = TABLE;
    }
    self[0] = CLASS_TABLE;
    return self;
}

// ---- start-up key commands ------------------------------------------------------------------------------------
// A key command defined at start-up (0x6B2720 on its global: its group 0xF4473C, the setting's name and the
// command's, its default keys made with KeyCombination::Set (0xBD1940) on the stack, then 1, 1); the global's
// destructor registered with atexit.
static volatile long g_key_commands;
template <uint32_t KEY1, uint32_t KEY2, uint32_t NAME, uint32_t SETTING, uint32_t GLOBAL, uint32_t DTOR>
static void __cdecl key_command() {
    g_key_commands++;
    alignas(4) uint8_t keys[0x14];
    void *k = ((void *(__fastcall *)(void *, void *, uint32_t, uint32_t, uint32_t, uint32_t))0xbd1940)(
        keys, nullptr, KEY1, KEY2, 0, 0);
    ((void(__fastcall *)(void *, void *, uint32_t, uint32_t, uint32_t, void *, uint32_t, uint32_t))0x6b2720)(
        (void *)GLOBAL, nullptr, 0xf4473c, SETTING, NAME, k, 1, 1);
    game_atexit((void(__cdecl *)())DTOR);
}

#define TEXT_REF_FROM_TEXT(at, suffix, table, class_table, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("NamedTextRef::NamedTextRef(text)", at, REWRITE_CAT(original_, at), 8, (void *)&named_text_ref_from_text<suffix, table, class_table>, &g_text_refs_from_text);
#define START_KEY(at, key2, key1, name, setting, global, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::KeyCommand", at, REWRITE_CAT(original_, at), 8, (void *)&key_command<key1, key2, name, setting, global, dtor>, &g_key_commands);
#define NAMED_TEXT_REF(at, suffix, table, class_table, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("NamedTextRef::NamedTextRef", at, REWRITE_CAT(original_, at), 8, (void *)&named_text_ref<suffix, table, class_table>, &g_named_text_refs);
#define GAME_TEXT_COPY(at, name, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("GameText::Copy", at, REWRITE_CAT(original_, at), 8, (void *)&game_text_copy<name>, &g_game_text_copies);
#define START_NAME(at, text, global, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::Name", at, REWRITE_CAT(original_, at), 8, (void *)&registered_name<text, global>, &g_names);
#define START_CLASS(at, name, factory, global, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::ClassDescription", at, REWRITE_CAT(original_, at), 8, (void *)&class_description<name, factory, global>, &g_class_descriptions);
#define START_SECTION(at, text, kind, global, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::Section", at, REWRITE_CAT(original_, at), 8, (void *)&named_section<text, kind, global, dtor>, &g_sections);
#define DELETING(at, dtor, freefn, ...)                                                                        \
    static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                        \
    static rewrite::Entry REWRITE_CAT(entry_, at)("DeletingDestructor", at, REWRITE_CAT(original_, at), 8,     \
                                                  (void *)&deleting<dtor>, &g_deleting);
#define QUERY(at, id, ...)                                                                                       \
    static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                        \
    static rewrite::Entry REWRITE_CAT(entry_, at)("QueryInterface", at, REWRITE_CAT(original_, at), 8,         \
                                                  (void *)&query<id>, &g_queries);
#define MODULE(at, start, global, dtor, ...)                                                                         static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                            static rewrite::Entry REWRITE_CAT(entry_, at)("ModuleGlobal", at, REWRITE_CAT(original_, at), 8,                                                             (void *)&module_global<start, global, dtor>, &g_module_globals);
#define CACHED(at, flag, ref, obj, id, cache, ...)                                                                   static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                            static rewrite::Entry REWRITE_CAT(entry_, at)("CachedInterface", at, REWRITE_CAT(original_, at), 8,                                                          (void *)&cached_interface<flag, ref, obj, id, cache>, &g_cached);
#define LOAD(at, stride, resize, ...)                                                                                static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                            static rewrite::Entry REWRITE_CAT(entry_, at)("StructArrayLoad", at, REWRITE_CAT(original_, at), 8,                                                          (void *)&struct_array_load<stride, resize>, &g_array_loads);
#define CLEANUP(at, global, ...)                                                                                     static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                            static rewrite::Entry REWRITE_CAT(entry_, at)("GlobalCleanup", at, REWRITE_CAT(original_, at), 8,                                                            (void *)&global_cleanup<global>, &g_cleanups);
#define REF_NULL(at, table, ...)                                                                                     static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                            static rewrite::Entry REWRITE_CAT(entry_, at)("Reference::Reference", at, REWRITE_CAT(original_, at), 8,                                                     (void *)&reference_null<table>, &g_references);
#define REF_TO(at, table, ...)                                                                                       static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                            static rewrite::Entry REWRITE_CAT(entry_, at)("Reference::Reference(object)", at,                                                                            REWRITE_CAT(original_, at), 8, (void *)&reference_to<table>,                                                     &g_references);
#define ARRAY_FREE(at, size, dtor, ...)                                                                              static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                            static rewrite::Entry REWRITE_CAT(entry_, at)("ObjectArray::Free", at, REWRITE_CAT(original_, at), 8,                                                        (void *)&object_array_free<size, dtor>, &g_array_frees);
#define SET_TABLE(at, table, ...)                                                                                    static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                            static rewrite::Entry REWRITE_CAT(entry_, at)("SetTable", at, REWRITE_CAT(original_, at), 7,                                                                 (void *)&set_table<table>, &g_table_sets);
#include "generated.inc"
#undef START_NAME
#undef START_CLASS
#undef START_SECTION
#define START_NAME(...)
#define START_CLASS(...)
#define START_SECTION(...)
#undef NAMED_TEXT_REF
#undef GAME_TEXT_COPY
#define NAMED_TEXT_REF(...)
#define GAME_TEXT_COPY(...)
#undef TEXT_REF_FROM_TEXT
#undef START_KEY
#define TEXT_REF_FROM_TEXT(...)
#define START_KEY(...)
#undef SET_TABLE
#define SET_TABLE(...)
#undef REF_NULL
#undef REF_TO
#undef ARRAY_FREE
#define REF_NULL(...)
#define REF_TO(...)
#define ARRAY_FREE(...)

// the cached interfaces' constants, for the equivalence test
#undef DELETING
#undef QUERY
#undef MODULE
#undef CACHED
#undef LOAD
#undef CLEANUP
#define CLEANUP(...)
#define LOAD(...)
#define DELETING(...)
#define QUERY(...)
#define MODULE(...)
#define CACHED(at, flag, ref, obj, id, cache, ...) {at, flag, ref, obj, id, cache},
static const uint32_t cached_table[][6] = {
#include "generated.inc"
};

namespace rewrite {
const uint32_t *cached_constants(uintptr_t at) {
    for (auto &c : cached_table)
        if (c[0] == at) return c;
    return nullptr;
}
}

// the global clean-ups' globals, for the equivalence test
#undef LOAD
#undef CLEANUP
#undef CACHED
#define CACHED(...)
#define LOAD(...)
#define CLEANUP(at, global, ...) {at, global},
static const uint32_t cleanup_table[][2] = {
#include "generated.inc"
};

namespace rewrite {
uint32_t cleanup_global(uintptr_t at) {
    for (auto &c : cleanup_table)
        if (c[0] == at) return c[1];
    return 0;
}
}
#undef CLEANUP
#define CLEANUP(...)

// the struct-array loads' constants, for the equivalence test
#undef CACHED
#undef LOAD
#define CACHED(...)
#define LOAD(at, stride, resize, ...) {at, stride, resize},
static const uint32_t load_table[][3] = {
#include "generated.inc"
};

namespace rewrite {
const uint32_t *load_constants(uintptr_t at) {
    for (auto &c : load_table)
        if (c[0] == at) return c;
    return nullptr;
}
}
#undef LOAD
#define LOAD(...)

// the start-up initialisers' and the text families' constants, for the equivalence test: {at, then the macro's constants (up to 6)}
#undef START_NAME
#undef START_CLASS
#undef START_SECTION
#define START_NAME(at, text, global, ...) {at, text, global},
#define START_CLASS(at, name, factory, global, ...) {at, name, factory, global},
#define START_SECTION(at, text, kind, global, dtor, ...) {at, text, kind, global, dtor},
#undef NAMED_TEXT_REF
#undef GAME_TEXT_COPY
#define NAMED_TEXT_REF(at, suffix, table, class_table, ...) {at, suffix, table, class_table},
#define GAME_TEXT_COPY(at, name, ...) {at, name},
#undef TEXT_REF_FROM_TEXT
#undef START_KEY
#define TEXT_REF_FROM_TEXT(at, suffix, table, class_table, ...) {at, suffix, table, class_table},
#define START_KEY(at, key2, key1, name, setting, global, dtor, ...) {at, key2, key1, name, setting, global, dtor},
static const uint32_t start_table[][7] = {
#include "generated.inc"
};

namespace rewrite {
const uint32_t *start_constants(uintptr_t at) {
    for (auto &c : start_table)
        if (c[0] == at) return c;
    return nullptr;
}
}

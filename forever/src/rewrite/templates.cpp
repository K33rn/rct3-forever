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
// A key command defined at start-up (0x6B2720 on its global: its key group, the setting's name and the command's,
// its default keys made with KeyCombination::Set (0xBD1940, up to three keys) on the stack, then two flags); the
// global's destructor registered with atexit. The copies differ in how many keys, the group and the flags, and in
// whether the keys passed on are what KeyCombination::Set gave or the buffer itself (the same in the game).
static volatile long g_key_commands;
template <uint32_t KEY1, uint32_t KEY2, uint32_t KEY3, uint32_t GROUP, uint32_t SETTING, uint32_t NAME, uint32_t FLAG1,
          uint32_t FLAG2, uint32_t GLOBAL, uint32_t DTOR, bool GIVEN_KEYS>
static void __cdecl key_command() {
    g_key_commands++;
    alignas(4) uint8_t keys[0x14];
    void *k = ((void *(__fastcall *)(void *, void *, uint32_t, uint32_t, uint32_t, uint32_t))0xbd1940)(
        keys, nullptr, KEY1, KEY2, KEY3, 0);
    ((void(__fastcall *)(void *, void *, uint32_t, uint32_t, uint32_t, void *, uint32_t, uint32_t))0x6b2720)(
        (void *)GLOBAL, nullptr, GROUP, SETTING, NAME, GIVEN_KEYS ? k : keys, FLAG1, FLAG2);
    game_atexit((void(__cdecl *)())DTOR);
}

// ---- small text and table helpers ---------------------------------------------------------------------------
// Text::Widen: C text (edi) into a wide-character buffer (edx) of a given size (ecx), stopping at the text's end or
// the buffer's last place, and ended with 0; returns how many characters were copied. Its arguments come in
// registers: a wrapper passes them on, keeping edx as the original does (esi, edi and ebx are kept by the C
// function itself). A size of 0 is taken as no limit, as in the original.
static volatile long g_widens;
extern "C" uint32_t __fastcall forever_text_widen(uint32_t size, uint16_t *buffer, const uint8_t *text) {
    g_widens++;
    uint32_t n = 0, last = size - 1;
    if (last != 0)
        while (text[n]) {
            buffer[n] = text[n];
            if (++n >= last) break;
        }
    buffer[n] = 0;
    return n;
}
extern "C" __attribute__((naked)) void forever_text_widen_regs() {
    __asm__("pushl %edx\n\tpushl %edi\n\tcall @forever_text_widen@12\n\tpopl %edx\n\tret");
}

// A new text from C text (MAKE: WideString::Create 0xBCF850 or String::FromC 0xBCFB10) in memory of its own, given
// in `out`; null when there was no memory. Returns `out`.
static volatile long g_text_creates;
template <uint32_t TEXT, uint32_t MAKE> static void **__stdcall text_create(void **out) {
    g_text_creates++;
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
    *out = p ? ((void *(__fastcall *)(void *, void *, const char *))MAKE)(p, nullptr, (const char *)TEXT) : nullptr;
    return out;
}

// Table20::Free: a table {entries, count} of 20-byte entries {?, pointers (+4), their count (+8), block (+0xC)}:
// frees every entry's blocks, its array of them and its block (all aligned), then the entries, and clears them.
static volatile long g_table20_frees;
static void __fastcall table20_free(uint8_t **table) {
    g_table20_frees++;
    void (*const free_aligned)(void *) = (void (*)(void *))0xbce010;
    for (int32_t i = 0; i < *(int32_t *)(table + 1); i++) {
        uint8_t *e = *table + i * 0x14;
        for (int32_t j = 0; j < *(int32_t *)(e + 8); j++) free_aligned((*(void ***)(e + 4))[j]);
        free_aligned(*(void **)(e + 4));
        free_aligned(*(void **)(e + 0xc));
    }
    if (*table) {
        free_aligned(*table);
        *table = nullptr;
    }
}

#define TEXT_WIDEN(at, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Text::Widen", at, REWRITE_CAT(original_, at), 8, (void *)&forever_text_widen_regs, &g_widens);
#define TEXT_CREATE(at, text, make, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Text::Create", at, REWRITE_CAT(original_, at), 8, (void *)&text_create<text, make>, &g_text_creates);
#define TABLE20_FREE(at, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Table20::Free", at, REWRITE_CAT(original_, at), 8, (void *)&table20_free, &g_table20_frees);
// A text reference in a global, made at start-up: the wide text TEXT (0xBCF850) with SUFFIX added (0x403BE0),
// copied (0xBD01C0) and looked up as a NamedTextRef (0x410C90, in the texts table 0x1303CFC, with its suffix); the
// three texts are let go of after, and the global's destructor registered with atexit.
static volatile long g_start_texts;
template <uint32_t TEXT, uint32_t SUFFIX, uint32_t GLOBAL, uint32_t DTOR> static void __cdecl start_text() {
    g_start_texts++;
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
    Text volatile text = p ? ((Text(__fastcall *)(void *, void *, const char *))0xbcf850)(p, nullptr, (const char *)TEXT)
                           : nullptr;
    Text volatile with_suffix, copy;
    Text volatile *r = ((Text volatile * (__fastcall *)(Text volatile *, void *, Text volatile *, const char *))
                            0x403be0)(&text, nullptr, &with_suffix, (const char *)SUFFIX);
    ((void(__fastcall *)(Text volatile *, void *, Text volatile *))0xbd01c0)(&copy, nullptr, r);
    ((void(__fastcall *)(void *, void *, Text volatile *, void *, uint32_t))0x410c90)((void *)GLOBAL, nullptr, &copy,
                                                                                    (void *)0x1303cfc, 1);
    release_text(copy);
    release_text(with_suffix);
    release_text(text);
    game_atexit((void(__cdecl *)())DTOR);
}
#define START_TEXT(at, text, suffix, global, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::Text", at, REWRITE_CAT(original_, at), 8, (void *)&start_text<text, suffix, global, dtor>, &g_start_texts);
// ---- resource types -----------------------------------------------------------------------------------------
// The constructor of a resource type (a loader's description of the files it reads): its name, file extension and
// engine as wide texts made from C text (0xBCF850), passed to the base constructor (0xC2FFB0: the engine, the
// extension, a flag (1 or 2), the name), let go of after; then the class's own table.
static volatile long g_resource_types;
static Text wide_text(const char *c) {
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
    return p ? ((Text(__fastcall *)(void *, void *, const char *))0xbcf850)(p, nullptr, c) : nullptr;
}
template <uint32_t NAME, uint32_t EXTENSION, uint32_t ENGINE, uint32_t TABLE, uint32_t FLAG>
static void *__fastcall resource_type(uint32_t *self) {
    g_resource_types++;
    Text volatile name = wide_text((const char *)NAME);
    Text volatile extension = wide_text((const char *)EXTENSION);
    Text volatile engine = wide_text((const char *)ENGINE);
    ((void(__fastcall *)(void *, void *, Text volatile *, Text volatile *, uint32_t, Text volatile *))0xc2ffb0)(
        self, nullptr, &engine, &extension, FLAG, &name);
    release_text(engine);
    release_text(extension);
    release_text(name);
    self[0] = TABLE;
    return self;
}
#define RESOURCE_TYPE(at, name, extension, engine, table, flag, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("ResourceType::ResourceType", at, REWRITE_CAT(original_, at), 8, (void *)&resource_type<name, extension, engine, table, flag>, &g_resource_types);
// QueryInterface that reports a miss: the interface (id in a global) of the object, as QueryInterface; when the
// object has none, the source file (edx) and line (on the stack) are reported (0xBEC8A0) unless no file was given.
// For no object at all it gives null without a report.
static volatile long g_query_reports;
template <uint32_t ID> static void *__fastcall query_or_report(void *object, const char *file, int32_t line) {
    g_query_reports++;
    if (!object) return nullptr;
    ClassInterfaces *c = ((ClassInterfaces * (__fastcall *)(void *))(*(void ***)object)[1])(object);
    if (c) {
        int32_t id = *(int32_t *)ID;
        if (id < c->count && c->by_id[id]) return (uint8_t *)object + c->by_id[id]->offset;
    }
    if (file) ((void(__fastcall *)(const char *, int32_t))0xbec8a0)(file, line);
    return nullptr;
}
#define QUERY_REPORT(at, id, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("QueryInterface::OrReport", at, REWRITE_CAT(original_, at), 8, (void *)&query_or_report<id>, &g_query_reports);
// ---- wide texts from globals -------------------------------------------------------------------------------
// A new wide string copied from a wide C text (0xBCF7D0, in memory of its own; null when there was none), or the
// empty one (0xBD0490) when there is no text. Kept in the global itself, or at +8 of the object the global points
// to (the original reads it without checking the object, and so does this); given in `out`.
static volatile long g_wide_copies;
static Text wide_copy(const wchar_t *text) {
    if (!text) return ((Text(__fastcall *)(char))0xbd0490)(1);
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
    return p ? ((Text(__fastcall *)(void *, void *, const wchar_t *))0xbcf7d0)(p, nullptr, text) : nullptr;
}
template <uint32_t GLOBAL> static Text *__stdcall wide_copy_of_object(Text *out) {
    g_wide_copies++;
    *out = wide_copy(*(const wchar_t **)(*(uint8_t **)GLOBAL + 8));
    return out;
}
template <uint32_t GLOBAL> static Text *__stdcall wide_copy_of_global(Text *out) {
    g_wide_copies++;
    *out = wide_copy(*(const wchar_t **)GLOBAL);
    return out;
}
template <uint32_t GLOBAL> static Text *__fastcall wide_copy_of_global_this(Text *out) {
    g_wide_copies++;
    *out = wide_copy(*(const wchar_t **)GLOBAL);
    return out;
}

// A scenery behaviour registered at start-up (0x833F90 on its global, with its name as a wide text and its
// factory); the name let go of after, the global's destructor registered with atexit. With no memory for the name
// the original crashes on the null text, and so does this.
static volatile long g_behaviours;
template <uint32_t NAME, uint32_t FACTORY, uint32_t GLOBAL, uint32_t DTOR> static void __cdecl start_behaviour() {
    g_behaviours++;
    Text volatile name = wide_text((const char *)NAME);
    ((void(__fastcall *)(void *, void *, Text volatile *, void *))0x833f90)((void *)GLOBAL, nullptr, &name,
                                                                           (void *)FACTORY);
    release_text(name);
    game_atexit((void(__cdecl *)())DTOR);
}
#define WIDE_COPY_OBJECT(at, global, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("WideText::CopyOf(object)", at, REWRITE_CAT(original_, at), 8, (void *)&wide_copy_of_object<global>, &g_wide_copies);
#define WIDE_COPY_GLOBAL(at, global, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("WideText::CopyOf", at, REWRITE_CAT(original_, at), 8, (void *)&wide_copy_of_global<global>, &g_wide_copies);
#define WIDE_COPY_THIS(at, global, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("WideText::CopyOf(this)", at, REWRITE_CAT(original_, at), 8, (void *)&wide_copy_of_global_this<global>, &g_wide_copies);
#define START_BEHAVIOUR(at, name, factory, global, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::Behaviour", at, REWRITE_CAT(original_, at), 8, (void *)&start_behaviour<name, factory, global, dtor>, &g_behaviours);
// ---- factories ------------------------------------------------------------------------------------------------
// Create: a new object of SIZE bytes (0xBCDFC0) constructed by CTOR, or null when there was no memory. Either
// from the two arguments (the constructor given 0 and them), or from two fields of `this` (given 0, 0 and them).
static volatile long g_factories;
template <uint32_t SIZE, uint32_t CTOR> static void *__stdcall factory2(uint32_t a, uint32_t b) {
    g_factories++;
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
    return p ? ((void *(__fastcall *)(void *, void *, uint32_t, uint32_t, uint32_t))CTOR)(p, nullptr, 0, a, b) : nullptr;
}
template <uint32_t SIZE, uint32_t FIELD1, uint32_t FIELD2, uint32_t CTOR>
static void *__fastcall factory_fields(uint8_t *self) {
    g_factories++;
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
    return p ? ((void *(__fastcall *)(void *, void *, uint32_t, uint32_t, uint32_t, uint32_t))CTOR)(
                   p, nullptr, 0, 0, *(uint32_t *)(self + FIELD1), *(uint32_t *)(self + FIELD2))
             : nullptr;
}
#define FACTORY2(at, size, ctor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Factory::Create", at, REWRITE_CAT(original_, at), 8, (void *)&factory2<size, ctor>, &g_factories);
#define FACTORY_FIELDS(at, size, field1, field2, ctor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Factory::CreateFromFields", at, REWRITE_CAT(original_, at), 8, (void *)&factory_fields<size, field1, field2, ctor>, &g_factories);
// ---- arrays of objects ------------------------------------------------------------------------------------------
// ObjectArray::SetCapacity: an array {items, count} of SIZE-byte objects made with new[] (the count at the start of a
// HEADER of 4 or 16 bytes in front of them) resized to n: the new items allocated (aligned) and constructed (the runtime's vector constructor iterator,
// with CTOR and DTOR), the old ones that fit assigned over (ASSIGN on each new item, given the old one), the old
// items destroyed and freed; gives n. No memory leaves the items null (the original then assigns into it too).
static volatile long g_object_setcaps;
template <uint32_t SIZE, uint32_t HEADER, uint32_t CTOR, uint32_t DTOR, uint32_t ASSIGN>
static int32_t __fastcall object_set_capacity(uint8_t **array, void *, int32_t n) {
    g_object_setcaps++;
    int32_t count = ((int32_t *)array)[1];
    if (n == count) return count;
    uint8_t *items = nullptr;
    if (n != 0) {
        int32_t *block = ((int32_t * (__cdecl *)(uint32_t))0xbcdfd0)((uint32_t)n * SIZE + HEADER);
        if (block) {
            block[0] = n;
            items = (uint8_t *)block + HEADER;
            ((void(__stdcall *)(void *, uint32_t, int32_t, void *, void *))0xddcf99)(items, SIZE, n, (void *)CTOR,
                                                                                   (void *)DTOR);
        }
        int32_t keep = n > count ? count : n;
        for (int32_t i = 0; i < keep; i++)
            ((void(__fastcall *)(void *, void *, void *))ASSIGN)(items + i * SIZE, nullptr, *array + i * SIZE);
    }
    if (*array) {
        ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0xddcf39)(*array, SIZE, *(int32_t *)(*array - HEADER),
                                                                       (void *)DTOR);
        ((void(__cdecl *)(void *))0xbce010)(*array - HEADER);
        *array = nullptr;
    }
    ((int32_t *)array)[1] = n;
    *array = items;
    return n;
}
#define OBJECT_SETCAP(at, size, header, ctor, dtor, assign, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("ObjectArray::SetCapacity", at, REWRITE_CAT(original_, at), 8, (void *)&object_set_capacity<size, header, ctor, dtor, assign>, &g_object_setcaps);
// A global object made by a factory: SIZE bytes constructed by CTOR (or null without memory), kept in GLOBAL and
// given back; the lazy ones only when the global is still empty, and give what it holds. One kind also gives its
// constructor an argument, then sets its table and a byte field.
static volatile long g_global_factories;
template <uint32_t SIZE, uint32_t CTOR, uint32_t GLOBAL> static void *__cdecl global_factory() {
    g_global_factories++;
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
    void *made = p ? ((void *(__fastcall *)(void *))CTOR)(p) : nullptr;
    *(void **)GLOBAL = made;
    return made;
}
template <uint32_t GLOBAL, uint32_t SIZE, uint32_t CTOR> static void *__cdecl lazy_factory() {
    g_global_factories++;
    void *made = *(void **)GLOBAL;
    if (!made) {
        void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
        made = p ? ((void *(__fastcall *)(void *))CTOR)(p) : nullptr;
        *(void **)GLOBAL = made;
    }
    return made;
}
template <uint32_t GLOBAL, uint32_t SIZE, uint32_t ARG, uint32_t CTOR, uint32_t TABLE, uint32_t FIELD, uint32_t VALUE>
static void *__cdecl lazy_factory_init() {
    g_global_factories++;
    uint32_t *made = *(uint32_t **)GLOBAL;
    if (!made) {
        made = (uint32_t *)((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
        if (made) {
            ((void(__fastcall *)(void *, void *, uint32_t))CTOR)(made, nullptr, ARG);
            made[0] = TABLE;
            ((uint8_t *)made)[FIELD] = (uint8_t)VALUE;
        }
        *(uint32_t **)GLOBAL = made;
    }
    return made;
}

// A destructor with nothing of its own to undo: its two tables put back, the member at +4 destroyed (DTOR), then the
// base class's table.
static volatile long g_dtor_tables;
template <uint32_t TABLE, uint32_t MEMBER_TABLE, uint32_t DTOR, uint32_t BASE_TABLE>
static void __fastcall dtor_tables(uint32_t *self) {
    g_dtor_tables++;
    self[0] = TABLE;
    self[1] = MEMBER_TABLE;
    ((void(__fastcall *)(void *))DTOR)(self + 1);
    self[0] = BASE_TABLE;
}

// Frees two aligned blocks the object owns (the one at FIELD first, then the one at +0), clearing the pointers.
static volatile long g_free_twos;
template <uint32_t FIELD> static void __fastcall free_two(uint8_t *self) {
    g_free_twos++;
    void (*const free_aligned)(void *) = (void (*)(void *))0xbce010;
    if (*(void **)(self + FIELD)) {
        free_aligned(*(void **)(self + FIELD));
        *(void **)(self + FIELD) = nullptr;
    }
    if (*(void **)self) {
        free_aligned(*(void **)self);
        *(void **)self = nullptr;
    }
}
// ObjectArray::Resize: the array's old items destroyed and freed, then n new ones allocated and constructed (none
// for n = 0 or without memory); gives n.
static volatile long g_object_resizes;
template <uint32_t SIZE, uint32_t HEADER, uint32_t CTOR, uint32_t DTOR>
static int32_t __fastcall object_resize(uint8_t **array, void *, int32_t n) {
    g_object_resizes++;
    if (*array) {
        ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0xddcf39)(*array, SIZE, *(int32_t *)(*array - HEADER),
                                                                       (void *)DTOR);
        ((void(__cdecl *)(void *))0xbce010)(*array - HEADER);
        *array = nullptr;
    }
    if (n != 0) {
        int32_t *block = ((int32_t * (__cdecl *)(uint32_t))0xbcdfd0)((uint32_t)n * SIZE + HEADER);
        uint8_t *items = nullptr;
        if (block) {
            block[0] = n;
            items = (uint8_t *)block + HEADER;
            ((void(__stdcall *)(void *, uint32_t, int32_t, void *, void *))0xddcf99)(items, SIZE, n, (void *)CTOR,
                                                                                   (void *)DTOR);
        }
        *array = items;
    }
    ((int32_t *)array)[1] = n;
    return n;
}

// Calls FUNCTION (on the part at FIELD) with a temporary reference to the given object: a reference (class TABLE,
// built on the reference base 0xF26AE0) joined into the object's handle's list, then destroyed (DTOR); gives what
// the destructor gives.
static volatile long g_with_references;
template <uint32_t TABLE, uint32_t FIELD, uint32_t FUNCTION, uint32_t DTOR>
static uint32_t __fastcall with_reference(uint8_t *self, void *, void *object) {
    g_with_references++;
    uint32_t ref[7];
    ref[0] = 0xf26ae0;
    ref[4] = (uint32_t)(uintptr_t)ref;
    join_list(ref, ((uint8_t * (__fastcall *)(void *))0x805700)(object));
    ref[0] = TABLE;
    ref[5] = (uint32_t)(uintptr_t)object;
    ((uint8_t *)ref)[0x18] = 1;
    ((void(__fastcall *)(void *, void *, void *))FUNCTION)(self + FIELD, nullptr, ref);
    return ((uint32_t(__fastcall *)(void *))DTOR)(ref);
}
#define OBJECT_RESIZE(at, size, header, ctor, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("ObjectArray::Resize", at, REWRITE_CAT(original_, at), 8, (void *)&object_resize<size, header, ctor, dtor>, &g_object_resizes);
#define WITH_REFERENCE(at, table, field, function, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("ObjectRef::CallWith", at, REWRITE_CAT(original_, at), 8, (void *)&with_reference<table, field, function, dtor>, &g_with_references);
// ---- growable arrays of 4-byte values {count, capacity, growth step, data} -------------------------------------
// PointerArray::Resize: to the capacity n (unless it already is): the values that fit copied into a new aligned
// block (none for n = 0), the old block freed (even when there is none), the count cut to n.
static volatile long g_pointer_resizes;
static uint32_t __fastcall pointer_resize(uint32_t *self, void *, uint32_t n) {
    g_pointer_resizes++;
    if (n == self[1]) return n;   // (the original leaves eax as it was; its callers don't use it)
    uint32_t *values = n ? ((uint32_t * (__cdecl *)(uint32_t))0xbcdfd0)(n * 4) : nullptr;
    uint32_t keep = self[0] > n ? n : self[0];
    for (uint32_t i = 0; i < keep; i++) values[i] = ((uint32_t *)(uintptr_t)self[3])[i];
    ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)self[3]);
    keep = self[0] > n ? n : self[0];
    self[3] = (uint32_t)(uintptr_t)values;
    self[0] = keep;
    self[1] = n;
    return keep;
}
// Array::Reserve: room for at least n values: when the capacity is smaller, it grows from the capacity (at least 1)
// by the growth step (or doubles when there is none) until n fits, then the array's own resize (RESIZE) is called.
// Gives the capacity when there is room already.
static volatile long g_array_reserves;
template <uint32_t RESIZE> static uint32_t __fastcall array_reserve(uint32_t *self, void *, uint32_t n) {
    g_array_reserves++;
    uint32_t capacity = self[1];
    if (n <= capacity) return capacity;
    uint32_t grown = capacity > 1 ? capacity : 1;
    while (n > grown) grown += self[2] ? self[2] : grown;
    return ((uint32_t(__fastcall *)(void *, void *, uint32_t))RESIZE)(self, nullptr, grown);
}

// ---- interface parts -----------------------------------------------------------------------------------------
// Interface parts built on 0x6D7450 (given their owner, two values and 1), then their three tables. The owner is
// the given one or, for none, the shared one (in the global MANAGER); an owner not yet active is activated (its
// slot 1), and the shared one is then marked active. Made in place (Init), or new: SIZE bytes, with the shared
// owner and 0, 0 (Make) or the two arguments (Create); null without memory.
static volatile long g_gui_parts;
template <uint32_t T0, uint32_t T1, uint32_t T40> static void gui_tables(uint32_t *self) {
    self[0] = T0;
    self[1] = T1;
    self[0x10] = T40;
}
static void gui_activate(uint8_t *owner) {
    if (!owner[4]) ((void(__fastcall *)(void *))(*(void ***)owner)[1])(owner);
}
typedef void(__fastcall *GuiBase)(void *, void *, void *, uint32_t, uint32_t, uint32_t);   // 0x6D7450, ret 0x10
template <uint32_t MANAGER, uint32_t T0, uint32_t T1, uint32_t T40>
static void *__fastcall gui_init(uint32_t *self, void *, uint8_t *owner, uint32_t a, uint32_t b) {
    g_gui_parts++;
    ((GuiBase)0x6d7450)(self, nullptr, owner ? owner : *(uint8_t **)MANAGER, a, b, 1);
    gui_tables<T0, T1, T40>(self);
    bool shared = !owner;
    if (shared) owner = *(uint8_t **)MANAGER;
    gui_activate(owner);
    if (shared) (*(uint8_t **)MANAGER)[4] = 1;
    return self;
}
template <uint32_t SIZE, uint32_t MANAGER, uint32_t T0, uint32_t T1, uint32_t T40>
static void *gui_new(uint32_t a, uint32_t b) {
    uint32_t *self = (uint32_t *)((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
    if (!self) return nullptr;
    ((GuiBase)0x6d7450)(self, nullptr, *(void **)MANAGER, a, b, 1);
    gui_tables<T0, T1, T40>(self);
    gui_activate(*(uint8_t **)MANAGER);
    (*(uint8_t **)MANAGER)[4] = 1;
    return self;
}
template <uint32_t SIZE, uint32_t MANAGER, uint32_t T0, uint32_t T1, uint32_t T40> static void *__cdecl gui_make() {
    g_gui_parts++;
    return gui_new<SIZE, MANAGER, T0, T1, T40>(0, 0);
}
template <uint32_t SIZE, uint32_t MANAGER, uint32_t T0, uint32_t T1, uint32_t T40>
static void *__stdcall gui_create2(uint32_t a, uint32_t b) {
    g_gui_parts++;
    return gui_new<SIZE, MANAGER, T0, T1, T40>(a, b);
}
#define POINTER_RESIZE(at, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("PointerArray::Resize", at, REWRITE_CAT(original_, at), 8, (void *)&pointer_resize, &g_pointer_resizes);
#define ARRAY_RESERVE(at, resize, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Array::Reserve", at, REWRITE_CAT(original_, at), 8, (void *)&array_reserve<resize>, &g_array_reserves);
#define GUI_INIT(at, manager, t0, t1, t40, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("GuiPart::Init", at, REWRITE_CAT(original_, at), 8, (void *)&gui_init<manager, t0, t1, t40>, &g_gui_parts);
#define GUI_MAKE(at, size, manager, t0, t1, t40, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("GuiPart::Make", at, REWRITE_CAT(original_, at), 8, (void *)&gui_make<size, manager, t0, t1, t40>, &g_gui_parts);
#define GUI_CREATE2(at, size, manager, t0, t1, t40, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("GuiPart::Create", at, REWRITE_CAT(original_, at), 8, (void *)&gui_create2<size, manager, t0, t1, t40>, &g_gui_parts);
#define GLOBAL_FACTORY(at, size, ctor, global, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Factory::Global", at, REWRITE_CAT(original_, at), 8, (void *)&global_factory<size, ctor, global>, &g_global_factories);
#define LAZY_FACTORY(at, global, size, ctor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Factory::Lazy", at, REWRITE_CAT(original_, at), 8, (void *)&lazy_factory<global, size, ctor>, &g_global_factories);
#define LAZY_FACTORY_INIT(at, global, size, arg, ctor, table, field, value, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Factory::LazyInit", at, REWRITE_CAT(original_, at), 8, (void *)&lazy_factory_init<global, size, arg, ctor, table, field, value>, &g_global_factories);
#define DTOR_TABLES(at, table, member_table, dtor, base_table, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Destructor::Tables", at, REWRITE_CAT(original_, at), 8, (void *)&dtor_tables<table, member_table, dtor, base_table>, &g_dtor_tables);
#define FREE_TWO(at, field, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("OwnedPair::FreeFields", at, REWRITE_CAT(original_, at), 8, (void *)&free_two<field>, &g_free_twos);
#define TEXT_REF_FROM_TEXT(at, suffix, table, class_table, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("NamedTextRef::NamedTextRef(text)", at, REWRITE_CAT(original_, at), 8, (void *)&named_text_ref_from_text<suffix, table, class_table>, &g_text_refs_from_text);
#define START_KEY(at, key2, key1, name, setting, global, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::KeyCommand", at, REWRITE_CAT(original_, at), 8, (void *)&key_command<key1, key2, 0, 0xf4473c, setting, name, 1, 1, global, dtor, true>, &g_key_commands);
#define START_KEY3(at, key3, key2, key1, name, setting, global, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::KeyCommand", at, REWRITE_CAT(original_, at), 8, (void *)&key_command<key1, key2, key3, 0xf4473c, setting, name, 1, 1, global, dtor, true>, &g_key_commands);
#define START_KEY1(at, key1, name, setting, group, global, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::KeyCommand", at, REWRITE_CAT(original_, at), 8, (void *)&key_command<key1, 0, 0, group, setting, name, 1, 1, global, dtor, false>, &g_key_commands);
#define START_KEY1_FLAG(at, key1, name, setting, group, global, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::KeyCommand", at, REWRITE_CAT(original_, at), 8, (void *)&key_command<key1, 0, 0, group, setting, name, 1, 0, global, dtor, false>, &g_key_commands);
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
#undef START_KEY3
#undef START_KEY1
#undef START_KEY1_FLAG
#define START_KEY3(...)
#define START_KEY1(...)
#define START_KEY1_FLAG(...)
#undef TEXT_WIDEN
#undef TEXT_CREATE
#undef TABLE20_FREE
#define TEXT_WIDEN(...)
#define TEXT_CREATE(...)
#define TABLE20_FREE(...)
#undef START_TEXT
#define START_TEXT(...)
#undef RESOURCE_TYPE
#define RESOURCE_TYPE(...)
#undef QUERY_REPORT
#define QUERY_REPORT(...)
#undef WIDE_COPY_OBJECT
#undef WIDE_COPY_GLOBAL
#undef WIDE_COPY_THIS
#undef START_BEHAVIOUR
#define WIDE_COPY_OBJECT(...)
#define WIDE_COPY_GLOBAL(...)
#define WIDE_COPY_THIS(...)
#define START_BEHAVIOUR(...)
#undef FACTORY2
#undef FACTORY_FIELDS
#define FACTORY2(...)
#define FACTORY_FIELDS(...)
#undef OBJECT_SETCAP
#define OBJECT_SETCAP(...)
#undef GLOBAL_FACTORY
#undef LAZY_FACTORY
#undef LAZY_FACTORY_INIT
#undef DTOR_TABLES
#undef FREE_TWO
#define GLOBAL_FACTORY(...)
#undef POINTER_RESIZE
#undef ARRAY_RESERVE
#undef GUI_INIT
#undef GUI_MAKE
#undef GUI_CREATE2
#define POINTER_RESIZE(...)
#define ARRAY_RESERVE(...)
#define GUI_INIT(...)
#define GUI_MAKE(...)
#define GUI_CREATE2(...)
#undef OBJECT_RESIZE
#undef WITH_REFERENCE
#define OBJECT_RESIZE(...)
#define WITH_REFERENCE(...)
#define LAZY_FACTORY(...)
#define LAZY_FACTORY_INIT(...)
#define DTOR_TABLES(...)
#define FREE_TWO(...)
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
#undef TEXT_CREATE
#define TEXT_CREATE(at, text, make, ...) {at, text, make},
#undef START_TEXT
#define START_TEXT(at, text, suffix, global, dtor, ...) {at, text, suffix, global, dtor},
#undef QUERY_REPORT
#define QUERY_REPORT(at, id, ...) {at, id},
#undef WIDE_COPY_OBJECT
#undef WIDE_COPY_GLOBAL
#undef WIDE_COPY_THIS
#undef START_BEHAVIOUR
#define WIDE_COPY_OBJECT(at, global, ...) {at, global},
#define WIDE_COPY_GLOBAL(at, global, ...) {at, global},
#define WIDE_COPY_THIS(at, global, ...) {at, global},
#define START_BEHAVIOUR(at, name, factory, global, dtor, ...) {at, name, factory, global, dtor},
#undef FACTORY2
#undef FACTORY_FIELDS
#define FACTORY2(at, size, ctor, ...) {at, size, ctor},
#define FACTORY_FIELDS(at, size, field1, field2, ctor, ...) {at, size, field1, field2, ctor},
#undef OBJECT_SETCAP
#define OBJECT_SETCAP(at, size, header, ctor, dtor, assign, ...) {at, size, header, ctor, dtor, assign},
#undef GLOBAL_FACTORY
#undef LAZY_FACTORY
#undef LAZY_FACTORY_INIT
#undef DTOR_TABLES
#undef FREE_TWO
#define GLOBAL_FACTORY(at, size, ctor, global, ...) {at, size, ctor, global},
#define LAZY_FACTORY(at, global, size, ctor, ...) {at, global, size, ctor},
#define LAZY_FACTORY_INIT(at, global, size, arg, ctor, table, field, ...) {at, global, size, arg, ctor, table, field},
#define DTOR_TABLES(at, table, member_table, dtor, base_table, ...) {at, table, member_table, dtor, base_table},
#define FREE_TWO(at, field, ...) {at, field},
#undef POINTER_RESIZE
#undef ARRAY_RESERVE
#undef GUI_INIT
#undef GUI_MAKE
#undef GUI_CREATE2
#define POINTER_RESIZE(...)
#define ARRAY_RESERVE(at, resize, ...) {at, resize},
#define GUI_INIT(at, manager, ...) {at, manager},
#define GUI_MAKE(at, size, manager, ...) {at, size, manager},
#define GUI_CREATE2(at, size, manager, ...) {at, size, manager},
#undef OBJECT_RESIZE
#undef WITH_REFERENCE
#define OBJECT_RESIZE(at, size, header, ctor, dtor, ...) {at, size, header, ctor, dtor},
#define WITH_REFERENCE(at, table, field, function, dtor, ...) {at, table, field, function, dtor},
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

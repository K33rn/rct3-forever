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

template <uint32_t SIZE, uint32_t DTOR, uint32_t HEADER = 4> static void __fastcall object_array_free(uint8_t **array) {
    g_array_frees++;
    uint8_t *items = *array;
    if (!items) return;
    ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0xddcf39)(items, SIZE, *(int32_t *)(items - HEADER),
                                                                     (void *)DTOR);
    ((void(__cdecl *)(void *))0xbce010)(items - HEADER);
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

// The same, with one or two extra arguments it ignores (they change only how many bytes it clears from the stack).
template <uint32_t TEXT, uint32_t MAKE> static void **__stdcall text_create2(void **out, uint32_t) {
    return text_create<TEXT, MAKE>(out);
}
template <uint32_t TEXT, uint32_t MAKE> static void **__stdcall text_create3(void **out, uint32_t, uint32_t) {
    return text_create<TEXT, MAKE>(out);
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
#define TEXT_CREATE2(at, text, make, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Text::Create(2args)", at, REWRITE_CAT(original_, at), 8, (void *)&text_create2<text, make>, &g_text_creates);
#define TEXT_CREATE3(at, text, make, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Text::Create(3args)", at, REWRITE_CAT(original_, at), 8, (void *)&text_create3<text, make>, &g_text_creates);
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
// The name of an objective condition: the game text found by the condition's name (a wide text, TEXT) after the
// prefix 0xF65630 (0x403CE0), copied (0xBD01C0) and looked up as a NamedTextRef(text) in the texts table 0x1303CFC;
// its text copied into `out` as a wide string of its own, or the empty one. The texts made are let go of after.
static volatile long g_condition_names;
template <uint32_t TEXT> static Text *__stdcall condition_name(Text *out) {
    g_condition_names++;
    Text made = wide_text((const char *)TEXT);
    Text volatile name = made, joined, copy;
    Text volatile *r = ((Text volatile * (__fastcall *)(Text volatile *, const char *, Text volatile *))0x403ce0)(
        &joined, (const char *)0xf65630, &name);
    ((void(__fastcall *)(Text volatile *, void *, Text volatile *))0xbd01c0)(&copy, nullptr, r);
    alignas(4) uint8_t ref[0x20];
    uint8_t *found = ((uint8_t * (__fastcall *)(void *, void *, Text volatile *, void *, uint32_t))0x410c90)(
        ref, nullptr, &copy, (void *)0x1303cfc, 1);
    *out = wide_copy(*(const wchar_t **)(found + 8));
    ((void(__fastcall *)(void *))0xbd1880)(ref);
    release_text(copy);
    release_text(joined);
    release_text(made);
    return out;
}
#define CONDITION_NAME(at, text, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("ObjectiveCondition::Name", at, REWRITE_CAT(original_, at), 8, (void *)&condition_name<text>, &g_condition_names);
// More factories: SIZE bytes constructed with constant arguments (given back, kept in a global, or kept in a field
// of `this`, the constructor then also given `this`); null without memory.
template <uint32_t SIZE, uint32_t CTOR, uint32_t A, uint32_t B, uint32_t C> static void *__cdecl factory_k3() {
    g_factories++;
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
    return p ? ((void *(__fastcall *)(void *, void *, uint32_t, uint32_t, uint32_t))CTOR)(p, nullptr, A, B, C) : nullptr;
}
template <uint32_t SIZE, uint32_t CTOR, uint32_t A, uint32_t GLOBAL> static void *__cdecl global_factory_k1() {
    g_global_factories++;
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
    void *made = p ? ((void *(__fastcall *)(void *, void *, uint32_t))CTOR)(p, nullptr, A) : nullptr;
    *(void **)GLOBAL = made;
    return made;
}
template <uint32_t SIZE, uint32_t CTOR, uint32_t A, uint32_t B, uint32_t FIELD>
static void *__fastcall field_factory(uint8_t *self) {
    g_factories++;
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
    void *made = p ? ((void *(__fastcall *)(void *, void *, uint32_t, uint32_t, void *))CTOR)(p, nullptr, A, B, self)
                   : nullptr;
    *(void **)(self + FIELD) = made;
    return made;
}
#define FACTORY_K3(at, size, ctor, a, b, c, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Factory::CreateWith", at, REWRITE_CAT(original_, at), 8, (void *)&factory_k3<size, ctor, a, b, c>, &g_factories);
#define GLOBAL_FACTORY_K1(at, size, ctor, a, global, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Factory::GlobalWith", at, REWRITE_CAT(original_, at), 8, (void *)&global_factory_k1<size, ctor, a, global>, &g_global_factories);
#define FIELD_FACTORY(at, size, ctor, a, b, field, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Factory::IntoField", at, REWRITE_CAT(original_, at), 8, (void *)&field_factory<size, ctor, a, b, field>, &g_factories);
// The interface (id in the global ID, as QueryInterface) of the object the reference at REF refers to (its
// handle's object at +0xC), or null.
static volatile long g_ref_interfaces;
template <uint32_t REF, uint32_t ID> static void *__fastcall ref_interface(uint8_t *self) {
    g_ref_interfaces++;
    void *object = *(void **)(*(uint8_t **)(self + REF) + 0xc);
    if (!object) return nullptr;
    ClassInterfaces *c = ((ClassInterfaces * (__fastcall *)(void *))(*(void ***)object)[1])(object);
    if (!c) return nullptr;
    int32_t id = *(int32_t *)ID;
    if (id >= c->count || !c->by_id[id]) return nullptr;
    return (uint8_t *)object + c->by_id[id]->offset;
}

// Destructors: tables put back, the object at MEMBER deleted (its deleting destructor, slot 0, with 1) if any, then
// the base class's destructor; or the member at MEMBER destroyed (MEMBER_DTOR), then the base class's destructor.
static volatile long g_member_dtors;
template <uint32_t TABLE, uint32_t TABLE4, uint32_t MEMBER, uint32_t BASE_DTOR>
static void __fastcall dtor_delete_member(uint32_t *self) {
    g_member_dtors++;
    self[0] = TABLE;
    self[1] = TABLE4;
    void *member = *(void **)((uint8_t *)self + MEMBER);
    if (member) ((void(__fastcall *)(void *, void *, uint32_t))(*(void ***)member)[0])(member, nullptr, 1);
    ((void(__fastcall *)(void *))BASE_DTOR)(self);
}
template <uint32_t MEMBER, uint32_t MEMBER_DTOR, uint32_t BASE_DTOR> static void __fastcall dtor_member_base(uint8_t *self) {
    g_member_dtors++;
    ((void(__fastcall *)(void *))MEMBER_DTOR)(self + MEMBER);
    ((void(__fastcall *)(void *))BASE_DTOR)(self);
}
#define REF_INTERFACE(at, ref, id, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Reference::Interface", at, REWRITE_CAT(original_, at), 8, (void *)&ref_interface<ref, id>, &g_ref_interfaces);
#define DTOR_DELETE_MEMBER(at, table, table4, member, base, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Destructor::DeleteMember", at, REWRITE_CAT(original_, at), 8, (void *)&dtor_delete_member<table, table4, member, base>, &g_member_dtors);
#define DTOR_MEMBER_BASE(at, member, member_dtor, base, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Destructor::MemberThenBase", at, REWRITE_CAT(original_, at), 8, (void *)&dtor_member_base<member, member_dtor, base>, &g_member_dtors);
// Factories giving their constructor constants (two or five), or a constant, the two arguments and a constant.
template <uint32_t SIZE, uint32_t CTOR, uint32_t... ARGS> static void *__cdecl factory_k() {
    g_factories++;
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
    typedef void *(__fastcall * Ctor)(void *, void *, decltype(ARGS)...);
    return p ? ((Ctor)CTOR)(p, nullptr, ARGS...) : nullptr;
}
template <uint32_t SIZE, uint32_t CTOR, uint32_t FIRST, uint32_t LAST>
static void *__stdcall factory_2k(uint32_t a, uint32_t b) {
    g_factories++;
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
    typedef void *(__fastcall * Ctor)(void *, void *, uint32_t, uint32_t, uint32_t, uint32_t);
    return p ? ((Ctor)CTOR)(p, nullptr, FIRST, a, b, LAST) : nullptr;
}

// A constructor with nothing of its own but its table: the base class's constructor, then the table.
static volatile long g_ctor_tables;
template <uint32_t BASE, uint32_t TABLE> static void *__fastcall ctor_base_table(uint32_t *self) {
    g_ctor_tables++;
    ((void(__fastcall *)(void *))BASE)(self);
    self[0] = TABLE;
    return self;
}

// Sets two 3-value vectors (+0, +0xC) and a value (+0x18) from the arguments; each vector is read whole before it
// is written, as the original copies it through the stack. Gives the object.
static volatile long g_vector_sets;
static void *__fastcall set_two_vectors(uint32_t *self, void *, const uint32_t *a, const uint32_t *b, uint32_t value) {
    g_vector_sets++;
    uint32_t t0 = a[0], t1 = a[1], t2 = a[2];
    self[0] = t0, self[1] = t1, self[2] = t2;
    t0 = b[0], t1 = b[1], t2 = b[2];
    self[3] = t0, self[4] = t1, self[5] = t2;
    self[6] = value;
    return self;
}
#define FACTORY_K2(at, size, ctor, a, b, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Factory::CreateWith2", at, REWRITE_CAT(original_, at), 8, (void *)&factory_k<size, ctor, a, b>, &g_factories);
#define FACTORY_K5(at, size, ctor, a, b, c, d, e, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Factory::CreateWith5", at, REWRITE_CAT(original_, at), 8, (void *)&factory_k<size, ctor, a, b, c, d, e>, &g_factories);
#define FACTORY_2K(at, size, ctor, first, last, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Factory::CreateAround", at, REWRITE_CAT(original_, at), 8, (void *)&factory_2k<size, ctor, first, last>, &g_factories);
#define CTOR_BASE_TABLE(at, base, table, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Constructor::BaseAndTable", at, REWRITE_CAT(original_, at), 8, (void *)&ctor_base_table<base, table>, &g_ctor_tables);
#define SET_TWO_VECTORS(at, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Vectors::SetTwoAndValue", at, REWRITE_CAT(original_, at), 8, (void *)&set_two_vectors, &g_vector_sets);
// A lazy global factory whose constructor takes a constant; and a factory given its two arguments in registers
// (ecx, edx: a fastcall), which its constructor takes in that order.
template <uint32_t GLOBAL, uint32_t SIZE, uint32_t A, uint32_t CTOR> static void *__cdecl lazy_factory_k1() {
    g_global_factories++;
    void *made = *(void **)GLOBAL;
    if (!made) {
        void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
        made = p ? ((void *(__fastcall *)(void *, void *, uint32_t))CTOR)(p, nullptr, A) : nullptr;
        *(void **)GLOBAL = made;
    }
    return made;
}
template <uint32_t SIZE, uint32_t CTOR> static void *__fastcall factory_regs(uint32_t a, uint32_t b) {
    g_factories++;
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
    return p ? ((void *(__fastcall *)(void *, void *, uint32_t, uint32_t))CTOR)(p, nullptr, a, b) : nullptr;
}
#define LAZY_FACTORY_K1(at, global, size, a, ctor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Factory::LazyWith", at, REWRITE_CAT(original_, at), 8, (void *)&lazy_factory_k1<global, size, a, ctor>, &g_global_factories);
#define FACTORY_REGS(at, size, ctor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Factory::CreateFromRegisters", at, REWRITE_CAT(original_, at), 8, (void *)&factory_regs<size, ctor>, &g_factories);
// RenderState::Set: a Direct3D render state set through the device (+0x24C; SetRenderState, slot 57) only when the
// value differs from the one kept at KEPT; gives the device's answer, or the value when nothing was set.
static volatile long g_render_states;
template <uint32_t KEPT, uint32_t STATE> static uint32_t __fastcall render_state(uint8_t *self, void *, uint32_t value) {
    g_render_states++;
    if (value == *(uint32_t *)(self + KEPT)) return value;
    void *device = *(void **)(self + 0x24c);
    uint32_t r = ((uint32_t(__stdcall *)(void *, uint32_t, uint32_t))(*(void ***)device)[0xe4 / 4])(device, STATE, value);
    *(uint32_t *)(self + KEPT) = value;
    return r;
}

// FreeList::Take, for a free list kept in globals {+4 block size, +0x10 first free item, +0x14 items made,
// +0x18 items free, +0x1C most in use}: an item from the list, a new block of items made first (MAKE) when it is
// empty; null when there are still none. Counts the item in use and keeps the most in use.
static volatile long g_free_list_takes;
template <uint32_t BASE, uint32_t MAKE, uint32_t FIRST, uint32_t BLOCK, uint32_t FREE, uint32_t MADE, uint32_t MOST>
static void *__cdecl free_list_take() {
    static_assert(FIRST == BASE + 0x10 && BLOCK == BASE + 4 && FREE == BASE + 0x18 && MADE == BASE + 0x14 &&
                      MOST == BASE + 0x1c,
                  "a free list's fields");
    g_free_list_takes++;
    void **item = *(void ***)FIRST;
    if (!item) {
        ((void(__fastcall *)(void *, void *, uint32_t))MAKE)((void *)BASE, nullptr, *(uint32_t *)BLOCK);
        if (!*(uint32_t *)FREE) return nullptr;
        item = *(void ***)FIRST;
    }
    *(void **)FIRST = *item;
    uint32_t free = *(uint32_t *)FREE - 1;
    uint32_t used = *(uint32_t *)MADE - free;
    *(uint32_t *)FREE = free;
    if (used > *(uint32_t *)MOST) *(uint32_t *)MOST = used;
    return item;
}

// The destructor of a listener: its table put back, then it leaves its list (a two-way node at NODE); a node not
// linked both ways means the list is broken: one kind stops at a breakpoint there, as the original does, the other
// leaves it as it is. Gives the node.
static volatile long g_listener_dtors;
template <uint32_t TABLE, uint32_t NODE, bool BREAK = true> static void *__fastcall listener_dtor(uint32_t *self) {
    g_listener_dtors++;
    uint32_t **node = (uint32_t **)((uint8_t *)self + NODE);
    self[0] = TABLE;
    uint32_t *next = node[0], *prev = node[1];
    if (!prev || !next || (uint32_t **)(uintptr_t)next[1] != node || (uint32_t **)(uintptr_t)prev[0] != node) {
        if (BREAK) __builtin_debugtrap();
    }
    else {
        next[1] = (uint32_t)(uintptr_t)prev;
        ((uint32_t *)node[1])[0] = (uint32_t)(uintptr_t)node[0];
        node[0] = node[1] = nullptr;
    }
    return node;
}

// A destructor: the member at MEMBER destroyed (MEMBER_DTOR; given null for a null object, the pointer adjusted
// only when there is one), then the base destructor.
template <uint32_t MEMBER, uint32_t MEMBER_DTOR, uint32_t BASE_DTOR> static void __fastcall dtor_member_if(uint8_t *self) {
    g_member_dtors++;
    ((void(__fastcall *)(void *))MEMBER_DTOR)(self ? self + MEMBER : nullptr);
    ((void(__fastcall *)(void *))BASE_DTOR)(self);
}

// A lazy singleton: made the first time (SIZE bytes, its constructor given three constants; it keeps itself in
// GLOBAL), then activated (ManagedObject::Activate) unless it is already active (bit 0 of +0x2C); gives the global.
static volatile long g_singletons;
template <uint32_t GLOBAL, uint32_t SIZE, uint32_t A, uint32_t B, uint32_t C, uint32_t CTOR>
static void *__cdecl lazy_singleton() {
    g_singletons++;
    if (*(void **)GLOBAL) return *(void **)GLOBAL;
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
    if (p) ((void *(__fastcall *)(void *, void *, uint32_t, uint32_t, uint32_t))CTOR)(p, nullptr, C, B, A);
    uint8_t *made = *(uint8_t **)GLOBAL;
    if (!(made[0x2c] & 1)) ((void(__fastcall *)(void *))0x800860)(made);
    return *(void **)GLOBAL;
}
#define RENDER_STATE(at, kept, state, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("RenderState::Set", at, REWRITE_CAT(original_, at), 8, (void *)&render_state<kept, state>, &g_render_states);
#define FREE_LIST_TAKE(at, base, make, first, block, free, made, most, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("FreeList::Take", at, REWRITE_CAT(original_, at), 8, (void *)&free_list_take<base, make, first, block, free, made, most>, &g_free_list_takes);
#define LISTENER_DTOR(at, table, node, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Listener::~Listener", at, REWRITE_CAT(original_, at), 8, (void *)&listener_dtor<table, node>, &g_listener_dtors);
#define LISTENER_DTOR_QUIET(at, table, node, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Listener::~Listener(quiet)", at, REWRITE_CAT(original_, at), 8, (void *)&listener_dtor<table, node, false>, &g_listener_dtors);
#define DTOR_MEMBER_IF(at, member, member_dtor, base, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Destructor::MemberThenBaseNullSafe", at, REWRITE_CAT(original_, at), 8, (void *)&dtor_member_if<member, member_dtor, base>, &g_member_dtors);
#define LAZY_SINGLETON(at, global, size, a, b, c, ctor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Singleton::Get", at, REWRITE_CAT(original_, at), 8, (void *)&lazy_singleton<global, size, a, b, c, ctor>, &g_singletons);
// Destructors putting back tables (pairs of field and table), deleting members (their deleting destructors, slot 0,
// with 1; none for a null one) or destroying one in place, then running the base destructor.
template <uint32_t... OT> static void put_tables(uint8_t *self) {
    const uint32_t ot[] = {OT...};
    for (size_t i = 0; i < sizeof ot / sizeof ot[0]; i += 2) *(uint32_t *)(self + ot[i]) = ot[i + 1];
}
static void delete_member(uint8_t *self, uint32_t field) {
    void *member = *(void **)(self + field);
    if (member) ((void(__fastcall *)(void *, void *, uint32_t))(*(void ***)member)[0])(member, nullptr, 1);
}
// A destructor the compiler stamped out: it sets some tables, then deletes a list of members (each via its deleting
// destructor, slot 0, flag 1, when not null), then runs the base-class destructor. tools/gen_rewrites finds each
// one's tables, members (in order) and base. `data` holds the NTABLES (offset, value) pairs followed by the member
// offsets; `len` is its length in words.
static void member_dtor_run(uint8_t *self, const uint32_t *data, int len) {
    g_member_dtors++;
    for (int i = 0; i + 2 < len; i += 3) {
        if (data[i] == 0) *(uint32_t *)(self + data[i + 1]) = data[i + 2];          // store a field
        else if (data[i] == 1) delete_member(self, data[i + 1]);                    // delete a member (slot 0, flag 1)
        else if (data[i] == 2) ((void(__fastcall *)(void *))data[i + 1])(self + data[i + 2]);   // direct destructor on this/sub-object
        else if (data[i] == 3) {                                                    // free a pointer member (operator delete)
            void *p = *(void **)(self + data[i + 1]);
            if (p) { ((void(__cdecl *)(void *))data[i + 2])(p); *(void **)(self + data[i + 1]) = nullptr; }
        } else {                                                                    // release a refcounted member
            uint8_t *p = *(uint8_t **)(self + data[i + 1]);
            if (p) {
                uint32_t rc = data[i + 2] & 0xffff, slot = (data[i + 2] >> 16) & 0xff;
                if (--*(uint16_t *)(p + rc) == 0) {
                    if (data[i + 2] >> 24)   // scalar deleting destructor, flag 1 (free)
                        ((void(__fastcall *)(void *, void *, uint32_t))(*(void ***)p)[slot / 4])(p, nullptr, 1);
                    else                     // plain destructor, no flag
                        ((void(__fastcall *)(void *))(*(void ***)p)[slot / 4])(p);
                }
                *(void **)(self + data[i + 1]) = nullptr;
            }
        }
    }
}
template <uint32_t BASE, uint32_t M1, uint32_t M2, uint32_t... OT> static void __fastcall dtor_delete2(uint8_t *self) {
    g_member_dtors++;
    put_tables<OT...>(self);
    delete_member(self, M1);
    delete_member(self, M2);
    ((void(__fastcall *)(void *))BASE)(self);
}
template <uint32_t T0, uint32_t T4, uint32_t MEMBER, uint32_t MEMBER_TABLE, uint32_t MEMBER_DTOR, uint32_t BASE>
static void __fastcall dtor_tables_member(uint8_t *self) {
    g_member_dtors++;
    put_tables<0, T0, 4, T4, MEMBER, MEMBER_TABLE>(self);
    ((void(__fastcall *)(void *))MEMBER_DTOR)(self + MEMBER);
    ((void(__fastcall *)(void *))BASE)(self);
}
#define DTOR_TABLES_MEMBER(at, t0, t4, member, member_table, member_dtor, base, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Destructor::TablesAndMember", at, REWRITE_CAT(original_, at), 8, (void *)&dtor_tables_member<t0, t4, member, member_table, member_dtor, base>, &g_member_dtors);
#define DTOR_DELETE2(at, t0, t4, m1, m2, base, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Destructor::DeleteTwoMembers", at, REWRITE_CAT(original_, at), 8, (void *)&dtor_delete2<base, m1, m2, 0, t0, 4, t4>, &g_member_dtors);
#define DTOR5_DELETE2(at, t0, t4, o2, t2, o3, t3, o4, t4b, m1, m2, base, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Destructor::FiveTablesTwoMembers", at, REWRITE_CAT(original_, at), 8, (void *)&dtor_delete2<base, m1, m2, 0, t0, 4, t4, o2, t2, o3, t3, o4, t4b>, &g_member_dtors);
// ---- quadtrees ----------------------------------------------------------------------------------------------------
// QuadTree::Node::Add: a node {..., +0xC first free slot, +0x10 slots used up to, +0x14 capacity, +0x18 items} of
// 32-byte items {corner A (3 values), corner B (3 values), the item, in use}. When full it grows (to 8, then twice
// as many: aligned memory, the items constructed by the vector constructor iterator 0x402B90 with CTOR, the old ones
// copied over and freed, the new ones marked free). Stores the item and its box in the first free slot, marks it,
// and moves the first free slot on; gives the slot. The first two values of each corner are copied through the FPU,
// as the original does (a signalling NaN comes out quiet).
static volatile long g_quadtree_adds;
static void copy_through_fpu(uint32_t *to, const uint32_t *from) {
    __asm__ volatile("flds %1\n\tfstps %0" : "=m"(*to) : "m"(*from));
}
template <uint32_t CTOR>
static int32_t __fastcall quadtree_add(uint8_t *node, void *, const uint32_t *item, const uint32_t *a, const uint32_t *b) {
    g_quadtree_adds++;
    int32_t &first_free = *(int32_t *)(node + 0xc), &used = *(int32_t *)(node + 0x10),
            &capacity = *(int32_t *)(node + 0x14);
    uint32_t *&items = *(uint32_t **)(node + 0x18);
    if (first_free >= capacity) {
        int32_t grown = capacity ? capacity * 2 : 8;
        uint32_t *made = ((uint32_t * (__cdecl *)(uint32_t))0xbcdfd0)((uint32_t)grown << 5);
        if (made)
            ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0x402b90)(made, 0x20, grown, (void *)CTOR);
        if (items) {
            memcpy(made, items, (uint32_t)capacity << 5);
            ((void(__cdecl *)(void *))0xbce010)(items);
        }
        for (int32_t k = capacity; k < grown; k++) made[k * 8 + 7] = 0;
        capacity = grown;
        items = made;
    }
    int32_t slot = first_free;
    uint32_t *x = items + slot * 8;
    copy_through_fpu(x, a);
    copy_through_fpu(x + 1, a + 1);
    x[2] = a[2];
    x[5] = b[2];
    copy_through_fpu(x + 3, b);
    copy_through_fpu(x + 4, b + 1);
    x[6] = *item;
    x[7] = 1;
    if (first_free == used) used++;
    first_free++;
    while (first_free < used && items[first_free * 8 + 7]) first_free++;
    return slot;
}
// QuadTree::Insert(item, corner A, corner B): the node that should hold the box (QuadTree::FindNode, 0xCDB600:
// ecx 0, edx the tree's depth (+4), then the root's origin (+0xC) and size (+0x14), the box's first corner (x, z) and
// its size (B - A in x and z, worked out in the FPU and kept as floats)); the item added to it (ADD, the nodes 28
// bytes apart at +8) and that node and those above it marked changed (MARK). Gives the node in the high 16 bits and
// the item's slot in the low ones.
static volatile long g_quadtree_inserts;
template <uint32_t ADD, uint32_t MARK>
static uint32_t __fastcall quadtree_insert(uint8_t *self, void *, void *item, const uint32_t *a, const uint32_t *b) {
    g_quadtree_inserts++;
    volatile float size[2];
    size[0] = ((const float *)b)[0] - ((const float *)a)[0];
    uint32_t corner[2] = {a[0], a[2]};
    size[1] = ((const float *)b)[2] - ((const float *)a)[2];
    typedef int32_t(__fastcall * Find)(int32_t, uint32_t, void *, void *, const uint32_t *, volatile float *);
    int32_t node = ((Find)0xcdb600)(0, *(uint32_t *)(self + 4), self + 0xc, self + 0x14, corner, size);
    uint32_t slot = ((uint32_t(__fastcall *)(void *, void *, void *, const uint32_t *, const uint32_t *))ADD)(
        *(uint8_t **)(self + 8) + node * 0x1c, nullptr, item, a, b);
    ((void(__fastcall *)(void *, void *, int32_t))MARK)(self, nullptr, node);
    return ((uint32_t)node << 16) | slot;
}
#define QUADTREE_INSERT(at, add, mark, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("QuadTree::Insert", at, REWRITE_CAT(original_, at), 8, (void *)&quadtree_insert<add, mark>, &g_quadtree_inserts);
#define QUADTREE_ADD(at, ctor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("QuadTree::Node::Add", at, REWRITE_CAT(original_, at), 8, (void *)&quadtree_add<ctor>, &g_quadtree_adds);
// A call forwarded to an interface of what a reference refers to, looked up once (kept at CACHE, FLAG set): the
// interface (id in ID, as QueryInterface) of the object the reference at REF points to (its handle's object at +0xC);
// then a slot (SLOT bytes into its table) of the sub-object at +0x40 of that interface is tail-called with the two arguments (its `this` is that
// sub-object). Gives null (nothing forwarded) when there is no interface.
static volatile long g_interface_forwards;
template <uint32_t REF, uint32_t FLAG, uint32_t CACHE, uint32_t ID, uint32_t SLOT>
static uint32_t __fastcall interface_forward(uint8_t *self, void *, uint32_t a, uint32_t b) {
    g_interface_forwards++;
    if (!self[FLAG]) {
        void *found = nullptr;
        void *object = *(void **)(*(uint8_t **)(self + REF) + 0xc);
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
    uint8_t *interface = *(uint8_t **)(self + CACHE);
    if (!interface) return 0;
    uint8_t *part = interface + 0x40;
    return ((uint32_t(__fastcall *)(void *, void *, uint32_t, uint32_t))(*(void ***)part)[SLOT / 4])(part, nullptr, a, b);
}
#define INTERFACE_FORWARD(at, ref, flag, cache, id, slot, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Interface::Forward", at, REWRITE_CAT(original_, at), 8, (void *)&interface_forward<ref, flag, cache, id, slot>, &g_interface_forwards);
// A float field reached through a cached interface: the interface (id in ID, as QueryInterface) of the object the
// reference at REF points to (its handle's object at +0xC), looked up once (kept at CACHE, FLAG set). Then an inner
// object at +INNER of the interface, and one of its two float fields: F2 when the field at +COND of this is nonzero,
// else F1. The interface is used without a null check (the original crashes without one, and so does this).
static volatile long g_interface_floats;
template <uint32_t REF, uint32_t FLAG, uint32_t CACHE, uint32_t ID, uint32_t COND, uint32_t INNER, uint32_t F1,
          uint32_t F2>
static float __fastcall interface_float(uint8_t *self) {
    g_interface_floats++;
    if (!self[FLAG]) {
        void *found = nullptr;
        void *object = *(void **)(*(uint8_t **)(self + REF) + 0xc);
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
    uint8_t *interface = *(uint8_t **)(self + CACHE);
    uint8_t *inner = *(uint8_t **)(interface + INNER);
    return *(float *)(inner + (*(uint32_t *)(self + COND) ? F2 : F1));
}
#define INTERFACE_FLOAT(at, ref, flag, cache, id, cond, inner, f1, f2, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Interface::Float", at, REWRITE_CAT(original_, at), 8, (void *)&interface_float<ref, flag, cache, id, cond, inner, f1, f2>, &g_interface_floats);
// Frustum::FromCamera(camera, flags): copies the camera's six clipping planes into this frustum. Each plane in the
// camera is a 3-value normal then its distance (stride 0x20, from +0x10); in the frustum they are packed at stride
// 0x10 with the distance negated. The flags go at +0x60. The distance is negated through the x87 FPU, as the
// original does (a signalling NaN comes back quiet), so a plain float negate would not match. Gives the frustum.
static volatile long g_frustums;
static void frustum_neg(uint32_t *to, const uint32_t *from) {
    __asm__ volatile("flds %1\n\tfchs\n\tfstps %0" : "=m"(*to) : "m"(*from));
}
static void *__fastcall frustum_from_camera(uint8_t *self, void *, const uint8_t *camera, uint32_t flags) {
    g_frustums++;
    *(uint32_t *)(self + 0x60) = flags;
    for (int i = 0; i < 6; i++) {
        const uint8_t *s = camera + 0x10 + i * 0x20;
        uint8_t *d = self + i * 0x10;
        *(uint32_t *)(d + 0) = *(const uint32_t *)(s + 0);
        *(uint32_t *)(d + 4) = *(const uint32_t *)(s + 4);
        *(uint32_t *)(d + 8) = *(const uint32_t *)(s + 8);
        frustum_neg((uint32_t *)(d + 0xc), (const uint32_t *)(s + 0x10));
    }
    return self;
}
static const uint8_t frustum_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x08, 0x89, 0x48};
static const uint32_t frustum_copies[] = {0x42fa40, 0x449320, 0x488520, 0x7b2cf0, 0xa0bfd0, 0xb5b8d0, 0xb6baa0};
REWRITE_COPIES("Frustum::FromCamera", frustum_copies, frustum_original, frustum_from_camera, &g_frustums);

// SortedMap::SortedMap(n, cap): two n*4-byte blocks (keys at +0, values at +8), the counts at +4 and +0xC set to n,
// the capacity at +0x10 set to cap (or 0x80 when cap <= 0), +0x14 cleared. Gives the map. (The order of the stores
// and the two allocations matches the original.)
static volatile long g_sorted_map_ctors;
static void *__fastcall sorted_map_ctor(uint32_t *self, void *, int32_t n, int32_t cap) {
    g_sorted_map_ctors++;
    self[1] = n;
    self[0] = (uint32_t)(uintptr_t)((void *(__cdecl *)(uint32_t))0xbcdfd0)((uint32_t)n * 4);
    self[3] = n;
    self[2] = (uint32_t)(uintptr_t)((void *(__cdecl *)(uint32_t))0xbcdfd0)((uint32_t)n * 4);
    self[4] = cap > 0 ? (uint32_t)cap : 0x80;
    self[5] = 0;
    return self;
}
#define SORTED_MAP_CTOR(at, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("SortedMap::SortedMap", at, REWRITE_CAT(original_, at), 8, (void *)&sorted_map_ctor, &g_sorted_map_ctors);

// ObjectPair::Release: drops a reference on the objects at +4 and +0 (each a reference count at its +0); when a
// count reaches zero its aligned block (+0xC) and itself are freed. The count is decremented before the null check,
// as the original does. (+4 first, then +0.)
static volatile long g_pair_releases;
static void pair_release_one(uint32_t *obj) {
    if (--obj[0] == 0 && obj) {
        ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)obj[3]);
        ((void(__cdecl *)(void *))0xbce000)(obj);
    }
}
static void __fastcall pair_release(uint32_t **self) {
    g_pair_releases++;
    pair_release_one(self[1]);
    pair_release_one(self[0]);
}
#define PAIR_RELEASE(at, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("ObjectPair::Release", at, REWRITE_CAT(original_, at), 8, (void *)&pair_release, &g_pair_releases);

// Array::SetCapacity for trivially-copyable items (SIZE bytes, no per-item destructor): resized to n, the new items
// allocated (aligned, no count header) and constructed (the vector constructor iterator 0x402B90, CTOR only), the old
// ones that fit copied over as raw bytes, the old block freed. Gives n. (As the original, the copy runs even when the
// allocation failed, so a crash there matches.)
static volatile long g_trivial_setcaps;
template <uint32_t SIZE, uint32_t CTOR> static int32_t __fastcall trivial_set_capacity(uint8_t **array, void *, int32_t n) {
    g_trivial_setcaps++;
    int32_t count = ((int32_t *)array)[1];
    if (n == count) return n;
    uint8_t *items = nullptr;
    if (n != 0) {
        items = (uint8_t *)((void *(__cdecl *)(uint32_t))0xbcdfd0)((uint32_t)n * SIZE);
        if (items) ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0x402b90)(items, SIZE, n, (void *)CTOR);
    }
    int32_t keep = n > count ? count : n;
    for (int32_t i = 0; i < keep; i++) __builtin_memcpy(items + i * SIZE, *array + i * SIZE, SIZE);
    if (*array) {
        ((void(__cdecl *)(void *))0xbce010)(*array);
        *array = nullptr;
    }
    *array = items;
    ((int32_t *)array)[1] = n;
    return n;
}
#define TRIVIAL_SETCAP(at, size, ctor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("ArrayTrivial::SetCapacity", at, REWRITE_CAT(original_, at), 8, (void *)&trivial_set_capacity<size, ctor>, &g_trivial_setcaps);
// ObjectArray::Construct: a new array of n SIZE-byte objects made with new[] (the count kept in front, at items-4),
// the count also stored at +4 of the object; the items constructed by the vector constructor iterator with CTOR and
// DTOR. No memory leaves the pointer null (the count at +4 is still set). Gives the object.
static volatile long g_object_constructs;
template <uint32_t SIZE, uint32_t CTOR, uint32_t DTOR>
static void *__fastcall object_construct(uint8_t *self, void *, int32_t n) {
    g_object_constructs++;
    *(int32_t *)(self + 4) = n;
    int32_t *block = ((int32_t * (__cdecl *)(uint32_t))0xbcdfd0)((uint32_t)n * SIZE + 4);
    if (block) {
        block[0] = n;
        uint8_t *items = (uint8_t *)(block + 1);
        ((void(__stdcall *)(void *, uint32_t, int32_t, void *, void *))0xddcf99)(items, SIZE, n, (void *)CTOR,
                                                                               (void *)DTOR);
        *(void **)self = items;
    } else {
        *(void **)self = nullptr;
    }
    return self;
}

// A behaviour object made at its owner's request: SIZE bytes constructed by CTOR (given three zeros), linked to the
// owner (LINK, with the owner), then set up (ManagedObject::Activate 0x800860) unless already active (bit 0 of
// +0x2C). Gives the object. With no memory the original goes on with null and crashes, and so does this.
static volatile long g_behaviour_creates;
template <uint32_t SIZE, uint32_t CTOR, uint32_t LINK> static void *__fastcall behaviour_create(void *owner) {
    g_behaviour_creates++;
    void *p = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE);
    void *obj = p ? ((void *(__fastcall *)(void *, void *, uint32_t, uint32_t, uint32_t))CTOR)(p, nullptr, 0, 0, 0)
                  : nullptr;
    ((void(__fastcall *)(void *, void *, void *))LINK)(obj, nullptr, owner);
    if (!(*((uint8_t *)obj + 0x2c) & 1)) ((void(__fastcall *)(void *))0x800860)(obj);
    return obj;
}
#define OBJECT_CONSTRUCT(at, size, ctor, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("ObjectArray::Construct", at, REWRITE_CAT(original_, at), 8, (void *)&object_construct<size, ctor, dtor>, &g_object_constructs);
// A function that sets some fields of this to constants and returns this. data is (offset, value) pairs.
static volatile long g_setfields;
static void *setfields_run(uint8_t *self, const uint32_t *data, int len) {
    g_setfields++;
    for (int i = 0; i + 2 < len; i += 3) {   // (offset, value, size) — size 1/2/4 = byte/word/dword
        if (data[i + 2] == 1) *(uint8_t *)(self + data[i]) = (uint8_t)data[i + 1];
        else if (data[i + 2] == 2) *(uint16_t *)(self + data[i]) = (uint16_t)data[i + 1];
        else *(uint32_t *)(self + data[i]) = data[i + 1];
    }
    return self;
}
#define SETFIELDS(at, b0, b1, b2, b3, b4, b5, b6, b7, ...) static const uint32_t REWRITE_CAT(sfdata_, at)[] = {__VA_ARGS__}; static void *__fastcall REWRITE_CAT(sffn_, at)(uint8_t *s) { return setfields_run(s, REWRITE_CAT(sfdata_, at), (int)(sizeof(REWRITE_CAT(sfdata_, at)) / 4)); } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::SetFields", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(sffn_, at), &g_setfields);
// Free a pointer field: if this->[OFF] is non-null, operator delete it, then null the field.
static volatile long g_freefields;
#define FREEFIELD(at, off, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void __fastcall REWRITE_CAT(fffn_, at)(uint8_t *s) { g_freefields++; void *p = *(void **)(s + off); if (p) ((void(__cdecl *)(void *))0xbce010)(p); *(void **)(s + off) = nullptr; } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::FreeField", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(fffn_, at), &g_freefields);
// A destructor thunk: set some table fields on this, then tail-call the base destructor (void, this only). The
// original tail-jumps to BASE; set-fields-then-call-then-return is observationally identical for a void base.
// data is (offset, value) pairs, any count.
static volatile long g_tables_then_base;
#define TABLES_THEN_BASE(at, base, b0, b1, b2, b3, b4, b5, b6, b7, ...) static const uint32_t REWRITE_CAT(ttbdata_, at)[] = {__VA_ARGS__}; static void __fastcall REWRITE_CAT(ttbfn_, at)(uint8_t *s) { g_tables_then_base++; const uint32_t *d = REWRITE_CAT(ttbdata_, at); int ln = (int)(sizeof(REWRITE_CAT(ttbdata_, at)) / 4); for (int i = 0; i + 1 < ln; i += 2) *(uint32_t *)(s + d[i]) = d[i + 1]; ((void(__fastcall *)(void *))base)(s); } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Destructor::TablesThenBase", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(ttbfn_, at), &g_tables_then_base);
// A get-and-addref accessor: *out = this->[OFF] (a refcounted object), then ++ its refcount (first dword); gives out.
static volatile long g_getrefs;
#define GETREF(at, off, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void *__thiscall REWRITE_CAT(getref_, at)(uint8_t *s, void **out) { g_getrefs++; void *p = *(void **)(s + off); *out = p; ++*(uint32_t *)p; return out; } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::GetRef", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(getref_, at), &g_getrefs);
#define MEMBER_DTOR(at, t0, t1, t2, f0, f1, f2, b0, b1, b2, b3, b4, b5, b6, b7, ...) static const uint32_t REWRITE_CAT(mddata_, at)[] = {__VA_ARGS__}; static void __fastcall REWRITE_CAT(mdfn_, at)(uint8_t *s) { member_dtor_run(s, REWRITE_CAT(mddata_, at), (int)(sizeof(REWRITE_CAT(mddata_, at)) / 4)); } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Destructor::Members", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(mdfn_, at), &g_member_dtors);
#define BEHAVIOUR_CREATE(at, size, ctor, link, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Behaviour::Create", at, REWRITE_CAT(original_, at), 8, (void *)&behaviour_create<size, ctor, link>, &g_behaviour_creates);
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
#define START_KEY_FULL(at, key1, key2, key3, group, setting, name, flag1, flag2, global, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::KeyCommand", at, REWRITE_CAT(original_, at), 8, (void *)&key_command<key1, key2, key3, group, setting, name, flag1, flag2, global, dtor, true>, &g_key_commands);
#define START_KEY_FULL0(at, key1, key2, key3, group, setting, name, flag1, flag2, global, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::KeyCommand", at, REWRITE_CAT(original_, at), 8, (void *)&key_command<key1, key2, key3, group, setting, name, flag1, flag2, global, dtor, false>, &g_key_commands);
#define NAMED_TEXT_REF(at, suffix, table, class_table, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("NamedTextRef::NamedTextRef", at, REWRITE_CAT(original_, at), 8, (void *)&named_text_ref<suffix, table, class_table>, &g_named_text_refs);
#define GAME_TEXT_COPY(at, name, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("GameText::Copy", at, REWRITE_CAT(original_, at), 8, (void *)&game_text_copy<name>, &g_game_text_copies);
#define START_NAME(at, text, global, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::Name", at, REWRITE_CAT(original_, at), 8, (void *)&registered_name<text, global>, &g_names);
#define START_CLASS(at, name, factory, global, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::ClassDescription", at, REWRITE_CAT(original_, at), 8, (void *)&class_description<name, factory, global>, &g_class_descriptions);
#define START_SECTION(at, text, kind, global, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("Start::Section", at, REWRITE_CAT(original_, at), 8, (void *)&named_section<text, kind, global, dtor>, &g_sections);
#define DELETING(at, dtor, freefn, ...)                                                                        \
    static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__};                                        \
    static rewrite::Entry REWRITE_CAT(entry_, at)("DeletingDestructor", at, REWRITE_CAT(original_, at), 8,     \
                                                  (void *)&deleting<dtor>, &g_deleting);
// A scalar deleting destructor whose free is a member (thiscall) delete FREE rather than the plain operator delete:
// dtor(this); if(flags&1) free(this); return this.
#define DELETING2(at, dtor, free, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void *__fastcall REWRITE_CAT(del2_, at)(uint8_t *s, void *, uint32_t flags) { g_deleting++; ((void(__fastcall *)(void *))dtor)(s); if (flags & 1) ((void(__fastcall *)(void *))free)(s); return s; } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("DeletingDestructor(memberfree)", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(del2_, at), &g_deleting);
// Forward both arguments to a member method (thiscall) and return this: method(this, a0, a1); return this.
static volatile long g_forward2;
#define FORWARD2(at, method, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void *__thiscall REWRITE_CAT(fwd2_, at)(uint8_t *s, uint32_t a0, uint32_t a1) { g_forward2++; ((void(__thiscall *)(void *, uint32_t, uint32_t))method)(s, a0, a1); return s; } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::Forward2", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(fwd2_, at), &g_forward2);
// Count a chain: start at this->[0], follow each node's [0], counting, stopping when a node's [0] is null. The
// nodes are dereferenced before the null test (as the original does), so a valid chain must be null-terminated.
static volatile long g_count_lists;
static uint32_t __fastcall count_list(uint8_t *self) {
    g_count_lists++;
    uint32_t *p = *(uint32_t **)self;
    uint32_t nx = *p, eax = 0;
    if (nx == 0 || p == nullptr) return 0;
    for (;;) {
        p = *(uint32_t **)p;
        nx = *p;
        eax++;
        if (nx == 0 || p == nullptr) return eax;
    }
}
#define COUNT_LIST(at, b0, b1, b2, b3, b4, b5, b6, b7, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::CountList", at, REWRITE_CAT(original_, at), 8, (void *)&count_list, &g_count_lists);
// Call A(this), then tail-call B(this) (both void thiscall). Observationally A(this); B(this); for void B.
static volatile long g_call_then_tail;
#define CALL_THEN_TAIL(at, a, b, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void __fastcall REWRITE_CAT(ctt_, at)(uint8_t *s) { g_call_then_tail++; ((void(__fastcall *)(void *))a)(s); ((void(__fastcall *)(void *))b)(s); } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::CallThenTail", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(ctt_, at), &g_call_then_tail);
// A checked-build singleton accessor wrapper: set ecx to a global, call a member method; the following
// __RTC_CheckEsp is a no-op stack assertion, omitted. Ignores its own arguments; returns void.
static volatile long g_dbg_singletons;
#define DBG_SINGLETON(at, global, method, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void __fastcall REWRITE_CAT(dbgs_, at)(uint8_t *) { g_dbg_singletons++; ((void(__thiscall *)(void *))method)((void *)global); } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::DbgSingleton", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(dbgs_, at), &g_dbg_singletons);
// A getter: return this->[OFF].
static volatile long g_getfields;
#define GETFIELD(at, off, b0, b1, b2, b3, b4, b5, b6, b7, ...) static uint32_t __fastcall REWRITE_CAT(getf_, at)(uint8_t *s) { g_getfields++; return *(uint32_t *)(s + off); } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::GetField", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(getf_, at), &g_getfields);
// If this is non-null, call a member method on (*GLOBAL + 4) passing this as the argument.
static volatile long g_global_member_calls;
#define GLOBAL_MEMBER_CALL(at, global, method, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void __fastcall REWRITE_CAT(gmc_, at)(uint8_t *s) { g_global_member_calls++; if (s) ((void(__thiscall *)(void *, void *))method)((void *)(*(uintptr_t *)global + 4), s); } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::GlobalMemberCall", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(gmc_, at), &g_global_member_calls);
// If this->[0xc] (signed) is >= 0, call a member method on this->[0] forwarding this->[0xc] and this->[8].
static volatile long g_cond_forwards;
#define COND_FORWARD(at, method, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void __fastcall REWRITE_CAT(cf_, at)(uint8_t *s) { g_cond_forwards++; int32_t v = *(int32_t *)(s + 0xc); if (v >= 0) ((void(__thiscall *)(void *, uint32_t, uint32_t))method)(*(void **)s, (uint32_t)v, *(uint32_t *)(s + 8)); } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::CondForward", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(cf_, at), &g_cond_forwards);
// Append *arg to a counted array: reserve(count+1); this->[0xc][count] = *arg; ++count.
static volatile long g_array_pushes;
#define ARRAY_PUSH(at, reserve, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void __thiscall REWRITE_CAT(ap_, at)(uint8_t *s, void *arg) { g_array_pushes++; uint32_t c = *(uint32_t *)s; ((void(__thiscall *)(void *, uint32_t))reserve)(s, c + 1); uint32_t c2 = *(uint32_t *)s; (*(uint32_t **)(s + 0xc))[c2] = *(uint32_t *)arg; ++*(uint32_t *)s; } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::ArrayPush", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(ap_, at), &g_array_pushes);
// Delete a member field via its deleting destructor (slot 0, flag 1) when non-null, then null the field.
static volatile long g_del_member_fields;
#define DELMEMBER_FIELD(at, off, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void __fastcall REWRITE_CAT(dmf_, at)(uint8_t *s) { g_del_member_fields++; void *m = *(void **)(s + off); if (m) ((void(__fastcall *)(void *, void *, uint32_t))(*(void ***)m)[0])(m, nullptr, 1); *(void **)(s + off) = nullptr; } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::DelMemberField", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(dmf_, at), &g_del_member_fields);
// Free two pointer fields with operator delete (no null check, no clear).
static volatile long g_free_two_fields;
#define FREE_TWO_FIELDS(at, off0, off1, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void __fastcall REWRITE_CAT(ftf_, at)(uint8_t *s) { g_free_two_fields++; ((void(__cdecl *)(void *))0xbce010)(*(void **)(s + off0)); ((void(__cdecl *)(void *))0xbce010)(*(void **)(s + off1)); } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::FreeTwoFields", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(ftf_, at), &g_free_two_fields);
// A checked-build wrapper: call a member method on a global, then a __cdecl helper with an immediate; the trailing
// __RTC_CheckEsp is a no-op, omitted.
static volatile long g_dbg_wrap2;
#define DBG_WRAP2(at, global, method, helper, argimm, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void __fastcall REWRITE_CAT(dw2_, at)(uint8_t *) { g_dbg_wrap2++; ((void(__thiscall *)(void *))method)((void *)global); ((void(__cdecl *)(uint32_t))helper)(argimm); } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::DbgWrap2", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(dw2_, at), &g_dbg_wrap2);
// A constructor thunk: call the base constructor with one immediate argument, set a table at [this], return this.
static volatile long g_ctor_base_args;
#define CTOR_BASE_ARG(at, base, argimm, table, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void *__fastcall REWRITE_CAT(cba_, at)(uint8_t *s) { g_ctor_base_args++; ((void(__thiscall *)(void *, uint32_t))base)(s, argimm); *(uint32_t *)s = table; return s; } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::CtorBaseArg", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(cba_, at), &g_ctor_base_args);
// Call an allocator with ecx = (this with low byte set to 1), store the result at [this], return this.
static volatile long g_alloc_stores;
#define ALLOC_STORE(at, alloc, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void *__fastcall REWRITE_CAT(als_, at)(uint8_t *s) { g_alloc_stores++; void *r = ((void *(__fastcall *)(void *))alloc)((void *)(((uintptr_t)s & ~(uintptr_t)0xff) | 1)); *(void **)s = r; return s; } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::AllocStore", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(als_, at), &g_alloc_stores);
// If the argument (signed) exceeds this->[4], call a member method with it; then store it at this->[8].
static volatile long g_cond_call_sets;
#define COND_CALL_SET(at, method, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void __thiscall REWRITE_CAT(ccs_, at)(uint8_t *s, uint32_t arg) { g_cond_call_sets++; if ((int32_t)arg > *(int32_t *)(s + 4)) ((void(__thiscall *)(void *, uint32_t))method)(s, arg); *(uint32_t *)(s + 8) = arg; } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::CondCallSet", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(ccs_, at), &g_cond_call_sets);
// Set a table at [this], then operator delete two pointer fields (no null check, no clear).
static volatile long g_free_two_tables;
#define FREE_TWO_TABLE(at, table, off0, off1, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void __fastcall REWRITE_CAT(ftt_, at)(uint8_t *s) { g_free_two_tables++; *(uint32_t *)s = table; ((void(__cdecl *)(void *))0xbce010)(*(void **)(s + off0)); ((void(__cdecl *)(void *))0xbce010)(*(void **)(s + off1)); } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::FreeTwoTable", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(ftt_, at), &g_free_two_tables);
// A grid/quadtree level-grow: ensure the records array (this->[0], 20-byte records) covers `level` (reserve +
// fill with q = 0x1000/(M*(1<<level)), clamped >=1), then for record[level] reallocate its r1 index buffer (+1
// entry) and its r3 pointer table, subdividing the per-level block of M*(1<<level) bytes into r0 pieces. Returns
// record[level].r3[--r4]. M (80 or 20) and the reserve call vary per copy; malloc(0xbcdfd0)/free(0xbce010) fixed.
static volatile long g_grid_grows;
template <uint32_t RESERVE, uint32_t M> static void *__thiscall grid_grow(uint8_t *self, int32_t level) {
    g_grid_grows++;
    uint32_t ebx = 1u << ((uint32_t)level & 31);
    int32_t f4 = *(int32_t *)(self + 4);
    if (level >= f4) {
        ((void(__thiscall *)(void *, uint32_t))RESERVE)(self, (uint32_t)level + 1);
        if (f4 <= level) {
            uint32_t q = 0x1000u / (ebx * M);
            uint32_t pos = (uint32_t)f4 * 20u;
            int32_t cnt = level - f4 + 1;
            do {
                uint8_t *b = *(uint8_t **)self;
                *(uint32_t *)(b + pos) = q;
                b = *(uint8_t **)self;
                if (*(int32_t *)(b + pos) < 1) *(int32_t *)(b + pos) = 1;
                b = *(uint8_t **)self; *(uint32_t *)(b + pos + 4) = 0;
                b = *(uint8_t **)self; *(uint32_t *)(b + pos + 8) = 0;
                b = *(uint8_t **)self; *(uint32_t *)(b + pos + 0xc) = 0;
                b = *(uint8_t **)self; *(uint32_t *)(b + pos + 0x10) = 0;
                pos += 0x14;
                cnt--;
            } while (cnt != 0);
        }
    }
    uint8_t *rec = *(uint8_t **)self + (uint32_t)level * 20u;
    if (*(int32_t *)(rec + 0x10) <= 0) {
        uint32_t r2p1 = *(uint32_t *)(rec + 8) + 1;
        uint8_t *oldr1 = *(uint8_t **)(rec + 4);
        *(void **)(rec + 4) = ((void *(__cdecl *)(uint32_t))0xbcdfd0)(r2p1 * 4u);
        int32_t n = *(int32_t *)(rec + 8);
        for (uint32_t i = 0; (int32_t)i < n; i++) {
            *(uint32_t *)(*(uint8_t **)(rec + 4) + i * 4) = *(uint32_t *)(oldr1 + i * 4);
            n = *(int32_t *)(rec + 8);
        }
        ((void(__cdecl *)(void *))0xbce010)(oldr1);
        ((void(__cdecl *)(void *))0xbce010)(*(void **)(rec + 0xc));
        *(void **)(rec + 0xc) = ((void *(__cdecl *)(uint32_t))0xbcdfd0)(*(uint32_t *)rec * r2p1 * 4u);
        int32_t edi = (int32_t)r2p1 - 1;
        while (edi >= *(int32_t *)(rec + 8)) {
            *(void **)(*(uint8_t **)(rec + 4) + (uint32_t)edi * 4) = ((void *(__cdecl *)(uint32_t))0xbcdfd0)(*(uint32_t *)rec * ebx * M);
            int32_t r0m1 = *(int32_t *)rec - 1;
            if (r0m1 >= 0) {
                uint32_t eax = (uint32_t)r0m1 * ebx * M;
                uint32_t edx = ebx * (uint32_t)(-(int32_t)M);
                uint32_t loopn = (uint32_t)r0m1 + 1;
                do {
                    uint32_t val = *(uint32_t *)(*(uint8_t **)(rec + 4) + (uint32_t)edi * 4);
                    uint32_t r4 = *(uint32_t *)(rec + 0x10);
                    *(uint32_t *)(*(uint8_t **)(rec + 0xc) + r4 * 4) = val + eax;
                    *(uint32_t *)(rec + 0x10) = r4 + 1;
                    eax += edx;
                    loopn--;
                } while (loopn != 0);
            }
            edi--;
        }
        *(uint32_t *)(rec + 8) = r2p1;
    }
    uint32_t r4 = *(uint32_t *)(rec + 0x10) - 1;
    *(uint32_t *)(rec + 0x10) = r4;
    return (void *)(uintptr_t) * (uint32_t *)(*(uint8_t **)(rec + 0xc) + r4 * 4);
}
#define GRID_GROW(at, reserve, m, b0, b1, b2, b3, b4, b5, b6, b7, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::GridGrow", at, REWRITE_CAT(original_, at), 8, (void *)&grid_grow<reserve, m>, &g_grid_grows);
// A recursive quadtree collect: for node[idx] (this->[8], 0x1c-byte nodes), if it has entries (f_c or f_10), append
// each of its f_10 children's f_18 (whose f_1c != 0) to a collector (arg1->[COFF], buffer at +4, count at +0x104,
// flushing via vtbl[0] when count hits 0x40); then for each set low-nibble bit of node.f_0, recurse into child
// node (idx*4 + bit + 1). COFF (the collector-pointer field offset) varies per copy.
static volatile long g_quad_collects;
template <uint32_t COFF> static void __thiscall quad_collect(uint8_t *self, int32_t idx, uint8_t *arg1) {
    g_quad_collects++;
    uint8_t *node = *(uint8_t **)(self + 8) + (uint32_t)idx * 0x1c;
    uint32_t a = *(uint32_t *)(node + 0xc);
    if (a != 0 || *(uint32_t *)(node + 0x10) != 0) {
        int32_t cnt = *(int32_t *)(node + 0x10);
        for (int32_t ebp = 0, ebx = 0; ebp < cnt; ebp++, ebx += 0x20) {
            uint8_t *ch = *(uint8_t **)(node + 0x18);
            if (*(uint32_t *)(ch + ebx + 0x1c) != 0) {
                uint8_t *col = *(uint8_t **)(arg1 + COFF);
                uint32_t v = *(uint32_t *)(ch + ebx + 0x18);
                *(uint32_t *)(col + (uint32_t)*(int32_t *)(col + 0x104) * 4 + 4) = v;
                int32_t nc = *(int32_t *)(col + 0x104) + 1;
                *(int32_t *)(col + 0x104) = nc;
                if (nc == 0x40) {
                    ((void(__thiscall *)(void *, void *, uint32_t))(*(void ***)col)[0])(col, col + 4, 0x40);
                    *(int32_t *)(col + 0x104) = 0;
                }
            }
            cnt = *(int32_t *)(node + 0x10);
        }
    }
    if (*(uint8_t *)node & 0xf)
        for (int32_t s = 0; s < 4; s++)
            if (*(uint32_t *)node & (1u << s)) quad_collect<COFF>(self, s + idx * 4 + 1, arg1);
}
#define QUAD_COLLECT(at, coff, b0, b1, b2, b3, b4, b5, b6, b7, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::QuadCollect", at, REWRITE_CAT(original_, at), 8, (void *)&quad_collect<coff>, &g_quad_collects);
// Drain a list: node = head(this); while (node) { visit(node); node = next(node); }. head (0x59b030) and next
// (0x5d49d0) are fixed thiscalls; the visit method M1 varies per copy.
static volatile long g_list_drains;
template <uint32_t M1> static void __fastcall list_drain(uint8_t *self) {
    g_list_drains++;
    void *node = ((void *(__fastcall *)(void *))0x59b030)(self);
    while (node) {
        ((void(__fastcall *)(void *))M1)(node);
        node = ((void *(__fastcall *)(void *))0x5d49d0)(node);
    }
}
#define LIST_DRAIN(at, m1, b0, b1, b2, b3, b4, b5, b6, b7, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::ListDrain", at, REWRITE_CAT(original_, at), 8, (void *)&list_drain<m1>, &g_list_drains);
// A pointer-chain getter: a = this->[O0]->[O1]; if (word a->[O2] != 0) return ***(a->[O3]); else trap (unreachable).
static volatile long g_deref_chains;
template <uint32_t O0, uint32_t O1, uint32_t O2, uint32_t O3> static uint32_t __fastcall deref_chain(uint8_t *self) {
    g_deref_chains++;
    uint8_t *a = *(uint8_t **)(*(uint8_t **)(self + O0) + O1);
    if (*(uint16_t *)(a + O2) != 0) {
        uint8_t *p = *(uint8_t **)(a + O3);
        p = *(uint8_t **)p;
        p = *(uint8_t **)p;
        return *(uint32_t *)p;
    }
    return *(volatile uint32_t *)(uintptr_t)0;
}
#define DEREF_CHAIN(at, o0, o1, o2, o3, b0, b1, b2, b3, b4, b5, b6, b7, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::DerefChain", at, REWRITE_CAT(original_, at), 8, (void *)&deref_chain<o0, o1, o2, o3>, &g_deref_chains);
// Call a helper (0xbcde60) with this in ecx and a global in edx, then return this. GLOBAL varies per copy.
static volatile long g_init_edx;
#define INIT_EDX(at, global, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void *__fastcall REWRITE_CAT(ie_, at)(uint8_t *s) { g_init_edx++; ((void(__fastcall *)(void *, void *))0xbcde60)(s, (void *)(uintptr_t)global); return s; } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::InitEdx", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(ie_, at), &g_init_edx);
// Set a table at [this], then call this->[4]'s vtable slot 2 passing it as the argument. TABLE varies per copy.
static volatile long g_set_table_vcalls;
#define SET_TABLE_VCALL(at, table, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void __fastcall REWRITE_CAT(stv_, at)(uint8_t *s) { g_set_table_vcalls++; *(uint32_t *)s = table; uint8_t *obj = *(uint8_t **)(s + 4); ((void(__thiscall *)(void *, void *))(*(void ***)obj)[2])(obj, obj); } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::SetTableVCall", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(stv_, at), &g_set_table_vcalls);
// Forward both arguments to a member method, then clear this->[0x30]. METHOD varies per copy.
static volatile long g_fwd2_clears;
#define FWD2_CLEAR(at, method, b0, b1, b2, b3, b4, b5, b6, b7, ...) static void __thiscall REWRITE_CAT(f2c_, at)(uint8_t *s, uint32_t a0, uint32_t a1) { g_fwd2_clears++; ((void(__thiscall *)(void *, uint32_t, uint32_t))method)(s, a0, a1); *(uint32_t *)(s + 0x30) = 0; } static const uint8_t REWRITE_CAT(original_, at)[] = {b0, b1, b2, b3, b4, b5, b6, b7}; static rewrite::Entry REWRITE_CAT(entry_, at)("Object::Fwd2Clear", at, REWRITE_CAT(original_, at), 8, (void *)&REWRITE_CAT(f2c_, at), &g_fwd2_clears);
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
#define ARRAY_FREE16(at, size, dtor, ...) static const uint8_t REWRITE_CAT(original_, at)[] = {__VA_ARGS__}; static rewrite::Entry REWRITE_CAT(entry_, at)("ObjectArray::Free", at, REWRITE_CAT(original_, at), 8, (void *)&object_array_free<size, dtor, 0x10>, &g_array_frees);
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
#define START_KEY_FULL(...)
#define START_KEY_FULL0(...)
#undef TEXT_WIDEN
#undef TEXT_CREATE
#undef TABLE20_FREE
#define TEXT_WIDEN(...)
#define TEXT_CREATE(...)
#define TEXT_CREATE2(...)
#define TEXT_CREATE3(...)
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
#undef OBJECT_CONSTRUCT
#undef BEHAVIOUR_CREATE
#define OBJECT_CONSTRUCT(...)
#undef MEMBER_DTOR
#define MEMBER_DTOR(...)
#undef GETREF
#define GETREF(...)
#undef SETFIELDS
#define SETFIELDS(...)
#undef FREEFIELD
#define FREEFIELD(...)
#undef TABLES_THEN_BASE
#define TABLES_THEN_BASE(...)
#undef DELETING2
#define DELETING2(...)
#undef FORWARD2
#define FORWARD2(...)
#undef COUNT_LIST
#define COUNT_LIST(...)
#undef CALL_THEN_TAIL
#define CALL_THEN_TAIL(...)
#undef DBG_SINGLETON
#define DBG_SINGLETON(...)
#undef GETFIELD
#define GETFIELD(...)
#undef GLOBAL_MEMBER_CALL
#define GLOBAL_MEMBER_CALL(...)
#undef COND_FORWARD
#define COND_FORWARD(...)
#undef ARRAY_PUSH
#define ARRAY_PUSH(...)
#undef DELMEMBER_FIELD
#define DELMEMBER_FIELD(...)
#undef FREE_TWO_FIELDS
#define FREE_TWO_FIELDS(...)
#undef DBG_WRAP2
#define DBG_WRAP2(...)
#undef CTOR_BASE_ARG
#define CTOR_BASE_ARG(...)
#undef ALLOC_STORE
#define ALLOC_STORE(...)
#undef COND_CALL_SET
#define COND_CALL_SET(...)
#undef FREE_TWO_TABLE
#define FREE_TWO_TABLE(...)
#undef GRID_GROW
#define GRID_GROW(...)
#undef QUAD_COLLECT
#define QUAD_COLLECT(...)
#undef LIST_DRAIN
#define LIST_DRAIN(...)
#undef DEREF_CHAIN
#define DEREF_CHAIN(...)
#undef INIT_EDX
#define INIT_EDX(...)
#undef SET_TABLE_VCALL
#define SET_TABLE_VCALL(...)
#undef FWD2_CLEAR
#define FWD2_CLEAR(...)
#undef TRIVIAL_SETCAP
#define TRIVIAL_SETCAP(...)
#undef PAIR_RELEASE
#define PAIR_RELEASE(...)
#undef SORTED_MAP_CTOR
#define SORTED_MAP_CTOR(...)
#undef INTERFACE_FLOAT
#define INTERFACE_FLOAT(...)
#define BEHAVIOUR_CREATE(...)
#undef INTERFACE_FORWARD
#define INTERFACE_FORWARD(...)
#undef QUADTREE_INSERT
#define QUADTREE_INSERT(...)
#undef QUADTREE_ADD
#define QUADTREE_ADD(...)
#undef LISTENER_DTOR_QUIET
#undef DTOR_TABLES_MEMBER
#undef DTOR_DELETE2
#undef DTOR5_DELETE2
#define LISTENER_DTOR_QUIET(...)
#define DTOR_TABLES_MEMBER(...)
#define DTOR_DELETE2(...)
#define DTOR5_DELETE2(...)
#undef RENDER_STATE
#undef FREE_LIST_TAKE
#undef LISTENER_DTOR
#undef DTOR_MEMBER_IF
#undef LAZY_SINGLETON
#define RENDER_STATE(...)
#define FREE_LIST_TAKE(...)
#define LISTENER_DTOR(...)
#define DTOR_MEMBER_IF(...)
#define LAZY_SINGLETON(...)
#undef LAZY_FACTORY_K1
#undef FACTORY_REGS
#define LAZY_FACTORY_K1(...)
#define FACTORY_REGS(...)
#undef FACTORY_K2
#undef FACTORY_K5
#undef FACTORY_2K
#undef CTOR_BASE_TABLE
#undef SET_TWO_VECTORS
#define FACTORY_K2(...)
#define FACTORY_K5(...)
#define FACTORY_2K(...)
#define CTOR_BASE_TABLE(...)
#define SET_TWO_VECTORS(...)
#undef REF_INTERFACE
#undef DTOR_DELETE_MEMBER
#undef DTOR_MEMBER_BASE
#define REF_INTERFACE(...)
#define DTOR_DELETE_MEMBER(...)
#define DTOR_MEMBER_BASE(...)
#undef FACTORY_K3
#undef GLOBAL_FACTORY_K1
#undef FIELD_FACTORY
#define FACTORY_K3(...)
#define GLOBAL_FACTORY_K1(...)
#define FIELD_FACTORY(...)
#undef CONDITION_NAME
#define CONDITION_NAME(...)
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
#define ARRAY_FREE16(...)

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
#define INTERFACE_FLOAT(at, ref, flag, cache, id, cond, inner, f1, f2, ...) {at, ref, flag, cache, id, cond, inner, f1, f2},
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
#define TEXT_CREATE2(at, text, make, ...) {at, text, make},
#define TEXT_CREATE3(at, text, make, ...) {at, text, make},
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
#undef OBJECT_CONSTRUCT
#undef BEHAVIOUR_CREATE
#define OBJECT_CONSTRUCT(at, size, ctor, dtor, ...) {at, size, ctor, dtor},
#undef MEMBER_DTOR
#define MEMBER_DTOR(...)
#undef GETREF
#define GETREF(...)
#undef SETFIELDS
#define SETFIELDS(...)
#undef FREEFIELD
#define FREEFIELD(...)
#undef TABLES_THEN_BASE
#define TABLES_THEN_BASE(...)
#undef DELETING2
#define DELETING2(...)
#undef FORWARD2
#define FORWARD2(...)
#undef COUNT_LIST
#define COUNT_LIST(...)
#undef CALL_THEN_TAIL
#define CALL_THEN_TAIL(...)
#undef DBG_SINGLETON
#define DBG_SINGLETON(...)
#undef GETFIELD
#define GETFIELD(...)
#undef GLOBAL_MEMBER_CALL
#define GLOBAL_MEMBER_CALL(...)
#undef COND_FORWARD
#define COND_FORWARD(...)
#undef ARRAY_PUSH
#define ARRAY_PUSH(...)
#undef DELMEMBER_FIELD
#define DELMEMBER_FIELD(...)
#undef FREE_TWO_FIELDS
#define FREE_TWO_FIELDS(...)
#undef DBG_WRAP2
#define DBG_WRAP2(...)
#undef CTOR_BASE_ARG
#define CTOR_BASE_ARG(...)
#undef ALLOC_STORE
#define ALLOC_STORE(...)
#undef COND_CALL_SET
#define COND_CALL_SET(...)
#undef FREE_TWO_TABLE
#define FREE_TWO_TABLE(...)
#undef GRID_GROW
#define GRID_GROW(...)
#undef QUAD_COLLECT
#define QUAD_COLLECT(...)
#undef LIST_DRAIN
#define LIST_DRAIN(...)
#undef DEREF_CHAIN
#define DEREF_CHAIN(...)
#undef INIT_EDX
#define INIT_EDX(...)
#undef SET_TABLE_VCALL
#define SET_TABLE_VCALL(...)
#undef FWD2_CLEAR
#define FWD2_CLEAR(...)
#undef TRIVIAL_SETCAP
#define TRIVIAL_SETCAP(at, size, ctor, ...) {at, size, ctor},
#undef PAIR_RELEASE
#define PAIR_RELEASE(...)
#define BEHAVIOUR_CREATE(at, size, ctor, link, ...) {at, size, ctor, link},
#undef INTERFACE_FORWARD
#define INTERFACE_FORWARD(at, ref, flag, cache, id, slot, ...) {at, ref, flag, cache, id, slot},
#undef QUADTREE_INSERT
#define QUADTREE_INSERT(at, add, mark, ...) {at, add, mark},
#undef QUADTREE_ADD
#define QUADTREE_ADD(...)
#undef LISTENER_DTOR_QUIET
#undef DTOR_TABLES_MEMBER
#undef DTOR_DELETE2
#undef DTOR5_DELETE2
#define LISTENER_DTOR_QUIET(at, table, node, ...) {at, table, node},
#undef LISTENER_DTOR
#define LISTENER_DTOR(at, table, node, ...) {at, table, node},
#define DTOR_TABLES_MEMBER(at, t0, t4, member, member_table, member_dtor, base, ...) {at, member, member_dtor, base},
#define DTOR_DELETE2(at, t0, t4, m1, m2, base, ...) {at, m1, m2, base},
#define DTOR5_DELETE2(at, t0, t4, o2, t2, o3, t3, o4, t4b, m1, m2, base, ...) {at, m1, m2, base},
#undef RENDER_STATE
#undef FREE_LIST_TAKE
#undef DTOR_MEMBER_IF
#undef LAZY_SINGLETON
#define RENDER_STATE(at, kept, state, ...) {at, kept, state},
#define FREE_LIST_TAKE(at, base, make, ...) {at, base, make},
#define DTOR_MEMBER_IF(at, member, member_dtor, base, ...) {at, member, member_dtor, base},
#define LAZY_SINGLETON(at, global, size, a, b, c, ctor, ...) {at, global, size, a, b, c, ctor},
#undef LAZY_FACTORY_K1
#undef FACTORY_REGS
#define LAZY_FACTORY_K1(at, global, size, a, ctor, ...) {at, global, size, a, ctor},
#define FACTORY_REGS(at, size, ctor, ...) {at, size, ctor},
#undef FACTORY_K2
#undef FACTORY_K5
#undef FACTORY_2K
#undef CTOR_BASE_TABLE
#undef SET_TWO_VECTORS
#define FACTORY_K2(at, size, ctor, ...) {at, size, ctor},
#define FACTORY_K5(at, size, ctor, ...) {at, size, ctor},
#define FACTORY_2K(at, size, ctor, ...) {at, size, ctor},
#define CTOR_BASE_TABLE(at, base, ...) {at, base},
#define SET_TWO_VECTORS(...)
#undef REF_INTERFACE
#undef DTOR_DELETE_MEMBER
#undef DTOR_MEMBER_BASE
#define REF_INTERFACE(at, ref, id, ...) {at, ref, id},
#define DTOR_DELETE_MEMBER(at, table, table4, member, base, ...) {at, table, table4, member, base},
#define DTOR_MEMBER_BASE(at, member, member_dtor, base, ...) {at, member, member_dtor, base},
#undef FACTORY_K3
#undef GLOBAL_FACTORY_K1
#undef FIELD_FACTORY
#define FACTORY_K3(at, size, ctor, ...) {at, size, ctor},
#define GLOBAL_FACTORY_K1(at, size, ctor, a, global, ...) {at, size, ctor, a, global},
#define FIELD_FACTORY(at, size, ctor, ...) {at, size, ctor},
#undef CONDITION_NAME
#define CONDITION_NAME(...)
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
#undef MEMBER_DTOR
#define MEMBER_DTOR(at, t0, t1, t2, f0, f1, f2, ...) {at, t0, t1, t2, f0, f1, f2},
#undef GETREF
#define GETREF(at, off, ...) {at, off},
#undef FREEFIELD
#define FREEFIELD(at, off, ...) {at, off},
#undef TABLES_THEN_BASE
#define TABLES_THEN_BASE(at, base, ...) {at, base},
#undef DELETING2
#define DELETING2(at, dtor, free, ...) {at, dtor, free},
#undef FORWARD2
#define FORWARD2(at, method, ...) {at, method},
#undef COUNT_LIST
#define COUNT_LIST(...)
#undef CALL_THEN_TAIL
#define CALL_THEN_TAIL(...)
#undef DBG_SINGLETON
#define DBG_SINGLETON(...)
#undef GETFIELD
#define GETFIELD(...)
#undef GLOBAL_MEMBER_CALL
#define GLOBAL_MEMBER_CALL(...)
#undef COND_FORWARD
#define COND_FORWARD(...)
#undef ARRAY_PUSH
#define ARRAY_PUSH(...)
#undef DELMEMBER_FIELD
#define DELMEMBER_FIELD(...)
#undef FREE_TWO_FIELDS
#define FREE_TWO_FIELDS(...)
#undef DBG_WRAP2
#define DBG_WRAP2(...)
#undef CTOR_BASE_ARG
#define CTOR_BASE_ARG(...)
#undef ALLOC_STORE
#define ALLOC_STORE(...)
#undef COND_CALL_SET
#define COND_CALL_SET(...)
#undef FREE_TWO_TABLE
#define FREE_TWO_TABLE(...)
#undef GRID_GROW
#define GRID_GROW(...)
#undef QUAD_COLLECT
#define QUAD_COLLECT(...)
#undef LIST_DRAIN
#define LIST_DRAIN(...)
#undef DEREF_CHAIN
#define DEREF_CHAIN(...)
#undef INIT_EDX
#define INIT_EDX(...)
#undef SET_TABLE_VCALL
#define SET_TABLE_VCALL(...)
#undef FWD2_CLEAR
#define FWD2_CLEAR(...)
#undef SETFIELDS
#define SETFIELDS(...)
#undef CALL_THEN_TAIL
#define CALL_THEN_TAIL(at, a, b, ...) {at, a, b},
#undef DBG_SINGLETON
#define DBG_SINGLETON(at, global, method, ...) {at, global, method},
#undef GETFIELD
#define GETFIELD(at, off, ...) {at, off},
#undef GLOBAL_MEMBER_CALL
#define GLOBAL_MEMBER_CALL(at, global, method, ...) {at, global, method},
#undef COND_FORWARD
#define COND_FORWARD(at, method, ...) {at, method},
#undef ARRAY_PUSH
#define ARRAY_PUSH(at, reserve, ...) {at, reserve},
#undef DELMEMBER_FIELD
#define DELMEMBER_FIELD(at, off, ...) {at, off},
#undef FREE_TWO_FIELDS
#define FREE_TWO_FIELDS(at, off0, off1, ...) {at, off0, off1},
#undef DBG_WRAP2
#define DBG_WRAP2(at, global, method, helper, argimm, ...) {at, global, method, helper, argimm},
#undef CTOR_BASE_ARG
#define CTOR_BASE_ARG(at, base, argimm, table, ...) {at, base, argimm, table},
#undef ALLOC_STORE
#define ALLOC_STORE(at, alloc, ...) {at, alloc},
#undef COND_CALL_SET
#define COND_CALL_SET(at, method, ...) {at, method},
#undef FREE_TWO_TABLE
#define FREE_TWO_TABLE(at, table, off0, off1, ...) {at, table, off0, off1},
#undef GRID_GROW
#define GRID_GROW(at, reserve, m, ...) {at, reserve},
#undef QUAD_COLLECT
#define QUAD_COLLECT(at, coff, ...) {at, coff},
#undef LIST_DRAIN
#define LIST_DRAIN(at, m1, ...) {at, m1},
#undef DEREF_CHAIN
#define DEREF_CHAIN(at, o0, o1, o2, o3, ...) {at, o0, o1, o2, o3},
#undef INIT_EDX
#define INIT_EDX(at, global, ...) {at, global},
#undef SET_TABLE_VCALL
#define SET_TABLE_VCALL(at, table, ...) {at, table},
#undef FWD2_CLEAR
#define FWD2_CLEAR(at, method, ...) {at, method},
static const uint32_t start_table[][10] = {
#include "generated.inc"
};

namespace rewrite {
const uint32_t *start_constants(uintptr_t at) {
    for (auto &c : start_table)
        if (c[0] == at) return c;
    return nullptr;
}
}

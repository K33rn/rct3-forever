// References between game objects. An object is reached through a handle the game shares between everyone who
// refers to it: {..., +8 references, +0xc the object, +0x20 the list of references}. A reference holds the handle
// and is itself linked into that list, so the object knows who refers to it. Each kind of reference is its own
// class, so the compiler made copies of these functions for every one.
#include "rewrite.h"
#include <string.h>

struct ListNode {
    ListNode *prev;
    ListNode *next;
};

struct Handle {
    char unknown[8];
    int32_t references;   // +0x8
    void *object;         // +0xc
    char unknown2[0x10];
    ListNode *refs;       // +0x20: the list's head node
};

struct ObjectRef {
    void **vtable;
    Handle *handle;   // +0x4
    ListNode node;    // +0x8: in handle->refs
    void *owner;      // +0x10: the reference itself, set when it's made
    int32_t extra;    // +0x14
    uint8_t flag;     // +0x18
};
static_assert(__builtin_offsetof(ObjectRef, extra) == 0x14, "layout used by RCT3plus.exe");

typedef void(__fastcall *ReleaseHandle)(Handle *, void *, int32_t);
static const ReleaseHandle release_handle = (ReleaseHandle)0x8055c0;
static Handle *(__cdecl *const null_handle)() = (Handle * (__cdecl *)())0x805750;   // with one more reference

// Leave the current handle's list (a broken list stops at a breakpoint, as in the game) and give the handle up.
static void leave(ObjectRef *r) {
    ListNode *n = &r->node;
    if (n->next && n->prev && n->prev->next == n && n->next->prev == n) {
        n->prev->next = n->next;
        n->next->prev = n->prev;
        n->prev = nullptr;
        n->next = nullptr;
    } else {
        __builtin_debugtrap();
    }
    release_handle(r->handle, nullptr, 0);
}

// Join a handle's list, at its end (just before the head node).
static void join(ObjectRef *r, Handle *h) {
    r->handle = h;
    ListNode *head = h->refs;
    r->node.prev = head->prev;
    head->prev->next = &r->node;
    r->node.next = head;
    head->prev = &r->node;
}

static void changed(ObjectRef *r) { ((void(__fastcall *)(ObjectRef *))r->vtable[0])(r); }   // the class's own reaction

// ---- Construct: a new reference, to nothing (the null handle); returns this ---------------------------------
// One class of reference (vtable 0xf26ae0) has its constructor copied 13 times.

static volatile long g_constructs;

static ObjectRef *__fastcall construct(ObjectRef *r) {
    g_constructs++;
    r->vtable = (void **)0xf26ae0;
    r->owner = r;
    join(r, null_handle());
    return r;
}

static const uint32_t construct_copies[] = {
    0x42e030, 0x6e7580, 0x6fcf20, 0x71f480, 0x746550, 0x762940, 0x77a500, 0x7a3370,
    0x7bd350, 0x876460, 0x876500, 0xa02100, 0xa8dea0,
};
static const uint8_t construct_original[] = {0x56, 0x8b, 0xf1, 0xc7, 0x06, 0xe0, 0x6a, 0xf2,
                                             0x00, 0x89, 0x76, 0x10, 0xe8};
REWRITE_COPIES("ObjectRef::Construct", construct_copies, construct_original, construct, &g_constructs);

// ---- Assign(other): refer to what `other` refers to; returns this ------------------------------------------

static volatile long g_assigns;

static ObjectRef *__fastcall assign(ObjectRef *r, void *, const ObjectRef *other) {
    g_assigns++;
    leave(r);
    other->handle->references++;
    join(r, other->handle);
    changed(r);
    r->extra = other->extra;
    r->flag = other->flag;
    return r;
}

static const uint32_t assign_copies[] = {
    0x410100, 0x44e540, 0x44e5c0, 0x44eaf0, 0x45fa50, 0x460200, 0x4603c0, 0x46d1b0,
    0x48f5c0, 0x4a47b0, 0x4aa760, 0x4aa7e0, 0x4aad80, 0x4b52d0, 0x4c5240, 0x4d2460,
    0x4de510, 0x4defc0, 0x4e6410, 0x5068d0, 0x510780, 0x510800, 0x510890, 0x530fe0,
    0x532a80, 0x560f00, 0x591cc0, 0x591fd0, 0x5a3a30, 0x5b7ad0, 0x5b7b50, 0x5bff60,
    0x5c1d50, 0x5eb870, 0x5eb9b0, 0x6f2270, 0x733a90, 0x738c90, 0x73e100, 0x75ab70,
    0x75ae10, 0x75cd70, 0x771dd0, 0x777660, 0x79a280, 0x7a8670, 0x7c8a90, 0x7cbf50,
    0x8155f0, 0x8156c0, 0x81f530, 0x82e010, 0x8487c0, 0x8b7700, 0x9250b0, 0x981990,
    0x98a550, 0x98a6e0, 0x98a770, 0x98bc00, 0x9ac830, 0x9ac8b0, 0x9bd8f0, 0x9bd970,
    0x9d2640, 0x9d7430, 0xa11350, 0xa11790, 0xa14b70, 0xa30900, 0xa7b900, 0xa85410,
    0xa87450, 0xa8ef10, 0xa9a440, 0xa9a810, 0xaa01d0, 0xaaf2e0, 0xac8ca0, 0xac8f00,
    0xad6ef0, 0xad8380, 0xae1620, 0xaeff30, 0xb153b0, 0xb179b0, 0xb1b930, 0xb1ecb0,
    0xb21200, 0xb24230, 0xb246c0, 0xb2d370, 0xb2fbc0, 0xb368d0, 0xb36950, 0xb37d60,
    0xb44830, 0xb53610, 0xb573d0, 0xb574b0, 0xb63dc0, 0xb87110, 0xb87190, 0xb87210,
    0xb872f0, 0xb89460, 0xb956f0, 0xb9bfa0,
};
static const uint8_t assign_original[] = {0x53, 0x56, 0x57, 0x8b, 0xf9, 0x8d, 0x77, 0x08, 0x85, 0xf6, 0x74, 0x2f};
REWRITE_COPIES("ObjectRef::Assign", assign_copies, assign_original, assign, &g_assigns);

// The same without the flag, for references with no flag.
static volatile long g_assign_extras;
static ObjectRef *__fastcall assign_extra(ObjectRef *r, void *, const ObjectRef *other) {
    g_assign_extras++;
    leave(r);
    other->handle->references++;
    join(r, other->handle);
    changed(r);
    r->extra = other->extra;
    return r;
}
static const uint32_t assign_extra_copies[] = {0x6e75c0, 0x7a3030, 0x8760c0, 0x876190, 0xa8e8f0, 0xab4250};
REWRITE_COPIES("ObjectRef::AssignExtra", assign_extra_copies, assign_original, assign_extra, &g_assign_extras);

// RefArray::Remove(object): an array {data, ?, count} of 20-byte references. Drops the first one to that object:
// each later reference is assigned to the one before it, and the count goes down. The last is left as it was.
struct RefArray {
    uint8_t *data;
    int32_t unknown;
    int32_t count;
};
static volatile long g_ref_removes;
static void __fastcall ref_array_remove(RefArray *a, void *, void *object) {
    g_ref_removes++;
    int32_t count = a->count, i = 0;
    for (;;) {
        if (i >= count) return;
        ObjectRef *r = (ObjectRef *)(a->data + i * 20);
        i++;
        if (r->handle->object == object) break;
    }
    for (; i < a->count; i++) {
        ObjectRef *from = (ObjectRef *)(a->data + i * 20), *to = (ObjectRef *)((uint8_t *)from - 20);
        leave(to);
        from->handle->references++;
        join(to, from->handle);
        changed(to);
    }
    a->count--;
}
static const uint32_t ref_remove_copies[] = {0x731880, 0x739ad0, 0xac3510, 0xba5380};
static const uint8_t ref_remove_original[] = {0x51, 0x53, 0x8b, 0xd9, 0x8b, 0x4b, 0x08, 0x55};
REWRITE_COPIES("RefArray::Remove", ref_remove_copies, ref_remove_original, ref_array_remove, &g_ref_removes);

// ---- DeleteTarget: delete the object referred to (if any) and refer to nothing -------------------------------
// Returns what the class's own reaction returns.

static volatile long g_deletes;

static uint32_t __fastcall delete_target(ObjectRef *r) {
    g_deletes++;
    if (void *object = r->handle->object) {
        // its virtual function 0 with 1: the compiler's "destroy and free"
        ((void(__fastcall *)(void *, void *, int32_t))(*(void ***)object)[0])(object, nullptr, 1);
    }
    leave(r);
    join(r, null_handle());
    return ((uint32_t(__fastcall *)(ObjectRef *))r->vtable[0])(r);
}

static const uint32_t delete_copies[] = {
    0x490800, 0x4ad740, 0x4ad820, 0x4d2c10, 0x512840, 0x533190, 0x564680, 0x5937e0,
    0x593900, 0x5aa9d0, 0x5aaa50, 0x5b7cd0, 0x5c2590, 0x5c2670, 0x5ec480, 0x7622d0,
    0x762490, 0x7a9550, 0x82e1b0, 0x9251d0, 0xa103f0, 0xa8f030, 0xa8f0b0, 0xa9aa60,
    0xaa0a20, 0xaa0aa0, 0xab3d90, 0xabeac0, 0xb45700, 0xb898d0, 0xb899f0,
};
static const uint8_t delete_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8b, 0x47, 0x04, 0x8b, 0x48, 0x0c, 0x85, 0xc9};
REWRITE_COPIES("ObjectRef::DeleteTarget", delete_copies, delete_original, delete_target, &g_deletes);

// ---- LoadField: read a reference stored in a save file -------------------------------------------------------
// Each kind of saved object describes its fields; a reference field's description knows where in the object the
// reference sits. Called as field->Load(object, stream, loader). Loading reads the 8-byte id the save file holds, has the loader turn it into the id of the object
// as loaded now, and points the reference at that object's handle. Returns what the class's own reaction returns.

struct SaveStream {
    void **vtable;
    int32_t position;       // +0x4: where reading is
    int32_t buffer_start;   // +0x8: position of the first buffered byte
    int32_t buffer_end;     // +0xc
    char unknown[4];
    uint8_t buffer[8];      // +0x14: the buffered bytes (longer)
};

struct FieldInfo {
    char unknown[0x28];
    int32_t offset;   // +0x28: where the field sits in the object
};

struct ReferenceField {
    void **vtable;
    FieldInfo *info;   // +0x4
};

typedef void(__fastcall *StreamRead)(SaveStream *, void *, void *into, int32_t size);
static const StreamRead stream_read = (StreamRead)0x809210;   // reads past the buffer
typedef uint64_t(__fastcall *ResolveId)(void *loader, void *, uint32_t id_low, uint32_t id_high);
static const ResolveId resolve_id = (ResolveId)0x808840;
static Handle *(__stdcall *const handle_for)(uint32_t id_low, uint32_t id_high) =
    (Handle * (__stdcall *)(uint32_t, uint32_t))0x805650;   // with one more reference

static volatile long g_loads;

static uint32_t __fastcall load_field(ReferenceField *field, void *, uint8_t *object, SaveStream *stream, void *loader) {
    g_loads++;
    uint32_t id[2];
    int32_t at = stream->position;
    if (at >= stream->buffer_start && at + 8 <= stream->buffer_end) {
        memcpy(id, stream->buffer + (at - stream->buffer_start), 8);
        stream->position = at + 8;
    } else {
        stream_read(stream, nullptr, id, 8);
    }
    ObjectRef *r = (ObjectRef *)(object + field->info->offset);
    uint64_t now = resolve_id(loader, nullptr, id[0], id[1]);
    leave(r);
    join(r, handle_for((uint32_t)now, (uint32_t)(now >> 32)));
    return ((uint32_t(__fastcall *)(ObjectRef *))r->vtable[0])(r);
}

static const uint32_t load_copies[] = {
    0x44eeb0, 0x44ef90, 0x44f070, 0x460c60, 0x46dc30, 0x4723d0, 0x4724b0, 0x472590,
    0x472670, 0x48fc00, 0x4a5890, 0x4adcc0, 0x4adda0, 0x4c1bf0, 0x4c6010, 0x4d2c90,
    0x4ea4a0, 0x507c00, 0x5131c0, 0x5132a0, 0x513380, 0x533680, 0x533760, 0x5339d0,
    0x54da50, 0x54dcb0, 0x54dd90, 0x5649b0, 0x593da0, 0x593e80, 0x5ab100, 0x5b8000,
    0x5c2900, 0x5c2b50, 0x5ec630, 0x5ec710, 0x6f3670, 0x73a030, 0x73ec80, 0x762780,
    0x79a360, 0x7cce80, 0x7ccf60, 0x7cd040, 0x815cd0, 0x815db0, 0x820380, 0x82e660,
    0x84ac30, 0x84ad10, 0x98d170, 0x98d250, 0x98d330, 0x9d7970, 0xa11b50, 0xa153f0,
    0xa31fd0, 0xa7cb40, 0xa87660, 0xa8f3d0, 0xaa9320, 0xab4020, 0xab4100, 0xad8c30,
    0xae31b0, 0xaf2250, 0xb157d0, 0xb17b70, 0xb1bc20, 0xb1ee60, 0xb22850, 0xb24ac0,
    0xb24c90, 0xb2d750, 0xb305f0, 0xb3a9e0, 0xb3aac0, 0xb45990, 0xb45a70, 0xb542f0,
    0xb58d80, 0xb58e60, 0xb63ea0, 0xb8a230, 0xb8a310, 0xb8a3f0, 0xb8a4d0, 0xb8a5b0,
    0xba0150, 0xba0230,
};
static const uint8_t load_original[] = {0x83, 0xec, 0x08, 0x53, 0x55, 0x56, 0x57, 0x8b,
                                        0xf9, 0x8b, 0x4c, 0x24, 0x20, 0x8b, 0x41, 0x04};
REWRITE_COPIES("ReferenceField::Load", load_copies, load_original, load_field, &g_loads);

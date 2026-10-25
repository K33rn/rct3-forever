// References between game objects. An object is reached through a handle the game shares between everyone who
// refers to it: {..., +8 references, +0xc the object, +0x20 the list of references}. A reference holds the handle
// and is itself linked into that list, so the object knows who refers to it. Each kind of reference is its own
// class, so the compiler made copies of these functions for every one.
#include "rewrite.h"

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
    int32_t unknown;  // +0x10: not touched here
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

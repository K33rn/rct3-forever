// Carried items (things a person holds: a camera, a balloon...): made on first use and kept in the person's slot.
#include "wave.h"

// ---- CarriedItem::GetOrMake: the slot at SLOT - 4 {table, handle, link {next, prev}} holds the item's handle; when
// the handle has no object (+0xC) yet, the item is made: its type is looked up by name (NAME, through 0xA2C580), a
// CarriedItemInstance (0xB0 bytes) made of it, and the slot's link moved from the old handle's list to the new one's;
// then the slot is told (its virtual slot 0). Returns the object's carried-item interface: through its virtual slot 1
// and the shared index at 0x1342768, or none ------------------------------------------------------------------------------
static volatile long g_carried_gets;
template <uint32_t SLOT, uintptr_t NAME> static void *__thiscall carried_get_or_make(uint8_t *self) {
    g_carried_gets++;
    uint8_t *slot = self + SLOT - 4;
    if (!*(uint32_t *)(*(uint8_t **)(slot + 4) + 0xc)) {
        void *made = memory_alloc(0x10);
        void *name = made ? ((void *(__thiscall *)(void *, const char *))0xbcf850)(made, (const char *)NAME) : nullptr;
        uint8_t *type = ((uint8_t * (__thiscall *)(void *, void **))0xa2c580)(self, &name);
        int32_t *text = (int32_t *)name;
        if (!--*text) {
            memory_free_aligned((void *)(uintptr_t)text[3]);
            memory_free(text);
        }
        if (type) {
            void *memory = memory_alloc(0xb0);
            void *item = memory ? ((void *(__thiscall *)(void *, int32_t, int32_t, int32_t))0xa29230)(memory, 0, 0, 0) : nullptr;
            ((void(__thiscall *)(void *, uint32_t, int32_t))0xa297d0)(item, *(uint32_t *)(type + 0xbc), 0);
            uint32_t *link = (uint32_t *)(slot + 8);
            uint32_t *next = (uint32_t *)(uintptr_t)link[0], *prev = (uint32_t *)(uintptr_t)link[1];
            if (prev && next && next[1] == (uintptr_t)link && prev[0] == (uintptr_t)link) {
                next[1] = (uint32_t)(uintptr_t)prev;
                prev[0] = (uint32_t)(uintptr_t)next;
                link[0] = link[1] = 0;
            } else {
                __asm__ volatile("int3");   // a broken list: the game stops here too
            }
            ((void(__thiscall *)(void *, int32_t))0x8055c0)(*(void **)(slot + 4), 0);
            uint8_t *handle = ((uint8_t * (__thiscall *)(void *))0x805700)(item);
            *(uint8_t **)(slot + 4) = handle;
            uint32_t *head = *(uint32_t **)(handle + 0x20);
            link[0] = head[0];
            ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)link;
            link[1] = (uint32_t)(uintptr_t)head;
            head[0] = (uint32_t)(uintptr_t)link;
            ((void(__thiscall *)(void *))(*(void ***)slot)[0])(slot);
        }
    }
    uint8_t *object = *(uint8_t **)(*(uint8_t **)(self + SLOT) + 0xc);
    if (!object) return nullptr;
    uint8_t *info = ((uint8_t * (__thiscall *)(void *))(*(void ***)object)[1])(object);
    int32_t index = *(int32_t *)0x1342768;
    if (!info || index >= *(int32_t *)(info + 0x18)) return nullptr;
    uint8_t *part = (*(uint8_t ***)(info + 0x14))[index];
    if (!part) return nullptr;
    return object + *(int32_t *)(part + 0x10);
}
static const uint8_t carried_get_or_make_original[] = {0x64, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x6a, 0xff};
REWRITE("CarriedItem::GetOrMake", 0xa2c990, carried_get_or_make_original, (&carried_get_or_make<0x9c, 0xf3130c>), &g_carried_gets);
REWRITE("CarriedItem::GetOrMake", 0xa2cb40, carried_get_or_make_original, (&carried_get_or_make<0xb0, 0xf8935c>), &g_carried_gets);
REWRITE("CarriedItem::GetOrMake", 0xa2ccf0, carried_get_or_make_original, (&carried_get_or_make<0xc4, 0xf89368>), &g_carried_gets);
REWRITE("CarriedItem::GetOrMake", 0xa2cea0, carried_get_or_make_original, (&carried_get_or_make<0xd8, 0xf89374>), &g_carried_gets);
REWRITE("CarriedItem::GetOrMake", 0xa2d050, carried_get_or_make_original, (&carried_get_or_make<0xec, 0xf89380>), &g_carried_gets);

// ---- HandymanTools::Make: at start-up, a handyman's tool (a brush, a sponge...) is set up on its global from three
// names (the tool, `People\HandyMan\Tools`, the model), then its clean-up registered for the exit ----------------------------
static volatile long g_tool_makes;
static void *make_text(const char *text) {
    void *made = memory_alloc(0x10);
    return made ? ((void *(__thiscall *)(void *, const char *))0xbcf850)(made, text) : nullptr;
}
static void release_text(void *text) {
    int32_t *block = (int32_t *)text;
    if (--*block) return;
    memory_free_aligned((void *)(uintptr_t)block[3]);
    memory_free(block);
}
template <uintptr_t TOOL, uintptr_t MODEL, uintptr_t GLOBAL, uintptr_t CLEAN_UP> static void __cdecl handyman_tool_make() {
    g_tool_makes++;
    void *tool = make_text((const char *)TOOL), *folder = make_text((const char *)0xf6fa0c), *model = make_text((const char *)MODEL);
    ((void(__thiscall *)(void *, int32_t, void **, void **, void **))0x753fe0)((void *)GLOBAL, 0, &model, &folder, &tool);
    release_text(model);
    release_text(folder);
    release_text(tool);
    ((int(__cdecl *)(void *))0xddc870)((void *)CLEAN_UP);
}
static const uint8_t handyman_tool_ed88b0_original[] = {0x6a, 0xff, 0x68, 0xe9, 0x4f, 0xe5, 0x00};
REWRITE("HandymanTools::Make", 0xed88b0, handyman_tool_ed88b0_original, (&handyman_tool_make<0xf6f9ec, 0xf6f9ec, 0x1306b1c, 0xf20140>), &g_tool_makes);
static const uint8_t handyman_tool_ed8a20_original[] = {0x6a, 0xff, 0x68, 0x39, 0x50, 0xe5, 0x00};
REWRITE("HandymanTools::Make", 0xed8a20, handyman_tool_ed8a20_original, (&handyman_tool_make<0xf6f9f4, 0xf6f9f4, 0x1306b64, 0xf20150>), &g_tool_makes);
static const uint8_t handyman_tool_ed8b90_original[] = {0x6a, 0xff, 0x68, 0x89, 0x50, 0xe5, 0x00};
REWRITE("HandymanTools::Make", 0xed8b90, handyman_tool_ed8b90_original, (&handyman_tool_make<0xf6fa2c, 0xf6fa24, 0x1306b7c, 0xf20160>), &g_tool_makes);
static const uint8_t handyman_tool_ed8d00_original[] = {0x6a, 0xff, 0x68, 0xd9, 0x50, 0xe5, 0x00};
REWRITE("HandymanTools::Make", 0xed8d00, handyman_tool_ed8d00_original, (&handyman_tool_make<0xf6fa34, 0xf6fa00, 0x1306b4c, 0xf20170>), &g_tool_makes);
static const uint8_t handyman_tool_ed8e70_original[] = {0x6a, 0xff, 0x68, 0x29, 0x51, 0xe5, 0x00};
REWRITE("HandymanTools::Make", 0xed8e70, handyman_tool_ed8e70_original, (&handyman_tool_make<0xf6fa44, 0xf6fa40, 0x1306b00, 0xf20180>), &g_tool_makes);

// ---- Ref::PassTo(object): a reference to the object (a handle linked into its list, table REF_TABLE, flag 1) is made on
// the stack and given to the member at +MEMBER (ADD); then the reference is let go (0x40F730) and the object set up
// (0x800860) unless it already is (bit 0 of +0x2C) ------------------------------------------------------------------------------
struct StackRef {
    uint32_t table;
    uint8_t *handle;
    uint32_t link[2];
    void *self;   // the reference itself
    void *object;
    uint8_t flag;
};
static volatile long g_ref_passes;
template <uintptr_t REF_TABLE, uint32_t MEMBER, uintptr_t ADD> static void __thiscall ref_pass_to(uint8_t *self, void *object) {
    g_ref_passes++;
    StackRef ref;
    ref.table = 0xf26ae0;
    ref.self = &ref;
    ref.handle = ((uint8_t * (__thiscall *)(void *))0x805700)(object);
    uint32_t *head = *(uint32_t **)(ref.handle + 0x20);
    ref.link[0] = head[0];
    ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)ref.link;
    ref.link[1] = (uint32_t)(uintptr_t)head;
    head[0] = (uint32_t)(uintptr_t)ref.link;
    ref.table = REF_TABLE;
    ref.object = object;
    ref.flag = 1;
    ((void(__thiscall *)(void *, StackRef *))ADD)(self + MEMBER, &ref);
    ((void(__thiscall *)(StackRef *))0x40f730)(&ref);
    if (!(self[0x2c] & 1)) ((void(__thiscall *)(void *))0x800860)(self);
}
static const uint8_t ref_pass_5d7b10_original[] = {0x6a, 0xff, 0x68, 0xc8, 0x57, 0xe2, 0x00};
REWRITE("Ref::PassTo", 0x5d7b10, ref_pass_5d7b10_original, (&ref_pass_to<0xf3e11c, 0x50, 0x5a3a30>), &g_ref_passes);
static const uint8_t ref_pass_7f76b0_original[] = {0x6a, 0xff, 0x68, 0xe8, 0xad, 0xe5, 0x00};
REWRITE("Ref::PassTo", 0x7f76b0, ref_pass_7f76b0_original, (&ref_pass_to<0xf6c748, 0x44, 0x75cd70>), &g_ref_passes);
static const uint8_t ref_pass_b59810_original[] = {0x6a, 0xff, 0x68, 0xe8, 0x65, 0xe9, 0x00};
REWRITE("Ref::PassTo", 0xb59810, ref_pass_b59810_original, (&ref_pass_to<0xf97c7c, 0x68, 0xb573d0>), &g_ref_passes);

// NumSceneryCondition::ClassificationName and NumItemsBoughtCondition::ModeName (one function written out twice; they
// differ in their four texts): a condition's kind written as its name (fastcall: where to put the text, the kind;
// gives that place).
//   - Kind 0, 1 or 2: its key looked up (a text reference made on the stack, 0x4040F0 with the key, the game's text
//     table and 1); the text it holds copied (0xBCF7D0; none without memory), or the empty text when it holds none;
//     the reference destroyed (0xBD1880) after the text is put.
//   - Any other kind: a text made from the copy's "unknown" literal (0xBCF850; none without memory).
#include "rewrite.h"

namespace {
volatile long g_kind_names;
template <uint32_t KEY0, uint32_t KEY1, uint32_t KEY2, uint32_t UNKNOWN> void **__fastcall kind_name(void **out, uint32_t kind) {
    g_kind_names++;
    if (kind > 2) {
        void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
        *out = memory ? ((void *(__thiscall *)(void *, const char *))0xbcf850)(memory, (const char *)(uintptr_t)UNKNOWN) : nullptr;
        return out;
    }
    static const uint32_t keys[3] = {KEY0, KEY1, KEY2};
    alignas(16) uint8_t reference[0x40];
    uint8_t *made = (uint8_t *)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x4040f0)(reference, keys[kind], 0x1303cfc, 1);
    void *held = *(void **)(made + 8);
    void *text;
    if (held) {
        void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
        text = memory ? ((void *(__thiscall *)(void *, void *))0xbcf7d0)(memory, held) : nullptr;
    } else {
        text = ((void *(__fastcall *)(uint32_t))0xbd0490)(1);
    }
    *out = text;
    ((void(__thiscall *)(void *))0xbd1880)(reference);
    return out;
}

const uint8_t kind_name_first[][7] = {{0x6a, 0xff, 0x68, 0x24, 0x48, 0xe4, 0x00}, {0x6a, 0xff, 0x68, 0x94, 0x48, 0xe4, 0x00}};
}   // namespace
REWRITE("Condition::KindName", 0x6df770, kind_name_first[0], (&kind_name<0xf65744, 0xf65764, 0xf65784, 0xf657a4>), &g_kind_names);
REWRITE("Condition::KindName", 0x6dfb50, kind_name_first[1], (&kind_name<0xf657bc, 0xf657e8, 0xf65810, 0xf65840>), &g_kind_names);

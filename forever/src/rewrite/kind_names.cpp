// NumSceneryCondition::ClassificationName, NumItemsBoughtCondition::ModeName, RideBreakdown::Names and
// PlaceObject::ErrorText, and AnimalCamera::ModeNames as a stdcall (one function
// written out with different keys): a kind written as its name (fastcall: where to put the text, the kind; gives that
// place).
//   - A kind from the copy's first (0 or 1) and below its count past that (compared unsigned): its key looked up (a text reference made on the stack, 0x4040F0
//     with the key, the copy's text table and 1); the text it holds copied (0xBCF7D0; none without memory), or the empty
//     text when it holds none; the reference destroyed (0xBD1880) after the text is put.
//   - Any other kind: the copy's default, either looked up the same way, or a text made from a literal (0xBCF850; none
//     without memory).
#include "rewrite.h"

namespace {
volatile long g_kind_names;
void looked_up(void **out, uint32_t key, uint32_t table) {
    alignas(16) uint8_t reference[0x40];
    uint8_t *made = (uint8_t *)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x4040f0)(reference, key, table, 1);
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
}

template <const uint32_t *KEYS, uint32_t COUNT, uint32_t TABLE, uint32_t DEFAULT, bool DEFAULT_IS_KEY, uint32_t FIRST = 0>
void **__fastcall kind_name(void **out, uint32_t kind) {
    g_kind_names++;
    if (kind - FIRST < COUNT) {
        looked_up(out, KEYS[kind - FIRST], TABLE);
    } else if (DEFAULT_IS_KEY) {
        looked_up(out, DEFAULT, TABLE);
    } else {
        void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
        *out = memory ? ((void *(__thiscall *)(void *, const char *))0xbcf850)(memory, (const char *)(uintptr_t)DEFAULT) : nullptr;
    }
    return out;
}

// (the same, its two values on the stack: stdcall)
template <const uint32_t *KEYS, uint32_t COUNT, uint32_t TABLE, uint32_t DEFAULT, bool DEFAULT_IS_KEY>
void **__stdcall kind_name_stack(void **out, uint32_t kind) {
    return kind_name<KEYS, COUNT, TABLE, DEFAULT, DEFAULT_IS_KEY>(out, kind);
}

constexpr uint32_t scenery_keys[] = {0xf65744, 0xf65764, 0xf65784};
constexpr uint32_t bought_keys[] = {0xf657bc, 0xf657e8, 0xf65810};
constexpr uint32_t breakdown_keys[] = {0xf380b4, 0xf38094, 0xf38068, 0xf3803c, 0xf38018, 0xf37ff0, 0xf37fc4, 0xf37fa0, 0xf37f80};
constexpr uint32_t camera_keys[] = {0xf9b4a8, 0xf9b4bc, 0xf9b4d0};
constexpr uint32_t place_error_keys[] = {0xf77af4, 0xf77abc, 0xf77adc, 0xf77aa8, 0xf77a8c, 0xf77a6c, 0xf77a4c};   // (kinds 1 to 7)
const uint8_t kind_name_first[][7] = {{0x6a, 0xff, 0x68, 0x24, 0x48, 0xe4, 0x00}, {0x6a, 0xff, 0x68, 0x94, 0x48, 0xe4, 0x00}, {0x6a, 0xff, 0x68, 0xf0, 0xb0, 0xe1, 0x00},
                                     {0x6a, 0xff, 0x68, 0xe1, 0x0d, 0xe6, 0x00}, {0x6a, 0xff, 0x68, 0xb4, 0x72, 0xe9, 0x00}};
}   // namespace
REWRITE("Condition::KindName", 0x6df770, kind_name_first[0], (&kind_name<scenery_keys, 3, 0x1303cfc, 0xf657a4, false>), &g_kind_names);
REWRITE("Condition::KindName", 0x6dfb50, kind_name_first[1], (&kind_name<bought_keys, 3, 0x1303cfc, 0xf65840, false>), &g_kind_names);
REWRITE("Condition::KindName", 0x532110, kind_name_first[2], (&kind_name<breakdown_keys, 9, 0x12f4c04, 0xf264ad, true>), &g_kind_names);
REWRITE("Condition::KindName", 0x844220, kind_name_first[3], (&kind_name<place_error_keys, 7, 0x1303cfc, 0xf264ad, false, 1>), &g_kind_names);   // PlaceObject::ErrorText
REWRITE("Condition::KindName", 0xb6fe70, kind_name_first[4], (&kind_name_stack<camera_keys, 3, 0x1303cfc, 0xf264ad, false>), &g_kind_names);   // AnimalCamera::ModeNames (stdcall)

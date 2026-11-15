// Animal::ValueA32200, Animal::ValueA32320 and Animal::ValueA32440 (one function written out three times;
// tools/family.py: animal_values.inc; they differ in the two fields they use): one of an animal's values, a float
// (thiscall; in st(0)).
//   - It starts as the record's (+0x9C) first field (+0x34, +0x38, +0x3C), kept as it is.
//   - For each item of the record's list (+0x5C count, +0x60 items; count and record read again each time; an item
//     out of range is none): the item found through the animal's interface (+0x94, looked up once and kept, +0x98;
//     as farther_point.cpp's: its reference at +0x84, class number [0x12F5494]) with 0x591E60. When the animal has
//     the item's bit set (+0x60, bit number i mod 32) or what was found has kind 5 (+0x54): the kind's weight (0: 0,
//     1: 0.5, 2: 1, 3: 1.2, 4: 1.5, 5: 1, others 0; the game's own float constants) times the found one's second
//     field (+0x58's +0x10, +0x14, +0x18) added on (on the FPU, kept as a float).
#include "rewrite.h"

namespace {
struct AnimalValue {
    uint32_t at, first, second;
};
#define ANIMAL_VALUES(at, first, second, ...) {at, first, second},
constexpr AnimalValue animal_values[] = {
#include "animal_values.inc"
};
#undef ANIMAL_VALUES
#define ANIMAL_VALUES(at, first, second, ...) {__VA_ARGS__},
const uint8_t animal_values_first[][7] = {
#include "animal_values.inc"
};
#undef ANIMAL_VALUES
typedef long double ext;

// An interface kept by its holder: looked up once (the flag set), then read back (as farther_point.cpp's).
uint8_t *interface_of(uint8_t *reference, uint32_t class_global, uint8_t *volatile *kept, volatile uint8_t *looked) {
    if (!*looked) {
        uint8_t *object = *(uint8_t **)(reference + 0xc), *found = nullptr;
        if (object) {
            uint8_t *info = ((uint8_t * (__thiscall *)(void *))(*(void ***)object)[1])(object);
            if (info) {
                const int32_t number = *(volatile int32_t *)(uintptr_t)class_global;
                if (number < *(int32_t *)(info + 0x18)) {
                    uint8_t *entry = (*(uint8_t ***)(info + 0x14))[number];
                    if (entry) found = object + *(int32_t *)(entry + 0x10);
                }
            }
        }
        *kept = found;
        *looked = 1;
    }
    return *kept;
}
// a kind's weight: the game's float constants, by the address each case uses
const uintptr_t weights[6] = {0xf264e0, 0xf2651c, 0xf264dc, 0xf27068, 0xf26648, 0xf264dc};   // 0, 0.5, 1, 1.2, 1.5, 1

volatile long g_animal_values;
float animal_value(uint8_t *self, const AnimalValue &kind) {
    g_animal_values++;
    volatile float sum;
    *(volatile uint32_t *)&sum = *(volatile uint32_t *)(*(uint8_t **)(self + 0x9c) + kind.first);   // (kept as it is)
    for (int32_t i = 0; i < *(volatile int32_t *)(*(uint8_t **)(self + 0x9c) + 0x5c); i++) {
        uint8_t *record = *(uint8_t **)(self + 0x9c);
        const uint32_t item = i >= 0 && i < *(int32_t *)(record + 0x5c) ? (*(uint32_t **)(record + 0x60))[i] : 0;
        uint8_t *to = interface_of(*(uint8_t **)(self + 0x84), 0x12f5494, (uint8_t * volatile *)(self + 0x94), self + 0x98);
        const uint8_t *found = ((const uint8_t *(__thiscall *)(void *, uint32_t))0x591e60)(to, item);
        const uint32_t kind_of = *(const uint32_t *)(found + 0x54);
        if ((*(volatile uint32_t *)(self + 0x60) & (1u << (i & 31))) || kind_of == 5) {
            const float weight = *(const float *)(kind_of <= 5 ? weights[kind_of] : 0xf264e0);
            const float *values = *(const float *const *)(found + 0x58);
            sum = (float)((ext)weight * *(const float *)((const uint8_t *)values + kind.second) + sum);
        }
    }
    return sum;
}

template <int K> float __fastcall animal_value_copy(uint8_t *self) { return animal_value(self, animal_values[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(animal_values[0].at == 0xa32200 && animal_values[1].at == 0xa32320 && animal_values[2].at == 0xa32440, "animal_values.inc changed");
REWRITE("Animal::Value", 0xa32200, animal_values_first[0], (&animal_value_copy<0>), &g_animal_values);
REWRITE("Animal::Value", 0xa32320, animal_values_first[1], (&animal_value_copy<1>), &g_animal_values);
REWRITE("Animal::Value", 0xa32440, animal_values_first[2], (&animal_value_copy<2>), &g_animal_values);

// The constructors at 0x4D60A0 and 0xB80020 (one function written out twice; tools/family.py: titled_lists.inc; they
// differ in their default description and tables): a window given a reference by value (thiscall: a description or
// none, a title or none, a value x, a reference (0x14 bytes, owned and destroyed here), a value y; gives the window).
//   - Titled (titled_head.h) and built (0x6AAE60: the description, that title, y).
//   - x kept at +0x114; the two tables set.
//   - A reference of its own at +0x118 to the same handle (class 0xF26AE0, itself its owner; the handle's count at +8
//     raised; joined first in the handle's list).
//   - With no description given, the default marked used; the reference given destroyed (ObjectRef's 0x40F730).
#include "titled_head.h"

namespace {
struct TitledList {
    uint32_t at, description, table0, table4;
};
#define TITLED_LISTS(at, description, t0, t4, ...) {at, description, t0, t4},
constexpr TitledList titled_lists[] = {
#include "titled_lists.inc"
};
#undef TITLED_LISTS
#define TITLED_LISTS(at, description, t0, t4, ...) {__VA_ARGS__},
const uint8_t titled_lists_first[][7] = {
#include "titled_lists.inc"
};
#undef TITLED_LISTS
struct Reference {   // an object reference as it is passed by value: {class, handle, list links, owner}
    uint32_t w[5];
};

volatile long g_titled_lists;
uint8_t *titled_list(uint8_t *self, uint8_t *description, int32_t **title, uint32_t x, Reference *given, uint32_t y, const TitledList &kind) {
    g_titled_lists++;
    int32_t *volatile copy;
    void *named = titled::title_for(kind.description, title, &copy);
    ((void *(__thiscall *)(void *, void *, void *, uint32_t))0x6aae60)(self, titled::description_or_default(description, kind.description), named, y);
    titled::title_done(title, &copy);
    *(volatile uint32_t *)(self + 0x114) = x;
    *(volatile uint32_t *)(self + 0) = kind.table0;
    *(volatile uint32_t *)(self + 4) = kind.table4;
    uint32_t *own = (uint32_t *)(self + 0x118);
    own[0] = 0xf26ae0;
    own[4] = (uint32_t)(uintptr_t)own;
    uint8_t *handle = (uint8_t *)(uintptr_t)given->w[1];
    ++*(volatile int32_t *)(handle + 8);
    own[1] = (uint32_t)(uintptr_t)handle;
    uint32_t *head = *(uint32_t **)(handle + 0x20);
    own[2] = head[0];
    ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)&own[2];
    own[3] = (uint32_t)(uintptr_t)head;
    head[0] = (uint32_t)(uintptr_t)&own[2];
    if (!description) (*(uint8_t *volatile *)(uintptr_t)kind.description)[4] = 1;
    ((void(__thiscall *)(void *))0x40f730)(given);
    return self;
}

template <int K> uint8_t *__thiscall titled_list_copy(uint8_t *self, uint8_t *description, int32_t **title, uint32_t x, Reference given, uint32_t y) {
    return titled_list(self, description, title, x, &given, y, titled_lists[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(titled_lists[0].at == 0x4d60a0 && titled_lists[1].at == 0xb80020, "titled_lists.inc changed");
REWRITE("Window::BuildWithReference", 0x4d60a0, titled_lists_first[0], (&titled_list_copy<0>), &g_titled_lists);
REWRITE("Window::BuildWithReference", 0xb80020, titled_lists_first[1], (&titled_list_copy<1>), &g_titled_lists);

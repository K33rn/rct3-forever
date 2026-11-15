// Window::Title634820 and Window::Title64AC90 (one function written out twice; tools/family.py: details_titles.inc;
// they differ in their default description, the object's part given, their tables and the game value they set):
// build a details window for an object (thiscall: a description or none, a title or none, a value, the object; gives
// the window).
//   - Titled (titled_head.h) and its curved background built (DetailsWindow::BuildBackground, 0x669750) with the
//     description, that title, the value, the object's part (+0x108 or +0x278) and a reference to the object passed
//     by value (made where the call reads it: in_place_call.h).
//   - Its three tables set (+0, +4, +0xD8); with no description given, the default marked used; the copy's game value
//     set ([0x10EEAB0] = 10, or [0x10EEB74] = 1).
#include "in_place_call.h"
#include "titled_head.h"

namespace {
struct DetailsTitle {
    uint32_t at, description, part, table0, table4, table_d8, global, value;
};
#define DETAILS_TITLES(at, description, part, t0, t4, td8, global, value, ...) {at, description, part, t0, t4, td8, global, value},
constexpr DetailsTitle details_titles[] = {
#include "details_titles.inc"
};
#undef DETAILS_TITLES
#define DETAILS_TITLES(at, description, part, t0, t4, td8, global, value, ...) {__VA_ARGS__},
const uint8_t details_titles_first[][7] = {
#include "details_titles.inc"
};
#undef DETAILS_TITLES

struct BackgroundArguments {
    uint32_t description, named, value;
    uint8_t *object;
    uint32_t part;
};
void __cdecl fill_background(uint8_t *area, void *context) {
    const BackgroundArguments &a = *(const BackgroundArguments *)context;
    in_place::reference_to((uint32_t *)(area + 0x10), a.object);   // (made first, as the game does)
    uint32_t *words = (uint32_t *)area;
    words[0] = a.description, words[1] = a.named, words[2] = a.value, words[3] = (uint32_t)(uintptr_t)(a.object + a.part);
}

volatile long g_details_titles;
uint8_t *details_title(uint8_t *self, uint8_t *description, int32_t **title, uint32_t value, uint8_t *object, const DetailsTitle &kind) {
    g_details_titles++;
    int32_t *volatile copy;
    void *named = titled::title_for(kind.description, title, &copy);
    BackgroundArguments a = {(uint32_t)(uintptr_t)titled::description_or_default(description, kind.description), (uint32_t)(uintptr_t)named, value, object,
                             kind.part};
    call_in_place(0x669750, self, 0x24, fill_background, &a);
    titled::title_done(title, &copy);
    *(volatile uint32_t *)(self + 0) = kind.table0;
    *(volatile uint32_t *)(self + 4) = kind.table4;
    *(volatile uint32_t *)(self + 0xd8) = kind.table_d8;
    if (!description) (*(uint8_t *volatile *)(uintptr_t)kind.description)[4] = 1;
    *(volatile uint32_t *)(uintptr_t)kind.global = kind.value;
    return self;
}

template <int K> uint8_t *__thiscall details_title_copy(uint8_t *self, uint8_t *description, int32_t **title, uint32_t value, uint8_t *object) {
    return details_title(self, description, title, value, object, details_titles[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(details_titles[0].at == 0x634820 && details_titles[1].at == 0x64ac90, "details_titles.inc changed");
REWRITE("DetailsWindow::Build", 0x634820, details_titles_first[0], (&details_title_copy<0>), &g_details_titles);
REWRITE("DetailsWindow::Build", 0x64ac90, details_titles_first[1], (&details_title_copy<1>), &g_details_titles);

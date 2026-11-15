// AttractionResultsDialog::Labels2, AttractionResultsDialog::Labels and PoolInfo::Labels (one function written out
// three times; tools/family.py: result_labels.inc; they differ in their statics, prefix, text table and size): a
// dialog's label for a name (the name's text holder; gives the label, or none without memory).
//   - Three function statics, each made once (the copy's flags global, bits 1, 2, 4): " - " as a wide text (none
//     without memory; its clean-up registered with the C library's atexit), and two colours 0xFF000000.
//   - The label (0xE4 new bytes): the copy's prefix (e.g. "AttractionResultsDialogLabel_") as a wide text joined to
//     the name (0x403BB0), the result copied (0xBD01C0) and looked up as a named text in the copy's table with 1
//     (NamedTextRef's constructor); the label built (0x615EE0) with it, the static text's and colours' addresses,
//     the copy's size, 1, 0, 0. Then the lookup destroyed and the three texts let go (newest first).
#include "rewrite.h"

namespace {
struct ResultLabel {
    uint32_t at, flags, cleanup, dash, colour1, colour2, prefix, table, size;
};
#define RESULT_LABELS(at, flags, cleanup, dash, colour1, colour2, prefix, table, size, ...) {at, flags, cleanup, dash, colour1, colour2, prefix, table, size},
constexpr ResultLabel result_labels[] = {
#include "result_labels.inc"
};
#undef RESULT_LABELS
#define RESULT_LABELS(at, flags, cleanup, dash, colour1, colour2, prefix, table, size, ...) {__VA_ARGS__},
const uint8_t result_labels_first[][7] = {
#include "result_labels.inc"
};
#undef RESULT_LABELS

typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc = (Alloc)0xbcdfc0;
inline void let_go(int32_t *volatile *held) {   // a shared text let go (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
typedef void *(__thiscall *FromC)(void *, uintptr_t);

volatile long g_result_labels;
void *result_label(void *name, const ResultLabel &kind) {
    g_result_labels++;
    volatile uint32_t &flags = *(volatile uint32_t *)(uintptr_t)kind.flags;
    if (!(flags & 1)) {
        flags = flags | 1;
        void *memory = alloc(0x10);
        *(void *volatile *)(uintptr_t)kind.dash = memory ? ((FromC)0xbcf850)(memory, 0xf2738c) : nullptr;   // " - "
        ((int(__cdecl *)(uintptr_t))0xddc870)(kind.cleanup);   // atexit
    }
    if (!(flags & 2)) {
        flags = flags | 2;
        *(volatile uint32_t *)(uintptr_t)kind.colour1 = 0xff000000;
    }
    if (!(flags & 4)) {
        flags = flags | 4;
        *(volatile uint32_t *)(uintptr_t)kind.colour2 = 0xff000000;
    }
    void *label = alloc(0xe4);
    if (!label) return nullptr;
    void *memory = alloc(0x10);
    int32_t *volatile prefix = memory ? (int32_t *)((FromC)0xbcf850)(memory, kind.prefix) : nullptr;
    int32_t *volatile joined, *volatile copy;
    void *j = ((void *(__thiscall *)(void *, void *, void *))0x403bb0)((void *)&prefix, (void *)&joined, name);
    ((void *(__thiscall *)(void *, void *))0xbd01c0)((void *)&copy, j);
    alignas(4) uint8_t reference[0x18];
    void *r = ((void *(__thiscall *)(void *, void *, uint32_t, uint32_t))0x410c90)(reference, (void *)&copy, kind.table, 1);
    typedef void *(__thiscall * Make)(void *, void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void *made = ((Make)0x615ee0)(label, r, kind.dash, kind.size, kind.colour1, kind.colour2, 1, 0, 0);
    ((void(__thiscall *)(void *))0xbd1880)(reference);
    let_go(&copy);
    let_go(&joined);
    let_go(&prefix);
    return made;
}

template <int K> void *__stdcall result_label_copy(void *name) { return result_label(name, result_labels[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(result_labels[0].at == 0x55d300 && result_labels[1].at == 0x66c7d0 && result_labels[2].at == 0xa82a40, "result_labels.inc changed");
REWRITE("Dialog::Label", 0x55d300, result_labels_first[0], (&result_label_copy<0>), &g_result_labels);
REWRITE("Dialog::Label", 0x66c7d0, result_labels_first[1], (&result_label_copy<1>), &g_result_labels);
REWRITE("Dialog::Label", 0xa82a40, result_labels_first[2], (&result_label_copy<2>), &g_result_labels);

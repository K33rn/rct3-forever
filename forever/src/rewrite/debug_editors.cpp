// The debug console's number editors: DebugEditor::Int, Int32, UInt32, Float, Int16, UInt16, Int8, UInt8, and three
// with no name (their base window given an empty text). One constructor in the game written out eleven times (tools/debug_editors.py checks they differ only in their name, class table, range
// record table and value size, and writes those to debug_editors.inc). It takes a label (C text, or none), a width, the
// value's place, its lowest and highest values and step, and whether to show a slider:
//   - the editor's base window (0xC69EB0) is made with a text made from its name, then that text is let go, and its
//     class table set;
//   - a row (0xF0 bytes) holding the label: the label's text (a copy of it, or the shared empty text when there is none)
//     shown in an Arial font (0xC661D0, the GUI's font size), in a window part (0xC699C0) of the width given, added;
//   - when asked for, a slider (0xD0 bytes, 0xC6B400) given a range record, added;
//   - an edit field (0xF0 bytes, 0xC699C0) holding a font record (0x100 bytes, 0xC686C0) given a range record, added;
//   - the temporary texts let go, last made first.
// A range record is {its table, the value's place, lowest, highest, step}, the last three as many bytes as the value has.
// Each new block may be missing (no memory): what it would have held is skipped and nothing (0) is added in its place.
#include "rewrite.h"

namespace {
struct EditorKind {
    uint32_t at, name, table, range_table, size;
};
#define EDITOR(at, name, table, range_table, size, ...) {at, name, table, range_table, size},
constexpr EditorKind editor_kinds[] = {
#include "debug_editors.inc"
};
#undef EDITOR
#define EDITOR(at, name, table, range_table, size, ...) {__VA_ARGS__},
const uint8_t editor_first[][7] = {
#include "debug_editors.inc"
};
#undef EDITOR

typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc = (Alloc)0xbcdfc0;

// a shared text {count, ..., buffer at +0xC} let go: the count taken down; at 0 its buffer freed, then it
inline void let_go(int32_t *volatile *held) {
    --**held;
    int32_t *text = *held;   // read again, as the game does
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
inline void *text_from(const char *c_text, void *memory) { return ((void *(__thiscall *)(void *, const char *))0xbcf850)(memory, c_text); }
inline void empty_text(int32_t *volatile *into) { ((void(__thiscall *)(void *, uint32_t))0xbd05b0)((void *)into, 0); }
inline void *add_child(uint8_t *self, void *child) { return ((void *(__thiscall *)(void *, void *))0xc69f00)(self, child); }
typedef void *(__thiscall *Part)(void *, void *, void *, const uint32_t *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

// a range record (none without memory): its table, the value's place, then lowest, highest and step of `size` bytes each
void *range_record(const EditorKind &kind, uint32_t target, uint32_t low, uint32_t high, uint32_t step) {
    uint8_t *r = (uint8_t *)alloc(kind.size == 4 ? 0x14 : kind.size == 2 ? 0x10 : 0xc);
    if (!r) return nullptr;
    *(uint32_t *)(r + 4) = target;
    if (kind.size == 4) {
        *(uint32_t *)(r + 8) = low, *(uint32_t *)(r + 0xc) = high, *(uint32_t *)(r + 0x10) = step;
    } else if (kind.size == 2) {
        *(uint16_t *)(r + 8) = (uint16_t)low, *(uint16_t *)(r + 0xa) = (uint16_t)high, *(uint16_t *)(r + 0xc) = (uint16_t)step;
    } else {
        r[8] = (uint8_t)low, r[9] = (uint8_t)high, r[0xa] = (uint8_t)step;
    }
    *(uint32_t *)r = kind.range_table;
    return r;
}

volatile long g_editors;
uint8_t *editor(uint8_t *self, const EditorKind &kind, const char *label, uint32_t width, uint32_t target, uint32_t low, uint32_t high, uint32_t step,
                uint8_t slider) {
    g_editors++;
    // the base window, named (or given an empty text, for the three editors with no name)
    int32_t *volatile name;
    if (kind.name) {
        void *memory = alloc(0x10);
        name = memory ? (int32_t *)text_from((const char *)(uintptr_t)kind.name, memory) : nullptr;
    } else {
        empty_text(&name);
    }
    ((void *(__thiscall *)(void *, void *, uint32_t, uint32_t, uint32_t))0xc69eb0)(self, (void *)&name, 4, 0x3f800000, 0);
    let_go(&name);
    *(volatile uint32_t *)self = kind.table;
    // the label's row
    int32_t *volatile label_text, *volatile row_text;
    bool have_label = false, have_row = false;
    void *child = nullptr;
    if (uint8_t *row = (uint8_t *)alloc(0xf0)) {
        void *font = nullptr;
        if (uint8_t *box = (uint8_t *)alloc(0xf0)) {
            if (label) {
                void *m = alloc(0x10);
                label_text = m ? (int32_t *)text_from(label, m) : nullptr;
            } else {
                label_text = ((int32_t * (__fastcall *)(uint32_t))0xbd0490)(1);
            }
            have_label = true;
            font = ((void *(__thiscall *)(void *, void *, uint32_t, uint32_t))0xc661d0)(box, (void *)&label_text, *(volatile uint32_t *)0x10f3434, 0x1000);
        }
        empty_text(&row_text);
        have_row = true;
        const uint32_t size[2] = {width, 0};
        child = ((Part)0xc699c0)(row, (void *)&row_text, font, size, 3, 0, 0, 0xff202020, 0);
    }
    add_child(self, child);
    if (have_row) let_go(&row_text);
    if (have_label) let_go(&label_text);
    // the slider
    if (slider) {
        int32_t *volatile slider_text;
        bool have_slider = false;
        void *part = nullptr;
        if (uint8_t *made = (uint8_t *)alloc(0xd0)) {
            void *range = range_record(kind, target, low, high, step);
            empty_text(&slider_text);
            have_slider = true;
            const uint32_t size[2] = {0x3d4ccccd, 0x3ca3d70a};   // 0.05, 0.02
            part = ((void *(__thiscall *)(void *, void *, void *, const uint32_t *, uint32_t, uint32_t))0xc6b400)(made, (void *)&slider_text, range, size,
                                                                                                                   0x3c83126f, 0x14);   // 0.016
        }
        add_child(self, part);
        if (have_slider) let_go(&slider_text);
    }
    // the edit field
    int32_t *volatile edit_text, *volatile field_text;
    bool have_edit = false, have_field = false;
    void *field = nullptr;
    if (uint8_t *made = (uint8_t *)alloc(0xf0)) {
        void *font = nullptr;
        if (uint8_t *record = (uint8_t *)alloc(0x100)) {
            void *range = range_record(kind, target, low, high, step);
            empty_text(&edit_text);
            have_edit = true;
            font = ((void *(__thiscall *)(void *, void *, uint32_t, void *, uint32_t, uint32_t))0xc686c0)(record, (void *)&edit_text, 0x10, range, 0xff000000, 0x80);
        }
        empty_text(&field_text);
        have_field = true;
        const uint32_t size[2] = {0x3d8f5c29, 0};   // 0.07
        field = ((Part)0xc699c0)(made, (void *)&field_text, font, size, 0x21, 0x3f800000, 0, 0xff202020, 0);
    }
    add_child(self, field);
    if (have_field) let_go(&field_text);
    if (have_edit) let_go(&edit_text);
    return self;
}

template <int K>
uint8_t *__thiscall debug_editor(uint8_t *self, const char *label, uint32_t width, uint32_t target, uint32_t low, uint32_t high, uint32_t step, uint32_t slider) {
    return editor(self, editor_kinds[K], label, width, target, low, high, step, (uint8_t)slider);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(editor_kinds[0].at == 0xca88d0, "debug_editors.inc changed");
static_assert(editor_kinds[1].at == 0xca8dc0, "debug_editors.inc changed");
static_assert(editor_kinds[2].at == 0xca92b0, "debug_editors.inc changed");
static_assert(editor_kinds[3].at == 0xcaab80, "debug_editors.inc changed");
static_assert(editor_kinds[4].at == 0xca97a0, "debug_editors.inc changed");
static_assert(editor_kinds[5].at == 0xca9ca0, "debug_editors.inc changed");
static_assert(editor_kinds[6].at == 0xcaa1a0, "debug_editors.inc changed");
static_assert(editor_kinds[7].at == 0xcaa690, "debug_editors.inc changed");
static_assert(editor_kinds[8].at == 0xcabc00 && editor_kinds[9].at == 0xcac0d0 && editor_kinds[10].at == 0xcac5a0, "debug_editors.inc changed");
REWRITE("DebugEditor::Number", 0xca88d0, editor_first[0], (&debug_editor<0>), &g_editors);
REWRITE("DebugEditor::Number", 0xca8dc0, editor_first[1], (&debug_editor<1>), &g_editors);
REWRITE("DebugEditor::Number", 0xca92b0, editor_first[2], (&debug_editor<2>), &g_editors);
REWRITE("DebugEditor::Number", 0xcaab80, editor_first[3], (&debug_editor<3>), &g_editors);
REWRITE("DebugEditor::Number", 0xca97a0, editor_first[4], (&debug_editor<4>), &g_editors);
REWRITE("DebugEditor::Number", 0xca9ca0, editor_first[5], (&debug_editor<5>), &g_editors);
REWRITE("DebugEditor::Number", 0xcaa1a0, editor_first[6], (&debug_editor<6>), &g_editors);
REWRITE("DebugEditor::Number", 0xcaa690, editor_first[7], (&debug_editor<7>), &g_editors);
REWRITE("DebugEditor::Number", 0xcabc00, editor_first[8], (&debug_editor<8>), &g_editors);
REWRITE("DebugEditor::Number", 0xcac0d0, editor_first[9], (&debug_editor<9>), &g_editors);
REWRITE("DebugEditor::Number", 0xcac5a0, editor_first[10], (&debug_editor<10>), &g_editors);

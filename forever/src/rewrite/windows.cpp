// Windows built from a shared title: the constructors of the game's window classes all start the same way (their text
// made from the title and the text given, the base window built) and end by setting their tables and a few members.
#include "wave.h"

static void release_wide(void *text) {   // a wide string's shared block: freed when no one has it (no check for none)
    int32_t *block = (int32_t *)text;
    if (--*block) return;
    memory_free_aligned((void *)(uintptr_t)block[3]);
    memory_free(block);
}

// What a window class's constructor does after the base window: store a table or value (kind 0: a dword, 1: a byte),
// store its third argument (2), or build a member at +off with the function at value (3). In order.
struct WindowOp {
    uint32_t kind, off, value;
};

// ---- Window::Construct(parent, text, value): when there is a text, it becomes the title's name (made from the title
// object's +8, or the empty string), WideString::AssignC'd from the separator 0xF2738C and AssignWide'd with the text,
// let go as the game does; with none, a copy of the title. The base window (0x6AF7E0) is built with the parent (or the
// title object) and that text; then the class's own ops; with no parent, the title object is marked used (+4; MARK) ----
static volatile long g_window_constructs;
template <uintptr_t TITLE, const WindowOp *OPS, uint32_t COUNT, bool MARK = true>
static void *window_construct_body(uint8_t *self, void *parent, void **text, uint32_t value) {
    g_window_constructs++;
    uint8_t *title = *(uint8_t **)TITLE;
    void *made = nullptr;   // the copy of the title when there's no text
    bool copied = false;
    void *used;
    if (text) {
        void *name;
        const char *title_name = *(const char **)(title + 8);
        if (title_name) {
            void *memory = memory_alloc(0x10);
            name = memory ? ((void *(__thiscall *)(void *, const char *))0xbcf850)(memory, title_name) : nullptr;
        } else {
            name = ((void *(__fastcall *)(uint32_t))0xbd0490)(1);
        }
        void *with_separator = nullptr, *joined = nullptr;
        void *first = ((void *(__thiscall *)(void *, void **, const char *))0x403be0)(&name, &with_separator, (const char *)0xf2738c);
        void **result = (void **)((void *(__thiscall *)(void *, void **, void **))0x403bb0)(first, &joined, text);
        if (*result != *text) {
            int32_t *old = (int32_t *)*text;
            --*old;
            old = (int32_t *)*text;
            if (!*old && old) {
                memory_free_aligned((void *)(uintptr_t)old[3]);
                memory_free(old);
            }
            *text = *result;
            ++*(int32_t *)*result;
        }
        release_wide(joined);
        release_wide(with_separator);
        release_wide(name);
        used = text;
    } else {
        used = ((void *(__thiscall *)(void **, void *))0x41c740)(&made, title);
        copied = true;
    }
    ((void(__thiscall *)(void *, void *, void *))0x6af7e0)(self, parent ? parent : *(void **)TITLE, used);
    if (copied) release_wide(made);
    for (uint32_t k = 0; k < COUNT; k++) {
        const WindowOp &op = OPS[k];
        if (op.kind == 0) *(uint32_t *)(self + op.off) = op.value;
        else if (op.kind == 1) self[op.off] = (uint8_t)op.value;
        else if (op.kind == 2) *(uint32_t *)(self + op.off) = value;
        else ((void(__thiscall *)(void *))(uintptr_t)op.value)(self + op.off);
    }
    if (MARK && !parent) (*(uint8_t **)TITLE)[4] = 1;
    return self;
}
// The classes' constructors take (parent, text, value) or just (parent, text).
template <uintptr_t TITLE, const WindowOp *OPS, uint32_t COUNT, bool MARK = true>
static void *__thiscall window_construct(uint8_t *self, void *parent, void **text, uint32_t value) {
    return window_construct_body<TITLE, OPS, COUNT, MARK>(self, parent, text, value);
}
template <uintptr_t TITLE, const WindowOp *OPS, uint32_t COUNT, bool MARK = true>
static void *__thiscall window_construct2(uint8_t *self, void *parent, void **text) {
    return window_construct_body<TITLE, OPS, COUNT, MARK>(self, parent, text, 0);
}
static const WindowOp window_467a80_ops[] = {{3, 0xd4, 0x441030}, {0, 0x0, 0xf2a9d8}, {0, 0x4, 0xf2a9c4}, {0, 0xd4, 0xf2a9c0}, {0, 0xe4, 0x0}};
static const uint8_t window_467a80_original[] = {0x6a, 0xff, 0x68, 0xb4, 0xc2, 0xe0, 0x00};
REWRITE("Window::Construct", 0x467a80, window_467a80_original, (&window_construct2<0x12efff4, window_467a80_ops, 5>), &g_window_constructs);
static const WindowOp window_503d10_ops[] = {{0, 0x0, 0xf36a00}, {0, 0x4, 0xf369ec}, {1, 0xdc, 0x0}};
static const uint8_t window_503d10_original[] = {0x6a, 0xff, 0x68, 0xf4, 0x90, 0xe1, 0x00};
REWRITE("Window::Construct", 0x503d10, window_503d10_original, (&window_construct2<0x12f5054, window_503d10_ops, 3>), &g_window_constructs);
static const WindowOp window_643800_ops[] = {{0, 0x0, 0xf49cf0}, {0, 0x4, 0xf49cd8}, {2, 0xd4, 0x0}};
static const uint8_t window_643800_original[] = {0x6a, 0xff, 0x68, 0x94, 0x1e, 0xe3, 0x00};
REWRITE("Window::Construct", 0x643800, window_643800_original, (&window_construct<0x12f7f04, window_643800_ops, 3>), &g_window_constructs);
static const WindowOp window_659fc0_ops[] = {{0, 0x0, 0xf4d278}, {0, 0x4, 0xf4d260}, {2, 0xd4, 0x0}};
static const uint8_t window_659fc0_original[] = {0x6a, 0xff, 0x68, 0x74, 0x56, 0xe3, 0x00};
REWRITE("Window::Construct", 0x659fc0, window_659fc0_original, (&window_construct<0x12f8098, window_659fc0_ops, 3>), &g_window_constructs);
static const WindowOp window_6a0d40_ops[] = {{0, 0x0, 0xf5e720}, {0, 0x4, 0xf5e708}, {2, 0xd4, 0x0}, {0, 0xd8, 0xffffffff}};
static const uint8_t window_6a0d40_original[] = {0x6a, 0xff, 0x68, 0x94, 0xcc, 0xe3, 0x00};
REWRITE("Window::Construct", 0x6a0d40, window_6a0d40_original, (&window_construct<0x1305c28, window_6a0d40_ops, 4>), &g_window_constructs);
static const WindowOp window_6ac730_ops[] = {{0, 0x0, 0xf60538}, {0, 0x4, 0xf60524}, {1, 0xd4, 0x0}, {0, 0xd8, 0xffffffff}};
static const uint8_t window_6ac730_original[] = {0x6a, 0xff, 0x68, 0xc4, 0xe0, 0xe3, 0x00};
REWRITE("Window::Construct", 0x6ac730, window_6ac730_original, (&window_construct2<0x1305ce8, window_6ac730_ops, 4>), &g_window_constructs);
static const WindowOp window_9ab3d0_ops[] = {{0, 0x0, 0xf82558}, {0, 0x4, 0xf82540}, {2, 0xd4, 0x0}};
static const uint8_t window_9ab3d0_original[] = {0x6a, 0xff, 0x68, 0x34, 0x16, 0xe7, 0x00};
REWRITE("Window::Construct", 0x9ab3d0, window_9ab3d0_original, (&window_construct<0x1341428, window_9ab3d0_ops, 3>), &g_window_constructs);
static const WindowOp window_b01e70_ops[] = {{0, 0x0, 0xf970c0}, {0, 0x4, 0xf970ac}, {1, 0xdc, 0x0}};
static const uint8_t window_b01e70_original[] = {0x6a, 0xff, 0x68, 0x14, 0xeb, 0xe8, 0x00};
REWRITE("Window::Construct", 0xb01e70, window_b01e70_original, (&window_construct2<0x13ce690, window_b01e70_ops, 3>), &g_window_constructs);

static const WindowOp window_4efdf0_ops[] = {{0, 0x0, 0xf35a00}, {0, 0x4, 0xf359ec}};
static const uint8_t window_4efdf0_original[] = {0x6a, 0xff, 0x68, 0xf4, 0x6f, 0xe1, 0x00};
REWRITE("Window::Construct", 0x4efdf0, window_4efdf0_original, (&window_construct2<0x12f8144, window_4efdf0_ops, 2, false>), &g_window_constructs);
static const WindowOp window_6180b0_ops[] = {{0, 0x0, 0xf430c8}, {0, 0x4, 0xf430b0}, {2, 0xd4, 0x0}};
static const uint8_t window_6180b0_original[] = {0x6a, 0xff, 0x68, 0x14, 0xb4, 0xe2, 0x00};
REWRITE("Window::Construct", 0x6180b0, window_6180b0_original, (&window_construct<0x12f5778, window_6180b0_ops, 3, false>), &g_window_constructs);
static const WindowOp window_687480_ops[] = {{0, 0x0, 0xf51f78}, {0, 0x4, 0xf51f60}};
static const uint8_t window_687480_original[] = {0x6a, 0xff, 0x68, 0x54, 0xa6, 0xe3, 0x00};
REWRITE("Window::Construct", 0x687480, window_687480_original, (&window_construct2<0x12f845c, window_687480_ops, 2, false>), &g_window_constructs);
static const WindowOp window_6b49a0_ops[] = {{0, 0x0, 0xf61138}, {0, 0x4, 0xf61120}, {2, 0xd4, 0x0}};
static const uint8_t window_6b49a0_original[] = {0x6a, 0xff, 0x68, 0x64, 0xec, 0xe3, 0x00};
REWRITE("Window::Construct", 0x6b49a0, window_6b49a0_original, (&window_construct<0x1305d78, window_6b49a0_ops, 3, false>), &g_window_constructs);



































































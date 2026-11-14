// Window::Message442C60, 442FD0, 443340, 4436B0, 443B70: one function written out five times (tools/family.py checks they
// differ only in the six game globals in messages.inc, their jump tables sending each index to the same case). Given a
// number (0 to 4, anything else taking the last), it makes the message for it: the wide text the number's global points
// to (a copy of it, or the shared empty text when there is none), with every "#" in it replaced by the number written
// out ("%d", the game's sprintf). The result is given to the caller (ecx: where it goes; the number in edx).
#include "rewrite.h"

namespace {
#define MESSAGES(at, p0, p1, p2, p3, p4, p5, ...) {at, {p0, p1, p2, p3, p4, p5}},
struct MessageKind {
    uint32_t at, texts[6];
};
constexpr MessageKind message_kinds[] = {
#include "messages.inc"
};
#undef MESSAGES
#define MESSAGES(at, p0, p1, p2, p3, p4, p5, ...) {__VA_ARGS__},
const uint8_t message_first[][7] = {
#include "messages.inc"
};
#undef MESSAGES

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
inline int32_t *empty_text() { return ((int32_t * (__fastcall *)(uint32_t))0xbd0490)(1); }
inline int32_t *text_c(const char *c_text) {
    void *memory = alloc(0x10);
    return memory ? ((int32_t * (__thiscall *)(void *, const char *))0xbcf850)(memory, c_text) : nullptr;
}

volatile long g_messages;
int32_t **message(int32_t **out, uint32_t number, const MessageKind &kind) {
    g_messages++;
    int32_t *volatile text = empty_text();
    const void *characters = *(const void *volatile *)(uintptr_t)kind.texts[number > 4 ? 5 : number];
    let_go(&text);
    if (characters) {
        void *memory = alloc(0x10);
        text = memory ? ((int32_t * (__thiscall *)(void *, const void *))0xbcf7d0)(memory, characters) : nullptr;
    } else {
        text = empty_text();
    }
    char written[0x20];
    ((int(__cdecl *)(char *, const char *, ...))0xddcdb5)(written, (const char *)0xf28e5c, number);   // "%d"
    int32_t *volatile number_text = text_c(written);
    int32_t *volatile mark = text_c((const char *)0xf28e58);   // "#"
    ((void(__fastcall *)(void *, void *, void *))0xcd23a0)((void *)&text, (void *)&mark, (void *)&number_text);   // every "#" replaced
    let_go(&mark);
    let_go(&number_text);
    *out = text;
    ++*text;
    let_go(&text);
    return out;
}

template <int K>
int32_t **__fastcall message_copy(int32_t **out, uint32_t number) {
    return message(out, number, message_kinds[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(message_kinds[0].at == 0x442c60 && message_kinds[1].at == 0x442fd0 && message_kinds[2].at == 0x443340 && message_kinds[3].at == 0x4436b0 &&
                  message_kinds[4].at == 0x443b70,
              "messages.inc changed");
REWRITE("Window::MessageNumbered", 0x442c60, message_first[0], (&message_copy<0>), &g_messages);
REWRITE("Window::MessageNumbered", 0x442fd0, message_first[1], (&message_copy<1>), &g_messages);
REWRITE("Window::MessageNumbered", 0x443340, message_first[2], (&message_copy<2>), &g_messages);
REWRITE("Window::MessageNumbered", 0x4436b0, message_first[3], (&message_copy<3>), &g_messages);
REWRITE("Window::MessageNumbered", 0x443b70, message_first[4], (&message_copy<4>), &g_messages);

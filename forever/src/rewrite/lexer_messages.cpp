// ShaderPreprocessorLexer::Append2 and ShaderPreprocessorLexer::Append (one function written out twice; they differ in
// the kind: ") : warning : " counted at +0x18, or ") : error : " at +0x14): add a message to the lexer's text
// (cdecl: the lexer, a printf format, its values).
//   - The lexer's text (+0xC, a narrow shared text: its length at +4, its characters at +0xC) gets in turn: the file's
//     name (the text at +0x10, its length), "(", the line ([0x10F45C8], written by the C library's _itoa in base 10),
//     the kind, the message (_vsnprintf into 1024 bytes, its last made 0) and a new line.
//   - Each part added as the game's inlined append does: room made for length + n + 1 (0xBD02B0), at most n
//     characters copied (up to a 0; where they come from read after the room is made), a 0 after them, the length
//     made n more (n: the part's length).
//   - The kind's count one more.
#include "rewrite.h"
#include <stdarg.h>

namespace {
struct Narrow {
    int32_t count;
    uint32_t length, room;
    char *characters;
};
template <typename Part> inline void append(Narrow **text, uint32_t n, Part where) {
    ((void(__thiscall *)(void *, uint32_t))0xbd02b0)((void *)text, (*text)->length + n + 1);
    char *to = (*text)->characters + (*text)->length;
    const char *part = where();
    uint32_t i = 0;
    for (; i < n && part[i]; i++) to[i] = part[i];
    to[i] = 0;
    (*text)->length += n;
}
inline uint32_t length_of(const char *c) {
    uint32_t n = 0;
    while (c[n]) n++;
    return n;
}
inline void append_c(Narrow **text, uintptr_t game_text) {
    const char *c = (const char *)game_text;
    append(text, length_of(c), [c] { return c; });
}

volatile long g_lexer_messages;
template <uintptr_t KIND, uint32_t COUNT> void __cdecl lexer_message(uint8_t *self, const char *format, ...) {
    g_lexer_messages++;
    Narrow **text = (Narrow **)(self + 0xc);
    append(text, (*(Narrow **)(self + 0x10))->length, [self] { return (const char *)(*(Narrow *volatile *)(self + 0x10))->characters; });
    append_c(text, 0xf9bcd4);   // "("
    char buffer[0x400];
    const char *line = ((char *(__cdecl *)(int32_t, char *, int32_t))0xde07de)(*(volatile int32_t *)(uintptr_t)0x10f45c8, buffer, 10);
    append(text, length_of(line), [line] { return line; });
    append_c(text, KIND);
    va_list values;
    va_start(values, format);
    ((int(__cdecl *)(char *, uint32_t, const char *, va_list))0xde0014)(buffer, 0x400, format, values);
    va_end(values);
    buffer[0x3ff] = 0;
    append(text, length_of(buffer), [&buffer] { return (const char *)buffer; });
    append_c(text, 0xf27cb4);   // a new line
    ++*(int32_t *)(self + COUNT);
}

const uint8_t lexer_message_first[] = {0x81, 0xec, 0x04, 0x04, 0x00, 0x00, 0xa1};
}   // namespace
REWRITE("ShaderLexer::Message", 0xcbbe90, lexer_message_first, (&lexer_message<0xfc2de8, 0x18>), &g_lexer_messages);   // ") : warning : "
REWRITE("ShaderLexer::Message", 0xcbc140, lexer_message_first, (&lexer_message<0xfc2df8, 0x14>), &g_lexer_messages);   // ") : error : "

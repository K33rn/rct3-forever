// ShaderLexer::Message (forever/src/rewrite/lexer_messages.cpp). The lexer at arena 0x100: its text (+0xC) a narrow text
// at 0x1000 (0-20 characters, held 1-2 times), the file's name (+0x10) another one at 0x1040 or the same text. The
// room maker's stand-in (0xBD02B0) notes what it is asked for and, when the room is short or others hold the text,
// gives a sole-held text new characters in place (the old ones scribbled over) or a shared one a text of its own,
// from 0x2000 on (dev_take); _itoa and _vsnprintf are stand-ins (the C
// library's own snprintf / vsnprintf); the line ([0x10F45C8]) set in the call and put back after.
#if EQUIV_PART == 1
static uint32_t g_lm_saved;
struct LmNarrow {
    int32_t count;
    uint32_t length, room;
    char *characters;
};
static void __thiscall lm_room(LmNarrow **text, uint32_t need) {
    dev_note(680, need);
    LmNarrow *t = *text;
    if (t->count > 1) {   // held by others: a text of its own
        LmNarrow *made = (LmNarrow *)dev_take(0x10);
        char *characters = (char *)dev_take(0x100);
        for (uint32_t k = 0; k <= t->length; k++) characters[k] = t->characters[k];
        made->count = 1, made->length = t->length, made->room = need + 8, made->characters = characters;
        --t->count;
        *text = made;
    } else if (t->room < need) {   // its own, short of room: new characters in place, the old ones scribbled over (freed)
        char *characters = (char *)dev_take(0x100);
        for (uint32_t k = 0; k <= t->length; k++) characters[k] = t->characters[k];
        for (uint32_t k = 0; k <= t->length; k++) t->characters[k] = '#';
        t->characters = characters, t->room = need + 8;
    }
}
static char *__cdecl lm_itoa(int32_t value, char *out, int32_t base) {
    dev_note(681, (uint32_t)value); dev_note(681, (uint32_t)base);
    snprintf(out, 16, "%d", value);
    return out;
}
static int __cdecl lm_vsnprintf(char *out, uint32_t room, const char *format, va_list values) {
    dev_note(682, room);
    return vsnprintf(out, room, format, values);
}
static uint32_t lm_text_sum(const LmNarrow *t) {
    uint32_t h = t->length * 7 + (uint32_t)t->count;
    for (uint32_t k = 0; k <= t->length && k < 0x100; k++) h = h * 33 + (uint8_t)t->characters[k];
    return h;
}
#else
    Test lexer_messages{0xcbbe90,
                        [=](Rng &r) {
                            fresh(r);
                            arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                            for (uint32_t at : {0x1000u, 0x1040u}) {
                                LmNarrow *t = arena<LmNarrow>(at);
                                t->count = r.range(1, 2);
                                t->length = r.range(0, 20);
                                t->room = r.range(0, 2) ? t->length + r.range(1, 30) : t->length + 1;
                                t->characters = arena<char>(at + 0x400);
                                for (uint32_t k = 0; k < t->length; k++) t->characters[k] = (char)('a' + r.range(0, 25));
                                t->characters[t->length] = 0;
                                if (t->length > 2 && r.range(0, 4) == 0) t->characters[r.range(1, (int32_t)t->length - 1)] = 0;   // (a 0 inside sometimes)
                            }
                            *(void **)(arena<uint8_t>(0x100) + 0xc) = arena<void>(0x1000);
                            *(void **)(arena<uint8_t>(0x100) + 0x10) = r.range(0, 4) ? arena<void>(0x1040) : arena<void>(0x1000);
                            arena<int32_t>(0x50)[0] = r.range(0, 1) ? r.range(-5, 9999) : (int32_t)r.next();
                            arena<int32_t>(0x50)[1] = (int32_t)r.next();
                        },
                        [](void *fn) {
                            *(int32_t *)(uintptr_t)0x10f45c8 = arena<int32_t>(0x50)[0];
                            ((void(__cdecl *)(void *, const char *, ...))fn)(arena<void>(0x100), "value %d and %s", arena<int32_t>(0x50)[1], "word");
                            const LmNarrow *t = *(LmNarrow **)(arena<uint8_t>(0x100) + 0xc);
                            return Result{lm_text_sum(t) ^ *arena<uint32_t>(0x20)};
                        }};
    lexer_messages.iterations = 1000;
    lexer_messages.prepare = [](const rewrite::Entry *) {
        g_lm_saved = *(uint32_t *)(uintptr_t)0x10f45c8;
        stubs_for_now({{0xbd02b0, (void *)lm_room}, {0xde07de, (void *)lm_itoa}, {0xde0014, (void *)lm_vsnprintf}});
    };
    lexer_messages.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)0x10f45c8 = g_lm_saved;
    };
    quick(lexer_messages);
#endif

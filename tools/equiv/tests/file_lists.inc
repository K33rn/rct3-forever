// Window::FillFiles (forever/src/rewrite/file_lists.cpp). The window at arena 0x100 (its list at 0x400, the folder's
// text holder at +0x198); texts {count, length, room, characters} from dev_take; the search's entries at 0x7000.
#if EQUIV_PART == 1
static uint32_t g_fl_import;
static uint32_t *fl_text(uint32_t count, uint32_t length) {   // a text with room to spare (3 bytes a character and more)
    uint32_t *t = (uint32_t *)dev_take(0x10);
    uint16_t *c = (uint16_t *)dev_take(0x80);
    t[0] = count, t[1] = length, t[2] = length + (dev_next() % 3), t[3] = (uint32_t)(uintptr_t)c;
    for (uint32_t i = 0; i < length + 2; i++) {
        const uint32_t k = dev_next();
        c[i] = (uint16_t)(k % 7 == 0 ? 0 : k % 5 == 0 ? 0x100 + (k & 0x7ff) : 0x20 + (k & 0x5f));
    }
    if (dev_next() & 1) c[length] = 0;
    return t;
}
static void fl_note_text(uint32_t tag, const void *held) {   // a holder's text: where, count, length
    const uint32_t *t = *(const uint32_t *const *)held;
    dev_note(tag, (uint32_t)(uintptr_t)t);
    dev_note(tag, t[0]); dev_note(tag, t[1]);
}
static void __fastcall fl_fill(void *self) { dev_note(170, (uint32_t)(uintptr_t)self); }
static void __fastcall fl_empty(void *list) { dev_note(171, (uint32_t)(uintptr_t)list); }
static void *__thiscall fl_path(void *self, uint32_t **out) {
    dev_note(172, (uint32_t)(uintptr_t)self);
    *out = fl_text(1 + dev_next() % 2, dev_next() % 13);
    return out;
}
static void __thiscall fl_unique(uint32_t **held, uint32_t room) {
    dev_note(173, room); fl_note_text(173, held);
    if ((*held)[2] < room) (*held)[2] = room;
}
static int __stdcall fl_make_folder(const char *path, void *security) {
    dev_note(174, (uint32_t)(uintptr_t)security);
    for (const char *p = path;; p++) { dev_note(174, (uint8_t)*p); if (!*p) break; }
    return 1;
}
static void *__thiscall fl_pattern(void *self, uint32_t **out) { dev_note(175, (uint32_t)(uintptr_t)self); *out = fl_text(1, dev_next() % 5); return out; }
static void *__thiscall fl_copy(uint32_t **out, const void *from) { fl_note_text(176, from); *out = fl_text(1, (*(uint32_t *const *)from)[1]); return out; }
// the search: its object (at +4) is at 0x7000 {table, count, position, folder flags...}
static uint8_t __fastcall fl_more(uint32_t *s) { dev_note(177, s[2]); return s[2] < s[1]; }
static uint8_t __fastcall fl_is_folder(uint32_t *s) { dev_note(178, s[2]); return (uint8_t)s[3 + s[2] % 8]; }
static void __fastcall fl_next(uint32_t *s) { dev_note(179, s[2]); s[2]++; }
static void *fl_search_table[5] = {nullptr, (void *)fl_more, (void *)fl_next, nullptr, (void *)fl_is_folder};
static void __thiscall fl_search(void **search, const void *pattern, const void *folder) {
    fl_note_text(180, pattern); fl_note_text(180, folder);
    uint32_t *s = arena<uint32_t>(0x7000);
    s[2] = 0;
    *(void **)s = fl_search_table;
    search[0] = (void *)(uintptr_t)0x5ea7c4;
    search[1] = s;
}
static void *__thiscall fl_name(void **search, uint32_t **out) { dev_note(181, (uint32_t)(uintptr_t)search[0]); *out = fl_text(1, arena<uint32_t>(0x7000)[2] % 9); return out; }
static void *__thiscall fl_name_copy(uint32_t **out, const void *from) { fl_note_text(182, from); *out = fl_text(1, 3); return out; }
static void __thiscall fl_add(void *list, uint32_t at, const void *pattern, const void *name, uint32_t one) {
    dev_note(183, (uint32_t)(uintptr_t)list); dev_note(183, at); fl_note_text(183, pattern); fl_note_text(183, name); dev_note(183, one);
}
static void __fastcall fl_search_end(void **search) { dev_note(184, (uint32_t)(uintptr_t)search[0]); }
static void __thiscall fl_count(void *list, uint32_t n) { dev_note(185, (uint32_t)(uintptr_t)list); dev_note(185, n); }
static void __fastcall fl_redraw(void *list) { dev_note(186, (uint32_t)(uintptr_t)list); }
static void *fl_list_table[0xe4 / 4 + 1];
#else
    Test file_lists{0x680160,
                    [=](Rng &r) {
                        fresh(r);
                        for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                        fl_list_table[0xe4 / 4] = (void *)fl_redraw;
                        *(void **)arena<void>(0x400) = fl_list_table;
                        *(void **)(arena<uint8_t>(0x100) + 0x168) = arena<void>(0x400);
                        uint32_t *s = arena<uint32_t>(0x7000);
                        s[1] = r.range(0, 7);
                        for (int k = 0; k < 8; k++) s[3 + k] = r.range(0, 2) == 0 ? 1 + (r.next() & 0xff00) : r.next() & 0xff00;   // (only the low byte counts)
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                        *(uint32_t **)(arena<uint8_t>(0x100) + 0x198) = fl_text(1 + r.range(0, 1), r.range(0, 9));
                    },
                    [](void *fn) {
                        ((void(__fastcall *)(void *))fn)(arena<void>(0x100));
                        return Result{*arena<uint32_t>(0x20)};
                    }};
    file_lists.iterations = 500;
    file_lists.prepare = [](const rewrite::Entry *) {
        g_fl_import = *(uint32_t *)(uintptr_t)0xf2615c;
        *(void **)(uintptr_t)0xf2615c = (void *)fl_make_folder;   // the CreateDirectoryA import
        stubs_for_now({{0x687df0, (void *)fl_fill}, {0x685ac0, (void *)fl_empty}, {0x687720, (void *)fl_path}, {0xbcffe0, (void *)fl_unique},
                       {0x6879a0, (void *)fl_pattern}, {0xbd01c0, (void *)fl_copy}, {0xbe5570, (void *)fl_search}, {0x4042d0, (void *)fl_name},
                       {0xbcfef0, (void *)fl_name_copy}, {0x68c370, (void *)fl_add}, {0xbe47b0, (void *)fl_search_end}, {0x685a20, (void *)fl_count},
                       {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}});
    };
    file_lists.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)0xf2615c = g_fl_import;
    };
    quick(file_lists);
#endif

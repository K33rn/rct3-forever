// Info::SetText (forever/src/rewrite/info_set_texts.cpp). The info at arena 0x100 with its text (+0x68 and +0x5C) a
// dn_ text held 1-3 times; the text given (held at 0x60) a new one held 1-2 times (empty sometimes) or the info's own; the replacer's
// stand-in leaves an empty text empty, else changes it (a new copy when others hold it); the default's stand-in puts in its place a new text or the info's own (held once more) and
// gives that place; the second call notes the info. Texts from C are dn_ texts.
#if EQUIV_PART == 1
static void *__thiscall ist_default(void *self, DnText **into) {
    dev_note(630, (uint32_t)((uintptr_t)self - (uintptr_t)arena<void>(0)));
    if (arena<uint32_t>(0x50)[0]) {
        *into = *(DnText **)(arena<uint8_t>(0x100) + arena<uint32_t>(0x50)[1]);
        (*into)->count++;
    } else {
        const uint16_t wide[2] = {'d', 'f'};
        *into = dn_make((DnText *)dev_take(0x10), wide, 2);
    }
    return into;
}
static void __fastcall ist_replace(DnText **text, DnText **find, DnText **with) {   // (an empty text stays empty)
    dev_note(632, dn_hash(*text)); dev_note(632, dn_hash(*find)); dev_note(632, dn_hash(*with)); dev_note(632, (uint32_t)(*text)->count);
    DnText *t = *text;
    if (!t->length) return;
    if (t->count > 1) {
        --t->count;
        *text = dn_make((DnText *)dev_take(0x10), t->characters, t->length);
    }
    (*text)->characters[0] = (uint16_t)((*text)->characters[0] + 1);
}
static void __fastcall ist_after(void *self) { dev_note(631, (uint32_t)((uintptr_t)self - (uintptr_t)arena<void>(0))); }
#else
    Test info_set_texts{0xa8ac30,
                        [=](Rng &r) {
                            fresh(r);
                            arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                            for (uint32_t off : {0x68u, 0x5cu}) {
                                const uint16_t wide[3] = {'o', 'l', 'd'};
                                DnText *text = dn_make((DnText *)dev_take(0x10), wide, 3);
                                text->count = r.range(1, 3);
                                *(DnText **)(arena<uint8_t>(0x100) + off) = text;
                            }
                            arena<uint32_t>(0x50)[0] = r.range(0, 2) == 0;
                            arena<uint32_t>(0x50)[1] = r.range(0, 1) ? 0x68 : 0x5c;
                            arena<uint32_t>(0x50)[2] = r.range(0, 4);   // the text given: 0 its own, else new (1: empty, 2: one character)
                            arena<uint32_t>(0x50)[3] = r.range(1, 2);
                        },
                        [](void *fn) {
                            DnText *given;
                            const uint32_t kind = arena<uint32_t>(0x50)[2];
                            if (kind == 0) {
                                given = *(DnText **)(arena<uint8_t>(0x100) + arena<uint32_t>(0x50)[1]);
                                given->count++;
                            } else {
                                const uint16_t wide[4] = {'n', 'e', 'w', '!'};
                                given = dn_make((DnText *)dev_take(0x10), wide, kind == 1 ? 0 : kind == 2 ? 1 : 4);
                                given->count = (int32_t)arena<uint32_t>(0x50)[3];
                            }
                            *arena<DnText *>(0x60) = given;
                            ((void(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), nullptr, arena<void>(0x60));
                            return Result{*arena<uint32_t>(0x20)};
                        }};
    info_set_texts.iterations = 1000;
    info_set_texts.prepare = [](const rewrite::Entry *e) {
        const bool first = e->at == 0xa8ac30;
        stubs_for_now({{0xbcdfc0, (void *)int_alloc}, {0xbcf850, (void *)dn_text_c}, {0xcd23a0, (void *)ist_replace}, {0xbce010, (void *)cheat_free_aligned},
                       {0xbce000, (void *)cheat_free}, {first ? 0xa8a040u : 0xb87cd0u, (void *)ist_default}, {first ? 0xa89f30u : 0xb87b10u, (void *)ist_after}});
    };
    info_set_texts.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(info_set_texts);
#endif

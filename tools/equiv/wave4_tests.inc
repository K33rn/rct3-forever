    // Batch::Push: batches of any fill, often one short of full
    Test w4_batch{0x7b39c0,
                  [=](Rng &r) {
                      fresh(r);
                      static void *vt[1] = {(void *)w4_stub_flush};
                      uint32_t *b = arena<uint32_t>(0x800);
                      for (int i = 0; i < 0x42; i++) b[i] = r.next();
                      b[0] = (uint32_t)(uintptr_t)vt;
                      b[0x41] = r.range(0, 1) ? 63 : r.range(0, 62);
                      *arena<uint32_t>(0x10) = r.next();
                  },
                  [](void *fn) {
                      return Result{((uint32_t(__thiscall *)(void *, void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x800), arena<void>(0x10), 1, 2, 3)};
                  }};
    quick(w4_batch);
    // Linked::Destroy: self at 0x800, its node at the copy's offset between two neighbours
    static uint32_t w4_off;
    Test w4_linked{0x633860,
                   [=](Rng &r) {
                       fresh(r);
                       uint32_t *s = arena<uint32_t>(0x800);
                       for (int i = 0; i < 16; i++) s[i] = r.next();
                       void **node = arena<void *>(0x800 + w4_off), **prev = arena<void *>(0x900), **next = arena<void *>(0x940);
                       node[0] = next; node[1] = prev; prev[0] = node; next[1] = node;
                       prev[1] = (void *)(uintptr_t)r.next(); next[0] = (void *)(uintptr_t)r.next();
                   },
                   [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4_linked.prepare = [](const rewrite::Entry *e) {
        w4_off = *(uint8_t *)(e->at + 2);
        stubs_for_now({{0x6a1df0, (void *)stub_plain_dtor}});
    };
    w4_linked.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_linked);
    // Reference::Make: the handle at 0x300, its ring sometimes with a node already
    Test w4_refmake{0x4ff6f0,
                    [=](Rng &r) {
                        handles(r);
                        for (int i = 0; i < 32; i++) arena<uint32_t>(0x800)[i] = r.next();
                        *(void **)arena<uint8_t>(0x800 + w4_off) = arena<void>(0x300);
                        for (int i = 0; i < 8; i++) arena<uint32_t>(0x500)[i] = r.next();
                        if (r.range(0, 1)) {
                            void **head = arena<void *>(0x340), **n = arena<void *>(0x560);
                            n[0] = head[0]; n[1] = head; ((void **)head[0])[1] = n; head[0] = n;
                        }
                    },
                    [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0x500))}; }};
    w4_refmake.prepare = [](const rewrite::Entry *e) { w4_off = *(uint8_t *)(e->at + 0x10); };
    quick(w4_refmake);
    // Rows::Copy
    Test w4_rows{0x8e62b0,
                 [=](Rng &r) { fresh(r); for (int i = 0; i < 16; i++) arena<uint32_t>(0x800)[i] = r.next(); },
                 [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0x100))}; }};
    quick(w4_rows);
    // Array::GrowTo: counts below, at and above the count and the capacity
    Test w4_grow{0x6659a0,
                 [=](Rng &r) {
                     fresh(r);
                     int32_t *a = arena<int32_t>(0x800);
                     a[0] = r.next(); a[1] = r.range(0, 10); a[2] = a[1] + r.range(0, 5); a[3] = r.range(0, 3);
                     *arena<int32_t>(0x10) = r.range(-2, 20);
                 },
                 [](void *fn) { ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x800), *arena<int32_t>(0x10)); return Result{}; }};
    w4_grow.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x22 + 5 + *(int32_t *)(e->at + 0x23)), (void *)stub_reserve}}); };
    w4_grow.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_grow);
    // Vectors::Clear: the vectors come as the copy's last stack argument
    Test w4_vectors{0xccf0d0,
                    [=](Rng &r) {
                        fresh(r);
                        for (int i = 0; i < 40; i++) arena<uint32_t>(0x800)[i] = r.next();
                        *arena<int32_t>(0x10) = r.range(-1, 8);
                    },
                    [](void *fn) {
                        int32_t n = *arena<int32_t>(0x10);
                        void *v = arena<void>(0x800);
                        if (w4_off == 0x14) ((void(__fastcall *)(int32_t, void *, uint32_t, uint32_t, uint32_t, void *))fn)(n, nullptr, 1, 2, 3, v);
                        else if (w4_off == 0x18) ((void(__fastcall *)(int32_t, void *, uint32_t, uint32_t, uint32_t, uint32_t, void *))fn)(n, nullptr, 1, 2, 3, 4, v);
                        else ((void(__fastcall *)(int32_t, void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, void *))fn)(n, nullptr, 1, 2, 3, 4, 5, v);
                        return Result{};
                    }};
    w4_vectors.prepare = [](const rewrite::Entry *e) { w4_off = *(uint8_t *)(e->at + 0xb); };
    quick(w4_vectors);
    // Screen::IsCurrent: the globals are set in the call
    static uint32_t w4_global, w4_saved[2];
    Test w4_screen{0x6245e0,
                   [=](Rng &r) {
                       fresh(r);
                       uint32_t object = r.range(0, 2) ? 0x1000 + r.range(0, 8) * 4 : 0;
                       int kind = r.range(0, 3);
                       arena<uint32_t>(0x10)[0] = object;
                       arena<uint32_t>(0x10)[1] = kind == 0 ? 0 : kind == 1 ? object : object + 0xc;
                   },
                   [](void *fn) {
                       *(uint32_t *)w4_global = arena<uint32_t>(0x10)[0];
                       *(uint32_t *)0x12f503c = arena<uint32_t>(0x10)[1];
                       return Result{((uint8_t(__cdecl *)())fn)()};
                   }};
    w4_screen.prepare = [](const rewrite::Entry *e) {
        w4_global = *(uint32_t *)(e->at + 1);
        w4_saved[0] = *(uint32_t *)w4_global; w4_saved[1] = *(uint32_t *)0x12f503c;
    };
    w4_screen.finish = [](const rewrite::Entry *) { *(uint32_t *)w4_global = w4_saved[0]; *(uint32_t *)0x12f503c = w4_saved[1]; };
    quick(w4_screen);
    // Fields::SavedSize: a table entry at 0x900 + the argument, 0-3 fields at 0x600..
    Test w4_fields{0x5ba7a0,
                   [=](Rng &r) {
                       fresh(r);
                       static void *vt[3] = {nullptr, nullptr, (void *)w4_stub_field_size};
                       uint8_t *self = arena<uint8_t>(0x800);
                       *(void **)(self + 0x28) = arena<void>(0x900);
                       *(void **)(self + 0xc) = arena<void>(0x700);
                       *(int32_t *)(self + 0x18) = r.range(0, 3);
                       for (int i = 0; i < 3; i++) { arena<void *>(0x700)[i] = arena<void>(0x600 + i * 0x10); arena<void *>(0x600 + i * 0x10)[0] = vt; }
                       int32_t table = r.range(0, 3) * 8;
                       *arena<int32_t>(0x10) = table;
                       *(void **)arena<uint8_t>(0x900 + table) = arena<void>(0xa00 + r.range(0, 7) * 0x10);
                       *(int32_t *)arena<uint8_t>(0x904 + table) = r.range(-1, 4);
                   },
                   [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x800), *arena<int32_t>(0x10))}; }};
    quick(w4_fields);
    // Items::At: the interface always there (the game would crash without one), found now or kept already
    Test w4_items{0x532dc0,
                  [=](Rng &r) {
                      fresh(r);
                      iface_handle(r);
                      *(void **)arena<uint8_t>(0x340 + 0xc) = arena<void>(0x360);
                      *arena<void *>(0x1f0) = arena<void>(0x200);
                      *g_iface_id = (uint32_t)r.range(1, 2);
                      uint8_t *self = arena<uint8_t>(0x800);
                      *(void **)(self + 0x208) = arena<void>(0x340);
                      self[0x21c] = (uint8_t)r.range(0, 1);
                      *(void **)(self + 0x218) = arena<void>(0x3c0);
                      *arena<uint32_t>(0x3a0) = r.next(); *arena<uint32_t>(0x3a8) = r.next(); *arena<uint32_t>(0x400) = r.next();
                      *arena<int32_t>(0x10) = r.range(0, 50);
                  },
                  [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t))fn)(arena<void>(0x800), *arena<int32_t>(0x10))}; }};
    w4_items.prepare = [](const rewrite::Entry *) { g_iface_id = (uint32_t *)0x12f5338; };
    quick(w4_items);
    // Refs::Selected: three references at 0x900, some kept
    Test w4_selected{0xb44da0,
                     [=](Rng &r) {
                         fresh(r);
                         iface_handle(r);
                         uint8_t *self = arena<uint8_t>(0x800);
                         *(int32_t *)(self + 0x78) = r.range(-2, 2);
                         *(void **)(self + 0x7c) = arena<void>(0x900);
                         for (int i = 0; i < 3; i++) {
                             uint8_t *ref = arena<uint8_t>(0x900 + i * 0x1c);
                             *(void **)(ref + 4) = arena<void>(0x340);
                             *(uint32_t *)(ref + 0x14) = r.next();
                             ref[0x18] = (uint8_t)r.range(0, 1);
                         }
                     },
                     [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4_selected.prepare = [](const rewrite::Entry *) { g_iface_id = (uint32_t *)0x13d0058; };
    quick(w4_selected);
    // Children::Any: 0-4 children, the check sometimes lowering the count
    Test w4_any{0x9c6950,
                [=](Rng &r) {
                    fresh(r);
                    uint8_t *self = arena<uint8_t>(0x800);
                    *(int32_t *)(self + 0xd0) = r.range(0, 4);
                    *(void **)(self + 0xcc) = arena<void>(0x700);
                    for (int i = 0; i < 4; i++) {
                        uint8_t *child = arena<uint8_t>(0x600 + i * 0x10);
                        arena<void *>(0x700)[i] = child;
                        child[0] = r.range(0, 3) == 0;
                        child[1] = r.range(0, 3) == 0;
                    }
                },
                [](void *fn) { return Result{((uint8_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4_any.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x19 + 5 + *(int32_t *)(e->at + 0x1a)), (void *)w4_stub_child}}); };
    w4_any.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_any);
    // Mode::Step: the copy's own mode and others
    Test w4_mode{0x799750,
                 [=](Rng &r) {
                     fresh(r);
                     uint8_t *self = arena<uint8_t>(0x800);
                     *(int32_t *)(self + 0x190) = r.range(0, 1) ? (int32_t)w4_off : r.range(0, 30);
                     *(uint32_t *)(self + 0x1a4) = r.next();
                     *arena<uint8_t>(0x20) = (uint8_t)r.next();
                 },
                 [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800)) & 0xff}; }};
    w4_mode.prepare = [](const rewrite::Entry *e) {
        w4_off = *(uint8_t *)(e->at + 9);
        stubs_for_now({{0x7cd220, (void *)stub_thiscall2}, {(uintptr_t)(e->at + 0x28 + 5 + *(int32_t *)(e->at + 0x29)), (void *)w4_stub_step}});
    };
    w4_mode.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_mode);
    // Part::Pass: each slot its own stand-in
    Test w4_part{0xaae180,
                 [=](Rng &r) {
                     fresh(r);
                     static void *vt[5] = {(void *)w4_stub_slot<0>, (void *)w4_stub_slot<1>, (void *)w4_stub_slot<2>, (void *)w4_stub_slot<3>, (void *)w4_stub_slot<4>};
                     uint8_t *self = arena<uint8_t>(0x800);
                     *(void **)(self + 0x40) = vt;
                     self[0x1c0] = (uint8_t)r.range(0, 1);
                     arena<uint32_t>(0x10)[0] = r.next(); arena<uint32_t>(0x10)[1] = r.next();
                 },
                 [](void *fn) { return Result{((uint8_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1])}; }};
    quick(w4_part);
    // Chain::Free: no head, a head alone, a head with more
    Test w4_chainfree{0x42c1e0,
                      [=](Rng &r) {
                          fresh(r);
                          arena<void *>(0x100)[0] = r.range(0, 3) ? arena<void>(0x4000) : nullptr;
                          *(void **)arena<uint8_t>(0x4000 + w4_off) = r.range(0, 1) ? arena<void>(0x300) : nullptr;
                      },
                      [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w4_chainfree.prepare = [](const rewrite::Entry *e) {
        w4_off = *(uint32_t *)(e->at + 9);
        stubs_for_now({{(uintptr_t)(e->at + 0x13 + 5 + *(int32_t *)(e->at + 0x14)), (void *)stub_deleting}});
    };
    w4_chainfree.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_chainfree);
    // Lookup::Refresh
    Test w4_lookup{0xbfbe30,
                   [=](Rng &r) {
                       fresh(r);
                       uint8_t *self = arena<uint8_t>(0x800);
                       for (int i = 0; i < 16; i++) ((uint32_t *)self)[i] = r.next();
                       *(void **)(self + 0x14) = arena<void>(0x900);
                       *(uint32_t *)(self + 0x28) = r.range(0, 1);
                       self[0x2c] = (uint8_t)r.range(0, 1);
                       *(void **)arena<uint8_t>(0x900 + 0x470) = arena<void>(0x300);
                   },
                   [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4_lookup.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xc5c670, (void *)w4_stub_lookup}}); };
    w4_lookup.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_lookup);
    // Registered::ForEach: 0-3 nodes and the end node; the list's global is set in the call
    Test w4_registered{0x49ed40,
                       [=](Rng &r) {
                           fresh(r);
                           int n = r.range(0, 3);
                           for (int i = 0; i < n; i++) {
                               void **node = arena<void *>(0x400 + i * 0x20);
                               node[0] = arena<void>(0x400 + (i + 1) * 0x20);
                               node[2] = (void *)(uintptr_t)r.next();
                           }
                           arena<void *>(0x400 + n * 0x20)[0] = nullptr;
                       },
                       [](void *fn) { *(void **)w4_global = arena<void>(0x400); ((void(__cdecl *)())fn)(); return Result{}; }};
    w4_registered.prepare = [](const rewrite::Entry *e) {
        w4_global = *(uint32_t *)(e->at + 3);
        w4_saved[0] = *(uint32_t *)w4_global;
        stubs_for_now({{(uintptr_t)(e->at + 0x13 + 5 + *(int32_t *)(e->at + 0x14)), (void *)stub_plain_dtor}});
    };
    w4_registered.finish = [](const rewrite::Entry *) { *(uint32_t *)w4_global = w4_saved[0]; unstub_many(); };
    quick(w4_registered);
    // Hold::Release: a hold at 0x800, its device at 0x300
    auto w4_hold = [=](Rng &r) {
        fresh(r);
        static void *vt[13];
        vt[12] = (void *)w4_stub_pay;
        arena<void *>(0x300)[0] = vt;
        uint8_t *h = arena<uint8_t>(0x800);
        for (int i = 0; i < 4; i++) ((uint32_t *)h)[i] = r.next();
        *(void **)(h + 4) = arena<void>(0x300);
        *(uint16_t *)(h + 0xc) = (uint16_t)r.range(0, 3);
        *(uint16_t *)(h + 0xe) = (uint16_t)r.range(0, 3);
    };
    Test w4_release{0xbe9420, w4_hold, [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    quick(w4_release);
    // Hold::End: on and off, through a holder at 0x900 to the hold at 0x800
    Test w4_end{0xbfd8f0,
                [=](Rng &r) {
                    w4_hold(r);
                    uint8_t *self = arena<uint8_t>(0xa00);
                    for (int i = 0; i < 12; i++) ((uint32_t *)self)[i] = r.next();
                    *(void **)(self + 0x20) = arena<void>(0x900);
                    *(uint32_t *)(self + 0x24) = r.range(0, 2);
                    arena<void *>(0x900)[1] = arena<void>(0x800);
                },
                [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0xa00)); return Result{}; }};
    quick(w4_end);
    // Flags::Refresh: flags at 0x858 for the interface at 0x900
    Test w4_flags{0x4ebe70,
                  [=](Rng &r) {
                      fresh(r);
                      static void *vt[2] = {nullptr, (void *)w4_stub_on};
                      *(uint32_t *)arena<uint8_t>(0x858) = r.next();
                      arena<void *>(0x300)[0] = vt;
                      arena<uint32_t>(0x300)[1] = r.next();
                      *arena<uint8_t>(0x308) = (uint8_t)r.range(0, 1);
                      *(void **)arena<uint8_t>(0x908) = arena<void>(0x300);
                  },
                  [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x900), 7)}; }};
    quick(w4_flags);
    // Pair::Touch: handles at 0x300 and 0x380; objects there or not, touched or not
    Test w4_touch{0x548ca0,
                  [=](Rng &r) {
                      fresh(r);
                      uint8_t *self = arena<uint8_t>(0x800);
                      *(void **)(self + 0x4c) = arena<void>(0x300);
                      *(void **)(self + 0x60) = arena<void>(0x380);
                      *(void **)arena<uint8_t>(0x30c) = r.range(0, 3) ? arena<void>(0x500) : nullptr;
                      *(void **)arena<uint8_t>(0x38c) = r.range(0, 3) ? arena<void>(0x540) : nullptr;
                      *(uint32_t *)arena<uint8_t>(0x52c) = r.next();
                      *(uint32_t *)arena<uint8_t>(0x56c) = r.next();
                      *arena<uint8_t>(0x84) = (uint8_t)r.range(0, 1);
                  },
                  [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4_touch.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x800860, (void *)w4_stub_touch}}); };
    w4_touch.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_touch);
    // Query::Ask: answered or not, by the value's lowest bit
    Test w4_query{0x4a9210,
                  [=](Rng &r) {
                      fresh(r);
                      static void *vt[33];
                      vt[32] = (void *)w4_stub_query;
                      arena<void *>(0x800)[0] = vt;
                      *arena<uint32_t>(0x10) = r.next();
                  },
                  [](void *fn) { return Result{((uint8_t(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0x10))}; }};
    quick(w4_query);
    // Kept::Value: the reference at the copy's offset, kept or looked up, sometimes nothing
    Test w4_kept{0x4abc40,
                 [=](Rng &r) {
                     fresh(r);
                     iface_handle(r);
                     uint8_t *ref = arena<uint8_t>(0x800 + w4_off);
                     *(void **)(ref + 4) = arena<void>(0x340);
                     ref[0x18] = (uint8_t)r.range(0, 1);
                     *(void **)(ref + 0x14) = r.range(0, 1) ? arena<void>(0x380) : nullptr;
                     *arena<uint8_t>(0x20) = (uint8_t)r.next();
                 },
                 [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4_kept.prepare = [](const rewrite::Entry *e) {
        w4_off = *(uint32_t *)(e->at + 0x10) - 4;
        g_iface_id = (uint32_t *)0x130bbec;
        stubs_for_now({{0x846ae0, (void *)w4_stub_kept}});
    };
    w4_kept.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_kept);
    // Refs::AnyHas: references at 0x900, the last index -1 to 3; the second id is the copy's own global
    Test w4_anyhas{0xb453a0,
                   [=](Rng &r) {
                       fresh(r);
                       iface_handle(r);
                       *(uint32_t *)w4_global = (uint32_t)r.range(0, 3);
                       uint8_t *self = arena<uint8_t>(0x800);
                       *(int32_t *)(self + 0x78) = r.range(-1, 3);
                       *(void **)(self + 0x7c) = arena<void>(0x900);
                       for (int i = 0; i < 4; i++) {
                           uint8_t *ref = arena<uint8_t>(0x900 + i * 0x1c);
                           *(void **)(ref + 4) = arena<void>(0x340);
                           ref[0x18] = (uint8_t)r.range(0, 1);
                           *(void **)(ref + 0x14) = r.range(0, 1) ? arena<void>(0x360) : nullptr;
                       }
                   },
                   [](void *fn) { return Result{((uint8_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4_anyhas.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x13d0058;
        w4_global = *(uint32_t *)(e->at + 0x7b);
        w4_saved[0] = *(uint32_t *)w4_global;
    };
    w4_anyhas.finish = [](const rewrite::Entry *) { *(uint32_t *)w4_global = w4_saved[0]; };
    quick(w4_anyhas);
    // Reference::Copy: the reference at 0x100 takes the one at 0x180, with its fields
    Test w4_refcopy{0x82e860, handles,
                    [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x180))}; }};
    quick(w4_refcopy);
    // Reference::AssignAt: the same, the reference at the copy's offset
    Test w4_refassign{0x530f40, handles,
                      [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<uint8_t>(0x100) - w4_off, arena<void>(0x180)); return Result{}; }};
    w4_refassign.prepare = [](const rewrite::Entry *e) { w4_off = *(uint32_t *)(e->at + 4); };
    quick(w4_refassign);
    // ItemArray::Resize: bigger, smaller, the same, to nothing, from nothing, and memory that can't be had
    Test w4_resize{0x5d42a0,
                   [=](Rng &r) {
                       fresh(r);
                       int count = r.range(0, 8);
                       arena<void *>(0x100)[0] = count ? arena<void>(0x800) : nullptr;
                       arena<int32_t>(0x100)[1] = count;
                       *arena<int32_t>(0x10) = r.range(0, 8);
                       *arena<uint32_t>(0x28) = r.next();
                   },
                   [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), *arena<int32_t>(0x10))}; }};
    w4_resize.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbcdfd0, (void *)w4_stub_alloc_maybe}, {0x5d3d00, (void *)w4_stub_construct},
                       {(uintptr_t)(e->at + 0x78 + 5 + *(int32_t *)(e->at + 0x79)), (void *)stub_thiscall1}});
    };
    w4_resize.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_resize);
    // ItemArray::Reset
    Test w4_reset{0x5d7370,
                  [=](Rng &r) {
                      fresh(r);
                      arena<void *>(0x100)[0] = r.range(0, 2) ? arena<void>(0x800) : nullptr;
                      arena<int32_t>(0x100)[1] = r.range(0, 8);
                      *arena<int32_t>(0x10) = r.range(0, 8);
                      *arena<uint32_t>(0x28) = r.next();
                  },
                  [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), *arena<int32_t>(0x10))}; }};
    w4_reset.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)w4_stub_alloc_maybe}, {0x5d3d00, (void *)w4_stub_construct}}); };
    w4_reset.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_reset);
    // a list of 0-4 nodes at 0x420.. between a start node (0x3e0) and an end node (0x4e0, next null); gives the first node
    auto w4_list = [](Rng &r, int n) {
        void **start = arena<void *>(0x3e0), **prev = start;
        start[1] = (void *)(uintptr_t)r.next();
        for (int i = 0; i < n; i++) {
            void **node = arena<void *>(0x420 + i * 0x20);
            prev[0] = node; node[1] = prev; prev = node;
        }
        void **end = arena<void *>(0x4e0);
        prev[0] = end; end[0] = nullptr; end[1] = prev;
        return (void *)start[0];
    };
    // List::DeleteAll: some nodes empty
    Test w4_deleteall{0x6af540,
                      [=](Rng &r) {
                          fresh(r);
                          static void *vt[1] = {(void *)w4_stub_delete_value};
                          int n = r.range(0, 4);
                          *(void **)arena<uint8_t>(0x800 + w4_off) = w4_list(r, n);
                          for (int i = 0; i < n; i++) {
                              void **value = arena<void *>(0x500 + i * 0x20), **node = arena<void *>(0x420 + i * 0x20);
                              value[0] = vt; value[1] = node;
                              node[2] = r.range(0, 2) ? value : nullptr;
                          }
                      },
                      [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4_deleteall.prepare = [](const rewrite::Entry *e) { w4_off = *(uint8_t *)(e->at + 5); };
    quick(w4_deleteall);
    // Members::Notify: nodes empty, members without an owner, members with one
    Test w4_members{0x6a20e0,
                    [=](Rng &r) {
                        fresh(r);
                        static void *vt[3] = {nullptr, nullptr, (void *)w4_stub_owner};
                        arena<void *>(0x600)[0] = vt;
                        int n = r.range(0, 4);
                        *(void **)arena<uint8_t>(0x800 + 0xd4) = w4_list(r, n);
                        for (int i = 0; i < n; i++) {
                            uint8_t *member = arena<uint8_t>(0x1000 + i * 0x200);
                            *(void **)(member + 0x13c) = r.range(0, 2) ? arena<void>(0x600) : nullptr;
                            arena<void *>(0x420 + i * 0x20)[2] = r.range(0, 2) ? member : nullptr;
                        }
                    },
                    [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), 5); return Result{}; }};
    w4_members.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x69a980, (void *)stub_plain_dtor}}); };
    w4_members.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_members);
    // List::Visit
    Test w4_visit{0x49bae0,
                  [=](Rng &r) {
                      fresh(r);
                      static void *vt[1] = {(void *)w4_stub_visit};
                      arena<void *>(0x600)[0] = vt;
                      int n = r.range(0, 4);
                      arena<void *>(0x100)[0] = w4_list(r, n);
                      for (int i = 0; i < n; i++) {
                          uint8_t *value = arena<uint8_t>(0x1000 + i * 0x40);
                          *(uint32_t *)(value + 0x18) = r.next();
                          arena<void *>(0x420 + i * 0x20)[2] = r.range(0, 2) ? value : nullptr;
                      }
                  },
                  [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x600)); return Result{}; }};
    quick(w4_visit);
    // Target::Set: each state, the same target and a new one, the copy's own field
    Test w4_target{0x4f65f0,
                   [=](Rng &r) {
                       fresh(r);
                       uint8_t *self = arena<uint8_t>(0x800);
                       *(int32_t *)(self + 0x144) = r.range(0, 5);
                       *(uint32_t *)(self + 0x5dc) = r.range(0, 2) == 0;
                       int32_t target = r.range(0, 9);
                       *(int32_t *)(self + w4_off) = r.range(0, 2) ? target : target + 1;
                       *arena<int32_t>(0x10) = target;
                   },
                   [](void *fn) { ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x800), *arena<int32_t>(0x10)); return Result{}; }};
    w4_target.prepare = [](const rewrite::Entry *e) {
        w4_off = *(uint32_t *)(e->at + 0x28);
        stubs_for_now({{0x4f5950, (void *)w4_stub_four}, {0x4f1740, (void *)stub_thiscall1}, {0x4f5060, (void *)stub_plain_dtor}, {0x4f41c0, (void *)stub_thiscall1}});
    };
    w4_target.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_target);
    // SlotPool::Take: room left, full, empty; slots are small indices into the copy's own item size
    Test w4_slot{0x735b20,
                 [=](Rng &r) {
                     fresh(r);
                     int32_t *self = arena<int32_t>(0x100);
                     self[0] = (int32_t)(uintptr_t)arena<void>(0x5000);
                     self[1] = (int32_t)(uintptr_t)arena<void>(0x3000);
                     self[3] = r.range(0, 6);
                     self[2] = r.range(0, 1) ? self[3] : r.range(0, self[3]);
                     for (int i = 0; i < 0x200; i++) arena<int32_t>(0x3000)[i] = r.range(0, 7);
                 },
                 [](void *fn) { return Result{(uint32_t)((int32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w4_slot.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x1b + 5 + *(int32_t *)(e->at + 0x1c)), (void *)w4_stub_grow}}); };
    w4_slot.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_slot);
    // Limit::Check: levels around 1; the object's limit at both fields
    Test w4_limit{0x536b50,
                  [=](Rng &r) {
                      fresh(r);
                      *(void **)arena<uint8_t>(0x800 + 0x294) = arena<void>(0x300);
                      arena<uint32_t>(0x300)[10] = r.next();
                      arena<uint32_t>(0x300)[11] = r.next();
                      arena<uint8_t>(0x510)[1] = (uint8_t)r.range(0, 4);
                  },
                  [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4_limit.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbb3e70, (void *)w4_stub_record}, {0x532ea0, (void *)w4_stub_first},
                       {(uintptr_t)(e->at + 0x34 + 5 + *(int32_t *)(e->at + 0x35)), (void *)w4_stub_second},
                       {(uintptr_t)(e->at + 0x3d + 5 + *(int32_t *)(e->at + 0x3e)), (void *)w4_stub_check}});
    };
    w4_limit.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_limit);
    // Events::Add: with room and without; only the argument's low byte counts
    Test w4_events{0x475cb0,
                   [=](Rng &r) {
                       fresh(r);
                       uint8_t *self = arena<uint8_t>(0x800);
                       *(void **)(self + 0x7c) = arena<void>(0x900);
                       int count = r.range(0, 6);
                       *(int32_t *)(self + 0x84) = count;
                       *(int32_t *)(self + 0x80) = count + r.range(-1, 2);
                       *arena<uint32_t>(0x10) = (uint32_t)r.range(0, 3) << 7;
                   },
                   [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x10)); return Result{}; }};
    w4_events.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x475a30, (void *)w4_stub_events_room}}); };
    w4_events.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_events);
    // Tree::Update: a root at 0x800 with up to 2 children (0x1400, 0x1700), each with its own parts and no children
    Test w4_tree{0x86ed80,
                 [=](Rng &r) {
                     fresh(r);
                     auto chain = [&](uint32_t at, int n, uint32_t holders) {
                         for (int i = 0; i < n; i++) {
                             void **node = arena<void *>(at + i * 0x10);
                             node[0] = arena<void>(at + (i + 1) * 0x10);
                             node[2] = holders ? arena<void>(holders + i * 0x10) : (void *)(uintptr_t)r.next();
                         }
                         arena<void *>(at + n * 0x10)[0] = nullptr;
                         return arena<void>(at);
                     };
                     uint32_t objects[2] = {0x1400, 0x1700};
                     for (int i = 0; i < 2; i++) {
                         *(void **)arena<uint8_t>(objects[i] + 0x130) = chain(0x1a00 + i * 0x80, r.range(0, 3), 0);
                         *(void **)arena<uint8_t>(objects[i] + 0x148) = chain(0x1b00 + i * 0x80, 0, 0);
                         arena<void *>(0x1c00 + i * 0x10)[0] = arena<void>(objects[i]);
                     }
                     *(void **)arena<uint8_t>(0x800 + 0x130) = chain(0x1000, r.range(0, 3), 0);
                     *(void **)arena<uint8_t>(0x800 + 0x148) = chain(0x1100, r.range(0, 2), 0x1c00);
                 },
                 [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4_tree.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x16 + 5 + *(int32_t *)(e->at + 0x17)), (void *)stub_plain_dtor}}); };
    w4_tree.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_tree);

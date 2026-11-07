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
    auto w4_noise = [=](Rng &r) {
        fresh(r);
        for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x800)[i] = r.next();
    };
    auto w4_self = [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x800))}; };
    Test w4_slotreset{0xcdf3c0, w4_noise, w4_self};
    quick(w4_slotreset);
    Test w4_treeinit{0xada8b0, w4_noise, w4_self};
    quick(w4_treeinit);
    // Id::Release: the system's global is set in the call
    Test w4_idrelease{0x910380,
                      [=](Rng &r) {
                          w4_noise(r);
                          static void *vt[0xc8 / 4 + 1];
                          vt[0xc8 / 4] = (void *)stub_thiscall1;
                          arena<void *>(0x600)[0] = vt;
                      },
                      [](void *fn) { *(void **)0x13d0db8 = arena<void>(0x600); ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4_idrelease.prepare = [](const rewrite::Entry *) { w4_saved[0] = *(uint32_t *)0x13d0db8; };
    w4_idrelease.finish = [](const rewrite::Entry *) { *(uint32_t *)0x13d0db8 = w4_saved[0]; };
    quick(w4_idrelease);
    // Registration::Destroy
    Test w4_registration{0x8272e0,
                         [=](Rng &r) { w4_noise(r); if (!r.range(0, 2)) arena<uint32_t>(0x800)[2] = 0; },
                         [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4_registration.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x45ee50, (void *)stub_thiscall1}}); };
    w4_registration.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_registration);
    // Owned::Replace
    Test w4_owned{0x7975c0,
                  [=](Rng &r) {
                      fresh(r);
                      static void *vt[1] = {(void *)stub_deleting};
                      arena<void *>(0x300)[0] = vt;
                      *(void **)arena<uint8_t>(0x800 + w4_off) = r.range(0, 2) ? arena<void>(0x300) : nullptr;
                      *arena<uint32_t>(0x10) = r.next();
                  },
                  [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x10)); return Result{}; }};
    w4_owned.prepare = [](const rewrite::Entry *e) { w4_off = *(uint32_t *)(e->at + 5); };
    quick(w4_owned);
    // a plain list of n nodes at 0x420.. and its end node; values from the arena at 0x500..
    auto w4_values = [=](Rng &r, int n) {
        void *first = w4_list(r, n);
        for (int i = 0; i < n; i++) {
            arena<void *>(0x420 + i * 0x20)[2] = arena<void>(0x500 + i * 0x20);
            arena<void *>(0x500 + i * 0x20)[1] = arena<void>(0x420 + i * 0x20);
        }
        return first;
    };
    // Members::Each
    Test w4_each{0x737210,
                 [=](Rng &r) { fresh(r); *(void **)arena<uint8_t>(0x800 + 0x44) = w4_values(r, r.range(0, 4)); *arena<uint32_t>(0x10) = r.next(); },
                 [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x10)); return Result{}; }};
    w4_each.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x16 + 5 + *(int32_t *)(e->at + 0x17)), (void *)w4_stub_ask}}); };
    w4_each.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_each);
    // Lookup::Through
    Test w4_through{0x7e5280,
                    [=](Rng &r) {
                        w4_noise(r);
                        iface_handle(r);
                        arena<void *>(0x800)[1] = arena<void>(0x340);
                    },
                    [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4_through.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x12f51b4;
        stubs_for_now({{(uintptr_t)(e->at + 0x3f + 5 + *(int32_t *)(e->at + 0x40)), (void *)w4_stub_ask}});
    };
    w4_through.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_through);
    // Pair::Release: with ids and without; the system's global is set in the call
    Test w4_pairrelease{0x50c090,
                        [=](Rng &r) { w4_noise(r); if (!r.range(0, 2)) arena<uint32_t>(0x800)[0] = 0xffffffff; },
                        [](void *fn) { *(void **)0x133a380 = arena<void>(0x600); ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4_pairrelease.prepare = [](const rewrite::Entry *) { w4_saved[0] = *(uint32_t *)0x133a380; stubs_for_now({{0x94ef60, (void *)stub_thiscall1}}); };
    w4_pairrelease.finish = [](const rewrite::Entry *) { *(uint32_t *)0x133a380 = w4_saved[0]; unstub_many(); };
    quick(w4_pairrelease);
    // Table::Find: no table, a table, a table that the mapping replaces
    Test w4_find{0xcafa00,
                 [=](Rng &r) {
                     fresh(r);
                     *(void **)arena<uint8_t>(0x834) = r.range(0, 3) ? arena<void>(0x300) : nullptr;
                     *arena<uint8_t>(0x84) = (uint8_t)r.range(0, 1);
                     *arena<uint32_t>(0x10) = r.next();
                 },
                 [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x10))}; }};
    w4_find.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xcada50, (void *)w4_stub_map}, {(uintptr_t)(e->at + 0x18 + 5 + *(int32_t *)(e->at + 0x19)), (void *)w4_stub_ask}});
    };
    w4_find.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_find);
    // Level::ValueOrNone
    Test w4_levelnone{0x5a11d0,
                      [=](Rng &r) {
                          fresh(r);
                          arena<uint8_t>(0x510)[1] = (uint8_t)r.range(0, 5);
                          *(uint32_t *)arena<uint8_t>(0x800 + w4_off) = r.next();
                      },
                      [](void *fn) { return Result{(uint32_t)((int32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4_levelnone.prepare = [](const rewrite::Entry *e) {
        w4_off = *(uint32_t *)(e->at + 0x20);
        stubs_for_now({{0xbb4280, (void *)w4_stub_a}, {0xbb3e70, (void *)w4_stub_record}});
    };
    w4_levelnone.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_levelnone);
    // Buffers::Clear
    Test w4_buffers{0x94d6d0, w4_noise, [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    quick(w4_buffers);
    // Level::Allowed
    Test w4_allowed{0x52ff60,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint8_t>(0x510)[1] = (uint8_t)r.range(0, 5);
                        *(uint32_t *)arena<uint8_t>(0x800 + w4_off) = r.range(0, 1) ? r.next() << 8 : 0;
                    },
                    [](void *fn) { return Result{((uint8_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4_allowed.prepare = [](const rewrite::Entry *e) {
        w4_off = *(uint8_t *)(e->at + 0x19);
        stubs_for_now({{0xbb3e70, (void *)w4_stub_record}});
    };
    w4_allowed.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_allowed);
    // List::EachVirtual: a value can cut its node off once called, which the walk has already read past
    Test w4_eachvirtual{0x9817b0,
                        [=](Rng &r) {
                            fresh(r);
                            static void *vt[2] = {nullptr, (void *)w4_stub_value_v1};
                            int n = r.range(0, 4);
                            *(void **)arena<uint8_t>(0x800 + w4_off) = w4_values(r, n);
                            for (int i = 0; i < n; i++) arena<void *>(0x500 + i * 0x20)[0] = vt;
                            *arena<uint8_t>(0x84) = (uint8_t)r.range(0, 1);
                        },
                        [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4_eachvirtual.prepare = [](const rewrite::Entry *e) { w4_off = *(uint8_t *)(e->at + 2); };
    quick(w4_eachvirtual);
    // Lookup::Forget
    Test w4_forget{0xbfc400,
                   [=](Rng &r) {
                       w4_noise(r);
                       static void *vt[3] = {nullptr, nullptr, (void *)w4_stub_release2};
                       arena<void *>(0x600)[0] = vt;
                       *(void **)arena<uint8_t>(0x814) = arena<void>(0x900);
                       *(void **)arena<uint8_t>(0x900 + 0x470) = arena<void>(0x300);
                       *(void **)arena<uint8_t>(0x800 + w4_off) = r.range(0, 2) ? arena<void>(0x600) : nullptr;
                   },
                   [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4_forget.prepare = [](const rewrite::Entry *e) {
        w4_off = *(uint8_t *)(e->at + 0x17);
        stubs_for_now({{0xc5cb30, (void *)stub_thiscall1}});
    };
    w4_forget.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_forget);
    // Part::Cycle: flag 2 set or clear, before and after the first virtual
    Test w4_cycle{0xc501a0,
                  [=](Rng &r) {
                      w4_noise(r);
                      static void *vt[3] = {nullptr, (void *)w4_stub_step, (void *)w4_stub_part_first};
                      arena<void *>(0x300)[0] = vt;
                      *(void **)arena<uint8_t>(0x828) = arena<void>(0x300);
                      *arena<uint32_t>(0x84) = r.next();
                      *arena<uint8_t>(0x20) = (uint8_t)r.next();
                  },
                  [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    quick(w4_cycle);
    // Registry::Join
    Test w4_join{0x786d20, [=](Rng &r) { w4_noise(r); *arena<uint8_t>(0x20) = (uint8_t)r.next(); },
                 [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4_join.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xcdee80, (void *)w4_stub_ask}, {0x833890, (void *)w4_stub_step}}); };
    w4_join.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_join);
    // Status::Busy
    Test w4_busy{0x559bc0,
                 [=](Rng &r) {
                     fresh(r);
                     *arena<uint32_t>(0x20) = r.range(0, 1) ? 0x100 : 0;
                     *arena<uint8_t>(0x890) = (uint8_t)(r.range(0, 1) * 0x40);
                 },
                 [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4_busy.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 3 + 5 + *(int32_t *)(e->at + 4)), (void *)w4_stub_check_flag}}); };
    w4_busy.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_busy);
    // Counter::Next: around the copy's limit; the counter is set in the call, and what it became is part of the result
    Test w4_counter{0x59a170,
                    [=](Rng &r) { fresh(r); *arena<int32_t>(0x10) = (int32_t)w4_off + r.range(-3, 1); },
                    [](void *fn) {
                        *(int32_t *)w4_global = *arena<int32_t>(0x10);
                        uint32_t value = (uint32_t)((int32_t(__cdecl *)())fn)();
                        return Result{value * 0x10000 + *(uint32_t *)w4_global};
                    }};
    w4_counter.prepare = [](const rewrite::Entry *e) {
        w4_global = *(uint32_t *)(e->at + 1);
        w4_off = *(uint8_t *)(e->at + 8);
        w4_saved[0] = *(uint32_t *)w4_global;
    };
    w4_counter.finish = [](const rewrite::Entry *) { *(uint32_t *)w4_global = w4_saved[0]; };
    quick(w4_counter);
    // Step::Then
    Test w4_then{0x67ac00,
                 [=](Rng &r) {
                     fresh(r);
                     *(void **)arena<uint8_t>(0x800 + w4_off) = r.range(0, 2) ? arena<void>(0x300) : nullptr;
                 },
                 [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4_then.prepare = [](const rewrite::Entry *e) {
        w4_off = *(uint32_t *)(e->at + 0xa);
        stubs_for_now({{(uintptr_t)(e->at + 3 + 5 + *(int32_t *)(e->at + 4)), (void *)stub_plain_dtor},
                       {(uintptr_t)(e->at + 0x13 + 5 + *(int32_t *)(e->at + 0x14)), (void *)w4_stub_step}});
    };
    w4_then.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_then);
    // Default::Copy: the global is set in the call
    Test w4_default{0x4bf080,
                    [=](Rng &r) {
                        w4_noise(r);
                        arena<uint32_t>(0x600)[0x12] = r.next();
                        arena<uint32_t>(0x700)[0x12] = r.next();
                        *arena<void *>(0x10) = r.range(0, 1) ? arena<void>(0x600) : nullptr;
                    },
                    [](void *fn) { *(void **)0x1106c54 = *arena<void *>(0x10); ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4_default.prepare = [](const rewrite::Entry *) { w4_saved[0] = *(uint32_t *)0x1106c54; stubs_for_now({{0x444090, (void *)w4_stub_default}}); };
    w4_default.finish = [](const rewrite::Entry *) { *(uint32_t *)0x1106c54 = w4_saved[0]; unstub_many(); };
    quick(w4_default);
    // Rows::Ask: three rows at 0x900, each with its own object
    Test w4_rowsask{0x40bda0,
                    [=](Rng &r) {
                        fresh(r);
                        arena<void *>(0x800)[0] = arena<void>(0x900);
                        for (int i = 0; i < 3; i++) *(uint32_t *)arena<uint8_t>(0x900 + i * 0x70 + 0x54) = r.next();
                        arena<int32_t>(0x10)[0] = r.range(0, 2);
                        arena<uint32_t>(0x10)[1] = r.next();
                    },
                    [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, int32_t, uint32_t))fn)(arena<void>(0x800), arena<int32_t>(0x10)[0], arena<uint32_t>(0x10)[1])}; }};
    w4_rowsask.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x12 + 5 + *(int32_t *)(e->at + 0x13)), (void *)w4_stub_ask}}); };
    w4_rowsask.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_rowsask);
    // Entries::Ask
    Test w4_entriesask{0x8915d0,
                       [=](Rng &r) {
                           fresh(r);
                           arena<int32_t>(0x10)[0] = r.range(0, 5);
                           arena<uint32_t>(0x10)[1] = r.next();
                       },
                       [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, int32_t, uint32_t))fn)(arena<void>(0x800), arena<int32_t>(0x10)[0], arena<uint32_t>(0x10)[1])}; }};
    w4_entriesask.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x10 + 5 + *(int32_t *)(e->at + 0x11)), (void *)w4_stub_ask}}); };
    w4_entriesask.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_entriesask);
    // Triple::Set
    Test w4_triple{0x6a4b10,
                   [=](Rng &r) { w4_noise(r); for (int i = 0; i < 3; i++) arena<uint32_t>(0x10)[i] = r.next(); *arena<uint8_t>(0x20) = (uint8_t)r.next(); },
                   [](void *fn) {
                       uint32_t *v = arena<uint32_t>(0x10);
                       return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x800), v[0], v[1], v[2])};
                   }};
    w4_triple.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x1e + 5 + *(int32_t *)(e->at + 0x1f)), (void *)w4_stub_step}}); };
    w4_triple.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_triple);
    // Counted::Assign: objects at 0x300 and 0x340, the same one sometimes, the old one sometimes at its last count
    Test w4_counted{0xc425e0,
                    [=](Rng &r) {
                        fresh(r);
                        static void *vt[1] = {(void *)stub_deleting};
                        for (uint32_t at : {0x300u, 0x340u}) { arena<void *>(at)[0] = vt; arena<int32_t>(at)[1] = r.range(1, 3); }
                        arena<void *>(0x100)[0] = arena<void>(0x300);
                        arena<void *>(0x110)[0] = r.range(0, 3) ? arena<void>(0x340) : arena<void>(0x300);
                    },
                    [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x110))}; }};
    quick(w4_counted);
    // Versioned::Field: versions around 1, a negative one
    Test w4_versioned{0x87f330,
                      [=](Rng &r) {
                          fresh(r);
                          static const uint32_t versions[] = {0xffff, 0x10000, 0x80000000, 0x7fff1234, 0};
                          *arena<uint8_t>(0x20) = (uint8_t)(r.range(0, 3) ? 0x10 : 0);
                          *(void **)arena<uint8_t>(0x818) = arena<void>(0x300);
                          arena<uint32_t>(0x300)[2] = versions[r.range(0, 4)];
                          *(uint32_t *)arena<uint8_t>(0x300 + 0xdc) = r.next();
                          *(uint32_t *)arena<uint8_t>(0x300 + 0xe0) = r.next();
                      },
                      [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4_versioned.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbd1420, (void *)w4_stub_flagged}}); };
    w4_versioned.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_versioned);
    // Map::Value
    Test w4_map{0xbdf160,
                [=](Rng &r) { fresh(r); *arena<uint32_t>(0x600) = r.next(); *arena<uint32_t>(0x10) = r.next(); },
                [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x10))}; }};
    w4_map.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 8 + 5 + *(int32_t *)(e->at + 9)), (void *)w4_stub_find}}); };
    w4_map.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4_map);
    // Queue::DeleteAll: 0-4 objects from a random front; the queue's globals are set in the call
    static uint32_t w4_queue_saved[4];
    Test w4_queue{0x79bfd0,
                  [=](Rng &r) {
                      fresh(r);
                      static void *vt[1] = {(void *)w4_stub_dequeue};
                      for (int i = 0; i < 8; i++) { arena<void *>(0x700)[i] = arena<void>(0x600 + i * 0x10); arena<void *>(0x600 + i * 0x10)[0] = vt; }
                      arena<int32_t>(0x10)[0] = r.range(0, 4);
                      arena<int32_t>(0x10)[1] = r.range(0, 3);
                      arena<int32_t>(0x10)[2] = r.range(0, 9);
                  },
                  [](void *fn) {
                      int32_t *q = arena<int32_t>(0x10);
                      *(void **)0x1306bc8 = arena<void>(0x700);
                      *(int32_t *)0x1306bd8 = q[2];
                      *(int32_t *)0x1306bcc = q[2] + q[0];
                      *(int32_t *)0x1306be0 = q[1];
                      ((void(__cdecl *)())fn)();
                      return Result{(uint32_t)(*(int32_t *)0x1306be0 * 0x100 + *(int32_t *)0x1306bcc)};
                  }};
    w4_queue.prepare = [](const rewrite::Entry *) {
        w4_queue_saved[0] = *(uint32_t *)0x1306bc8; w4_queue_saved[1] = *(uint32_t *)0x1306bd8;
        w4_queue_saved[2] = *(uint32_t *)0x1306bcc; w4_queue_saved[3] = *(uint32_t *)0x1306be0;
    };
    w4_queue.finish = [](const rewrite::Entry *) {
        *(uint32_t *)0x1306bc8 = w4_queue_saved[0]; *(uint32_t *)0x1306bd8 = w4_queue_saved[1];
        *(uint32_t *)0x1306bcc = w4_queue_saved[2]; *(uint32_t *)0x1306be0 = w4_queue_saved[3];
    };
    quick(w4_queue);
    // Global::Free: the copy's own global, empty or holding a block
    static uint32_t w4b_global, w4b_global_saved;
    Test w4b_gfree{0x9de660,
                   [=](Rng &r) { fresh(r); *arena<uint32_t>(0x10) = r.range(0, 2) ? r.next() | 1 : 0; },
                   [](void *fn) {
                       *(uint32_t *)(uintptr_t)w4b_global = *arena<uint32_t>(0x10);
                       ((void(__cdecl *)())fn)();
                       return Result{*(uint32_t *)(uintptr_t)w4b_global};
                   }};
    w4b_gfree.prepare = [](const rewrite::Entry *e) { w4b_global = *(uint32_t *)(e->at + 1); w4b_global_saved = *(uint32_t *)(uintptr_t)w4b_global; };
    w4b_gfree.finish = [](const rewrite::Entry *) { *(uint32_t *)(uintptr_t)w4b_global = w4b_global_saved; };
    quick(w4b_gfree);
    // Out::Fill
    Test w4b_fill{0x9da5e0,
                  [=](Rng &r) { fresh(r); for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x800)[i] = r.next(); },
                  [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0x300))}; }};
    w4b_fill.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x13 + 5 + *(int32_t *)(e->at + 0x14)), (void *)w4b_stub_fill}}); };
    w4b_fill.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4b_fill);
    // Block::Past: no block, a block saying it has no header (each copy's own word, and the other's), or any other
    Test w4b_past{0x4a1880,
                  [=](Rng &r) {
                      fresh(r);
                      static const int32_t words[] = {-1, 2, 0, 1, 0x1234};
                      *arena<void *>(0x800 + 0x138) = r.range(0, 3) ? arena<void>(0x300) : nullptr;
                      *arena<void *>(0x800 + 0x170) = r.range(0, 3) ? arena<void>(0x300) : nullptr;
                      *arena<int32_t>(0x300) = words[r.range(0, 4)];
                  },
                  [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    quick(w4b_past);
    // Bar::Add: taken or refused; the bytes around the bar's are random
    Test w4b_bar{0x724490,
                 [=](Rng &r) {
                     fresh(r);
                     for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x800)[i] = r.next();
                     *arena<uint32_t>(0x20) = r.range(0, 1);
                     *arena<uint32_t>(0x10) = r.next();
                 },
                 [](void *fn) { return Result{((uint8_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x10))}; }};
    w4b_bar.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{(uintptr_t)(e->at + 0xc + 5 + *(int32_t *)(e->at + 0xd)), (void *)w4b_stub_take}, {0x723e50, (void *)stub_plain_dtor}});
    };
    w4b_bar.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4b_bar);
    // Owner::Delete: the object's table is overwritten, its member destroyed, and freed for odd flags
    Test w4b_owner{0x73ee00,
                   [=](Rng &r) { fresh(r); *arena<uint32_t>(0x800) = r.next(); *arena<uint32_t>(0x10) = r.range(0, 3); },
                   [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x10))}; }};
    w4b_owner.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0xc + 5 + *(int32_t *)(e->at + 0xd)), (void *)stub_plain_dtor}}); };
    w4b_owner.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4b_owner);
    // Pair::Delete
    Test w4b_pair{0xbdd150,
                  [=](Rng &r) { fresh(r); for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x800)[i] = r.next(); *arena<uint32_t>(0x10) = r.range(0, 3); },
                  [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x10))}; }};
    quick(w4b_pair);
    // Vector::FreeBoth: each of the two may be missing; the count sits in the copy's own header
    Test w4b_vec{0x735ae0,
                 [=](Rng &r) {
                     fresh(r);
                     for (int i = 0; i < 8; i++) arena<uint32_t>(0x300)[i] = r.next();
                     arena<void *>(0x800)[0] = r.range(0, 3) ? arena<void>(0x310) : nullptr;
                     arena<void *>(0x800)[1] = r.range(0, 3) ? (void *)(uintptr_t)(r.next() | 1) : nullptr;
                 },
                 [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    quick(w4b_vec);
    // Table::Free: the array at the copy's field, or none
    static uint32_t w4b_table_off;
    Test w4b_table{0xb0e090,
                   [=](Rng &r) {
                       fresh(r);
                       for (int i = 0; i < 8; i++) { arena<uint32_t>(0x300)[i] = r.next(); arena<uint32_t>(0x800)[i] = r.next(); }
                       *arena<void *>(0x800 + w4b_table_off) = r.range(0, 3) ? arena<void>(0x310) : nullptr;
                   },
                   [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4b_table.prepare = [](const rewrite::Entry *e) { w4b_table_off = *(uint8_t *)(e->at + 0xb); };
    quick(w4b_table);
    // Shared::AssignAt: the same object or another; the old count reaching zero or not
    static uint32_t w4b_owner_off, w4b_slot_off;
    auto w4b_shared_setup = [=](Rng &r, uint32_t **slot) {
        for (int i = 0; i < 8; i++) { arena<uint32_t>(0x300)[i] = r.next(); arena<uint32_t>(0x340)[i] = r.next(); }
        *arena<int32_t>(0x300) = r.range(1, 3);
        *arena<int32_t>(0x340) = r.range(1, 3);
        *slot = arena<uint32_t>(0x300);
        *arena<void *>(0x10) = r.range(0, 3) ? arena<void>(0x340) : arena<void>(0x300);
    };
    Test w4b_assign{0x615d70,
                    [=](Rng &r) {
                        fresh(r);
                        *arena<void *>(0x800 + w4b_owner_off) = arena<void>(0x400);
                        w4b_shared_setup(r, arena<uint32_t *>(0x400 + w4b_slot_off));
                    },
                    [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0x10))}; }};
    w4b_assign.prepare = [](const rewrite::Entry *e) { w4b_owner_off = *(uint32_t *)(e->at + 8); w4b_slot_off = *(uint32_t *)(e->at + 0xe); };
    quick(w4b_assign);
    // Shared::Refresh: the object found is the slot's or another
    Test w4b_refresh{0x4d6d80,
                     [=](Rng &r) {
                         fresh(r);
                         for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x800)[i] = r.next();
                         w4b_shared_setup(r, arena<uint32_t *>(0x800 + w4b_slot_off));
                     },
                     [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), 7)}; }};
    w4b_refresh.prepare = [](const rewrite::Entry *e) {
        w4b_slot_off = *(uint8_t *)(e->at + 0x10);
        stubs_for_now({{(uintptr_t)(e->at + 7 + 5 + *(int32_t *)(e->at + 8)), (void *)w4b_stub_get}});
    };
    w4b_refresh.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4b_refresh);
    // Owned::FreeAll: none to several, some slots empty, a negative count too
    Test w4b_owned{0x811820,
                   [=](Rng &r) {
                       fresh(r);
                       arena<void *>(0x800)[0] = arena<void>(0x300);
                       arena<int32_t>(0x800)[1] = r.range(0, 5) ? r.range(0, 6) : -1;
                       for (int i = 0; i < 6; i++) arena<void *>(0x300)[i] = r.range(0, 2) ? arena<void>(0x400 + i * 0x10) : nullptr;
                   },
                   [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4b_owned.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x1b + 5 + *(int32_t *)(e->at + 0x1c)), (void *)stub_plain_dtor}}); };
    w4b_owned.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4b_owned);
    // Owned::DeleteAll: 0-4 objects, some slots empty; no array when the count is 0
    static uint32_t w4b_items_off, w4b_count_off;
    Test w4b_delall{0xa40680,
                    [=](Rng &r) {
                        fresh(r);
                        static void *vt[1] = {(void *)stub_deleting};
                        int32_t n = r.range(0, 4);
                        *arena<int32_t>(0x800 + w4b_count_off) = n;
                        *arena<void *>(0x800 + w4b_items_off) = n || r.range(0, 1) ? arena<void>(0x300) : nullptr;
                        for (int i = 0; i < 4; i++) {
                            arena<void *>(0x300)[i] = r.range(0, 3) ? arena<void>(0x400 + i * 0x10) : nullptr;
                            *arena<void *>(0x400 + i * 0x10) = vt;
                        }
                    },
                    [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w4b_delall.prepare = [](const rewrite::Entry *e) { w4b_count_off = *(uint32_t *)(e->at + 5); w4b_items_off = *(uint32_t *)(e->at + 0x12); };
    quick(w4b_delall);
    // Matrix::Of
    Test w4b_matrix{0xb88270,
                    [=](Rng &r) {
                        fresh(r);
                        static void *vt[3] = {nullptr, nullptr, (void *)w4b_stub_matrix};
                        *arena<void *>(0x308) = vt;
                        *arena<uint32_t>(0x20) = r.range(0, 1);
                        for (int i = 0; i < 16; i++) arena<uint32_t>(0x600)[i] = r.next();
                        for (int i = 0; i < 0x70; i++) arena<uint32_t>(0x800)[i] = r.next();
                    },
                    [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0x700))}; }};
    w4b_matrix.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xb87f70, (void *)w4b_stub_owner}}); };
    w4b_matrix.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4b_matrix);
    // Listeners::Notify: an empty list or 1-4 listeners, kept or looked up, some matching one key or both
    Test w4b_notify{0x54f880,
                    [=](Rng &r) {
                        fresh(r);
                        iface_handle(r);
                        for (uint32_t o : {0x3a4u, 0x3acu, 0x3d4u, 0x3dcu, 0x404u, 0x434u}) *arena<uint32_t>(o) = r.range(0, 1);
                        int n = r.range(0, 4);
                        for (int i = 0; i <= n; i++) {
                            void **node = arena<void *>(0x900 + i * 0x10);
                            node[0] = i < n ? arena<void>(0x900 + (i + 1) * 0x10) : nullptr;
                            node[2] = arena<void>(0x680 + i * 0x1c);
                            uint8_t *ref = arena<uint8_t>(0x680 + i * 0x1c);
                            *(void **)(ref + 4) = arena<void>(0x340);
                            ref[0x18] = (uint8_t)r.range(0, 1);
                            *(void **)(ref + 0x14) = r.range(0, 3) ? arena<void>(0x3c0) : nullptr;
                        }
                        *arena<void *>(0x800 + 0x78) = arena<void>(0x900);
                        arena<uint32_t>(0x10)[0] = r.range(0, 1);
                        arena<uint32_t>(0x10)[1] = r.range(0, 1);
                    },
                    [](void *fn) {
                        uint32_t *a = arena<uint32_t>(0x10);
                        ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), a[0], a[1]);
                        return Result{};
                    }};
    w4b_notify.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x12f52bc;
        stubs_for_now({{(uintptr_t)(e->at + 0x6f + 5 + *(int32_t *)(e->at + 0x70)), (void *)stub_plain_dtor}});
    };
    w4b_notify.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4b_notify);
    // Items::Any: none to four items; the check holds on some call or never
    Test w4b_any{0x5d6ac0,
                 [=](Rng &r) {
                     fresh(r);
                     iface_handle(r);
                     uint8_t *self = arena<uint8_t>(0x800);
                     *(void **)(self + 0x54) = arena<void>(0x340);
                     *(void **)(self + 0x64) = r.range(0, 3) ? arena<void>(0x3c0) : nullptr;
                     self[0x68] = (uint8_t)r.range(0, 1);
                     *(int32_t *)(self + 0xe8) = r.range(0, 4);
                     *(void **)(self + 0xe4) = arena<void>(0x4a0);
                     for (int i = 0; i < 4; i++) arena<uint32_t>(0x4a0)[i] = r.next();
                     *arena<int32_t>(0x20) = r.range(0, 5);
                     arena<uint32_t>(0x10)[0] = r.next();
                     arena<uint32_t>(0x10)[1] = r.next();
                 },
                 [](void *fn) {
                     uint32_t *a = arena<uint32_t>(0x10);
                     return Result{((uint8_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), a[0], a[1])};
                 }};
    w4b_any.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x12f54e0;
        stubs_for_now({{0x59acc0, (void *)w4b_stub_of_iface}, {0x5a7300, (void *)w4b_stub_item},
                       {(uintptr_t)(e->at + 0x78 + 5 + *(int32_t *)(e->at + 0x79)), (void *)w4b_stub_check}});
    };
    w4b_any.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4b_any);
    // Chain::Clear: links at 0x1000 each going on, back to the start or nowhere, kept or looked up; a looked-up link at 0x360/0x368 ends there
    Test w4b_chain{0x5d55f0,
                   [=](Rng &r) {
                       fresh(r);
                       iface_handle(r);
                       for (uint32_t o : {0x360u, 0x368u}) {
                           *arena<void *>(o + 0x16c) = nullptr;
                           *arena<uint8_t>(o + 0x170) = 1;
                       }
                       uint8_t *chain[4];
                       for (int i = 0; i < 4; i++) chain[i] = arena<uint8_t>(0x1000 + i * 0x180);
                       for (int i = 0; i < 4; i++) {
                           uint8_t *link = chain[i];
                           link[0x10] = (uint8_t)r.next();
                           *(void **)(link + 0x15c) = arena<void>(0x340);
                           int to = r.range(0, 5);
                           *(void **)(link + 0x16c) = to == 0 ? nullptr : to == 1 || i == 3 ? chain[0] : chain[i + 1];
                           link[0x170] = r.range(0, 4) != 0;
                       }
                       uint8_t *self = arena<uint8_t>(0x800);
                       *(void **)(self + 8) = r.range(0, 7) ? chain[0] : nullptr;
                       *(void **)(self + 0x14) = r.range(0, 1) ? chain[r.range(0, 3)] : nullptr;
                       *arena<uint32_t>(0x10) = r.next();
                   },
                   [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x10)); return Result{}; }};
    w4b_chain.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x12f5608;
        stubs_for_now({{0x5c00c0, (void *)w4b_stub_held}, {(uintptr_t)(e->at + 0x2a + 5 + *(int32_t *)(e->at + 0x2b)), (void *)stub_thiscall1}});
    };
    w4b_chain.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4b_chain);
    // Members: the global list in the arena, set in the call; none yet, or 0-1 other members before this one
    static uint32_t w4b_list, w4b_list_saved;
    auto w4b_members = [=](Rng &r, bool self_in) {
        void **list = arena<void *>(0x300), **other = arena<void *>(0x340), **self = arena<void *>(0x804);
        void **end = list + 3, **last = list;
        memset(list, 0, 0x18);
        if (r.range(0, 1)) { other[1] = last; other[2] = (void *)(uintptr_t)r.next(); last[0] = other; last = other; }
        if (self_in) { self[1] = last; last[0] = self; last = self; }
        last[0] = end;
        end[1] = last;
    };
    auto w4b_list_call = [](void *fn) {
        *(uint32_t *)(uintptr_t)w4b_list = *arena<uint32_t>(0x10);
        ((void(__fastcall *)(void *))fn)(arena<void>(0x800));
        return Result{*(uint32_t *)(uintptr_t)w4b_list};
    };
    Test w4b_join{0x49cc80,
                  [=](Rng &r) {
                      fresh(r);
                      for (int i = 0; i < 4; i++) arena<uint32_t>(0x800)[i] = r.next();
                      w4b_members(r, false);
                      *arena<void *>(0x10) = r.range(0, 1) ? arena<void>(0x300) : nullptr;
                  },
                  w4b_list_call};
    w4b_join.prepare = [](const rewrite::Entry *e) {
        w4b_list = *(uint32_t *)(e->at + 0xe); w4b_list_saved = *(uint32_t *)(uintptr_t)w4b_list;
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc_next}});
    };
    w4b_join.finish = [](const rewrite::Entry *) { *(uint32_t *)(uintptr_t)w4b_list = w4b_list_saved; unstub_many(); };
    quick(w4b_join);
    Test w4b_leave{0x49ccf0,
                   [=](Rng &r) {
                       fresh(r);
                       *arena<uint32_t>(0x800) = r.next();
                       w4b_members(r, true);
                       *arena<void *>(0x10) = arena<void>(0x300);
                   },
                   w4b_list_call};
    w4b_leave.prepare = [](const rewrite::Entry *e) { w4b_list = *(uint32_t *)(e->at + 0x3e); w4b_list_saved = *(uint32_t *)(uintptr_t)w4b_list; };
    w4b_leave.finish = [](const rewrite::Entry *) { *(uint32_t *)(uintptr_t)w4b_list = w4b_list_saved; };
    quick(w4b_leave);
    // Grid::Neighbours: cells at every edge of small grids
    Test w4b_grid{0x7238a0,
                  [=](Rng &r) {
                      fresh(r);
                      uint8_t *self = arena<uint8_t>(0x800), *cell = arena<uint8_t>(0x400);
                      for (int i = 0; i < 0x28; i++) arena<uint32_t>(0x400)[i] = r.next();
                      for (int i = 0; i < 48; i++) arena<uint32_t>(0x300 - 0x40)[i] = r.next();
                      self[0x54] = (uint8_t)r.range(1, 5);
                      self[0x5c] = (uint8_t)r.range(0, 3);
                      self[0x5d] = (uint8_t)r.range(0, 3);
                      *(void **)(self + 0x58) = arena<void>(0x300);
                      cell[0x88] = (uint8_t)r.range(0, 4);
                      cell[0x89] = (uint8_t)r.range(0, 4);
                      *arena<uint32_t>(0x20) = r.range(0, 1);
                  },
                  [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0x400)); return Result{}; }};
    w4b_grid.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x46 + 5 + *(int32_t *)(e->at + 0x47)), (void *)w4b_stub_join}}); };
    w4b_grid.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4b_grid);
    // Kept::Then: kept, or looked up (found or not), in the copy's own reference
    static uint32_t w4b_ref_off;
    Test w4b_then{0x4c5890,
                  [=](Rng &r) {
                      fresh(r);
                      iface_handle(r);
                      uint8_t *ref = arena<uint8_t>(0x800 + w4b_ref_off);
                      *(void **)(ref + 4) = arena<void>(0x340);
                      ref[0x18] = (uint8_t)r.range(0, 1);
                      *(void **)(ref + 0x14) = r.range(0, 1) ? arena<void>(0x380) : nullptr;
                  },
                  [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4b_then.prepare = [](const rewrite::Entry *e) {
        w4b_ref_off = *(uint8_t *)(e->at + 0xc) - 4;
        g_iface_id = *(uint32_t **)(e->at + 0x22);
        stubs_for_now({{(uintptr_t)(e->at + 0x45 + 5 + *(int32_t *)(e->at + 0x46)), (void *)w4b_stub_then}});
    };
    w4b_then.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4b_then);
    // Once::Make: made already or not, with the flag's other bits random
    struct W4bWords { uint32_t words[14]; };
    static uint32_t w4b_made, w4b_made_saved;
    Test w4b_once{0x696000,
                  [=](Rng &r) {
                      fresh(r);
                      for (int i = 0; i < 4; i++) arena<uint32_t>(0x800)[i] = r.next();
                      arena<uint32_t>(0x10)[0] = r.next();
                      arena<uint32_t>(0x10)[1] = r.next();
                      arena<uint32_t>(0x10)[2] = r.next();
                  },
                  [](void *fn) {
                      uint32_t *a = arena<uint32_t>(0x10);
                      *(uint32_t *)(uintptr_t)w4b_made = a[2];
                      W4bWords unused{};
                      void *back = ((void *(__thiscall *)(void *, void *, void *, W4bWords))fn)(arena<void>(0x800), a, a + 1, unused);
                      return Result{(uint32_t)(uintptr_t)back ^ *(uint32_t *)(uintptr_t)w4b_made};
                  }};
    w4b_once.prepare = [](const rewrite::Entry *e) {
        w4b_made = *(uint32_t *)(e->at + 0x1a); w4b_made_saved = *(uint32_t *)(uintptr_t)w4b_made;
        stubs_for_now({{0x416110, (void *)w4b_stub_table}});
    };
    w4b_once.finish = [](const rewrite::Entry *) { *(uint32_t *)(uintptr_t)w4b_made = w4b_made_saved; unstub_many(); };
    quick(w4b_once);
    // Matrix::Refresh: no object behind the handle, a source not live, or live with the bits of 0x408 either way
    Test w4b_mrefresh{0x7a0fd0,
                      [=](Rng &r) {
                          fresh(r);
                          iface_handle(r);
                          static void *vt[6] = {(void *)stub_deleting, (void *)stub_class_of, nullptr, nullptr, nullptr, (void *)w4b_stub_live};
                          static void *view_vt[8] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, (void *)w4b_stub_view_matrix};
                          arena<void *>(0x360)[0] = vt;
                          arena<void *>(0x368)[0] = vt;
                          *arena<void *>(0x700) = view_vt;
                          *arena<void *>(0x1f0) = arena<void>(0x200);
                          *g_iface_id = (uint32_t)r.range(1, 2);
                          uint8_t *self = arena<uint8_t>(0x800);
                          for (int i = 0; i < 0x30; i++) arena<uint32_t>(0x800)[i] = r.next();
                          for (int i = 0; i < 16; i++) arena<uint32_t>(0x600)[i] = r.next();
                          *(void **)(self + 0x510) = arena<void>(0x340);
                          self[0x408] = (uint8_t)r.range(0, 3);
                          *arena<uint32_t>(0x20) = r.range(0, 2) ? r.next() | 1 : 0;
                      },
                      [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), 5)}; }};
    w4b_mrefresh.prepare = [](const rewrite::Entry *) {
        g_iface_id = (uint32_t *)0x12f4f44;
        stubs_for_now({{0x7a0000, (void *)w4b_stub_path_a}, {0x7a0480, (void *)w4b_stub_path_b}, {0x79e260, (void *)w4b_stub_view}});
    };
    w4b_mrefresh.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w4b_mrefresh);
    // Scanner::Switch: no buffer yet, the same buffer, or another; the scanner's globals are the copy's own, set in the call
    static uint32_t w4b_scan[7], w4b_scan_saved[7];   // current, hold, position, count, text, file, fresh
    Test w4b_scanner{0xc0ac90,
                     [=](Rng &r) {
                         fresh(r);
                         for (int i = 0; i < 16; i++) { arena<uint32_t>(0x300)[i] = r.next(); arena<uint32_t>(0x340)[i] = r.next(); arena<uint32_t>(0x380)[i] = r.next(); }
                         arena<void *>(0x340)[2] = arena<void>(0x3a0);
                         uint32_t choice = r.range(0, 3);
                         arena<void *>(0x10)[0] = choice == 0 ? nullptr : choice == 1 ? arena<void>(0x340) : arena<void>(0x300);
                         for (int i = 1; i < 6; i++) arena<uint32_t>(0x10)[i] = r.next();
                     },
                     [](void *fn) {
                         uint32_t *a = arena<uint32_t>(0x10);
                         *(uint32_t *)(uintptr_t)w4b_scan[0] = a[0];
                         *(uint8_t *)(uintptr_t)w4b_scan[1] = (uint8_t)a[1];
                         *(void **)(uintptr_t)w4b_scan[2] = arena<void>(0x384);
                         *(uint32_t *)(uintptr_t)w4b_scan[3] = a[2];
                         *(uint32_t *)(uintptr_t)w4b_scan[4] = a[3];
                         *(uint32_t *)(uintptr_t)w4b_scan[5] = a[4];
                         *(uint32_t *)(uintptr_t)w4b_scan[6] = a[5];
                         ((void(__fastcall *)(void *))fn)(arena<void>(0x340));
                         uint32_t mix = *(uint8_t *)(uintptr_t)w4b_scan[1];
                         for (int i : {0, 2, 3, 4, 5, 6}) mix = mix * 31 + *(uint32_t *)(uintptr_t)w4b_scan[i];
                         return Result{mix};
                     }};
    w4b_scanner.prepare = [](const rewrite::Entry *e) {
        static const uint8_t at[7] = {0x1, 0xf, 0x16, 0x1e, 0x35, 0x43, 0x56};
        for (int i = 0; i < 7; i++) { w4b_scan[i] = *(uint32_t *)(e->at + at[i]); w4b_scan_saved[i] = *(uint32_t *)(uintptr_t)w4b_scan[i]; }
    };
    w4b_scanner.finish = [](const rewrite::Entry *) { for (int i = 0; i < 7; i++) *(uint32_t *)(uintptr_t)w4b_scan[i] = w4b_scan_saved[i]; };
    quick(w4b_scanner);
    // ---- third batch ----
    auto w4c_fill = [=](Rng &r) {
        fresh(r);
        for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x800)[i] = r.next();
        *arena<uint32_t>(0x20) = r.next();
    };
    auto w4c_owned = [=](Rng &r, uint32_t off) {
        static void *vt[1] = {(void *)stub_thiscall1};
        arena<void *>(0xd00)[0] = vt;
        *arena<void *>(0x800 + off) = r.range(0, 1) ? arena<void>(0xd00) : nullptr;
    };
    auto w4c_self = [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; };
    static uint32_t w4c_off, w4c_k, w4c_global, w4c_global_saved, w4c_global2_saved;
    auto w4c_unstub = [](const rewrite::Entry *) { unstub_many(); };
    // Part::Destroy
    Test w4c_part{0x6a5a80, w4c_fill, w4c_self};
    w4c_part.prepare = [](const rewrite::Entry *e) { stubs_for_now({{w4c_target(e, 0x2b), (void *)stub_plain_dtor}}); };
    w4c_part.finish = w4c_unstub;
    quick(w4c_part);
    // Handle::Named: with and without memory
    Test w4c_named{0x453340, w4c_fill, [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4c_named.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfc0, (void *)w4c_stub_alloc}, {0xbcf850, (void *)w4c_stub_ctor1}}); };
    w4c_named.finish = w4c_unstub;
    quick(w4c_named);
    // Five-table destructor: owning something or not
    Test w4c_five{0x6cc840, [=](Rng &r) { w4c_fill(r); w4c_owned(r, 0x154); }, w4c_self};
    w4c_five.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x63a030, (void *)stub_plain_dtor}}); };
    w4c_five.finish = w4c_unstub;
    quick(w4c_five);
    // Three-table destructor: the copy's own owned offset and member destructor
    Test w4c_three{0x623bc0, [=](Rng &r) { w4c_fill(r); w4c_owned(r, w4c_off); }, w4c_self};
    w4c_three.prepare = [](const rewrite::Entry *e) {
        w4c_off = *(uint32_t *)(e->at + 0x3b);
        stubs_for_now({{w4c_target(e, 0x59), (void *)stub_plain_dtor}, {0x6b0ea0, (void *)stub_plain_dtor}});
    };
    w4c_three.finish = w4c_unstub;
    quick(w4c_three);
    // Four-table destructor
    Test w4c_four{0xa80350, w4c_fill, w4c_self};
    w4c_four.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x40f730, (void *)stub_plain_dtor}, {0x67b9f0, (void *)stub_plain_dtor}}); };
    w4c_four.finish = w4c_unstub;
    quick(w4c_four);
    // Holder destructor: owning something or not, at the copy's own offset
    Test w4c_holder{0x99b1f0, [=](Rng &r) { w4c_fill(r); w4c_owned(r, w4c_off + 4); }, w4c_self};
    w4c_holder.prepare = [](const rewrite::Entry *e) {
        w4c_off = *(uint32_t *)(e->at + 0x36);
        stubs_for_now({{w4c_target(e, 0x6b), (void *)stub_plain_dtor}});
    };
    w4c_holder.finish = w4c_unstub;
    quick(w4c_holder);
    // Mode::Handle: the mode or not; a name or none; memory or none. The name is the copy's own global
    Test w4c_mode{0x756b00,
                  [=](Rng &r) {
                      w4c_fill(r);
                      *(uint32_t *)arena<uint8_t>(0x800 + 0x190) = r.range(0, 3) ? w4c_k : w4c_k + 1;
                      *arena<uint32_t>(0x10) = r.range(0, 2) ? r.next() | 1 : 0;
                  },
                  [](void *fn) {
                      *(uint32_t *)(uintptr_t)w4c_global = *arena<uint32_t>(0x10);
                      return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0xa00))};
                  }};
    w4c_mode.prepare = [](const rewrite::Entry *e) {
        w4c_k = *(uint8_t *)(e->at + 0x23);
        w4c_global = *(uint32_t *)(e->at + 0x46); w4c_global_saved = *(uint32_t *)(uintptr_t)w4c_global;
        stubs_for_now({{w4c_target(e, 0x2d), (void *)stub_thiscall1}, {0xbcdfc0, (void *)w4c_stub_alloc}, {0xbcf7d0, (void *)w4c_stub_ctor1}, {0xbd0490, (void *)w4c_stub_default}});
    };
    w4c_mode.finish = [](const rewrite::Entry *) { *(uint32_t *)(uintptr_t)w4c_global = w4c_global_saved; unstub_many(); };
    quick(w4c_mode);
    // Registered::Make: an owner given or the shared one, ready or not
    Test w4c_reg{0x968000,
                 [=](Rng &r) {
                     w4c_fill(r);
                     arena<uint8_t>(0x904)[0] = (uint8_t)r.range(0, 1);
                     arena<uint8_t>(0xa44)[0] = (uint8_t)r.range(0, 1);
                     *arena<uint32_t>(0x10) = r.range(0, 1);
                 },
                 [](void *fn) {
                     *(void **)(uintptr_t)w4c_global = arena<void>(0x900);
                     void *back = ((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x800), *arena<uint32_t>(0x10) ? arena<void>(0xa40) : nullptr);
                     return Result{(uint32_t)(uintptr_t)back};
                 }};
    w4c_reg.prepare = [](const rewrite::Entry *e) {
        w4c_global = *(uint32_t *)(e->at + 0x58); w4c_global_saved = *(uint32_t *)(uintptr_t)w4c_global;
        stubs_for_now({{0x967c10, (void *)stub_thiscall1}, {0xbeca40, (void *)w4c_stub_key}, {0xbecc10, (void *)stub_thiscall1}, {0xbec5e0, (void *)w4c_stub_key_gone}});
    };
    w4c_reg.finish = [](const rewrite::Entry *) { *(uint32_t *)(uintptr_t)w4c_global = w4c_global_saved; unstub_many(); };
    quick(w4c_reg);
    // Refs::Total: references kept already or found now, live or not
    Test w4c_total{0xa898f0,
                   [=](Rng &r) {
                       w4c_fill(r);
                       iface_handle(r);
                       static void *vt[0x40] = {(void *)stub_deleting, (void *)stub_class_of};
                       vt[0xe4 / 4] = (void *)w4c_stub_amount; vt[0xe8 / 4] = (void *)w4c_stub_amount2;
                       arena<void *>(0x360)[0] = vt;
                       arena<void *>(0x368)[0] = vt;
                       *g_iface_id = (uint32_t)r.range(0, 3);
                       uint8_t *self = arena<uint8_t>(0x800);
                       *(int32_t *)(self + 0xd0) = r.range(0, 4);
                       *(void **)(self + 0xcc) = arena<void>(0xa00);
                       for (int i = 0; i < 4; i++) {
                           uint8_t *ref = arena<uint8_t>(0xa00 + i * 0x1c);
                           *(void **)(ref + 4) = arena<void>(0x340);
                           *(void **)(ref + 0x14) = r.range(0, 1) ? arena<void>(0x368) : nullptr;
                           ref[0x18] = (uint8_t)r.range(0, 1);
                       }
                   },
                   [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4c_total.prepare = [](const rewrite::Entry *) { g_iface_id = (uint32_t *)0x13cde74; };
    quick(w4c_total);
    // Once::Make3: made or not, in the copy's own global
    Test w4c_once{0x71f340,
                  [=](Rng &r) {
                      w4c_fill(r);
                      for (int i = 0; i < 4; i++) arena<uint32_t>(0x10)[i] = r.next();
                  },
                  [](void *fn) {
                      uint32_t *a = arena<uint32_t>(0x10);
                      *(uint32_t *)(uintptr_t)w4c_global = a[3];
                      W4cWords unused{};
                      void *back = ((void *(__thiscall *)(void *, void *, void *, void *, W4cWords))fn)(arena<void>(0x800), a, a + 1, a + 2, unused);
                      return Result{(uint32_t)(uintptr_t)back ^ *(uint32_t *)(uintptr_t)w4c_global};
                  }};
    w4c_once.prepare = [](const rewrite::Entry *e) {
        w4c_global = *(uint32_t *)(e->at + 0x23); w4c_global_saved = *(uint32_t *)(uintptr_t)w4c_global;
        stubs_for_now({{0x416110, (void *)w4b_stub_table}});
    };
    w4c_once.finish = [](const rewrite::Entry *) { *(uint32_t *)(uintptr_t)w4c_global = w4c_global_saved; unstub_many(); };
    quick(w4c_once);
    // Place::At: a position, the two globals, and the copy's own next step
    Test w4c_place{0xbf7130,
                   [=](Rng &r) {
                       w4c_fill(r);
                       static void *vt[0x30];
                       vt[0xac / 4] = (void *)stub_thiscall2; vt[0xb8 / 4] = (void *)w4c_stub_place;
                       arena<void *>(0x800)[0] = vt;
                       for (int i = 0; i < 8; i++) arena<uint32_t>(0x10)[i] = r.next();
                   },
                   [](void *fn) {
                       uint32_t *a = arena<uint32_t>(0x10);
                       *(uint32_t *)0x13d28a0 = a[3]; *(uint32_t *)0x13d0e34 = a[4];
                       return Result{((uint32_t(__fastcall *)(void *, void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x800), a, a[5], a[6], a[7])};
                   }};
    w4c_place.prepare = [](const rewrite::Entry *e) {
        w4c_global_saved = *(uint32_t *)0x13d28a0; w4c_global2_saved = *(uint32_t *)0x13d0e34;
        stubs_for_now({{w4c_target(e, 0xc2), (void *)w4c_stub_then}});
    };
    w4c_place.finish = [](const rewrite::Entry *) { *(uint32_t *)0x13d28a0 = w4c_global_saved; *(uint32_t *)0x13d0e34 = w4c_global2_saved; unstub_many(); };
    quick(w4c_place);
    // Big::Run
    Test w4c_big{0xc161e0, w4c_fill, [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x20))}; }};
    w4c_big.prepare = [](const rewrite::Entry *e) { stubs_for_now({{w4c_target(e, 0x82), (void *)w4c_stub_job}}); };
    w4c_big.finish = w4c_unstub;
    quick(w4c_big);
    // ---- fourth batch ----
    static uint32_t w4d_saved;
    // Pack::Send: the copy's own word packed and sent with +0xdc to the renderer in the global
    Test w4d_pack{0x949090, w4c_fill, [](void *fn) {
                      static void *vt[0x40];
                      vt[0xb0 / 4] = (void *)w4d_stub_render;
                      arena<void *>(0x2100)[0] = vt;
                      *(void **)0x13d0db8 = arena<void>(0x2100);
                      return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))};
                  }};
    w4d_pack.prepare = [](const rewrite::Entry *) { w4d_saved = *(uint32_t *)0x13d0db8; stubs_for_now({{0x431070, (void *)w4d_stub_pack}}); };
    w4d_pack.finish = [](const rewrite::Entry *) { *(uint32_t *)0x13d0db8 = w4d_saved; unstub_many(); };
    quick(w4d_pack);
    // Parts::FindKind: 0-4 parts whose interfaces (offset 0 or 8) carry kinds 8, 0xc or 5; every lookup succeeds (a failed one reads 0x190)
    Test w4d_find{0x7743d0,
                  [=](Rng &r) {
                      w4c_fill(r);
                      static void *vt[2] = {nullptr, (void *)w4d_stub_class};
                      uint8_t *self = arena<uint8_t>(0x800);
                      *(int32_t *)(self + 0x48) = r.range(0, 4);
                      *(void **)(self + 0x60) = arena<void>(0x2200);
                      uint8_t *cls = arena<uint8_t>(0x2000);
                      *(void **)(cls + 0x14) = arena<void>(0x2040);
                      *(int32_t *)(cls + 0x18) = 3;
                      arena<void *>(0x2040)[1] = arena<void>(0x2080);
                      *arena<int32_t>(0x2090) = r.range(0, 1) * 8;
                      static const int32_t kinds[3] = {8, 0xc, 5};
                      for (int k = 0; k < 4; k++) {
                          *(void **)arena<uint8_t>(0x2200 + k * 0x14 + 4) = arena<void>(0x2300 + k * 0x10);
                          *(void **)arena<uint8_t>(0x2300 + k * 0x10 + 0xc) = arena<void>(0x3000 + k * 0x400);
                          arena<void *>(0x3000 + k * 0x400)[0] = vt;
                          *arena<int32_t>(0x3000 + k * 0x400 + 0x190) = kinds[r.range(0, 2)];
                          *arena<int32_t>(0x3000 + k * 0x400 + 0x198) = kinds[r.range(0, 2)];
                      }
                  },
                  [](void *fn) {
                      *(int32_t *)0x130654c = 1;
                      return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x800))};
                  }};
    w4d_find.prepare = [](const rewrite::Entry *) { w4d_saved = *(uint32_t *)0x130654c; };
    w4d_find.finish = [](const rewrite::Entry *) { *(uint32_t *)0x130654c = w4d_saved; };
    quick(w4d_find);
    // Slots::Match: sides 0-3, slots 0-1, ends and forced flags in words with junk above the byte; conversions fail now and then
    Test w4d_match{0x71d330,
                   [=](Rng &r) {
                       w4c_fill(r);
                       for (uint32_t at : {0x2000u, 0x2100u}) {
                           arena<uint8_t>(at)[0x8d] = (uint8_t)r.range(0, 3);
                           *arena<uint32_t>(at + 0xa4) = r.range(0, 1);
                       }
                       *arena<uint32_t>(0x10) = (r.next() & ~0xffu) | r.range(0, 3);
                       *arena<uint32_t>(0x14) = (r.next() & ~0xffu) | (r.range(0, 2) ? 0 : r.range(1, 255));
                   },
                   [](void *fn) {
                       return Result{((uint32_t(__stdcall *)(void *, void *, uint32_t, uint32_t))fn)(arena<void>(0x2000), arena<void>(0x2100), *arena<uint32_t>(0x10),
                                                                                                   *arena<uint32_t>(0x14)) & 0xff};
                   }};
    w4d_match.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x71b0f0, (void *)w4d_stub_convert}}); };
    w4d_match.finish = w4c_unstub;
    quick(w4d_match);
    // Batches::Add: four batches, one picked (or index 0); some fill up and are freed, blocks present or not
    Test w4d_batch{0x8552b0,
                   [=](Rng &r) {
                       w4c_fill(r);
                       *arena<void *>(0x800) = arena<void>(0x2000);
                       for (int i = 0; i < 4; i++) {
                           uint8_t *b = arena<uint8_t>(0x2000 + i * 0x14);
                           int32_t per = r.range(1, 3), n = r.range(0, 3);
                           *(int32_t *)b = per;
                           *(void **)(b + 4) = arena<void>(0x2100 + i * 0x10);
                           *(int32_t *)(b + 8) = n;
                           *(void **)(b + 0xc) = arena<void>(0x2200 + i * 0x40);
                           *(int32_t *)(b + 0x10) = per * n ? r.range(0, per * n - 1) : 0;
                           for (int j = 0; j < 4; j++) {
                               uint8_t *block = arena<uint8_t>(0x2400 + (i * 4 + j) * 0x20);
                               *(uint32_t *)block = r.range(1, 9);
                               arena<void *>(0x2100 + i * 0x10)[j] = r.range(0, 3) ? block + 4 : nullptr;
                           }
                       }
                       *arena<uint32_t>(0x10) = r.range(0, 3);
                   },
                   [](void *fn) {
                       ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x10), *arena<uint32_t>(0x20));
                       return Result{};
                   }};
    w4d_batch.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xddcf39, (void *)w4d_stub_vector_dtor}, {0xbce010, (void *)stub_free_aligned}}); };
    w4d_batch.finish = w4c_unstub;
    quick(w4d_batch);
    // Once::Make2: made or not, in the copy's own global
    Test w4d_once{0x85b290,
                  [=](Rng &r) {
                      w4c_fill(r);
                      for (int i = 0; i < 4; i++) arena<uint32_t>(0x10)[i] = r.next();
                  },
                  [](void *fn) {
                      uint32_t *a = arena<uint32_t>(0x10);
                      *(uint32_t *)(uintptr_t)w4c_global = a[3];
                      W4dWords14 unused{};
                      void *back = ((void *(__thiscall *)(void *, void *, void *, W4dWords14))fn)(arena<void>(0x800), a, a + 1, unused);
                      return Result{(uint32_t)(uintptr_t)back ^ *(uint32_t *)(uintptr_t)w4c_global};
                  }};
    w4d_once.prepare = [](const rewrite::Entry *e) {
        w4c_global = *(uint32_t *)(e->at + 0x1a); w4c_global_saved = *(uint32_t *)(uintptr_t)w4c_global;
        stubs_for_now({{0x416110, (void *)w4b_stub_table}});
    };
    w4d_once.finish = [](const rewrite::Entry *) { *(uint32_t *)(uintptr_t)w4c_global = w4c_global_saved; unstub_many(); };
    quick(w4d_once);
    // Grid::CellRef: bounds of 1-3 cells each way, points just inside and outside; each cell's reference cached or not, on the shared handle
    Test w4d_cell{0xabd360,
                  [=](Rng &r) {
                      w4c_fill(r);
                      iface_handle(r);
                      uint8_t *self = arena<uint8_t>(0x800);
                      int32_t x0 = r.range(0, 4) - 2, y0 = r.range(0, 4) - 2, x1 = x0 + r.range(0, 2), y1 = y0 + r.range(0, 2);
                      *(int32_t *)(self + 0x4c) = x0; *(int32_t *)(self + 0x50) = y0; *(int32_t *)(self + 0x54) = x1; *(int32_t *)(self + 0x58) = y1;
                      *(void **)(self + 0x5c) = arena<void>(0x2000);
                      for (int c = 0; c < 9; c++)
                          for (uint32_t ref : {0u, 0x1cu}) {
                              uint8_t *at = arena<uint8_t>(0x2000 + c * 0x58 + ref);
                              *(void **)(at + 4) = arena<void>(0x340);
                              *(uint32_t *)(at + 0x14) = r.next();
                              at[0x18] = (uint8_t)r.range(0, 1);
                          }
                      *arena<int32_t>(0x10) = x0 - 1 + r.range(0, 4);
                      *arena<int32_t>(0x14) = y0 - 1 + r.range(0, 4);
                  },
                  [](void *fn) {
                      return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t, int32_t))fn)(arena<void>(0x800), *arena<int32_t>(0x10), *arena<int32_t>(0x14))};
                  }};
    w4d_cell.prepare = [](const rewrite::Entry *) { g_iface_id = (uint32_t *)0x13ce3bc; };
    quick(w4d_cell);
    // Words::Push: full or not, empty or not, growing through the copy's own call
    Test w4d_push{0x427790,
                  [=](Rng &r) {
                      w4c_fill(r);
                      uint32_t *list = arena<uint32_t>(0x800 + w4c_off);
                      int32_t cap = r.range(0, 4);
                      list[0] = (uint32_t)(uintptr_t)arena<void>(0x2000);
                      list[1] = (uint32_t)cap;
                      list[2] = (uint32_t)(r.range(0, 1) ? cap : r.range(0, cap));
                  },
                  [](void *fn) {
                      ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x20));
                      return Result{};
                  }};
    w4d_push.prepare = [](const rewrite::Entry *e) { w4c_off = *(uint32_t *)(e->at + 5) - 8; stubs_for_now({{w4c_target(e, 0x25), (void *)w4d_stub_grow}}); };
    w4d_push.finish = w4c_unstub;
    quick(w4d_push);
    // Owned::Destroy
    Test w4d_owned{0xc65790, w4c_fill, w4c_self};
    w4d_owned.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbce010, (void *)stub_free_aligned}}); };
    w4d_owned.finish = w4c_unstub;
    quick(w4d_owned);
    // Single::Get: there already or not; out of memory or not; the maker registering itself or not
    Test w4d_single{0x436350, w4c_fill, [](void *fn) {
                        *(void **)(uintptr_t)w4d_instance = *arena<uint32_t>(0x20) & 2 ? arena<void>(0x900) : nullptr;
                        return Result{(uint32_t)(uintptr_t)((void *(__cdecl *)())fn)()};
                    }};
    w4d_single.prepare = [](const rewrite::Entry *e) {
        w4d_instance = *(uint32_t *)(e->at + 0x17); w4c_global_saved = *(uint32_t *)(uintptr_t)w4d_instance;
        stubs_for_now({{0xbcdfc0, (void *)w4c_stub_alloc}, {w4c_target(e, 0x41), (void *)w4d_stub_make3}});
    };
    w4d_single.finish = [](const rewrite::Entry *) { *(uint32_t *)(uintptr_t)w4d_instance = w4c_global_saved; unstub_many(); };
    quick(w4d_single);
    // Holder::Destroy
    Test w4d_holder{0x42bae0, w4c_fill, w4c_self};
    w4d_holder.prepare = [](const rewrite::Entry *e) { stubs_for_now({{0xcd2b50, (void *)w4d_stub_member_gone}, {w4c_target(e, 0x4b), (void *)w4d_stub_base_gone}}); };
    w4d_holder.finish = w4c_unstub;
    quick(w4d_holder);
    // Thing::New: out of memory or not
    Test w4d_new{0x6c6150, w4c_fill, [](void *fn) {
                     return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(uint32_t, uint32_t))fn)(*arena<uint32_t>(0x800), *arena<uint32_t>(0x804))};
                 }};
    w4d_new.prepare = [](const rewrite::Entry *e) { stubs_for_now({{0xbcdfc0, (void *)w4c_stub_alloc}, {w4c_target(e, 0x43), (void *)w4d_stub_make4}}); };
    w4d_new.finish = w4c_unstub;
    quick(w4d_new);
    // Boxed::Destroy: owning something or not, at the copy's own offset
    Test w4d_boxed{0xbfca60, [=](Rng &r) { w4c_fill(r); *(void **)arena<uint8_t>(0x800 + w4c_off) = r.range(0, 2) ? arena<void>(0x900) : nullptr; }, w4c_self};
    w4d_boxed.prepare = [](const rewrite::Entry *e) {
        w4c_off = *(uint8_t *)(e->at + 0x26);
        stubs_for_now({{w4c_target(e, 0x36), (void *)w4d_stub_gone_a}, {0xbce000, (void *)stub_free}, {w4c_target(e, 0x4e), (void *)w4d_stub_base_gone}});
    };
    w4d_boxed.finish = w4c_unstub;
    quick(w4d_boxed);
    // Shared::Release: the count reaching zero or not
    Test w4d_shared{0xb367b0,
                    [=](Rng &r) {
                        w4c_fill(r);
                        *arena<void *>(0x800) = arena<void>(0x900);
                        *arena<int32_t>(0x900) = r.range(1, 2);
                    },
                    w4c_self};
    w4d_shared.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{w4c_target(e, 0x29), (void *)w4d_stub_gone_a}, {0xbce010, (void *)stub_free_aligned}, {0xbce000, (void *)stub_free}});
    };
    w4d_shared.finish = w4c_unstub;
    quick(w4d_shared);
    // Twin::Destroy
    Test w4d_twin{0xb15320, w4c_fill, w4c_self};
    w4d_twin.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xb0e0d0, (void *)w4d_stub_gone_a}, {0x40f730, (void *)w4d_stub_gone_b}, {0xb7a4c0, (void *)w4d_stub_base_gone}});
    };
    w4d_twin.finish = w4c_unstub;
    quick(w4d_twin);
    // Pair::Release: each slot negative, zero or positive
    Test w4d_pair{0x8535c0, [=](Rng &r) { w4c_fill(r); arena<int32_t>(0x800)[3] = r.range(0, 3) - 1; arena<int32_t>(0x800)[8] = r.range(0, 3) - 1; }, w4c_self};
    w4d_pair.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x8501c0, (void *)w4d_stub_give_back}}); };
    w4d_pair.finish = w4c_unstub;
    quick(w4d_pair);
    // Owners::Clear: 0-4 items, some empty; the list there or not
    Test w4d_clear{0x7fbad0,
                   [=](Rng &r) {
                       w4c_fill(r);
                       static void *vt[1] = {(void *)w4d_stub_delete};
                       uint32_t *self = arena<uint32_t>(0x800);
                       self[1] = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x2000) : 0;
                       self[2] = (uint32_t)(r.range(0, 5) - 1);
                       for (int i = 0; i < 4; i++) {
                           arena<void *>(0x2100 + i * 0x10)[0] = vt;
                           arena<void *>(0x2000)[i] = r.range(0, 3) ? arena<void>(0x2100 + i * 0x10) : nullptr;
                       }
                       if (!self[1] && (int32_t)self[2] > 0) self[2] = 0;
                   },
                   w4c_self};
    w4d_clear.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbce010, (void *)stub_free_aligned}}); };
    w4d_clear.finish = w4c_unstub;
    quick(w4d_clear);
    // Mark::New (out of memory would write through null in both, so memory is always there)
    Test w4d_mark{0xb14b70, w4c_fill, [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w4d_mark.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfc0, (void *)w4d_stub_alloc_ok}, {0xb60b60, (void *)w4d_stub_mark}}); };
    w4d_mark.finish = w4c_unstub;
    quick(w4d_mark);
    // Boxed::Add: out of memory or not, through the copy's own adder
    Test w4d_badd{0xc33cb0, w4c_fill, [](void *fn) {
                      return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x804), *arena<uint32_t>(0x808))};
                  }};
    w4d_badd.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbcdfc0, (void *)w4c_stub_alloc}, {0xca8610, (void *)w4d_stub_box}, {w4c_target(e, 0x54), (void *)w4d_stub_add}});
    };
    w4d_badd.finish = w4c_unstub;
    quick(w4d_badd);
    // Watch::Add: the watcher had or not; both lists empty or holding one node
    Test w4d_watch{0x784b20,
                   [=](Rng &r) {
                       w4c_fill(r);
                       *arena<void *>(0x2620) = arena<void>(0x2700);
                       *(void **)arena<uint8_t>(0x800 + w4c_off) = arena<void>(0x2800);
                       for (uint32_t h : {0x2700u, 0x2800u}) {
                           W4dNode *head = arena<W4dNode>(h), *other = arena<W4dNode>(h + 0x10);
                           if (r.range(0, 1)) {
                               head->next = head->prev = other;
                               other->next = other->prev = head;
                           } else {
                               head->next = head->prev = head;
                           }
                       }
                   },
                   [](void *fn) {
                       w4d_allocs = 0;
                       return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), *arena<uint32_t>(0x804))};
                   }};
    w4d_watch.prepare = [](const rewrite::Entry *e) {
        w4c_off = *(uint8_t *)(e->at + 0x91);
        stubs_for_now({{0xbcdfc0, (void *)w4d_stub_alloc_two}, {0x805700, (void *)w4d_stub_target}});
    };
    w4d_watch.finish = w4c_unstub;
    quick(w4d_watch);

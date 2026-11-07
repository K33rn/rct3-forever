    // Stamp::Compare: each part mostly equal to the other, so every level decides sometimes; values that differ as signed and unsigned
    Test w2_stamp{0x88f750,
                  [=](Rng &r) {
                      static const uint32_t values[] = {0, 1, 0x80000000, 0xffffffff};
                      uint32_t *a = arena<uint32_t>(0x400), *b = arena<uint32_t>(0x420);
                      for (int i = 0; i < 6; i++) {
                          a[i] = values[r.range(0, 3)];
                          b[i] = r.range(0, 4) ? a[i] : values[r.range(0, 3)];
                      }
                  },
                  [](void *fn) {
                      uint32_t *a = arena<uint32_t>(0x400), *b = arena<uint32_t>(0x420);
                      return Result{(uint32_t)((int32_t(__stdcall *)(uint32_t, uint32_t, void *, uint32_t, uint32_t, uint32_t, void *, uint32_t))fn)(
                          a[0], a[1], a + 2, a[5], b[0], b[1], b + 2, b[5])};
                  }};
    quick(w2_stamp);
    // a node at `at` between neighbours at `p` and `n`, properly linked
    auto w2_link = [](Rng &r, size_t at, size_t p, size_t n) {
        void **node = arena<void *>(at), **prev = arena<void *>(p), **next = arena<void *>(n);
        prev[0] = node; prev[1] = (void *)(uintptr_t)r.next();
        next[0] = (void *)(uintptr_t)r.next(); next[1] = node;
        node[0] = next; node[1] = prev;
    };
    // Linked::LeaveBoth: the two nodes at 0x800 and 0x80c, each between two neighbours
    Test w2_leave_both{0x49b6e0,
                       [=](Rng &r) {
                           fresh(r);
                           w2_link(r, 0x800, 0x900, 0x910);
                           w2_link(r, 0x80c, 0x920, 0x930);
                       },
                       [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    quick(w2_leave_both);
    // Linked::Leave: linked well, or each way of not being: no node, no neighbour, or a neighbour that does not point back
    Test w2_leave{0x593700,
                  [=](Rng &r) {
                      fresh(r);
                      w2_link(r, 0x800, 0x900, 0x910);
                      void **node = arena<void *>(0x800), **prev = arena<void *>(0x900), **next = arena<void *>(0x910);
                      switch (r.range(0, 7)) {
                      case 0: node[0] = nullptr; break;
                      case 1: node[1] = nullptr; break;
                      case 2: next[1] = prev; break;
                      case 3: prev[0] = next; break;
                      }
                      arena<uint32_t>(0x20)[0] = r.range(0, 9) ? 1 : 0;
                  },
                  [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<uint32_t>(0x20)[0] ? arena<void>(0x800) : nullptr); return Result{}; }};
    quick(w2_leave);
    // ListPair::Construct: over old contents
    Test w2_list_pair{0xbfa820,
                      [=](Rng &r) { fresh(r); },
                      [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x1c0)[0])}; }};
    quick(w2_list_pair);
    // Event::Send: the handler at slot 0x20 records the event words
    Test w2_event{0x8451d0,
                  [=](Rng &r) {
                      fresh(r);
                      w2_event_table[0x20] = (void *)w2_stub_event;
                      arena<void *>(0x400)[0] = w2_event_table;
                      arena<uint32_t>(0x20)[0] = r.next();
                      arena<uint32_t>(0x20)[1] = r.next();
                      arena<uint32_t>(0x20)[2] = r.next();
                  },
                  [](void *fn) {
                      uint32_t *v = arena<uint32_t>(0x20);
                      return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x400), v[1], v[2])};
                  }};
    quick(w2_event);
    // Lock::Release: held or not, 0-2 holders, 0-3 jobs waiting
    Test w2_lock{0xbe9be0,
                 [=](Rng &r) {
                     fresh(r);
                     w2_job_table[0xc] = (void *)w2_stub_job;
                     uint8_t *self = arena<uint8_t>(0x100);
                     *(uint32_t *)(self + 0x20) = (uint32_t)r.range(0, 3);
                     *(void **)(self + 0x18) = arena<void>(0x300);
                     arena<void *>(0x300)[1] = arena<void>(0x400);
                     arena<void *>(0x400)[1] = arena<void>(0x500);
                     arena<void *>(0x500)[0] = w2_job_table;
                     arena<uint16_t>(0x40c)[0] = (uint16_t)r.range(0, 2);
                     arena<uint16_t>(0x40e)[0] = (uint16_t)r.range(0, 3);
                 },
                 [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(w2_lock);
    // Triple::Forward: each copy has its own target, read from its call
    Test w2_triple{0x48bb00,
                   [=](Rng &r) {
                       fresh(r);
                       for (int i = 0; i < 5; i++) arena<uint32_t>(0x20)[i] = r.next();
                   },
                   [](void *fn) {
                       uint32_t *v = arena<uint32_t>(0x20);
                       return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), v[2], v[3], v[4])};
                   }};
    w2_triple.prepare = [](const rewrite::Entry *e) { stubs_for_now({{e->at + 0x23 + *(int32_t *)(e->at + 0x1f), (void *)w2_stub_triple}}); };
    w2_triple.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w2_triple);
    // Node::Free: a node between two neighbours
    Test w2_node_free{0x846450,
                      [=](Rng &r) { fresh(r); w2_link(r, 0x800, 0x900, 0x910); },
                      [](void *fn) { ((void(__stdcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    quick(w2_node_free);
    // Children::Tell: 0-4 children, some places empty; a child may shrink the count while it runs
    Test w2_tell{0xb6c590,
                 [=](Rng &r) {
                     fresh(r);
                     uint8_t *self = arena<uint8_t>(0x600);
                     *(void **)(self + 0x2b4) = arena<void>(0xa00);
                     *(int32_t *)(self + 0x2b8) = r.range(-1, 4);
                     for (int i = 0; i < 4; i++) {
                         arena<void *>(0xa00)[i] = r.range(0, 3) ? arena<void>(0xa40 + i * 0x10) : nullptr;
                         arena<void *>(0xa40 + i * 0x10)[0] = w2_tell_table;
                     }
                     arena<uint32_t>(0x20)[2] = r.next();
                     arena<uint32_t>(0x20)[3] = r.next();
                 },
                 [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x600), arena<uint32_t>(0x20)[3]); return Result{}; }};
    quick(w2_tell);
    // ArrayHolder::Clear: nothing held, an array with objects (virtual delete) or without (freed from BACK bytes before)
    static uint32_t g_w2_back;
    Test w2_array{0x47e380,
                  [=](Rng &r) {
                      fresh(r);
                      arena<void *>(0x100)[0] = r.range(0, 3) ? arena<void>(0x810) : nullptr;
                      arena<void *>(0x810)[0] = w2_array_table;
                      *(uint32_t *)arena<uint8_t>(0x810 - g_w2_back) = (uint32_t)r.range(0, 1);
                  },
                  [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w2_array.prepare = [](const rewrite::Entry *e) { g_w2_back = 0x100 - *(uint8_t *)(e->at + 0xb); };
    quick(w2_array);
    // Refs::InterfaceAt: handles with an object or without
    static uint32_t g_w2_refs_off;
    Test w2_refs_at{0x738270,
                    [=](Rng &r) {
                        fresh(r);
                        iface_handle(r);
                        for (int i = 0; i < 4; i++) *(void **)arena<uint8_t>(0x800 + g_w2_refs_off + i * 20 + 4) = r.range(0, 3) ? arena<void>(0x340) : arena<void>(0x380);
                        *(void **)arena<uint8_t>(0x380 + 0xc) = nullptr;
                        arena<uint32_t>(0x20)[0] = (uint32_t)r.range(0, 3);
                    },
                    [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t))fn)(arena<void>(0x800), arena<int32_t>(0x20)[0])}; }};
    w2_refs_at.prepare = [](const rewrite::Entry *e) { g_iface_id = (uint32_t *)0x1310f90; g_w2_refs_off = *(uint32_t *)(e->at + 0xa); };
    quick(w2_refs_at);
    // Record::Assign: this copy has its own table, or the object has another one; the other side has its base offset
    static uint32_t g_w2_assign_table;
    Test w2_assign{0x5d3de0,
                   [=](Rng &r) {
                       fresh(r);
                       uint32_t *self = arena<uint32_t>(0x800), *other = arena<uint32_t>(0xa00);
                       for (int i = 0; i < 0x40; i++) { self[i] = r.next(); other[i] = r.next(); }
                       self[0] = r.range(0, 2) ? g_w2_assign_table : 0x1234;
                       other[0] = (uint32_t)(uintptr_t)arena<void>(0xc00);
                       arena<int32_t>(0xc00)[1] = r.range(1, 9) * 4;
                   },
                   [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0xa00))}; }};
    w2_assign.prepare = [](const rewrite::Entry *e) {
        g_w2_assign_table = *(uint32_t *)(e->at + 7);
        stubs_for_now({{0x5d2970, (void *)stub_thiscall1}});
    };
    w2_assign.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w2_assign);
    // Quad::Construct
    Test w2_quad{0xca8000,
                 [=](Rng &r) { fresh(r); for (int i = 0; i < 4; i++) arena<uint32_t>(0x20)[i] = r.next(); },
                 [](void *fn) {
                     uint32_t *v = arena<uint32_t>(0x20);
                     return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), v[0], v[1], v[2], v[3])};
                 }};
    quick(w2_quad);
    // Kind::Map: every kind, and some past the end
    Test w2_clamp{0x5d1470,
                  [=](Rng &r) { arena<uint32_t>(0x20)[0] = r.range(0, 3) ? (uint32_t)r.range(0, 9) : r.next(); },
                  [](void *fn) { return Result{((uint32_t(__fastcall *)(uint32_t))fn)(arena<uint32_t>(0x20)[0])}; }};
    quick(w2_clamp);
    // Words::Copy
    auto w2_copy_call = [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0x100))}; };
    auto w2_copy_setup = [=](Rng &r) { fresh(r); for (int i = 0; i < 16; i++) arena<uint32_t>(0x800)[i] = r.next(); };
    Test w2_copy6{0x843950, w2_copy_setup, w2_copy_call};
    quick(w2_copy6);
    Test w2_copy4{0x47d700, w2_copy_setup, w2_copy_call};
    quick(w2_copy4);
    // Items::ValueAt: some places empty
    Test w2_value_at{0x7578c0,
                     [=](Rng &r) {
                         fresh(r);
                         arena<void *>(0x100)[1] = arena<void>(0x800);
                         for (int i = 0; i < 4; i++) {
                             arena<void *>(0x800)[i] = r.range(0, 2) ? arena<void>(0x900 + i * 0x80) : nullptr;
                             *(uint32_t *)arena<uint8_t>(0x900 + i * 0x80 + 0x44) = r.next();
                         }
                         arena<uint32_t>(0x20)[0] = (uint32_t)r.range(0, 3);
                     },
                     [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x20)[0])}; }};
    quick(w2_value_at);
    // Slots::Reset: a count from none (or below) to 6
    Test w2_slots{0x581240,
                  [=](Rng &r) {
                      fresh(r);
                      arena<void *>(0x100)[1] = arena<void>(0x800);
                      arena<int32_t>(0x100)[2] = r.range(-1, 6);
                      for (int i = 0; i < 8; i++) arena<uint32_t>(0x800)[i] = r.next();
                  },
                  [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(w2_slots);

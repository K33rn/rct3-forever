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
    auto w2_values = [=](Rng &r) { fresh(r); for (int i = 0; i < 6; i++) arena<uint32_t>(0x20)[i] = r.next(); };
    auto w2_unstub = [](const rewrite::Entry *) { unstub_many(); };
    // Rows::Resize: rows held or not, a count of none or some
    Test w2_rows{0x45e8c0,
                 [=](Rng &r) {
                     fresh(r);
                     arena<void *>(0x100)[0] = r.range(0, 2) ? arena<void>(0x800) : nullptr;
                     arena<uint32_t>(0x20)[0] = (uint32_t)r.range(0, 5);
                     *arena<uint32_t>(ALLOC_NEXT) = 0;
                 },
                 [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0])}; }};
    w2_rows.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)stub_alloc_aligned}}); };
    w2_rows.finish = w2_unstub;
    quick(w2_rows);
    // Block::Delete: with or without freeing itself
    Test w2_block{0x72f200,
                  [=](Rng &r) { fresh(r); arena<void *>(0x100)[0] = arena<void>(0x800); arena<uint32_t>(0x20)[0] = (uint32_t)r.range(0, 3); },
                  [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0])}; }};
    quick(w2_block);
    // SharedBlock: the last reference or not
    auto w2_shared_block = [=](Rng &r) {
        fresh(r);
        arena<uint32_t>(0x800)[0] = (uint32_t)r.range(1, 3);
        arena<void *>(0x800)[3] = arena<void>(0x900);
        arena<void *>(0x100)[0] = arena<void>(0x800);
        arena<uint32_t>(0x20)[0] = (uint32_t)r.range(0, 3);
    };
    Test w2_block_release{0x75a530, w2_shared_block, [](void *fn) { ((void(__stdcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    quick(w2_block_release);
    Test w2_block_delete{0x438f30, w2_shared_block,
                         [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0])}; }};
    quick(w2_block_delete);
    // Owner::Allowed: the global flag, the record flag, the state
    Test w2_allowed{0x5a1760,
                    [=](Rng &r) {
                        fresh(r);
                        *(uint8_t *)0x12f552a = r.range(0, 3) ? 0 : (uint8_t)r.range(1, 255);
                        *(uint32_t *)arena<uint8_t>(0x510 + 0x30) = r.next() & ~0x200u | (r.range(0, 1) ? 0x200 : 0);
                        *(void **)arena<uint8_t>(0x800 + 0x6f4) = arena<void>(0x700);
                        *(int32_t *)arena<uint8_t>(0x800 + 0x38c) = r.range(10, 14);
                    },
                    [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w2_allowed.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbb4280, (void *)stub_lookup_a}, {0xbb3e70, (void *)stub_lookup_b}}); };
    w2_allowed.finish = [](const rewrite::Entry *) { *(uint8_t *)0x12f552a = 0; unstub_many(); };
    quick(w2_allowed);
    // Field::Is
    Test w2_field_is{0xcc1c30,
                     [=](Rng &r) {
                         fresh(r);
                         arena<uint32_t>(0x20)[0] = r.range(0, 1) ? arena<uint32_t>(0x100)[6] : r.next();
                     },
                     [](void *fn) { return Result{((uint8_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0])}; }};
    quick(w2_field_is);
    // Ref::Copy, Ref::Assign
    auto w2_ref_setup = [=](Rng &r) {
        fresh(r);
        arena<void *>(0x100)[0] = arena<void>(0x800);
        arena<void *>(0x100)[1] = arena<void>(0x810);
    };
    auto w2_ref_call = [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x104))}; };
    auto w2_ref_stubs = [](const rewrite::Entry *) { stubs_for_now({{0xcc2110, (void *)w2_stub_add_ref}, {0xcc2250, (void *)w2_stub_drop_ref}}); };
    Test w2_ref_copy{0xcc2010, w2_ref_setup, w2_ref_call};
    w2_ref_copy.prepare = w2_ref_stubs;
    w2_ref_copy.finish = w2_unstub;
    quick(w2_ref_copy);
    Test w2_ref_assign{0xcc2050, w2_ref_setup, w2_ref_call};
    w2_ref_assign.prepare = w2_ref_stubs;
    w2_ref_assign.finish = w2_unstub;
    quick(w2_ref_assign);
    // Text::Store: strings from empty up
    Test w2_text{0x403be0,
                 [=](Rng &r) {
                     fresh(r);
                     int n = r.range(0, 12);
                     for (int i = 0; i < n; i++) arena<char>(0x800)[i] = (char)r.range(1, 255);
                     arena<char>(0x800)[n] = 0;
                 },
                 [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x180), arena<void>(0x800))}; }};
    w2_text.prepare = [](const rewrite::Entry *e) { stubs_for_now({{e->at + 0x2c + *(int32_t *)(e->at + 0x28), (void *)w2_stub_text}}); };
    w2_text.finish = w2_unstub;
    quick(w2_text);
    // Entries::Visit: some entries active; a visit may shorten the count
    Test w2_visit{0xb6c360,
                  [=](Rng &r) {
                      fresh(r);
                      arena<void *>(0x154)[0] = arena<void>(0x800);
                      arena<int32_t>(0x160)[0] = r.range(-1, 5);
                      for (int i = 0; i < 5; i++) {
                          arena<uint8_t>(0x800 + i * 12)[0] = (uint8_t)(r.range(0, 2) ? r.range(1, 255) : 0);
                          arena<uint32_t>(0x804 + i * 12)[0] = r.next();
                      }
                      arena<uint32_t>(0x20)[0] = r.next();
                  },
                  [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w2_visit.prepare = [](const rewrite::Entry *e) { stubs_for_now({{e->at + 0x24 + *(int32_t *)(e->at + 0x20), (void *)w2_stub_visit}}); };
    w2_visit.finish = w2_unstub;
    quick(w2_visit);
    // Writer::Put: the write can move the place
    Test w2_put{0xca55f0,
                [=](Rng &r) { w2_values(r); arena<void *>(0x100)[0] = w2_put_table; },
                [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[2])}; }};
    quick(w2_put);
    // Typed::Forward
    Test w2_typed{0x6af080, w2_values,
                  [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[1])}; }};
    w2_typed.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6aed50, (void *)w2_stub_typed}}); };
    w2_typed.finish = w2_unstub;
    quick(w2_typed);
    // Event::Ask, Event::AskFlag: the handler answers or leaves the starting answer
    Test w2_ask{0x845ab0,
                [=](Rng &r) { w2_values(r); w2_ask_table[0x20] = (void *)w2_stub_ask; arena<void *>(0x100)[0] = w2_ask_table; },
                [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    quick(w2_ask);
    Test w2_ask_flag{0x845a50,
                     [=](Rng &r) { w2_values(r); w2_ask_flag_table[0x20] = (void *)w2_stub_ask_flag; arena<void *>(0x100)[0] = w2_ask_flag_table; },
                     [](void *fn) { return Result{((uint8_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    quick(w2_ask_flag);
    // Level::Field: each level around the limit; each copy has its own field, and both are filled
    Test w2_level_field{0x4a15f0,
                        [=](Rng &r) {
                            w2_values(r);
                            arena<uint8_t>(0x510)[1] = (uint8_t)r.range(0, 4);
                        },
                        [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w2_level_field.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbb5130, (void *)stub_lookup_a}, {0xbb3e70, (void *)stub_lookup_b}}); };
    w2_level_field.finish = w2_unstub;
    quick(w2_level_field);
    // Holder::Delete
    Test w2_holder{0xbea240,
                   [=](Rng &r) {
                       fresh(r);
                       arena<void *>(0x100)[1] = arena<void>(0x800);
                       arena<void *>(0x800)[0] = w2_go_table;
                       arena<uint32_t>(0x20)[0] = (uint32_t)r.range(0, 3);
                   },
                   [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0])}; }};
    quick(w2_holder);
    // Counted::Drop: the last or not
    Test w2_counted{0xc321e0,
                    [=](Rng &r) {
                        fresh(r);
                        arena<void *>(0x100)[0] = arena<void>(0x800);
                        arena<void *>(0x800)[0] = w2_counted_table;
                        arena<uint32_t>(0x800)[1] = (uint32_t)r.range(1, 3);
                    },
                    [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(w2_counted);
    // Items::Pass: each copy has its own target, read from its call
    Test w2_pass{0x8387c0,
                 [=](Rng &r) {
                     w2_values(r);
                     arena<void *>(0x154)[0] = arena<void>(0x800);
                     for (int i = 0; i < 4; i++) arena<void *>(0x800)[i] = arena<void>(0x900 + i * 0x10);
                     arena<uint32_t>(0x20)[1] = (uint32_t)r.range(0, 3);
                 },
                 [](void *fn) {
                     uint32_t *v = arena<uint32_t>(0x20);
                     return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), v[1], v[2], v[3])};
                 }};
    w2_pass.prepare = [](const rewrite::Entry *e) { stubs_for_now({{e->at + 0x19 + *(int32_t *)(e->at + 0x15), (void *)w2_stub_pass}}); };
    w2_pass.finish = w2_unstub;
    quick(w2_pass);
    // Word::MakeDefault
    Test w2_default{0x49cd50, w2_values,
                    [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w2_default.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbd0490, (void *)w2_stub_default}}); };
    w2_default.finish = w2_unstub;
    quick(w2_default);
    // a list of `count` nodes from `at` (each 0x10 apart), ending in an end node; its values from `value`
    auto w2_chain = [](size_t at, int count, const std::function<void *(int)> &value) {
        for (int i = 0; i <= count; i++) {
            void **n = arena<void *>(at + i * 0x10);
            n[0] = i < count ? (void *)arena<void>(at + (i + 1) * 0x10) : nullptr;
            n[1] = i ? (void *)arena<void>(at + (i - 1) * 0x10) : (void *)arena<void>(at + 0x100);
            n[2] = i < count ? value(i) : nullptr;
        }
        arena<void *>(at + 0x100)[0] = arena<void>(at);
    };
    // List::RemoveAll: 0-5 nodes holding one of two values
    static uint32_t g_w2_ra_off;
    Test w2_remove_all{0x59ab70,
                       [=](Rng &r) {
                           fresh(r);
                           *(void **)arena<uint8_t>(0x800 + g_w2_ra_off) = arena<void>(0xc00);
                           w2_chain(0xc00, r.range(0, 5), [&](int) { return (void *)(uintptr_t)r.range(1, 2); });
                       },
                       [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x800), (void *)1); return Result{}; }};
    w2_remove_all.prepare = [](const rewrite::Entry *e) { g_w2_ra_off = *(uint32_t *)(e->at + 3); };
    quick(w2_remove_all);
    // Rows::Total: 0-4 rows; all three fields filled, each copy reads its own
    Test w2_total{0x6bc850,
                  [=](Rng &r) {
                      fresh(r);
                      *(void **)arena<uint8_t>(0x16c) = arena<void>(0x800);
                      *(int32_t *)arena<uint8_t>(0x170) = r.range(-1, 4);
                      for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x800)[i] = r.next();
                      *(void **)0x13ce934 = arena<void>(0x700);
                  },
                  [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w2_total.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xb329f0, (void *)stub_thiscall1}}); };
    w2_total.finish = [](const rewrite::Entry *) { *(void **)0x13ce934 = nullptr; unstub_many(); };
    quick(w2_total);
    // Interface::Member: found or not (then it is the offset alone)
    Test w2_member{0x736010,
                   [=](Rng &r) {
                       fresh(r);
                       iface_handle(r);
                       *(void **)arena<uint8_t>(0x800 + 0x94) = arena<void>(0x340);
                   },
                   [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w2_member.prepare = [](const rewrite::Entry *) { g_iface_id = (uint32_t *)0x12f51b4; };
    quick(w2_member);
    // Lookup::Then
    Test w2_lookup{0x48ca20, w2_values,
                   [](void *fn) {
                       uint32_t *v = arena<uint32_t>(0x20);
                       return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), v[2], v[3])};
                   }};
    w2_lookup.prepare = [](const rewrite::Entry *e) { stubs_for_now({{0x44cc40, (void *)w2_stub_lookup}, {e->at + 0x17 + *(int32_t *)(e->at + 0x13), (void *)w2_stub_found}}); };
    w2_lookup.finish = w2_unstub;
    quick(w2_lookup);
    // Member::Read, Member::Into
    auto w2_member_call = [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x800))}; };
    Test w2_read{0x9eaf10, w2_values, w2_member_call};
    w2_read.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x9ea680, (void *)w2_stub_read}}); };
    w2_read.finish = w2_unstub;
    quick(w2_read);
    Test w2_into{0x495bf0, w2_values, w2_member_call};
    w2_into.prepare = [](const rewrite::Entry *e) { stubs_for_now({{e->at + 0x19 + *(int32_t *)(e->at + 0x15), (void *)w2_stub_into}}); };
    w2_into.finish = w2_unstub;
    quick(w2_into);
    // Base::Delete: each copy has its own destructor, read from its call
    auto w2_flags_call = [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0] & 3)}; };
    Test w2_base{0x4029c0, w2_values, w2_flags_call};
    w2_base.prepare = [](const rewrite::Entry *e) { stubs_for_now({{e->at + 0xe + *(int32_t *)(e->at + 0xa), (void *)w2_stub_update}}); };
    w2_base.finish = w2_unstub;
    quick(w2_base);
    // Member::Update: each copy has its own update, read from its call
    Test w2_update{0x6c6db0,
                   [=](Rng &r) { w2_values(r); for (int i = 0; i < 0x40; i++) arena<void *>(0x800)[0x40 + i] = arena<void>(0x400 + i * 4); },
                   [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1])}; }};
    w2_update.prepare = [](const rewrite::Entry *e) { stubs_for_now({{e->at + 8 + *(int32_t *)(e->at + 4), (void *)w2_stub_update}, {0x6a0040, (void *)w2_stub_one}}); };
    w2_update.finish = w2_unstub;
    quick(w2_update);
    // Refs::TellFlagged: 0-4 references, looked up or kept, with the flag or without; each copy has its own id and function
    Test w2_flagged{0x54b020,
                    [=](Rng &r) {
                        fresh(r);
                        iface_handle(r);
                        arena<void *>(0x100)[0] = arena<void>(0xc00);
                        w2_chain(0xc00, r.range(0, 4), [&](int i) {
                            uint8_t *ref = arena<uint8_t>(0xe00 + i * 0x20);
                            *(void **)(ref + 4) = arena<void>(0x340);
                            ref[0x18] = (uint8_t)r.range(0, 1);
                            *(void **)(ref + 0x14) = r.range(0, 3) ? arena<void>(0x3c0) : nullptr;
                            return (void *)ref;
                        });
                        for (size_t at : {0x3a0, 0x3a8, 0x400}) *arena<uint8_t>(at) = (uint8_t)r.next();
                        arena<uint32_t>(0x20)[0] = r.next();
                    },
                    [](void *fn) { ((void(__stdcall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0]); return Result{}; }};
    w2_flagged.prepare = [](const rewrite::Entry *e) {
        g_iface_id = *(uint32_t **)(e->at + 0x38);
        stubs_for_now({{e->at + 0x70 + *(int32_t *)(e->at + 0x6c), (void *)stub_thiscall1}});
    };
    w2_flagged.finish = w2_unstub;
    quick(w2_flagged);
    // Log::Line: with a log or without, with text or without
    Test w2_log{0xa3e860,
                [=](Rng &r) {
                    fresh(r);
                    arena<void *>(0x100)[0] = r.range(0, 3) ? arena<void>(0x700) : nullptr;
                    int n = r.range(0, 9);
                    for (int i = 0; i < n; i++) arena<char>(0x800)[i] = (char)r.range(1, 255);
                    arena<char>(0x800)[n] = 0;
                    arena<uint32_t>(0x20)[0] = r.range(0, 3);
                },
                [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0] ? arena<void>(0x800) : nullptr); return Result{}; }};
    w2_log.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbe4060, (void *)w2_stub_log}}); };
    w2_log.finish = w2_unstub;
    quick(w2_log);
    // Chunks::Take: no chunk yet, room left, full with a next chunk, full without
    Test w2_chunks{0x42c7d0,
                   [=](Rng &r) {
                       fresh(r);
                       *arena<uint32_t>(ALLOC_NEXT) = 0;
                       void **self = arena<void *>(0x100);
                       self[0] = self[1] = r.range(0, 3) ? arena<void>(0x800) : nullptr;
                       arena<int32_t>(0x800 + 0x500)[0] = r.range(0x3e, 0x41);
                       arena<void *>(0x800 + 0x504)[0] = r.range(0, 1) ? arena<void>(0xe00) : nullptr;
                       arena<int32_t>(0xe00 + 0x500)[0] = r.range(0, 9);
                   },
                   [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w2_chunks.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc_next}, {e->at + 0x22 + *(int32_t *)(e->at + 0x1e), (void *)w2_stub_chunk}});
    };
    w2_chunks.finish = w2_unstub;
    quick(w2_chunks);
    // Order::Move*: five items on the order list, edges only to later items; marks, ranks and the pass all small so every test goes both ways
    auto w2_order = [=](Rng &r) {
        fresh(r);
        uint8_t *self = arena<uint8_t>(0x100);
        *(int32_t *)(self + 0x18) = r.range(1, 3);
        *(uint32_t *)(self + 0x1c) = (uint32_t)r.range(0, 3);
        *(uint32_t *)(self + 0x20) = (uint32_t)r.range(0, 3);
        auto item = [](int i) { return arena<uint8_t>(0x800 + i * 0x80); };
        void **start = arena<void *>(0xc00), **end = arena<void *>(0xc10);
        start[0] = item(0) + 0x50; start[1] = arena<void>(0xc20);
        end[0] = arena<void>(0xc30); end[1] = item(4) + 0x50;
        for (int i = 0; i < 5; i++) {
            uint8_t *it = item(i);
            *(int32_t *)(it + 0x18) = r.range(0, 3);
            *(int32_t *)(it + 0x1c) = r.range(0, 5);
            *(void **)(it + 0x50) = i < 4 ? (void *)(item(i + 1) + 0x50) : (void *)end;
            *(void **)(it + 0x54) = i ? (void *)(item(i - 1) + 0x50) : (void *)start;
            size_t edges = 0x1000 + i * 0x100;
            *(void **)(it + 0x20) = arena<void>(edges);
            int n = 0;
            for (int j = i + 1; j < 5; j++)
                if (r.range(0, 1)) {
                    void **e = arena<void *>(edges + n * 0x10);
                    e[0] = arena<void>(edges + (n + 1) * 0x10);
                    e[2] = item(j);
                    n++;
                }
            arena<void *>(edges + n * 0x10)[0] = nullptr;
        }
        int a = r.range(0, 3);
        arena<uint32_t>(0x20)[0] = a;
        arena<uint32_t>(0x20)[1] = (uint32_t)r.range(a + 1, 4);
    };
    auto w2_order_call = [](void *fn) {
        uint32_t *v = arena<uint32_t>(0x20);
        return Result{((uint8_t(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x800 + v[0] * 0x80), arena<void>(0x800 + v[1] * 0x80))};
    };
    Test w2_before{0x42c490, w2_order, w2_order_call};
    quick(w2_before);
    Test w2_after{0x42c550, w2_order, w2_order_call};
    quick(w2_after);
    // List::VisitAt: each copy has its own function, read from its call
    static uint32_t g_w2_va_off;
    Test w2_visit_at{0x6a1e20,
                     [=](Rng &r) {
                         fresh(r);
                         *(void **)arena<uint8_t>(0x800 + g_w2_va_off) = arena<void>(0xc00);
                         w2_chain(0xc00, r.range(0, 4), [&](int) { return (void *)(uintptr_t)r.next(); });
                     },
                     [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w2_visit_at.prepare = [](const rewrite::Entry *e) {
        g_w2_va_off = *(uint8_t *)(e->at + 3);
        stubs_for_now({{e->at + 0x18 + *(int32_t *)(e->at + 0x14), (void *)stub_plain_dtor}});
    };
    w2_visit_at.finish = w2_unstub;
    quick(w2_visit_at);
    // Owner::Delete: owning or not
    Test w2_owner{0x99af20,
                  [=](Rng &r) {
                      w2_values(r);
                      arena<void *>(0x100)[1] = r.range(0, 3) ? arena<void>(0x800) : nullptr;
                      arena<void *>(0x800)[0] = w2_counted_table;
                  },
                  w2_flags_call};
    quick(w2_owner);
    // Ref::Field: looked up or kept; always found, as the game assumes; each copy has its own place, id and field
    static uint32_t g_w2_rf_ref;
    Test w2_ref_field{0x532ea0,
                      [=](Rng &r) {
                          fresh(r);
                          iface_handle(r);
                          *(void **)arena<uint8_t>(0x340 + 0xc) = arena<void>(0x360);
                          *arena<void *>(0x1f0) = arena<void>(0x200);
                          *g_iface_id = (uint32_t)r.range(1, 2);
                          for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x3a0)[i] = r.next();
                          uint8_t *ref = arena<uint8_t>(0x800 + g_w2_rf_ref);
                          *(void **)(ref + 4) = arena<void>(0x340);
                          *(void **)(ref + 0x14) = arena<void>(0x368);
                          ref[0x18] = (uint8_t)r.range(0, 1);
                      },
                      [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w2_ref_field.prepare = [](const rewrite::Entry *e) {
        g_w2_rf_ref = *(uint32_t *)(e->at + 0xf) - 4;
        g_iface_id = *(uint32_t **)(e->at + 0x28);
    };
    quick(w2_ref_field);
    // Message::Post: each copy has its size, queue and functions; the stubs write and read the whole message
    Test w2_post{0x7bb470,
                 [=](Rng &r) {
                     w2_values(r);
                     arena<void *>(0x800)[0] = w2_flush_table;
                     *(uint32_t *)arena<uint8_t>(0x800 + 0x104) = r.range(0, 1) ? r.next() : 0;
                 },
                 [](void *fn) {
                     uint32_t *v = arena<uint32_t>(0x20);
                     ((void(__thiscall *)(void *, uint32_t, uint32_t, void *))fn)(arena<void>(0x100), v[0], v[1], arena<void>(0x800));
                     return Result{};
                 }};
    w2_post.prepare = [](const rewrite::Entry *e) {
        g_w2_message_size = *(uint8_t *)(e->at + 6);
        stubs_for_now({{e->at + 0x20 + *(int32_t *)(e->at + 0x1c), (void *)w2_stub_build}, {e->at + 0x2c + *(int32_t *)(e->at + 0x28), (void *)w2_stub_post}});
    };
    w2_post.finish = w2_unstub;
    quick(w2_post);
    // SharedBlock::AssignPair: the same block or another, the last reference or not
    Test w2_block_pair{0x4a1670,
                       [=](Rng &r) {
                           fresh(r);
                           for (int i = 0; i < 4; i++) {
                               arena<uint32_t>(0x800 + i * 0x20)[0] = (uint32_t)r.range(1, 3);
                               arena<void *>(0x800 + i * 0x20)[3] = arena<void>(0xa00 + i * 0x10);
                           }
                           for (int k = 0; k < 2; k++) {
                               arena<void *>(0x100)[k] = arena<void>(0x800 + k * 0x40);
                               arena<void *>(0x180)[k] = arena<void>(0x800 + k * 0x40 + r.range(0, 1) * 0x20);
                           }
                       },
                       [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x180))}; }};
    quick(w2_block_pair);
    // State::Step: with a target or without, the check passing or failing, each state around the two that act
    Test w2_step{0x4e5fb0,
                 [=](Rng &r) {
                     fresh(r);
                     w2_state_table[0x15] = (void *)w2_stub_state2;
                     w2_member_table[4] = (void *)w2_stub_member;
                     arena<void *>(0x800)[0] = w2_state_table;
                     arena<void *>(0x800 + 0xe0)[0] = w2_member_table;
                     *(int32_t *)arena<uint8_t>(0x800 + 0xb0) = r.range(1, 6);
                     arena<void *>(0x20)[0] = r.range(0, 2) ? arena<void>(0xc00) : nullptr;
                     arena<uint32_t>(0x20)[1] = (uint32_t)r.range(0, 1) * 0x100;
                     arena<uint32_t>(0x20)[2] = (uint32_t)r.range(0, 1) * 3 + 0x100;
                     arena<uint32_t>(0x20)[3] = r.next();
                 },
                 [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[3]); return Result{}; }};
    w2_step.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x4d18c0, (void *)w2_stub_target}, {0x4d1840, (void *)w2_stub_has_target}, {0x4d2800, (void *)w2_stub_check}, {0x7a1840, (void *)stub_plain_dtor}});
    };
    w2_step.finish = w2_unstub;
    quick(w2_step);
    // Kind::Lookup: every kind, and past the end
    auto w2_kind_setup = [](Rng &r) { arena<uint32_t>(0x20)[0] = r.range(0, 3) ? (uint32_t)r.range(0, 6) : r.next(); };
    auto w2_kind_call = [](void *fn) { return Result{((uint32_t(__fastcall *)(uint32_t))fn)(arena<uint32_t>(0x20)[0])}; };
    Test w2_kind_a{0x59a2d0, w2_kind_setup, w2_kind_call};
    quick(w2_kind_a);
    Test w2_kind_b{0x5bd960, w2_kind_setup, w2_kind_call};
    quick(w2_kind_b);
    // Timer::Run: with a mode or without, the timer below 0 or not; the fields of both copies are filled
    Test w2_timer{0x543b40,
                  [=](Rng &r) {
                      w2_values(r);
                      uint8_t *s = arena<uint8_t>(0x800);
                      for (int off : {0x17c, 0x190}) *(uint32_t *)(s + off) = (uint32_t)r.range(0, 1);
                      for (int off : {0x188, 0x19c}) *(int32_t *)(s + off) = r.range(-2, 2);
                      for (int off : {0x184, 0x198}) *(uint32_t *)(s + off) = r.next();
                      *(void **)(s + 0xec) = arena<void>(0xc00);
                  },
                  [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w2_timer.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6b0990, (void *)w2_stub_plain_update}, {0x6afc20, (void *)w2_stub_timed_update}}); };
    w2_timer.finish = w2_unstub;
    quick(w2_timer);
    // Scanner::Flush: no buffer, the current one or another; the globals are set in the call and folded into the result
    static uint32_t g_w2_sc[6];
    Test w2_flush{0xc09740,
                  [=](Rng &r) {
                      fresh(r);
                      uint32_t *b = arena<uint32_t>(0x800);
                      for (int i = 0; i < 0x10; i++) b[i] = r.next();
                      b[1] = (uint32_t)(uintptr_t)arena<void>(0x900);
                      arena<uint32_t>(0x900)[0] = r.next();
                      arena<uint32_t>(0x20)[0] = (uint32_t)r.range(0, 4);
                      arena<uint32_t>(0x20)[1] = r.next();
                  },
                  [](void *fn) {
                      uint32_t kind = arena<uint32_t>(0x20)[0], seed = arena<uint32_t>(0x20)[1];
                      *(void **)(uintptr_t)g_w2_sc[0] = kind & 1 ? nullptr : arena<void>(0x800);
                      for (int i = 1; i < 5; i++) *(uint32_t *)(uintptr_t)g_w2_sc[i] = seed + i;
                      *(uint8_t *)(uintptr_t)g_w2_sc[5] = (uint8_t)seed;
                      ((void(__fastcall *)(void *))fn)(kind == 4 ? nullptr : arena<void>(0x800));
                      uint32_t sum = *(uint8_t *)(uintptr_t)g_w2_sc[5];
                      for (int i = 1; i < 5; i++) sum = sum * 31 + *(uint32_t *)(uintptr_t)g_w2_sc[i];
                      return Result{sum};
                  }};
    w2_flush.prepare = [](const rewrite::Entry *e) {
        static const int at[6] = {0x1b, 0x32, 0x3b, 0x43, 0x4b, 0x50};
        for (int i = 0; i < 6; i++) g_w2_sc[i] = *(uint32_t *)(e->at + at[i]);
    };
    quick(w2_flush);
    // Pooled::Set: nothing held, the last holder or not; a value or none
    static uint32_t g_w2_ps_off;
    Test w2_pooled{0xc0bc10,
                   [=](Rng &r) {
                       fresh(r);
                       arena<int32_t>(0x800)[0] = r.range(1, 3);
                       arena<int32_t>(0x810)[0] = r.range(1, 3);
                       *(void **)arena<uint8_t>(0x100 + g_w2_ps_off) = r.range(0, 3) ? arena<void>(0x800) : nullptr;
                       arena<uint32_t>(0x20)[0] = (uint32_t)r.range(0, 3);
                       *(uint32_t *)0x13e3318 = (uint32_t)(uintptr_t)arena<void>(0x5f0);
                   },
                   [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0] ? arena<void>(0x810) : nullptr)}; }};
    w2_pooled.prepare = [](const rewrite::Entry *e) {
        g_w2_ps_off = *(uint8_t *)(e->at + 6);
        stubs_for_now({{0xc0a750, (void *)stub_plain_dtor}, {0xca1f80, (void *)stub_thiscall1}});
    };
    w2_pooled.finish = w2_unstub;
    quick(w2_pooled);
    // Quad::Write: through the game's own writer, onto the stand-in slot it calls
    Test w2_quad_write{0xc2f850,
                       [=](Rng &r) {
                           w2_values(r);
                           arena<void *>(0x800)[0] = w2_put_table;
                           arena<uint32_t>(0x800)[3] = r.next();
                       },
                       [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x800))}; }};
    quick(w2_quad_write);
    // Flagged::Copy: each copy has its own place
    Test w2_flag_copy{0x581120, [=](Rng &r) { w2_values(r); for (int off : {0x100, 0x234, 0x240}) *arena<uint8_t>(0x800 + off) = (uint8_t)r.next(); },
                      [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0xc00))}; }};
    w2_flag_copy.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x423fb0, (void *)w2_stub_one}}); };
    w2_flag_copy.finish = w2_unstub;
    quick(w2_flag_copy);
    // Slot::Reset: below, inside and past the owner's table
    Test w2b_slot{0x8278b0,
                  [=](Rng &r) {
                      w2_values(r);
                      *(void **)(arena<uint8_t>(0x100) + 0x9c) = arena<void>(0x800);
                      *(void **)(arena<uint8_t>(0x100) + 0x140) = arena<void>(0x900);
                      arena<int32_t>(0x800)[0x70 / 4] = r.range(0, 4);
                      arena<int32_t>(0x20)[0] = r.range(-2, 5);
                      for (int i = 0; i < 0x44; i++) arena<uint32_t>(0x900)[i] = r.next();
                  },
                  [](void *fn) { ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x20)[0]); return Result{0}; }};
    quick(w2b_slot);
    // Visitor::Run: the walker sees each argument and the visitor's contents, and its answer comes back
    Test w2b_visitor{0x76ff60, [=](Rng &r) { w2_values(r); },
                     [](void *fn) {
                         uint32_t *v = arena<uint32_t>(0x20);
                         return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), v[1], v[2], v[3], v[4])};
                     }};
    w2b_visitor.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x76fb20, (void *)w2b_stub_walker}}); };
    w2b_visitor.finish = w2_unstub;
    quick(w2b_visitor);
    // Trio::Update: three members in order, the last one's answer back
    Test w2b_trio_test{0xb010d0,
                       [=](Rng &r) {
                           w2_values(r);
                           for (int off : {0x128, 0x12c, 0x130}) *(uint32_t *)(arena<uint8_t>(0x800) + off) = r.next();
                       },
                       [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w2b_trio_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6b0990, (void *)w2_stub_plain_update}, {0x6afc20, (void *)w2_stub_plain_update}}); };
    w2b_trio_test.finish = w2_unstub;
    quick(w2b_trio_test);
    // Owner::Drop: nothing held; held and still there after stopping; held and gone after stopping
    Test w2b_drop_test{0x40ddc0,
                       [=](Rng &r) {
                           w2_values(r);
                           arena<void *>(0x800)[0] = w2b_delete_table;
                           *(void **)(arena<uint8_t>(0x100) + 0x2c) = r.range(0, 3) ? arena<void>(0x800) : nullptr;
                       },
                       [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{0}; }};
    w2b_drop_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x771f90, (void *)w2b_stub_stop}, {0x771e50, (void *)w2b_stub_stop}}); };
    w2b_drop_test.finish = w2_unstub;
    quick(w2b_drop_test);
    // Listener::Construct: each copy's manager holds its own list, so the wrong list shows
    Test w2b_listener_test{0x872940,
                           [=](Rng &r) {
                               w2_values(r);
                               *(void **)(arena<uint8_t>(0x800) + 0x2c) = arena<void>(0x900);
                               *(void **)(arena<uint8_t>(0x800) + 0x20) = arena<void>(0x940);
                               arena<void *>(0x900)[0] = arena<void>(0x980);
                               arena<void *>(0x940)[0] = arena<void>(0x9c0);
                           },
                           [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *))fn)(arena<void>(0x100))}; }};
    w2b_listener_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x8839b0, (void *)w2b_stub_manager}, {0xa04bc0, (void *)w2b_stub_manager}}); };
    w2b_listener_test.finish = w2_unstub;
    quick(w2b_listener_test);
    // Owned::Clear: none, a negative count, gaps in the table
    Test w2b_clear_test{0xc2b760,
                        [=](Rng &r) {
                            w2_values(r);
                            arena<int32_t>(0x100)[0] = r.range(-1, 5);
                            arena<void *>(0x100)[1] = arena<void>(0x800);
                            for (int i = 0; i < 5; i++) arena<void *>(0x800)[i] = r.range(0, 2) ? arena<void>(0x900 + i * 0x10) : nullptr;
                        },
                        [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{0}; }};
    w2b_clear_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xc25690, (void *)stub_plain_dtor}, {0xc437f0, (void *)stub_plain_dtor}, {0xbce000, (void *)w2b_stub_free_item}, {0xbce010, (void *)w2b_stub_free_table}});
    };
    w2b_clear_test.finish = w2_unstub;
    quick(w2b_clear_test);
    // Part::Attach: with a scene and without, each copy's place
    Test w2b_attach_test{0x67abe0,
                         [=](Rng &r) {
                             w2_values(r);
                             void *scene = r.range(0, 2) ? arena<void>(0x800) : nullptr;
                             *(void **)(arena<uint8_t>(0x100) + 0xf8) = scene;
                             *(void **)(arena<uint8_t>(0x100) + 0x12c) = scene;
                         },
                         [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{0}; }};
    w2b_attach_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6afd60, (void *)stub_plain_dtor}, {0x6b1330, (void *)w2b_stub_scene}}); };
    w2b_attach_test.finish = w2_unstub;
    quick(w2b_attach_test);
    // Stages::Run: a different stand-in at each step, so a change of order shows
    Test w2b_stages_test{0x550af0, [=](Rng &r) { w2_values(r); },
                         [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w2b_stages_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x549aa0, (void *)stub_plain_dtor}, {0x5498c0, (void *)w2_stub_plain_update}, {0x54e360, (void *)w2_stub_timed_update}, {0x5499a0, (void *)w2b_stub_root},
                       {0x8f6cb0, (void *)stub_plain_dtor}, {0x8f6920, (void *)w2_stub_plain_update}, {0x8f33f0, (void *)w2_stub_timed_update}, {0x8f3b90, (void *)w2b_stub_root}});
    };
    w2b_stages_test.finish = w2_unstub;
    quick(w2b_stages_test);
    // Group::Update: the fixed members, then a list of none, some, or a negative count
    Test w2b_group_test{0x74d800,
                        [=](Rng &r) {
                            w2_values(r);
                            uint8_t *self = arena<uint8_t>(0x100);
                            *(uint32_t *)(self + 0x3c) = r.next();
                            *(uint32_t *)(self + 0x40) = r.next();
                            *(void **)(self + 0x44) = arena<void>(0x800);
                            *(int32_t *)(self + 0x48) = r.range(-1, 5);
                            for (int i = 0; i < 5; i++) arena<uint32_t>(0x800)[i] = r.next();
                        },
                        [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{0}; }};
    w2b_group_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6b0990, (void *)stub_plain_dtor}, {0x6afc20, (void *)stub_plain_dtor}}); };
    w2b_group_test.finish = w2_unstub;
    quick(w2b_group_test);
    // Node::Announce: no root, a root without the flag, a root with it
    Test w2b_announce_test{0x6afdc0,
                           [=](Rng &r) {
                               w2_values(r);
                               arena<uint32_t>(0x20)[1] = (uint32_t)r.range(0, 2);
                               arena<uint32_t>(0x800)[0x94 / 4] = (r.next() & ~0x800000u) | (r.range(0, 1) ? 0x800000u : 0);
                           },
                           [](void *fn) {
                               uint32_t root = *(uint32_t *)0x12f83f0, target = *(uint32_t *)0x1342708;
                               *(void **)0x12f83f0 = arena<uint32_t>(0x20)[1] ? arena<void>(0x800) : nullptr;
                               *(uint32_t *)0x1342708 = arena<uint32_t>(0x20)[2];
                               ((void(__fastcall *)(void *))fn)(arena<void>(0x100));
                               *(uint32_t *)0x12f83f0 = root;
                               *(uint32_t *)0x1342708 = target;
                               return Result{0};
                           }};
    w2b_announce_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6ae7c0, (void *)w2b_stub_visit}, {0xa1f890, (void *)w2_stub_one}, {0xa1f8a0, (void *)w2_stub_one}}); };
    w2b_announce_test.finish = w2_unstub;
    quick(w2b_announce_test);
    // Shared::Assign: the same block, a new one with the old still held, a new one with the old let go
    Test w2b_assign_test{0x41bea0,
                         [=](Rng &r) {
                             w2_values(r);
                             arena<int32_t>(0x800)[0] = r.range(1, 2);
                             arena<uint32_t>(0x800)[3] = r.next();
                             arena<int32_t>(0x840)[0] = r.range(1, 3);
                             *(void **)(arena<uint8_t>(0x100) + 0x114) = arena<void>(0x800);
                             *(void **)(arena<uint8_t>(0x100) + 0xb0) = arena<void>(0x800);
                             arena<void *>(0x40)[0] = r.range(0, 3) ? arena<void>(0x840) : arena<void>(0x800);
                         },
                         [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    w2b_assign_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbce000, (void *)w2b_stub_free_item}, {0xbce010, (void *)w2b_stub_free_table}}); };
    w2b_assign_test.finish = w2_unstub;
    quick(w2b_assign_test);
    // Quarter::Value: each index, and one past them that falls to the last
    Test w2b_quarter_test{0xb35490,
                          [=](Rng &r) {
                              w2_values(r);
                              *(int32_t *)(arena<uint8_t>(0x100) + 0xa4) = r.range(0, 4);
                              *(void **)(arena<uint8_t>(0x800) + 0x54) = arena<void>(0x880);
                              for (int i = 0; i < 4; i++) {
                                  *(void **)(arena<uint8_t>(0x880) + 0x18 + i * 4) = arena<void>(0x900 + i * 0x40);
                                  *(uint32_t *)(arena<uint8_t>(0x900 + i * 0x40) + 0x14) = r.next();
                                  *(uint32_t *)(arena<uint8_t>(0x900 + i * 0x40) + 0x2c) = r.next();
                              }
                          },
                          [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w2b_quarter_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xb44970, (void *)w2b_stub_root}}); };
    w2b_quarter_test.finish = w2_unstub;
    quick(w2b_quarter_test);
    // Setup::Once: already run, and each answer of each check; only the low byte of an answer counts
    Test w2b_setup_test{0xa12cb0,
                        [=](Rng &r) {
                            w2_values(r);
                            uint8_t flag = (uint8_t)(r.range(0, 3) ? 0 : 2);
                            arena<uint8_t>(0x100)[0x15c] = flag;
                            arena<uint8_t>(0x100)[0x14c] = flag;
                            arena<uint32_t>(0x20)[0] = (r.next() & ~0xffu) | (uint32_t)r.range(0, 1);
                            arena<uint32_t>(0x20)[1] = (r.next() & ~0xffu) | (uint32_t)r.range(0, 1);
                        },
                        [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{0}; }};
    w2b_setup_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xa113d0, (void *)w2b_stub_check1}, {0xa114a0, (void *)w2b_stub_check2}, {0xa11680, (void *)stub_plain_dtor}, {0xa11570, (void *)w2_stub_plain_update},
                       {0xa14bf0, (void *)w2b_stub_check1}, {0xa14cc0, (void *)w2b_stub_check2}, {0xa14ef0, (void *)stub_plain_dtor}, {0xa14d90, (void *)w2_stub_plain_update}});
    };
    w2b_setup_test.finish = w2_unstub;
    quick(w2b_setup_test);

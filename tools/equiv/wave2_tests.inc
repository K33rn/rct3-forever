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
    // Rows::Resize: an old table or none; a zero count, a count that gets its memory, one that does not
    Test w2b_resize_test{0x5d4350,
                         [=](Rng &r) {
                             w2_values(r);
                             arena<void *>(0x100)[0] = r.range(0, 1) ? arena<void>(0xa00) : nullptr;
                             arena<uint32_t>(0x20)[2] = (uint32_t)r.range(0, 3);
                         },
                         [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[2])}; }};
    w2b_resize_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbce010, (void *)w2b_stub_free_table}, {0xbcdfd0, (void *)w2b_stub_alloc}, {0x5d3d00, (void *)w2b_stub_build}});
    };
    w2b_resize_test.finish = w2_unstub;
    quick(w2b_resize_test);
    // Weights::Ensure: a place already filled, an empty one that gets its weight, an empty one that does not
    Test w2b_ensure_test{0x40c160,
                         [=](Rng &r) {
                             w2_values(r);
                             *(void **)(arena<uint8_t>(0x100) + 0x14) = arena<void>(0x800);
                             *(void **)(arena<uint8_t>(0x100) + 0x28) = arena<void>(0x800);
                             for (int i = 0; i < 4; i++) arena<uint32_t>(0x800)[i] = r.range(0, 1) ? r.next() | 1 : 0;
                             arena<uint32_t>(0x900)[0] = r.next();
                         },
                         [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[2])}; }};
    w2b_ensure_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x40b550, (void *)w2b_stub_find}, {0xcea320, (void *)w2b_stub_find}, {0xbcdfc0, (void *)w2b_stub_alloc}});
    };
    w2b_ensure_test.finish = w2_unstub;
    quick(w2b_ensure_test);
    // Quarter::Has: each index, with something there and nothing there, in each copy's place
    Test w2b_has_test{0xb356b0,
                      [=](Rng &r) {
                          w2_values(r);
                          *(int32_t *)(arena<uint8_t>(0x100) + 0xa4) = r.range(0, 4);
                          *(void **)(arena<uint8_t>(0x800) + 0x54) = arena<void>(0x880);
                          for (int i = 0; i < 4; i++) {
                              *(void **)(arena<uint8_t>(0x880) + 0x18 + i * 4) = arena<void>(0x900 + i * 0x40);
                              *(uint32_t *)(arena<uint8_t>(0x900 + i * 0x40) + 0x44) = r.range(0, 1) ? r.next() | 0x100 : 0;
                              *(uint32_t *)(arena<uint8_t>(0x900 + i * 0x40) + 0x48) = r.range(0, 1) ? r.next() | 0x100 : 0;
                          }
                      },
                      [](void *fn) { return Result{((uint8_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w2b_has_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xb44970, (void *)w2b_stub_root}}); };
    w2b_has_test.finish = w2_unstub;
    quick(w2b_has_test);
    // Judge::Act: not ready, not allowed, and each verdict
    Test w2b_judge_test{0x7a0f50,
                        [=](Rng &r) {
                            w2_values(r);
                            arena<uint32_t>(0x20)[0] = (r.next() & ~0xffu) | (uint32_t)(r.range(0, 3) ? 1 : 0);
                            arena<uint8_t>(0x100)[0x408] = (uint8_t)r.range(0, 3);
                        },
                        [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[2])}; }};
    w2b_judge_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x79ff40, (void *)w2b_stub_check1}, {0x7c4970, (void *)w2b_stub_verdict}, {0x7a0030, (void *)w2_stub_one}, {0x7a0000, (void *)w2_stub_timed_update}});
    };
    w2b_judge_test.finish = w2_unstub;
    quick(w2b_judge_test);
    // Cached::Pass: already looked up; looked up with a source; looked up without one
    static uint32_t g_w2b_seen;
    static void *w2b_target_table[0x49];
    Test w2b_cached_test{0xa80240,
                         [=](Rng &r) {
                             w2_values(r);
                             w2b_target_table[0x48] = (void *)w2_stub_one;
                             uint8_t *self = arena<uint8_t>(0x100);
                             *(uint32_t *)(self + g_w2b_seen) = (uint32_t)r.range(0, 1);
                             *(uint32_t *)(self + g_w2b_seen - 4) = r.next();
                             *(void **)(self + g_w2b_seen - 0x14) = arena<void>(0x800);
                             *(void **)(self + g_w2b_seen + 4) = arena<void>(0x900);
                             arena<void *>(0x900)[0] = w2b_target_table;
                             arena<void *>(0x800)[3] = r.range(0, 2) ? arena<void>(0x880) : nullptr;
                         },
                         [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w2b_cached_test.prepare = [](const rewrite::Entry *e) {
        g_w2b_seen = *(uint32_t *)(e->at + 5);
        stubs_for_now({{0x4a8700, (void *)w2_stub_plain_update}, {0xb85520, (void *)w2_stub_plain_update}});
    };
    w2b_cached_test.finish = w2_unstub;
    quick(w2b_cached_test);
    // ======== third pass ========
    auto w2c_add = [&](uintptr_t at, std::function<void(Rng &)> setup, std::function<Result(void *)> call, std::function<void(const rewrite::Entry *)> prep) {
        Test x{at, setup, call};
        x.prepare = prep;
        x.finish = w2_unstub;
        quick(x);
    };
    auto w2c_self_call = [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; };
    auto w2c_self_ret = [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; };
    auto w2c_self_arg = [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0])}; };
    auto w2c_void_arg = [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0]); return Result{}; };
    auto w2c_args2 = [](void *fn) { return Result{((uint32_t(__stdcall *)(uint32_t, uint32_t))fn)(arena<uint32_t>(0x20)[0], arena<uint32_t>(0x20)[1])}; };
    auto w2c_seed = [=](Rng &r) { w2_values(r); *arena<uint32_t>(0x40) = 0; };
    auto w2c_shared_make = [](Rng &r, uint32_t at, int low) {
        *arena<int32_t>(at) = r.range(low, 3);
        arena<uint32_t>(at)[3] = r.next();
    };
    // Singleton::Get: already there; built; out of memory
    w2c_add(0x40da80, w2c_seed,
            [](void *fn) {
                *(uint32_t *)(uintptr_t)g_w2c_a = arena<uint32_t>(0x20)[5] & 1 ? (uint32_t)(uintptr_t)arena<void>(0x700) : 0;
                Result out{((uint32_t(__cdecl *)())fn)()};
                *(uint32_t *)(uintptr_t)g_w2c_a = 0;
                return out;
            },
            [](const rewrite::Entry *e) {
                g_w2c_a = w2c_word(e->at, 0x17);
                stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {w2c_target(e->at, 0x40), (void *)w2c_ctor_sets}});
            });
    // Singleton::Keep: the same, and stored here
    w2c_add(0x4095b0, w2c_seed,
            [](void *fn) {
                *(uint32_t *)(uintptr_t)g_w2c_a = arena<uint32_t>(0x20)[5] & 1 ? (uint32_t)(uintptr_t)arena<void>(0x700) : 0;
                Result out{((uint32_t(__cdecl *)())fn)()};
                W2C_REC(0xe2cb, *(uint32_t *)(uintptr_t)g_w2c_a);
                *(uint32_t *)(uintptr_t)g_w2c_a = 0;
                return out;
            },
            [](const rewrite::Entry *e) {
                g_w2c_a = w2c_word(e->at, 0x17);
                stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {0xbe79b0, (void *)w2c_s2<0xe2d0>}});
            });
    // Factory::Make: with memory, without; the real buffer initialiser runs
    w2c_add(0x7ff4e0, w2c_seed, [](void *fn) { return Result{((uint32_t(__cdecl *)())fn)()}; },
            [](const rewrite::Entry *) { stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {0xbcdfd0, (void *)w2c_alloc_step}}); });
    // Shared::Assign: same buffer or another; each may reach no owners
    w2c_add(0x591190,
            [=](Rng &r) {
                w2c_seed(r);
                w2c_shared_make(r, 0x300, 1);
                w2c_shared_make(r, 0x340, 0);
                *(void **)(arena<uint8_t>(0x100) + g_w2c_a) = arena<void>(0x300);
                arena<uint32_t>(0x20)[0] = (uint32_t)(uintptr_t)arena<void>(r.range(0, 3) ? 0x340 : 0x300);
            },
            w2c_void_arg,
            [](const rewrite::Entry *e) {
                g_w2c_a = *(uint8_t *)(e->at + 0x1c);
                stubs_for_now({{0xbce010, (void *)w2c_free<0xe2d1>}, {0xbce000, (void *)w2c_free<0xe2d2>}});
            });
    // Shared::DropTwo: each buffer freed or still owned
    w2c_add(0x7fd6f0,
            [=](Rng &r) {
                w2c_seed(r);
                w2c_shared_make(r, 0x300, 1);
                w2c_shared_make(r, 0x340, 1);
                *(void **)(arena<uint8_t>(0x100) + g_w2c_a) = arena<void>(0x300);
                *(void **)(arena<uint8_t>(0x100) + g_w2c_b) = arena<void>(0x340);
            },
            w2c_self_call,
            [](const rewrite::Entry *e) {
                g_w2c_a = *(uint8_t *)(e->at + 0x20);
                g_w2c_b = *(uint8_t *)(e->at + 0x4b);
                stubs_for_now({{0xbce010, (void *)w2c_free<0xe2d1>}, {0xbce000, (void *)w2c_free<0xe2d2>}});
            });
    // the destructors: every member and base, in order, with each copy's own offsets (the calls record them)
    w2c_add(0x456980, w2c_seed, w2c_self_call, [](const rewrite::Entry *e) { stubs_for_now({{w2c_target(e->at, 0x36), (void *)w2c_s0<0xe2d3>}, {w2c_target(e->at, 0x45), (void *)w2c_s0<0xe2d4>}}); });
    w2c_add(0x4c68d0, w2c_seed, w2c_self_call, [](const rewrite::Entry *e) {
        uint32_t base = w2c_target(e->at, 0x45);
        if (base == 0x40f730) stubs_for_now({{0x40f730, (void *)w2c_s0<0xe2d3>}});
        else stubs_for_now({{0x40f730, (void *)w2c_s0<0xe2d3>}, {base, (void *)w2c_s0<0xe2d4>}});
    });
    w2c_add(0x49bbc0, w2c_seed, w2c_self_call, [](const rewrite::Entry *) { stubs_for_now({{0x40f730, (void *)w2c_s0<0xe2d3>}}); });
    w2c_add(0x496dc0, w2c_seed, w2c_self_call, [](const rewrite::Entry *e) { stubs_for_now({{w2c_target(e->at, 0x26), (void *)w2c_s0<0xe2d3>}, {w2c_target(e->at, 0x35), (void *)w2c_s0<0xe2d4>}}); });
    w2c_add(0x4dd140, w2c_seed, w2c_self_call, [](const rewrite::Entry *e) { stubs_for_now({{w2c_target(e->at, 0x33), (void *)w2c_s0<0xe2d3>}, {w2c_target(e->at, 0x42), (void *)w2c_s0<0xe2d4>}}); });
    // Dtor::FourTables: with an owned object and without
    w2c_add(0x41b1a0,
            [=](Rng &r) {
                w2c_seed(r);
                arena<void *>(0x800)[0] = w2c_table;
                *(void **)(arena<uint8_t>(0x100) + g_w2c_a) = r.range(0, 1) ? arena<void>(0x800) : nullptr;
            },
            w2c_self_call,
            [](const rewrite::Entry *e) {
                g_w2c_a = w2c_word(e->at, 0x40);
                stubs_for_now({{w2c_target(e->at, 0x61), (void *)w2c_s0<0xe2d3>}});
            });
    // Buffers::Init: each buffer its own block
    w2c_add(0x7fc020, w2c_seed, w2c_self_ret, [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)w2c_alloc_step}}); });
    // Buffers::Free: with and without the second block
    w2c_add(0x84f220,
            [=](Rng &r) {
                w2c_seed(r);
                if (r.range(0, 1)) *(uint32_t *)(arena<uint8_t>(0x100) + g_w2c_a) = 0;
                else *(uint32_t *)(arena<uint8_t>(0x100) + g_w2c_a) |= 1;
            },
            w2c_self_call,
            [](const rewrite::Entry *e) {
                g_w2c_a = *(uint8_t *)(e->at + 0x30);
                stubs_for_now({{0xbce010, (void *)w2c_free<0xe2d1>}});
            });
    // Make::OnceFor: already there; made; out of memory
    w2c_add(0x754de0,
            [=](Rng &r) {
                w2c_seed(r);
                *(uint32_t *)(arena<uint8_t>(0x100) + 0x4b4) = r.range(0, 1) ? 0 : r.next() | 1;
            },
            [](void *fn) { return Result{((uint8_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), 7)}; },
            [](const rewrite::Entry *e) { stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {w2c_target(e->at, 0x49), (void *)w2c_s2<0xe2d5>}}); });
    // the makers: with memory and without, each copy's size and constructor
    w2c_add(0x555cb0, w2c_seed, w2c_self_arg, [](const rewrite::Entry *e) { stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {w2c_target(e->at, 0x3f), (void *)w2c_s2<0xe2d5>}}); });
    w2c_add(0x632790, w2c_seed, w2c_self_ret, [](const rewrite::Entry *e) { stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {w2c_target(e->at, 0x49), (void *)w2c_s5<0xe2d5>}}); });
    w2c_add(0x6d1060, w2c_seed, w2c_args2, [](const rewrite::Entry *e) { stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {w2c_target(e->at, 0x45), (void *)w2c_s5<0xe2d5>}}); });
    w2c_add(0x6d5bd0, w2c_seed, w2c_args2, [](const rewrite::Entry *e) { stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {w2c_target(e->at, 0x42), (void *)w2c_s5<0xe2d5>}}); });
    // Make::FromPool: refused; accepted with and without pool memory
    w2c_add(0xc0e4c0, w2c_seed,
            [](void *fn) {
                *(void **)0x13e3318 = arena<void>(0x700);
                Result out{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0])};
                *(void **)0x13e3318 = nullptr;
                return out;
            },
            [](const rewrite::Entry *e) {
                stubs_for_now({{0xc0d5a0, (void *)w2c_s1<0xe2d6>}, {0xca1eb0, (void *)w2c_pool}, {w2c_target(e->at, 0x51), (void *)w2c_s2<0xe2d5>}});
            });
    // Make::Choosing: either word of the source, and out of memory
    w2c_add(0x4a8910,
            [=](Rng &r) {
                w2c_seed(r);
                for (int i = 0; i < 8; i++) arena<uint32_t>(0x800)[i] = r.next();
                *(void **)(arena<uint8_t>(0x100) + g_w2c_a) = arena<void>(0x800);
            },
            w2c_self_ret,
            [](const rewrite::Entry *e) {
                g_w2c_a = w2c_word(e->at, 0x3c);
                stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {w2c_target(e->at, 0x43), (void *)w2c_s0<0xe2d6>}, {0x4e2630, (void *)w2c_s3<0xe2d5>}});
            });
    // Text::Copy, Text::CopyLeft: with memory and without; the limit on both sides of the length, also past the sign bit
    auto w2c_text_seed = [=](Rng &r) {
        w2c_seed(r);
        arena<void *>(0x300)[0] = arena<void>(0x340);
        for (int i = 0; i < 4; i++) arena<uint32_t>(0x340)[i] = r.next();
        arena<uint32_t>(0x340)[1] = (uint32_t)r.range(0, 9) | (r.range(0, 3) ? 0 : 0x80000000u);
        arena<uint32_t>(0x20)[1] = (uint32_t)r.range(0, 9) | (r.range(0, 3) ? 0 : 0x80000000u);
    };
    w2c_add(0xbcfef0, w2c_text_seed,
            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x300))}; },
            [](const rewrite::Entry *e) { stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {w2c_target(e->at, 0x44), (void *)w2c_s2<0xe2d5>}}); });
    w2c_add(0xbcff60, w2c_text_seed,
            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x300), arena<uint32_t>(0x20)[1])}; },
            [](const rewrite::Entry *e) { stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {w2c_target(e->at, 0x5f), (void *)w2c_s2<0xe2d5>}}); });
    // Text::FromField: nothing there; something there with and without memory
    w2c_add(0x6aade0,
            [=](Rng &r) {
                w2c_seed(r);
                *(uint32_t *)(arena<uint8_t>(0x100) + g_w2c_a) = r.range(0, 2) ? r.next() | 1 : 0;
            },
            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x800))}; },
            [](const rewrite::Entry *e) {
                g_w2c_a = w2c_word(e->at, 0x21);
                stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {0xbcf7d0, (void *)w2c_s1<0xe2d5>}, {0xbd0490, (void *)w2c_flag}});
            });
    // Lookup::ByPair: two references, sometimes the same one; the key seen by both calls
    w2c_add(0x661660,
            [=](Rng &r) {
                w2c_seed(r);
                arena<uint32_t>(0x300)[0] = r.next() & 0xff;
                arena<uint32_t>(0x340)[0] = r.next() & 0xff;
                arena<void *>(0x380)[0] = arena<void>(0x300);
                arena<void *>(0x380)[1] = arena<void>(r.range(0, 3) ? 0x340 : 0x300);
            },
            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x380), arena<void>(0x384))}; },
            [](const rewrite::Entry *e) { stubs_for_now({{0xbe82b0, (void *)w2c_key_find}, {w2c_target(e->at, 0x54), (void *)w2c_key_drop}}); });
    // Ctor::Registered: what the owner is told, the class at that moment, and each text its own
    w2c_add(0x7fc400, w2c_seed,
            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0], arena<void>(0x800))}; },
            [](const rewrite::Entry *) { stubs_for_now({{0x7fbf50, (void *)w2c_join}, {0xbd0490, (void *)w2c_flag}}); });
    // Ctor::Grid: the four values in their pairs
    w2c_add(0x641750, w2c_seed,
            [](void *fn) {
                uint32_t *v = arena<uint32_t>(0x20);
                return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), v[0], v[1], v[2], v[5])};
            },
            [](const rewrite::Entry *) { stubs_for_now({{0x488290, (void *)w2c_grid_base}, {0xddcf99, (void *)w2c_vector_make}}); });
    // ======== fourth pass ========
    // Text::FromField: nothing held; held with and without memory; the copy's own field, maker and empty maker
    w2c_add(0x61cb70,
            [=](Rng &r) {
                w2c_seed(r);
                *(uint32_t *)(arena<uint8_t>(0x100) + g_w2c_a) = r.range(0, 2) ? r.next() | 1 : 0;
            },
            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x800))}; },
            [](const rewrite::Entry *e) {
                g_w2c_a = *(uint8_t *)(e->at + 0x21);
                stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {w2c_target(e->at, 0x44), (void *)w2c_s1<0xe2d5>}, {w2c_target(e->at, 0x51), (void *)w2c_flag}});
            });
    // Text::Ctor: no string; a string with and without memory
    w2c_add(0x401080, w2c_seed,
            [](void *fn) {
                uint32_t s = arena<uint32_t>(0x20)[0] & 1 ? arena<uint32_t>(0x20)[1] | 1 : 0;
                return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), s)};
            },
            [](const rewrite::Entry *e) {
                stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {w2c_target(e->at, 0x3f), (void *)w2c_s1<0xe2d5>}, {w2c_target(e->at, 0x4c), (void *)w2c_flag}});
            });
    // Text::FromOwner: no owner; an owner without and with a source (at the copy's own place); memory or none
    w2c_add(0x7312d0,
            [=](Rng &r) {
                w2c_seed(r);
                arena<void *>(0x100)[0x8c / 4] = r.range(0, 4) ? arena<void>(0x300) : nullptr;
                *(uint32_t *)(arena<uint8_t>(0x300) + g_w2c_a) = r.range(0, 2) ? r.next() | 1 : 0;
            },
            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x800))}; },
            [](const rewrite::Entry *e) {
                g_w2c_a = *(uint8_t *)(e->at + 0x2a);
                stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {0xbcf7d0, (void *)w2c_s1<0xe2d5>}, {0xbcf850, (void *)w2c_s1<0xe2d6>}});
            });
    // Text::NameOr: an object (at the copy's own place) with its name; none, the temporary kept alive or let go
    w2c_add(0x99ad00,
            [=](Rng &r) {
                w2c_seed(r);
                *(void **)(arena<uint8_t>(0x100) + g_w2c_a) = r.range(0, 2) ? arena<void>(0x300) : nullptr;
                *(void **)(arena<uint8_t>(0x300) + 0xf0) = arena<void>(0x380);
                arena<uint32_t>(0x380)[0] = r.range(1, 5);
            },
            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x800), arena<uint32_t>(0x20)[0])}; },
            [](const rewrite::Entry *e) {
                g_w2c_a = *(uint8_t *)(e->at + 0x1f);
                stubs_for_now({{0xbcdfc0, (void *)w2d_alloc_always}, {0xbcf850, (void *)w2d_text_make}, {0xbce010, (void *)w2c_free<0xe2d7>}, {0xbce000, (void *)w2c_free<0xe2d8>}});
            });
    // Reset::WithName: the text handed over, with and without memory
    w2c_add(0xbaa100, w2c_seed,
            [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x140)); return Result{}; },
            [](const rewrite::Entry *) { stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {0xbcf850, (void *)w2c_s1<0xe2d5>}, {0xba9700, (void *)w2c_s1<0xe2d6>}}); });
    // Add::Made: the part as built, with and without memory, added by the copy's own adder
    w2c_add(0xc33d20, w2c_seed,
            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0], arena<uint32_t>(0x20)[1])}; },
            [](const rewrite::Entry *e) { stubs_for_now({{0xbcdfc0, (void *)w2c_alloc}, {0xca8000, (void *)w2c_s4<0xe2d5>}, {w2c_target(e->at, 0x60), (void *)w2c_s2<0xe2d6>}}); });
    // Text::SetBuilt: the temporary built, its text stored, then let go
    w2c_add(0x757aa0, w2c_seed,
            [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0]); return Result{}; },
            [](const rewrite::Entry *) { stubs_for_now({{0x7578e0, (void *)w2d_build}, {0xbd14e0, (void *)w2c_s1<0xe2d5>}, {0xbd1880, (void *)w2d_build_drop}}); });
    // Dtor::TwoTexts: both texts, in order, then the copy's own base
    w2c_add(0x6e8280, w2c_seed, w2c_self_call,
            [](const rewrite::Entry *e) { stubs_for_now({{0x40f730, (void *)w2c_s0<0xe2d5>}, {w2c_target(e->at, 0x48), (void *)w2d_s_table<0xe2d6>}}); });
    w2c_add(0x510710, w2c_seed, w2c_self_call,
            [](const rewrite::Entry *e) { stubs_for_now({{w2c_target(e->at, 0x2c), (void *)w2c_s0<0xe2d5>}}); });
    // Dtor::SharedText: the text (at the copy's own place) kept or freed
    w2c_add(0x478630,
            [=](Rng &r) {
                w2c_seed(r);
                *(void **)(arena<uint8_t>(0x100) + g_w2c_a) = arena<void>(0x300);
                arena<uint32_t>(0x300)[0] = r.range(1, 4);
            },
            w2c_self_call,
            [](const rewrite::Entry *e) {
                g_w2c_a = *(uint8_t *)(e->at + 0x20);
                stubs_for_now({{0xbce010, (void *)w2c_free<0xe2d7>}, {0xbce000, (void *)w2c_free<0xe2d8>}, {w2c_target(e->at, 0x54), (void *)w2c_s0<0xe2d5>}});
            });
    // Dtor::ThreeFaces: the tables each step sees
    w2c_add(0x8e5d50, w2c_seed, w2c_self_call,
            [](const rewrite::Entry *e) {
                stubs_for_now({{w2c_target(e->at, 0x42), (void *)w2d_s_table<0xe2d5>}, {0x6835a0, (void *)w2d_s_table<0xe2d6>}, {0x5f7b70, (void *)w2d_s_table<0xe2d7>}});
            });
    // Dtor::Parts: an object and no object, with the copy's own parts
    w2c_add(0x6b1eb0, w2c_seed,
            [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<uint32_t>(0x20)[0] % 4 ? arena<void>(0x100) : nullptr); return Result{}; },
            [](const rewrite::Entry *e) {
                stubs_for_now({{w2c_target(e->at, 0x2e), (void *)w2c_s0<0xe2d5>}, {w2c_target(e->at, 0x43), (void *)w2c_s0<0xe2d6>}, {w2c_target(e->at, 0x52), (void *)w2c_s0<0xe2d7>}});
            });
    // Dtor::Handles: each handle there or not; the first called through the copy's own slot
    w2c_add(0xbfc920,
            [=](Rng &r) {
                w2c_seed(r);
                for (int i = 0; i < 24; i++) arena<void *>(0x400)[i] = (void *)w2c_s0<0xe2d5>;
                arena<void *>(0x400)[g_w2c_a / 4] = (void *)w2c_s0<0xe2d6>;
                for (int i = 0; i < 4; i++) arena<void *>(0x480)[i] = (void *)w2d_release<0xe2d7>;
                arena<void *>(0x300)[0] = arena<void>(0x400);
                arena<void *>(0x340)[0] = arena<void>(0x480);
                *(void **)(arena<uint8_t>(0x100) + g_w2c_b) = r.range(0, 3) ? arena<void>(0x300) : nullptr;
                *(void **)(arena<uint8_t>(0x100) + g_w2c_b + 4) = r.range(0, 3) ? arena<void>(0x340) : nullptr;
            },
            w2c_self_call,
            [](const rewrite::Entry *e) {
                g_w2c_a = *(uint8_t *)(e->at + 0x36);
                g_w2c_b = *(uint8_t *)(e->at + 0x25);
                stubs_for_now({{0xbfc840, (void *)w2d_s_table<0xe2d8>}});
            });
    // Dtor::Owned: something owned or not; the tables each step sees
    w2c_add(0x63ab90,
            [=](Rng &r) {
                w2c_seed(r);
                arena<void *>(0x400)[0] = (void *)w2c_s1<0xe2d5>;
                arena<void *>(0x300)[0] = arena<void>(0x400);
                arena<void *>(0x100)[0x44 / 4] = r.range(0, 3) ? arena<void>(0x300) : nullptr;
            },
            w2c_self_call,
            [](const rewrite::Entry *) { stubs_for_now({{0x6b0150, (void *)w2d_s_table<0xe2d6>}, {0x6a1df0, (void *)w2d_s_table<0xe2d7>}}); });
    // Array::Remake: old elements or none; zero or some new ones, with and without memory
    w2c_add(0x967360,
            [=](Rng &r) {
                w2c_seed(r);
                arena<uint32_t>(0x100)[0] = r.range(0, 2) ? r.next() | 1 : 0;
                arena<uint32_t>(0x20)[0] = r.range(0, 3) ? r.range(1, 6) : 0;
            },
            w2c_self_arg,
            [](const rewrite::Entry *) { stubs_for_now({{0xbce010, (void *)w2c_free<0xe2d7>}, {0xbcdfd0, (void *)w2c_alloc}, {0x402b90, (void *)w2d_vector_make}}); });
    // Release::Five: every held part, by the copy's own releasers
    w2c_add(0x501e90, w2c_seed, w2c_self_call,
            [](const rewrite::Entry *) { stubs_for_now({{0x6b0990, (void *)w2c_s0<0xe2d5>}, {0x6afc20, (void *)w2c_s0<0xe2d6>}}); });
    // Dtor::TextAndPart: the text (at the copy's own place) kept or freed
    w2c_add(0x45c1c0,
            [=](Rng &r) {
                w2c_seed(r);
                *(void **)(arena<uint8_t>(0x100) + g_w2c_a) = arena<void>(0x700);
                arena<uint32_t>(0x700)[0] = r.range(1, 4);
            },
            w2c_self_call,
            [](const rewrite::Entry *e) {
                g_w2c_a = w2c_word(e->at, 0x22);
                stubs_for_now({{0xbce010, (void *)w2c_free<0xe2d7>}, {0xbce000, (void *)w2c_free<0xe2d8>}, {w2c_target(e->at, 0x65), (void *)w2c_s0<0xe2d5>},
                               {w2c_target(e->at, 0x74), (void *)w2c_s0<0xe2d6>}});
            });
    // Text::ItemOr: inside and past the copy's own count; the temporary kept alive or let go
    w2c_add(0xc3c610,
            [=](Rng &r) {
                w2c_seed(r);
                *(uint32_t *)(arena<uint8_t>(0x100) + g_w2c_a) = r.range(0, 5);
                *(void **)(arena<uint8_t>(0x100) + g_w2c_b) = arena<void>(0x400);
                for (int i = 0; i < 6; i++) {
                    arena<void *>(0x400)[i] = arena<void>(0x500 + 0x10 * i);
                    arena<uint32_t>(0x500 + 0x10 * i)[0] = r.range(1, 5);
                }
                arena<uint32_t>(0x20)[1] = r.range(0, 6);
            },
            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x800), arena<uint32_t>(0x20)[1])}; },
            [](const rewrite::Entry *e) {
                g_w2c_a = *(uint8_t *)(e->at + 0x23);
                g_w2c_b = *(uint8_t *)(e->at + 0x29);
                stubs_for_now({{0xbcdfc0, (void *)w2d_alloc_always}, {0xbcfb10, (void *)w2d_text_make}, {0xbce010, (void *)w2c_free<0xe2d7>}, {0xbce000, (void *)w2c_free<0xe2d8>}});
            });
    // Look::Pair: kind one and the other
    w2c_add(0x4c5e10,
            [=](Rng &r) {
                w2c_seed(r);
                arena<uint32_t>(0x100)[0xc0 / 4] = r.range(0, 2) ? 1 : r.next();
                arena<uint32_t>(0x600)[0x120 / 4] = r.next();
            },
            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x800))}; },
            [](const rewrite::Entry *) { stubs_for_now({{0x4c5890, (void *)w2d_look}, {0xa87e00, (void *)w2d_look_make}}); });
    // Dtor::OwnedPart: the part and the owned object each there or not, at the copy's own places
    w2c_add(0x63a2a0,
            [=](Rng &r) {
                w2c_seed(r);
                arena<void *>(0x400)[0] = (void *)w2c_s1<0xe2d5>;
                arena<void *>(0x300)[0] = arena<void>(0x400);
                *(void **)(arena<uint8_t>(0x100) + g_w2c_a) = r.range(0, 3) ? arena<void>(0x380) : nullptr;
                *(void **)(arena<uint8_t>(0x100) + g_w2c_b) = r.range(0, 3) ? arena<void>(0x300) : nullptr;
            },
            w2c_self_call,
            [](const rewrite::Entry *e) {
                g_w2c_a = *(uint8_t *)(e->at + 0x3d);
                g_w2c_b = *(uint8_t *)(e->at + 0x5c);
                stubs_for_now({{0x6af760, (void *)w2c_s0<0xe2d8>}, {0xbce000, (void *)w2c_free<0xe2d9>}, {0x6b0150, (void *)w2d_s_table<0xe2d6>}, {0x6a1df0, (void *)w2d_s_table<0xe2d7>}});
            });
    // Grid::Mark: rectangles inside, clipped, inverted and empty; the copy's own flag and box
    w2c_add(0x874d90,
            [=](Rng &r) {
                w2c_seed(r);
                arena<int32_t>(0x100)[0x48 / 4] = r.range(0, 7);
                arena<int32_t>(0x100)[0x4c / 4] = r.range(0, 7);
                arena<void *>(0x100)[0x50 / 4] = arena<void>(0x1000);
                for (int i = 0; i < 4; i++) *(int32_t *)(arena<uint8_t>(0x100) + g_w2c_a + 4 * i) = r.range(-3, 10);
                for (int i = 0; i < 4; i++) arena<int32_t>(0x20)[i] = r.range(-2, 9);
            },
            [](void *fn) {
                int32_t *v = arena<int32_t>(0x20);
                ((void(__thiscall *)(void *, int32_t, int32_t, int32_t, int32_t))fn)(arena<void>(0x100), v[0], v[1], v[2], v[3]);
                return Result{};
            },
            [](const rewrite::Entry *e) { g_w2c_a = *(uint8_t *)(e->at + 0x78); });
    // Stat::Get: every plain statistic, table entries in and out of use and range, and out of range both ways
    w2c_add(0xbc5ab0,
            [=](Rng &r) {
                w2c_seed(r);
                for (int i = 0; i < 0x50; i++) arena<uint32_t>(0x100)[i] = r.next();
                for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x400)[i] = r.next();
                arena<uint32_t>(0x100)[0x18 / 4] = r.range(0, 3) ? 0x400000 | r.next() : r.next() & ~0x400000u;
                arena<int32_t>(0x100)[0xe4 / 4] = r.range(0, 4);
                arena<int32_t>(0x100)[0x11c / 4] = r.range(0, 25);
                arena<void *>(0x100)[0x120 / 4] = arena<void>(0x400);
                arena<int32_t>(0x20)[0] = r.range(-2, 0x1c);
            },
            w2c_self_arg, [](const rewrite::Entry *) {});
    // Pool::Release: a free or used block on either side, or none; the spare list in the copy's own globals
    w2c_add(0xbe9590,
            [=](Rng &r) {
                w2c_seed(r);
                uint8_t *b[4] = {arena<uint8_t>(0x300), arena<uint8_t>(0x340), arena<uint8_t>(0x380), arena<uint8_t>(0x3c0)};
                for (int i = 0; i < 4; i++) {
                    *(uint32_t *)(b[i] + 8) = r.range(0, 2);
                    *(int32_t *)(b[i] + 0x10) = r.range(1, 100);
                    *(uint8_t **)(b[i] + 0x14) = i ? b[i - 1] : nullptr;
                    *(uint8_t **)(b[i] + 0x18) = i < 3 ? b[i + 1] : nullptr;
                }
                if (r.range(0, 4) == 0) *(uint8_t **)(b[1] + 0x14) = nullptr;
                if (r.range(0, 4) == 0) *(uint8_t **)(b[1] + 0x18) = nullptr;
                if (r.range(0, 3) == 0) *(uint8_t **)(b[2] + 0x18) = nullptr;
                arena<int32_t>(0x100)[0x1c / 4] = r.range(0, 250);
            },
            [](void *fn) {
                uint32_t *spare = (uint32_t *)(uintptr_t)g_w2c_a, *count = (uint32_t *)(uintptr_t)g_w2c_b;
                uint32_t keep[2] = {*spare, *count};
                *spare = arena<uint32_t>(0x20)[2];
                *count = arena<uint32_t>(0x20)[3];
                ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x340));
                arena<uint32_t>(0x60)[0] = *spare;
                arena<uint32_t>(0x60)[1] = *count;
                *spare = keep[0];
                *count = keep[1];
                return Result{};
            },
            [](const rewrite::Entry *e) {
                g_w2c_a = w2c_word(e->at, 0x3e);
                g_w2c_b = w2c_word(e->at, 0x4b);
            });
    // a link properly in a ring (a broken one stops at a breakpoint in the original), its neighbours sometimes the same node
    auto w2d_ring = [](Rng &r, uint8_t *link, uint32_t at_prev, uint32_t at_next) {
        uint32_t *l = (uint32_t *)link, *p = arena<uint32_t>(at_prev), *n = arena<uint32_t>(at_next);
        if (r.range(0, 4) == 0) n = p;
        l[0] = (uint32_t)(uintptr_t)p;
        l[1] = (uint32_t)(uintptr_t)n;
        p[1] = (uint32_t)(uintptr_t)l;
        n[0] = (uint32_t)(uintptr_t)l;
    };
    // Dtor::Linked: out of its ring, the shared block still used or empty, the text kept or freed
    w2c_add(0xc18490,
            [=](Rng &r) {
                w2c_seed(r);
                w2d_ring(r, arena<uint8_t>(0x100) + (g_w2c_a & 0xff), 0x300, 0x340);
                *(void **)(arena<uint8_t>(0x100) + (g_w2c_a >> 8)) = arena<void>(0x700);
                arena<uint32_t>(0x700)[0] = r.range(1, 4);
                arena<void *>(0x380)[0] = arena<void>(0x3c0);
                arena<uint32_t>(0x3c0)[0] = r.range(0, 2);
            },
            [](void *fn) {
                void **shared = (void **)(uintptr_t)g_w2c_b, *keep = *shared;
                *shared = arena<void>(0x380);
                ((void(__fastcall *)(void *))fn)(arena<void>(0x100));
                arena<void *>(0x60)[0] = *shared;
                *shared = keep;
                return Result{};
            },
            [](const rewrite::Entry *e) {
                g_w2c_a = *(uint8_t *)(e->at + 0x25) | *(uint8_t *)(e->at + 0x83) << 8;
                g_w2c_b = w2c_word(e->at, 0x63);
                stubs_for_now({{0xbce010, (void *)w2c_free<0xe2d7>}, {0xbce000, (void *)w2c_free<0xe2d8>}});
            });
    // Dtor::Bound: settling or not; a bound object or none, each taken out of its ring
    w2c_add(0xbebf70,
            [=](Rng &r) {
                w2c_seed(r);
                uint8_t *self = arena<uint8_t>(0x100);
                *(uint32_t *)(self + (g_w2c_a & 0xff)) = r.range(0, 2);
                *(void **)(self + (g_w2c_a >> 8 & 0xff)) = r.range(0, 3) ? arena<void>(0x300) : nullptr;
                arena<void *>(0x300)[1] = arena<void>(0x340);
                arena<void *>(0x340)[0] = arena<void>(0x400);
                arena<void *>(0x400)[0] = (void *)w2c_s1<0xe2d5>;
                w2d_ring(r, arena<uint8_t>(0x350), 0x380, 0x3a0);
                w2d_ring(r, self + (g_w2c_a >> 16), 0x3c0, 0x3e0);
            },
            w2c_self_call,
            [](const rewrite::Entry *e) {
                g_w2c_a = *(uint8_t *)(e->at + 0x26) | *(uint8_t *)(e->at + 0x3a) << 8 | *(uint8_t *)(e->at + 0x85) << 16;
                stubs_for_now({{w2c_target(e->at, 0x34), (void *)w2d_s_table<0xe2d6>}});
            });
    // Text::Refresh: the text unchanged or new, the kept one let go or not, the asked one let go or not
    w2c_add(0x64a320,
            [=](Rng &r) {
                w2c_seed(r);
                arena<uint32_t>(0x40)[0] = 0;
                arena<int32_t>(0x340)[0] = r.range(1, 4);
                arena<int32_t>(0x380)[0] = r.range(1, 4);
                *(void **)(arena<uint8_t>(0x100) + (g_w2c_a & 0xff)) = arena<void>(0x340);
                *(void **)(arena<uint8_t>(0x100) + (g_w2c_a >> 8)) = arena<void>(0x700);
                arena<void *>(0x60)[0] = arena<void>(r.range(0, 3) ? 0x380 : 0x340);
                arena<void *>(0x3c0)[0] = arena<void>(r.range(0, 2) ? 0x380 : 0x340);
            },
            w2c_void_arg,
            [](const rewrite::Entry *e) {
                g_w2c_a = *(uint8_t *)(e->at + 0x2b) | *(uint8_t *)(e->at + 0x1c) << 8;
                stubs_for_now({{w2c_target(e->at, 0x23), (void *)w2d_find_text}, {0xbce010, (void *)w2c_free<0xe2d7>}, {0xbce000, (void *)w2c_free<0xe2d8>}});
            });
    // Dtor::Queues: each queue empty or holding some; the text kept or freed
    w2c_add(0x4a71f0,
            [=](Rng &r) {
                w2c_seed(r);
                arena<uint32_t>(0x40)[0] = arena<uint32_t>(0x40)[1] = 0;
                arena<void *>(0x400)[0] = (void *)w2c_s1<0xe2d5>;
                for (int i = 0; i < 4; i++) arena<void *>(0x300 + 0x40 * i)[0] = arena<void>(0x400);
                arena<void *>(0x100)[0x18 / 4] = arena<void>(0x700);
                arena<int32_t>(0x700)[0] = r.range(1, 4);
            },
            w2c_self_call,
            [](const rewrite::Entry *e) {
                stubs_for_now({{0xbe8400, (void *)w2d_queue_next}, {0xbe8230, (void *)w2c_s2<0xe2d6>}, {w2c_target(e->at, 0x7a), (void *)w2c_s0<0xe2da>},
                               {0xbe88b0, (void *)w2d_s_table<0xe2db>}, {0xbce010, (void *)w2c_free<0xe2d7>}, {0xbce000, (void *)w2c_free<0xe2d8>}});
            });
    // Array::Resize: the same count; none; fewer and more than before, from an old block or none
    w2c_add(0x430690,
            [=](Rng &r) {
                w2c_seed(r);
                arena<void *>(0x100)[0] = r.range(0, 4) ? arena<void>(0x300) : nullptr;
                arena<int32_t>(0x100)[1] = r.range(0, 5);
                arena<int32_t>(0x20)[0] = r.range(0, 4) ? r.range(0, 6) : arena<int32_t>(0x100)[1];
            },
            w2c_self_arg,
            [](const rewrite::Entry *e) {
                stubs_for_now({{0xbce010, (void *)w2c_free<0xe2d7>}, {0xbcdfd0, (void *)w2d_alloc_always}, {0x402b90, (void *)w2d_vector_make},
                               {w2c_target(e->at, 0x9c), (void *)w2c_s1<0xe2d5>}});
            });
    // ======== fifth pass (w2e) ========
    // Mesh::Transform: one to four vertices of finite floats through a finite matrix
    w2c_add(0x856830,
            [=](Rng &r) {
                w2c_seed(r);
                for (int i = 0; i < 16; i++) arena<uint32_t>(0x200)[i] = r.next() & 0xbfffffff;
                for (int i = 0; i < 4 * 9; i++) arena<uint32_t>(0x300)[i] = r.next() & 0xbfffffff;
            },
            [](void *fn) {
                ((void(__cdecl *)(void *, void *, int32_t, void *, uint32_t))fn)(arena<void>(0x500), arena<void>(0x300), 1 + arena<uint32_t>(0x20)[0] % 4,
                                                                                 arena<void>(0x200), arena<uint32_t>(0x20)[1]);
                return Result{};
            },
            [](const rewrite::Entry *) {});
    // Gfx::Create: switch on or off; the device says ok, invalid call or another failure
    w2c_add(0xc03d20,
            [=](Rng &r) {
                w2c_seed(r);
                arena<void *>(0x100)[1] = arena<void>(0x300);
                arena<void *>(0x300)[0] = arena<void>(0x400);
                arena<void *>(0x400)[11] = (void *)w2e_dev_make;
                for (int i = 0; i < 8; i++) arena<uint32_t>(0x200)[i] = r.next();
            },
            [](void *fn) {
                volatile uint8_t *flag = (volatile uint8_t *)0x13d3170;
                uint8_t was = *flag;
                *flag = arena<uint32_t>(0x20)[2] & 1 ? (uint8_t)(arena<uint32_t>(0x20)[2] >> 8 | 1) : 0;
                Result out{((uint32_t(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x200), arena<uint32_t>(0x20)[0])};
                *flag = was;
                return out;
            },
            [](const rewrite::Entry *) { stubs_for_now({{0xbd3c60, (void *)w2c_s0<0xe2f3>}}); });
    // View::ApplyTransform: the matrix handed back is the caller's buffer or a kept one
    w2c_add(0x630250,
            [=](Rng &r) {
                w2c_seed(r);
                for (int i = 0; i < 0x58; i++) arena<uint32_t>(0x100)[i] = r.next();
                for (int i = 0; i < 16; i++) arena<uint32_t>(0x600)[i] = r.next();
                arena<void *>(0x300)[0] = arena<void>(0x400);
                arena<void *>(0x400)[14] = (void *)w2c_s0<0xe2f4>;
                arena<void *>(0x400)[46] = (void *)w2e_dev_matrix;
            },
            [](void *fn) {
                void *was_dev = *(void **)0x13d0db8, *was_cam = *(void **)0x12f018c;
                *(void **)0x13d0db8 = arena<void>(0x300);
                *(void **)0x12f018c = arena<void>(0x700);
                ((void(__fastcall *)(void *))fn)(arena<void>(0x100));
                *(void **)0x13d0db8 = was_dev;
                *(void **)0x12f018c = was_cam;
                return Result{};
            },
            [](const rewrite::Entry *) { stubs_for_now({{0xcd78a0, (void *)w2e_make_matrix}, {0x480f80, (void *)w2c_s2<0xe2f5>}}); });

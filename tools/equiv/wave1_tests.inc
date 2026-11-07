    // Wave 1.
    static uint32_t w1_a, w1_b;
    auto w1_unstub = [](const rewrite::Entry *) { unstub_many(); };
    // BlockArea::Take: blocks at 0x1000, used or free, a largest that is right or too big; the split's block at 0x3000
    Test w1_take{0xbea3b0,
                 [=](Rng &r) {
                     fresh(r);
                     w1_fill(r, 0x3000, 0x20);
                     int n = r.range(0, 6), largest = 0;
                     void *prev = arena<void>(0x100);
                     *(void **)arena<uint8_t>(0x120) = nullptr;
                     for (int i = 0; i < n; i++) {
                         uint32_t *b = arena<uint32_t>(0x1000 + i * 0x20);
                         b[0] = r.next(); b[1] = r.next();
                         b[2] = r.range(0, 3) ? 0 : 4;
                         b[3] = r.range(0, 2000);
                         b[4] = r.range(0, 2) ? r.range(100, 1000) : r.range(0, 100);
                         b[5] = i ? (uint32_t)(uintptr_t)prev : 0;
                         b[6] = i + 1 < n ? (uint32_t)(uintptr_t)arena<void>(0x1000 + (i + 1) * 0x20) : 0;
                         if (!i) *(void **)arena<uint8_t>(0x120) = b;
                         if (!b[2] && (int)b[4] > largest) largest = b[4];
                         prev = b;
                     }
                     arena<int32_t>(0x100)[7] = r.range(0, 3) ? largest : largest + r.range(1, 50);
                     static const int aligns[] = {1, 2, 3, 4, 8, 16};
                     arena<int32_t>(0x20)[0] = r.range(0, 3) ? r.range(1, 40) : r.range(1, 300);
                     arena<int32_t>(0x20)[1] = aligns[r.range(0, 5)];
                 },
                 [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x20)[0], arena<int32_t>(0x20)[1])}; }};
    w1_take.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbea270, (void *)w1_stub_pool_alloc}, {0xbfde90, (void *)w1_stub_pool_alloc}}); };
    w1_take.finish = w1_unstub;
    quick(w1_take);
    // copies of one object to another
    auto w1_two = [=](Rng &r) { fresh(r); w1_fill(r, 0x100, 0x200); w1_fill(r, 0x400, 0x100); };
    auto w1_copy_call = [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x400))}; };
    Test w1_record_copy{0x436b40, w1_two, w1_copy_call};
    quick(w1_record_copy);
    Test w1_plain_copy{0x84dbf0, w1_two, w1_copy_call};
    quick(w1_plain_copy);
    Test w1_rows{0x988500, w1_two, [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x400)); return Result{}; }};
    quick(w1_rows);
    // VirtualBase::Assign: the object's table is either of this copy's or another; the other's table at 0x300
    Test w1_assign{0x5d3d40,
                   [=](Rng &r) {
                       w1_two(r);
                       uint32_t tables[] = {w1_a, w1_b, 0x1234};
                       arena<uint32_t>(0x100)[0] = tables[r.range(0, 2)];
                       arena<void *>(0x400)[0] = arena<void>(0x300);
                       arena<uint32_t>(0x300)[1] = (uint32_t)r.range(4, 0x20) * 4;
                   },
                   w1_copy_call};
    w1_assign.prepare = [](const rewrite::Entry *e) { w1_a = *(uint32_t *)(e->at + 0xb); w1_b = *(uint32_t *)(e->at + 0x2c); };
    quick(w1_assign);
    auto w1_arg_call = [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0])}; };
    auto w1_arg = [=](Rng &r) { w1_two(r); arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x20)[1] = r.next(); };
    auto w1_self_call = [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))}; };
    auto w1_void_call = [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; };
    Test w1_tagged{0x87b650, w1_arg, w1_arg_call};
    quick(w1_tagged);
    Test w1_cell{0xcc2cc0, w1_two, w1_self_call};
    quick(w1_cell);
    // Listed::Construct: the list's own node at 0x300, its next at 0x340; the global set for each call
    Test w1_listed{0x4535c0,
                   [=](Rng &r) {
                       w1_two(r); w1_fill(r, 0x300, 0x80);
                       arena<void *>(0x300)[0] = arena<void>(0x340);
                   },
                   [](void *fn) {
                       *(void **)(uintptr_t)w1_a = arena<void>(0x300);
                       return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))};
                   }};
    w1_listed.prepare = [](const rewrite::Entry *e) { w1_a = *(uint32_t *)(e->at + 0xd); };
    quick(w1_listed);
    // Shared::Current: self at 0x4000, a table for each copy's place, counted items at 0x380
    Test w1_current{0x685b70,
                    [=](Rng &r) {
                        fresh(r); w1_fill(r, 0x4000, 0x600); w1_fill(r, 0x380, 0x40);
                        for (int i = 0; i < 4; i++) {
                            arena<void *>(0x300)[i] = arena<void>(0x380 + i * 4);
                            arena<void *>(0x340)[i] = arena<void>(0x3a0 + i * 4);
                        }
                        *(void **)arena<uint8_t>(0x4578) = arena<void>(0x300);
                        *(void **)arena<uint8_t>(0x4580) = arena<void>(0x340);
                        *(int32_t *)arena<uint8_t>(0x4278) = r.range(0, 3);
                    },
                    [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x4000), arena<void>(0x20))}; }};
    quick(w1_current);
    Test w1_pair{0xa5ece0, w1_arg,
                 [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0], arena<uint32_t>(0x20)[1])}; }};
    quick(w1_pair);
    auto w1_number = [=](Rng &r) { fresh(r); arena<int32_t>(0x20)[0] = r.range(-2, 13); };
    auto w1_number_call = [](void *fn) { return Result{((uint32_t(__fastcall *)(int32_t))fn)(arena<int32_t>(0x20)[0])}; };
    Test w1_kind{0x5bcfb0, w1_number, w1_number_call};
    quick(w1_kind);
    Test w1_names8{0xbc4150, w1_number, w1_number_call};
    quick(w1_names8);
    Test w1_names11{0x59a080, w1_number, w1_number_call};
    quick(w1_names11);
    // Screen::Destroy: the global it clears is copied into the arena to be compared
    Test w1_screen{0x631fe0, w1_two,
                   [](void *fn) {
                       *(uint32_t *)(uintptr_t)w1_a = 7;
                       ((void(__fastcall *)(void *))fn)(arena<void>(0x100));
                       arena<uint32_t>(0x20)[0] = *(uint32_t *)(uintptr_t)w1_a;
                       return Result{};
                   }};
    w1_screen.prepare = [](const rewrite::Entry *e) { w1_a = *(uint32_t *)(e->at + 0x19); stubs_for_now({{0x669660, (void *)stub_plain_dtor}}); };
    w1_screen.finish = w1_unstub;
    quick(w1_screen);
    // Placed::SetPosition: attached object at 0x600 or none, its data at 0x700
    Test w1_position{0x579b90,
                     [=](Rng &r) {
                         w1_two(r); w1_fill(r, 0x600, 0x20); w1_fill(r, 0x700, 0x100);
                         arena<void *>(0x100)[1] = r.range(0, 1) ? arena<void>(0x600) : nullptr;
                         arena<void *>(0x600)[3] = arena<void>(0x700);
                     },
                     [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x400)); return Result{}; }};
    quick(w1_position);
    Test w1_state{0x79df20, w1_two, w1_void_call};
    w1_state.prepare = [](const rewrite::Entry *e) { *(uint32_t *)(uintptr_t)*(uint32_t *)(e->at + 0xb) = 0xabcd1234; };
    quick(w1_state);
    // Updated::SetActive: manager at 0x300 with each copy's list; self at 0x1000
    Test w1_active{0xbd31f0,
                   [=](Rng &r) {
                       fresh(r); w1_fill(r, 0x1000, 0x180); w1_fill(r, 0x300, 0x200);
                       *(void **)arena<uint8_t>(0x344) = arena<void>(0x400);
                       *(void **)arena<uint8_t>(0x35c) = arena<void>(0x440);
                       arena<void *>(0x400)[0] = arena<void>(0x480);
                       arena<void *>(0x440)[0] = arena<void>(0x4c0);
                       uint8_t on = (uint8_t)r.range(0, 1);
                       arena<uint8_t>(0x1098)[0] = on;
                       arena<uint8_t>(0x1118)[0] = on;
                       static const uint32_t args[] = {0, 1, 0x100, 0xff};
                       arena<uint32_t>(0x20)[0] = args[r.range(0, 3)];
                   },
                   [](void *fn) {
                       *(void **)(uintptr_t)0x13d0d84 = arena<void>(0x300);
                       ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x20)[0]);
                       return Result{};
                   }};
    w1_active.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbd2960, (void *)stub_thiscall1}, {0xbd29e0, (void *)stub_thiscall1}}); };
    w1_active.finish = w1_unstub;
    quick(w1_active);
    // ValueList::Clear: 0..4 nodes, some without a value
    Test w1_value_clear{0xb781b0,
                        [=](Rng &r) {
                            w1_two(r);
                            arena<void *>(0x100)[0] = w1_chain(0x300, r.range(0, 4), 0x700, r.next() & 0xf);
                        },
                        w1_void_call};
    quick(w1_value_clear);
    // the table at 0x700: odd and even slots go to different stubs, so a wrong slot shows
    auto w1_table = [] {
        for (int i = 0; i < 0x20; i++) arena<void *>(0x700)[i] = i & 1 ? (void *)stub_plain_dtor : (void *)w1_stub_other;
    };
    Test w1_global_each{0x452e40,
                        [=](Rng &r) { w1_two(r); w1_table(); w1_chain(0x300, r.range(0, 4), 0x700); },
                        [](void *fn) {
                            *(void **)(uintptr_t)w1_a = arena<void>(0x300);
                            ((void(__cdecl *)())fn)();
                            return Result{};
                        }};
    w1_global_each.prepare = [](const rewrite::Entry *e) { w1_a = *(uint32_t *)(e->at + 1); };
    quick(w1_global_each);
    Test w1_member_each{0x440e40,
                        [=](Rng &r) {
                            w1_two(r); w1_table();
                            *(void **)arena<uint8_t>(0x100 + w1_a) = w1_chain(0x300, r.range(0, 4), 0x700);
                        },
                        w1_void_call};
    w1_member_each.prepare = [](const rewrite::Entry *e) { w1_a = *(uint8_t *)(e->at + 3); };
    quick(w1_member_each);
    Test w1_owned_clear{0xbf53e0,
                        [=](Rng &r) {
                            w1_two(r);
                            arena<void *>(0x700)[0] = (void *)w1_stub_pop;
                            w1_pop_holder = 0x104;
                            arena<void *>(0x100)[1] = w1_chain(0x300, r.range(0, 4), 0x700);
                        },
                        w1_void_call};
    quick(w1_owned_clear);
    Test w1_owned_delete{0x4ccdb0,
                         [=](Rng &r) {
                             w1_two(r);
                             arena<void *>(0x700)[0] = (void *)w1_stub_pop;
                             w1_pop_holder = 0x100 + w1_a;
                             *(void **)arena<uint8_t>(0x100 + w1_a) = w1_chain(0x300, r.range(0, 4), 0x700, r.range(0, 1) ? 0 : r.next() & 0xf);
                         },
                         w1_void_call};
    w1_owned_delete.prepare = [](const rewrite::Entry *e) { w1_a = *(uint8_t *)(e->at + 5); };
    quick(w1_owned_delete);
    Test w1_level{0x4d6650,
                  [=](Rng &r) { w1_two(r); arena<uint8_t>(0x510)[1] = (uint8_t)r.range(0, 4); },
                  w1_self_call};
    w1_level.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbb3e70, (void *)w1_stub_level}}); };
    w1_level.finish = w1_unstub;
    quick(w1_level);
    Test w1_push_back{0x9f3dc0,
                      [=](Rng &r) {
                          w1_arg(r);
                          *arena<uint32_t>(ALLOC_NEXT) = 0;
                          w1_fill(r, 0x2000, 0x20); w1_fill(r, 0x300, 0x80);
                          *(void **)arena<uint8_t>(0x100 + w1_a) = arena<void>(0x300);
                          arena<void *>(0x300)[1] = arena<void>(0x340);
                      },
                      [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0]); return Result{}; }};
    w1_push_back.prepare = [](const rewrite::Entry *e) { w1_a = *(uint8_t *)(e->at + 0x1e); stubs_for_now({{0xbcdfc0, (void *)stub_alloc_next}}); };
    w1_push_back.finish = w1_unstub;
    quick(w1_push_back);
    // Device::SetCached: self at 0x4000 (its value may be far in), a table with the stub at this copy's slot
    Test w1_cached{0xbe8960,
                   [=](Rng &r) {
                       fresh(r); w1_fill(r, 0x4000, 0x2500);
                       *(void **)arena<uint8_t>(0x424c) = w1_object(0x300, 0x800);
                       memset(arena<void>(0x800), 0, 0x200);
                       *(void **)arena<uint8_t>(0x800 + w1_b) = (void *)w1_stub_std2;
                       arena<uint32_t>(0x20)[0] = r.range(0, 1) ? *(uint32_t *)arena<uint8_t>(0x4000 + w1_a) : r.next();
                   },
                   [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x4000), arena<uint32_t>(0x20)[0]); return Result{}; }};
    w1_cached.prepare = [](const rewrite::Entry *e) { w1_a = *(uint32_t *)(e->at + 5); w1_b = *(uint32_t *)(e->at + 0x1e); };
    quick(w1_cached);
    Test w1_arrays_free{0x8e7200, w1_two, w1_void_call};
    quick(w1_arrays_free);
    Test w1_forward{0xcc2090, [=](Rng &r) { w1_two(r); arena<void *>(0x100)[0] = arena<void>(0x300 + r.range(0, 8) * 4); }, w1_self_call};
    w1_forward.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xcc2250, (void *)w1_stub_value}, {0xcc4280, (void *)w1_stub_value}}); };
    w1_forward.finish = w1_unstub;
    quick(w1_forward);
    // Text::FromRange: source -> text at 0x600
    Test w1_from_range{0x403bb0,
                       [=](Rng &r) { w1_two(r); w1_fill(r, 0x600, 0x20); arena<void *>(0x300)[0] = arena<void>(0x600); },
                       [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x400), arena<void>(0x300))}; }};
    w1_from_range.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbd0640, (void *)w1_stub_this3}, {0xbd0ce0, (void *)w1_stub_this3}, {0xbd0720, (void *)w1_stub_this3}}); };
    w1_from_range.finish = w1_unstub;
    quick(w1_from_range);
    Test w1_query{0xbc5420, w1_arg,
                  [](void *fn) {
                      *(uint32_t *)(uintptr_t)w1_a = arena<uint32_t>(0x20)[0];
                      *(uint32_t *)(uintptr_t)w1_b = arena<uint32_t>(0x20)[1];
                      return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))};
                  }};
    w1_query.prepare = [](const rewrite::Entry *e) { w1_a = *(uint32_t *)(e->at + 8); w1_b = *(uint32_t *)(e->at + 0xe); stubs_for_now({{0xbc4ff0, (void *)w1_stub_this4}}); };
    w1_query.finish = w1_unstub;
    quick(w1_query);
    Test w1_ref{0x82a390,
                [=](Rng &r) {
                    w1_two(r); w1_fill(r, 0x600, 0x80);
                    g_null_handle = arena<void>(0x600);
                    *(void **)arena<uint8_t>(0x620) = arena<void>(0x680);
                    arena<void *>(0x680)[0] = arena<void>(0x6c0);
                },
                w1_self_call};
    quick(w1_ref);
    // Array::Delete: nothing, or items at 0x304 with their count before them
    Test w1_array_delete{0x8f8910,
                         [=](Rng &r) {
                             w1_two(r);
                             arena<void *>(0x100)[0] = r.range(0, 3) ? w1_object(0x304, 0x700) : nullptr;
                             arena<uint32_t>(0x300)[0] = r.range(0, 1) * 3;
                             arena<void *>(0x700)[1] = (void *)stub_thiscall1;
                         },
                         w1_void_call};
    quick(w1_array_delete);
    Test w1_pair_delete{0x86e5c0,
                        [=](Rng &r) {
                            w1_two(r);
                            arena<void *>(0x100)[0] = r.range(0, 1) ? w1_object(0x300, 0x700) : nullptr;
                            arena<void *>(0x100)[1] = r.range(0, 1) ? w1_object(0x340, 0x700) : nullptr;
                            arena<void *>(0x700)[0] = (void *)stub_thiscall1;
                        },
                        w1_void_call};
    quick(w1_pair_delete);
    Test w1_scene{0x63d280,
                  [=](Rng &r) {
                      w1_arg(r);
                      arena<void *>(0x100)[8] = r.range(0, 2) ? w1_object(0x300, 0x700) : nullptr;
                      arena<void *>(0x700)[0] = (void *)stub_thiscall1;
                  },
                  [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0]); return Result{}; }};
    w1_scene.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6a1e20, (void *)stub_plain_dtor}}); };
    w1_scene.finish = w1_unstub;
    quick(w1_scene);
    Test w1_pointers{0x50cb50,
                     [=](Rng &r) { w1_two(r); *arena<uint32_t>(ALLOC_NEXT) = 0; arena<uint32_t>(0x20)[0] = r.range(0, 9); },
                     w1_arg_call};
    quick(w1_pointers);
    Test w1_found{0x4e5f00,
                  [=](Rng &r) { w1_arg(r); w1_fill(r, 0x500, 0x100); },
                  [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[1]); return Result{}; }};
    w1_found.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x4d1840, (void *)w1_stub_maybe}}); };
    w1_found.finish = w1_unstub;
    quick(w1_found);
    Test w1_shown{0xb19d40, w1_two, w1_void_call};
    w1_shown.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xb5f790, (void *)stub_plain_dtor}}); };
    w1_shown.finish = w1_unstub;
    quick(w1_shown);
    Test w1_sized{0x458130, w1_two, w1_self_call};
    w1_sized.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x458010, (void *)w1_stub_fast2}}); };
    w1_sized.finish = w1_unstub;
    quick(w1_sized);
    Test w1_com{0xc5be10,
                [=](Rng &r) {
                    w1_two(r);
                    arena<void *>(0x700)[2] = (void *)w1_stub_std1;
                    *(void **)arena<uint8_t>(0x124) = r.range(0, 2) ? w1_object(0x300, 0x700) : nullptr;
                    *(void **)arena<uint8_t>(0x128) = r.range(0, 2) ? w1_object(0x340, 0x700) : nullptr;
                },
                w1_void_call};
    quick(w1_com);
    // the kept interface of a reference at self + ref: its handle at 0x340, sometimes already looked up
    static auto w1_ref_setup = [](Rng &r, uint8_t *ref) {
        *(void **)(ref + 4) = arena<void>(0x340);
        ref[0x18] = (uint8_t)(r.range(0, 3) == 0);
        *(void **)(ref + 0x14) = r.range(0, 1) ? arena<void>(0x900) : nullptr;
    };
    Test w1_part{0x4abd00,
                 [=](Rng &r) {
                     fresh(r); w1_fill(r, 0x800, 0x400);
                     iface_handle(r);
                     w1_ref_setup(r, arena<uint8_t>(0x800 + w1_a));
                     w1_object(0x600, 0x700);
                     arena<void *>(0x700)[0] = (void *)stub_thiscall2;
                 },
                 [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0x600)); return Result{}; }};
    w1_part.prepare = [](const rewrite::Entry *e) {
        w1_a = *(uint32_t *)(e->at + 0x1b) - 4;
        g_iface_id = (uint32_t *)0x130bbec;
        stubs_for_now({{0x534e60, (void *)stub_thiscall1}, {0x4bee90, (void *)stub_thiscall1}});
    };
    w1_part.finish = w1_unstub;
    quick(w1_part);
    Test w1_enabled{0x512080,
                    [=](Rng &r) {
                        fresh(r); w1_fill(r, 0x800, 0x200);
                        iface_handle(r);
                        w1_ref_setup(r, arena<uint8_t>(0x800 + w1_a));
                        arena<uint8_t>(0x8c0)[0] = (uint8_t)r.range(0, 3);
                    },
                    [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w1_enabled.prepare = [](const rewrite::Entry *e) { w1_a = *(uint32_t *)(e->at + 0x19) - 4; g_iface_id = (uint32_t *)0x12f5608; };
    quick(w1_enabled);
    Test w1_option{0x4f6770,
                   [=](Rng &r) {
                       fresh(r); w1_fill(r, 0x800, 0x700);
                       *(int32_t *)arena<uint8_t>(0x944) = r.range(0, 5);
                       for (int i = 0; i < 4; i++) if (r.range(0, 1)) arena<uint32_t>(0xdf0)[i] = 0;
                   },
                   [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1_option.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x4f1740, (void *)stub_thiscall1}, {0x4f5530, (void *)w1_stub_this4}}); };
    w1_option.finish = w1_unstub;
    quick(w1_option);
    Test w1_tables{0xbe0690,
                   [=](Rng &r) {
                       w1_two(r);
                       arena<uint32_t>(0x300)[0] = (uint32_t)r.range(0, 5);
                       arena<void *>(0x100)[4] = r.range(0, 1) ? arena<void>(0x304) : nullptr;
                   },
                   w1_void_call};
    quick(w1_tables);
    Test w1_pair_destroy{0x86e610,
                         [=](Rng &r) {
                             w1_arg(r);
                             arena<void *>(0x100)[0] = r.range(0, 1) ? w1_object(0x300, 0x700) : nullptr;
                             arena<void *>(0x100)[1] = r.range(0, 1) ? w1_object(0x340, 0x700) : nullptr;
                             arena<void *>(0x700)[0] = (void *)stub_thiscall1;
                         },
                         w1_arg_call};
    quick(w1_pair_destroy);
    Test w1_pairs_add{0xcb2b40,
                      [=](Rng &r) {
                          w1_two(r); w1_fill(r, 0x600, 0x80);
                          arena<uint32_t>(0x100)[0] = (uint32_t)r.range(0, 8);
                          arena<void *>(0x100)[3] = arena<void>(0x600);
                      },
                      [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x400)); return Result{}; }};
    w1_pairs_add.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xcb29a0, (void *)stub_reserve}, {0xcbcf50, (void *)stub_reserve}}); };
    w1_pairs_add.finish = w1_unstub;
    quick(w1_pairs_add);
    Test w1_wrapped{0x49d580, w1_arg, w1_arg_call};
    w1_wrapped.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x49cd90, (void *)stub_thiscall1}, {0x49d5a0, (void *)stub_thiscall1}}); };
    w1_wrapped.finish = w1_unstub;
    quick(w1_wrapped);
    Test w1_ref_with{0x9d7050,
                     [=](Rng &r) {
                         w1_two(r); w1_fill(r, 0x600, 0x80);
                         g_null_handle = arena<void>(0x600);
                         *(void **)arena<uint8_t>(0x620) = arena<void>(0x680);
                         arena<void *>(0x680)[0] = arena<void>(0x6c0);
                     },
                     w1_self_call};
    quick(w1_ref_with);
    Test w1_items{0x66a810,
                  [=](Rng &r) {
                      fresh(r); w1_fill(r, 0x800, 0x300); w1_fill(r, 0x600, 0x80);
                      *(void **)arena<uint8_t>(0x800 + w1_a) = arena<void>(0x600);
                      *(uint32_t *)arena<uint8_t>(0x804 + w1_a) = (uint32_t)r.range(0, 8);
                      arena<uint32_t>(0x20)[0] = r.next();
                  },
                  [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[0]); return Result{}; }};
    w1_items.prepare = [](const rewrite::Entry *e) {
        w1_a = *(uint32_t *)(e->at + 0xd);
        stubs_for_now({{0x66a4c0, (void *)w1_stub_grow}, {0x6719e0, (void *)w1_stub_grow}, {0x9ce530, (void *)w1_stub_grow}});
    };
    w1_items.finish = w1_unstub;
    quick(w1_items);
    Test w1_buffer{0xc0acf0,
                   [=](Rng &r) {
                       w1_two(r);
                       arena<uint32_t>(0x100)[5] = (uint32_t)r.range(0, 1);
                       arena<uint32_t>(0x20)[0] = (uint32_t)r.range(0, 3);
                   },
                   [](void *fn) {
                       uint32_t pick = arena<uint32_t>(0x20)[0];
                       *(void **)(uintptr_t)w1_a = pick == 1 ? arena<void>(0x100) : arena<void>(0x200);
                       ((void(__fastcall *)(void *))fn)(pick ? arena<void>(0x100) : nullptr);
                       arena<uint32_t>(0x20)[1] = *(uint32_t *)(uintptr_t)w1_a;
                       return Result{};
                   }};
    w1_buffer.prepare = [](const rewrite::Entry *e) { w1_a = *(uint32_t *)(e->at + 9); stubs_for_now({{0xdddc77, (void *)w1_stub_crt_free}}); };
    w1_buffer.finish = w1_unstub;
    quick(w1_buffer);

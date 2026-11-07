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
    // Chain::Broadcast: the chain starts at the interface found (0x360, 0x368) or kept (0x900), linked at +4, listeners at +0x40
    Test w1_broadcast{0x59bb40,
                      [=](Rng &r) {
                          fresh(r); w1_fill(r, 0x800, 0x100);
                          iface_handle(r);
                          w1_ref_setup(r, arena<uint8_t>(0x82c));
                          for (int i = 0; i < 3; i++) {
                              w1_object(0x1040 + i * 0x80, 0x700);
                              arena<void *>(0x1000 + i * 0x80)[1] = i < 2 && r.range(0, 2) ? arena<void>(0x1000 + (i + 1) * 0x80) : nullptr;
                          }
                          w1_object(0x3a0, 0x700); w1_object(0x3a8, 0x700); w1_object(0x940, 0x700);
                          arena<void *>(0x364)[0] = arena<void>(0x1000);
                          arena<void *>(0x36c)[0] = nullptr;
                          arena<void *>(0x904)[0] = arena<void>(0x1080);
                          for (int i = 0; i < 4; i++) arena<void *>(0x700)[i] = i & 1 ? (void *)stub_thiscall2 : (void *)w1_stub_two_b;
                          arena<uint32_t>(0x20)[0] = r.range(0, 1) ? r.next() : 0;
                          arena<uint32_t>(0x20)[1] = r.next();
                      },
                      [](void *fn) { ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[0], arena<uint32_t>(0x20)[1]); return Result{}; }};
    w1_broadcast.prepare = [](const rewrite::Entry *) { g_iface_id = (uint32_t *)0x12f5638; stubs_for_now({{0x5d49d0, (void *)w1_stub_next}}); };
    w1_broadcast.finish = w1_unstub;
    quick(w1_broadcast);
    Test w1_grid{0x730720,
                 [=](Rng &r) {
                     fresh(r); w1_fill(r, 0x800, 0x900);
                     iface_handle(r);
                     arena<int32_t>(0x20)[0] = r.range(0, 1);
                     arena<int32_t>(0x20)[1] = r.range(0, 3);
                     for (int i = 0; i < 10; i++) *(void **)arena<uint8_t>(0x800 + w1_a + i * 20 + 4) = arena<void>(0x3c0);   // the others have no object
                     *(void **)arena<uint8_t>(0x3cc) = nullptr;
                     uint32_t at = 0x800 + w1_a + (arena<int32_t>(0x20)[1] + arena<int32_t>(0x20)[0] * 4) * 20;
                     *(void **)arena<uint8_t>(at + 4) = arena<void>(0x340);
                 },
                 [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t, int32_t))fn)(arena<void>(0x800), arena<int32_t>(0x20)[0], arena<int32_t>(0x20)[1])}; }};
    w1_grid.prepare = [](const rewrite::Entry *e) { w1_a = *(uint32_t *)(e->at + 0x11); g_iface_id = (uint32_t *)0x1310f90; };
    quick(w1_grid);
    // sounds: each at 0x2000 + i*0x40 with an owner at 0x600, for either layout
    static auto w1_sound = [](Rng &r, uint32_t off) {
        w1_fill(r, off, 0x40);
        *(void **)arena<uint8_t>(off + 0x30) = arena<void>(0x600);
        *(void **)arena<uint8_t>(off + 0x28) = arena<void>(0x600);
    };
    auto w1_sound_list = [=](Rng &r) {
        w1_two(r); w1_fill(r, 0x600, 0x500); w1_fill(r, 0xb00, 0x40);
        arena<uint32_t>(0x24)[0] = r.next();
        int n = r.range(0, 4);
        arena<void *>(0x100)[1] = w1_chain(0x300, n, 0x700);
        for (int i = 0; i < n; i++) {
            w1_sound(r, 0x2000 + i * 0x40);
            arena<void *>(0x300 + i * 0x10)[2] = arena<void>(0x2000 + i * 0x40);
        }
    };
    auto w1_sound_one = [=](Rng &r) {
        fresh(r); w1_fill(r, 0x600, 0x500);
        arena<uint32_t>(0x24)[0] = r.next();
        w1_sound(r, 0x100);
    };
    auto w1_sound_stubs = [](const rewrite::Entry *) {
        stubs_for_now({{0xc5c7f0, (void *)w1_stub_load}, {0xc5c990, (void *)w1_stub_load}, {0xc5cb90, (void *)stub_thiscall1}, {0xc5cc00, (void *)stub_thiscall1}});
    };
    Test w1_sound_load{0xbfa560, w1_sound_one, w1_void_call};
    w1_sound_load.prepare = w1_sound_stubs; w1_sound_load.finish = w1_unstub;
    quick(w1_sound_load);
    Test w1_sound_all{0xbfa8e0, w1_sound_list, w1_void_call};
    w1_sound_all.prepare = w1_sound_stubs; w1_sound_all.finish = w1_unstub;
    quick(w1_sound_all);
    Test w1_sound_unload{0xbfa8a0, w1_sound_list, w1_void_call};
    w1_sound_unload.prepare = w1_sound_stubs; w1_sound_unload.finish = w1_unstub;
    quick(w1_sound_unload);
    Test w1_sound_length{0xbf9d70,
                         [=](Rng &r) {
                             w1_sound_one(r);
                             *(void **)arena<uint8_t>(0x100 + w1_a) = r.range(0, 1) ? arena<void>(0x900) : nullptr;
                         },
                         w1_self_call};
    w1_sound_length.prepare = [=](const rewrite::Entry *e) { w1_a = *(uint8_t *)(e->at + 5); w1_sound_stubs(e); };
    w1_sound_length.finish = w1_unstub;
    quick(w1_sound_length);
    Test w1_pass_on{0xb5b1d0,
                    [=](Rng &r) {
                        fresh(r); w1_fill(r, 0x800, 0x200);
                        *(void **)arena<uint8_t>(0x920) = r.range(0, 3) ? w1_object(0x300, 0xa00) : nullptr;
                        arena<void *>(0xa00)[0x44] = (void *)w1_stub_willing;
                        arena<void *>(0xa00)[0x14] = (void *)stub_thiscall1;
                        arena<void *>(0xa00)[0x17] = (void *)w1_stub_one_b;
                        arena<uint32_t>(0x20)[0] = r.next();
                        arena<uint32_t>(0x24)[0] = r.next();
                    },
                    [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[0]); return Result{}; }};
    quick(w1_pass_on);
    Test w1_twice{0xba2ff0,
                  [=](Rng &r) { w1_two(r); for (int i = 0; i < 4; i++) arena<uint32_t>(0x20)[i] = r.next(); },
                  [](void *fn) {
                      uint32_t *a = arena<uint32_t>(0x20);
                      ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3]);
                      return Result{};
                  }};
    w1_twice.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xb9dae0, (void *)w1_stub_five}, {0xb9de50, (void *)w1_stub_four}, {0xb9dee0, (void *)w1_stub_four}}); };
    w1_twice.finish = w1_unstub;
    quick(w1_twice);
    Test w1_ready{0x87f210,
                  [=](Rng &r) {
                      fresh(r); w1_fill(r, 0x800, 0x100); w1_fill(r, 0x600, 0x80);
                      *(void **)arena<uint8_t>(0x88c) = arena<void>(0x600);
                      arena<uint32_t>(0x24)[0] = r.next();
                  },
                  [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w1_ready.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbd1420, (void *)w1_stub_ask}}); };
    w1_ready.finish = w1_unstub;
    quick(w1_ready);
    Test w1_held{0x4085f0,
                 [=](Rng &r) {
                     w1_two(r); w1_fill(r, 0x300, 0x20);
                     arena<void *>(0x100)[0] = r.range(0, 3) ? arena<void>(0x300) : nullptr;
                 },
                 w1_void_call};
    w1_held.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xcc2250, (void *)w1_stub_value}, {0xbe4f30, (void *)w1_stub_value}}); };
    w1_held.finish = w1_unstub;
    quick(w1_held);
    // Second pass.
    auto w1b_flagged_call = [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x20)[0])}; };
    Test w1b_com{0xc5c5b0,
                 [=](Rng &r) {
                     w1_two(r);
                     arena<uint32_t>(0x20)[0] = r.next();
                     *(void **)arena<uint8_t>(0x100 + w1b_at1) = r.range(0, 2) ? w1_object(0x300, 0xa00) : nullptr;
                     arena<void *>(0xa00)[2] = (void *)w1_stub_std1;
                 },
                 w1b_flagged_call};
    w1b_com.prepare = [](const rewrite::Entry *e) { w1b_at1 = *(uint8_t *)(e->at + 5); };
    quick(w1b_com);
    Test w1b_pair{0x6412a0,
                  [=](Rng &r) { w1_two(r); for (int i = 0; i < 4; i++) arena<uint32_t>(0x20)[i] = r.next(); },
                  [](void *fn) {
                      uint32_t *a = arena<uint32_t>(0x20);
                      return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3])};
                  }};
    w1b_pair.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x488290, (void *)w1b_stub_pair_base}}); };
    w1b_pair.finish = w1_unstub;
    quick(w1b_pair);
    Test w1b_twin{0xbdbe10, w1_two,
                  [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x400))}; }};
    w1b_twin.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbd9b20, (void *)stub_thiscall1}}); };
    w1b_twin.finish = w1_unstub;
    quick(w1b_twin);
    Test w1b_each_test{0xad1350,
                       [=](Rng &r) {
                           w1_two(r);
                           arena<void *>(0xa00)[0xe4 / 4] = (void *)stub_plain_dtor;
                           int n = r.range(0, 4);
                           void *end = arena<void>(0x100 + w1b_at2);
                           *(void **)arena<uint8_t>(0x100 + w1b_at1) = n ? arena<void>(0x600) : end;
                           for (int i = 0; i < n; i++) {
                               uint32_t *node = arena<uint32_t>(0x600 + i * 0x10);
                               node[0] = (uint32_t)(uintptr_t)(i + 1 < n ? arena<void>(0x600 + (i + 1) * 0x10) : end);
                               node[2] = (uint32_t)(uintptr_t)w1_object(0x1800 + i * 0x10, 0xa00);
                           }
                       },
                       w1_void_call};
    w1b_each_test.prepare = [](const rewrite::Entry *e) { w1b_at1 = *(uint32_t *)(e->at + 4); w1b_at2 = *(uint32_t *)(e->at + 0xb); };
    quick(w1b_each_test);
    Test w1b_slot{0x557c20,
                  [=](Rng &r) {
                      w1_two(r);
                      int32_t values[] = {-1, -2, 0, 1, 7, (int32_t)0x80000000};
                      *(int32_t *)arena<uint8_t>(0x100 + w1b_at1) = values[r.range(0, 5)];
                  },
                  [](void *fn) { *(void **)(uintptr_t)w1b_at2 = arena<void>(0x500); ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w1b_slot.prepare = [](const rewrite::Entry *e) {
        w1b_at1 = *(uint32_t *)(e->at + 5); w1b_at2 = *(uint32_t *)(e->at + 0xf);
        stubs_for_now({{0x4cde30, (void *)stub_thiscall1}, {0xa9ef40, (void *)stub_thiscall1}});
    };
    w1b_slot.finish = w1_unstub;
    quick(w1b_slot);
    Test w1b_unreg{0xbd37e0,
                   [=](Rng &r) { w1_two(r); arena<uint8_t>(0x100)[w1b_at1] = r.range(0, 1) ? (uint8_t)(r.next() | 1) : 0; },
                   [](void *fn) { *(void **)0x13d0d84 = arena<void>(0x500); ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w1b_unreg.prepare = [](const rewrite::Entry *e) {
        w1b_at1 = *(uint32_t *)(e->at + 2);
        stubs_for_now({{0xbd2960, (void *)stub_thiscall1}, {0xbd29e0, (void *)stub_thiscall1}});
    };
    w1b_unreg.finish = w1_unstub;
    quick(w1b_unreg);
    Test w1b_attach_test{0xcb0bb0,
                         [=](Rng &r) { w1_two(r); arena<uint32_t>(0x24)[0] = r.next(); },
                         [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x500))}; }};
    w1b_attach_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xc51360, (void *)w1b_stub_entry}, {0xc53340, (void *)w1b_stub_add}}); };
    w1b_attach_test.finish = w1_unstub;
    quick(w1b_attach_test);
    Test w1b_named{0xb70080, [=](Rng &r) { fresh(r); },
                   [](void *fn) { return Result{((uint32_t(__cdecl *)())fn)()}; }};
    w1b_named.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x41cb90, (void *)w1b_stub_ref}, {0xbd1880, (void *)w1b_stub_ref_end}}); };
    w1b_named.finish = w1_unstub;
    quick(w1b_named);
    Test w1b_buffers{0x891760, [=](Rng &r) { w1_two(r); arena<uint32_t>(0x20)[0] = r.next(); }, w1b_flagged_call};
    quick(w1b_buffers);
    Test w1b_whole_test{0xba2fb0, w1_two, w1_void_call};
    w1b_whole_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xb9dae0, (void *)w1_stub_five}, {0xb9de50, (void *)w1_stub_four}, {0xb9dee0, (void *)w1_stub_four}}); };
    w1b_whole_test.finish = w1_unstub;
    quick(w1b_whole_test);
    Test w1b_array{0x421120,
                   [=](Rng &r) {
                       w1_two(r); w1_fill(r, 0x600, 0x40);
                       *(void **)arena<uint8_t>(0x100 + w1b_at1) = r.range(0, 2) ? arena<void>(0x604) : nullptr;
                   },
                   w1_void_call};
    w1b_array.prepare = [](const rewrite::Entry *e) { w1b_at1 = *(uint8_t *)(e->at + 5); stubs_for_now({{0xddcf39, (void *)w1b_stub_vector_dtor}}); };
    w1b_array.finish = w1_unstub;
    quick(w1b_array);
    Test w1b_finish_test{0x65ea60,
                         [=](Rng &r) { w1_two(r); *(int32_t *)arena<uint8_t>(0x100 + w1b_at1) = (int32_t)r.range(0, 7) - 2; },
                         w1_void_call};
    w1b_finish_test.prepare = [](const rewrite::Entry *e) {
        w1b_at1 = *(uint32_t *)(e->at + 0xa);
        stubs_for_now({{0x5f7710, (void *)w1_stub_other}, {0x8e58f0, (void *)w1_stub_other}, {0x42df90, (void *)w1b_stub_manager}, {0x6595d0, (void *)stub_thiscall1}});
    };
    w1b_finish_test.finish = w1_unstub;
    quick(w1b_finish_test);
    Test w1b_past_test{0x4a1800,
                       [=](Rng &r) {
                           w1_two(r);
                           *(void **)arena<uint8_t>(0x100 + w1b_at1) = r.range(0, 3) ? arena<void>(0x600) : nullptr;
                           *arena<int32_t>(0x600) = r.range(0, 1) ? -1 : (int32_t)r.next();
                       },
                       w1_self_call};
    w1b_past_test.prepare = [](const rewrite::Entry *e) { w1b_at1 = *(uint32_t *)(e->at + 3); };
    quick(w1b_past_test);
    Test w1b_sync_test{0x62e790, [=](Rng &r) { w1_two(r); arena<uint32_t>(0x24)[0] = r.next(); }, w1_void_call};
    w1b_sync_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x40a090, (void *)w1_stub_level}, {0x8d30d0, (void *)w1_stub_willing}, {0x67a580, (void *)stub_thiscall1}});
    };
    w1b_sync_test.finish = w1_unstub;
    quick(w1b_sync_test);
    Test w1b_drop_test{0xbfa530,
                       [=](Rng &r) { w1_two(r); *(void **)arena<uint8_t>(0x100 + w1b_at1) = arena<void>(0x400); },
                       w1_void_call};
    w1b_drop_test.prepare = [](const rewrite::Entry *e) {
        w1b_at1 = *(uint8_t *)(e->at + 8);
        stubs_for_now({{0xc5cb90, (void *)stub_thiscall1}, {0xc5cc00, (void *)stub_thiscall1}});
    };
    w1b_drop_test.finish = w1_unstub;
    quick(w1b_drop_test);
    Test w1b_six{0x85a5e0, w1_two, w1_void_call};
    quick(w1b_six);
    Test w1b_parts{0x81f0b0,
                   [=](Rng &r) {
                       fresh(r); w1_fill(r, 0x800, 0x400);
                       iface_handle(r);
                       *(void **)arena<uint8_t>(0x800 + w1b_at1) = arena<void>(0xa00);
                       for (int i = 0; i < 4; i++) w1_ref_setup(r, arena<uint8_t>(0xa00 + i * 0x1c));
                       arena<uint32_t>(0x20)[0] = r.range(0, 3);
                   },
                   [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[0])}; }};
    w1b_parts.prepare = [](const rewrite::Entry *e) { w1b_at1 = *(uint8_t *)(e->at + 2); g_iface_id = *(uint32_t **)(e->at + 0x2d); };
    quick(w1b_parts);
    Test w1b_delete{0x419000,
                    [=](Rng &r) {
                        w1_two(r);
                        int n = r.range(0, 4);
                        void *head = arena<void>(0x100 + w1b_at1);
                        *(void **)head = arena<void>(0x600);
                        for (int i = 0; i <= n; i++) {
                            uint32_t *node = arena<uint32_t>(0x600 + i * 0x10);
                            node[0] = i < n ? (uint32_t)(uintptr_t)arena<void>(0x600 + (i + 1) * 0x10) : 0;
                            node[1] = (uint32_t)(uintptr_t)(i ? arena<void>(0x600 + (i - 1) * 0x10) : head);
                            node[2] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x1800 + i * 0x10) : 0;
                        }
                    },
                    w1_void_call};
    w1b_delete.prepare = [](const rewrite::Entry *e) { w1b_at1 = *(uint8_t *)(e->at + 5); stubs_for_now({{0x4188f0, (void *)stub_plain_dtor}, {0x827660, (void *)stub_plain_dtor}}); };
    w1b_delete.finish = w1_unstub;
    quick(w1b_delete);
    Test w1b_list_copy_test{0xc106b0,
                            [=](Rng &r) {
                                w1_two(r); w1_fill(r, 0x2000, 0x400);
                                *(void **)arena<uint8_t>(0x404) = w1_chain(0x600, r.range(0, 3), 0xa00);
                            },
                            [](void *fn) { w1b_clones = 0; return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x400))}; }};
    w1b_list_copy_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xc0e4c0, (void *)w1b_stub_clone}, {0xc10ca0, (void *)w1b_stub_clone}}); };
    w1b_list_copy_test.finish = w1_unstub;
    quick(w1b_list_copy_test);
    Test w1b_clear{0x5dfb60,
                   [=](Rng &r) {
                       w1_two(r);
                       *(void **)arena<uint8_t>(0x100 + w1b_at1) = arena<void>(0x600);
                       *(int32_t *)arena<uint8_t>(0x100 + w1b_at1 + 4) = (int32_t)r.range(0, 5) - 1;
                   },
                   w1_void_call};
    w1b_clear.prepare = [](const rewrite::Entry *e) {
        w1b_at1 = *(uint32_t *)(e->at + 0x15);
        stubs_for_now({{0x5aaa50, (void *)stub_plain_dtor}, {0x4ad740, (void *)stub_plain_dtor}, {0x5aec30, (void *)stub_thiscall1}, {0xb8cae0, (void *)stub_thiscall1}});
    };
    w1b_clear.finish = w1_unstub;
    quick(w1b_clear);
    Test w1b_preset_test{0x4796b0,
                         [=](Rng &r) { fresh(r); w1_fill(r, 0x30, 0x20); arena<uint32_t>(0x20)[0] = r.range(0, 3); },
                         [](void *fn) {
                             uint32_t index = arena<uint32_t>(0x20)[0];
                             memcpy((uint8_t *)(uintptr_t)w1b_at1 + index * 20, arena<void>(0x30), 20);
                             ((void(__fastcall *)(uint32_t, void *))fn)(index, arena<void>(0x500));
                             return Result{};
                         }};
    w1b_preset_test.prepare = [](const rewrite::Entry *e) {
        w1b_at1 = *(uint32_t *)(e->at + 0x16);
        stubs_for_now({{0x6a8350, (void *)w1_stub_this3}, {0x6a82d0, (void *)w1_stub_this3}, {0x6a8380, (void *)stub_thiscall1}, {0x6a8300, (void *)stub_thiscall1}});
    };
    w1b_preset_test.finish = w1_unstub;
    quick(w1b_preset_test);
    Test w1b_spot_test{0x7089e0, [=](Rng &r) { w1_two(r); w1_fill(r, 0x500, 0x200); }, w1_void_call};
    w1b_spot_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x833850, (void *)w1_stub_level}, {0x40a010, (void *)w1b_stub_manager}, {0x71b370, (void *)w1b_stub_spot}});
    };
    w1b_spot_test.finish = w1_unstub;
    quick(w1b_spot_test);
    Test w1b_chain{0x5eddf0, w1_two, w1_void_call};
    w1b_chain.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x5ebeb0, (void *)w1_stub_value}, {0x5ec2c0, (void *)w1_stub_level}, {0x5f1a50, (void *)stub_thiscall1}, {0x5f1a80, (void *)stub_thiscall1}});
    };
    w1b_chain.finish = w1_unstub;
    quick(w1b_chain);
    Test w1b_tell{0x5329c0,
                  [=](Rng &r) {
                      fresh(r); w1_fill(r, 0x800, 0x400);
                      iface_handle(r);
                      w1_ref_setup(r, arena<uint8_t>(0x800 + w1b_at1));
                  },
                  [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1b_tell.prepare = [](const rewrite::Entry *e) {
        w1b_at1 = *(uint32_t *)(e->at + 0xf) - 4; g_iface_id = *(uint32_t **)(e->at + 0x28);
        stubs_for_now({{0x55b5f0, (void *)stub_thiscall1}, {0x5e02c0, (void *)stub_thiscall1}});
    };
    w1b_tell.finish = w1_unstub;
    quick(w1b_tell);
    Test w1b_seven{0x7854e0,
                   [=](Rng &r) {
                       fresh(r); w1_fill(r, 0x800, 0x400);
                       iface_handle(r);
                       *(void **)arena<uint8_t>(0x3a0 + 0xc) = nullptr;
                       for (int i = 0; i < 7; i++) *(void **)arena<uint8_t>(0x860 + i * 0x14) = r.range(0, 2) ? arena<void>(0x3a0) : arena<void>(0x340);
                       arena<void *>(0x400)[0] = arena<void>(0x600);
                       arena<void *>(0x408)[0] = arena<void>(0x600);
                       *arena<uint32_t>(0x628) = r.range(0, 3) ? r.next() : 0;
                       arena<uint32_t>(0x24)[0] = r.next();
                   },
                   [](void *fn) { return Result{(uint8_t)((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w1b_seven.prepare = [](const rewrite::Entry *) { g_iface_id = (uint32_t *)0x1342768; stubs_for_now({{0x784320, (void *)w1_stub_ask}}); };
    w1b_seven.finish = w1_unstub;
    quick(w1b_seven);
    Test w1b_walk{0x59abe0,
                  [=](Rng &r) {
                      fresh(r); w1_fill(r, 0x800, 0x400);
                      iface_handle(r);
                      w1_ref_setup(r, arena<uint8_t>(0x86c));
                      *(void **)arena<uint8_t>(0x364) = arena<void>(0x380);
                      *(void **)arena<uint8_t>(0x36c) = nullptr;
                      *(void **)arena<uint8_t>(0x904) = r.range(0, 1) ? arena<void>(0x380) : nullptr;
                      *(void **)arena<uint8_t>(0x384) = nullptr;
                      arena<uint32_t>(0x20)[0] = r.next();
                  },
                  [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[0]); return Result{}; }};
    w1b_walk.prepare = [](const rewrite::Entry *) {
        g_iface_id = (uint32_t *)0x12f5638;
        stubs_for_now({{0x5d9cf0, (void *)w1_stub_one_b}, {0x5e40d0, (void *)w1_stub_one_b}, {0x5d9af0, (void *)w1_stub_one_b}, {0x5d49d0, (void *)w1_stub_next}, {0x5d4970, (void *)w1_stub_next}});
    };
    w1b_walk.finish = w1_unstub;
    quick(w1b_walk);
    Test w1b_take_test{0x4b7090,
                       [=](Rng &r) { fresh(r); w1_fill(r, 0x800, 0x100); iface_handle(r); },
                       [](void *fn) {
                           void *object = *(void **)arena<uint8_t>(0x34c);
                           return Result{(uint8_t)((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x80c), object)};
                       }};
    w1b_take_test.prepare = [](const rewrite::Entry *e) {
        g_iface_id = *(uint32_t **)(e->at + 0x1f);
        stubs_for_now({{0x501f80, (void *)stub_thiscall1}, {0x4b6d10, (void *)w1_stub_one_b}, {0xa64760, (void *)w1_stub_one_b}});
    };
    w1b_take_test.finish = w1_unstub;
    quick(w1b_take_test);
    // Third pass.
    // Two registries sharing a table: 0x600 passed in, or 0x620 in the class's global.
    auto w1c_registries = [=](Rng &r) {
        fresh(r); w1_fill(r, 0x800, 0x100);
        static void *table[4];
        table[1] = (void *)stub_plain_dtor;
        arena<void *>(0x600)[0] = table; arena<void *>(0x620)[0] = table;
        *arena<uint8_t>(0x604) = r.range(0, 1); *arena<uint8_t>(0x624) = r.range(0, 1);
        arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x20)[1] = r.next(); arena<uint32_t>(0x20)[2] = r.next();
    };
    Test w1c_registered_test{0x4d3cb0, w1c_registries,
                             [](void *fn) {
                                 *w1c_global = (uint32_t)(uintptr_t)arena<void>(0x620);
                                 void *given = arena<uint32_t>(0x20)[0] & 1 ? arena<void>(0x600) : nullptr;
                                 return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, uint32_t, uint32_t))fn)(arena<void>(0x800), given, arena<uint32_t>(0x20)[1], arena<uint32_t>(0x20)[2])};
                             }};
    w1c_registered_test.prepare = [](const rewrite::Entry *e) {
        w1c_global = (uint32_t *)w1c_word(e, 0x2c);
        stubs_for_now({{w1c_at(e, 0x3e), (void *)w1_stub_this3}, {0xbeca40, (void *)w1c_stub_text}, {0xbecc10, (void *)stub_thiscall1}, {0xbec5e0, (void *)w1c_stub_text_end}});
    };
    w1c_registered_test.finish = w1_unstub;
    quick(w1c_registered_test);
    Test w1c_interfaced_test{0x79b340, w1c_registries,
                             [](void *fn) {
                                 *w1c_global = (uint32_t)(uintptr_t)arena<void>(0x620);
                                 void *given = arena<uint32_t>(0x20)[0] & 1 ? arena<void>(0x600) : nullptr;
                                 return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x800), given)};
                             }};
    w1c_interfaced_test.prepare = [](const rewrite::Entry *e) {
        w1c_global = (uint32_t *)w1c_word(e, 0x57);
        stubs_for_now({{w1c_at(e, 0x36), (void *)stub_plain_dtor}, {0xbeca40, (void *)w1c_stub_text}, {0xbecc10, (void *)stub_thiscall1}, {0xbec5e0, (void *)w1c_stub_text_end}});
    };
    w1c_interfaced_test.finish = w1_unstub;
    quick(w1c_interfaced_test);
    static uint32_t w1c_held;
    Test w1c_named_send_test{0x5fd440,
                             [=](Rng &r) { fresh(r); w1_fill(r, 0x800, 0x200); *(void **)arena<uint8_t>(0x800 + w1c_held) = arena<void>(0x1000); },
                             [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1c_named_send_test.prepare = [](const rewrite::Entry *e) {
        w1c_held = w1c_word(e, 0x37);
        stubs_for_now({{0x6afd60, (void *)stub_plain_dtor}, {0x4040f0, (void *)w1b_stub_ref}, {0xbd14e0, (void *)w1c_stub_take_ref}, {0xbd1880, (void *)w1b_stub_ref_end}});
    };
    w1c_named_send_test.finish = w1_unstub;
    quick(w1c_named_send_test);
    Test w1c_spawn_test{0x52f4c0,
                        [=](Rng &r) {
                            fresh(r); w1_fill(r, 0x800, 0x100);
                            static void *table[4];
                            table[2] = (void *)w1c_stub_virtual;
                            arena<void *>(0x640)[0] = table;
                            arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x20)[1] = r.next();
                        },
                        [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x850), arena<uint32_t>(0x20)[0], arena<uint32_t>(0x20)[1])}; }};
    w1c_spawn_test.prepare = [](const rewrite::Entry *e) { stubs_for_now({{0xbcdfc0, (void *)stub_alloc_next}, {w1c_at(e, 0x3f), (void *)w1c_stub_spawned}}); };
    w1c_spawn_test.finish = w1_unstub;
    quick(w1c_spawn_test);
    Test w1c_singleton_test{0x6c1da0,
                            [=](Rng &r) {
                                fresh(r); w1_fill(r, 0x600, 0x100);
                                arena<uint32_t>(0x20)[0] = r.next();
                            },
                            [](void *fn) {
                                *w1c_global = (uint32_t)(uintptr_t)arena<void>(0x600);
                                return Result{(uint32_t)(uintptr_t)((void *(__cdecl *)())fn)()};
                            }};
    w1c_singleton_test.prepare = [](const rewrite::Entry *e) {
        w1c_global = (uint32_t *)w1c_word(e, 0x40);
        stubs_for_now({{0xbcdfc0, (void *)w1c_stub_alloc_maybe}, {w1c_at(e, 0x3b), (void *)w1_stub_this3}, {0x800860, (void *)w1c_stub_swap}});
    };
    w1c_singleton_test.finish = w1_unstub;
    quick(w1c_singleton_test);
    Test w1c_twice_test{0x6b0b50,
                        [=](Rng &r) { fresh(r); arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x20)[1] = r.next(); },
                        [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1])}; }};
    w1c_twice_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6b0370, (void *)w1c_stub_mid}, {0x6b0a90, (void *)w1c_stub_mid_end}, {0x6aed50, (void *)w1c_stub_add}}); };
    w1c_twice_test.finish = w1_unstub;
    quick(w1c_twice_test);
    auto w1c_pool_setup = [=](Rng &r) { fresh(r); arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x20)[1] = r.next(); arena<uint32_t>(0x24)[0] = r.next(); };
    auto w1c_pool_prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xc0d5a0, (void *)w1_stub_ask}, {0xca1eb0, (void *)w1c_stub_pool}, {w1c_at(e, 0x4e), (void *)w1c_stub_built}});
    };
    Test w1c_pool_test{0xc0e1b0, w1c_pool_setup,
                       [](void *fn) {
                           *(void **)0x13e3318 = arena<void>(0x600);
                           return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1])};
                       }};
    w1c_pool_test.prepare = w1c_pool_prepare;
    w1c_pool_test.finish = w1_unstub;
    quick(w1c_pool_test);
    Test w1c_pool2_test{0xc12600, w1c_pool_setup,
                        [](void *fn) {
                            *(void **)0x13e3318 = arena<void>(0x600);
                            return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1], 5)};
                        }};
    w1c_pool2_test.prepare = w1c_pool_prepare;
    w1c_pool2_test.finish = w1_unstub;
    quick(w1c_pool2_test);
    Test w1c_managed_test{0x46d230,
                          [=](Rng &r) { fresh(r); w1_fill(r, 0x800, 0x100); arena<uint32_t>(0x20)[0] = r.next(); },
                          [](void *fn) {
                              *w1c_global = arena<uint32_t>(0x20)[0];
                              ((void(__fastcall *)(void *))fn)(arena<void>(0x800));
                              return Result{*w1c_global};
                          }};
    w1c_managed_test.prepare = [](const rewrite::Entry *e) {
        w1c_global = (uint32_t *)w1c_word(e, 0x34);
        stubs_for_now({{w1c_at(e, 0x39), (void *)w1c_stub_see_global}, {0x800940, (void *)stub_plain_dtor}});
    };
    w1c_managed_test.finish = w1_unstub;
    quick(w1c_managed_test);
    Test w1c_listener_test{0x7fd780,
                           [=](Rng &r) { fresh(r); w1_fill(r, 0x2000, 0x20); arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x20)[1] = r.next(); },
                           [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(uint32_t, void *))fn)(arena<uint32_t>(0x20)[1], arena<void>(0x800))}; }};
    w1c_listener_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfc0, (void *)w1c_stub_alloc_maybe}, {0x7fbf50, (void *)w1c_stub_pair}}); };
    w1c_listener_test.finish = w1_unstub;
    quick(w1c_listener_test);
    Test w1c_resize_test{0x7e5350,
                         [=](Rng &r) {
                             fresh(r);
                             arena<uint32_t>(0x20)[0] = r.next();
                             arena<uint32_t>(0x20)[1] = r.range(0, 2) ? r.range(1, 9) : 0;
                             arena<uint32_t>(0x800)[0] = r.range(0, 1) ? r.next() | 1 : 0;
                             arena<uint32_t>(0x800)[1] = r.next();
                         },
                         [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1])}; }};
    w1c_resize_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)w1c_stub_alloc_maybe}, {0xbce010, (void *)w1_stub_crt_free}, {0x402b90, (void *)w1b_stub_vector_dtor}}); };
    w1c_resize_test.finish = w1_unstub;
    quick(w1c_resize_test);
    Test w1d_holder_test{0x7e6d70,
                         [=](Rng &r) { fresh(r); w1_fill(r, 0x800, 0x40); arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x20)[1] = r.next(); },
                         [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1])}; }};
    w1d_holder_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)w1d_stub_aligned}}); };
    w1d_holder_test.finish = w1_unstub;
    quick(w1d_holder_test);
    Test w1d_watched_test{0x7cd180,
                          [=](Rng &r) { fresh(r); w1_fill(r, 0x800, 0x40); },
                          [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1d_watched_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x40f730, (void *)w1d_stub_member_end}}); };
    w1d_watched_test.finish = w1_unstub;
    quick(w1d_watched_test);
    Test w1d_text_pair_test{0x87f560,
                            [=](Rng &r) { fresh(r); w1_fill(r, 0x800, 0x10); arena<uint32_t>(0x30)[0] = r.next(); },
                            [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w1d_text_pair_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbd0490, (void *)w1d_stub_empty_text}}); };
    w1d_text_pair_test.finish = w1_unstub;
    quick(w1d_text_pair_test);
    Test w1d_forward_test{0x588ae0,
                          [=](Rng &r) {
                              fresh(r);
                              iface_handle(r);
                              w1_fill(r, 0x800, 0x48);
                              arena<void *>(0x800)[0x11] = arena<void>(0x340);
                              arena<uint32_t>(0x20)[1] = r.next();
                          },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1])}; }};
    w1d_forward_test.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)w1c_word(e, 0x1b);
        stubs_for_now({{w1c_at(e, 0x3a), (void *)w1d_stub_act}});
    };
    w1d_forward_test.finish = w1_unstub;
    quick(w1d_forward_test);
    Test w1d_linked_test{0xc72810,
                         [=](Rng &r) {
                             fresh(r);
                             w1_fill(r, 0x800, 0x40);
                             w1_fill(r, 0x900, 0x10);
                             static void *vt[3] = {nullptr, nullptr, (void *)w1d_stub_release};
                             arena<void *>(0x900)[0] = vt;
                             arena<void *>(0x800)[5] = r.range(0, 1) ? arena<void>(0x900) : nullptr;
                             void *node = arena<uint8_t>(0x800) + 4;
                             arena<void *>(0x800)[1] = arena<void>(0xa00);
                             arena<void *>(0x800)[2] = arena<void>(0xa20);
                             arena<void *>(0xa00)[1] = node;
                             arena<void *>(0xa20)[0] = node;
                             arena<uint32_t>(0xa40)[0] = r.next();
                             arena<void *>(0x800)[4] = arena<void>(0xa40);
                         },
                         [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1d_linked_test.finish = w1_unstub;
    quick(w1d_linked_test);
    Test w1d_owned_test{0xbea2d0,
                        [=](Rng &r) { fresh(r); w1_fill(r, 0x800, 0x40); w1_fill(r, 0x2000, 0x20); arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x20)[1] = r.next(); },
                        [](void *fn) {
                            return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[0], arena<uint32_t>(0x20)[1])};
                        }};
    w1d_owned_test.prepare = [](const rewrite::Entry *e) { stubs_for_now({{w1c_at(e, 0x4d), (void *)w1d_stub_record}}); };
    w1d_owned_test.finish = w1_unstub;
    quick(w1d_owned_test);
    Test w1d_shared_test{0x87f8c0,
                         [=](Rng &r) {
                             fresh(r);
                             w1_fill(r, 0x800, 0x40);
                             w1_fill(r, 0xa00, 0x20);
                             arena<void *>(0x800)[2] = arena<void>(0xa00);
                             arena<uint32_t>(0xa00)[0] = r.range(1, 2);
                         },
                         [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1d_shared_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbce010, (void *)w1_stub_crt_free}, {0xbce000, (void *)w1d_stub_free}, {0xbd99e0, (void *)w1d_stub_end_seen}});
    };
    w1d_shared_test.finish = w1_unstub;
    quick(w1d_shared_test);
    Test w1d_maker_test{0xc59f00,
                        [=](Rng &r) {
                            fresh(r);
                            w1_fill(r, 0x800, 0x40);
                            if (r.range(0, 2) == 0) arena<uint32_t>(0x800)[8] = 0;
                            for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                        },
                        [](void *fn) {
                            return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1], arena<uint32_t>(0x20)[2])};
                        }};
    w1d_maker_test.prepare = [](const rewrite::Entry *e) { stubs_for_now({{0xbcdfc0, (void *)w1c_stub_alloc_maybe}, {w1c_at(e, 0x48), (void *)w1d_stub_build3}}); };
    w1d_maker_test.finish = w1_unstub;
    quick(w1d_maker_test);
    Test w1d_derived_test{0xca3d30,
                          [=](Rng &r) { fresh(r); w1_fill(r, 0x800, 0x40); for (int i = 0; i < 5; i++) arena<uint32_t>(0x20)[i] = r.next(); },
                          [](void *fn) {
                              uint32_t *v = arena<uint32_t>(0x20);
                              return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, void *, uint32_t))fn)(
                                  arena<void>(0x800), v[0], v[1], v[2], arena<void>(0x900), v[4])};
                          }};
    w1d_derived_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xca3750, (void *)w1d_stub_base4}, {0xcb8360, (void *)w1d_stub_apply}}); };
    w1d_derived_test.finish = w1_unstub;
    quick(w1d_derived_test);
    Test w1d_fresh_test{0x77a5b0,
                        [=](Rng &r) { fresh(r); arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x20)[1] = r.next(); },
                        [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(uint32_t, uint32_t))fn)(arena<uint32_t>(0x20)[1], 7)}; }};
    w1d_fresh_test.prepare = [](const rewrite::Entry *e) { stubs_for_now({{0xbcdfc0, (void *)w1c_stub_alloc_maybe}, {w1c_at(e, 0x3c), (void *)w1d_stub_build_ref}}); };
    w1d_fresh_test.finish = w1_unstub;
    quick(w1d_fresh_test);
    Test w1d_buffered_test{0x90b3b0,
                           [=](Rng &r) {
                               fresh(r);
                               w1_fill(r, 0x800, 0x40);
                               if (r.range(0, 2) == 0) { arena<uint32_t>(0x800)[1] = 0; arena<uint32_t>(0x800)[2] = 0; }
                           },
                           [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1d_buffered_test.prepare = [](const rewrite::Entry *e) { stubs_for_now({{0xbce010, (void *)w1_stub_crt_free}, {w1c_at(e, 0x29), (void *)w1d_stub_end_seen}}); };
    w1d_buffered_test.finish = w1_unstub;
    quick(w1d_buffered_test);
    Test w1d_pooled_test{0xbebcb0,
                         [=](Rng &r) {
                             fresh(r);
                             w1_fill(r, 0x800, 0x40);
                             if (r.range(0, 2) == 0) { arena<uint32_t>(0x800)[8] = 0; arena<uint32_t>(0x800)[9] = 0; }
                         },
                         [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1d_pooled_test.prepare = [](const rewrite::Entry *e) { stubs_for_now({{0xbce010, (void *)w1_stub_crt_free}, {w1c_at(e, 0x4a), (void *)w1d_stub_end_seen}}); };
    w1d_pooled_test.finish = w1_unstub;
    quick(w1d_pooled_test);
    Test w1d_paired_test{0x8cdcd0,
                         [=](Rng &r) { fresh(r); w1_fill(r, 0x800, 0x100); },
                         [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1d_paired_test.prepare = [](const rewrite::Entry *e) { stubs_for_now({{w1c_at(e, 0x39), (void *)w1d_stub_first}, {w1c_at(e, 0x48), (void *)w1d_stub_end_seen}}); };
    w1d_paired_test.finish = w1_unstub;
    quick(w1d_paired_test);
    Test w1d_built_test{0xc595a0,
                        [=](Rng &r) { fresh(r); for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next(); arena<uint32_t>(0x800)[0] = r.next(); },
                        [](void *fn) {
                            uint32_t *v = arena<uint32_t>(0x20);
                            ((void(__fastcall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), v[1], v[2]);
                            return Result{};
                        }};
    w1d_built_test.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbcdfc0, (void *)w1c_stub_alloc_maybe}, {w1c_at(e, 0x42), (void *)w1c_stub_built}, {0xc51900, (void *)w1d_stub_first}, {0xc501a0, (void *)w1d_stub_end_seen}});
    };
    w1d_built_test.finish = w1_unstub;
    quick(w1d_built_test);
    Test w1d_single_test{0x402f70,
                         [=](Rng &r) { fresh(r); arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x20)[1] = r.next(); },
                         [](void *fn) {
                             *w1c_global = arena<uint32_t>(0x20)[0] & 4 ? arena<uint32_t>(0x20)[0] : 0;
                             uint32_t got = ((uint32_t(__cdecl *)())fn)();
                             return Result{got ^ *w1c_global};
                         }};
    w1d_single_test.prepare = [](const rewrite::Entry *e) {
        w1c_global = (uint32_t *)w1c_word(e, 0x17);
        stubs_for_now({{0xbcdfc0, (void *)w1c_stub_alloc_maybe}, {w1c_at(e, 0x3b), (void *)w1d_stub_registers}});
    };
    w1d_single_test.finish = w1_unstub;
    quick(w1d_single_test);
    Test w1d_started_test{0x416f10,
                          [=](Rng &r) { fresh(r); arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x20)[1] = r.next(); },
                          [](void *fn) {
                              *w1c_global = arena<uint32_t>(0x20)[0] & 4 ? (uint32_t)(uintptr_t)arena<void>(0x900) : 0;
                              uint32_t got = (uint32_t)(uintptr_t)((void *(__cdecl *)())fn)();
                              return Result{got ^ *w1c_global};
                          }};
    w1d_started_test.prepare = [](const rewrite::Entry *e) {
        w1c_global = (uint32_t *)w1c_word(e, 0x17);
        stubs_for_now({{0xbcdfc0, (void *)w1d_stub_alloc_always}, {w1c_at(e, 0x41), (void *)w1d_stub_build_flags}, {0x800860, (void *)w1d_stub_first}});
    };
    w1d_started_test.finish = w1_unstub;
    quick(w1d_started_test);
    Test w1d_parts_test{0x598620,
                        [=](Rng &r) {
                            fresh(r);
                            w1_fill(r, 0x800, 0x40);
                            arena<void *>(0x800)[7] = arena<void>(0x900);
                            static void *vt[1] = {(void *)w1d_stub_delete};
                            for (int i = 0; i < 5; i++) {
                                arena<void *>(0xc00 + i * 0x10)[0] = vt;
                                arena<void *>(0x900 + 0x190)[i] = r.range(0, 2) ? arena<void>(0xc00 + i * 0x10) : nullptr;
                            }
                        },
                        [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), 9); return Result{}; }};
    w1d_parts_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6a1e20, (void *)w1d_stub_end_seen}}); };
    w1d_parts_test.finish = w1_unstub;
    quick(w1d_parts_test);
    Test w1d_pair_resize_test{0x733560,
                              [=](Rng &r) {
                                  fresh(r);
                                  arena<uint32_t>(0x20)[0] = r.next();
                                  arena<uint32_t>(0x20)[1] = r.range(0, 2) ? r.range(1, 9) : 0;
                                  arena<uint32_t>(0x800)[0] = r.range(0, 1) ? r.next() | 1 : 0;
                                  arena<uint32_t>(0x800)[1] = r.next();
                              },
                              [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1])}; }};
    w1d_pair_resize_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)w1c_stub_alloc_maybe}, {0xbce010, (void *)w1_stub_crt_free}, {0x402b90, (void *)w1b_stub_vector_dtor}}); };
    w1d_pair_resize_test.finish = w1_unstub;
    quick(w1d_pair_resize_test);
    Test w1d_allows_test{0x563920,
                         [=](Rng &r) {
                             fresh(r);
                             iface_handle(r);
                             w1_fill(r, 0x800, 0x100);
                             for (int i = 0; i < 4; i++) arena<uint32_t>(0x20)[i] = r.next();
                             *(void **)arena<uint8_t>(0x800 + 0xdc) = arena<void>(0x340);
                             arena<uint8_t>(0x800)[0xf0] = (uint8_t)r.range(0, 1);
                             int pick = r.range(0, 5);
                             *(uint32_t *)arena<uint8_t>(0x800 + 0x60) = pick < 4 ? w1d_modes[pick] : r.next() & 0x7f;
                             static void *vt[14] = {};
                             vt[13] = (void *)w1d_stub_switch;
                             arena<void *>(0x800)[0] = vt;
                         },
                         [](void *fn) { return Result{((bool(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1])}; }};
    w1d_allows_test.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x12f5638;
        static const uint32_t at[4] = {0x6d, 0x72, 0x77, 0x7c};
        for (int i = 0; i < 4; i++) w1d_modes[i] = *(uint8_t *)(e->at + at[i]);
        stubs_for_now({{0x5d4e20, (void *)w1d_stub_owner}});
    };
    w1d_allows_test.finish = w1_unstub;
    quick(w1d_allows_test);
    Test w1d_matrix_test{0x559c20,
                         [=](Rng &r) {
                             fresh(r);
                             iface_handle(r);
                             *(void **)arena<uint8_t>(0x340 + 0xc) = arena<void>(0x360);
                             *arena<void *>(0x1f0) = arena<void>(0x200);
                             *g_iface_id = (uint32_t)r.range(1, 2);
                             w1_fill(r, 0x400, 0x80);
                             w1_fill(r, 0x600, 0x40);
                             for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                             uint8_t *ref = arena<uint8_t>(0xa00 + 0x94);
                             *(void **)(ref + 4) = arena<void>(0x340);
                             *(void **)(ref + 0x14) = arena<void>(0x510);
                             ref[0x18] = (uint8_t)r.range(0, 1);
                         },
                         [](void *fn) {
                             return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x800), arena<void>(0x700), arena<uint32_t>(0x20)[2])};
                         }};
    w1d_matrix_test.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x130b5c8;
        stubs_for_now({{w1c_at(e, 0x5), (void *)w1d_stub_part}, {0x557ee0, (void *)w1d_stub_slow}});
    };
    w1d_matrix_test.finish = w1_unstub;
    quick(w1d_matrix_test);
    Test w1d_pass_test{0x4ee6b0,
                       [=](Rng &r) {
                           fresh(r);
                           w1_fill(r, 0x3000, 0x700);
                           for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                           static void *vt[4] = {nullptr, (void *)w1d_stub_slot<1>, (void *)w1d_stub_slot<2>, (void *)w1d_stub_slot<3>};
                           arena<void *>(0xa40)[0] = vt;
                           arena<void *>(0xb40)[0] = vt;
                           arena<uint8_t>(0xb00)[0xd4] = (uint8_t)r.range(0, 1);
                           uint8_t *self = arena<uint8_t>(0x3000);
                           static const int32_t modes[5] = {2, 3, 4, 1, 5};
                           *(int32_t *)(self + 0x144) = modes[r.range(0, 4)];
                           *(void **)(self + 0x660) = r.range(0, 2) ? arena<void>(0xa00) : nullptr;
                           self[0x668] = (uint8_t)r.range(0, 1);
                       },
                       [](void *fn) {
                           ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x3000), arena<uint32_t>(0x20)[1], arena<uint32_t>(0x20)[2]);
                           return Result{};
                       }};
    w1d_pass_test.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{w1c_at(e, 0x16), (void *)w1d_stub_helper}, {0x5d5030, (void *)w1d_stub_car_a}, {0x5d4fd0, (void *)w1d_stub_car_b}});
    };
    w1d_pass_test.finish = w1_unstub;
    quick(w1d_pass_test);
    Test w1d_update_test{0x4d1e80,
                         [=](Rng &r) {
                             fresh(r);
                             iface_handle(r);
                             *(void **)arena<uint8_t>(0x340 + 0xc) = arena<void>(0x360);
                             *arena<void *>(0x1f0) = arena<void>(0x200);
                             *g_iface_id = (uint32_t)r.range(1, 2);
                             w1_fill(r, 0x3a0, 0x60);
                             w1_fill(r, 0x800, 0xc0);
                             *(void **)arena<uint8_t>(0x800 + 0x58) = arena<void>(0x340);
                             for (int i = 0; i < 4; i++) arena<uint32_t>(0x20)[i] = r.next();
                         },
                         [](void *fn) {
                             uint32_t *v = arena<uint32_t>(0x20);
                             return Result{((bool(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x800), v[1], v[2], v[3])};
                         }};
    w1d_update_test.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x1309020;
        stubs_for_now({{w1c_at(e, 0x48), (void *)w1d_stub_ask}});
    };
    w1d_update_test.finish = w1_unstub;
    quick(w1d_update_test);
    Test w1d_owner_test{0xb87eb0,
                        [=](Rng &r) {
                            fresh(r);
                            iface_handle(r);
                            *(void **)arena<uint8_t>(0x340 + 0xc) = arena<void>(0x360);
                            *arena<void *>(0x1f0) = arena<void>(0x200);
                            *g_iface_id = (uint32_t)r.range(1, 2);
                            *(uint32_t *)0x130b5c8 = (uint32_t)r.range(1, 2);
                            w1_fill(r, 0x800, 0x100);
                            for (uint32_t ref : {0x8a0u, 0x8bcu}) {
                                *(void **)arena<uint8_t>(ref + 4) = arena<void>(0x340);
                                *(void **)arena<uint8_t>(ref + 0x14) = arena<void>(0xa00);
                                arena<uint8_t>(ref)[0x18] = (uint8_t)r.range(0, 1);
                            }
                            for (uint32_t ref : {0x3f4u, 0x3fcu, 0xa94u}) {
                                *(void **)arena<uint8_t>(ref + 4) = arena<void>(0x340);
                                *(void **)arena<uint8_t>(ref + 0x14) = arena<void>(ref == 0x3fcu ? 0xb40 : 0xb00);
                                arena<uint8_t>(ref)[0x18] = (uint8_t)r.range(0, 1);
                            }
                            w1_fill(r, 0xb80, 0x10);
                            w1_fill(r, 0xbc0, 0x10);
                            w1_fill(r, 0x3e0, 0x10);
                            w1_fill(r, 0x3e8, 0x8);
                        },
                        [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w1d_owner_test.prepare = [](const rewrite::Entry *) { g_iface_id = (uint32_t *)0x130bbec; };
    w1d_owner_test.finish = w1_unstub;
    quick(w1d_owner_test);
    auto w1d_owned_object = [](Rng &r, uint32_t slot) {
        static void *vt[1] = {(void *)w1d_stub_delete};
        arena<void *>(0xc00)[0] = vt;
        *(void **)arena<uint8_t>(0x800 + slot) = r.range(0, 2) ? arena<void>(0xc00) : nullptr;
    };
    Test w1d_quad_test{0x4def00,
                       [=](Rng &r) { fresh(r); w1_fill(r, 0x800, 0x80); w1d_owned_object(r, 0x24); },
                       [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1d_quad_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x40f730, (void *)w1d_stub_end_n<1>}, {0x4b48a0, (void *)w1d_stub_end_n<2>}, {0xcd2b50, (void *)w1d_stub_end_n<3>}});
    };
    w1d_quad_test.finish = w1_unstub;
    quick(w1d_quad_test);
    Test w1d_scene_test{0x6d85a0,
                        [=](Rng &r) { fresh(r); w1_fill(r, 0x800, 0x140); w1d_owned_object(r, 0x11c); },
                        [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1d_scene_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x6af760, (void *)w1d_stub_end_n<4>}, {0x6b0150, (void *)w1d_stub_end_n<5>}, {0x6d4520, (void *)w1d_stub_end_n<6>}});
    };
    w1d_scene_test.finish = w1_unstub;
    quick(w1d_scene_test);
    Test w1d_named_test{0xad1220,
                        [=](Rng &r) {
                            fresh(r);
                            w1_fill(r, 0x800, 0x3a0);
                            w1_fill(r, 0xc00, 0x20);
                            *(void **)arena<uint8_t>(0x800 + 0x398) = arena<void>(0xc00);
                            arena<uint32_t>(0xc00)[0] = r.range(1, 2);
                        },
                        [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1d_named_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbce010, (void *)w1_stub_crt_free}, {0xbce000, (void *)w1d_stub_free}, {0x6a69d0, (void *)w1d_stub_end_n<7>},
                       {0x8002c0, (void *)w1d_stub_end_n<8>}, {0x68a3d0, (void *)w1d_stub_end_n<9>}});
    };
    w1d_named_test.finish = w1_unstub;
    quick(w1d_named_test);
    Test w1d_label_test{0x503930,
                        [=](Rng &r) {
                            fresh(r);
                            w1_fill(r, 0x800, 0x40);
                            w1_fill(r, 0xc00, 0x80);
                            arena<uint32_t>(0x20)[0] = r.next();
                            static void *vt[20] = {};
                            vt[19] = (void *)w1d_stub_label;
                            arena<void *>(0x800)[0] = vt;
                            arena<void *>(0x800)[6] = r.range(0, 3) ? arena<void>(0xa00) : nullptr;
                            *(void **)arena<uint8_t>(0xa00 + 0xe0) = arena<void>(0xb00);
                            *(void **)arena<uint8_t>(0xb00 + 0x114) = arena<void>(0xc40);
                            arena<int32_t>(0xc00)[0] = r.range(0, 1);
                            arena<int32_t>(0xc40)[0] = r.range(1, 2);
                        },
                        [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1d_label_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbce010, (void *)w1_stub_crt_free}, {0xbce000, (void *)w1d_stub_free}}); };
    w1d_label_test.finish = w1_unstub;
    quick(w1d_label_test);
    Test w1d_record_resize_test{0x80c8b0,
                                [=](Rng &r) {
                                    fresh(r);
                                    w1_fill(r, 0x900, 0x100);
                                    arena<uint32_t>(0x20)[1] = r.range(0, 6);
                                    arena<uint32_t>(0x800)[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x900) : 0;
                                    arena<uint32_t>(0x800)[1] = arena<uint32_t>(0x800)[0] ? r.range(0, 6) : 0;
                                },
                                [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1])}; }};
    w1d_record_resize_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)w1d_stub_alloc_always}, {0xbce010, (void *)w1_stub_crt_free}, {0x402b90, (void *)w1b_stub_vector_dtor}}); };
    w1d_record_resize_test.finish = w1_unstub;
    quick(w1d_record_resize_test);
    auto w1d_array_setup = [=](Rng &r, uint32_t header) {
        fresh(r);
        w1_fill(r, 0x900, 0x200);
        uint32_t old = r.range(0, 3) ? r.range(1, 5) : 0;
        arena<uint32_t>(0x900)[0] = old;
        arena<uint32_t>(0x800)[0] = old ? (uint32_t)(uintptr_t)arena<void>(0x900 + header) : 0;
        arena<uint32_t>(0x800)[1] = old;
        arena<uint32_t>(0x20)[1] = r.range(0, 2) ? r.range(0, 6) : old;
    };
    auto w1d_array_prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfd0, (void *)w1d_stub_alloc_always}, {0xbce010, (void *)w1_stub_crt_free}, {0xddcf99, (void *)w1d_stub_build_items},
                       {0xddcf39, (void *)w1d_stub_destroy_items}});
    };
    auto w1d_array_call = [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1])}; };
    Test w1d_slot_resize_test{0x424540, [=](Rng &r) { w1d_array_setup(r, 4); }, w1d_array_call};
    w1d_slot_resize_test.prepare = [=](const rewrite::Entry *e) {
        w1d_array_prepare(e);
        stubs_for_now({{w1c_at(e, 0xb0), (void *)w1d_stub_assign}});
    };
    w1d_slot_resize_test.finish = w1_unstub;
    quick(w1d_slot_resize_test);
    static uint32_t w1d_header;
    Test w1d_block_reset_test{0x462fc0, [=](Rng &r) { w1d_array_setup(r, w1d_header); }, w1d_array_call};
    w1d_block_reset_test.prepare = [=](const rewrite::Entry *e) {
        w1d_header = 0x100 - *(uint8_t *)(e->at + 0x24);
        w1d_array_prepare(e);
    };
    w1d_block_reset_test.finish = w1_unstub;
    quick(w1d_block_reset_test);
    Test w1d_text_resize_test{0x460f20,
                              [=](Rng &r) {
                                  w1d_array_setup(r, 4);
                                  arena<uint32_t>(0x20)[2] = r.next();
                                  w1_fill(r, 0xe00, 0x80);
                                  for (int i = 0; i < 4; i++) arena<uint32_t>(0xe00 + i * 0x20)[0] = r.range(0, 2);
                                  for (int i = 0; i < 6; i++) arena<uint32_t>(0x904)[i] = (uint32_t)(uintptr_t)arena<void>(0xe00 + r.range(0, 3) * 0x20);
                              },
                              w1d_array_call};
    w1d_text_resize_test.prepare = [=](const rewrite::Entry *e) {
        w1d_array_prepare(e);
        stubs_for_now({{0xddcf99, (void *)w1d_stub_build_texts}, {0xbce000, (void *)w1d_stub_free}});
    };
    w1d_text_resize_test.finish = w1_unstub;
    quick(w1d_text_resize_test);
    Test w1d_plain_resize_test{0x863a60,
                               [=](Rng &r) {
                                   fresh(r);
                                   w1_fill(r, 0x900, 0x200);
                                   arena<uint32_t>(0x20)[1] = r.range(0, 6);
                                   arena<uint32_t>(0x800)[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x900) : 0;
                                   arena<uint32_t>(0x800)[1] = arena<uint32_t>(0x800)[0] ? r.range(0, 6) : 0;
                               },
                               [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1])}; }};
    w1d_plain_resize_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)w1d_stub_alloc_always}, {0xbce010, (void *)w1_stub_crt_free}, {0x402b90, (void *)w1b_stub_vector_dtor}}); };
    w1d_plain_resize_test.finish = w1_unstub;
    quick(w1d_plain_resize_test);
    Test w1d_panel_test{0x632da0,
                        [=](Rng &r) {
                            fresh(r);
                            w1_fill(r, 0x800, 0x360);
                            arena<uint8_t>(0x800)[0x30] = (uint8_t)r.range(0, 1);
                            arena<uint32_t>(0x20)[0] = r.next();
                            arena<uint32_t>(0x20)[1] = r.next();
                            static void *vt[1] = {(void *)w1d_stub_delete};
                            arena<void *>(0xc00)[0] = vt;
                            arena<void *>(0xc10)[0] = vt;
                            for (uint32_t at : {0x1e0u, 0x1e4u, 0x2ecu, 0x340u}) *(void **)arena<uint8_t>(0x800 + at) = r.range(0, 2) ? arena<void>(at & 4 ? 0xc10 : 0xc00) : nullptr;
                        },
                        [](void *fn) {
                            *(uint32_t *)0x10eeaac = arena<uint32_t>(0x20)[0];
                            *(uint32_t *)0x10eeab0 = arena<uint32_t>(0x20)[1];
                            ((void(__fastcall *)(void *))fn)(arena<void>(0x800));
                            return Result{*(uint32_t *)0x10eeaac};
                        }};
    w1d_panel_test.prepare = [](const rewrite::Entry *e) { stubs_for_now({{0x63ab90, (void *)w1d_stub_end_n<10>}, {w1c_at(e, 0x90), (void *)w1d_stub_end_n<11>}}); };
    w1d_panel_test.finish = w1_unstub;
    quick(w1d_panel_test);
    Test w1d_named_make_test{0xbe4940,
                             [=](Rng &r) { fresh(r); w1_fill(r, 0x800, 0x20); arena<uint32_t>(0x20)[1] = r.next(); },
                             [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    w1d_named_make_test.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbcdfc0, (void *)w1d_stub_alloc_always}, {w1c_at(e, 0x41), (void *)w1d_stub_make_text}, {w1c_at(e, 0x5d), (void *)w1d_stub_base_text},
                       {0xbce010, (void *)w1_stub_crt_free}, {0xbce000, (void *)w1d_stub_free}});
    };
    w1d_named_make_test.finish = w1_unstub;
    quick(w1d_named_make_test);
    auto w1d_attach_test = [=](uint32_t at, uint32_t find, uint32_t make, uint32_t build) {
        Test t{at,
               [=](Rng &r) { fresh(r); w1_fill(r, 0x800, 0x20); for (int i = 0; i < 2; i++) arena<uint32_t>(0x20)[i] = r.next(); },
               [](void *fn) {
                   *(void **)0x13e3318 = arena<void>(0x600);
                   return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x20)[1])};
               }};
        t.prepare = [=](const rewrite::Entry *e) {
            stubs_for_now({{w1c_at(e, find), (void *)w1d_stub_find_named}, {w1c_at(e, make), (void *)w1d_stub_copy_named}, {0xc0a6e0, (void *)w1d_stub_set_value},
                           {0xca1eb0, (void *)w1c_stub_pool}, {w1c_at(e, build), (void *)w1d_stub_build1}});
        };
        t.finish = w1_unstub;
        return t;
    };
    Test w1d_attach850 = w1d_attach_test(0xc0f850, 0x21, 0x38, 0x7b);
    quick(w1d_attach850);
    Test w1d_attach940 = w1d_attach_test(0xc0f940, 0x21, 0x38, 0x7e);
    quick(w1d_attach940);
    Test w1d_title_test{0x712040,
                        [=](Rng &r) { fresh(r); arena<uint32_t>(0x20)[0] = r.range(0, 1) ? r.next() : 0; },
                        [](void *fn) {
                            *w1c_global = arena<uint32_t>(0x20)[0];
                            ((void(__fastcall *)(void *))fn)(arena<void>(0x800));
                            return Result{};
                        }};
    w1d_title_test.prepare = [](const rewrite::Entry *e) {
        w1c_global = (uint32_t *)w1c_word(e, 0xf);
        stubs_for_now({{0x41cb90, (void *)w1d_stub_temp_text}, {0x67b4b0, (void *)w1d_stub_use_text}, {0xbd1880, (void *)w1d_stub_temp_end}});
    };
    w1d_title_test.finish = w1_unstub;
    quick(w1d_title_test);
    Test w1e_holder_test{0x69f500,
                         [=](Rng &r) { fresh(r); arena<int32_t>(0xe00)[0] = r.range(1, 2); arena<uint32_t>(0xe00)[3] = r.next(); },
                         [](void *fn) {
                             *(void **)arena<uint8_t>(0x800 + w1e_word) = arena<void>(0xe00);
                             ((void(__fastcall *)(void *))fn)(arena<void>(0x800));
                             return Result{(uint32_t)arena<int32_t>(0xe00)[0]};
                         }};
    w1e_holder_test.prepare = [](const rewrite::Entry *e) {
        w1e_word = w1c_word(e, 0x20);
        stubs_for_now({{0xbce010, (void *)w1_stub_crt_free}, {0xbce000, (void *)w1d_stub_free}, {w1c_at(e, 0x5a), (void *)w1e_stub_base}});
    };
    w1e_holder_test.finish = w1_unstub;
    quick(w1e_holder_test);
    Test w1e_ask_test{0xbe45e0,
                      [=](Rng &r) { fresh(r); arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x20)[1] = r.next(); },
                      [](void *fn) {
                          *(void **)0x13e33ac = arena<void>(0x600);
                          return Result{((uint8_t(__fastcall *)(void *))fn)(arena<void>(0x800))};
                      }};
    w1e_ask_test.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbcde60, (void *)w1e_stub_key}, {0xbcdf40, (void *)w1e_stub_guard}, {w1c_at(e, 0x42), (void *)w1e_stub_ask}, {0xbcddf0, (void *)w1e_stub_unlock}});
    };
    w1e_ask_test.finish = w1_unstub;
    quick(w1e_ask_test);
    Test w1e_drain_test{0x4a4950,
                        [=](Rng &r) {
                            fresh(r);
                            for (int i = 0; i < 4; i++) arena<uint8_t>(0xa00 + 0x40 * i)[0x2c] = (uint8_t)(r.next() & 0xfe);
                            arena<uint32_t>(0x30)[0] = r.range(1, 3);
                            arena<uint32_t>(0x900)[0] = r.range(0, 2) ? 1 : 0;
                            arena<uint32_t>(0x900)[2] = (uint32_t)(uintptr_t)arena<void>(0xa00);
                            arena<uint32_t>(0x20)[0] = r.range(0, 1) ? r.next() : 0;
                            arena<uint32_t>(0x20)[1] = r.range(0, 3);
                        },
                        [](void *fn) {
                            *(void **)w1e_word = arena<void>(0x900);
                            *(uint32_t *)w1e_word2 = arena<uint32_t>(0x20)[0];
                            void *self = arena<uint32_t>(0x20)[1] ? arena<void>(0x800) : nullptr;
                            uint32_t got = ((uint32_t(__fastcall *)(void *))fn)(self);
                            return Result{got ^ arena<uint32_t>(0x900)[0]};
                        }};
    w1e_drain_test.prepare = [](const rewrite::Entry *e) {
        w1e_word = w1c_word(e, 2);
        w1e_word2 = w1c_word(e, 0x31);
        stubs_for_now({{0x800860, (void *)w1e_stub_finish}, {0xbe83b0, (void *)w1e_stub_tell}});
    };
    w1e_drain_test.finish = w1_unstub;
    quick(w1e_drain_test);
    Test w1e_reset_test{0xa39f80,
                        [=](Rng &r) { fresh(r); arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x20)[1] = r.next(); },
                        [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    w1e_reset_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x9812f0, (void *)w1e_stub_clear}, {0xbcdfc0, (void *)w1c_stub_alloc_maybe}, {0xbcf850, (void *)w1d_stub_make_text}, {0x9828e0, (void *)w1e_stub_hand}});
    };
    w1e_reset_test.finish = w1_unstub;
    quick(w1e_reset_test);
    Test w1e_pairs_test{0xa5df20,
                        [=](Rng &r) {
                            fresh(r);
                            static void *vt[1] = {(void *)w1d_stub_delete};
                            for (int k = 0; k < 16; k++) arena<void *>(0xc00 + 0x10 * k)[0] = vt;
                            for (uint32_t at = 0; at < 0x1e0; at += 4) *(void **)arena<uint8_t>(0x800 + at) = r.range(0, 2) ? arena<void>(0xc00 + 0x10 * r.range(0, 15)) : nullptr;
                        },
                        [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{arena<uint32_t>(0x800)[0] ^ arena<uint32_t>(0x800)[1]}; }};
    w1e_pairs_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6b0ea0, (void *)w1e_stub_base}}); };
    w1e_pairs_test.finish = w1_unstub;
    quick(w1e_pairs_test);
    Test w1e_pooled_test{0xbe9480,
                         [=](Rng &r) {
                             fresh(r);
                             w1_fill(r, 0x800, 0x40);
                             arena<uint32_t>(0x800)[8] = (uint32_t)(uintptr_t)arena<void>(0x900);
                             static void *vt[3] = {nullptr, nullptr, (void *)w1e_stub_release};
                             arena<void *>(0xc80)[0] = vt;
                             arena<uint32_t>(0x800)[1] = (uint32_t)(uintptr_t)arena<void>(0xc80);
                             uint32_t *link = arena<uint32_t>(0x900 + 0x18);
                             int n = r.range(0, 4);
                             for (int k = 0; k < n; k++) {
                                 uint32_t *node = arena<uint32_t>(0xa00 + 0x20 * k);
                                 *link = (uint32_t)(uintptr_t)node;
                                 link = node + 6;
                             }
                             *link = 0;
                             arena<uint32_t>(0x20)[0] = r.next();
                             arena<uint32_t>(0x20)[1] = r.next();
                         },
                         [](void *fn) {
                             *(uint32_t *)w1e_word = arena<uint32_t>(0x20)[0];
                             *(uint32_t *)w1e_word2 = arena<uint32_t>(0x20)[1];
                             ((void(__fastcall *)(void *))fn)(arena<void>(0x800));
                             return Result{*(uint32_t *)w1e_word ^ (*(uint32_t *)w1e_word2 << 8)};
                         }};
    w1e_pooled_test.prepare = [](const rewrite::Entry *e) {
        w1e_word = w1c_word(e, 0x43);
        w1e_word2 = w1c_word(e, 0x52);
        stubs_for_now({{0xbce010, (void *)w1_stub_crt_free}});
    };
    w1e_pooled_test.finish = w1_unstub;
    quick(w1e_pooled_test);
    Test w1e_text_of_test{0x75a5d0,
                          [=](Rng &r) {
                              fresh(r);
                              for (uint32_t at : {0x84u, 0x88u, 0x8cu, 0x90u}) *(uint32_t *)arena<uint8_t>(0x800 + at) = r.range(0, 2) ? r.next() : 0;
                              arena<uint32_t>(0x20)[0] = r.next();
                              arena<uint32_t>(0x20)[1] = r.next();
                              arena<uint32_t>(0x20)[2] = r.range(0, 3) == 3 ? r.next() : r.range(0, 2);
                          },
                          [](void *fn) {
                              void **got = ((void **(__thiscall *)(void *, void *, int32_t))fn)(arena<void>(0x800), arena<void>(0x40), arena<int32_t>(0x20)[2]);
                              return Result{(uint32_t)(uintptr_t)got ^ (uint32_t)(uintptr_t)arena<void *>(0x40)[0]};
                          }};
    w1e_text_of_test.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)w1c_stub_alloc_maybe}, {0xbcf850, (void *)w1d_stub_make_text}, {0xbd0490, (void *)w1e_stub_empty_text}});
    };
    w1e_text_of_test.finish = w1_unstub;
    quick(w1e_text_of_test);

    // Wave 3.
    // Record::Assign: 64 bytes of noise over noise
    Test w3_assign{0x47e120,
                   [=](Rng &r) { fresh(r); for (int i = 0; i < 16; i++) arena<uint32_t>(0x800)[i] = r.next(); },
                   [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x800))}; }};
    quick(w3_assign);
    // Slots::Reset: over noise
    Test w3_slots{0xc71b30,
                  [=](Rng &r) { fresh(r); for (int i = 0; i < 0x180; i++) arena<uint32_t>(0x1000)[i] = r.next(); },
                  [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x1000))}; }};
    quick(w3_slots);
    // Base::Forward: two arguments for the base's constructor
    Test w3_forward{0xcc1ec0,
                    [=](Rng &r) { fresh(r); arena<uint32_t>(0x10)[0] = r.next(); arena<uint32_t>(0x10)[1] = r.next(); },
                    [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1])}; }};
    w3_forward.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbe79b0, (void *)stub_thiscall2}}); };
    w3_forward.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_forward);
    // Registered::Create: the registry at 0x400, the type's own list at 0x800 and every other type's at 0x900
    static uint32_t w3_reg_off;
    Test w3_registered{0x441030,
                       [=](Rng &r) {
                           fresh(r);
                           for (uint32_t h : {0x800u, 0x900u}) {
                               arena<void *>(h)[0] = arena<void>(h + 0x20);
                               arena<void *>(h)[1] = arena<void>(h + 0x20);
                               arena<void *>(h + 0x20)[0] = arena<void>(h);
                               arena<void *>(h + 0x20)[1] = arena<void>(h);
                           }
                           for (int i = 0; i < 0x20; i++) arena<void *>(0x400)[i] = arena<void>(0x900);
                           *(void **)arena<uint8_t>(0x400 + w3_reg_off + 0x10) = arena<void>(0x800);
                       },
                       [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w3_registered.prepare = [](const rewrite::Entry *e) {
        w3_reg_off = *(uint8_t *)(e->at + 0x17);
        stubs_for_now({{0x440fd0, (void *)w3_stub_registry}});
    };
    w3_registered.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_registered);
    // Tasks::FinishAll: 0-3 tasks at 0x900.. before the end node, some already finished
    static uint32_t *w3_tasks_list;
    Test w3_tasks{0x733270,
                  [=](Rng &r) {
                      fresh(r);
                      int n = r.range(0, 3);
                      for (int i = 0; i <= n; i++) {
                          void **node = arena<void *>(0x800 + i * 0x20);
                          node[0] = i < n ? arena<void>(0x820 + i * 0x20) : nullptr;
                          node[1] = nullptr;
                          node[2] = arena<void>(0x900 + i * 0x40);
                          arena<uint32_t>(0x900 + i * 0x40 + 0x2c)[0] = r.next();
                      }
                  },
                  [](void *fn) {
                      *w3_tasks_list = (uint32_t)(uintptr_t)arena<void>(0x800);
                      return Result{((uint32_t(__cdecl *)())fn)()};
                  }};
    w3_tasks.prepare = [](const rewrite::Entry *e) {
        w3_tasks_list = *(uint32_t **)(e->at + 3);
        stubs_for_now({{0x800860, (void *)stub_plain_dtor},
                       {(uintptr_t)(e->at + 0x29 + 5 + *(int32_t *)(e->at + 0x2a)), (void *)w3_stub_manager},
                       {(uintptr_t)(e->at + 0x31 + 5 + *(int32_t *)(e->at + 0x32)), (void *)w3_stub_then}});
    };
    w3_tasks.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_tasks);
    // List::PushFront: the list head at 0x800 with one node, kept at the copy's own offset
    static uint32_t w3_push_off;
    auto w3_push_setup = [=](Rng &r) {
        fresh(r);
        for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x1000)[i] = r.next();
        *(void **)arena<uint8_t>(0x1000 + w3_push_off) = arena<void>(0x800);
        arena<void *>(0x800)[0] = arena<void>(0x820);
        arena<void *>(0x800)[1] = arena<void>(0x820);
        arena<void *>(0x820)[0] = arena<void>(0x800);
        arena<void *>(0x820)[1] = arena<void>(0x800);
        arena<uint32_t>(0x10)[0] = r.next();
    };
    auto w3_push_call = [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x10)[0])}; };
    Test w3_push_wide{0x531060, w3_push_setup, w3_push_call};
    w3_push_wide.prepare = [](const rewrite::Entry *e) {
        w3_push_off = *(uint32_t *)(e->at + 0x1e);
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc_next}});
    };
    w3_push_wide.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_push_wide);
    Test w3_push_narrow{0x80b670, w3_push_setup, w3_push_call};
    w3_push_narrow.prepare = [](const rewrite::Entry *e) {
        w3_push_off = *(uint8_t *)(e->at + 0x1e);
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc_next}});
    };
    w3_push_narrow.finish = w3_push_wide.finish;
    quick(w3_push_narrow);
    // Marker::Create: 0-4 entries around level 2
    Test w3_marker{0xccb3c0,
                   [=](Rng &r) {
                       fresh(r);
                       int n = r.range(0, 4);
                       *(int32_t *)arena<uint8_t>(0x444) = n;
                       *(void **)arena<uint8_t>(0x448) = arena<void>(0x480);
                       for (int i = 0; i < n; i++) {
                           arena<void *>(0x480)[i] = arena<void>(0x500 + i * 0x20);
                           *(int32_t *)arena<uint8_t>(0x500 + i * 0x20 + 0xc) = r.range(0, 5) - 1;
                           *(uint32_t *)arena<uint8_t>(0x500 + i * 0x20 + 0x10) = r.next();
                       }
                       arena<uint32_t>(0x10)[0] = r.next();
                   },
                   [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<void>(0x420))}; }};
    quick(w3_marker);
    // Ref::Is: looked up or kept, against the two interfaces or nothing
    Test w3_ref_is{0x5639c0,
                   [=](Rng &r) {
                       fresh(r); iface_handle(r);
                       uint8_t *self = arena<uint8_t>(0x1000);
                       *(void **)(self + 0x240) = arena<void>(0x340);
                       *(void **)(self + 0x250) = arena<void>(0x368);
                       self[0x254] = (uint8_t)r.range(0, 1);
                       int pick = r.range(0, 2);
                       arena<void *>(0x10)[0] = pick == 0 ? nullptr : pick == 1 ? arena<void>(0x360) : arena<void>(0x368);
                   },
                   [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x1000), arena<void *>(0x10)[0])}; }};
    w3_ref_is.prepare = [](const rewrite::Entry *) { g_iface_id = (uint32_t *)0x12f537c; };
    quick(w3_ref_is);
    // Stream::ReadInt: swapped or not, with noise above the flag's byte
    Test w3_read_int{0xc42410,
                     [=](Rng &r) {
                         fresh(r);
                         arena<uint32_t>(0x20)[0] = r.next();
                         arena<uint32_t>(0x10)[0] = (r.next() & 0xffffff00) | (uint32_t)r.range(0, 1);
                     },
                     [](void *fn) { return Result{((uint32_t(__fastcall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    w3_read_int.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbe5020, (void *)w3_stub_read}}); };
    w3_read_int.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_read_int);
    // Owner::SetObject: an object with or without the interface, or none
    Test w3_set_object{0x4e0f20,
                       [=](Rng &r) {
                           fresh(r); iface_handle(r);
                           arena<void *>(0x10)[0] = r.range(0, 3) ? arena<void>(0x360) : nullptr;
                       },
                       [](void *fn) { return Result{((uint8_t(__thiscall *)(void *, void *))fn)(arena<void>(0x110), arena<void *>(0x10)[0])}; }};
    w3_set_object.prepare = [](const rewrite::Entry *e) {
        g_iface_id = *(uint32_t **)(e->at + 0x1f);
        stubs_for_now({{0x501f80, (void *)stub_thiscall1}, {(uintptr_t)(e->at + 0x3d + 5 + *(int32_t *)(e->at + 0x3e)), (void *)stub_thiscall2}});
    };
    w3_set_object.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_set_object);
    // References to objects at 0x1000 + i * 0x200: the handles at 0x900.., the array of 20-byte references at 0x800
    static uint32_t w3_refs_off;
    auto w3_refs = [=](Rng &r, bool always) {
        fresh(r); iface_handle(r);
        static void *vt[2] = {(void *)stub_deleting, (void *)stub_class_of};
        *arena<void *>(0x1f0) = always || r.range(0, 3) ? arena<void>(0x200) : nullptr;
        *g_iface_id = (uint32_t)(always ? r.range(1, 2) : r.range(0, 3));
        int n = r.range(0, 4);
        arena<void *>(0x100)[0] = arena<void>(0x800);
        arena<int32_t>(0x100)[2] = n;
        for (int i = 0; i < n; i++) {
            uint8_t *object = arena<uint8_t>(0x1000 + i * 0x200);
            *(void **)object = vt;
            *(void **)(object + 8) = vt;
            *(uint32_t *)(object + w3_refs_off) = r.range(0, 2) ? 0 : r.next();
            *(uint32_t *)(object + w3_refs_off + 8) = r.range(0, 2) ? 0 : r.next();
            *(void **)arena<uint8_t>(0x900 + i * 0x10 + 0xc) = always || r.range(0, 3) ? object : nullptr;
            *(void **)arena<uint8_t>(0x800 + i * 0x14 + 4) = arena<void>(0x900 + i * 0x10);
        }
        arena<uint32_t>(0x10)[0] = r.next();
    };
    Test w3_refs_delete{0x730bf0, [=](Rng &r) { w3_refs(r, true); },
                        [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w3_refs_delete.prepare = [](const rewrite::Entry *e) {
        g_iface_id = *(uint32_t **)(e->at + 0x2f);
        w3_refs_off = *(uint32_t *)(e->at + 0x4d);
    };
    quick(w3_refs_delete);
    Test w3_refs_find{0x730aa0, [=](Rng &r) { w3_refs(r, false); },
                      [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    w3_refs_find.prepare = [](const rewrite::Entry *e) {
        g_iface_id = *(uint32_t **)(e->at + 0x2e);
        w3_refs_off = *(uint32_t *)(e->at + 0x51);
        stubs_for_now({{0xbce970, (void *)w3_stub_compare}});
    };
    w3_refs_find.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_refs_find);
    // Names::AddStem: names of 4-40 letters into blocks with room, a little short, and far too short
    Test w3_stem{0x87ed50,
                 [=](Rng &r) {
                     fresh(r);
                     int cap = r.range(0, 64), size = cap ? r.range(0, cap - 1) : 0;
                     *(int32_t *)arena<uint8_t>(0x10c) = cap;
                     *(void **)arena<uint8_t>(0x110) = arena<void>(0x800);
                     *(int32_t *)arena<uint8_t>(0x114) = size;
                     for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x800)[i] = r.next();
                     int len = r.range(4, 40);
                     for (int i = 0; i < len; i++) arena<char>(0x700)[i] = (char)r.range('a', 'z');
                     arena<char>(0x700)[len] = 0;
                 },
                 [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x700))}; }};
    quick(w3_stem);
    // Watchers::Destroy: 0-3 nodes at 0x420.. between a head and an end node (a broken list stops at a breakpoint, which would stop the tester)
    Test w3_watchers{0xb53c70,
                     [=](Rng &r) {
                         fresh(r);
                         int n = r.range(0, 3);
                         void **head = arena<void *>(0x400);
                         head[0] = arena<void>(0x420);
                         head[1] = nullptr;
                         for (int i = 0; i <= n; i++) {
                             void **node = arena<void *>(0x420 + i * 0x20);
                             node[0] = i < n ? arena<void>(0x440 + i * 0x20) : nullptr;
                             node[1] = i ? arena<void>(0x400 + i * 0x20) : arena<void>(0x400);
                             node[2] = arena<void>(0x1000 + i * 0x100);
                             arena<uint32_t>(0x1000 + i * 0x100 + 0xc8)[0] = r.next();
                         }
                         arena<void *>(0x100)[2] = arena<void>(0x420);
                     },
                     [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(w3_watchers);
    // Refs::At: an index of 0-2, each reference looked up or kept
    static uint32_t w3_at_off;
    Test w3_at{0x510d90,
               [=](Rng &r) {
                   fresh(r); iface_handle(r);
                   for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x1000)[i] = r.next();
                   *(void **)arena<uint8_t>(0x1000 + w3_at_off) = arena<void>(0x800);
                   for (int i = 0; i < 3; i++) {
                       uint8_t *ref = arena<uint8_t>(0x800 + i * 0x1c);
                       *(void **)(ref + 4) = arena<void>(0x340);
                       *(uint32_t *)(ref + 0x14) = r.next();
                       ref[0x18] = (uint8_t)r.range(0, 1);
                   }
                   arena<int32_t>(0x10)[0] = r.range(0, 2);
               },
               [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t))fn)(arena<void>(0x1000), arena<int32_t>(0x10)[0])}; }};
    w3_at.prepare = [](const rewrite::Entry *e) {
        w3_at_off = *(uint32_t *)(e->at + 2);
        g_iface_id = *(uint32_t **)(e->at + 0x30);
    };
    quick(w3_at);
    // List::FirstInterface: 0-3 nodes at 0x800.., each with an object or none, then the end node
    Test w3_first{0x4ccf50,
                  [=](Rng &r) {
                      fresh(r); iface_handle(r);
                      static void *vt[2] = {(void *)stub_deleting, (void *)stub_class_of};
                      int n = r.range(0, 3);
                      *(void **)arena<uint8_t>(0x174) = arena<void>(0x800);
                      for (int i = 0; i <= n; i++) {
                          void **node = arena<void *>(0x800 + i * 0x20);
                          node[1] = i < n ? arena<void>(0x820 + i * 0x20) : nullptr;
                          arena<void *>(0x1000 + i * 0x100)[0] = vt;
                          node[2] = r.range(0, 1) ? arena<void>(0x1000 + i * 0x100) : nullptr;
                      }
                  },
                  [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w3_first.prepare = [](const rewrite::Entry *e) { g_iface_id = *(uint32_t **)(e->at + 0x24); };
    quick(w3_first);
    // Array::Push: full or not, empty or not, the array at the copy's own offset
    static uint32_t w3_ap_base;
    Test w3_array_push{0x80ed30,
                       [=](Rng &r) {
                           fresh(r);
                           uint8_t *array = arena<uint8_t>(0x100 + w3_ap_base);
                           *(void **)array = arena<void>(0x800);
                           *(int32_t *)(array + 4) = r.range(0, 6);
                           *(int32_t *)(array + 8) = r.range(0, 6);
                           arena<uint32_t>(0x10)[0] = r.next();
                       },
                       [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]); return Result{}; }};
    w3_array_push.prepare = [](const rewrite::Entry *e) {
        w3_ap_base = *(uint8_t *)(e->at + 0x1a);
        stubs_for_now({{(uintptr_t)(e->at + 0x1b + 5 + *(int32_t *)(e->at + 0x1c)), (void *)stub_thiscall1}});
    };
    w3_array_push.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_array_push);
    // Stream::Skip: through the two stream calls; the copies of ret 0xc are given their third argument
    static bool w3_skip3;
    Test w3_skip{0xbbdf90,
                 [=](Rng &r) { fresh(r); arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x10)[0] = r.next(); },
                 [](void *fn) {
                     if (w3_skip3) return Result{((uint32_t(__stdcall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], 7)};
                     return Result{((uint32_t(__stdcall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])};
                 }};
    w3_skip.prepare = [](const rewrite::Entry *e) {
        w3_skip3 = *(uint8_t *)(e->at + 0x1a) == 0xc;
        stubs_for_now({{0xbe40b0, (void *)w3_stub_tell}, {0xbe4090, (void *)w3_stub_seek}});
    };
    w3_skip.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_skip);
    // FreeList::Add: the globals set in the call
    static uint32_t *w3_fl_head, *w3_fl_count;
    Test w3_free_add{0xbe8940,
                     [=](Rng &r) { fresh(r); arena<uint32_t>(0x10)[0] = r.next(); arena<uint32_t>(0x10)[1] = r.next(); },
                     [](void *fn) {
                         *w3_fl_head = arena<uint32_t>(0x10)[0];
                         *w3_fl_count = arena<uint32_t>(0x10)[1];
                         return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))};
                     }};
    w3_free_add.prepare = [](const rewrite::Entry *e) {
        w3_fl_head = *(uint32_t **)(e->at + 1);
        w3_fl_count = *(uint32_t **)(e->at + 8);
    };
    quick(w3_free_add);
    // Visitor::Run: what the visitor holds
    Test w3_visit{0x6af3a0, fresh, [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w3_visit.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6ae7c0, (void *)w3_stub_visit}}); };
    w3_visit.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_visit);
    // Static::Init: the base constructor under the real builder, and the exit function
    Test w3_static{0xf04320, fresh, [](void *fn) { ((void(__cdecl *)())fn)(); return Result{}; }};
    w3_static.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbe79b0, (void *)stub_thiscall2}}); };
    w3_static.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_static);
    // Value::Describe: the owner at 0x400, its table at 0x508
    Test w3_describe{0x8b5320,
                     [=](Rng &r) {
                         fresh(r);
                         static void *vt[3] = {nullptr, nullptr, (void *)stub_thiscall2};
                         for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x1000)[i] = r.next();
                         *(void **)arena<uint8_t>(0x484) = arena<void>(0x500);
                         *(void **)arena<uint8_t>(0x508) = vt;
                         arena<uint32_t>(0x10)[0] = r.next();
                     },
                     [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x10)[0])}; }};
    w3_describe.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x833850, (void *)w3_stub_registry}}); };
    w3_describe.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_describe);
    // Block::Delete: with and without the flag, noise above it
    Test w3_block_delete{0xbd6fb0,
                         [=](Rng &r) { fresh(r); arena<uint32_t>(0x10)[0] = r.next(); },
                         [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    quick(w3_block_delete);
    // Slot::Release: a slot, none, or slot 0
    static uint32_t w3_sr_off;
    Test w3_slot_release{0x818f80,
                         [=](Rng &r) { fresh(r); arena<int32_t>(0x100 + w3_sr_off)[0] = r.range(0, 3) - 1; },
                         [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w3_slot_release.prepare = [](const rewrite::Entry *e) {
        w3_sr_off = *(uint8_t *)(e->at + 5);
        stubs_for_now({{0xcdede0, (void *)stub_thiscall1}, {0x8338b0, (void *)w3_stub_then}});
    };
    w3_slot_release.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_slot_release);
    // Part::Clear: a part or none
    static uint32_t w3_pc_off;
    Test w3_part_clear{0x4e2c10,
                       [=](Rng &r) { fresh(r); arena<void *>(0x100 + w3_pc_off)[0] = r.range(0, 1) ? arena<void>(0x800) : nullptr; },
                       [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w3_part_clear.prepare = [](const rewrite::Entry *e) {
        w3_pc_off = *(uint8_t *)(e->at + 5);
        stubs_for_now({{(uintptr_t)(e->at + 0xa + 5 + *(int32_t *)(e->at + 0xb)), (void *)stub_plain_dtor}});
    };
    w3_part_clear.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_part_clear);
    // Setting::Set: either setting, changed or not, 0-3 things to update
    Test w3_setting{0x86cfb0,
                    [=](Rng &r) {
                        fresh(r);
                        for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x1000)[i] = r.range(0, 2);
                        *(int32_t *)arena<uint8_t>(0x1108) = r.range(0, 3);
                        arena<int32_t>(0x10)[0] = r.range(0, 1);
                        arena<int32_t>(0x10)[1] = r.range(0, 2);
                    },
                    [](void *fn) { ((void(__thiscall *)(void *, int32_t, int32_t))fn)(arena<void>(0x1000), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1]); return Result{}; }};
    w3_setting.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x8699c0, (void *)stub_plain_dtor}, {0x8625d0, (void *)stub_thiscall1}}); };
    w3_setting.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_setting);
    // Flags::SetVisible: the object at the copy's own offset
    static uint32_t w3_fv_off;
    Test w3_visible{0x6a7c70,
                    [=](Rng &r) {
                        fresh(r);
                        for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x1000)[i] = (uint32_t)(uintptr_t)arena<void>(0x1800);
                        arena<void *>(0x1000 + w3_fv_off)[0] = arena<void>(0x800);
                        arena<uint32_t>(0x894)[0] = r.next();
                        arena<uint32_t>(0x1894)[0] = r.next();
                        arena<uint32_t>(0x10)[0] = (r.next() & 0xffffff00) | (uint32_t)r.range(0, 1);
                    },
                    [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x10)[0])}; }};
    w3_visible.prepare = [](const rewrite::Entry *e) { w3_fv_off = *(uint32_t *)(e->at + 8); };
    quick(w3_visible);
    // SharedSlots::Copy: each slot a shared value or none
    Test w3_slots_copy{0xc09fa0,
                       [=](Rng &r) {
                           fresh(r);
                           for (int i = 0; i < 3; i++) arena<uint16_t>(0x5a0 + i * 0x10)[0] = (uint16_t)r.next();
                           for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x800)[i] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x5a0 + r.range(0, 2) * 0x10) : 0;
                           for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x1000)[i] = r.next();
                       },
                       [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x1000), arena<void>(0x800))}; }};
    quick(w3_slots_copy);
    // Kind::Matches: the kinds of all the copies, and something else
    Test w3_matches{0x772f10,
                    [=](Rng &r) {
                        fresh(r);
                        static const int32_t kinds[] = {4, 0xe, 0x10, 5};
                        *(int32_t *)arena<uint8_t>(0x990) = kinds[r.range(0, 3)];
                        *(int32_t *)arena<uint8_t>(0x994) = r.range(1, 3);
                        *(int32_t *)arena<uint8_t>(0x17c) = kinds[r.range(1, 3)];
                    },
                    [](void *fn) { return Result{((uint8_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x800))}; }};
    quick(w3_matches);
    // Pool::Take: full or not; indices around the free list's start at 0x840, entries at 0x1000
    auto w3_pool_setup = [=](Rng &r) {
        fresh(r);
        arena<void *>(0x100)[0] = arena<void>(0x1000);
        arena<void *>(0x100)[1] = arena<void>(0x840);
        arena<int32_t>(0x100)[2] = r.range(0, 6);
        arena<int32_t>(0x100)[3] = r.range(0, 6);
        for (int i = 0; i < 0x20; i++) arena<int32_t>(0x800)[i] = r.range(0, 7);
        for (int i = 0; i < 0x50; i++) arena<uint32_t>(0x1000)[i] = r.next();
    };
    auto w3_pool_call = [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; };
    auto w3_pool_prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{(uintptr_t)(e->at + 0x1b + 5 + *(int32_t *)(e->at + 0x1c)), (void *)stub_thiscall1}});
    };
    Test w3_take36{0x40d480, w3_pool_setup, w3_pool_call};
    w3_take36.prepare = w3_pool_prepare;
    w3_take36.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_take36);
    Test w3_take8{0xcefd20, w3_pool_setup, w3_pool_call};
    w3_take8.prepare = w3_pool_prepare;
    w3_take8.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_take8);
    // Row::Draw: rows of noise
    Test w3_row{0x828fc0,
                [=](Rng &r) {
                    fresh(r);
                    for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x800)[i] = r.next();
                    for (int i = 0; i < 0x30; i++) arena<uint32_t>(0x1080)[i] = r.next();
                    *(void **)arena<uint8_t>(0x109c) = arena<void>(0x400);
                    *(void **)arena<uint8_t>(0x438) = arena<void>(0x800);
                    arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 2);
                    arena<uint32_t>(0x10)[1] = r.next();
                    arena<uint32_t>(0x10)[2] = r.next();
                },
                [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1], arena<uint32_t>(0x10)[2])}; }};
    w3_row.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x8275d0, (void *)w3_stub_eight}}); };
    w3_row.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_row);
    // Lists::DeleteAll: two lists of 0-2 values, each taking its node away
    Test w3_delete_all{0xbfa860,
                       [=](Rng &r) {
                           fresh(r);
                           static void *vt[1] = {(void *)w3_stub_drop};
                           for (int l = 0; l < 2; l++) {
                               int n = r.range(0, 2);
                               uint32_t *slot = arena<uint32_t>(0x100 + (l ? 0x1c : 4));
                               *slot = (uint32_t)(uintptr_t)arena<void>(0x800 + l * 0x100);
                               for (int i = 0; i <= n; i++) {
                                   void **node = arena<void *>(0x800 + l * 0x100 + i * 0x20);
                                   node[0] = i < n ? arena<void>(0x820 + l * 0x100 + i * 0x20) : nullptr;
                                   uint32_t *value = arena<uint32_t>(0x1000 + l * 0x200 + i * 0x40);
                                   node[2] = value;
                                   *(void **)value = vt;
                                   value[1] = (uint32_t)(uintptr_t)slot;
                                   value[2] = (uint32_t)(uintptr_t)node[0];
                               }
                           }
                       },
                       [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(w3_delete_all);
    // Named::Create: three arguments
    Test w3_named{0xcc3e60,
                  [=](Rng &r) { fresh(r); for (int i = 0; i < 3; i++) arena<uint32_t>(0x10)[i] = r.next(); },
                  [](void *fn) { uint32_t *a = arena<uint32_t>(0x10); return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), a[0], a[1], a[2])}; }};
    w3_named.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x407350, (void *)stub_thiscall1}}); };
    w3_named.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_named);
    // Reader::Create: over noise
    Test w3_reader{0xc30260,
                   [=](Rng &r) { fresh(r); for (int i = 0; i < 3; i++) arena<uint32_t>(0x10)[i] = r.next(); },
                   [](void *fn) { uint32_t *a = arena<uint32_t>(0x10); return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), a[0], a[1], a[2])}; }};
    quick(w3_reader);
    // Pick::Value: both records, converted or not
    Test w3_pick{0xbc54b0,
                 [=](Rng &r) {
                     fresh(r);
                     for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x400)[i] = r.next();
                     arena<void *>(0x100)[0] = arena<void>(0x400);
                 },
                 [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w3_pick.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x1b + 5 + *(int32_t *)(e->at + 0x1c)), (void *)w3_stub_then}}); };
    w3_pick.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_pick);
    // Three::Each: the flag with noise above its byte
    Test w3_three{0x47ca60,
                  [=](Rng &r) { fresh(r); arena<uint32_t>(0x10)[0] = r.next(); },
                  [](void *fn) { ((void(__fastcall *)(uint32_t))fn)(arena<uint32_t>(0x10)[0]); return Result{}; }};
    w3_three.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0xa + 5 + *(int32_t *)(e->at + 0xb)), (void *)w3_stub_each}}); };
    w3_three.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_three);
    // Reference::CreateAt: a new object at 0x600 over noise
    Test w3_ref_create{0x7142f0,
                       [=](Rng &r) { handles(r); for (int i = 0; i < 0x30; i++) arena<uint32_t>(0x600)[i] = r.next(); },
                       [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x600))}; }};
    quick(w3_ref_create);
    // Reference::Assign: the reference at 0x100 takes on the one at 0x180
    Test w3_ref_assign{0x71f220, handles,
                       [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x180))}; }};
    quick(w3_ref_assign);
    // Reference::ResetAt: the reference at 0x100 is at the copy's own offset of its object
    static uint32_t w3_rr_off;
    Test w3_ref_reset{0x742710, handles,
                      [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100 - w3_rr_off))}; }};
    w3_ref_reset.prepare = [](const rewrite::Entry *e) { w3_rr_off = *(uint8_t *)(e->at + 4); };
    quick(w3_ref_reset);
    // Objects::Clear: 0-3 objects, some null, or no array; the task finished or not
    Test w3_objects{0x9d0ac0,
                    [=](Rng &r) {
                        fresh(r);
                        static void *vt[1] = {(void *)stub_deleting};
                        int n = r.range(0, 3);
                        uint8_t *self = arena<uint8_t>(0x1000);
                        *(void **)(self + 0x214) = n || r.range(0, 1) ? arena<void>(0x800) : nullptr;
                        *(int32_t *)(self + 0x218) = n;
                        for (int i = 0; i < n; i++) {
                            arena<void *>(0x900 + i * 0x10)[0] = vt;
                            arena<void *>(0x800)[i] = r.range(0, 3) ? arena<void>(0x900 + i * 0x10) : nullptr;
                        }
                        arena<uint32_t>(0x42c)[0] = r.next();
                        arena<uint32_t>(0x10)[0] = (r.next() & 0xffffff00) | (uint32_t)r.range(0, 1);
                    },
                    [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x10)[0]); return Result{}; }};
    w3_objects.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x40a090, (void *)w3_stub_registry}, {0x800860, (void *)stub_plain_dtor}}); };
    w3_objects.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_objects);
    // Tree::Walk: 21 nodes at 0x1000, the last 16 without children; 0-2 items each at 0x1400..
    Test w3_tree{0xa0e240,
                 [=](Rng &r) {
                     fresh(r);
                     arena<void *>(0x100)[2] = arena<void>(0x1000);
                     for (int i = 0; i < 21; i++) {
                         uint8_t *node = arena<uint8_t>(0x1000 + i * 0x1c);
                         *(uint32_t *)node = (r.next() & ~0xfu) | (i < 5 ? (uint32_t)r.range(0, 15) : 0);
                         *(int32_t *)(node + 0xc) = r.range(0, 1);
                         *(int32_t *)(node + 0x10) = r.range(0, 2);
                         *(void **)(node + 0x18) = arena<void>(0x1400 + i * 0x40);
                         for (int j = 0; j < 2; j++) {
                             uint8_t *item = arena<uint8_t>(0x1400 + i * 0x40 + j * 0x20);
                             *(void **)(item + 0x18) = arena<void>(0x1c00 + i * 0x10 + j * 8);
                             *(uint32_t *)(item + 0x1c) = (uint32_t)r.range(0, 1);
                         }
                     }
                     arena<int32_t>(0x10)[0] = r.range(0, 1) ? 0 : r.range(1, 4);
                     arena<uint32_t>(0x10)[1] = r.next();
                 },
                 [](void *fn) { ((void(__thiscall *)(void *, int32_t, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<uint32_t>(0x10)[1]); return Result{}; }};
    w3_tree.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x40 + 5 + *(int32_t *)(e->at + 0x41)), (void *)stub_plain_dtor}}); };
    w3_tree.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_tree);
    // Com::Query: each step failing or not
    Test w3_com{0xdff1b0,
                [=](Rng &r) {
                    fresh(r);
                    static void *source_vt[7] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, (void *)w3_com_get};
                    static void *object_vt[3] = {(void *)w3_com_query, nullptr, (void *)w3_com_release};
                    *(void **)arena<uint8_t>(0x118) = arena<void>(0x380);
                    arena<void *>(0x380)[0] = source_vt;
                    arena<void *>(0x400)[0] = object_vt;
                    arena<int32_t>(0x20)[0] = r.range(0, 3) ? r.range(0, 5) : -r.range(1, 5);
                    arena<int32_t>(0x20)[1] = r.range(0, 3) ? r.range(0, 5) : -r.range(1, 5);
                    arena<uint32_t>(0x800)[0] = r.next();
                },
                [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x800))}; }};
    quick(w3_com);
    // Registry::Find: found or not, with or without the interface
    static uint32_t *w3_rf_key;
    Test w3_find{0xcb8de0,
                 [=](Rng &r) {
                     fresh(r); iface_handle(r);
                     arena<uint32_t>(0x20)[0] = r.next();
                     arena<uint32_t>(0x10)[0] = r.next();
                 },
                 [](void *fn) {
                     *w3_rf_key = arena<uint32_t>(0x10)[0];
                     return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))};
                 }};
    w3_find.prepare = [](const rewrite::Entry *e) {
        w3_rf_key = *(uint32_t **)(e->at + 4);
        g_iface_id = *(uint32_t **)(e->at + 0x3c);
        stubs_for_now({{0xbf47b0, (void *)w3_stub_key}, {0xbf5b60, (void *)w3_stub_lookup}, {0xbec8a0, (void *)w3_stub_report}});
    };
    w3_find.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_find);
    // List::PushCounted: as List::PushFront
    Test w3_push_counted{0x4ccde0, w3_push_setup, w3_push_call};
    w3_push_counted.prepare = w3_push_narrow.prepare;
    w3_push_counted.finish = w3_push_wide.finish;
    quick(w3_push_counted);
    // Array::Claim: full or not, with room for what's wanted or not
    Test w3_claim{0x7e5ee0,
                  [=](Rng &r) {
                      fresh(r);
                      arena<int32_t>(0x100)[1] = r.range(0, 6);
                      arena<int32_t>(0x100)[2] = r.range(0, 6);
                      arena<int32_t>(0x10)[0] = r.range(0, 4) - 2;
                  },
                  [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0])}; }};
    w3_claim.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x1a + 5 + *(int32_t *)(e->at + 0x1b)), (void *)stub_thiscall1}}); };
    w3_claim.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_claim);
    // Items::Forward: found or not
    Test w3_items{0x9024f0,
                  [=](Rng &r) { fresh(r); for (int i = 0; i < 5; i++) arena<uint32_t>(0x10)[i] = r.next(); arena<uint32_t>(0x20)[0] = r.next(); },
                  [](void *fn) { uint32_t *a = arena<uint32_t>(0x10); return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3], a[4])}; }};
    w3_items.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0x8fd720, (void *)w3_stub_item}, {(uintptr_t)(e->at + 0x29 + 5 + *(int32_t *)(e->at + 0x2a)), (void *)w3_stub_four}});
    };
    w3_items.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_items);
    // Chain::Run: an empty list, or a first value with 0-3 more after it
    static uint32_t w3_chain_off;
    Test w3_chain{0x4cdb90,
                  [=](Rng &r) {
                      fresh(r);
                      *(void **)arena<uint8_t>(0x100 + w3_chain_off) = arena<void>(0x400);
                      arena<void *>(0x400)[0] = r.range(0, 3) ? arena<void>(0x420) : nullptr;
                      arena<void *>(0x400)[2] = r.range(0, 4) ? arena<void>(0x800) : nullptr;
                      int n = r.range(0, 3);
                      for (int i = 0; i <= n; i++) arena<void *>(0x800 + i * 0x10)[0] = i < n ? arena<void>(0x810 + i * 0x10) : nullptr;
                  },
                  [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w3_chain.prepare = [](const rewrite::Entry *e) {
        w3_chain_off = *(uint8_t *)(e->at + 2);
        stubs_for_now({{(uintptr_t)(e->at + 0x26 + 5 + *(int32_t *)(e->at + 0x27)), (void *)stub_plain_dtor},
                       {(uintptr_t)(e->at + 0x2d + 5 + *(int32_t *)(e->at + 0x2e)), (void *)w3_stub_next}});
    };
    w3_chain.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_chain);
    // Global::CreateBlock: a block of noise or none
    Test w3_global_block{0x4097e0,
                         [=](Rng &r) {
                             fresh(r);
                             for (int i = 0; i < 16; i++) arena<uint32_t>(0x2000)[i] = r.next();
                             arena<uint32_t>(0x20)[0] = r.next();
                         },
                         [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__cdecl *)())fn)()}; }};
    w3_global_block.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfc0, (void *)w3_stub_alloc_maybe}}); };
    w3_global_block.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_global_block);
    // Member::Delete: joined or not, with the flag or not
    static uint32_t w3_md_flag;
    Test w3_member_delete{0xbd3880,
                          [=](Rng &r) {
                              fresh(r);
                              for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x1000)[i] = r.next();
                              arena<uint8_t>(0x1000 + w3_md_flag)[0] = (uint8_t)r.range(0, 2);
                              arena<uint32_t>(0x10)[0] = r.next();
                          },
                          [](void *fn) {
                              *(void **)0x13d0d84 = arena<void>(0x400);
                              return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x10)[0])};
                          }};
    w3_member_delete.prepare = [](const rewrite::Entry *e) {
        w3_md_flag = *(uint32_t *)(e->at + 5);
        stubs_for_now({{(uintptr_t)(e->at + 0x21 + 5 + *(int32_t *)(e->at + 0x22)), (void *)stub_thiscall1}});
    };
    w3_member_delete.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_member_delete);
    // Member::Create: the manager at 0x400, its list at the copy's own offset, every other at 0x900
    static uint32_t w3_mc_list;
    Test w3_member_create{0xbd37a0,
                          [=](Rng &r) {
                              fresh(r);
                              for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x1000)[i] = r.next();
                              for (uint32_t h : {0x800u, 0x900u}) {
                                  arena<void *>(h)[0] = arena<void>(h + 0x20);
                                  arena<void *>(h)[1] = arena<void>(h + 0x20);
                                  arena<void *>(h + 0x20)[0] = arena<void>(h);
                                  arena<void *>(h + 0x20)[1] = arena<void>(h);
                              }
                              for (int i = 0; i < 0x20; i++) arena<void *>(0x400)[i] = arena<void>(0x900);
                              *(void **)arena<uint8_t>(0x400 + w3_mc_list) = arena<void>(0x800);
                          },
                          [](void *fn) {
                              *(void **)0x13d0d84 = arena<void>(0x400);
                              return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x1000))};
                          }};
    w3_member_create.prepare = [](const rewrite::Entry *e) { w3_mc_list = *(uint8_t *)(e->at + 0x28); };
    quick(w3_member_create);
    // Buffers::Destroy: over noise
    Test w3_buffers{0xc46cb0, fresh, [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w3_buffers.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbce060, (void *)stub_plain_dtor}}); };
    w3_buffers.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_buffers);
    // Control::Create: three arguments
    Test w3_control{0x7e2810,
                    [=](Rng &r) {
                        fresh(r);
                        for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x1000)[i] = r.next();
                        for (int i = 0; i < 3; i++) arena<uint32_t>(0x10)[i] = r.next();
                    },
                    [](void *fn) { uint32_t *a = arena<uint32_t>(0x10); return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x1000), a[0], a[1], a[2])}; }};
    w3_control.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6aae60, (void *)w3_stub_three}}); };
    w3_control.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_control);
    // List::DeleteAll: a list of 0-2 values, each taking its node away
    Test w3_list_delete_all{0x673ac0,
                            [=](Rng &r) {
                                fresh(r);
                                static void *vt[1] = {(void *)w3_stub_drop};
                                int n = r.range(0, 2);
                                uint32_t *slot = arena<uint32_t>(0x100);
                                *slot = (uint32_t)(uintptr_t)arena<void>(0x800);
                                for (int i = 0; i <= n; i++) {
                                    void **node = arena<void *>(0x800 + i * 0x20);
                                    node[0] = i < n ? arena<void>(0x820 + i * 0x20) : nullptr;
                                    uint32_t *value = arena<uint32_t>(0x1000 + i * 0x40);
                                    node[2] = value;
                                    *(void **)value = vt;
                                    value[1] = (uint32_t)(uintptr_t)slot;
                                    value[2] = (uint32_t)(uintptr_t)node[0];
                                }
                            },
                            [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(w3_list_delete_all);
    // Stock::Take: taken or not
    Test w3_stock{0x7242c0,
                  [=](Rng &r) { fresh(r); arena<uint32_t>(0x20)[0] = r.next(); arena<uint32_t>(0x10)[0] = r.next(); },
                  [](void *fn) { return Result{((uint8_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    w3_stock.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{(uintptr_t)(e->at + 0xc + 5 + *(int32_t *)(e->at + 0xd)), (void *)w3_stub_take}, {0x723e50, (void *)stub_plain_dtor}});
    };
    w3_stock.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_stock);
    // Block::Clear: a block or none
    static uint32_t w3_bc_off;
    Test w3_block_clear{0x66a7e0,
                        [=](Rng &r) {
                            fresh(r);
                            for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x1000)[i] = r.next();
                            arena<uint32_t>(0x1000 + w3_bc_off)[0] = r.range(0, 2) ? r.next() : 0;
                        },
                        [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x1000)); return Result{}; }};
    w3_block_clear.prepare = [](const rewrite::Entry *e) { w3_bc_off = *(uint32_t *)(e->at + 5); };
    quick(w3_block_clear);
    // Resource::Release: the owner at 0x400
    Test w3_resource{0xbfbe10,
                     [=](Rng &r) {
                         fresh(r);
                         arena<void *>(0x100)[5] = arena<void>(0x400);
                         *(uint32_t *)arena<uint8_t>(0x870) = r.next();
                     },
                     [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w3_resource.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xc5cb30, (void *)stub_thiscall1}}); };
    w3_resource.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_resource);
    // Slot::Return: a slot, none, or slot 0
    Test w3_slot_return{0x5d37f0,
                        [=](Rng &r) { fresh(r); arena<int32_t>(0x100)[0xa] = r.range(0, 3) - 1; },
                        [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w3_slot_return.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x425920, (void *)w3_stub_registry}, {0x9fe2a0, (void *)stub_thiscall1}}); };
    w3_slot_return.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_slot_return);
    // Result::Make: an argument
    Test w3_make{0x48d5b0,
                 [=](Rng &r) { fresh(r); arena<uint32_t>(0x10)[0] = r.next(); },
                 [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    w3_make.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x14 + 5 + *(int32_t *)(e->at + 0x15)), (void *)w3_stub_fill}}); };
    w3_make.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_make);
    // Inner::Delete: with the flag or not
    Test w3_inner{0xdff0c0,
                  [=](Rng &r) { fresh(r); arena<uint32_t>(0x10)[0] = r.next(); },
                  [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x108), arena<uint32_t>(0x10)[0])}; }};
    w3_inner.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 3 + 5 + *(int32_t *)(e->at + 4)), (void *)stub_plain_dtor}}); };
    w3_inner.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_inner);
    // Owned::Clear: an object or none
    static uint32_t w3_oc_off;
    Test w3_owned{0x713d40,
                  [=](Rng &r) {
                      fresh(r);
                      static void *vt[1] = {(void *)stub_deleting};
                      for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x1000)[i] = r.next();
                      arena<void *>(0x800)[0] = vt;
                      arena<void *>(0x1000 + w3_oc_off)[0] = r.range(0, 2) ? arena<void>(0x800) : nullptr;
                  },
                  [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x1000)); return Result{}; }};
    w3_owned.prepare = [](const rewrite::Entry *e) { w3_oc_off = *(uint32_t *)(e->at + 5); };
    quick(w3_owned);
    // Part::Offset: three arguments
    Test w3_offset{0x7b6760,
                   [=](Rng &r) { fresh(r); for (int i = 0; i < 3; i++) arena<uint32_t>(0x10)[i] = r.next(); },
                   [](void *fn) { uint32_t *a = arena<uint32_t>(0x10); return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), a[0], a[1], a[2])}; }};
    w3_offset.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x1c + 5 + *(int32_t *)(e->at + 0x1d)), (void *)w3_stub_four_ref}}); };
    w3_offset.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_offset);
    // State::Reset: over noise
    Test w3_state{0x548cd0, fresh, [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w3_state.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 3 + 5 + *(int32_t *)(e->at + 4)), (void *)stub_plain_dtor}}); };
    w3_state.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_state);
    // Names::Index: the match at each place, or none
    Test w3_name_index{0xc47120,
                       [=](Rng &r) { fresh(r); arena<int32_t>(0x20)[0] = r.range(0, 9); },
                       [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    w3_name_index.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xde22df, (void *)w3_stub_compare_name}}); };
    w3_name_index.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_name_index);
    // Value::Map: the values of every copy, and those on either side
    Test w3_map{0xbc41b0,
                [=](Rng &r) { fresh(r); arena<int32_t>(0x10)[0] = r.range(0, 10) - 1; },
                [](void *fn) { return Result{((uint32_t(__fastcall *)(int32_t))fn)(arena<int32_t>(0x10)[0])}; }};
    quick(w3_map);
    // Refs::Sum: 0-3 references, looked up or kept, with or without an interface
    Test w3_refs_sum{0xa895b0,
                     [=](Rng &r) {
                         fresh(r); iface_handle(r);
                         for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x3c0)[i] = r.next();
                         int n = r.range(0, 3);
                         *(void **)arena<uint8_t>(0x10cc) = arena<void>(0x800);
                         *(int32_t *)arena<uint8_t>(0x10d0) = n;
                         for (int i = 0; i < n; i++) {
                             uint8_t *ref = arena<uint8_t>(0x800 + i * 0x1c);
                             *(void **)(ref + 4) = arena<void>(0x340);
                             *(void **)(ref + 0x14) = r.range(0, 2) ? arena<void>(0x368) : nullptr;
                             ref[0x18] = (uint8_t)r.range(0, 1);
                         }
                     },
                     [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x1000))}; }};
    w3_refs_sum.prepare = [](const rewrite::Entry *) { g_iface_id = (uint32_t *)0x13cde74; };
    quick(w3_refs_sum);
    // Hash::Find: 1-4 buckets of 0-3 items, the key among them or not
    Test w3_hash{0xbdd210,
                 [=](Rng &r) {
                     fresh(r);
                     int buckets = r.range(1, 4);
                     arena<uint32_t>(0x100)[1] = (uint32_t)buckets;
                     arena<void *>(0x100)[3] = arena<void>(0x800);
                     arena<void *>(0x100)[4] = arena<void>(0x900);
                     arena<void *>(0x100)[5] = arena<void>(0xa00);
                     uint32_t at = 0;
                     for (int b = 0; b <= buckets; b++) { arena<uint32_t>(0x800)[b] = at; at += (uint32_t)r.range(0, 3); }
                     for (int i = 0; i < 16; i++) arena<uint32_t>(0x900)[i] = (uint32_t)r.range(0, 3);
                     arena<uint32_t>(0x20)[0] = r.next();
                     arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 3);
                 },
                 [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    w3_hash.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbdbf70, (void *)w3_stub_hash}, {0xbdbfd0, (void *)w3_stub_same}}); };
    w3_hash.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_hash);
    // Area::Around: near each edge of maps of 0-12, the map up to date or not
    Test w3_area{0x8dcaa0,
                 [=](Rng &r) {
                     fresh(r);
                     for (int i = 0; i < 4; i++) arena<int32_t>(0x140)[i] = r.range(0, 12);
                     arena<uint32_t>(0x42c)[0] = r.next();
                     *(void **)arena<uint8_t>(0x454) = arena<void>(0x500);
                     arena<uint8_t>(0x504)[0] = (uint8_t)r.range(0, 12);
                     arena<uint8_t>(0x505)[0] = (uint8_t)r.range(0, 12);
                 },
                 [](void *fn) {
                     int32_t *o = arena<int32_t>(0x800);
                     ((void(__thiscall *)(void *, int32_t *, int32_t *, int32_t *, int32_t *))fn)(arena<void>(0x100), o, o + 1, o + 2, o + 3);
                     return Result{};
                 }};
    w3_area.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x40a090, (void *)w3_stub_registry}, {0x800860, (void *)stub_plain_dtor}}); };
    w3_area.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3_area);

    // Refs::FirstFree: 0-4 items (and a negative count), each with or without a value, with each copy's layout
    static uint32_t w3b_ff_count, w3b_ff_items, w3b_ff_stride;
    Test w3b_first_free_test{0xa97380,
                             [=](Rng &r) {
                                 fresh(r);
                                 uint8_t *self = arena<uint8_t>(0x1000);
                                 int n = r.range(0, 5) - 1;
                                 *(int32_t *)(self + w3b_ff_count) = n;
                                 *(void **)(self + w3b_ff_items) = arena<void>(0x800);
                                 for (int i = 0; i < 4; i++) {
                                     *(void **)(arena<uint8_t>(0x800) + i * w3b_ff_stride + 4) = arena<void>(0x400 + i * 0x10);
                                     arena<uint32_t>(0x40c + i * 0x10)[0] = r.range(0, 2) ? r.next() | 1 : 0;
                                 }
                             },
                             [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x1000))}; }};
    w3b_first_free_test.prepare = [](const rewrite::Entry *e) {
        w3b_ff_count = *(uint32_t *)(e->at + 2); w3b_ff_items = *(uint32_t *)(e->at + 0x10); w3b_ff_stride = *(uint8_t *)(e->at + 0x23);
    };
    quick(w3b_first_free_test);
    // Items::EachCall: 0-4 items, the call sometimes shortening the list
    static uint32_t w3b_each_items;
    Test w3b_each_test{0x83fca0,
                       [=](Rng &r) {
                           fresh(r);
                           uint8_t *self = arena<uint8_t>(0x1000);
                           *(int32_t *)(self + w3b_count_at) = r.range(0, 4);
                           *(void **)(self + w3b_each_items) = arena<void>(0x800);
                           for (int i = 0; i < 4; i++) arena<uint32_t>(0x800)[i] = r.next();
                           arena<int32_t>(0x20)[0] = r.range(0, 5);
                       },
                       [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x1000)); return Result{}; }};
    w3b_each_test.prepare = [](const rewrite::Entry *e) {
        w3b_count_at = *(uint8_t *)(e->at + 6); w3b_each_items = *(uint8_t *)(e->at + 0x12);
        stubs_for_now({{(uintptr_t)(e->at + 0x16 + 5 + *(int32_t *)(e->at + 0x17)), (void *)w3b_stub_each}});
    };
    w3b_each_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_each_test);
    // Hook::Unhook: the notice's contents and both calls
    Test w3b_unhook_test{0x6afd60, [=](Rng &r) { fresh(r); },
                         [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w3b_unhook_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6ae7c0, (void *)w3b_stub_notice}, {0x6a1e20, (void *)stub_plain_dtor}}); };
    w3b_unhook_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_unhook_test);
    // Value::InSet: every value through and past both ranges, and a value below them that wraps
    Test w3b_in_set_test{0x49b190, [=](Rng &r) { fresh(r); },
                         [](void *fn) {
                             uint32_t sum = 0;
                             for (uint32_t v = 0; v < 0x130; v++) sum = sum * 3 + (uint8_t)((uint32_t(__fastcall *)(uint32_t))fn)(v);
                             return Result{sum * 3 + (uint8_t)((uint32_t(__fastcall *)(uint32_t))fn)(0xffffffff)};
                         }};
    w3b_in_set_test.iterations = 3;
    quick(w3b_in_set_test);
    // Child::Drop: with and without a child, each copy's field
    static uint32_t w3b_child_at;
    Test w3b_drop_test{0x63b4e0,
                       [=](Rng &r) { fresh(r); *(void **)(arena<uint8_t>(0x100) + w3b_child_at) = r.range(0, 1) ? arena<void>(0x800) : nullptr; },
                       [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), 7); return Result{}; }};
    w3b_drop_test.prepare = [](const rewrite::Entry *e) {
        w3b_child_at = *(uint8_t *)(e->at + 6);
        stubs_for_now({{0x6af760, (void *)stub_plain_dtor}, {0x6a1e20, (void *)stub_plain_dtor}});
    };
    w3b_drop_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_drop_test);
    // Message::Forward: with and without a target; the message and the zeroed flag reach their calls
    static uint32_t w3b_target_at;
    Test w3b_forward_test{0xa0eb00,
                          [=](Rng &r) {
                              fresh(r);
                              *(void **)(arena<uint8_t>(0x100) + w3b_target_at) = r.range(0, 2) ? arena<void>(0x300) : nullptr;
                              arena<uint32_t>(0x24)[0] = r.next();
                          },
                          [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x24)[0]); return Result{}; }};
    w3b_forward_test.prepare = [](const rewrite::Entry *e) {
        w3b_target_at = *(uint8_t *)(e->at + 6);
        stubs_for_now({{(uintptr_t)(e->at + 0x1e + 5 + *(int32_t *)(e->at + 0x1f)), (void *)w3b_stub_build2},
                       {(uintptr_t)(e->at + 0x2a + 5 + *(int32_t *)(e->at + 0x2b)), (void *)w3b_stub_send64}});
    };
    w3b_forward_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_forward_test);
    // Message::Post: three values in order, each copy's two calls
    Test w3b_post_test{0x7b9d30,
                       [=](Rng &r) { fresh(r); for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next(); },
                       [](void *fn) {
                           uint32_t *v = arena<uint32_t>(0x20);
                           ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), v[0], v[1], v[2]);
                           return Result{};
                       }};
    w3b_post_test.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{(uintptr_t)(e->at + 0x19 + 5 + *(int32_t *)(e->at + 0x1a)), (void *)w3b_stub_build3},
                       {(uintptr_t)(e->at + 0x25 + 5 + *(int32_t *)(e->at + 0x26)), (void *)w3b_stub_send1c}});
    };
    w3b_post_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_post_test);
    // Actor::Notify: the value this object's own or not, each copy's fields and call
    static uint32_t w3b_own_at;
    Test w3b_notify_test{0x710020,
                         [=](Rng &r) {
                             fresh(r);
                             for (int i = 0; i < 0x80; i++) arena<uint32_t>(0x1000)[i] = r.next();
                             *(uint32_t *)(arena<uint8_t>(0x1000) + w3b_own_at) = (uint32_t)r.range(0, 1);
                             arena<uint32_t>(0x24)[0] = (uint32_t)r.range(0, 1);
                         },
                         [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x24)[0]); return Result{}; }};
    w3b_notify_test.prepare = [](const rewrite::Entry *e) {
        w3b_own_at = *(uint32_t *)(e->at + 0x16);
        stubs_for_now({{0x4d7e70, (void *)stub_thiscall1}, {(uintptr_t)(e->at + 0x28 + 5 + *(int32_t *)(e->at + 0x29)), (void *)stub_thiscall2}});
    };
    w3b_notify_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_notify_test);
    // Registry::Forward: the handler's object and result
    Test w3b_registry_test{0x4a0e10,
                           [=](Rng &r) { fresh(r); arena<uint32_t>(0x430)[0] = r.next(); arena<uint32_t>(0x24)[0] = r.next(); },
                           [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x1000), 1, 2)}; }};
    w3b_registry_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x42df90, (void *)w3b_stub_registry}, {0x5f3df0, (void *)w3b_stub_handler}}); };
    w3b_registry_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_registry_test);
    // Clock::StreamTime: no source, and the source failing, succeeding and returning a positive code; borrows between halves
    static uint32_t w3b_source_at;
    Test w3b_time_test{0xdf9030,
                       [=](Rng &r) {
                           fresh(r);
                           static void *vt[4] = {nullptr, nullptr, nullptr, (void *)w3b_stub_time};
                           *(void **)arena<uint8_t>(0x300) = vt;
                           *(void **)(arena<uint8_t>(0x100) + w3b_source_at) = r.range(0, 4) ? arena<void>(0x300) : nullptr;
                           arena<int32_t>(0x24)[0] = r.range(0, 2) - 1;
                           arena<uint32_t>(0x28)[0] = r.next();
                           arena<uint32_t>(0x2c)[0] = r.next();
                           arena<uint32_t>(0x800)[0] = r.next();
                           arena<uint32_t>(0x800)[1] = r.next();
                       },
                       [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x800))}; }};
    w3b_time_test.prepare = [](const rewrite::Entry *e) { w3b_source_at = *(uint8_t *)(e->at + 6); };
    quick(w3b_time_test);
    // Text::Print: texts of 0-29 letters into buffers of 0-39
    Test w3b_print_test{0xca71d0,
                        [=](Rng &r) {
                            fresh(r);
                            *(void **)(arena<uint8_t>(0x100) + 4) = arena<void>(0x300);
                            arena<uint32_t>(0x300)[0] = r.next();
                            arena<uint32_t>(0x24)[0] = (uint32_t)r.range(0, 39);
                            for (int i = 0; i < 16; i++) arena<uint32_t>(0x7f0)[i] = r.next();
                        },
                        [](void *fn) { ((void(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x800), arena<uint32_t>(0x24)[0]); return Result{}; }};
    w3b_print_test.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xddcdb5, (void *)w3b_stub_sprintf}, {0xddef30, (void *)w3b_stub_strncpy}}); };
    w3b_print_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_print_test);

    // Child::DropBoth: each of the two present or not, each copy's fields
    static uint32_t w3b_db_child, w3b_db_owned;
    Test w3b_drop_both_test{0x63a180,
                            [=](Rng &r) {
                                fresh(r);
                                static void *vt[1] = {(void *)stub_deleting};
                                *(void **)arena<uint8_t>(0x300) = vt;
                                *(void **)(arena<uint8_t>(0x100) + w3b_db_child) = r.range(0, 1) ? arena<void>(0x800) : nullptr;
                                *(void **)(arena<uint8_t>(0x100) + w3b_db_owned) = r.range(0, 1) ? arena<void>(0x300) : nullptr;
                            },
                            [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), 7); return Result{}; }};
    w3b_drop_both_test.prepare = [](const rewrite::Entry *e) {
        w3b_db_child = *(uint8_t *)(e->at + 6); w3b_db_owned = *(uint8_t *)(e->at + 0x1d);
        stubs_for_now({{0x6af760, (void *)stub_plain_dtor}, {0x6a1e20, (void *)stub_plain_dtor}});
    };
    w3b_drop_both_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_drop_both_test);
    // Task::Release: with and without the COM object; the two virtual slots tell each copy's call apart
    static uint32_t w3b_rel_worker, w3b_rel_com;
    Test w3b_release_test{0xbfc3c0,
                          [=](Rng &r) {
                              fresh(r);
                              static void *vt[6] = {nullptr, nullptr, nullptr, nullptr, (void *)w3b_stub_slot_a, (void *)w3b_stub_slot_b};
                              static void *com_vt[3] = {nullptr, nullptr, (void *)w3b_stub_com_release};
                              uint8_t *self = arena<uint8_t>(0x100);
                              *(void **)(self + 0x14) = arena<void>(0x300);
                              arena<uint32_t>(0x770)[0] = r.next();
                              *(void **)arena<uint8_t>(0x900) = vt;
                              *(void **)arena<uint8_t>(0x950) = com_vt;
                              *(void **)(self + w3b_rel_worker) = arena<void>(0x900);
                              *(void **)(self + w3b_rel_com) = r.range(0, 1) ? arena<void>(0x950) : nullptr;
                          },
                          [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w3b_release_test.prepare = [](const rewrite::Entry *e) {
        w3b_rel_worker = *(uint8_t *)(e->at + 0x17); w3b_rel_com = *(uint8_t *)(e->at + 0x26);
        stubs_for_now({{0xc5cb30, (void *)stub_thiscall1}});
    };
    w3b_release_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_release_test);
    // Map::Refresh: flags of every kind, each map missing, up to date or not
    Test w3b_refresh_test{0x54b740,
                          [=](Rng &r) {
                              fresh(r);
                              uint8_t *self = arena<uint8_t>(0x100);
                              *(void **)(self + 0x4c) = arena<void>(0x300);
                              *(void **)(self + 0x60) = arena<void>(0x320);
                              *(void **)arena<uint8_t>(0x30c) = r.range(0, 2) ? arena<void>(0x400) : nullptr;
                              *(void **)arena<uint8_t>(0x32c) = r.range(0, 2) ? arena<void>(0x440) : nullptr;
                              arena<uint8_t>(0x42c)[0] = (uint8_t)r.next();
                              arena<uint8_t>(0x46c)[0] = (uint8_t)r.next();
                          },
                          [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w3b_refresh_test.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0x800860, (void *)w3b_stub_slot_a}, {(uintptr_t)(e->at + 0x4a + 5 + *(int32_t *)(e->at + 0x4b)), (void *)stub_plain_dtor}});
    };
    w3b_refresh_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_refresh_test);
    // Job::Poll: waiting or not, failed or not, every status, each copy's states
    static uint32_t w3b_waiting; static void *w3b_query_was;
    Test w3b_poll_test{0x45e270,
                       [=](Rng &r) {
                           fresh(r);
                           for (int i = 0; i < 4; i++) arena<uint32_t>(0x300)[i] = r.next();
                           arena<uint32_t>(0x300)[1] = r.range(0, 4) ? w3b_waiting : w3b_waiting + 1;
                           arena<int32_t>(0x24)[0] = r.range(0, 3) ? 0 : 5;
                           arena<uint32_t>(0x28)[0] = (uint32_t)r.range(0, 3);
                       },
                       [](void *fn) {
                           *(void **)0x1106e0c = (void *)w3b_stub_query;
                           ((void(__thiscall *)(void *, void *))fn)(nullptr, arena<void>(0x300));
                           return Result{};
                       }};
    w3b_poll_test.prepare = [](const rewrite::Entry *e) { w3b_waiting = *(uint8_t *)(e->at + 9); w3b_query_was = *(void **)0x1106e0c; };
    w3b_poll_test.finish = [](const rewrite::Entry *) { *(void **)0x1106e0c = w3b_query_was; };
    quick(w3b_poll_test);
    // Choice::Set: both kinds, the value new or the same, each copy's fields and call
    static uint32_t w3b_ch_main, w3b_ch_other;
    Test w3b_choice_test{0x82fa30,
                         [=](Rng &r) {
                             fresh(r);
                             uint8_t *self = arena<uint8_t>(0x1000);
                             for (int i = 0; i < 0x40; i++) ((uint32_t *)self)[i] = r.next();
                             *(void **)(self + 0x9c) = arena<void>(0x800);
                             arena<uint32_t>(0x800)[0] = r.next();
                             *(uint32_t *)(self + w3b_ch_main) = (uint32_t)r.range(0, 1);
                             *(uint32_t *)(self + w3b_ch_other) = (uint32_t)r.range(0, 1);
                             arena<uint32_t>(0x24)[0] = (uint32_t)r.range(0, 2);
                             arena<uint32_t>(0x28)[0] = (uint32_t)r.range(0, 1);
                         },
                         [](void *fn) { ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x24)[0], arena<uint32_t>(0x28)[0]); return Result{}; }};
    w3b_choice_test.prepare = [](const rewrite::Entry *e) {
        w3b_ch_main = *(uint32_t *)(e->at + 0xd); w3b_ch_other = *(uint32_t *)(e->at + 0x54);
        stubs_for_now({{0x82b350, (void *)stub_plain_dtor}, {0x87eee0, (void *)stub_thiscall1},
                       {(uintptr_t)(e->at + 0x30 + 5 + *(int32_t *)(e->at + 0x31)), (void *)stub_thiscall2}});
    };
    w3b_choice_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_choice_test);
    // Tint::Apply: on and off, with junk above the byte that counts
    static void *w3b_device_was;
    Test w3b_tint_test{0x9122a0,
                       [=](Rng &r) {
                           fresh(r);
                           static void *vt[0x2d];
                           vt[0x2c] = (void *)w3b_stub_device;
                           *(void **)arena<uint8_t>(0x300) = vt;
                           arena<uint32_t>(0x24)[0] = r.next();
                           arena<uint32_t>(0x28)[0] = r.next() & 0xffffff00 | (r.range(0, 1) ? r.next() & 0xff : 0);
                       },
                       [](void *fn) {
                           *(void **)0x13d0db8 = arena<void>(0x300);
                           return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x28)[0])};
                       }};
    w3b_tint_test.prepare = [](const rewrite::Entry *) { w3b_device_was = *(void **)0x13d0db8; stubs_for_now({{0x431070, (void *)w3b_stub_table}}); };
    w3b_tint_test.finish = [](const rewrite::Entry *) { unstub_many(); *(void **)0x13d0db8 = w3b_device_was; };
    quick(w3b_tint_test);
    // Slots::FirstFlagged: seven slots, each with an interface or none, flags set or not on the two states
    Test w3b_flagged_test{0x7857c0,
                          [=](Rng &r) {
                              fresh(r); iface_handle(r);
                              arena<void *>(0x60c)[0] = nullptr;
                              for (int i = 0; i < 7; i++) *(void **)(arena<uint8_t>(0x1060) + i * 0x14) = r.range(0, 2) ? arena<void>(0x340) : arena<void>(0x600);
                              *(void **)arena<uint8_t>(0x400) = arena<void>(0x500);
                              *(void **)arena<uint8_t>(0x408) = arena<void>(0x540);
                              arena<uint32_t>(0x528)[0] = r.range(0, 1) ? r.next() : 0;
                              arena<uint32_t>(0x568)[0] = r.range(0, 1) ? r.next() : 0;
                              arena<uint32_t>(0x24)[0] = r.next();
                          },
                          [](void *fn) { return Result{(uint8_t)((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x1000))}; }};
    w3b_flagged_test.prepare = [](const rewrite::Entry *) { g_iface_id = (uint32_t *)0x1342768; stubs_for_now({{0x784030, (void *)w3b_stub_act}}); };
    w3b_flagged_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_flagged_test);
    // Child::Refresh: each copy's child, the flag cleared, both calls
    static uint32_t w3b_cr_child;
    Test w3b_child_refresh_test{0xc69560,
                                [=](Rng &r) {
                                    fresh(r);
                                    static void *vt[0x21];
                                    vt[0x20] = (void *)stub_plain_dtor;
                                    *(void **)arena<uint8_t>(0x300) = vt;
                                    for (int i = 1; i < 0x20; i++) arena<uint32_t>(0x300)[i] = r.next();
                                    *(void **)(arena<uint8_t>(0x100) + w3b_cr_child) = arena<void>(0x300);
                                },
                                [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    w3b_child_refresh_test.prepare = [](const rewrite::Entry *e) { w3b_cr_child = *(uint32_t *)(e->at + 5); stubs_for_now({{0xc66cd0, (void *)stub_plain_dtor}}); };
    w3b_child_refresh_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_child_refresh_test);
    // Text::Use: the text room or none, ending in a zero or not, grown in place or replaced
    Test w3b_text_use_test{0xc57a80,
                           [=](Rng &r) {
                               fresh(r);
                               *(void **)arena<uint8_t>(0x100) = arena<void>(0x300);
                               arena<uint32_t>(0x304)[0] = (uint32_t)r.range(0, 3);
                               arena<uint32_t>(0x308)[0] = (uint32_t)r.range(0, 4);
                               *(void **)arena<uint8_t>(0x30c) = arena<void>(0x400);
                               for (int i = 0; i < 8; i++) arena<uint8_t>(0x400)[i] = (uint8_t)(r.range(0, 1) ? r.next() | 1 : 0);
                               arena<uint32_t>(0x504)[0] = 2;
                               arena<uint32_t>(0x508)[0] = 9;
                               *(void **)arena<uint8_t>(0x50c) = arena<void>(0x600);
                               for (int i = 0; i < 2; i++) arena<uint32_t>(0x600)[i] = 0xffffffff;
                               arena<uint32_t>(0x24)[0] = r.next();
                               arena<uint32_t>(0x28)[0] = r.next();
                           },
                           [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(nullptr, arena<void>(0x100))}; }};
    w3b_text_use_test.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbd02b0, (void *)w3b_stub_grow}, {(uintptr_t)(e->at + 0x32 + 5 + *(int32_t *)(e->at + 0x33)), (void *)w3b_stub_use}});
    };
    w3b_text_use_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_text_use_test);

    // Try::Both: both kinds and others, the first or second try finding something, or neither
    Test w3b_try_test{0x4cc090,
                      [=](Rng &r) {
                          fresh(r);
                          for (int i = 0; i < 5; i++) arena<uint32_t>(0x40)[i] = r.next();
                          arena<uint32_t>(0x40)[0] = (uint32_t)r.range(0, 3);
                          arena<uint32_t>(0x24)[0] = r.next() | 1;
                          arena<int32_t>(0x28)[0] = r.range(0, 2);
                      },
                      [](void *fn) {
                          uint32_t *v = arena<uint32_t>(0x40);
                          return Result{((uint32_t(__fastcall *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(v[0], v[1], v[2], v[3], v[4])};
                      }};
    w3b_try_test.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x28 + 5 + *(int32_t *)(e->at + 0x29)), (void *)w3b_stub_try}}); };
    w3b_try_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_try_test);
    // Object::Attach: no object, and the interface found at each offset or not, each copy's id, offset and call
    Test w3b_attach_test{0x8ba370,
                         [=](Rng &r) {
                             fresh(r); iface_handle(r);
                             arena<uint32_t>(0x24)[0] = r.next();
                             arena<uint32_t>(0x28)[0] = (uint32_t)r.range(0, 3);
                         },
                         [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x1000), arena<uint32_t>(0x28)[0] ? arena<void>(0x360) : nullptr)}; }};
    w3b_attach_test.prepare = [](const rewrite::Entry *e) {
        g_iface_id = *(uint32_t **)(e->at + 0x1f);
        stubs_for_now({{0x501f80, (void *)stub_thiscall1}, {(uintptr_t)(e->at + 0x3d + 5 + *(int32_t *)(e->at + 0x3e)), (void *)w3b_stub_handler}});
    };
    w3b_attach_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_attach_test);
    // Stored::Run: every field different, so each argument is checked in its place
    Test w3b_run_test{0x4b76f0,
                      [=](Rng &r) {
                          fresh(r);
                          for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x1000)[i] = r.next();
                          *(void **)arena<uint8_t>(0x1124) = arena<void>(0x400);
                          *(void **)arena<uint8_t>(0x1128) = arena<void>(0x600);
                          arena<uint32_t>(0x53c)[0] = r.next();
                          arena<uint32_t>(0x73c)[0] = r.next();
                          arena<uint32_t>(0x24)[0] = r.next();
                      },
                      [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x1000))}; }};
    w3b_run_test.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x55 + 5 + *(int32_t *)(e->at + 0x56)), (void *)w3b_stub_run}}); };
    w3b_run_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_run_test);
    // Turn::Finish: pending or not, each copy's flag, kind and state
    static uint32_t w3b_pending_at;
    Test w3b_finish_test{0xb1fd30,
                         [=](Rng &r) {
                             fresh(r);
                             static void *vt[0x15];
                             vt[0x14] = (void *)stub_thiscall1;
                             *(void **)arena<uint8_t>(0x300) = vt;
                             arena<uint8_t>(0x100)[w3b_pending_at] = (uint8_t)(r.range(0, 1) ? r.next() | 1 : 0);
                         },
                         [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), 3); return Result{}; }};
    w3b_finish_test.prepare = [](const rewrite::Entry *e) {
        w3b_pending_at = *(uint32_t *)(e->at + 5);
        stubs_for_now({{0xb1fb50, (void *)stub_plain_dtor}, {0xb7e830, (void *)w3b_stub_message}, {0xb7a5a0, (void *)w3b_stub_queue}, {0xb47380, (void *)stub_thiscall1}});
    };
    w3b_finish_test.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(w3b_finish_test);
    // Items::DrawAll: 0-3 items (and a negative count), with and without a fade, the debug state on and off
    static void *w3b_settings_was, *w3b_device_was2;
    Test w3b_draw_test{0x974900,
                       [=](Rng &r) {
                           fresh(r);
                           static void *vt[0x2a];
                           vt[0x29] = (void *)w3b_stub_state;
                           *(void **)arena<uint8_t>(0x380) = vt;
                           *(void **)arena<uint8_t>(0x324) = arena<void>(0x340);
                           arena<uint8_t>(0x342)[0] = (uint8_t)r.range(0, 1);
                           for (int i = 0; i < 3; i++) {
                               arena<void *>(0x800)[i] = arena<void>(0x900 + i * 0x100);
                               for (int j = 0; j < 0x40; j++) arena<uint32_t>(0x900 + i * 0x100)[j] = r.next();
                               arena<uint8_t>(0x900 + i * 0x100 + 0x89)[0] = (uint8_t)(r.range(0, 1) ? r.next() | 1 : 0);
                           }
                           arena<int32_t>(0x24)[0] = r.range(0, 4) - 1;
                       },
                       [](void *fn) {
                           *(void **)0x12f018c = arena<void>(0x300);
                           *(void **)0x13d0db8 = arena<void>(0x380);
                           ((void(__thiscall *)(void *, void *, int32_t))fn)(arena<void>(0x100), arena<void>(0x800), arena<int32_t>(0x24)[0]);
                           return Result{};
                       }};
    w3b_draw_test.prepare = [](const rewrite::Entry *) {
        w3b_settings_was = *(void **)0x12f018c; w3b_device_was2 = *(void **)0x13d0db8;
        stubs_for_now({{0xcec500, (void *)w3b_stub_models}, {0xcec090, (void *)w3b_stub_model}, {0xcf0c00, (void *)w3b_stub_draw}});
    };
    w3b_draw_test.finish = [](const rewrite::Entry *) { unstub_many(); *(void **)0x12f018c = w3b_settings_was; *(void **)0x13d0db8 = w3b_device_was2; };
    quick(w3b_draw_test);

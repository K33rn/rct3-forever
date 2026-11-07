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

// Ride::PointHolderAt (forever/src/rewrite/ride_updates.cpp). The ride at arena 0x100 (its table at 0x7000, slot 0x144
// a stand-in): its holder's reference at 0x600 to an object at 0x640 (or none) whose class info gives the interface at
// +0 or +0x10 (or none), kept already or not; the object given is that one sometimes; the byte flags random. The
// second holder (+0x204) always resolves: an interface with a count of 0-3 at +0x44. The assignment notes the stack
// reference and sometimes clears the holder's flag and points it at 0x700 (resolving); the parts (0x532930) there or
// not. The class numbers (game globals) set in the call and put back after.
#if EQUIV_PART == 1
static uint32_t g_ru_row, g_ru_saved[2];
static uint32_t ru_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__thiscall ru_info(uint8_t *object) { dev_note(670, ru_at(object)); return *(uint8_t **)(object + 4); }
static void *ru_object_table[2] = {nullptr, (void *)ru_info};
static void __fastcall ru_let_go(void *i) { dev_note(671, ru_at(i)); }
static void __thiscall ru_taken(void *i, uint32_t one) { dev_note(672, ru_at(i)); dev_note(672, one); }
static void *__fastcall ru_handle_of(void *object) { dev_note(673, ru_at(object)); return arena<void>(0x7800); }
static void __thiscall ru_assign(uint8_t *holder, uint32_t *ref) {
    dev_note(674, ru_at(holder)); dev_note(674, ref[0]); dev_note(674, ru_at((void *)(uintptr_t)ref[1])); dev_note(674, ru_at((void *)(uintptr_t)ref[5]));
    dev_note(674, (uint8_t)ref[6]); dev_note(674, ref[4] == (uint32_t)(uintptr_t)ref);
    dev_note(674, *(uint32_t *)(uintptr_t)ref[3] == (uint32_t)(uintptr_t)&ref[2]);
    if (dev_next() % 2) {
        holder[0x18] = 0;
        *(void **)(arena<uint8_t>(0x600) + 0xc) = arena<void>(0x700);
    }
}
static void __fastcall ru_reference_gone(uint32_t *ref) {
    uint32_t *next = (uint32_t *)(uintptr_t)ref[2], *head = (uint32_t *)(uintptr_t)ref[3];
    head[0] = (uint32_t)(uintptr_t)next, next[1] = (uint32_t)(uintptr_t)head;   // (left again)
    dev_note(675, ref[0]);
}
static void *__thiscall ru_part(void *ride, int32_t i) { dev_note(676, (uint32_t)i); return dev_next() % 3 ? arena<uint8_t>(0x7a00) + 0x10 * i : nullptr; }
static uint32_t __fastcall ru_value(void *ride) { dev_note(677, ru_at(ride)); return dev_next(); }
static void __thiscall ru_give(void *part, uint32_t value) { dev_note(678, ru_at(part)); dev_note(678, value); }
static void __fastcall ru_slot(void *ride) { dev_note(679, ru_at(ride)); }
static void *ru_ride_table[0x144 / 4 + 1];
static void ru_object(uint32_t o, uint32_t entry_off, bool has) {   // an object whose class info gives its interface at +entry_off
    uint8_t *object = arena<uint8_t>(o), *info = arena<uint8_t>(o + 0x40);
    *(void **)object = ru_object_table, *(void **)(object + 4) = info;
    *(int32_t *)(info + 0x18) = 1, *(void **)(info + 0x14) = info + 0x20;
    *(void **)(info + 0x20) = has ? info + 0x28 : nullptr;
    *(int32_t *)(info + 0x28 + 0x10) = (int32_t)entry_off;
}
#else
    Test ride_updates{0x5365b0,
                      [=](Rng &r) {
                          fresh(r);
                          arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next();
                          ru_ride_table[0x144 / 4] = (void *)ru_slot;
                          *(void **)arena<void>(0x100) = ru_ride_table;
                          const uint32_t holder_at = g_ru_row ? 0x1e8 : 0x1cc;
                          uint8_t *holder = arena<uint8_t>(0x100) + holder_at;
                          *(void **)(holder + 4) = arena<void>(0x600);
                          *(void **)(arena<uint8_t>(0x600) + 0xc) = r.range(0, 4) ? arena<void>(0x640) : nullptr;
                          ru_object(0x640, r.range(0, 1) ? 0 : 0x10, r.range(0, 3) != 0);
                          ru_object(0x700, 0, true);
                          holder[0x18] = (uint8_t)(r.range(0, 3) == 0);
                          if (holder[0x18]) *(void **)(holder + 0x14) = r.range(0, 1) ? arena<void>(0x640) : nullptr;
                          arena<uint8_t>(0x100)[0x2b4] = (uint8_t)r.range(0, 1), arena<uint8_t>(0x100)[0x2b5] = (uint8_t)r.range(0, 1);
                          uint8_t *second = arena<uint8_t>(0x100) + 0x204;   // (always resolves)
                          *(void **)(second + 4) = arena<void>(0x680);
                          *(void **)(arena<uint8_t>(0x680) + 0xc) = arena<void>(0x780);
                          ru_object(0x780, 0, true);
                          *(int32_t *)(arena<uint8_t>(0x780) + 0x44) = r.range(0, 3);
                          second[0x18] = 0;
                          arena<uint32_t>(0x7800)[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x7880);
                          arena<uint32_t>(0x7880)[0] = (uint32_t)(uintptr_t)arena<void>(0x7890);
                          arena<uint32_t>(0x7890)[1] = (uint32_t)(uintptr_t)arena<void>(0x7880);
                          arena<uint32_t>(0x50)[0] = r.range(0, 2) ? 0x640 : 0x7700;   // the object given
                      },
                      [](void *fn) {
                          *(int32_t *)(uintptr_t)0x1310f90 = 0, *(int32_t *)(uintptr_t)0x12f5338 = 0;
                          ((void(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), nullptr, arena<void>(arena<uint32_t>(0x50)[0]));
                          return Result{*arena<uint32_t>(0x20)};
                      }};
    ride_updates.iterations = 1000;
    ride_updates.prepare = [](const rewrite::Entry *e) {
        g_ru_row = e->at == 0x536880;
        g_ru_saved[0] = *(uint32_t *)(uintptr_t)0x1310f90, g_ru_saved[1] = *(uint32_t *)(uintptr_t)0x12f5338;
        stubs_for_now({{0x880f60, (void *)ru_let_go}, {0x8826f0, (void *)ru_taken}, {0x805700, (void *)ru_handle_of}, {0x532a80, (void *)ru_assign},
                       {0x40f730, (void *)ru_reference_gone}, {0x532930, (void *)ru_part}, {0x4aa3e0, (void *)ru_value}, {0x737210, (void *)ru_give}});
    };
    ride_updates.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)0x1310f90 = g_ru_saved[0], *(uint32_t *)(uintptr_t)0x12f5338 = g_ru_saved[1];
    };
    quick(ride_updates);
#endif

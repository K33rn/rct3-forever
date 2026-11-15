// TrackedRide::Interface (forever/src/rewrite/ride_checks.cpp). The ride at arena 0x100; its holder's reference at
// 0x600 to an object at 0x640 (or none) whose class info (0x680) gives the interface or not; kept already or not.
// When there is none: the maker's stand-ins; the holder's assignment notes the stack reference (fields, its own owner,
// first in its list) and sometimes clears the holder's flag and points it at an object that resolves (0x700); the
// reference's destruction leaves its list again.
#if EQUIV_PART == 1
static const uint32_t rc_rows[][4] = {
#define RIDE_CHECKS(at, flag, holder, value, ...) {at, flag, holder, value},
#include "../../../forever/src/rewrite/ride_checks.inc"
#undef RIDE_CHECKS
};
static uint32_t g_rc_row, g_rc_saved;
static uint32_t rc_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__thiscall rc_info(uint8_t *object) { dev_note(550, rc_at(object)); return *(uint8_t **)(object + 4); }
static void *rc_table[2] = {nullptr, (void *)rc_info};
static void __thiscall rc_maker(void *maker, uint32_t value) { dev_note(551, value); }
static void *__thiscall rc_make(void *maker, void *ride) { dev_note(552, rc_at(ride)); return arena<void>(0x7700); }
static void *__fastcall rc_handle_of(void *object) { dev_note(553, rc_at(object)); return arena<void>(0x7800); }
static void __thiscall rc_assign(uint8_t *holder, uint32_t *ref) {
    dev_note(554, rc_at(holder)); dev_note(554, ref[0]); dev_note(554, rc_at((void *)(uintptr_t)ref[1])); dev_note(554, rc_at((void *)(uintptr_t)ref[5]));
    dev_note(554, (uint8_t)ref[6]); dev_note(554, ref[4] == (uint32_t)(uintptr_t)ref);
    dev_note(554, *(uint32_t *)(uintptr_t)ref[3] == (uint32_t)(uintptr_t)&ref[2]);
    if (dev_next() % 2) {
        holder[0x18] = 0;
        *(void **)(arena<uint8_t>(0x600) + 0xc) = arena<void>(0x700);
    }
}
static void __fastcall rc_reference_gone(uint32_t *ref) {
    uint32_t *next = (uint32_t *)(uintptr_t)ref[2], *head = (uint32_t *)(uintptr_t)ref[3];
    head[0] = (uint32_t)(uintptr_t)next, next[1] = (uint32_t)(uintptr_t)head;   // (left again)
    dev_note(555, ref[0]);
}
static void __fastcall rc_maker_gone(void *maker) { dev_note(556, 1); }
#else
    Test ride_checks{0x5c6e30,
                     [=](Rng &r) {
                         fresh(r);
                         arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next();
                         const uint32_t *row = rc_rows[g_rc_row];
                         uint8_t *holder = arena<uint8_t>(0x100) + row[2];
                         *(void **)(holder + 4) = arena<void>(0x600);
                         *(void **)(arena<uint8_t>(0x600) + 0xc) = r.range(0, 2) ? arena<void>(0x640) : nullptr;
                         for (uint32_t o : {0x640u, 0x700u}) {
                             uint8_t *object = arena<uint8_t>(o), *info = arena<uint8_t>(o + 0x40);
                             *(void **)object = rc_table, *(void **)(object + 4) = info;
                             *(int32_t *)(info + 0x18) = 1, *(void **)(info + 0x14) = info + 0x20;
                             *(void **)(info + 0x20) = o == 0x700 || r.range(0, 1) ? info + 0x28 : nullptr;
                             *(int32_t *)(info + 0x28 + 0x10) = 0x10;
                         }
                         holder[0x18] = (uint8_t)(r.range(0, 3) == 0);
                         if (holder[0x18]) *(void **)(holder + 0x14) = r.range(0, 1) ? arena<void>(0x650) : nullptr;
                         arena<uint32_t>(0x7800)[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x7880);
                         arena<uint32_t>(0x7880)[0] = (uint32_t)(uintptr_t)arena<void>(0x7890);
                         arena<uint32_t>(0x7890)[1] = (uint32_t)(uintptr_t)arena<void>(0x7880);
                     },
                     [](void *fn) {
                         *(int32_t *)(uintptr_t)0x12f5608 = 0;
                         void *back = ((void *(__fastcall *)(void *))fn)(arena<void>(0x100));
                         return Result{rc_at(back) ^ *arena<uint32_t>(0x20)};
                     }};
    ride_checks.iterations = 1000;
    ride_checks.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 2; k++)
            if (rc_rows[k][0] == e->at) g_rc_row = k;
        g_rc_saved = *(uint32_t *)(uintptr_t)0x12f5608;
        stubs_for_now({{0x5bde60, (void *)rc_maker}, {0x5bdec0, (void *)rc_make}, {0x805700, (void *)rc_handle_of}, {0x4e6410, (void *)rc_assign},
                       {0x40f730, (void *)rc_reference_gone}, {0x4882d0, (void *)rc_maker_gone}});
    };
    ride_checks.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)0x12f5608 = g_rc_saved;
    };
    quick(ride_checks);
#endif

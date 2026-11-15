// ObjectArray::Grow (forever/src/rewrite/object_arrays.cpp). The array at arena 0x100 {objects, map, used, count}:
// none yet, or an old block (with its header) and map at 0x2000..; n below, at or above the count. The copy's
// constructor, destructor and copy are stand-ins noting each object (as an offset into the arena); the runtime's
// vector iterators run as the game's own. Memory of its own from 0x6000, filled with junk.
#if EQUIV_PART == 1
static const uint32_t oa_rows[][7] = {
#define OBJECT_ARRAYS(at, size, header, dtor, ctor, copy, back, ...) {at, size, header, dtor, ctor, copy, back},
#include "../../../forever/src/rewrite/object_arrays.inc"
#undef OBJECT_ARRAYS
};
static uint32_t g_oa_row;
static uint32_t oa_at(const void *p) { return (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)); }
static void *__cdecl oa_alloc(uint32_t size) {
    dev_note(380, size);
    uint32_t &next = *arena<uint32_t>(0x30);
    void *p = arena<void>(next);
    next += (size + 15) & ~15u;
    return p;
}
static void *__fastcall oa_ctor(uint32_t *object) { dev_note(381, oa_at(object)); object[0] = 0xc7; return object; }
static void __fastcall oa_dtor(uint32_t *object) { dev_note(382, oa_at(object)); dev_note(382, object[0]); }
static void *__thiscall oa_copy(uint32_t *to, const uint32_t *from) { dev_note(383, oa_at(to)); dev_note(383, oa_at(from)); to[1] = from[1]; return to; }
#else
    Test object_arrays{0x40c6a0,
                       [=](Rng &r) {
                           fresh(r);
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next();
                           arena<uint32_t>(0x30)[0] = 0x6000;
                           for (int i = 0; i < 0x1000; i++) arena<uint32_t>(0x6000)[i] = r.next();
                           const uint32_t *row = oa_rows[g_oa_row];
                           uint32_t *self = arena<uint32_t>(0x100);
                           const int32_t count = r.range(0, 3) ? r.range(0, 6) : 0;
                           if (count) {
                               uint8_t *block = arena<uint8_t>(0x2000);
                               *(int32_t *)block = count;
                               for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x2000 + 4)[i] = r.next();
                               self[0] = (uint32_t)(uintptr_t)(block + row[2]);
                               self[1] = (uint32_t)(uintptr_t)arena<void>(0x2800);
                               for (int i = 0; i < 8; i++) arena<int32_t>(0x2800)[i] = r.range(0, 9);
                           } else {
                               self[0] = 0, self[1] = 0;
                           }
                           self[2] = (uint32_t)r.range(0, count), self[3] = (uint32_t)count;
                           arena<int32_t>(0x40)[0] = r.range(0, 4) ? count + r.range(1, 6) : r.range(-1, count);
                       },
                       [](void *fn) {
                           ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0]);
                           return Result{*arena<uint32_t>(0x20)};
                       }};
    object_arrays.iterations = 1000;
    object_arrays.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 4; k++)
            if (oa_rows[k][0] == e->at) g_oa_row = k;
        const uint32_t *row = oa_rows[g_oa_row];
        stubs_for_now({{0xbcdfd0, (void *)oa_alloc}, {0xbce010, (void *)cheat_free_aligned}, {row[4], (void *)oa_ctor}, {row[3], (void *)oa_dtor},
                       {row[5], (void *)oa_copy}});
    };
    object_arrays.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(object_arrays);
#endif

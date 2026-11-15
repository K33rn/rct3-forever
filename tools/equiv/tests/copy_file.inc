// Files::Copy (forever/src/rewrite/copy_file.cpp). Paths are real texts (dn_); the files are stand-ins {open, where,
// size, which}: open or not (arena 0x50 for reading, 0x54 for writing), a size from arena 0x58, and a read that
// sometimes moves on less than it was asked (0x5C).
#if EQUIV_PART == 1
static void __thiscall cf_open(uint32_t *file, DnText **path, uint32_t writing) {
    dev_note(210, dn_hash(*path)); dev_note(210, writing);
    file[0] = arena<uint32_t>(0x50)[writing ? 1 : 0], file[1] = 0, file[2] = arena<uint32_t>(0x58)[0], file[3] = writing;
}
static uint8_t __fastcall cf_is_open(uint32_t *file) { dev_note(211, file[3]); return (uint8_t)file[0]; }
static int32_t __fastcall cf_where(uint32_t *file) { dev_note(212, file[1]); return (int32_t)file[1]; }
static int32_t __fastcall cf_size(uint32_t *file) { dev_note(213, file[2]); return (int32_t)file[2]; }
static void __thiscall cf_read(uint32_t *file, uint8_t *buffer, int32_t n) {
    dev_note(214, (uint32_t)n);
    for (int32_t k = 0; k < n && k < 0x400; k++) buffer[k] = (uint8_t)(file[1] + k * 7);
    file[1] += (arena<uint32_t>(0x5c)[0] & 1) && n > 1 ? n / 2 : n;
}
static void __thiscall cf_write(uint32_t *file, const uint8_t *buffer, int32_t n) {
    dev_note(215, (uint32_t)n); dev_note(215, file[3]);
    uint32_t h = 0;
    for (int32_t k = 0; k < n && k < 0x400; k++) h = h * 31 + buffer[k];
    dev_note(215, h);
}
static void __fastcall cf_close(uint32_t *file) { dev_note(216, file[3]); }
static void __fastcall cf_end(uint32_t *file) { dev_note(217, file[3]); }
#else
    Test copy_file{0x458840,
                   [=](Rng &r) {
                       fresh(r);
                       uint16_t a[] = {'a', ':', '/', 'x'}, b[] = {'b', '/', 'y'};
                       arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                       *arena<DnText *>(0x40) = dn_make((DnText *)dev_take(0x10), a, 4);
                       *arena<DnText *>(0x44) = dn_make((DnText *)dev_take(0x10), b, (uint32_t)r.range(0, 3));
                       arena<uint32_t>(0x50)[0] = r.range(0, 3) ? 1 + (r.next() & 0xff00) : r.next() & 0xff00;   // (only the low byte counts)
                       arena<uint32_t>(0x50)[1] = r.range(0, 3) ? 1 : 0;
                       const int32_t sizes[] = {0, 1, 0x3ff, 0x400, 0x401, 0x800, -5};
                       arena<int32_t>(0x58)[0] = r.range(0, 2) ? sizes[r.range(0, 6)] : r.range(0, 5000);
                       arena<uint32_t>(0x5c)[0] = r.next();
                   },
                   [](void *fn) {
                       ((void(__fastcall *)(void *, void *))fn)(arena<void>(0x40), arena<void>(0x44));
                       return Result{*arena<uint32_t>(0x20)};
                   }};
    copy_file.iterations = 1000;
    copy_file.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbd01c0, (void *)dn_copy}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbe5fd0, (void *)cf_open},
                       {0xbe40d0, (void *)cf_is_open}, {0xbe40b0, (void *)cf_where}, {0xbe4030, (void *)cf_size}, {0xbe5020, (void *)cf_read},
                       {0xbe4060, (void *)cf_write}, {0xbe5360, (void *)cf_close}, {0xbe4f30, (void *)cf_end}});
    };
    copy_file.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(copy_file);
#endif

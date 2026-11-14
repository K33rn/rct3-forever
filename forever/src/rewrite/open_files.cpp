// LaserDesigner::OpenFiles and WaterJetDesigner::OpenFiles: one function written out twice (tools/family.py:
// open_files.inc; they differ in where the designer keeps its folder's name, the folder ("Lasers", ...), the file
// window's size and its constructor, and two functions the window is given to). Given a value (kept at +0xE4):
//   - the designer's folder name, when empty (compared with ""), let go and made the copy's folder;
//   - the file window made (none without memory; the window's constructor with 0, 0 and the designer's +0xDC);
//   - laid out (0x6B1330's 0x6B1640) beside the designer: relation 6, the rectangle {0, 0, and the size its slot 8
//     gives};
//   - given the value (p4) and built (p5), kept at +0xE0, and the designer told (its slot 0xE4/4).
#include "rewrite.h"

namespace {
struct OpenFile {
    uint32_t at, folder_at, folder, size, make, give, build;
};
#define OPEN_FILES(at, folder_at, folder, size, make, give, build, ...) {at, folder_at, folder, size, make, give, build},
constexpr OpenFile open_files[] = {
#include "open_files.inc"
};
#undef OPEN_FILES
#define OPEN_FILES(at, folder_at, folder, size, make, give, build, ...) {__VA_ARGS__},
const uint8_t open_files_first[][7] = {
#include "open_files.inc"
};
#undef OPEN_FILES

typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc = (Alloc)0xbcdfc0;
inline void free_text(int32_t *text) {
    ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
    ((Free)0xbce000)(text);
}
inline int32_t *text_c(uintptr_t c_text) {
    void *memory = alloc(0x10);
    return memory ? ((int32_t * (__thiscall *)(void *, uintptr_t))0xbcf850)(memory, c_text) : nullptr;
}

volatile long g_open_files;
void open_file(uint8_t *self, uint32_t value, const OpenFile &kind) {
    g_open_files++;
    *(volatile uint32_t *)(self + 0xe4) = value;
    int32_t *volatile *folder = (int32_t *volatile *)(self + kind.folder_at);
    {
        int32_t *volatile empty = text_c(0xf264ad);   // ""
        const bool none = ((int32_t(__thiscall *)(void *, void *))0xbce970)((void *)folder, (void *)&empty) == 0;
        --*empty;
        int32_t *again = empty;   // read again, as the game does
        if (*(volatile int32_t *)again == 0) free_text(again);
        if (none) {
            --**folder;
            uintptr_t old = (uintptr_t)*folder;
            if (*(volatile int32_t *)old == 0 && old) free_text((int32_t *)old);
            *folder = text_c(kind.folder);
        }
    }
    void *memory = alloc(kind.size);
    uint8_t *window = memory ? (uint8_t *)((void *(__thiscall *)(void *, uint32_t, uint32_t, void *))kind.make)(memory, 0, 0, self + 0xdc) : nullptr;
    uint8_t scratch[0x10];
    const uint32_t *size = (const uint32_t *)((void *(__thiscall *)(void *, void *))(*(void ***)window)[0x20 / 4])(window, scratch);
    uint32_t rect[4];
    rect[0] = 0, rect[1] = 0, rect[2] = size[0], rect[3] = size[1];
    ((void(__thiscall *)(void *, uint32_t, void *, uint32_t *))0x6b1640)(window, 6, self, rect);
    ((void(__thiscall *)(void *, uint32_t))kind.give)(window, value);
    ((void(__thiscall *)(void *))kind.build)(window);
    *(uint8_t *volatile *)(self + 0xe0) = window;
    ((void(__thiscall *)(void *))(*(void ***)self)[0xe4 / 4])(self);
}

template <int K>
void __thiscall open_file_copy(uint8_t *self, uint32_t value) {
    open_file(self, value, open_files[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(open_files[0].at == 0xad6460 && open_files[1].at == 0xaef810, "open_files.inc changed");
REWRITE("Designer::OpenFiles", 0xad6460, open_files_first[0], (&open_file_copy<0>), &g_open_files);
REWRITE("Designer::OpenFiles", 0xaef810, open_files_first[1], (&open_file_copy<1>), &g_open_files);

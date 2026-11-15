// Window::Make458840, Window::Make479850 and Window::MakeA3B690 (one function written out three times; they differ
// only in their exception handler): copy a file (fastcall: the destination's path, the source's path, both texts).
//   - The source opened to read (a copy of its path given to the file's opening 0xBE5FD0 with 0; the copy let go).
//   - When it is open (0xBE40D0): the destination opened the same way to write (with 1); when that is open, the
//     source copied over in pieces of 0x400 bytes (fewer for the last: its size 0xBE4030 less where it is 0xBE40B0,
//     both asked again each time) read (0xBE5020) into a buffer and written (0xBE4060), for as long as where it is
//     stays below its size; then the destination closed (0xBE5360) and the source.
//   - The files' ends (0xBE4F30): the destination's when it was made, then the source's.
// (The game's stack cookie check guards only its own frame.)
#include "rewrite.h"

namespace {
typedef void(__cdecl *Free)(void *);
inline void let_go(int32_t *volatile *held) {   // a shared text let go (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
typedef int32_t(__thiscall *Ask)(void *);
inline int32_t where(void *file) { return ((Ask)0xbe40b0)(file); }
inline int32_t size(void *file) { return ((Ask)0xbe4030)(file); }
inline bool is_open(void *file) { return ((uint8_t(__thiscall *)(void *))0xbe40d0)(file) != 0; }
inline void call(uintptr_t fn, void *file) { ((void(__thiscall *)(void *))fn)(file); }
void open(void *file, void *path, uint32_t writing) {
    int32_t *volatile copy;
    ((void *(__thiscall *)(void *, void *))0xbd01c0)((void *)&copy, path);
    ((void(__thiscall *)(void *, void *, uint32_t))0xbe5fd0)(file, (void *)&copy, writing);
    let_go(&copy);
}

volatile long g_copy_files;
void __fastcall copy_file(void *to_path, void *from_path) {
    g_copy_files++;
    alignas(4) uint8_t from[16], to[16];
    open(from, from_path, 0);
    if (is_open(from)) {
        open(to, to_path, 1);
        if (is_open(to)) {
            // (always where it is first, then its size)
            auto below = [&] { const int32_t at = where(from); return at < size(from); };
            auto left = [&] { const int32_t at = where(from); return size(from) - at; };
            if (below()) {
                uint8_t buffer[0x400];
                do {
                    int32_t n = 0x400;
                    if (left() < n) n = left();
                    ((void(__thiscall *)(void *, void *, int32_t))0xbe5020)(from, buffer, n);
                    ((void(__thiscall *)(void *, void *, int32_t))0xbe4060)(to, buffer, n);
                } while (below());
            }
            call(0xbe5360, to);
        }
        call(0xbe5360, from);
        call(0xbe4f30, to);
    }
    call(0xbe4f30, from);
}

const uint8_t copy_files_first[][7] = {{0x6a, 0xff, 0x68, 0xec, 0xad, 0xe0, 0x00}, {0x6a, 0xff, 0x68, 0xac, 0xd7, 0xe0, 0x00}, {0x6a, 0xff, 0x68, 0xfc, 0xe2, 0xe7, 0x00}};
}   // namespace
REWRITE("Files::Copy", 0x458840, copy_files_first[0], &copy_file, &g_copy_files);
REWRITE("Files::Copy", 0x479850, copy_files_first[1], &copy_file, &g_copy_files);
REWRITE("Files::Copy", 0xa3b690, copy_files_first[2], &copy_file, &g_copy_files);

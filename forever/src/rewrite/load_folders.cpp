// Paths::LoadFolder, Queues::LoadFolder, Pools::LoadFolder, Fences::LoadFolder: one function written out four times
// (tools/family.py checks they differ only in the three parameters in load_folders.inc). It clears what is loaded
// (p0, on the object), then walks the files of its folder (p1: "Path\", "Queue\", "Pool\", "Enclosures\Fences\")
// matching "*": for each entry that is a file and not "." or "..", its name is copied and loaded (p2, on the object).
// The folder walker (0xBE5570, made from the folder and the pattern) holds the walk at +4: its slot 1 says whether
// there is an entry, slot 4 whether it is a file, slot 2 moves on; 0x4042D0 gives the entry's name.
#include "rewrite.h"

namespace {
struct LoadFolder {
    uint32_t at, clear, folder, load;
};
#define LOAD_FOLDERS(at, clear, folder, load, ...) {at, clear, folder, load},
constexpr LoadFolder load_folders[] = {
#include "load_folders.inc"
};
#undef LOAD_FOLDERS
#define LOAD_FOLDERS(at, clear, folder, load, ...) {__VA_ARGS__},
const uint8_t load_folders_first[][7] = {
#include "load_folders.inc"
};
#undef LOAD_FOLDERS

typedef void(__cdecl *Free)(void *);
// a shared text {count, ..., buffer at +0xC} let go: the count taken down; at 0 its buffer freed, then it
inline void let_go(int32_t *volatile *held) {
    --**held;
    int32_t *text = *held;   // read again, as the game does
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
// a text made from C text on 16 new bytes (none without memory)
inline int32_t *text_c(uintptr_t c_text) {
    void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
    return memory ? ((int32_t * (__thiscall *)(void *, uintptr_t))0xbcfb10)(memory, c_text) : nullptr;
}
// the walk's slot n, on what the walker holds at +4 (read each time)
inline bool walk(uint32_t *walker, int slot) {
    void *walk = (void *)(uintptr_t) * (volatile uint32_t *)&walker[1];
    return ((uint8_t(__thiscall *)(void *))(*(void ***)walk)[slot])(walk) != 0;
}
inline int32_t **entry_name(uint32_t *walker, int32_t *volatile *into) {
    return ((int32_t * *(__thiscall *)(void *, void *))0x4042d0)(walker, (void *)into);
}
// whether the name is the C text (String::Compare gives 0)
inline bool is(int32_t **name, uintptr_t c_text) {
    int32_t *volatile text = text_c(c_text);
    int32_t difference = ((int32_t(__thiscall *)(void *, void *))0xbcf040)(name, (void *)&text);
    let_go(&text);
    return difference == 0;
}

volatile long g_load_folders;
void load_folder(uint8_t *self, const LoadFolder &kind) {
    g_load_folders++;
    ((void(__thiscall *)(void *))kind.clear)(self);
    int32_t *volatile pattern = text_c(0xf2e7a4);   // "*"
    int32_t *volatile folder = text_c(kind.folder);
    uint32_t walker[2];
    ((void *(__thiscall *)(void *, void *, void *))0xbe5570)(walker, (void *)&folder, (void *)&pattern);
    let_go(&folder);
    let_go(&pattern);
    if (walk(walker, 1)) {
        do {
            if (!walk(walker, 4)) continue;   // (on to the next entry)
            int32_t *volatile first, *volatile second;
            bool keep = false, have_second = false;
            if (!is(entry_name(walker, &first), 0xf27994)) {   // "."
                have_second = true;
                keep = !is(entry_name(walker, &second), 0xf27840);   // ".."
            }
            if (have_second) let_go(&second);
            let_go(&first);
            if (keep) {
                int32_t *volatile name, *volatile copy;
                int32_t **given = entry_name(walker, &name);
                ((void *(__thiscall *)(void *, void *))0xbcfef0)((void *)&copy, given);
                ((void(__thiscall *)(void *, void *))kind.load)(self, (void *)&copy);
                let_go(&copy);
                let_go(&name);
            }
        } while ((walk(walker, 2), walk(walker, 1)));
    }
    ((void(__thiscall *)(void *))0xbe47b0)(walker);
}

template <int K>
void __thiscall load_folder_copy(uint8_t *self) {
    load_folder(self, load_folders[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(load_folders[0].at == 0x731e10 && load_folders[1].at == 0x73a660 && load_folders[2].at == 0xac3a80 && load_folders[3].at == 0xba5a90,
              "load_folders.inc changed");
REWRITE("Folder::LoadFiles", 0x731e10, load_folders_first[0], (&load_folder_copy<0>), &g_load_folders);
REWRITE("Folder::LoadFiles", 0x73a660, load_folders_first[1], (&load_folder_copy<1>), &g_load_folders);
REWRITE("Folder::LoadFiles", 0xac3a80, load_folders_first[2], (&load_folder_copy<2>), &g_load_folders);
REWRITE("Folder::LoadFiles", 0xba5a90, load_folders_first[3], (&load_folder_copy<3>), &g_load_folders);

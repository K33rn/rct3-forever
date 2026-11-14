// Record::Make458170, 4583A0 and 4585D0: one function written out three times (tools/family.py: folder_paths.inc; they
// differ in the special folder asked for (0xD, 0x27, 0xE: My Music, My Pictures, My Videos), the folder under My
// Documents to use instead ("\My Music", ...), and the function to fall back on). Given where the path goes (ecx): the
// special folder's path (0x458010); when that is empty, My Documents (0x458130: special folder 5) with the folder under
// it, and when that path is not there (0xBE45E0), the fallback's path instead.
#include "rewrite.h"

namespace {
struct FolderPath {
    uint32_t at, folder, under_documents, fallback;
};
#define FOLDER_PATHS(at, folder, under_documents, fallback, ...) {at, folder, under_documents, fallback},
constexpr FolderPath folder_paths[] = {
#include "folder_paths.inc"
};
#undef FOLDER_PATHS
#define FOLDER_PATHS(at, folder, under_documents, fallback, ...) {__VA_ARGS__},
const uint8_t folder_paths_first[][7] = {
#include "folder_paths.inc"
};
#undef FOLDER_PATHS

typedef void(__cdecl *Free)(void *);
inline void free_text(int32_t *text) {
    ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
    ((Free)0xbce000)(text);
}
// a shared text {count, ..., buffer at +0xC} let go: the count taken down; at 0 its buffer freed, then it
inline void let_go(int32_t *volatile *held) {
    --**held;
    int32_t *text = *held;   // read again, as the game does
    if (*(volatile int32_t *)text == 0) free_text(text);
}

volatile long g_folder_paths;
int32_t **folder_path(int32_t **out, const FolderPath &kind) {
    g_folder_paths++;
    int32_t *volatile path;
    ((void *(__fastcall *)(void *, uint32_t))0x458010)((void *)&path, kind.folder);
    void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
    int32_t *volatile empty = memory ? ((int32_t * (__thiscall *)(void *, uintptr_t))0xbcf850)(memory, 0xf264ad) : nullptr;   // ""
    const bool none = ((int32_t(__thiscall *)(void *, void *))0xbce970)((void *)&path, (void *)&empty) == 0;
    let_go(&empty);
    if (none) {
        int32_t *volatile documents, *volatile joined;
        void *made_documents = ((void *(__thiscall *)(void *))0x458130)((void *)&documents);
        int32_t **made = (int32_t **)((void *(__thiscall *)(void *, void *, uintptr_t))0x403be0)(made_documents, (void *)&joined, kind.under_documents);
        if (*(int32_t *volatile *)made != path) {   // the path made the joined one (the old let go)
            --*path;
            if (*(volatile int32_t *)path == 0) free_text(path);
            path = *(int32_t *volatile *)made;
            ++*path;
        }
        let_go(&joined);
        let_go(&documents);
        int32_t *volatile copy;
        ((void *(__thiscall *)(void *, void *))0xbd01c0)((void *)&copy, (void *)&path);
        const bool there = ((uint8_t(__thiscall *)(void *))0xbe45e0)((void *)&copy) != 0;
        let_go(&copy);
        if (!there) {
            ((void *(__thiscall *)(void *))(uintptr_t)kind.fallback)(out);
            let_go(&path);
            return out;
        }
    }
    *out = path;
    ++*path;
    let_go(&path);
    return out;
}

template <int K>
int32_t **__thiscall folder_path_copy(int32_t **out) {
    return folder_path(out, folder_paths[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(folder_paths[0].at == 0x458170 && folder_paths[1].at == 0x4583a0 && folder_paths[2].at == 0x4585d0, "folder_paths.inc changed");
REWRITE("Folder::SpecialOrDocuments", 0x458170, folder_paths_first[0], (&folder_path_copy<0>), &g_folder_paths);
REWRITE("Folder::SpecialOrDocuments", 0x4583a0, folder_paths_first[1], (&folder_path_copy<1>), &g_folder_paths);
REWRITE("Folder::SpecialOrDocuments", 0x4585d0, folder_paths_first[2], (&folder_path_copy<2>), &g_folder_paths);

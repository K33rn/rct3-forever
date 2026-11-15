// Scenery::Text8345A0 and Scenery::Text834710 (one function written out twice; they differ in the format): read a
// number from a text the scenery item gives (thiscall; where to put the number; 0 when there was a text to read, 1
// when there wasn't).
//   - The text: the shared empty one (0xBD0490), filled by 0x834260; when that gives anything but 0, the place gets
//     0, the text is let go and the answer is 1.
//   - Otherwise the text narrowed in place (as station_names.cpp's narrow_in_place) and sscanf (0xDDD661) of it with
//     the copy's format ("%f" or "%d") into the place; the text let go; 0.
#include "rewrite.h"

namespace {
typedef void(__thiscall *Unique)(void *, uint32_t);
typedef void(__cdecl *Free)(void *);
inline void let_go(int32_t *volatile *held) {   // (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
const char *narrow_in_place(int32_t *volatile *held) {   // as station_names.cpp's
    int32_t *t = *held;
    uint32_t length = (uint32_t)t[1];
    if (!((uint32_t)t[2] > length && ((uint16_t *)(uintptr_t)(uint32_t)t[3])[length] == 0)) {
        ((Unique)0xbcffe0)((void *)held, length);
        t = *held;
        ((uint16_t *)(uintptr_t)(uint32_t)t[3])[(uint32_t)t[1]] = 0;
    }
    ((Unique)0xbcffe0)((void *)held, ((uint32_t)(*held)[1] + 1) * 3);
    t = *held;
    const uint32_t n = (uint32_t)t[1];
    const uint16_t *wide = (const uint16_t *)(uintptr_t)(uint32_t)t[3];
    char *narrow = (char *)(wide + n + 1);
    uint32_t i = 0;
    for (; i < n && wide[i]; i++) narrow[i] = (char)wide[i];
    narrow[i] = 0;
    t = *held;
    return (const char *)(uintptr_t)(uint32_t)t[3] + (uint32_t)t[1] * 2 + 2;
}

volatile long g_scenery_scans;
template <uintptr_t FORMAT> uint32_t __fastcall scenery_scan(void *self, void *, uint32_t *out) {
    g_scenery_scans++;
    int32_t *volatile text = ((int32_t * (__fastcall *)(uint32_t))0xbd0490)(1);
    if (((int32_t(__thiscall *)(void *, void *))0x834260)(self, (void *)&text) != 0) {
        *out = 0;
        let_go(&text);
        return 1;
    }
    const char *narrow = narrow_in_place(&text);
    ((int(__cdecl *)(const char *, const char *, ...))0xddd661)(narrow, (const char *)FORMAT, out);
    let_go(&text);
    return 0;
}

const uint8_t scenery_scan_first[][7] = {{0x6a, 0xff, 0x68, 0xb8, 0xfc, 0xe5, 0x00}, {0x6a, 0xff, 0x68, 0xd8, 0xfc, 0xe5, 0x00}};
}   // namespace
REWRITE("Scenery::ScanNumber", 0x8345a0, scenery_scan_first[0], (&scenery_scan<0xf29a90>), &g_scenery_scans);   // "%f"
REWRITE("Scenery::ScanNumber", 0x834710, scenery_scan_first[1], (&scenery_scan<0xf28e5c>), &g_scenery_scans);   // "%d"

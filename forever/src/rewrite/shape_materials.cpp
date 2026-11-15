// ShapeMaterial::FromName and its second copy (one function written out twice; they differ only in their exception
// handler): settle a shape's materials from their names. For each material (the shape's count at +0x24, list at
// +0x28):
//   - flags (+0x10): with bit 2 or 8 set, flag 1 set; otherwise flag 1 set when its name (made with 0xBCFEF0 from
//     the name object at +8, its text at +8) holds "Alpha", "Glass", "FillZ", "Caustics" or "Water" (each looked for
//     with 0xBD0960 from 0, in that order) and not "AlphaMask" (looked for first: when there, none of the others are);
//   - when the game's switch [0x12F014C] is 1, the name made again and, by it, the material's +8 set to a resource
//     (the resource manager [0x13D0E04]'s 0xBDF160) and +0xC to 1:
//       "SIGlass" (WideString::Compare): "SIAlphaMaskSpecular100Reflection", set even when there is none;
//       "SIAlphaText": "SIOpaqueText", the same;
//       otherwise, a name holding "Alpha" but not "AlphaMask": its first "Alpha" made "AlphaMask" (the part up to
//       the end of "Alpha" + "Mask" + the rest), made a narrow C string in place and looked up; set only when found.
// Texts given by name are made in fresh memory (0x10 bytes; none when there is none), wide (0xBCF850) to compare or
// join, narrow (String::FromC) to look up; every made text is let go as the game does.
#include "rewrite.h"

namespace {
typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc = (Alloc)0xbcdfc0;
typedef int32_t *Text;
// a shared text {count, length, room, characters} let go: the count taken down (read again, as the game does), at 0
// its characters freed and then it
inline void let_go(Text volatile *held) {
    --**held;
    Text text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
typedef void(__thiscall *Unique)(void *, uint32_t);   // WideString::MakeUnique, with the room wanted
// The game's narrow copy of a wide text, made just after its characters in the same buffer (as file_lists.cpp's);
// gives it.
const char *narrow_in_place(Text volatile *held) {
    Text t = *held;
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
typedef Text(__thiscall *FromC)(void *, const char *);
inline Text wide_c(const char *c) {   // WideString from C text, in fresh memory
    void *m = alloc(0x10);
    return m ? ((FromC)0xbcf850)(m, c) : nullptr;
}
inline Text narrow_c(void *m, const char *c) { return m ? ((FromC)0xbcfb10)(m, c) : nullptr; }   // String::FromC
inline int32_t find(Text volatile *in, const char *what) { return ((int32_t(__thiscall *)(void *, const char *, uint32_t))0xbd0960)((void *)in, what, 0); }
inline bool same(Text volatile *a, Text volatile *b) { return ((int32_t(__thiscall *)(void *, void *))0xbce970)((void *)a, (void *)b) == 0; }
inline uint32_t look_up(Text volatile *key) {
    return ((uint32_t(__thiscall *)(void *, void *))0xbdf160)(*(void *volatile *)(uintptr_t)0x13d0e04, (void *)key);
}
inline void name_of(Text volatile *out, uint8_t *material) {
    ((void(__thiscall *)(void *, void *))0xbcfef0)((void *)out, *(uint8_t **)(material + 8) + 8);
}
typedef void *(__thiscall *Part)(void *, void *, int32_t, int32_t);   // WideString::Substring
typedef void *(__thiscall *Join)(void *, void *, void *);

void settle(uint8_t *material) {
    const uint32_t flags = *(volatile uint32_t *)(material + 0x10);
    if (flags & 0xa) {
        *(volatile uint32_t *)(material + 0x10) = flags | 1;
    } else {
        Text volatile name;
        name_of(&name, material);
        if (find(&name, "AlphaMask") == -1) {
            const int32_t a = find(&name, "Alpha"), b = find(&name, "Glass"), c = find(&name, "FillZ"), d = find(&name, "Caustics"),
                          e = find(&name, "Water");
            if (a != -1 || b != -1 || c != -1 || d != -1 || e != -1) *(volatile uint32_t *)(material + 0x10) |= 1;
        }
        let_go(&name);
    }
    if (*(volatile int32_t *)(uintptr_t)0x12f014c != 1) return;
    Text volatile name;
    name_of(&name, material);
    Text volatile probe = wide_c("SIGlass");
    const bool glass = same(&name, &probe);
    let_go(&probe);
    void *memory = alloc(0x10);
    uint32_t found;
    if (glass) {
        Text volatile key = narrow_c(memory, "SIAlphaMaskSpecular100Reflection");
        found = look_up(&key);
        let_go(&key);
    } else {
        Text volatile probe2 = memory ? ((FromC)0xbcf850)(memory, "SIAlphaText") : nullptr;
        const bool text = same(&name, &probe2);
        let_go(&probe2);
        if (text) {
            Text volatile key = narrow_c(alloc(0x10), "SIOpaqueText");
            found = look_up(&key);
            let_go(&key);
        } else {
            if (find(&name, "AlphaMask") != -1) {
                let_go(&name);
                return;
            }
            const int32_t end = find(&name, "Alpha") + 5;
            Text volatile rest, front, front_mask, whole;
            void *after = ((Part)0xbd0810)((void *)&name, (void *)&rest, end, -1);
            Text volatile mask = wide_c("Mask");
            Text mask_now = mask;
            void *before = ((Part)0xbd0810)((void *)&name, (void *)&front, 0, end);
            void *j1 = ((Join)0x403bb0)(before, (void *)&front_mask, (void *)&mask);
            Text volatile *joined = (Text volatile *)((Join)0x403bb0)(j1, (void *)&whole, after);
            if (*joined != name) {
                --*name;
                Text old = name;
                if (*(volatile int32_t *)old == 0) {
                    ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)old[3]);
                    ((Free)0xbce000)(old);
                }
                Text now = *joined;
                name = now;
                ++*now;
            }
            let_go(&whole);
            let_go(&front_mask);
            let_go(&front);
            if (--*mask_now == 0) {   // (the game keeps "Mask" in a register: not read again, never checked for none)
                ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)mask_now[3]);
                ((Free)0xbce000)(mask_now);
            }
            let_go(&rest);
            const char *narrow = narrow_in_place(&name);
            Text volatile key = narrow ? narrow_c(alloc(0x10), narrow) : ((Text(__fastcall *)(uint32_t))0xbd0520)(1);
            found = look_up(&key);
            let_go(&key);
            if (found) *(volatile uint32_t *)(material + 8) = found, *(volatile uint32_t *)(material + 0xc) = 1;
            let_go(&name);
            return;
        }
    }
    *(volatile uint32_t *)(material + 8) = found;
    *(volatile uint32_t *)(material + 0xc) = 1;
    let_go(&name);
}

volatile long g_shape_materials;
void __stdcall shape_materials(uint8_t *shape) {
    g_shape_materials++;
    for (int32_t i = 0; i < *(volatile int32_t *)(shape + 0x24); i++) settle((*(uint8_t ***)(shape + 0x28))[i]);
}

const uint8_t shape_materials_first[][7] = {{0x6a, 0xff, 0x68, 0x42, 0x6a, 0xea, 0x00}, {0x6a, 0xff, 0x68, 0xe2, 0x77, 0xea, 0x00}};
}   // namespace
REWRITE("ShapeMaterial::FromName", 0xccb610, shape_materials_first[0], &shape_materials, &g_shape_materials);
REWRITE("ShapeMaterial::FromName", 0xcee580, shape_materials_first[1], &shape_materials, &g_shape_materials);

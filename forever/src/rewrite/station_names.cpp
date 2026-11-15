// AtStation::FromName and SwingMode::FromName (one function written out twice; tools/family.py: station_names.inc;
// they differ in the prefix and the five names), and the same with four names (LeaveRideBehaviour::FromName) and two
// (SwingAnchor::FromName; rows written here): as names_by_text.cpp's lookups, with the names written in (a switch
// on the number) instead of a names function: the number of the name that matches the text given (fastcall: the
// text's holder), case ignored; the count when none does.
//   - The prefix ("AtStation", ...: 0x10 new bytes, WideString from C; none without memory) and the text joined
//     (0x403BB0); the prefix let go.
//   - For each number: the joined text made a narrow C string in place again (the game's text code, inlined) and
//     compared with that name by the C library's _stricmp.
//   - The joined text let go.
#include "rewrite.h"

namespace {
struct StationNames {
    uint32_t at, prefix, names[5], count;
};
#define STATION_NAMES(at, prefix, n0, n1, n2, n3, n4, ...) {at, prefix, {n0, n1, n2, n3, n4}, 5},
constexpr StationNames station_names[] = {
#include "station_names.inc"
};
#undef STATION_NAMES
#define STATION_NAMES(at, prefix, n0, n1, n2, n3, n4, ...) {__VA_ARGS__},
const uint8_t station_names_first[][7] = {
#include "station_names.inc"
};
#undef STATION_NAMES

typedef void(__cdecl *Free)(void *);
inline void let_go(int32_t *volatile *held) {   // a shared text let go (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
typedef void(__thiscall *Unique)(void *, uint32_t);   // WideString::MakeUnique, with the room wanted
const char *narrow_in_place(int32_t *volatile *held) {   // as file_lists.cpp's
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

volatile long g_station_names;
int32_t station_name(void *text, const StationNames &kind) {
    g_station_names++;
    int32_t *volatile prefix, *volatile joined;
    void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
    prefix = memory ? ((int32_t * (__thiscall *)(void *, uintptr_t))0xbcf850)(memory, kind.prefix) : nullptr;
    ((void *(__thiscall *)(void *, void *, void *))0x403bb0)((void *)&prefix, (void *)&joined, text);
    let_go(&prefix);
    for (int32_t i = 0; i < (int32_t)kind.count; i++) {
        const char *narrow = narrow_in_place(&joined);
        if (((int(__cdecl *)(const char *, const char *))0xde22df)(narrow, (const char *)(uintptr_t)kind.names[i]) == 0) {
            let_go(&joined);
            return i;
        }
    }
    let_go(&joined);
    return (int32_t)kind.count;
}

template <int K> int32_t __fastcall station_name_copy(void *text) { return station_name(text, station_names[K]); }
// the two with fewer names (their switches' cases in order of number)
constexpr StationNames leave_ride = {0xbb45f0, 0xf9f140, {0xf9f124, 0xf9f108, 0xf9f0e0, 0xf9f0bc, 0}, 4};   // LeaveRideBehaviour + WalkOff, RigidBody, ...WaterSlide, ...HighDive
constexpr StationNames swing_anchor = {0xbbfde0, 0xf9fef8, {0xf9febc, 0xf9fed4, 0, 0, 0}, 2};             // SwingAnchor + ParentOrigin, ParentPivot
const uint8_t leave_ride_first[] = {0x6a, 0xff, 0x68, 0xbb, 0xc6, 0xe9, 0x00}, swing_anchor_first[] = {0x6a, 0xff, 0x68, 0x7b, 0xcc, 0xe9, 0x00};
int32_t __fastcall leave_ride_name(void *text) { return station_name(text, leave_ride); }
int32_t __fastcall swing_anchor_name(void *text) { return station_name(text, swing_anchor); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(station_names[0].at == 0xbb3e90 && station_names[1].at == 0xbbfa30, "station_names.inc changed");
REWRITE("Names::FromTextInline", 0xbb3e90, station_names_first[0], (&station_name_copy<0>), &g_station_names);
REWRITE("Names::FromTextInline", 0xbbfa30, station_names_first[1], (&station_name_copy<1>), &g_station_names);
REWRITE("Names::FromTextInline", 0xbb45f0, leave_ride_first, &leave_ride_name, &g_station_names);
REWRITE("Names::FromTextInline", 0xbbfde0, swing_anchor_first, &swing_anchor_name, &g_station_names);

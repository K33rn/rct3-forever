// Pairs::Insert85A620 and Pairs::InsertCEAB50 (two identical copies): finds a pair (a, b) in a sorted index, or adds it.
// The index keeps six arrays of whole numbers and a count (+0x18) and room (+0x1C): in the order pairs were added, a
// spare word (+0, always 0), a (+4) and b (+0xC); sorted by a then b, a (+8), b (+0x10) and the pair's number (+0x14).
// Found (looked for at the two ends first, then by halving), its number is returned. Otherwise it is added: in place when
// there is room (the sorted entries after it moved up one), else all six arrays are made again twice as large (200
// entries for none; aligned memory) with the pair put in as they are copied, and the old ones freed. Its number (the old
// count) is returned.
#include "rewrite.h"

namespace {
struct Pairs {
    int32_t *spare, *added_a, *sorted_a, *added_b, *sorted_b, *number;
    int32_t count, room;
};
// whether (a1, b1) comes before (a2, b2)
inline bool before(int32_t a1, int32_t b1, int32_t a2, int32_t b2) { return a1 < a2 || (a1 == a2 && b1 < b2); }

volatile long g_pairs;
int32_t __thiscall pairs_insert(Pairs *self, int32_t a, int32_t b) {
    g_pairs++;
    int32_t at = 0;   // where it goes in the sorted arrays
    const int32_t n = self->count;
    if (n > 0) {
        int32_t hi = n - 1, lo = 0;
        const int32_t first_a = self->sorted_a[0], last_a = self->sorted_a[hi], first_b = self->sorted_b[0], last_b = self->sorted_b[hi];
        if (first_a == a && first_b == b) return self->number[0];
        if (last_a == a && last_b == b) return self->number[hi];
        if (before(a, b, first_a, first_b)) {
            at = 0;
        } else if (before(last_a, last_b, a, b)) {
            at = n;
        } else {
            at = hi;
            while (hi > 1 && hi > lo + 1) {   // (the game tests hi > 1 once, then hi > lo + 1 after each step)
                const int32_t mid = lo + (hi - lo) / 2;
                const int32_t mid_a = self->sorted_a[mid], mid_b = self->sorted_b[mid];
                if (mid_a == a && mid_b == b) return self->number[mid];
                if (before(mid_a, mid_b, a, b)) {
                    lo = mid, at = mid + 1;
                } else {
                    hi = mid, at = mid;
                }
            }
        }
    }
    const int32_t count = self->count;
    if (count < self->room) {
        for (int32_t i = count; i > at; i--) {
            self->sorted_a[i] = self->sorted_a[i - 1];
            self->sorted_b[i] = self->sorted_b[i - 1];
            self->number[i] = self->number[i - 1];
        }
        self->sorted_a[at] = a;
        self->sorted_b[at] = b;
        self->number[at] = self->count;
        self->added_a[self->count] = a;
        self->added_b[self->count] = b;
        self->spare[self->count] = 0;
    } else {
        int32_t *const spare = self->spare, *const added_a = self->added_a, *const sorted_a = self->sorted_a, *const added_b = self->added_b,
                       *const sorted_b = self->sorted_b, *const number = self->number;
        self->room = count > 0 ? count * 2 : 200;
        typedef int32_t *(__cdecl * Alloc)(uint32_t);
        self->spare = ((Alloc)0xbcdfd0)(self->room * 4);
        self->added_a = ((Alloc)0xbcdfd0)(self->room * 4);
        self->sorted_a = ((Alloc)0xbcdfd0)(self->room * 4);
        self->added_b = ((Alloc)0xbcdfd0)(self->room * 4);
        self->sorted_b = ((Alloc)0xbcdfd0)(self->room * 4);
        self->number = ((Alloc)0xbcdfd0)(self->room * 4);
        for (int32_t i = 0; i < self->count; i++) {
            self->spare[i] = spare[i];
            self->added_a[i] = added_a[i];
            self->added_b[i] = added_b[i];
        }
        self->spare[self->count] = 0;
        self->added_a[self->count] = a;
        self->added_b[self->count] = b;
        for (int32_t i = 0; i < at; i++) {
            self->sorted_a[i] = sorted_a[i];
            self->sorted_b[i] = sorted_b[i];
            self->number[i] = number[i];
        }
        self->sorted_a[at] = a;
        self->sorted_b[at] = b;
        self->number[at] = self->count;
        for (int32_t i = at; i < self->count; i++) {
            self->sorted_a[i + 1] = sorted_a[i];
            self->sorted_b[i + 1] = sorted_b[i];
            self->number[i + 1] = number[i];
        }
        typedef void(__cdecl * Free)(void *);
        int32_t *const old[6] = {spare, added_a, sorted_a, added_b, sorted_b, number};
        for (int32_t *block : old) ((Free)0xbce010)(block);
    }
    return self->count++;
}
const uint8_t pairs_insert_original[] = {0x83, 0xec, 0x1c, 0x53, 0x55, 0x8b, 0x6c, 0x24, 0x28};
}   // namespace
REWRITE("Pairs::Insert", 0x85a620, pairs_insert_original, &pairs_insert, &g_pairs);
REWRITE("Pairs::Insert", 0xceab50, pairs_insert_original, &pairs_insert, &g_pairs);

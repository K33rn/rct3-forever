// A map from ids to small numbers, handed out in order of first use: the ids are kept in order of arrival and,
// separately, sorted (with the number of each) for binary search. The compiler made a copy for every user.
#include "rewrite.h"

struct IdMap {
    int32_t *flags;     // by number; zero for a new id
    int32_t *ids;       // by number
    int32_t *sorted;    // the ids in order
    int32_t *numbers;   // the number of each id in `sorted`
    int32_t count, capacity;
};

static void *(__cdecl *const memory_alloc_aligned)(uint32_t) = (void *(__cdecl *)(uint32_t))0xbcdfd0;
static void(__cdecl *const memory_free_aligned)(void *) = (void(__cdecl *)(void *))0xbce010;

static volatile long g_find_or_add, g_free;

// FindOrAdd(id): the id's number, giving it the next one if it's new. Doubles the arrays when they're full.
static int32_t __thiscall find_or_add(IdMap *m, int32_t id) {
    g_find_or_add++;
    int32_t at = 0;   // where a new id goes in `sorted`
    if (m->count > 0) {
        int32_t lo = 0, hi = m->count - 1;
        int32_t first = m->sorted[0], last = m->sorted[hi];
        if (first == id) return m->numbers[0];
        if (last == id) return m->numbers[hi];
        if (first < id) {
            if (last < id) {
                at = hi + 1;
            } else {
                at = hi;
                while (hi > lo + 1) {
                    int32_t mid = lo + (hi - lo) / 2, found = m->sorted[mid];
                    if (found == id) return m->numbers[mid];
                    if (found < id) {
                        lo = mid;
                        at = mid + 1;
                    } else {
                        hi = mid;
                        at = mid;
                    }
                }
            }
        }
    }
    int32_t number = m->count;
    if (number < m->capacity) {
        for (int32_t i = number; i > at; i--) {
            m->sorted[i] = m->sorted[i - 1];
            m->numbers[i] = m->numbers[i - 1];
        }
        m->sorted[at] = id;
        m->numbers[at] = number;
        m->ids[number] = id;
        m->flags[number] = 0;
    } else {
        IdMap old = *m;
        m->capacity = number > 0 ? number * 2 : 200;
        uint32_t bytes = (uint32_t)m->capacity * 4;
        m->flags = (int32_t *)memory_alloc_aligned(bytes);
        m->ids = (int32_t *)memory_alloc_aligned(bytes);
        m->sorted = (int32_t *)memory_alloc_aligned(bytes);
        m->numbers = (int32_t *)memory_alloc_aligned(bytes);
        for (int32_t i = 0; i < number; i++) {
            m->flags[i] = old.flags[i];
            m->ids[i] = old.ids[i];
        }
        m->flags[number] = 0;
        m->ids[number] = id;
        for (int32_t i = 0; i < at; i++) {
            m->sorted[i] = old.sorted[i];
            m->numbers[i] = old.numbers[i];
        }
        m->sorted[at] = id;
        m->numbers[at] = number;
        for (int32_t i = at; i < number; i++) {
            m->sorted[i + 1] = old.sorted[i];
            m->numbers[i + 1] = old.numbers[i];
        }
        memory_free_aligned(old.flags);
        memory_free_aligned(old.ids);
        memory_free_aligned(old.sorted);
        memory_free_aligned(old.numbers);
    }
    m->count = number + 1;
    return number;
}
static const uint32_t find_or_add_copies[] = {0x408240, 0x40b550, 0x49a150, 0x8e7770, 0x8e7b00,
                                              0x9cdd70, 0xce9f00, 0xcea320, 0xcea730};
static const uint8_t find_or_add_original[] = {0x8b, 0x54, 0x24, 0x04, 0x83, 0xec, 0x10, 0x53};
REWRITE_COPIES("IdMap::FindOrAdd", find_or_add_copies, find_or_add_original, find_or_add, &g_find_or_add);

// Free: the four arrays (the map itself belongs to its owner).
static void __fastcall id_map_free(IdMap *m) {
    g_free++;
    memory_free_aligned(m->flags);
    memory_free_aligned(m->ids);
    memory_free_aligned(m->sorted);
    memory_free_aligned(m->numbers);
}
static const uint32_t free_copies[] = {0x408210, 0x40b520, 0x49a120, 0x8e7740, 0x8e7ad0,
                                       0x9cdd40, 0xce9ed0, 0xcea2f0, 0xcea700};
static const uint8_t free_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0x50, 0xe8};
REWRITE_COPIES("IdMap::Free", free_copies, free_original, id_map_free, &g_free);

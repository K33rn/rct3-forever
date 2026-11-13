// "Make with a shared table" constructors: each keeps some values from its arguments in the object, then, the first
// time (bit 0 of a global says whether it's been done), fills a shared table through Table::Fill16 with sixteen two-byte
// values, and points the object at the table. They differ only in what is kept and where, the global, the table and the
// sixteen values: those are read from the game by tools/table_makes.py into table_makes.inc.
#include "rewrite.h"

struct TableKeep {
    uint16_t to;     // where in the object
    uint8_t arg;     // which argument points at the value
    uint16_t from;   // where behind that pointer
};
struct TableArgs {   // every one takes sixteen arguments (0x40 bytes), whatever it uses
    const uint8_t *a[16];
};
static volatile long g_table_makes;
template <uint32_t MADE, uint32_t TABLE, const uint16_t *WORDS, const TableKeep *KEEPS, uint32_t N>
static void *__thiscall table_make(uint8_t *self, TableArgs args) {
    g_table_makes++;
    for (uint32_t i = 0; i < N; i++) *(volatile uint32_t *)(self + KEEPS[i].to) = *(const volatile uint32_t *)(args.a[KEEPS[i].arg] + KEEPS[i].from);
    volatile uint32_t &made = *(volatile uint32_t *)MADE;
    if (!(made & 1)) {
        made = made | 1;
        const uint16_t *w = WORDS;
        ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                             uint32_t, uint32_t, uint32_t, uint32_t))0x416110)((void *)TABLE, w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7], w[8], w[9], w[10], w[11],
                                                                               w[12], w[13], w[14], w[15]);
    }
    *(volatile uint32_t *)self = TABLE;
    return self;
}
#define ARG(...) __VA_ARGS__
#define TABLE_MAKE(at, first, made, table, words, n, keeps)                                 \
    static const uint8_t table_make_original_##at[] = {first};                             \
    static const uint16_t table_make_words_##at[16] = {words};                             \
    static const TableKeep table_make_keeps_##at[] = {keeps};                              \
    REWRITE("Table::Make", at, table_make_original_##at, (&table_make<made, table, table_make_words_##at, table_make_keeps_##at, n>), &g_table_makes);
#include "table_makes.inc"
#undef TABLE_MAKE
// for the tester: each one's address and its global
#define TABLE_MAKE(at, first, made, table, words, n, keeps) at, made,
extern const uint32_t table_make_info[];
const uint32_t table_make_info[] = {
#include "table_makes.inc"
    0};

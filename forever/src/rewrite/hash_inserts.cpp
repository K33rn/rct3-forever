// HashTable::Insert (two copies; they differ in how they make room first): a key (a counted text) and its value put
// into a table that keeps its entries sorted by bucket (thiscall: the key's place, the value's place; ret 8).
//   The table: [1] its bucket count, [2] its entry count, [3] where each bucket's entries start (one more than the
//   buckets), [4] the keys, [5] the values.
//   - Room made (the copy's call, on the table), the key hashed (0xBDBF70 on the table, given the key's place), its
//     bucket the hash's remainder by the bucket count.
//   - The entries from the next bucket's start to the end moved up by one, from the last down (each key a counted
//     assignment, each value copied); the new key and value put at the next bucket's start.
//   - The entry count raised, and the start of every bucket after the key's.
//   A counted assignment: nothing when it is the same text; else the old text's count lowered (its memory freed,
//   0xBCE010 then 0xBCE000, when that reaches 0 and the text is there), the new one put and its count raised.
#include "rewrite.h"

namespace {
volatile long g_hash_inserts;
struct Counted {
    int32_t count;
    uint32_t pad[2];
    void *memory;
};
void assign(Counted **to, Counted *from) {
    Counted *old = *to;
    if (old == from) return;
    old->count--;
    old = *to;
    if (old->count == 0 && old) {
        ((void(__cdecl *)(void *))0xbce010)(old->memory);
        ((void(__cdecl *)(void *))0xbce000)(old);
    }
    *to = from;
    from->count++;
}
struct Table {
    uint32_t pad;
    uint32_t buckets, count;
    uint32_t *starts;
    Counted **keys;
    uint32_t *values;
};

template <uint32_t MAKE_ROOM> void __fastcall hash_insert(Table *table, void *, Counted **key, uint32_t *value) {
    g_hash_inserts++;
    ((void(__thiscall *)(Table *))MAKE_ROOM)(table);
    uint32_t hash = ((uint32_t(__thiscall *)(Table *, Counted **))0xbdbf70)(table, key);
    uint32_t next = hash % table->buckets + 1;
    uint32_t at = table->count;
    for (; at > table->starts[next]; at--) {
        assign(&table->keys[at], table->keys[at - 1]);
        table->values[at] = table->values[at - 1];
    }
    assign(&table->keys[at], *key);
    table->values[at] = *value;
    table->count++;
    for (; next <= table->buckets; next++) table->starts[next]++;
}

const uint8_t hash_insert_first[][7] = {{0x51, 0x53, 0x55, 0x56, 0x57, 0x8b, 0xf1}};
}   // namespace
REWRITE("HashTable::Insert", 0xbdf1c0, hash_insert_first[0], &hash_insert<0xbe0730>, &g_hash_inserts);
REWRITE("HashTable::Insert", 0xbdf2d0, hash_insert_first[0], &hash_insert<0xbe0840>, &g_hash_inserts);

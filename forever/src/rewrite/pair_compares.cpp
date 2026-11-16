// Pair::Compare (two copies, one function): two pairs of counted texts compared, the first texts first, the second
// ones when the first compare gives 0 (fastcall: the two pairs; gives the compare's result).
//   - The first texts are taken as counted copies (each text's count raised by one), and the compare (0xCD5640, made on
//     an empty compare object on the stack, given the two copies' places) run on them.
//   - A result other than 0: the copies let go (the second, then the first: each count lowered, and the text's memory
//     freed, 0xBCE010 then 0xBCE000, when it reaches 0), the result given.
//   - 0: the compare run again on the two second texts (in place), then the copies let go the same way; its result given.
#include "rewrite.h"

namespace {
volatile long g_pair_compares;
struct Counted {   // a counted text's shared part
    int32_t count;
    uint32_t pad[2];
    void *memory;
};
void let_go(Counted *text) {
    if (--text->count == 0) {
        ((void(__cdecl *)(void *))0xbce010)(text->memory);
        ((void(__cdecl *)(void *))0xbce000)(text);
    }
}
using Compare = int(__thiscall *)(void *, Counted **, Counted **);

int __fastcall pair_compare(Counted **a, Counted **b) {
    g_pair_compares++;
    Counted *first = *a;
    first->count++;
    Counted *second = *b;
    second->count++;
    volatile uint8_t compare = 0;   // (the compare object: empty)
    int result = ((Compare)0xcd5640)((void *)&compare, &first, &second);
    if (result == 0) result = ((Compare)0xcd5640)((void *)&compare, a + 1, b + 1);
    let_go(second);
    let_go(first);
    return result;
}

const uint8_t pair_compare_first[][7] = {{0x6a, 0xff, 0x68, 0xb0, 0x07, 0xe1, 0x00}, {0x6a, 0xff, 0x68, 0xb0, 0x9a, 0xe9, 0x00}};
}   // namespace
REWRITE("Pair::Compare", 0x4a36e0, pair_compare_first[0], &pair_compare, &g_pair_compares);
REWRITE("Pair::Compare", 0xb92c80, pair_compare_first[1], &pair_compare, &g_pair_compares);

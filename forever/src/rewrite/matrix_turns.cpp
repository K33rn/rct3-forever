// Matrix::TurnAboutX / Y / Z, Matrix::RotateX / Z (two copies each), Matrix::TurnColumnsAboutY (two) and Shape::TurnAbout
// X / Y / Z: one function written out twelve times, differing only in which three pairs of the matrix's floats it
// turns (rows or columns, about x, y or z), and in the order of one addition: turn a matrix by an angle (thiscall; the
// angle; no value).
//   - The angle's cosine c and sine s from the FPU's fsincos, each stored as a float (as the game does: fld; fsincos;
//     fstp; fstp, so past 2^63 c is the angle and s the FPU's default NaN).
//   - For each pair (P, Q) in turn: P becomes c P - s Q, then Q becomes c Q + s P (P as it was, as loaded onto the
//     FPU), each rounded where the game stores it. Which NaN comes out depends on the operations' order and on what is
//     on the FPU or in memory, so each pair is done with the game's own sequence: the addition of the second with c Q
//     first (style x) or s P first (style y), as the copy has it.
#include "rewrite.h"

namespace {
volatile long g_matrix_turns;
template <bool Y_STYLE> inline void turn_pair(volatile float *p, volatile float *q, const float *c, const float *s) {
    if (Y_STYLE)
        __asm__ volatile(".intel_syntax noprefix\n\t"
                         "fld dword ptr [eax]\n\tfld dword ptr [ecx]\n\tfmul dword ptr [eax]\n\tfld dword ptr [esi]\n\tfmul dword ptr [edx]\n\t"
                         "fsubp st(1), st\n\tfstp dword ptr [eax]\n\t"
                         "fmul dword ptr [esi]\n\tfld dword ptr [ecx]\n\tfmul dword ptr [edx]\n\tfaddp st(1), st\n\tfstp dword ptr [edx]\n\t"
                         ".att_syntax prefix"
                         :
                         : "a"(p), "d"(q), "c"(c), "S"(s)
                         : "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    else
        __asm__ volatile(".intel_syntax noprefix\n\t"
                         "fld dword ptr [eax]\n\tfld dword ptr [ecx]\n\tfmul dword ptr [eax]\n\tfld dword ptr [esi]\n\tfmul dword ptr [edx]\n\t"
                         "fsubp st(1), st\n\tfstp dword ptr [eax]\n\t"
                         "fld dword ptr [ecx]\n\tfmul dword ptr [edx]\n\tfxch st(1)\n\tfmul dword ptr [esi]\n\tfaddp st(1), st\n\tfstp dword ptr [edx]\n\t"
                         ".att_syntax prefix"
                         :
                         : "a"(p), "d"(q), "c"(c), "S"(s)
                         : "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
}

// STYLES: bit k set when pair k adds s P first (style y)
template <uint32_t P0, uint32_t Q0, uint32_t P1, uint32_t Q1, uint32_t P2, uint32_t Q2, uint32_t STYLES>
void __fastcall matrix_turn(uint8_t *m, void *, float angle) {
    g_matrix_turns++;
    float c, s;
    __asm__ volatile("flds %2\n\tfsincos\n\tfstps %0\n\tfstps %1"
                     : "=m"(c), "=m"(s)
                     : "m"(angle)
                     : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    static const uint32_t pairs[3][2] = {{P0, Q0}, {P1, Q1}, {P2, Q2}};
    for (int k = 0; k < 3; k++) {
        volatile float *p = (volatile float *)(m + pairs[k][0]), *q = (volatile float *)(m + pairs[k][1]);
        if (STYLES >> k & 1) turn_pair<true>(p, q, &c, &s);
        else turn_pair<false>(p, q, &c, &s);
    }
}

const uint8_t matrix_turn_first[] = {0x83, 0xec, 0x08, 0x8b, 0xc1, 0xd9, 0x44};
}   // namespace
REWRITE("Matrix::TurnPairs", 0xbedd30, matrix_turn_first, (&matrix_turn<0x20, 0x10, 0x24, 0x14, 0x28, 0x18, 0>), &g_matrix_turns);   // TurnAboutX
REWRITE("Matrix::TurnPairs", 0xc20680, matrix_turn_first, (&matrix_turn<0x20, 0x10, 0x24, 0x14, 0x28, 0x18, 0>), &g_matrix_turns);   // Shape::TurnAboutX
REWRITE("Matrix::TurnPairs", 0xbedee0, matrix_turn_first, (&matrix_turn<0x8, 0x4, 0x18, 0x14, 0x28, 0x24, 0>), &g_matrix_turns);     // RotateX2
REWRITE("Matrix::TurnPairs", 0xc20830, matrix_turn_first, (&matrix_turn<0x8, 0x4, 0x18, 0x14, 0x28, 0x24, 0>), &g_matrix_turns);     // RotateX
REWRITE("Matrix::TurnPairs", 0xbeddc0, matrix_turn_first, (&matrix_turn<0x0, 0x20, 0x4, 0x24, 0x8, 0x28, 5>), &g_matrix_turns);      // TurnAboutY
REWRITE("Matrix::TurnPairs", 0xc20710, matrix_turn_first, (&matrix_turn<0x0, 0x20, 0x4, 0x24, 0x8, 0x28, 5>), &g_matrix_turns);      // Shape::TurnAboutY
REWRITE("Matrix::TurnPairs", 0xbedf70, matrix_turn_first, (&matrix_turn<0x0, 0x8, 0x10, 0x18, 0x20, 0x28, 5>), &g_matrix_turns);     // TurnColumnsAboutY
REWRITE("Matrix::TurnPairs", 0xc208c0, matrix_turn_first, (&matrix_turn<0x0, 0x8, 0x10, 0x18, 0x20, 0x28, 5>), &g_matrix_turns);     // TurnColumnsAboutY
REWRITE("Matrix::TurnPairs", 0xbede50, matrix_turn_first, (&matrix_turn<0x10, 0x0, 0x14, 0x4, 0x18, 0x8, 4>), &g_matrix_turns);      // TurnAboutZ
REWRITE("Matrix::TurnPairs", 0xc207a0, matrix_turn_first, (&matrix_turn<0x10, 0x0, 0x14, 0x4, 0x18, 0x8, 4>), &g_matrix_turns);      // Shape::TurnAboutZ
REWRITE("Matrix::TurnPairs", 0xbee000, matrix_turn_first, (&matrix_turn<0x4, 0x0, 0x14, 0x10, 0x24, 0x20, 4>), &g_matrix_turns);     // RotateZ2
REWRITE("Matrix::TurnPairs", 0xc20950, matrix_turn_first, (&matrix_turn<0x4, 0x0, 0x14, 0x10, 0x24, 0x20, 4>), &g_matrix_turns);     // RotateZ

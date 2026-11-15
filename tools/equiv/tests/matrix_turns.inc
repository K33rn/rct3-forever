// Matrix::TurnPairs (forever/src/rewrite/matrix_turns.cpp). A 4x4 matrix of floats at arena 0x100 (ordinary values,
// sometimes any bit pattern, signalling NaNs among them, given as bits); the angle small, large, past 2^63, or any
// bit pattern.
#if EQUIV_PART == 1
static uint32_t mt_bits(float f) {
    uint32_t b;
    memcpy(&b, &f, 4);
    return b;
}
#else
    Test matrix_turns{0xbedd30,
                      [=](Rng &r) {
                          fresh(r);
                          for (int k = 0; k < 16; k++) {
                              uint32_t bits;
                              switch (r.range(0, 9)) {
                              case 0: bits = r.next(); break;
                              case 1: bits = 0x7fa00001u; break;
                              default: bits = mt_bits(r.real(-50, 50)); break;
                              }
                              ((volatile uint32_t *)arena<uint32_t>(0x100))[k] = bits;
                          }
                          float angle;
                          switch (r.range(0, 7)) {
                          case 0: angle = r.any_float(); break;
                          case 1: angle = r.range(0, 1) ? 1e19f : -3e20f; break;
                          case 2: angle = r.real(-1e6f, 1e6f); break;
                          default: angle = r.real(-7, 7); break;
                          }
                          arena<float>(0x50)[0] = angle;
                      },
                      [](void *fn) {
                          ((void(__fastcall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), nullptr, arena<uint32_t>(0x50)[0]);
                          return Result{};
                      }};
    matrix_turns.iterations = 3000;
    quick(matrix_turns);
#endif

// Quaternion::Slerp (forever/src/rewrite/rotation_blend.cpp): two rotation vectors and a fraction, out at arena 0x100.
#if EQUIV_PART == 1
static float rb_value(Rng &r, float scale) {   // one component: small, ordinary, or any bit pattern
    switch (r.range(0, 9)) {
    case 0: { float f; do f = r.any_float(); while (!std::isfinite(f)); return f; }   // (acos of a NaN goes to the C library's
            // error handler, which needs a running game: finite rotations only)
    case 1: return 0.0f;
    case 2: return r.real(-1e-7f, 1e-7f);
    case 3: { const float odd[] = {1e-30f, -1e20f, 1e30f, 3.14159265f, -3.14159265f, 6.2831853f}; return odd[r.range(0, 5)]; }
    default: return r.real(-scale, scale);
    }
}
#else
    Test rotation_blend{0xcc5c80,
                        [=](Rng &r) {
                            fresh(r);
                            float *a = arena<float>(0x40), *b = arena<float>(0x50);
                            const float scale = r.range(0, 2) ? 3.5f : 0.05f;
                            for (int k = 0; k < 3; k++) a[k] = rb_value(r, scale);
                            switch (r.range(0, 5)) {
                            case 0: for (int k = 0; k < 3; k++) b[k] = a[k] + r.real(-0.06f, 0.06f); break;   // close: a straight blend
                            case 1: for (int k = 0; k < 3; k++) b[k] = a[k] + r.real(-1e-3f, 1e-3f) + (k ? 0 : 0.2f); break;   // nearly the same rotation
                            case 2: for (int k = 0; k < 3; k++) b[k] = -a[k]; break;   // turned round (a half turn gives 0)
                            case 3: for (int k = 0; k < 3; k++) a[k] = r.real(-1e-7f, 1e-7f), b[k] = rb_value(r, 3.5f); break;
                            default: for (int k = 0; k < 3; k++) b[k] = rb_value(r, scale); break;
                            }
                            if (r.range(0, 7) == 0) for (int k = 0; k < 3; k++) a[k] = (k ? 0.0f : 3.14159265f), b[k] = (k ? 0.0f : -3.14159265f);
                            const float ts[] = {0.0f, 1.0f, 0.5f, -0.25f, 1.5f, NAN};
                            *arena<float>(0x60) = r.range(0, 3) ? r.real(0, 1) : ts[r.range(0, 5)];
                            if (r.range(0, 7) == 0) {   // opposite small turns blended halfway: exactly 0 (its -0 kept apart from +0)
                                const float v = r.real(0.06f, 1.5f);
                                a[0] = v, a[1] = -0.0f, a[2] = 0.0f, b[0] = -v, b[1] = -0.0f, b[2] = 0.0f;
                                if (r.range(0, 1)) a[1] = b[1] = 0.0f, a[2] = b[2] = -0.0f;
                                *arena<float>(0x60) = 0.5f;
                            }
                        },
                        [](void *fn) {
                            void *back = ((void *(__fastcall *)(void *, const void *, const void *, float))fn)(arena<void>(0x100), arena<void>(0x40),
                                                                                                             arena<void>(0x50), *arena<float>(0x60));
                            return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0))};
                        }};
    rotation_blend.iterations = 3000;
    quick(rotation_blend);
#endif

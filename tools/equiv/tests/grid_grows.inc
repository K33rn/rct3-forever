// Grid::GrowM16 (templates.cpp's grid_grow with M 16: Water::Grow96E0F0): as the Grid::GrowM8 and M4 tests in equiv.cpp
// (their setup, call and stand-ins).
#if EQUIV_PART == 2
    Test grid_grow_m16{0x96e0f0, grid_grow_m8.setup, grid_grow_m8.call};
    grid_grow_m16.prepare = grid_grow_m8.prepare;
    grid_grow_m16.finish = grid_grow_m8.finish;
    quick(grid_grow_m16);
#endif

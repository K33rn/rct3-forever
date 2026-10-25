# Tables filled in at start-up

Static initializers: functions the compiler runs before `main` to fill in global tables.

| Address | Name | What it does |
|---|---|---|
| `0xf05ca0` | Colours table | Fills in a table of 2,720 colours at `0x13F7188` onward, each made by `0xCC4D60` (stored as blue, green, red, 0) |
| `0xef42a0` | YUV to RGB table (V for red) | Fills in a 256-entry table at `0x13E43C0`: 1.596 x i, for converting video colours (YUV) to RGB; 1.596 is the standard (BT.601) factor of V in red |
| `0xef7c10` | YUV to RGB table (V for green) | 256 entries at `0x13E3FC0`: 0.813 x i in fixed point (x 0x400), the factor of V in green |
| `0xeff4f0` | YUV to RGB table (U for green) | 256 entries at `0x13E3BC0`: 0.391 x i in fixed point (x 0x400), the factor of U in green |
| `0xefb880` | YUV to RGB table | 256 entries at `0x13E37C0`: i x 0x200000, a companion of the three above |
| `0xf047d0` | Colours table (256) | Fills in a table of 256 colours at `0x13F9C08` onward with `0xCC4D60` (blue, green, red, 0) |

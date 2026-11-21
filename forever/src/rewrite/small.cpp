// Small game functions written by hand, one at a time (each from what it does: see its comment).
#include "wave.h"

// ---- CrashDump::ExceptionName(code) (0x456FD0): the name of a Windows exception code, for the crash report ------------
// The code comes in eax and the name goes back in eax: the game's own texts (its read-only data), "Unknown exception
// type" for any other code.
static volatile long g_exception_names;
static const char *__attribute__((regparm(1))) crash_exception_name(uint32_t code) {
    g_exception_names++;
    uint32_t text;
    switch (code) {
    case 0x40010005: text = 0xf29a34; break;   // Control-C
    case 0x40010008: text = 0xf29a40; break;   // Control-Break
    case 0x80000002: text = 0xf29a50; break;   // Datatype Misalignment
    case 0x80000003: text = 0xf29a28; break;   // Breakpoint
    case 0xc0000005: text = 0xf29a04; break;   // Access Violation
    case 0xc0000006: text = 0xf29a18; break;   // In Page Error
    case 0xc0000017: text = 0xf299f8; break;   // No Memory
    case 0xc000001d: text = 0xf299e4; break;   // Illegal Instruction
    case 0xc0000025: text = 0xf299c8; break;   // Noncontinuable Exception
    case 0xc0000026: text = 0xf299b4; break;   // Invalid Disposition
    case 0xc000008c: text = 0xf2999c; break;   // Array Bounds Exceeded
    case 0xc000008d: text = 0xf29984; break;   // Float Denormal Operand
    case 0xc000008e: text = 0xf2996c; break;   // Float Divide by Zero
    case 0xc000008f: text = 0xf29954; break;   // Float Inexact Result
    case 0xc0000090: text = 0xf2993c; break;   // Float Invalid Operation
    case 0xc0000091: text = 0xf2992c; break;   // Float Overflow
    case 0xc0000092: text = 0xf29918; break;   // Float Stack Check
    case 0xc0000093: text = 0xf29908; break;   // Float Underflow
    case 0xc0000094: text = 0xf298f0; break;   // Integer Divide by Zero
    case 0xc0000095: text = 0xf298dc; break;   // Integer Overflow
    case 0xc0000096: text = 0xf298c4; break;   // Privileged Instruction
    case 0xc00000fd: text = 0xf298b4; break;   // Stack Overflow
    case 0xc0000142: text = 0xf29868; break;   // DLL Initialization Failed
    case 0xe06d7363: text = 0xf29884; break;   // Microsoft C++ Exception
    default: text = 0xf2989c; break;           // Unknown exception type
    }
    return (const char *)(uintptr_t)text;
}
static const uint8_t crash_exception_name_original[] = {0x3d, 0x8e, 0x00, 0x00, 0xc0, 0x0f, 0x87, 0xad};
REWRITE("CrashDump::ExceptionName", 0x456fd0, crash_exception_name_original, &crash_exception_name, &g_exception_names);

// ---- Device::ErrorText(code) (0xCE6090): the text for a DirectSound result code ----------------------------------------
// The code comes in ecx; the text ("NAME:what it means", the game's own) goes back in eax, DSERR_GENERIC's "Unknown
// error code" for any other.
static volatile long g_sound_error_texts;
static const char *__fastcall sound_error_text(int32_t code) {
    g_sound_error_texts++;
    uint32_t text;
    switch ((uint32_t)code) {
    case 0x00000000: text = 0xfc804c; break;   // DS_OK
    case 0x0878000a: text = 0xfc8070; break;   // DS_NO_VIRTUALIZATION
    case 0x80004001: text = 0xfc87e0; break;   // DSERR_UNSUPPORTED
    case 0x80004002: text = 0xfc8828; break;   // DSERR_NOINTERFACE
    case 0x80004003: text = 0xfc87c0; break;   // E_POINTER
    case 0x80004005: text = 0xfc8730; break;   // DSERR_GENERIC
    case 0x80040110: text = 0xfc8780; break;   // DSERR_NOAGGREGATION
    case 0x80070005: text = 0xfc86e8; break;   // DSERR_ACCESSDENIED
    case 0x8007000e: text = 0xfc8620; break;   // DSERR_OUTOFMEMORY
    case 0x80070057: text = 0xfc8698; break;   // DSERR_INVALIDPARAM
    case 0x8878000a: text = 0xfc85a8; break;   // DSERR_ALLOCATED
    case 0x8878001e: text = 0xfc84e0; break;   // DSERR_CONTROLUNAVAIL
    case 0x88780032: text = 0xfc8550; break;   // DSERR_INVALIDCALL
    case 0x88780046: text = 0xfc8470; break;   // DSERR_PRIOLEVELNEEDED
    case 0x88780064: text = 0xfc83f0; break;   // DSERR_BADFORMAT
    case 0x88780078: text = 0xfc8338; break;   // DSERR_NODRIVER
    case 0x88780082: text = 0xfc8430; break;   // DSERR_ALREADYINITIALIZED
    case 0x88780096: text = 0xfc83a8; break;   // DSERR_BUFFERLOST
    case 0x887800a0: text = 0xfc82c8; break;   // DSERR_OTHERAPPHASPRIO
    case 0x887800aa: text = 0xfc8230; break;   // DSERR_UNINITIALIZED
    case 0x887800b4: text = 0xfc8130; break;   // DSERR_BUFFERTOOSMALL
    case 0x887800be: text = 0xfc8188; break;   // DSERR_DS8_REQUIRED
    case 0x88781161: text = 0xfc80f0; break;   // DSERR_OBJECTNOTFOUND
    default: text = 0xfc80cc; break;           // DSERR_GENERIC: Unknown error code
    }
    return (const char *)(uintptr_t)text;
}
static const uint8_t sound_error_text_original[] = {0x81, 0xf9, 0x46, 0x00, 0x78, 0x88, 0x0f, 0x8f};
REWRITE("Device::ErrorText", 0xce6090, sound_error_text_original, &sound_error_text, &g_sound_error_texts);

// ---- Ride::Case7AD8A0(code) (0x7AD8A0): a ride piece code's number in a short list ------------------------------------
// The code comes in ecx: 0x84-0xA1 give 0-0x1D, 0xAC 0x1E, 0xC0-0xC5 0x1F-0x24, 0xC7 0x25, 0xE6 0x26, 0xE5 0x27,
// 0xE7-0xF3 0x28-0x34, 0xFC 0x35, 0xFD 0x36; anything else 0x38.
static volatile long g_ride_cases;
static uint32_t __fastcall ride_case(uint32_t code) {
    g_ride_cases++;
    if (code >= 0x84 && code <= 0xa1) return code - 0x84;
    if (code >= 0xc0 && code <= 0xc5) return code - 0xc0 + 0x1f;
    if (code >= 0xe7 && code <= 0xf3) return code - 0xe7 + 0x28;
    switch (code) {
    case 0xac: return 0x1e;
    case 0xc7: return 0x25;
    case 0xe6: return 0x26;
    case 0xe5: return 0x27;
    case 0xfc: return 0x35;
    case 0xfd: return 0x36;
    default: return 0x38;
    }
}
static const uint8_t ride_case_original[] = {0x8d, 0x81, 0x7c, 0xff, 0xff, 0xff, 0x83, 0xf8};
REWRITE("Ride::Case7AD8A0", 0x7ad8a0, ride_case_original, &ride_case, &g_ride_cases);

// A 4x4 identity matrix written as the game writes it (whole dwords)
static void identity_matrix(uint32_t *out) {
    volatile uint32_t *const o = out;
    for (int i = 0; i < 16; i++) o[i] = (i % 5 == 0) ? 0x3f800000 : 0;
}

// ---- Matrices::Get(this, out, n) (0xCEEFB0): matrix n (64 bytes each, at +0x30; count +0x2C) copied out, or the
// identity when n is outside the list --------------------------------------------------------------------------------
static volatile long g_matrix_gets;
static void __thiscall matrices_get(const uint8_t *self, uint32_t *out, int32_t n) {
    g_matrix_gets++;
    if (n < 0 || n >= *(const int32_t *)(self + 0x2c)) {
        identity_matrix(out);
        return;
    }
    const uint32_t *const m = (const uint32_t *)(*(const uint8_t *const *)(self + 0x30) + n * 64);
    for (int i = 0; i < 16; i++) ((volatile uint32_t *)out)[i] = m[i];
}
static const uint8_t matrices_get_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Matrices::Get", 0xceefb0, matrices_get_original, &matrices_get, &g_matrix_gets);

// ---- Object::Matrix(this, out) (0x4CC210): the matrix of the object's part (+0x41C, its matrix at +0x60) copied out,
// or the identity when it has none ---------------------------------------------------------------------------------------
static volatile long g_object_matrices;
static void __thiscall object_matrix(const uint8_t *self, uint32_t *out) {
    g_object_matrices++;
    const uint8_t *const part = *(const uint8_t *const *)(self + 0x41c);
    if (!part) {
        identity_matrix(out);
        return;
    }
    for (int i = 0; i < 16; i++) ((volatile uint32_t *)out)[i] = ((const uint32_t *)(part + 0x60))[i];
}
static const uint8_t object_matrix_original[] = {0x8b, 0x81, 0x1c, 0x04, 0x00, 0x00, 0x33, 0xc9};
REWRITE("Object::Matrix", 0x4cc210, object_matrix_original, &object_matrix, &g_object_matrices);

// ---- HandleRef::FromRef(this, from) (0x98AC70): this made a copy of another handle record -----------------------------
// Two references (+0 and +0x1C: the reference base 0xF26AE0, a pointer to themselves at +0x10 of each, the object
// referred to with its count (+8) raised, and their list node put first in that object's list (its +0x20)); +0x14 and
// the counted value at +0x18 (its count raised); the rest (+0x30 to +0x54, a byte at +0x40) copied.
static volatile long g_handle_refs;
static void handle_ref_link(uint32_t *ref, uint32_t *object) {
    volatile uint32_t *const r = ref;
    ((volatile uint32_t *)object)[2]++;
    r[1] = (uint32_t)(uintptr_t)object;
    volatile uint32_t *const list = (volatile uint32_t *)(uintptr_t)object[8];
    r[2] = list[0];
    ((volatile uint32_t *)(uintptr_t)list[0])[1] = (uint32_t)(uintptr_t)&ref[2];
    r[3] = (uint32_t)(uintptr_t)list;
    list[0] = (uint32_t)(uintptr_t)&ref[2];
}
static uint32_t *__thiscall handle_ref_from_ref(uint32_t *self, const uint32_t *from) {
    g_handle_refs++;
    volatile uint32_t *const t = self;
    t[0] = 0xf26ae0;
    t[4] = (uint32_t)(uintptr_t)self;
    handle_ref_link(self, (uint32_t *)(uintptr_t)from[1]);
    t[5] = from[5];
    t[6] = from[6];
    (*(volatile uint32_t *)(uintptr_t)from[6])++;
    t[7] = 0xf26ae0;
    t[11] = (uint32_t)(uintptr_t)&self[7];
    handle_ref_link(self + 7, (uint32_t *)(uintptr_t)from[8]);
    for (int i = 12; i < 16; i++) t[i] = from[i];
    ((volatile uint8_t *)self)[0x40] = ((const uint8_t *)from)[0x40];
    for (int i = 17; i < 22; i++) t[i] = from[i];
    return self;
}
static const uint8_t handle_ref_from_ref_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0xc7, 0x00};
REWRITE("HandleRef::FromRef", 0x98ac70, handle_ref_from_ref_original, &handle_ref_from_ref, &g_handle_refs);

// ---- TrackImporter::MapPieceCode(code, other) (0xA3FAB0): an imported piece code (0-9) mapped to the game's ------------
// By one of two tables, the second when the flag (a byte) is set.
static volatile long g_piece_codes;
static int32_t __stdcall track_importer_map_piece_code(int32_t code, uint32_t other) {
    g_piece_codes++;
    static const int32_t tables[20] = {0, 1, 2, 3, 4, 5, 6, 7, 3, 6, 0, 1, 1, 2, 4, 4, 5, 7, 3, 6};
    return tables[code + ((other & 0xff) ? 10 : 0)];
}
static const uint8_t track_importer_map_piece_code_original[] = {0x83, 0xec, 0x50, 0xb9, 0x04, 0x00, 0x00, 0x00};
REWRITE("TrackImporter::MapPieceCode", 0xa3fab0, track_importer_map_piece_code_original, &track_importer_map_piece_code, &g_piece_codes);

// ---- Object59F270::Object59F270(this, m, a, b, c, point, flag) (0x59F270): a record from a matrix and values ----------
// The table 0xF3DC68; +0x10 row 0's first two values, +0x20, +0x30, +0x40 rows 1-3's first three; +0x50 a, b, c; +0x60
// the point; +0x70 the flag and a cleared byte; an empty list at +0x74 (its head +0x80 pointing at itself through +0x74,
// +0x84 back to it); +0x8C cleared.
static volatile long g_object_59f270s;
static uint32_t *__thiscall object_59f270(uint32_t *self, const uint32_t *m, uint32_t a, uint32_t b, uint32_t c, const uint32_t *point, uint32_t flag) {
    g_object_59f270s++;
    volatile uint32_t *const t = self;
    t[0] = 0xf3dc68;
    t[4] = m[0], t[5] = m[1];
    for (int row = 1; row < 4; row++)
        for (int k = 0; k < 3; k++) t[4 + 4 * row + k] = m[4 * row + k];
    t[0x50 / 4] = a, t[0x54 / 4] = b, t[0x58 / 4] = c;
    t[0x60 / 4] = point[0], t[0x64 / 4] = point[1], t[0x68 / 4] = point[2];
    ((volatile uint8_t *)self)[0x70] = (uint8_t)flag;
    ((volatile uint8_t *)self)[0x71] = 0;
    t[0x7c / 4] = 0, t[0x88 / 4] = 0, t[0x78 / 4] = 0, t[0x80 / 4] = 0;
    t[0x74 / 4] = (uint32_t)(uintptr_t)(self + 0x80 / 4);
    t[0x84 / 4] = (uint32_t)(uintptr_t)(self + 0x74 / 4);
    t[0x8c / 4] = 0;
    return self;
}
static const uint8_t object_59f270_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0xc7, 0x00};
REWRITE("Object59F270::Object59F270", 0x59f270, object_59f270_original, &object_59f270, &g_object_59f270s);

// ---- Rect::SideCell(this, n, x, y) (0xBAD250): the cell beside place n along one side of a rectangle -------------------
// When n is under the side's length (+0x1EC) and the side (+0x1E4) is 0-3: from the corner (+0x1DC, +0x1E0, bytes)
// side 0 gives (x + 1, y + n), 1 (x + n, y - 1), 2 (x - 1, y + n), 3 (x + n, y + 1); otherwise nothing is written.
static volatile long g_side_cells;
static void __thiscall rect_side_cell(const uint8_t *self, int32_t n, uint8_t *x, uint8_t *y) {
    g_side_cells++;
    if (n >= *(const int32_t *)(self + 0x1ec) || *(const uint32_t *)(self + 0x1e4) > 3)
        return;
    const uint8_t cx = self[0x1dc], cy = self[0x1e0];
    switch (*(const uint32_t *)(self + 0x1e4)) {
    case 0: *(volatile uint8_t *)x = (uint8_t)(cx + 1), *(volatile uint8_t *)y = (uint8_t)(cy + n); break;
    case 1: *(volatile uint8_t *)x = (uint8_t)(cx + n), *(volatile uint8_t *)y = (uint8_t)(cy - 1); break;
    case 2: *(volatile uint8_t *)x = (uint8_t)(cx - 1), *(volatile uint8_t *)y = (uint8_t)(cy + n); break;
    default: *(volatile uint8_t *)x = (uint8_t)(cx + n), *(volatile uint8_t *)y = (uint8_t)(cy + 1); break;
    }
}
static const uint8_t rect_side_cell_original[] = {0x8b, 0x44, 0x24, 0x04, 0x3b, 0x81, 0xec, 0x01};
REWRITE("Rect::SideCell", 0xbad250, rect_side_cell_original, &rect_side_cell, &g_side_cells);

// ---- Object::SetHeld(this, on) (0xC5E9F0): a held mode (+0x110) switched -----------------------------------------------
// The byte is set to the value given; when that differs from what it was, nothing more. Otherwise, on: the flags (+0x1C) are kept at +0x120 and become (flags without bits 0-1) | 0x180;
// the values at +0xD8, +0xF0, +0xF4 are kept at +0x114 to +0x11C; the partner's (+0x104) +0xD8 and this one's are
// cleared. Off: all of it put back (the partner's +0xD8 too).
static volatile long g_held_switches;
static void __thiscall object_set_held(uint8_t *self, uint32_t on_arg) {
    g_held_switches++;
    volatile uint32_t *const s = (volatile uint32_t *)self;
    const uint8_t on = (uint8_t)on_arg, old = self[0x110];
    ((volatile uint8_t *)self)[0x110] = on;
    if (on != old)   // (as the game has it: a change does nothing more; the same value given again does the switch)
        return;
    if (on) {
        const uint32_t flags = s[0x1c / 4];
        s[0x120 / 4] = flags;
        s[0x1c / 4] = (flags & 0xfffffffc) | 0x180;
        s[0x114 / 4] = s[0xd8 / 4];
        const uint32_t f0 = s[0xf0 / 4], f4 = s[0xf4 / 4];
        s[0x118 / 4] = f0;
        s[0x11c / 4] = f4;
        ((volatile uint32_t *)(uintptr_t)s[0x104 / 4])[0xd8 / 4] = 0;
        s[0xd8 / 4] = 0;
    } else {
        s[0x1c / 4] = s[0x120 / 4];
        ((volatile uint32_t *)(uintptr_t)s[0x104 / 4])[0xd8 / 4] = s[0x114 / 4];
        s[0xd8 / 4] = s[0x114 / 4];
        s[0xf0 / 4] = s[0x118 / 4];
        s[0xf4 / 4] = s[0x11c / 4];
    }
}
static const uint8_t object_set_held_original[] = {0x8a, 0x91, 0x10, 0x01, 0x00, 0x00, 0x8a, 0x44};
REWRITE("Object::SetHeld", 0xc5e9f0, object_set_held_original, &object_set_held, &g_held_switches);

// ---- Grid::RotateCoords(this, x, y, x_out, y_out) (0x838C00): a cell's coordinates turned by the grid's rotation -------
// With a grid (+0x16C; width +0x10, height +0x14) and a rotation (+0x17C) of 0-3: 0 keeps (x, y), 1 gives
// (y, width - x - 1), 2 (width - x - 1, height - y - 1), 3 (height - y - 1, x); otherwise nothing is written.
static volatile long g_grid_rotations;
static void __thiscall grid_rotate_coords(const uint8_t *self, int32_t x, int32_t y, int32_t *x_out, int32_t *y_out) {
    g_grid_rotations++;
    const int32_t *const grid = *(const int32_t *const *)(self + 0x16c);
    if (!grid || *(const uint32_t *)(self + 0x17c) > 3)
        return;
    volatile int32_t *const ox = x_out, *const oy = y_out;
    switch (*(const uint32_t *)(self + 0x17c)) {
    case 0: *ox = x, *oy = y; break;
    case 1: *ox = y, *oy = grid[4] - x - 1; break;
    case 2: *ox = grid[4] - x - 1, *oy = grid[5] - y - 1; break;
    default: *ox = grid[5] - y - 1, *oy = x; break;
    }
}
static const uint8_t grid_rotate_coords_original[] = {0x8b, 0x91, 0x6c, 0x01, 0x00, 0x00, 0x85, 0xd2};
REWRITE("Grid::RotateCoords", 0x838c00, grid_rotate_coords_original, &grid_rotate_coords, &g_grid_rotations);

// ---- Matrix::FacingRotation(out, facing) (0x585870): the 4x4 matrix for one of four facings ----------------------------
// Quarter turns about the vertical: facing 1 puts (0, 0, 1 / -1, 0, 0) in the first and third rows' x and z, 2 negates
// both, 3 is 1's opposite; 0 (or anything over 3) is the identity.
static volatile long g_facing_rotations;
static void __stdcall matrix_facing_rotation(uint32_t *out, uint32_t facing) {
    g_facing_rotations++;
    const uint32_t one = 0x3f800000, minus = 0xbf800000;
    uint32_t xx = one, xz = 0, zx = 0, zz = one;
    if (facing == 1) xx = 0, xz = one, zx = minus, zz = 0;
    if (facing == 2) xx = minus, zz = minus;
    if (facing == 3) xx = 0, xz = minus, zx = one, zz = 0;
    volatile uint32_t *const o = out;
    for (int i = 0; i < 16; i++) o[i] = (i == 5 || i == 15) ? one : 0;
    o[0] = xx, o[2] = xz, o[8] = zx, o[10] = zz;
}
static const uint8_t matrix_facing_rotation_original[] = {0x8b, 0x44, 0x24, 0x08, 0x83, 0xf8, 0x03, 0x77};
REWRITE("Matrix::FacingRotation", 0x585870, matrix_facing_rotation_original, &matrix_facing_rotation, &g_facing_rotations);

// ---- Table::Map16(n, flag) (0xA3FB60): a number (0-7) through one of two fixed eight-entry tables ---------------------
// The second when the flag (a byte) is set.
static volatile long g_map16s;
static int32_t __stdcall table_map16(int32_t n, uint32_t flag) {
    g_map16s++;
    static const int32_t tables[16] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 2, 3, 8, 5, 6, 9, 7};
    return tables[n + ((flag & 0xff) ? 8 : 0)];
}
static const uint8_t table_map16_original[] = {0x83, 0xec, 0x40, 0xb9, 0x06, 0x00, 0x00, 0x00};
REWRITE("Table::Map16", 0xa3fb60, table_map16_original, &table_map16, &g_map16s);

// ---- Grid3::Rotate / RotateBack(x, y, rotation, x_out, y_out) (0x8750C0 / 0x875020): a place in a 3x3 grid turned -----
// Rotate: 0 keeps (x, y), 1 gives (2 - y, x), 2 (2 - x, 2 - y), 3 (y, 2 - x); RotateBack turns the other way (1 and 3
// swapped). Any other rotation writes nothing.
static volatile long g_grid3_rotations;
static void grid3_turn(int32_t x, int32_t y, uint32_t rotation, int32_t *x_out, int32_t *y_out) {
    volatile int32_t *const ox = x_out, *const oy = y_out;
    switch (rotation) {
    case 0: *ox = x, *oy = y; break;
    case 1: *ox = 2 - y, *oy = x; break;
    case 2: *ox = 2 - x, *oy = 2 - y; break;
    case 3: *ox = y, *oy = 2 - x; break;
    default: break;
    }
}
static void __stdcall grid3_rotate(int32_t x, int32_t y, uint32_t rotation, int32_t *x_out, int32_t *y_out) {
    g_grid3_rotations++;
    grid3_turn(x, y, rotation, x_out, y_out);
}
static void __stdcall grid3_rotate_back(int32_t x, int32_t y, uint32_t rotation, int32_t *x_out, int32_t *y_out) {
    g_grid3_rotations++;
    grid3_turn(x, y, rotation == 1 ? 3 : rotation == 3 ? 1 : rotation, x_out, y_out);
}
static const uint8_t grid3_rotate_original[] = {0x8b, 0x44, 0x24, 0x0c, 0x83, 0xf8, 0x03, 0x77};
REWRITE("Grid3::Rotate", 0x8750c0, grid3_rotate_original, &grid3_rotate, &g_grid3_rotations);
REWRITE("Grid3::RotateBack", 0x875020, grid3_rotate_original, &grid3_rotate_back, &g_grid3_rotations);

// ---- Request::Request(this, owner, a, b, c, flag, value, d, e) (0xC51870): a request record ----------------------------
// Put first in its owner's list (the list head the owner's +0x10 points to; next at +0, previous at +4); +8 itself, +0x10
// cleared, +0x14 a, +0x18 c and +0x1C c + b, +0x20 e, +0x24 the flag, +0x28 *value, +0x2C d; +0xC the next number of the
// game's request count (0x13E6C94; 0 is skipped: it wraps to 1).
static volatile long g_requests;
static uint32_t *__thiscall request_request(uint32_t *self, const uint8_t *owner, uint32_t a, uint32_t b, uint32_t c, uint32_t flag, const uint32_t *value,
                                            uint32_t d, uint32_t e) {
    g_requests++;
    volatile uint32_t *const t = self;
    t[5] = a;
    t[2] = (uint32_t)(uintptr_t)self;
    t[4] = 0, t[6] = 0, t[7] = 0;
    t[8] = e;
    ((volatile uint8_t *)self)[0x24] = (uint8_t)flag;
    t[10] = *value;
    t[11] = d;
    volatile uint32_t *const list = *(volatile uint32_t *const *)(owner + 0x10);
    t[0] = list[0];
    ((volatile uint32_t *)(uintptr_t)list[0])[1] = (uint32_t)(uintptr_t)self;
    t[1] = (uint32_t)(uintptr_t)list;
    list[0] = (uint32_t)(uintptr_t)self;
    t[6] = c, t[7] = c;
    t[7] = t[7] + b;
    volatile uint32_t *const count = (volatile uint32_t *)0x13e6c94;
    if (++*count == 0) *count = 1;
    t[3] = *count;
    return self;
}
static const uint8_t request_request_original[] = {0x8b, 0x54, 0x24, 0x08, 0x8b, 0xc1, 0x89, 0x50};
REWRITE("Request::Request", 0xc51870, request_request_original, &request_request, &g_requests);

// ---- Record::Copy485690(this, from) (0x485690): a record copied, its shared part counted once more ----------------------
// +0 the shared part (its count, its first value, raised), +0x10 to +0x58 copied as they are; given back.
static volatile long g_record_copies;
static uint32_t *__thiscall record_copy_485690(uint32_t *self, const uint32_t *from) {
    g_record_copies++;
    volatile uint32_t *const t = self;
    t[0] = from[0];
    (*(volatile uint32_t *)(uintptr_t)from[0])++;
    for (int i = 4; i <= 0x58 / 4; i++) t[i] = from[i];
    return self;
}
static const uint8_t record_copy_485690_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x11};
REWRITE("Record::Copy485690", 0x485690, record_copy_485690_original, &record_copy_485690, &g_record_copies);

// ---- Object3CC90::Object3CC90(this, owner, m) (0x58A6C0): the table 0xF3CC90, the owner, +8 cleared, a 4x4 matrix ----
static volatile long g_object_3cc90s;
static uint32_t *__thiscall object_3cc90(uint32_t *self, uint32_t owner, const uint32_t *m) {
    g_object_3cc90s++;
    volatile uint32_t *const t = self;
    t[1] = owner;
    t[0] = 0xf3cc90;
    t[2] = 0;
    for (int i = 0; i < 16; i++) t[4 + i] = m[i];
    return self;
}
static const uint8_t object_3cc90_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x89, 0x48};
REWRITE("Object3CC90::Object3CC90", 0x58a6c0, object_3cc90_original, &object_3cc90, &g_object_3cc90s);

// ---- Object::SetMatrixAndMore(this, -, m, v) (0xBBD210): a 4x4 matrix into +0x10 and three more values into +0x50 -----
static volatile long g_matrix_sets;
static void __thiscall object_set_matrix_and_more(uint32_t *self, uint32_t, const uint32_t *m, const uint32_t *v) {
    g_matrix_sets++;
    volatile uint32_t *const t = self;
    for (int i = 0; i < 16; i++) t[4 + i] = m[i];
    t[0x50 / 4] = v[0], t[0x54 / 4] = v[1], t[0x58 / 4] = v[2];
}
static const uint8_t object_set_matrix_and_more_original[] = {0x8b, 0x44, 0x24, 0x08, 0x8b, 0x10, 0x89, 0x51};
REWRITE("Object::SetMatrixAndMore", 0xbbd210, object_set_matrix_and_more_original, &object_set_matrix_and_more, &g_matrix_sets);

// ---- Range::Clip(this, out, other) (0xC517A0): the overlap of two ranges {start, end} (signed) --------------------------
// start = the larger start, end = the smaller end but not below start (an empty range at the start when they miss).
static volatile long g_range_clips;
static void __thiscall range_clip(const int32_t *self, int32_t *out, const int32_t *other) {
    g_range_clips++;
    const int32_t start = self[0] < other[0] ? other[0] : self[0];
    const int32_t end0 = self[1] > other[1] ? other[1] : self[1];
    const int32_t end = start < end0 ? end0 : start;
    ((volatile int32_t *)out)[0] = start;
    ((volatile int32_t *)out)[1] = end;
}
static const uint8_t range_clip_original[] = {0x83, 0xec, 0x10, 0x8b, 0x54, 0x24, 0x18, 0x8b};
REWRITE("Range::Clip", 0xc517a0, range_clip_original, &range_clip, &g_range_clips);

// ---- Footprint::RotateCell(add_x, add_y, x, y, x_out, y_out, turn, footprint) (0x8275D0): cell coordinates turned ------
// Within the footprint (width +0x10, depth +0x14): turn 0 keeps (x, y), 1 gives (y, width - x - 1), 2 (width - x - 1,
// depth - y - 1), 3 (depth - y - 1, x); then add_x and add_y are added to the outputs (for any other turn the outputs
// are only added to).
static volatile long g_footprint_turns;
static void __fastcall footprint_rotate_cell(int32_t add_x, int32_t add_y, int32_t x, int32_t y, int32_t *x_out, int32_t *y_out, uint32_t turn,
                                             const int32_t *footprint) {
    g_footprint_turns++;
    volatile int32_t *const ox = x_out, *const oy = y_out;
    switch (turn) {
    case 0: *ox = x, *oy = y; break;
    case 1: *ox = y, *oy = footprint[4] - x - 1; break;
    case 2: *ox = footprint[4] - x - 1, *oy = footprint[5] - y - 1; break;
    case 3: *ox = footprint[5] - y - 1, *oy = x; break;
    default: break;
    }
    *ox += add_x;
    *oy += add_y;
}
static const uint8_t footprint_rotate_cell_original[] = {0x8b, 0x44, 0x24, 0x10, 0x53, 0x56, 0x8b, 0x74};
REWRITE("Footprint::RotateCell", 0x8275d0, footprint_rotate_cell_original, &footprint_rotate_cell, &g_footprint_turns);

// ---- Object::PartMatrix(this, out) (0xA637E0): the matrix of the object's part (+0xE0, its matrix at +0x60) copied out
static volatile long g_part_matrices;
static void __thiscall object_part_matrix(const uint8_t *self, uint32_t *out) {
    g_part_matrices++;
    const uint32_t *const m = (const uint32_t *)(*(const uint8_t *const *)(self + 0xe0) + 0x60);
    for (int i = 0; i < 16; i++) ((volatile uint32_t *)out)[i] = m[i];
}
static const uint8_t object_part_matrix_original[] = {0x8b, 0x89, 0xe0, 0x00, 0x00, 0x00, 0x8b, 0x51};
REWRITE("Object::PartMatrix", 0xa637e0, object_part_matrix_original, &object_part_matrix, &g_part_matrices);

// ---- Window::PartFor(this, n) (0x619890): one of ten parts of a window by number, or null -------------------------------
// The parts sit 8 bytes apart from +0x13C, but numbers 6, 7 and 8 take +0x174, +0x17C and +0x16C.
static volatile long g_window_parts;
static uint32_t __thiscall window_part_for(const uint8_t *self, uint32_t n) {
    g_window_parts++;
    static const uint16_t at[10] = {0x13c, 0x144, 0x14c, 0x154, 0x15c, 0x164, 0x174, 0x17c, 0x16c, 0x184};
    return n <= 9 ? *(const uint32_t *)(self + at[n]) : 0;
}
static const uint8_t window_part_for_original[] = {0x8b, 0x44, 0x24, 0x04, 0x83, 0xf8, 0x09, 0x77};
REWRITE("Window::PartFor", 0x619890, window_part_for_original, &window_part_for, &g_window_parts);

// ---- Record::CopyCC6200(this, from) (0xCC6200): sixteen values copied into the record's part (+4) -----------------------
static volatile long g_record_cc6200s;
static void __thiscall record_copy_cc6200(const uint8_t *self, const uint32_t *from) {
    g_record_cc6200s++;
    volatile uint32_t *const to = *(volatile uint32_t *const *)(self + 4);
    for (int i = 0; i < 16; i++) to[i] = from[i];
}
static const uint8_t record_copy_cc6200_original[] = {0x8b, 0x41, 0x04, 0x8b, 0x4c, 0x24, 0x04, 0x8b};
REWRITE("Record::CopyCC6200", 0xcc6200, record_copy_cc6200_original, &record_copy_cc6200, &g_record_cc6200s);

// ---- Heap::Swap(this, n) (0x49A740): entry n swapped with the last entry of a heap list ---------------------------------
// The entries are 16 bytes in the shared table (*0x12F46DC's first value), each with its slot at +0xC. The list (+0x18)
// holds entry numbers and the places (+0x14) are by slot; count at +0x10. The last entry's place and entry n's are
// swapped, the last entry takes n's slot, the list at that slot gets the last entry, and the count drops by one.
static volatile long g_heap_swaps;
static void __thiscall heap_swap(uint8_t *self, uint32_t n) {
    g_heap_swaps++;
    uint8_t *const base = **(uint8_t *const *const *)0x12f46dc;
    volatile uint32_t *const count = (volatile uint32_t *)(self + 0x10);
    const uint32_t last = (*(volatile uint32_t *const *)(self + 0x18))[*count - 1];
    volatile uint32_t *const entry_n = (volatile uint32_t *)(base + n * 16), *const entry_last = (volatile uint32_t *)(base + last * 16);
    volatile uint32_t *const places = *(volatile uint32_t *const *)(self + 0x14);
    volatile uint32_t *const at_n = &places[entry_n[3]];
    const uint32_t saved = *at_n;
    *at_n = places[entry_last[3]];
    (*(volatile uint32_t *const *)(self + 0x14))[entry_last[3]] = saved;
    entry_last[3] = entry_n[3];
    (*(volatile uint32_t *const *)(self + 0x18))[entry_n[3]] = last;
    *count = *count - 1;
}
static const uint8_t heap_swap_original[] = {0xa1, 0xdc, 0x46, 0x2f, 0x01, 0x8b, 0x51, 0x10};
REWRITE("Heap::Swap", 0x49a740, heap_swap_original, &heap_swap, &g_heap_swaps);

// ---- ResourceRefFA57D0 / ResourceRefFA5798 constructors (0xBEBEE0, 0xBEB9A0): a reference joining its owner's list -----
// (this, owner, b, c, d, e, kind): +0 the table, +4 the word 1, +6 kind, +8 d, +0xC e, +0x10 c, +0x14 owner, +0x18 and
// +0x20 cleared, +0x1C b, +0x2C itself; its link (+0x24: next, previous) put first in the list whose head the owner holds
// at +0x244 (or +0x214).
static inline uint32_t *resource_ref_construct(uint32_t *self, uint32_t table, uint32_t list_at, uint8_t *owner, uint32_t b, uint32_t c, uint32_t d, uint32_t e,
                                               uint16_t kind) {
    volatile uint32_t *const t = self;
    ((volatile uint16_t *)self)[3] = kind;
    t[2] = d, t[3] = e;
    t[5] = (uint32_t)(uintptr_t)owner;
    t[4] = c;
    t[7] = b;
    ((volatile uint16_t *)self)[2] = 1;
    t[0] = table;
    t[6] = 0, t[8] = 0;
    t[11] = (uint32_t)(uintptr_t)self;
    volatile uint32_t *const list = *(volatile uint32_t *const *)((uint8_t *)(uintptr_t)t[5] + list_at);
    volatile uint32_t *const link = t + 9;
    link[0] = list[0];
    ((volatile uint32_t *)(uintptr_t)list[0])[1] = (uint32_t)(uintptr_t)link;
    link[1] = (uint32_t)(uintptr_t)list;
    list[0] = (uint32_t)(uintptr_t)link;
    return self;
}
static volatile long g_resource_refs;
static uint32_t *__thiscall resource_ref_fa57d0(uint32_t *self, uint8_t *owner, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t kind) {
    g_resource_refs++;
    return resource_ref_construct(self, 0xfa57d0, 0x244, owner, b, c, d, e, (uint16_t)kind);
}
static uint32_t *__thiscall resource_ref_fa5798(uint32_t *self, uint8_t *owner, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t kind) {
    g_resource_refs++;
    return resource_ref_construct(self, 0xfa5798, 0x214, owner, b, c, d, e, (uint16_t)kind);
}
static const uint8_t resource_ref_original[] = {0x8b, 0x54, 0x24, 0x10, 0x8b, 0xc1, 0x66, 0x8b};
REWRITE("ResourceRefFA57D0::ResourceRefFA57D0", 0xbebee0, resource_ref_original, &resource_ref_fa57d0, &g_resource_refs);
REWRITE("ResourceRefFA5798::ResourceRefFA5798", 0xbeb9a0, resource_ref_original, &resource_ref_fa5798, &g_resource_refs);

// ---- Object::KindCode(this) (0xAAD7F0): a code for the object's kind (+0x120) -------------------------------------------
// Kinds 2 and 3: the value at +0x124 plus 0x10, at most 0x30 (signed); 5: 0x30; 6, 8 and 9: 0x2F; 7: 0x2E; others 0x10.
static volatile long g_kind_codes;
static uint32_t __thiscall object_kind_code(const uint8_t *self) {
    g_kind_codes++;
    switch (*(const uint32_t *)(self + 0x120)) {
    case 2: case 3: {
        const int32_t v = *(const int32_t *)(self + 0x124) + 0x10;
        return 0x30 > v ? (uint32_t)v : 0x30;
    }
    case 5: return 0x30;
    case 6: case 8: case 9: return 0x2f;
    case 7: return 0x2e;
    default: return 0x10;
    }
}
static const uint8_t object_kind_code_original[] = {0x8b, 0x81, 0x20, 0x01, 0x00, 0x00, 0x83, 0xc0};
REWRITE("Object::KindCode", 0xaad7f0, object_kind_code_original, &object_kind_code, &g_kind_codes);

// ---- Object::SetKind(this, kind) (0xA93990): a kind (+0x144) and from it a group (+0x140) --------------------------------
// -1: 1; 0 and 1: 2; 5: 4; 2 to 4 and 6 to 9: 3; anything else 0.
static volatile long g_kind_sets;
static void __thiscall object_set_kind(uint8_t *self, int32_t kind) {
    g_kind_sets++;
    *(volatile int32_t *)(self + 0x144) = kind;
    uint32_t group;
    switch (kind) {
    case -1: group = 1; break;
    case 0: case 1: group = 2; break;
    case 5: group = 4; break;
    case 2: case 3: case 4: case 6: case 7: case 8: case 9: group = 3; break;
    default: group = 0; break;
    }
    *(volatile uint32_t *)(self + 0x140) = group;
}
static const uint8_t object_set_kind_original[] = {0x8b, 0x44, 0x24, 0x04, 0x89, 0x81, 0x44, 0x01};
REWRITE("Object::SetKind", 0xa93990, object_set_kind_original, &object_set_kind, &g_kind_sets);

// ---- Mesh::PointAndNormal(this, point, position, normal) (0x489A10): a mesh point's position and normal ------------------
// Nothing without a mesh (*this) or when the point's +0xC is negative. The point's number (+0x10): part in the high half
// (28-byte parts at the mesh's +8, their points at +0x18), point in the low half (32 bytes each: position, then normal).
static volatile long g_mesh_points;
static void __thiscall mesh_point_and_normal(const uint8_t *const *self, const uint8_t *point, uint32_t *position, uint32_t *normal) {
    g_mesh_points++;
    const uint8_t *const mesh = *self;
    if (!mesh || *(const int32_t *)(point + 0xc) < 0)
        return;
    const uint32_t id = *(const uint32_t *)(point + 0x10);
    const uint8_t *const parts = *(const uint8_t *const *)(mesh + 8);
    const uint32_t *const p = (const uint32_t *)(*(const uint8_t *const *)(parts + (id >> 16) * 0x1c + 0x18) + (id & 0xffff) * 32);
    ((volatile uint32_t *)position)[0] = p[0], ((volatile uint32_t *)position)[1] = p[1], ((volatile uint32_t *)position)[2] = p[2];
    ((volatile uint32_t *)normal)[0] = p[3], ((volatile uint32_t *)normal)[1] = p[4], ((volatile uint32_t *)normal)[2] = p[5];
}
static const uint8_t mesh_point_and_normal_original[] = {0x8b, 0x09, 0x85, 0xc9, 0x74, 0x59, 0x8b, 0x44};
REWRITE("Mesh::PointAndNormal", 0x489a10, mesh_point_and_normal_original, &mesh_point_and_normal, &g_mesh_points);

// ---- Animations::SetMove(this, n, from, to, w) (0x94DE60): animation n (208 bytes at *(+0x1A4)) moving between points ---
// +0x60 mode 3, +0x70 from, +0x90 to (three values each), +0xA0 w.
static volatile long g_animation_moves;
static void __thiscall animations_set_move(const uint8_t *self, int32_t n, const uint32_t *from, const uint32_t *to, uint32_t w) {
    g_animation_moves++;
    volatile uint32_t *const e = (volatile uint32_t *)(*(uint8_t *const *)(self + 0x1a4) + n * 0xd0);
    e[0x60 / 4] = 3;
    e[0x70 / 4] = from[0], e[0x74 / 4] = from[1], e[0x78 / 4] = from[2];
    e[0x90 / 4] = to[0], e[0x94 / 4] = to[1], e[0x98 / 4] = to[2];
    e[0xa0 / 4] = w;
}
static const uint8_t animations_set_move_original[] = {0x8b, 0x91, 0xa4, 0x01, 0x00, 0x00, 0x8b, 0x44};
REWRITE("Animations::SetMove", 0x94de60, animations_set_move_original, &animations_set_move, &g_animation_moves);

// ---- Request::CopyFrom(this, owner, from) (0xC51260): a request record made from another --------------------------------
// +8 itself, +0x10 cleared, +0xC and +0x14 to +0x2C copied (+0x24 a byte); put first in the owner's list (+0x10).
static uint32_t *__thiscall request_copy_from(uint32_t *self, const uint8_t *owner, const uint32_t *from) {
    g_requests++;
    volatile uint32_t *const t = self;
    t[2] = (uint32_t)(uintptr_t)self;
    t[3] = from[3];
    t[4] = 0;
    t[5] = from[5], t[6] = from[6], t[7] = from[7], t[8] = from[8];
    ((volatile uint8_t *)self)[0x24] = ((const uint8_t *)from)[0x24];
    t[10] = from[10], t[11] = from[11];
    volatile uint32_t *const list = *(volatile uint32_t *const *)(owner + 0x10);
    t[0] = list[0];
    ((volatile uint32_t *)(uintptr_t)list[0])[1] = (uint32_t)(uintptr_t)self;
    t[1] = (uint32_t)(uintptr_t)list;
    list[0] = (uint32_t)(uintptr_t)self;
    return self;
}
static const uint8_t request_copy_from_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x08, 0x89, 0x40};
REWRITE("Request::CopyFrom", 0xc51260, request_copy_from_original, &request_copy_from, &g_requests);

// ---- Object::CopyArea(this, other) (0xAAE2D0): another object's area (+0x148 to +0x154) copied when it has one -----------
// The flag byte +0x144 is set to 1 when there is one (and copied), else 0.
static volatile long g_area_copies;
static void __thiscall object_copy_area(uint8_t *self, const uint8_t *other) {
    g_area_copies++;
    if (!other || !other[0x144]) {
        ((volatile uint8_t *)self)[0x144] = 0;
        return;
    }
    ((volatile uint8_t *)self)[0x144] = 1;
    for (int i = 0; i < 4; i++) ((volatile uint32_t *)(self + 0x148))[i] = ((const uint32_t *)(other + 0x148))[i];
}
static const uint8_t object_copy_area_original[] = {0x8b, 0x44, 0x24, 0x04, 0x85, 0xc0, 0x74, 0x44};
REWRITE("Object::CopyArea", 0xaae2d0, object_copy_area_original, &object_copy_area, &g_area_copies);

// ---- ResourceRefFA5D28::ResourceRefFA5D28(this, owner, value, kind) (0xBFF8D0): a reference joining its owner's list ----
// +0 the table, +4 the word 1, +6 kind, +8 value, +0x14 and +0x30 itself, +0x1C owner, +0x18, +0x20, +0x24 cleared; its
// link (+0x28) put first in the list whose head the owner holds at +0xC4.
static uint32_t *__thiscall resource_ref_fa5d28(uint32_t *self, uint8_t *owner, uint32_t value, uint32_t kind) {
    g_resource_refs++;
    volatile uint32_t *const t = self;
    ((volatile uint16_t *)self)[3] = (uint16_t)kind;
    t[2] = value;
    ((volatile uint16_t *)self)[2] = 1;
    t[5] = (uint32_t)(uintptr_t)self;
    t[7] = (uint32_t)(uintptr_t)owner;
    t[0] = 0xfa5d28;
    t[6] = 0, t[8] = 0, t[9] = 0;
    t[12] = (uint32_t)(uintptr_t)self;
    volatile uint32_t *const list = *(volatile uint32_t *const *)((uint8_t *)(uintptr_t)t[7] + 0xc4);
    volatile uint32_t *const link = t + 10;
    link[0] = list[0];
    ((volatile uint32_t *)(uintptr_t)list[0])[1] = (uint32_t)(uintptr_t)link;
    link[1] = (uint32_t)(uintptr_t)list;
    list[0] = (uint32_t)(uintptr_t)link;
    return self;
}
static const uint8_t resource_ref_fa5d28_original[] = {0x8b, 0x54, 0x24, 0x08, 0x8b, 0xc1, 0x66, 0x8b};
REWRITE("ResourceRefFA5D28::ResourceRefFA5D28", 0xbff8d0, resource_ref_fa5d28_original, &resource_ref_fa5d28, &g_resource_refs);

// ---- AdMenu::ScaleLevel(a, b) (0x45E430): a level for the ad media from 8 a / b (signed division) ------------------------
// 1: 6, 2: 5, 4: 4, 16: 2, 32: 1, 64: 0, anything else 3.
static volatile long g_ad_levels;
static uint32_t __stdcall ad_menu_scale_level(int32_t a, int32_t b) {
    g_ad_levels++;
    switch ((int32_t)((uint32_t)a << 3) / b) {
    case 1: return 6;
    case 2: return 5;
    case 4: return 4;
    case 16: return 2;
    case 32: return 1;
    case 64: return 0;
    default: return 3;
    }
}
static const uint8_t ad_menu_scale_level_original[] = {0x8b, 0x44, 0x24, 0x04, 0xc1, 0xe0, 0x03, 0x99};
REWRITE("AdMenu::ScaleLevel", 0x45e430, ad_menu_scale_level_original, &ad_menu_scale_level, &g_ad_levels);

// ---- ResourceRefFA5CF0::ResourceRefFA5CF0(this, owner, value, kind) (0xBFF2B0): the same as ResourceRefFA5D28, joining
// the list whose head the owner holds at +0x94 ------------------------------------------------------------------------
static uint32_t *__thiscall resource_ref_fa5cf0(uint32_t *self, uint8_t *owner, uint32_t value, uint32_t kind) {
    g_resource_refs++;
    volatile uint32_t *const t = self;
    ((volatile uint16_t *)self)[3] = (uint16_t)kind;
    t[2] = value;
    ((volatile uint16_t *)self)[2] = 1;
    t[5] = (uint32_t)(uintptr_t)self;
    t[7] = (uint32_t)(uintptr_t)owner;
    t[0] = 0xfa5cf0;
    t[6] = 0, t[8] = 0, t[9] = 0;
    t[12] = (uint32_t)(uintptr_t)self;
    volatile uint32_t *const list = *(volatile uint32_t *const *)(owner + 0x94);
    volatile uint32_t *const link = t + 10;
    link[0] = list[0];
    ((volatile uint32_t *)(uintptr_t)list[0])[1] = (uint32_t)(uintptr_t)link;
    link[1] = (uint32_t)(uintptr_t)list;
    list[0] = (uint32_t)(uintptr_t)link;
    return self;
}
static const uint8_t resource_ref_fa5cf0_original[] = {0x8b, 0x54, 0x24, 0x08, 0x8b, 0xc1, 0x66, 0x8b};
REWRITE("ResourceRefFA5CF0::ResourceRefFA5CF0", 0xbff2b0, resource_ref_fa5cf0_original, &resource_ref_fa5cf0, &g_resource_refs);

// ---- PackedSlots::Set(this, id, v, a, b, c, d) (0x9CEDB0): the values of a slot found by a packed number ----------------
// Block id >> 20 (signed; its slots at *(this + (block + 12) 16)), slot id & 0xFFFFF (64 bytes each): +0x10 v's three
// values, +0x20 a, +0x24 b, +0x28 c, +0x2C d.
static volatile long g_packed_slot_sets;
static void __thiscall packed_slots_set(const uint8_t *self, int32_t id, const uint32_t *v, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    g_packed_slot_sets++;
    uint8_t *const base = *(uint8_t *const *)(self + (uint32_t)((id >> 20) + 0xc) * 16);
    volatile uint32_t *const s = (volatile uint32_t *)(base + ((uint32_t)id & 0xfffff) * 64);
    s[4] = v[0], s[5] = v[1], s[6] = v[2];
    s[8] = a, s[9] = b, s[10] = c, s[11] = d;
}
static const uint8_t packed_slots_set_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0xd0, 0xc1, 0xfa};
REWRITE("PackedSlots::Set", 0x9cedb0, packed_slots_set_original, &packed_slots_set, &g_packed_slot_sets);

// ---- Walker::Step(w) (0x713650): a position (+4, +8) moved one cell in its direction (+0x28), its height (+0x2C) by
// its slope kind (+0x24) ------------------------------------------------------------------------------------------------
// Direction 0: x - 1, 1: x + 1, 2: y - 1, 3: y + 1. Slope 1: height + 2, 2: - 2, 4: + 1, 5: - 1.
static volatile long g_walker_steps;
static void __stdcall walker_step(uint8_t *w) {
    g_walker_steps++;
    volatile int32_t *const t = (volatile int32_t *)w;
    switch (t[0x28 / 4]) {
    case 0: t[1] = t[1] - 1; break;
    case 1: t[1] = t[1] + 1; break;
    case 2: t[2] = t[2] - 1; break;
    case 3: t[2] = t[2] + 1; break;
    default: break;
    }
    switch (t[0x24 / 4]) {
    case 1: t[0x2c / 4] = t[0x2c / 4] + 2; break;
    case 2: t[0x2c / 4] = t[0x2c / 4] - 2; break;
    case 4: t[0x2c / 4] = t[0x2c / 4] + 1; break;
    case 5: t[0x2c / 4] = t[0x2c / 4] - 1; break;
    default: break;
    }
}
static const uint8_t walker_step_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x48, 0x28, 0x83};
REWRITE("Walker::Step", 0x713650, walker_step_original, &walker_step, &g_walker_steps);

// ---- Record::Init633B30(this, v) (0x633B30): a record set up and joined to the game's list (0x10F0D70) -----------------
// An empty list at +4 (its +0x10 and +0x14 pointing back), +0x24 itself, +0x28 v; its link (+0x1C) put last in the
// game's circular list (next at +0, previous at +4); the table 0xF47F0C, then 0xF48054.
static volatile long g_record_633b30s;
static uint32_t *__thiscall record_init_633b30(uint32_t *self, uint32_t v) {
    g_record_633b30s++;
    volatile uint32_t *const t = self;
    volatile uint32_t *const c = t + 1;
    c[2] = 0, c[5] = 0, c[1] = 0, c[3] = 0;
    c[4] = (uint32_t)(uintptr_t)c;
    c[0] = (uint32_t)(uintptr_t)(c + 3);
    t[0] = 0xf47f0c;
    t[9] = (uint32_t)(uintptr_t)self;
    volatile uint32_t *const list = *(volatile uint32_t *const *)0x10f0d70;
    volatile uint32_t *const link = t + 7;
    link[1] = list[1];
    *(volatile uint32_t *)(uintptr_t)list[1] = (uint32_t)(uintptr_t)link;
    link[0] = (uint32_t)(uintptr_t)list;
    list[1] = (uint32_t)(uintptr_t)link;
    t[0] = 0xf48054;
    t[10] = v;
    return self;
}
static const uint8_t record_init_633b30_original[] = {0x8b, 0xc1, 0x8d, 0x48, 0x04, 0x56, 0x33, 0xf6};
REWRITE("Record::Init633B30", 0x633b30, record_init_633b30_original, &record_init_633b30, &g_record_633b30s);

// ---- Ride::Offset(this, out) (0x5061B0): the ride's offset (+0x1E0) for kinds 2 and 3 (+0x1C8), or none ----------------
static volatile long g_ride_offsets;
static void __thiscall ride_offset(const uint8_t *self, uint32_t *out) {
    g_ride_offsets++;
    const uint32_t kind = *(const uint32_t *)(self + 0x1c8);
    static const uint32_t none[3] = {0, 0, 0};
    const uint32_t *const from = kind == 2 || kind == 3 ? (const uint32_t *)(self + 0x1e0) : none;
    ((volatile uint32_t *)out)[0] = from[0], ((volatile uint32_t *)out)[1] = from[1], ((volatile uint32_t *)out)[2] = from[2];
}
static const uint8_t ride_offset_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Ride::Offset", 0x5061b0, ride_offset_original, &ride_offset, &g_ride_offsets);

// ---- Object7FC84::Object7FC84(this, owner, p, v, f1, f2, f3) (0x9883D0): table 0xF7FC84, size 0xF0 -----------------------
// +0xC owner, +8 cleared, +0x10 the point's three values, +0x20 v, bytes +0x24 to +0x26 the flags.
static volatile long g_object_7fc84s;
static uint32_t *__thiscall object_7fc84(uint32_t *self, uint32_t owner, const uint32_t *p, uint32_t v, uint32_t f1, uint32_t f2, uint32_t f3) {
    g_object_7fc84s++;
    volatile uint32_t *const t = self;
    t[3] = owner;
    t[1] = 0xf0;
    t[2] = 0;
    t[0] = 0xf7fc84;
    t[4] = p[0], t[5] = p[1], t[6] = p[2];
    t[8] = v;
    ((volatile uint8_t *)self)[0x24] = (uint8_t)f1, ((volatile uint8_t *)self)[0x25] = (uint8_t)f2, ((volatile uint8_t *)self)[0x26] = (uint8_t)f3;
    return self;
}
static const uint8_t object_7fc84_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x89, 0x48};
REWRITE("Object7FC84::Object7FC84", 0x9883d0, object_7fc84_original, &object_7fc84, &g_object_7fc84s);

// ---- HandleRef::FromRefBit(this, other, n) (0x7A7040): a reference (table 0xF26AE0) to another's handle, with bit n ----
// +0x10 itself, +4 the handle (other's +4; its count +8 raised), its link (+8) put first in the handle's list (its +0x20
// points at the head), +0x14 1 << n, and the 0x37 values from +0x18 cleared.
static volatile long g_handle_ref_bits;
static uint32_t *__thiscall handle_ref_from_ref_bit(uint32_t *self, const uint32_t *other, uint32_t n) {
    g_handle_ref_bits++;
    volatile uint32_t *const t = self;
    t[0] = 0xf26ae0;
    t[4] = (uint32_t)(uintptr_t)self;
    volatile uint32_t *const h = (volatile uint32_t *)(uintptr_t)other[1];
    h[2] = h[2] + 1;
    t[1] = (uint32_t)(uintptr_t)h;
    volatile uint32_t *const list = (volatile uint32_t *)(uintptr_t)h[8];
    volatile uint32_t *const link = t + 2;
    link[0] = list[0];
    ((volatile uint32_t *)(uintptr_t)list[0])[1] = (uint32_t)(uintptr_t)link;
    link[1] = (uint32_t)(uintptr_t)list;
    list[0] = (uint32_t)(uintptr_t)link;
    t[5] = 1u << (n & 31);
    for (int i = 0; i < 0x37; i++) t[6 + i] = 0;
    return self;
}
static const uint8_t handle_ref_from_ref_bit_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0xd1, 0xc7, 0x02};
REWRITE("HandleRef::FromRefBit", 0x7a7040, handle_ref_from_ref_bit_original, &handle_ref_from_ref_bit, &g_handle_ref_bits);

// ---- Batches::InitC9C3C0(this, owner) (0xC9C3C0): a record set up and joined to its owner's list -----------------------
// +0 owner, +0xC itself, an empty list at +0x90 (its +0x10 back to it, +0 to its +0xC), +0xA8 cleared; its link (+4) put
// first in the list whose head the owner holds at +0x14; the owner's count (+0x60) goes up by one.
static volatile long g_batch_inits;
static uint32_t *__thiscall batches_init_c9c3c0(uint32_t *self, uint8_t *owner) {
    g_batch_inits++;
    volatile uint32_t *const t = self;
    t[0] = (uint32_t)(uintptr_t)owner;
    t[3] = (uint32_t)(uintptr_t)self;
    volatile uint32_t *const c = t + 0x90 / 4;
    c[2] = 0, c[5] = 0, c[1] = 0;
    c[4] = (uint32_t)(uintptr_t)c;
    c[3] = 0;
    c[0] = (uint32_t)(uintptr_t)(c + 3);
    t[0xa8 / 4] = 0;
    volatile uint32_t *const list = *(volatile uint32_t *const *)(owner + 0x14);
    volatile uint32_t *const link = t + 1;
    link[0] = list[0];
    ((volatile uint32_t *)(uintptr_t)list[0])[1] = (uint32_t)(uintptr_t)link;
    link[1] = (uint32_t)(uintptr_t)list;
    list[0] = (uint32_t)(uintptr_t)link;
    volatile uint32_t *const count = (volatile uint32_t *)(owner + 0x60);
    *count = *count + 1;
    return self;
}
static const uint8_t batches_init_c9c3c0_original[] = {0x8b, 0xc1, 0x56, 0x8b, 0x74, 0x24, 0x08, 0x89};
REWRITE("Batches::InitC9C3C0", 0xc9c3c0, batches_init_c9c3c0_original, &batches_init_c9c3c0, &g_batch_inits);

// ---- Segment3D::Segment3D(this, a, b, p, q) (0xC46E20): two values (+4, +8) and two points (+0xC, +0x18) ----------------
// The table 0xFB7158, then 0xFB715C.
static volatile long g_segment3ds;
static uint32_t *__thiscall segment3d(uint32_t *self, uint32_t a, uint32_t b, const uint32_t *p, const uint32_t *q) {
    g_segment3ds++;
    volatile uint32_t *const t = self;
    t[1] = a, t[2] = b;
    t[0] = 0xfb7158;
    t[3] = p[0], t[4] = p[1], t[5] = p[2];
    t[6] = q[0], t[7] = q[1], t[8] = q[2];
    t[0] = 0xfb715c;
    return self;
}
static const uint8_t segment3d_original[] = {0x8b, 0x54, 0x24, 0x08, 0x8b, 0xc1, 0x8b, 0x4c};
REWRITE("Segment3D::Segment3D", 0xc46e20, segment3d_original, &segment3d, &g_segment3ds);

// ---- Water::Entry94DE10(this, n, a, b) (0x94DE10): entry n (208 bytes at *(+0x1A4)) set to kind 2 with two points -------
static void __thiscall water_entry_94de10(const uint8_t *self, int32_t n, const uint32_t *a, const uint32_t *b) {
    g_animation_moves++;
    volatile uint32_t *const e = (volatile uint32_t *)(*(uint8_t *const *)(self + 0x1a4) + n * 0xd0);
    e[0x60 / 4] = 2;
    e[0x70 / 4] = a[0], e[0x74 / 4] = a[1], e[0x78 / 4] = a[2];
    e[0x80 / 4] = b[0], e[0x84 / 4] = b[1], e[0x88 / 4] = b[2];
}
static const uint8_t water_entry_94de10_original[] = {0x8b, 0x91, 0xa4, 0x01, 0x00, 0x00, 0x8b, 0x44};
REWRITE("Water::Entry94DE10", 0x94de10, water_entry_94de10_original, &water_entry_94de10, &g_animation_moves);

// ---- Guest::Take758590(this, n, kind) (0x758590): n taken from a count (+0) when there is enough (signed) ---------------
// Gives (in the low byte) 0 when n is more than the count; otherwise the count drops by n, +0x18 and the total for the
// kind (0: +0x10, 1: +4, 2: +8, 3: +0xC, 4: +0x14; none for others) rise by n, and 1.
static volatile long g_guest_takes;
static uint8_t __thiscall guest_take_758590(int32_t *self, int32_t n, uint32_t kind) {
    g_guest_takes++;
    volatile int32_t *const t = self;
    const int32_t count = t[0];
    if (n > count)
        return 0;
    t[0] = count - n;
    t[6] = t[6] + n;
    static const int8_t at[5] = {4, 1, 2, 3, 5};
    if (kind <= 4)
        t[at[kind]] = t[at[kind]] + n;
    return 1;
}
static const uint8_t guest_take_758590_original[] = {0x8b, 0x11, 0x8b, 0x44, 0x24, 0x04, 0x3b, 0xc2};
REWRITE("Guest::Take758590", 0x758590, guest_take_758590_original, &guest_take_758590, &g_guest_takes);

// ---- Window::Init65FB30(this, v) (0x65FB30): the same set-up as Record::Init633B30, table 0xF4DE64, +0x1C v, its link at
// +0x20 put last in the game's circular list at 0x10EECB0, +0x28 itself -----------------------------------------------
static volatile long g_window_65fb30s;
static void __thiscall window_init_65fb30(uint32_t *self, uint32_t v) {
    g_window_65fb30s++;
    volatile uint32_t *const t = self;
    volatile uint32_t *const c = t + 1;
    c[2] = 0, c[5] = 0, c[1] = 0;
    c[4] = (uint32_t)(uintptr_t)c;
    c[3] = 0;
    c[0] = (uint32_t)(uintptr_t)(c + 3);
    t[7] = v;
    t[0] = 0xf4de64;
    t[10] = (uint32_t)(uintptr_t)self;
    volatile uint32_t *const list = *(volatile uint32_t *const *)0x10eecb0;
    volatile uint32_t *const link = t + 8;
    link[1] = list[1];
    *(volatile uint32_t *)(uintptr_t)list[1] = (uint32_t)(uintptr_t)link;
    link[0] = (uint32_t)(uintptr_t)list;
    list[1] = (uint32_t)(uintptr_t)link;
}
static const uint8_t window_init_65fb30_original[] = {0x8b, 0xc1, 0x8d, 0x48, 0x04, 0x56, 0x33, 0xf6};
REWRITE("Window::Init65FB30", 0x65fb30, window_init_65fb30_original, &window_init_65fb30, &g_window_65fb30s);

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

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

// ---- Style::ColourFor(this, state, highlighted) (0x58FD30): one of six colours of a style (+0x1CC) ----------------------
// State 0: +0xC highlighted, else +0x18; 1: +0x10 or +0x1C; 2: +0x14 or +0x20; any other state 5.
static volatile long g_style_colours;
static uint32_t __thiscall style_colour_for(const uint8_t *self, uint32_t state, uint32_t highlighted) {
    g_style_colours++;
    if (state > 2)
        return 5;
    const uint8_t *const style = *(const uint8_t *const *)(self + 0x1cc);
    return *(const uint32_t *)(style + (highlighted ? 0xc : 0x18) + 4 * state);
}
static const uint8_t style_colour_for_original[] = {0x8b, 0x44, 0x24, 0x04, 0x83, 0xe8, 0x00, 0x74};
REWRITE("Style::ColourFor", 0x58fd30, style_colour_for_original, &style_colour_for, &g_style_colours);

// ---- SortedInts::Find(key, list, count, place, -) (0x7FB9C0): binary search of a sorted list of signed numbers ---------
// Gives (in the low byte) 1 and the place when *key is found; otherwise 0 and where it would go (0 for an empty list).
static volatile long g_sorted_int_finds;
static uint8_t __fastcall sorted_ints_find(const int32_t *key, const int32_t *list, int32_t count, int32_t *place, uint32_t) {
    g_sorted_int_finds++;
    if (count < 1) {
        *(volatile int32_t *)place = 0;
        return 0;
    }
    const int32_t k = *key;
    int32_t lo = 0, hi = count - 1;
    for (;;) {
        const int32_t mid = (hi + lo) >> 1;
        const int32_t v = list[mid];
        if (k < v) {
            if (mid > lo) {
                hi = mid - 1;
                continue;
            }
            *(volatile int32_t *)place = lo;
            return 0;
        }
        if (k == v) {
            *(volatile int32_t *)place = mid;
            return 1;
        }
        if (mid < hi) {
            lo = mid + 1;
            continue;
        }
        *(volatile int32_t *)place = hi + 1;
        return 0;
    }
}
static const uint8_t sorted_ints_find_original[] = {0x8b, 0x44, 0x24, 0x04, 0x83, 0xf8, 0x01, 0x7d};
REWRITE("SortedInts::Find", 0x7fb9c0, sorted_ints_find_original, &sorted_ints_find, &g_sorted_int_finds);

// ---- FlagSlots::Get / Set(this, flag[, value]) (0x5D11C0, 0x5D10F0): the slot of a flag (1, 2, 4, ... 0x200: slots 0-9) --
// Get gives -1 for any other flag; Set ignores it.
static inline int flag_slot(int32_t flag) {
    switch (flag) {
    case 1: return 0;
    case 2: return 1;
    case 4: return 2;
    case 8: return 3;
    case 0x10: return 4;
    case 0x20: return 5;
    case 0x40: return 6;
    case 0x80: return 7;
    case 0x100: return 8;
    case 0x200: return 9;
    default: return -1;
    }
}
static volatile long g_flag_slots;
static uint32_t __thiscall flag_slots_get(const uint32_t *self, int32_t flag) {
    g_flag_slots++;
    const int slot = flag_slot(flag);
    return slot < 0 ? 0xffffffffu : self[slot];
}
static void __thiscall flag_slots_set(uint32_t *self, int32_t flag, uint32_t value) {
    g_flag_slots++;
    const int slot = flag_slot(flag);
    if (slot >= 0)
        ((volatile uint32_t *)self)[slot] = value;
}
static const uint8_t flag_slots_get_original[] = {0x8b, 0x44, 0x24, 0x04, 0x83, 0xf8, 0x20, 0x7f};
REWRITE("FlagSlots::Get", 0x5d11c0, flag_slots_get_original, &flag_slots_get, &g_flag_slots);
REWRITE("FlagSlots::Set", 0x5d10f0, flag_slots_get_original, &flag_slots_set, &g_flag_slots);

// ---- Bits::AllSet(this, start, end) (0xC16660): whether every block of a range has its bit set --------------------------
// Blocks of the size at +4 (signed division): from start / size up to (end + size - 1) / size; block i's bit is bit
// i % 32 of word i / 32 of the list at +0xC (signed, as C divides); a word outside the list (+0x10 words) counts as
// unset. Gives the answer in the low byte (1 for an empty range).
static volatile long g_bits_all_set;
static uint8_t __thiscall bits_all_set(const uint8_t *self, int32_t start, int32_t end) {
    g_bits_all_set++;
    const int32_t size = *(const int32_t *)(self + 4);
    const int32_t last = (size + end - 1) / size;
    for (int32_t i = start / size; i < last; i++) {
        const int32_t word = i / 32;
        if (word < 0 || word >= *(const int32_t *)(self + 0x10))
            return 0;
        if (!((*(const uint32_t *const *)(self + 0xc))[word] & (1u << ((i % 32) & 31))))
            return 0;
    }
    return 1;
}
static const uint8_t bits_all_set_original[] = {0x8b, 0x44, 0x24, 0x08, 0x53, 0x55, 0x56, 0x57};
REWRITE("Bits::AllSet", 0xc16660, bits_all_set_original, &bits_all_set, &g_bits_all_set);

// ---- SortedTable::FindVersion(this, key, place) (0x7A2CF0): binary search for a 64-bit key -------------------------------
// Entries of 24 bytes at +8 (count +4), each pointing (+4) at a signed 64-bit value; the key's at +0x20. Gives (in the
// low byte) 1 and the place when found, else 0 and where it would go.
static volatile long g_version_finds;
static uint8_t __thiscall sorted_table_find_version(const uint8_t *self, const uint8_t *key, int32_t *place) {
    g_version_finds++;
    const int64_t k = *(const int64_t *)(key + 0x20);
    const uint8_t *const entries = *(const uint8_t *const *)(self + 8);
    int32_t lo = 0, hi = *(const int32_t *)(self + 4);
    while (hi > lo) {
        const int32_t mid = (hi - lo) / 2 + lo;
        const int64_t v = **(const int64_t *const *)(entries + mid * 24 + 4);
        if (v < k)
            lo = mid + 1;
        else if (v > k)
            hi = mid;
        else {
            *(volatile int32_t *)place = mid;
            return 1;
        }
    }
    *(volatile int32_t *)place = lo;
    return 0;
}
static const uint8_t sorted_table_find_version_original[] = {0x51, 0x53, 0x55, 0x56, 0x57, 0x8b, 0x79, 0x04};
REWRITE("SortedTable::FindVersion", 0x7a2cf0, sorted_table_find_version_original, &sorted_table_find_version, &g_version_finds);

// ---- Lod::LevelFor(this, n, fallback) (0xBC0B00): the detail level to use for number n --------------------------------------
// -1 without levels (+0x44). With levels (byte +4) less held back (byte +5) still above n and the object not ready (bit 1
// of the word at +2 when its bit 0 is set, else +0x40 set): levels - held when the fallback byte is set and some are held
// back, else -1. Otherwise n kept within 0 and levels - 1.
static volatile long g_lod_levels;
static int32_t __thiscall lod_level_for(const uint8_t *self, int32_t n, uint32_t fallback) {
    g_lod_levels++;
    if (!*(const uint32_t *)(self + 0x44))
        return -1;
    const int32_t levels = self[4], held = self[5];
    const int32_t usable = levels - held;
    if (n < usable) {
        const uint8_t flags = self[2];
        const bool ready = flags & 1 ? (flags >> 1) & 1 : *(const uint32_t *)(self + 0x40) != 0;
        if (!ready) {
            if ((uint8_t)fallback && usable < levels)
                return usable;
            return -1;
        }
    }
    if (n >= levels)
        return levels - 1;
    return n > 0 ? n : 0;
}
static const uint8_t lod_level_for_original[] = {0x8b, 0x41, 0x44, 0x85, 0xc0, 0x75, 0x06, 0x83};
REWRITE("Lod::LevelFor", 0xbc0b00, lod_level_for_original, &lod_level_for, &g_lod_levels);

// ---- SortedPointers::FindXY(this, x, y, flagged_only, place) (0x8FEF80): binary search for the entry at (x, y) -------------
// Entries (pointers at +0x30, count +0x34) sorted by y (+4) then x (+0). With flagged_only, an unflagged entry (byte +8)
// sends the search lower; without it, a flagged one sends it higher. Gives the entry (and its place when asked) or null.
static volatile long g_xy_finds;
static const uint8_t *__thiscall sorted_pointers_find_xy(const uint8_t *self, int32_t x, int32_t y, uint32_t flagged_only, int32_t *place) {
    g_xy_finds++;
    const uint8_t *const *const list = *(const uint8_t *const *const *)(self + 0x30);
    int32_t lo = 0, hi = *(const int32_t *)(self + 0x34) - 1;
    while (lo <= hi) {
        const int32_t mid = (hi - lo) / 2 + lo;
        const uint8_t *const e = list[mid];
        const uint8_t flagged = e[8];
        const int32_t ex = *(const int32_t *)e, ey = *(const int32_t *)(e + 4);
        bool lower;
        if ((uint8_t)flagged_only && !flagged)
            lower = true;
        else if (!(uint8_t)flagged_only && flagged)
            lower = false;
        else if (y != ey)
            lower = y < ey;
        else if (x != ex)
            lower = x < ex;
        else {
            if (place)
                *(volatile int32_t *)place = mid;
            return e;
        }
        if (lower)
            hi = mid - 1;
        else
            lo = mid + 1;
    }
    return nullptr;
}
static const uint8_t sorted_pointers_find_xy_original[] = {0x51, 0x53, 0x55, 0x8b, 0x69, 0x34, 0x56, 0x33};
REWRITE("SortedPointers::FindXY", 0x8fef80, sorted_pointers_find_xy_original, &sorted_pointers_find_xy, &g_xy_finds);

// ---- Refs20::Refs20(this, owner) (0xC09FE0): the table 0xFAF9B4, the owner, and 24 references copied from it -----------
// Each copied to +8 on; a non-null one has its 16-bit use count (at its +0) raised.
static volatile long g_refs20s;
static uint32_t *__thiscall refs20(uint32_t *self, const uint32_t *owner) {
    g_refs20s++;
    volatile uint32_t *const t = self;
    t[0] = 0xfaf9b4;
    t[1] = (uint32_t)(uintptr_t)owner;
    for (int i = 0; i < 24; i++) {
        const uint32_t r = owner[i];
        t[2 + i] = r;
        if (r)
            ++*(volatile uint16_t *)(uintptr_t)r;
    }
    return self;
}
static const uint8_t refs20_original[] = {0x53, 0x56, 0x8b, 0xc1, 0x57, 0x8b, 0x7c, 0x24};
REWRITE("Refs20::Refs20", 0xc09fe0, refs20_original, &refs20, &g_refs20s);

// ---- Gate::SetState(gate, action) (0x4EE3A0): a gate's two state values (+0x20, +0x24) for an action ------------------
// 0: (1, 2); 1: (0, and 2 becomes 0); 2: (1, 0); 3: (0, and 2 becomes 1); 4: (1, 1); others nothing.
static volatile long g_gate_states;
static void __stdcall gate_set_state(uint8_t *gate, uint32_t action) {
    g_gate_states++;
    volatile uint32_t *const s = (volatile uint32_t *)(gate + 0x20);
    switch (action) {
    case 0: s[0] = 1, s[1] = 2; break;
    case 1: s[0] = 0; if (s[1] == 2) s[1] = 0; break;
    case 2: s[0] = 1, s[1] = 0; break;
    case 3: s[0] = 0; if (s[1] == 2) s[1] = 1; break;
    case 4: s[0] = 1, s[1] = 1; break;
    default: break;
    }
}
static const uint8_t gate_set_state_original[] = {0x8b, 0x44, 0x24, 0x08, 0x83, 0xf8, 0x04, 0x77};
REWRITE("Gate::SetState", 0x4ee3a0, gate_set_state_original, &gate_set_state, &g_gate_states);

// ---- Quad::UpdateCounts(this) (0x8FDB30): vertex and index counts (+0x18, +0x1C) and the game's totals ------------------
// The old counts come off the totals (0x133A028, 0x133A02C); 4 and 6 when all four corners (+8 to +0x14) are set, else 0;
// the new counts go back on.
static volatile long g_quad_counts;
static void __thiscall quad_update_counts(uint8_t *self) {
    g_quad_counts++;
    volatile uint32_t *const t = (volatile uint32_t *)self;
    volatile uint32_t *const vertices = (volatile uint32_t *)0x133a028, *const indices = (volatile uint32_t *)0x133a02c;
    *vertices = *vertices - t[6];
    *indices = *indices - t[7];
    const bool whole = t[2] && t[3] && t[4] && t[5];
    t[6] = whole ? 4 : 0;
    t[7] = whole ? 6 : 0;
    *vertices = *vertices + t[6];
    *indices = *indices + t[7];
}
static const uint8_t quad_update_counts_original[] = {0x8b, 0x41, 0x18, 0x8b, 0x15, 0x28, 0xa0, 0x33};
REWRITE("Quad::UpdateCounts", 0x8fdb30, quad_update_counts_original, &quad_update_counts, &g_quad_counts);

// ---- String::Find(this, text, from) (0xBCF460): the place of a text inside a string from a place, or -1 -----------------
// The string record (*this): its text at +0xC, length +4. Tries each place p from `from` while p + the text's length is
// within the string; an empty text is found at once.
static volatile long g_string_finds;
static int32_t __thiscall string_find(const uint8_t *const *self, const char *text, uint32_t from) {
    g_string_finds++;
    const uint8_t *const s = *self;
    const char *const data = *(const char *const *)(s + 0xc);
    const uint32_t len = *(const uint32_t *)(s + 4);
    uint32_t tlen = 0;
    while (text[tlen]) tlen++;
    const uintptr_t last = (uintptr_t)data + len - tlen;
    for (uintptr_t p = (uintptr_t)data + from; p <= last; p++) {
        uint32_t i = 0;
        while (text[i] && text[i] == ((const char *)p)[i]) i++;
        if (!text[i])
            return (int32_t)(p - (uintptr_t)data);
    }
    return -1;
}
static const uint8_t string_find_original[] = {0x51, 0x8b, 0x09, 0x8b, 0x44, 0x24, 0x0c, 0x53};
REWRITE("String::Find", 0xbcf460, string_find_original, &string_find, &g_string_finds);

// ---- String::FindLast(this, text, from) (0xBCF600): the last place of a text, searching backwards, or -1 ----------------
// From `from` (the last character when -1; at most the last), down to the start: a place matches when the text's first
// character equals it (an empty text so matches a NUL there) and the rest follow.
static int32_t __thiscall string_find_last(const uint8_t *const *self, const char *text, uint32_t from) {
    g_string_finds++;
    const uint8_t *const s = *self;
    const uint32_t len = *(const uint32_t *)(s + 4);
    uint32_t start = len - 1;
    if (from != 0xffffffffu && !(from > start))
        start = from;
    const uintptr_t data = *(const uintptr_t *)(s + 0xc);
    for (uintptr_t p = data + start; p >= data; p--) {
        if (text[0] != *(const char *)p)
            continue;
        uint32_t i = 0;
        while (text[i] && text[i] == ((const char *)p)[i]) i++;
        if (!text[i])
            return (int32_t)(p - data);
        if (p == 0)
            break;
    }
    return -1;
}
static const uint8_t string_find_last_original[] = {0x8b, 0x44, 0x24, 0x08, 0x83, 0xec, 0x08, 0x83};
REWRITE("String::FindLast", 0xbcf600, string_find_last_original, &string_find_last, &g_string_finds);

// ---- Refs20::CopyFrom(this, other) (0xC0A060): the table 0xFAF9B4, other's +4, and its 24 references from +8 ------------
static uint32_t *__thiscall refs20_copy_from(uint32_t *self, const uint32_t *other) {
    g_refs20s++;
    volatile uint32_t *const t = self;
    t[0] = 0xfaf9b4;
    t[1] = other[1];
    for (int i = 0; i < 24; i++) {
        const uint32_t r = other[2 + i];
        t[2 + i] = r;
        if (r)
            ++*(volatile uint16_t *)(uintptr_t)r;
    }
    return self;
}
static const uint8_t refs20_copy_from_original[] = {0x53, 0x8b, 0xc1, 0x56, 0x57, 0x8b, 0x7c, 0x24};
REWRITE("Refs20::CopyFrom", 0xc0a060, refs20_copy_from_original, &refs20_copy_from, &g_refs20s);

// ---- Buckets::ClearLists(this, n) (0xC1AEA0): the list of each entry in bucket n emptied (every bucket when n < 0) -------
// Buckets at +0x10 (count +4, unsigned); each points at its first node; nodes run (next at +0) until one whose next is
// null; each node's entry (+8) gets an empty list (+0x14 cleared, +0x18 pointing at it).
static volatile long g_bucket_clears;
static inline void bucket_clear(const uint8_t *self, uint32_t b) {
    const uint32_t *node = *(const uint32_t *const *)((*(const uint32_t *const *)(self + 0x10))[b]);
    while (*node && node) {
        volatile uint32_t *const e = (volatile uint32_t *)(uintptr_t)node[2];
        e[5] = 0;
        e[6] = (uint32_t)(uintptr_t)&e[5];
        node = (const uint32_t *)(uintptr_t)*node;
    }
}
static void __thiscall buckets_clear_lists(const uint8_t *self, int32_t n) {
    g_bucket_clears++;
    if (n >= 0) {
        if ((uint32_t)n < *(const uint32_t *)(self + 4))
            bucket_clear(self, (uint32_t)n);
        return;
    }
    for (uint32_t b = 0; b < *(const volatile uint32_t *)(self + 4); b++) bucket_clear(self, b);
}
static const uint8_t buckets_clear_lists_original[] = {0x8b, 0x44, 0x24, 0x04, 0x85, 0xc0, 0x57, 0x7d};
REWRITE("Buckets::ClearLists", 0xc1aea0, buckets_clear_lists_original, &buckets_clear_lists, &g_bucket_clears);

// ---- Nodes::MatrixFor(this, n, shared) (0xCC68C0): the 64-byte matrix for node n, and which list it came from ------------
// Nodes of 0x60 bytes in the record's (*this) +0x10; with f = the node's flags (+0x54) and k = its low byte: shared when
// (k <= 1, signed, and bit 8) or (k == 0 and bit 11) or bit 10. Shared: this's +4 list; else the record's +0x18; the
// node's +0x58 picks the matrix.
static volatile long g_node_matrices;
static const uint8_t *__thiscall nodes_matrix_for(const uint8_t *const *self, int32_t n, uint8_t *shared) {
    g_node_matrices++;
    const uint8_t *const record = self[0];
    const uint32_t f = *(const uint32_t *)(*(const uint8_t *const *)(record + 0x10) + n * 0x60 + 0x54);
    const int32_t k = (int32_t)(f & 0xff);
    const bool is_shared = (k <= 1 && (f >> 8) & 1) || (k == 0 && (f >> 11) & 1) || (f >> 10) & 1;
    *(volatile uint8_t *)shared = is_shared;
    const uint32_t index = *(const uint32_t *)(*(const uint8_t *const *)(self[0] + 0x10) + n * 0x60 + 0x58);
    const uint8_t *const base = is_shared ? self[1] : *(const uint8_t *const *)(self[0] + 0x18);
    return base + (index << 6);
}
static const uint8_t nodes_matrix_for_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x11, 0x8b, 0x52};
REWRITE("Nodes::MatrixFor", 0xcc68c0, nodes_matrix_for_original, &nodes_matrix_for, &g_node_matrices);

// ---- Filter::Accepts(this, item) (0x758880): whether an item passes a filter by its kind (+0x120) and option (+0x121) -----
// The item's flags (+0x2C) are tested with: kind 0: 1, or 2 with the option; 1: 4 or 8; 2: 0x10 or 0x20; others all bits.
static volatile long g_filter_accepts;
static uint8_t __thiscall filter_accepts(const uint8_t *self, const uint8_t *item) {
    g_filter_accepts++;
    const bool option = self[0x121] != 0;
    uint32_t mask;
    switch (self[0x120]) {
    case 0: mask = option ? 2 : 1; break;
    case 1: mask = option ? 8 : 4; break;
    case 2: mask = option ? 0x20 : 0x10; break;
    default: mask = 0xffffffff; break;
    }
    return (*(const uint32_t *)(item + 0x2c) & mask) != 0;
}
static const uint8_t filter_accepts_original[] = {0x0f, 0xb6, 0x81, 0x20, 0x01, 0x00, 0x00, 0x83};
REWRITE("Filter::Accepts", 0x758880, filter_accepts_original, &filter_accepts, &g_filter_accepts);

// ---- List::FindSelected(this, id, -) (0x9F6490): the owner's selected value from the list entry with the id -------------
// Goes through the whole list (*(+0xE4); nodes: next +0, entry +8, until a node whose next is null): each entry whose
// first value is the id has its second value put in the owner's (+0xDC) +0x15C. The owner's +0x160 is then cleared.
static volatile long g_selected_finds;
static void __thiscall list_find_selected(const uint8_t *self, uint32_t id, uint32_t) {
    g_selected_finds++;
    const uint32_t *node = *(const uint32_t *const *)(self + 0xe4);
    if (*node && node) {
        do {
            const uint32_t *const e = (const uint32_t *)(uintptr_t)node[2];
            if (e[0] == id)
                *(volatile uint32_t *)(*(uint8_t *const *)(self + 0xdc) + 0x15c) = e[1];
            node = (const uint32_t *)(uintptr_t)*node;
        } while (*node && node);
    }
    *(volatile uint32_t *)(*(uint8_t *const *)(self + 0xdc) + 0x160) = 0;
}
static const uint8_t list_find_selected_original[] = {0x8b, 0x81, 0xe4, 0x00, 0x00, 0x00, 0x8b, 0x10};
REWRITE("List::FindSelected", 0x9f6490, list_find_selected_original, &list_find_selected, &g_selected_finds);

// ---- Bounds::Grow(this, min_x, min_y, max_x, max_y, v) (0x8E7130): a box (+0x190 to +0x19C) grown to take in another -----
// Each minimum replaced when below it, each maximum when above (as they are); +0x1A0 = v.
static volatile long g_bounds_grows;
static void __thiscall bounds_grow(uint8_t *self, float min_x, float min_y, float max_x, float max_y, uint32_t v) {
    g_bounds_grows++;
    volatile uint32_t *const b = (volatile uint32_t *)(self + 0x190);
    const float *const f = (const float *)(self + 0x190);
    if ((long double)min_x < (long double)((const volatile float *)f)[0]) b[0] = *(const uint32_t *)&min_x;
    if ((long double)min_y < (long double)((const volatile float *)f)[1]) b[1] = *(const uint32_t *)&min_y;
    if ((long double)max_x > (long double)((const volatile float *)f)[2]) b[2] = *(const uint32_t *)&max_x;
    if ((long double)max_y > (long double)((const volatile float *)f)[3]) b[3] = *(const uint32_t *)&max_y;
    b[4] = v;
}
static const uint8_t bounds_grow_original[] = {0xd9, 0x44, 0x24, 0x04, 0xd8, 0x99, 0x90, 0x01};
REWRITE("Bounds::Grow", 0x8e7130, bounds_grow_original, &bounds_grow, &g_bounds_grows);

// ---- Indices::CopyOffset(to, from, count, offset) (0xBEECB0): 16-bit indices copied, the offset added when given --------
// Without an offset: count * 2 bytes copied. With one: worked a 32-bit word (two indices) at a time with offset << 16 |
// offset added (a carry from the low index reaches the high one): count / 4 pairs of words, then one more word when bit 1
// of count is set (an odd last index is left).
static volatile long g_index_copies;
static void __fastcall indices_copy_offset(uint8_t *to, const uint8_t *from, uint32_t count, uint32_t offset) {
    g_index_copies++;
    if (!offset) {
        for (uint32_t i = 0; i < count * 2; i++) ((volatile uint8_t *)to)[i] = from[i];
        return;
    }
    const uint32_t add = offset << 16 | offset;
    const uint32_t words = (count >> 2) * 2 + ((count & 2) ? 1 : 0);
    for (uint32_t i = 0; i < words; i++) ((volatile uint32_t *)to)[i] = ((const uint32_t *)from)[i] + add;
}
static const uint8_t indices_copy_offset_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x08, 0x8b, 0x45};
REWRITE("Indices::CopyOffset", 0xbeecb0, indices_copy_offset_original, &indices_copy_offset, &g_index_copies);

// ---- Scenery::Turn827530(this, x, y, x_out, y_out) (0x827530): a cell turned within a footprint, and whether it is in ---
// (dx, dy) = (x, y) less the corner (+4, +8); by the turn (+0x10), with the footprint's (*this) width +0x10 and depth
// +0x14: 0 (dx, dy); 1 (width - dy - 1, dx); 2 (width - dx - 1, depth - dy - 1); 3 (dy, depth - dx - 1); others leave
// the outputs. Gives (in the low byte) whether the outputs lie inside the footprint (signed).
static volatile long g_scenery_turns;
static uint8_t __thiscall scenery_turn_827530(const int32_t *self, int32_t x, int32_t y, int32_t *x_out, int32_t *y_out) {
    g_scenery_turns++;
    const int32_t dx = x - self[1], dy = y - self[2];
    const int32_t *const f = (const int32_t *)(uintptr_t)self[0];
    volatile int32_t *const ox = x_out, *const oy = y_out;
    switch ((uint32_t)self[4]) {
    case 0: *ox = dx, *oy = dy; break;
    case 1: *ox = f[4] - dy - 1, *oy = dx; break;
    case 2: *ox = f[4] - dx - 1, *oy = f[5] - dy - 1; break;
    case 3: *ox = dy, *oy = f[5] - dx - 1; break;
    default: break;
    }
    const int32_t *const g = (const int32_t *)(uintptr_t)self[0];
    const int32_t cx = *ox;
    if (cx < 0 || cx >= g[4]) return 0;
    const int32_t cy = *oy;
    return cy >= 0 && cy < g[5];
}
static const uint8_t scenery_turn_827530_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x54, 0x24, 0x08};
REWRITE("Scenery::Turn827530", 0x827530, scenery_turn_827530_original, &scenery_turn_827530, &g_scenery_turns);

// ---- SortedPointers::Find(this, list, count, place, -) (0x87A6D0): binary search of objects by +4 (unsigned) then +0xC --
// The key is *this; gives (in the low byte) 1 and the place when found, else 0 and where it would go (0 for an empty
// list).
static volatile long g_pointer_finds;
static uint8_t __fastcall sorted_pointers_find(const uint8_t *const *self, const uint8_t *const *list, int32_t count, int32_t *place, uint32_t) {
    g_pointer_finds++;
    if (count < 1) {
        *(volatile int32_t *)place = 0;
        return 0;
    }
    const uint8_t *const key = *self;
    const uint32_t k1 = *(const uint32_t *)(key + 4);
    int32_t lo = 0, hi = count - 1;
    for (;;) {
        const int32_t mid = (hi + lo) >> 1;
        const uint8_t *const e = list[mid];
        const uint32_t e1 = *(const uint32_t *)(e + 4);
        bool lower;
        if (k1 != e1)
            lower = k1 < e1;
        else {
            const int32_t k2 = *(const int32_t *)(key + 0xc), e2 = *(const int32_t *)(e + 0xc);
            if (k2 == e2) {
                *(volatile int32_t *)place = mid;
                return 1;
            }
            lower = k2 < e2;
        }
        if (lower) {
            if (mid > lo) {
                hi = mid - 1;
                continue;
            }
            *(volatile int32_t *)place = lo;
            return 0;
        }
        if (mid < hi) {
            lo = mid + 1;
            continue;
        }
        *(volatile int32_t *)place = hi + 1;
        return 0;
    }
}
static const uint8_t sorted_pointers_find_original[] = {0x51, 0x8b, 0x44, 0x24, 0x08, 0x83, 0xf8, 0x01};
REWRITE("SortedPointers::Find", 0x87a6d0, sorted_pointers_find_original, &sorted_pointers_find, &g_pointer_finds);

// ---- SceneryItem::RemoveLink(this, object) (0x83FBC0): an object taken out of two lists, the later ones moved down ------
// The lists at +0x54 (count +0x58) and +0x60 (count +0x64): the first place holding the object is closed up and the
// count drops by one.
static volatile long g_link_removals;
static inline void remove_from(uint8_t *self, int at, uint32_t object, int count_at = 4) {
    volatile int32_t *const count = (volatile int32_t *)(self + at + count_at);
    const int32_t n = *count;
    for (int32_t i = 0; i < *count; i++) {
        if ((*(const uint32_t *const *)(self + at))[i] != object)
            continue;
        for (int32_t j = i; j < n - 1 && j < *count - 1; j++) {
            volatile uint32_t *const list = *(volatile uint32_t *const *)(self + at);
            list[j] = list[j + 1];
        }
        *count = *count - 1;
        return;
    }
}
static void __thiscall scenery_item_remove_link(uint8_t *self, uint32_t object) {
    g_link_removals++;
    remove_from(self, 0x54, object);
    remove_from(self, 0x60, object);
}
static const uint8_t scenery_item_remove_link_original[] = {0x53, 0x56, 0x8b, 0x71, 0x58, 0x33, 0xc0, 0x85};
REWRITE("SceneryItem::RemoveLink", 0x83fbc0, scenery_item_remove_link_original, &scenery_item_remove_link, &g_link_removals);

// ---- WideString::Find(this, text, from) (0xBCEDC0): the place of a wide text inside a wide string, or -1 ----------------
// As String::Find, in 16-bit characters.
static int32_t __thiscall wide_string_find(const uint8_t *const *self, const uint16_t *text, uint32_t from) {
    g_string_finds++;
    const uint8_t *const s = *self;
    const uintptr_t data = *(const uintptr_t *)(s + 0xc);
    const uint32_t len = *(const uint32_t *)(s + 4);
    uint32_t tlen = 0;
    while (text[tlen]) tlen++;
    const uintptr_t last = data + (uintptr_t)(len - tlen) * 2;
    for (uintptr_t p = data + (uintptr_t)from * 2; p <= last; p += 2) {
        uint32_t i = 0;
        while (text[i] && text[i] == ((const uint16_t *)p)[i]) i++;
        if (!text[i])
            return (int32_t)(p - data) >> 1;
    }
    return -1;
}
static const uint8_t wide_string_find_original[] = {0x51, 0x8b, 0x09, 0x8b, 0x44, 0x24, 0x0c, 0x8b};
REWRITE("WideString::Find", 0xbcedc0, wide_string_find_original, &wide_string_find, &g_string_finds);

// ---- Buffer::LayOut(this) (0xCB78E0): a buffer's parts laid out one after another -----------------------------------------
// From the owner's (+0x14) start (+0x40) rounded up to the alignment (+4, unsigned): each part (0x28 bytes at +0x30, count
// +8) gets the place (+4), and the place moves past its size (+8), rounded up again. The count (+8) then becomes the
// number of parts of kind (byte +1) 1 or 2, bit 1 of +0 is cleared, and 1 is given (in the low byte).
static volatile long g_buffer_layouts;
static inline uint32_t round_up(uint32_t v, uint32_t align) {
    const uint32_t r = v % align;
    return r ? v + (align - r) : v;
}
static uint8_t __thiscall buffer_lay_out(uint32_t *self) {
    g_buffer_layouts++;
    volatile uint32_t *const t = self;
    uint32_t place = round_up(*(const uint32_t *)((uint8_t *)(uintptr_t)t[5] + 0x40), t[1]);
    uint32_t kinds = 0;
    for (uint32_t i = 0; i < t[2]; i++) {
        uint8_t *const part = (uint8_t *)(uintptr_t)t[12] + i * 0x28;
        *(volatile uint32_t *)(part + 4) = place;
        place = round_up(*(const uint32_t *)((uint8_t *)(uintptr_t)t[12] + i * 0x28 + 8) + place, t[1]);
        const uint8_t kind = part[1];
        if (kind >= 1 && kind <= 2)
            kinds++;
    }
    t[0] = t[0] & 0xfffffffd;
    t[2] = kinds;
    return 1;
}
static const uint8_t buffer_lay_out_original[] = {0x8b, 0x41, 0x14, 0x83, 0xec, 0x08, 0x55, 0x56};
REWRITE("Buffer::LayOut", 0xcb78e0, buffer_lay_out_original, &buffer_lay_out, &g_buffer_layouts);

// ---- VertexLayout::Add(this, element) (0x413E40): an element added to a vertex layout (up to 16, count +0x20) ----------
// The 16-bit element is stored; the size (+0x24) grows by: high byte 0: the type's size (the table 0x10F2BA8 by the low
// byte); 1: 16; above 0x7F: (high - 0x7F) times the type's size, plus 4; otherwise 0.
static volatile long g_layout_adds;
static void __thiscall vertex_layout_add(uint8_t *self, uint32_t element) {
    g_layout_adds++;
    volatile uint32_t *const count = (volatile uint32_t *)(self + 0x20), *const size = (volatile uint32_t *)(self + 0x24);
    const uint32_t n = *count;
    if (n >= 16)
        return;
    ((volatile uint16_t *)self)[n] = (uint16_t)element;
    const uint8_t type = (uint8_t)element, high = (uint8_t)(element >> 8);
    const uint32_t *const sizes = (const uint32_t *)0x10f2ba8;
    uint32_t grow;
    if (high == 0) grow = sizes[type];
    else if (high == 1) grow = 0x10;
    else if (high > 0x7f) grow = (uint32_t)(high - 0x7f) * sizes[type] + 4;
    else grow = 0;
    *size = *size + grow;
    *count = *count + 1;
}
static const uint8_t vertex_layout_add_original[] = {0x8b, 0x51, 0x20, 0x83, 0xfa, 0x10, 0x0f, 0x83};
REWRITE("VertexLayout::Add", 0x413e40, vertex_layout_add_original, &vertex_layout_add, &g_layout_adds);

// ---- Path::Bounds71DC50(this, x, y, grow) (0x71DC50): byte bounds (+0x9C: min x, min y, max x, max y) and a cell --------
// Growing: each bound widened to take in (x, y) (unsigned bytes). Otherwise: +0xA0 set to 1 when the cell lies on any
// of the four bounds.
static volatile long g_path_bounds;
static void __thiscall path_bounds_71dc50(uint8_t *self, uint32_t x_arg, uint32_t y_arg, uint32_t grow) {
    g_path_bounds++;
    const uint8_t x = (uint8_t)x_arg, y = (uint8_t)y_arg;
    volatile uint8_t *const b = self + 0x9c;
    if ((uint8_t)grow) {
        b[0] = x < b[0] ? x : b[0];
        b[1] = y < b[1] ? y : b[1];
        b[2] = x > b[2] ? x : b[2];
        b[3] = y > b[3] ? y : b[3];
        return;
    }
    if (x == b[0] || y == b[1] || x == b[2] || y == b[3])
        b[4] = 1;
}
static const uint8_t path_bounds_71dc50_original[] = {0x8a, 0x44, 0x24, 0x0c, 0x84, 0xc0, 0x53, 0x74};
REWRITE("Path::Bounds71DC50", 0x71dc50, path_bounds_71dc50_original, &path_bounds_71dc50, &g_path_bounds);

// ---- Nodes::MarkChain(this, n, force) (0xCC75A0): node n and the nodes it leads to marked with bit 0x800 ----------------
// Nodes of 0x60 bytes at +0x10 (flags +0x54, next +0x50). Nothing when node n is marked already. Forced: n is marked,
// then each next node (while the next is above 0 and not yet marked). Not forced: n must be able to start a chain
// (its low flag byte at most 1 with bit 8, or bit 10); n itself is left and its next nodes are marked.
static volatile long g_chain_marks;
static void __thiscall nodes_mark_chain(const uint8_t *self, int32_t n, uint32_t force) {
    g_chain_marks++;
    uint8_t *const nodes = *(uint8_t *const *)(self + 0x10);
    volatile uint32_t *flags = (volatile uint32_t *)(nodes + n * 0x60 + 0x54);
    if (*flags & 0x800)
        return;
    int32_t i = n;
    if (!(uint8_t)force) {
        const uint32_t f = *flags;
        const bool starts = ((int32_t)(f & 0xff) <= 1 && (f >> 8) & 1) || (f >> 10) & 1;
        if (!starts)
            return;
        i = *(const int32_t *)(nodes + n * 0x60 + 0x50);
        if (i <= 0 || *(const uint32_t *)(nodes + i * 0x60 + 0x54) & 0x800)
            return;
    }
    for (;;) {
        uint8_t *const node = *(uint8_t *const *)(self + 0x10) + i * 0x60;
        *(volatile uint32_t *)(node + 0x54) = *(volatile uint32_t *)(node + 0x54) | 0x800;
        const int32_t next = *(const int32_t *)(*(uint8_t *const *)(self + 0x10) + i * 0x60 + 0x50);
        if (next <= 0)
            return;
        i = next;
        if (*(const uint32_t *)(*(uint8_t *const *)(self + 0x10) + i * 0x60 + 0x54) & 0x800)
            return;
    }
}
static const uint8_t nodes_mark_chain_original[] = {0x8b, 0x44, 0x24, 0x04, 0x57, 0x8b, 0xf9, 0x8b};
REWRITE("Nodes::MarkChain", 0xcc75a0, nodes_mark_chain_original, &nodes_mark_chain, &g_chain_marks);

// ---- String::CompareRaw(a, b) (0xCD5750): two plain texts compared character by character (unsigned): -1, 0 or 1 --------
// A missing text counts as empty.
static volatile long g_raw_compares;
static int32_t __stdcall string_compare_raw(const uint8_t *a, const uint8_t *b) {
    g_raw_compares++;
    static const uint8_t empty = 0;
    if (!a) a = &empty;
    if (!b) b = &empty;
    for (;; a++, b++) {
        if (*a != *b)
            return *a < *b ? -1 : 1;
        if (!*a)
            return 0;
    }
}
static const uint8_t string_compare_raw_original[] = {0x8b, 0x44, 0x24, 0x04, 0x85, 0xc0, 0x8b, 0x4c};
REWRITE("String::CompareRaw", 0xcd5750, string_compare_raw_original, &string_compare_raw, &g_raw_compares);

// ---- Key::FromSmallList(list, n) (0x9F8430): a short list sorted and packed into a 64-bit key ----------------------------
// The list sorted in place (each place swapped with any later smaller number, signed); then the key: the first number
// plus 1 (sign-extended to 64 bits), or'd with the second plus 1 (sign-extended) shifted up 16, the third plus 1 into the
// high word, and the fourth plus 1 shifted up 16 into the high word.
static volatile long g_small_list_keys;
static uint64_t __stdcall key_from_small_list(int32_t *list, int32_t n) {
    g_small_list_keys++;
    volatile int32_t *const l = list;
    for (int32_t i = 0; i < n - 1; i++)
        for (int32_t j = i + 1; j < n; j++)
            if (l[i] > l[j]) {
                const int32_t t = l[i];
                l[i] = l[j];
                l[j] = t;
            }
    uint64_t key = 0;
    if (n > 0) key = (uint64_t)((int64_t)l[0] + 1);
    if (n > 1) key |= (uint64_t)(int64_t)(int32_t)((uint32_t)l[1] + 1) << 16;
    if (n > 2) key |= (uint64_t)(uint32_t)((uint32_t)l[2] + 1) << 32;
    if (n > 3) key |= (uint64_t)(uint32_t)(((uint32_t)l[3] + 1) << 16) << 32;
    return key;
}
static const uint8_t key_from_small_list_original[] = {0x55, 0x8b, 0x6c, 0x24, 0x0c, 0x56, 0x8d, 0x45};
REWRITE("Key::FromSmallList", 0x9f8430, key_from_small_list_original, &key_from_small_list, &g_small_list_keys);

// ---- GridMarks::MarkRect0(this, x0, y0, x1, y1) (0x874CF0): byte 0 of each cell of a rectangle set, and the bounds grown --
// The rectangle kept within 0 and the grid's size (+0x48, +0x4C); cells of 0x34 bytes at +0x50. When it is not empty,
// the marked bounds (+0x54 to +0x60) grow to take it in.
static volatile long g_rect_marks;
static void __thiscall grid_marks_mark_rect0(uint8_t *self, int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
    g_rect_marks++;
    volatile int32_t *const t = (volatile int32_t *)self;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > t[0x48 / 4]) x1 = t[0x48 / 4];
    if (y1 > t[0x4c / 4]) y1 = t[0x4c / 4];
    if (x0 >= x1 || y0 >= y1)
        return;
    for (int32_t y = y0; y < y1; y++)
        for (int32_t x = x0; x < x1; x++) ((volatile uint8_t *)(uintptr_t)t[0x50 / 4])[(t[0x48 / 4] * y + x) * 0x34] = 1;
    if (x0 < t[0x54 / 4]) t[0x54 / 4] = x0;
    if (y0 < t[0x58 / 4]) t[0x58 / 4] = y0;
    if (x1 > t[0x5c / 4]) t[0x5c / 4] = x1;
    if (y1 > t[0x60 / 4]) t[0x60 / 4] = y1;
}
static const uint8_t grid_marks_mark_rect0_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x85, 0xdb, 0x56};
REWRITE("GridMarks::MarkRect0", 0x874cf0, grid_marks_mark_rect0_original, &grid_marks_mark_rect0, &g_rect_marks);

// ---- Key::Compare(a, b) (0x9731C0): two keys of six values (passed by value) compared: -1, 0 or 1 ----------------------
// The first two unsigned; the third points at three signed values compared in turn; the last three signed.
static volatile long g_key_compares;
static int32_t __stdcall key_compare(uint32_t a0, uint32_t a1, const int32_t *a2, int32_t a3, int32_t a4, int32_t a5, uint32_t b0, uint32_t b1, const int32_t *b2, int32_t b3,
                                     int32_t b4, int32_t b5) {
    g_key_compares++;
    if (a0 != b0) return a0 > b0 ? 1 : -1;
    if (a1 != b1) return a1 > b1 ? 1 : -1;
    for (int i = 0; i < 3; i++)
        if (a2[i] != b2[i]) return a2[i] > b2[i] ? 1 : -1;
    if (a3 != b3) return a3 > b3 ? 1 : -1;
    if (a4 != b4) return a4 > b4 ? 1 : -1;
    if (a5 != b5) return a5 > b5 ? 1 : -1;
    return 0;
}
static const uint8_t key_compare_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x4c, 0x24, 0x1c};
REWRITE("Key::Compare", 0x9731c0, key_compare_original, &key_compare, &g_key_compares);

// ---- Vertex::SizeFromFormat(format) (0xBD4060): the size in bytes of a vertex from its format bits --------------------------
// Position (bits 0-1: 1 gives 12, 2 gives 16), 12 each for bits 2 and 3, bits 4-5 (0x10: 12, 0x20: 16), bits 6-8 (0x40,
// 0x80, 0xC0: 4; 0x100: 8), 4 each for bits 9 and 10, 8 per texture coordinate (bits 12-15), 4 each for bits 16 and 17.
static volatile long g_vertex_sizes;
static uint32_t __fastcall vertex_size_from_format(uint32_t format) {
    g_vertex_sizes++;
    uint32_t size = (format & 3) == 1 ? 12 : (format & 3) == 2 ? 16 : 0;
    if (format & 4) size += 12;
    if (format & 8) size += 12;
    if ((format & 0x30) == 0x10) size += 12;
    else if ((format & 0x30) == 0x20) size += 16;
    const uint32_t mid = format & 0x1c0;
    if (mid == 0x40 || mid == 0x80 || mid == 0xc0) size += 4;
    else if (mid == 0x100) size += 8;
    if (format & 0x200) size += 4;
    if (format & 0x400) size += 4;
    size += ((format >> 12) & 0xf) * 8;
    if (format & 0x10000) size += 4;
    if (format & 0x20000) size += 4;
    return size;
}
static const uint8_t vertex_size_from_format_original[] = {0x8b, 0xd1, 0x83, 0xe2, 0x03, 0x33, 0xc0, 0x4a};
REWRITE("Vertex::SizeFromFormat", 0xbd4060, vertex_size_from_format_original, &vertex_size_from_format, &g_vertex_sizes);

// ---- PairMap::Find(this, a, b) (0xCA45C0): the value stored for a pair of numbers, or -1 ---------------------------------
// Entries {a, b, value} of 12 bytes at +0x20 (count +0x14), sorted by a then b (signed). Steps of halving size from the
// power of two that reaches half the count: from place lo, the place lo + step is taken when its pair is below; an equal
// pair is the answer. Finally the pair at lo is checked.
static volatile long g_pair_finds;
static uint32_t __thiscall pair_map_find(const uint8_t *self, int32_t a, int32_t b) {
    g_pair_finds++;
    const int32_t count = *(const int32_t *)(self + 0x14);
    int32_t step = 1;
    if (count > 2) {
        int32_t s = 2;
        do step = s, s = step + step;
        while (s < count);
    }
    int32_t lo = 0;
    for (; step; step >>= 1) {
        const int32_t i = lo + step;
        if (i >= count)
            continue;
        const int32_t *const e = (const int32_t *)(*(const uint8_t *const *)(self + 0x20) + i * 12);
        if (e[0] > a)
            continue;
        if (e[0] < a || e[1] < b) {
            lo = i;
            continue;
        }
        if (e[1] != b)
            continue;
        return (uint32_t)e[2];
    }
    if (lo < count) {
        const int32_t *const e = (const int32_t *)(*(const uint8_t *const *)(self + 0x20) + lo * 12);
        if (e[0] == a && e[1] == b)
            return (uint32_t)e[2];
    }
    return 0xffffffff;
}
static const uint8_t pair_map_find_original[] = {0x51, 0x53, 0x55, 0x8b, 0xd1, 0x8b, 0x6a, 0x14};
REWRITE("PairMap::Find", 0xca45c0, pair_map_find_original, &pair_map_find, &g_pair_finds);

// ---- Slots::Reserve(this) (0xA517C0): how many of the first slots to keep, the empty ones among them marked ---------------
// Slots of 8 bytes at +0xC (count +0x10), kind first. keep = (6, or 3 when +0x14 is above -1) + the slots of kinds 3, 4
// and 5, at most count - 2; count - 1 when the kinds 4 and 5 alone reach count - 2; count when kind 5 alone reaches
// count - 1. The first `keep` slots of kind 0 become kind 1.
static volatile long g_slot_reserves;
static void __thiscall slots_reserve(const uint8_t *self) {
    g_slot_reserves++;
    const int32_t base = *(const int32_t *)(self + 0x14) > -1 ? 3 : 6;
    const int32_t count = *(const int32_t *)(self + 0x10);
    const int32_t *const slots = *(const int32_t *const *)(self + 0xc);
    int32_t n3 = 0, n4 = 0, n5 = 0;
    for (int32_t i = 0; i < count; i++) {
        n3 += slots[2 * i] == 3;
        n4 += slots[2 * i] == 4;
        n5 += slots[2 * i] == 5;
    }
    int32_t keep = (n4 + n5) + n3 + base;
    if (keep > count - 2) keep = count - 2;
    if (n4 + n5 >= count - 2) keep = count - 1;
    if (n5 >= count - 1) keep = count;
    for (int32_t i = 0; i < keep; i++) {
        volatile int32_t *const s = (volatile int32_t *)(*(uint8_t *const *)(self + 0xc) + i * 8);
        if (!*s)
            *s = 1;
    }
}
static const uint8_t slots_reserve_original[] = {0x51, 0x83, 0x79, 0x14, 0xff, 0xc7, 0x04, 0x24};
REWRITE("Slots::Reserve", 0xa517c0, slots_reserve_original, &slots_reserve, &g_slot_reserves);

// ---- WideString::CompareRaw(a, b) (0xCD60E0): a wide string (its record *a: length +4, text +0xC) and a wide text -------
// Compared character by character (unsigned): -1, 0 or 1; a missing one counts as empty.
static int32_t __stdcall wide_string_compare_raw(const uint8_t *const *a, const uint16_t *b) {
    g_raw_compares++;
    if (!a)
        return !b || !*b ? 0 : -1;
    const uint8_t *const s = *a;
    const uint32_t len = *(const uint32_t *)(s + 4);
    if (!b)
        return len ? 1 : 0;
    const uint16_t *const text = *(const uint16_t *const *)(s + 0xc);
    uint32_t i = 0;
    for (; i < len; i++) {
        const uint16_t c = b[i];
        if (!c)
            break;
        if (text[i] != c)
            return text[i] < c ? -1 : 1;
    }
    if (i < len)
        return 1;
    return b[i] ? -1 : 0;
}
static const uint8_t wide_string_compare_raw_original[] = {0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x44, 0x24, 0x08};
REWRITE("WideString::CompareRaw", 0xcd60e0, wide_string_compare_raw_original, &wide_string_compare_raw, &g_raw_compares);

// ---- Grid::SpiralStep(grid, cx, cy, dx, dy) (0xB35000): one step along a square spiral out from a cell --------------------
// With (x, y) = (*dx, *dy) the offset so far and the grid's width (byte +4) and height (byte +5): when x + y > 0, x + 1
// while y > x and cx + x is below width - 1 (else y = -y), or y - 1 while y <= x and cy + y is above 0 (else x = -x);
// otherwise x - 1 while x > y and cx + x is above 0 (else y = 1 - y), or y + 1 while x <= y and cy + y is below
// height - 1 (else x = 1 - x). Gives 1 when the new cell lies in the grid.
static volatile long g_spiral_steps;
static uint32_t __fastcall grid_spiral_step(const uint8_t *grid, int32_t cx, int32_t cy, int32_t *dx, int32_t *dy) {
    g_spiral_steps++;
    volatile int32_t *const px = dx, *const py = dy;
    const int32_t y = *py, x = *px;
    if (y + x > 0) {
        if (y > x) {
            if (x + cx < grid[4] - 1) *px = x + 1;
            else *py = -y;
        } else {
            if (y + cy > 0) *py = y - 1;
            else *px = -x;
        }
    } else {
        if (x > y) {
            if (x + cx > 0) *px = x - 1;
            else *py = 1 - y;
        } else {
            if (cy + y < grid[5] - 1) *py = y + 1;
            else *px = 1 - x;
        }
    }
    const int32_t nx = *px + cx;
    if (nx < 0 || nx >= grid[4]) return 0;
    const int32_t ny = *py + cy;
    if (ny < 0 || ny >= grid[5]) return 0;
    return 1;
}
static const uint8_t grid_spiral_step_original[] = {0x51, 0x53, 0x8b, 0x5c, 0x24, 0x14, 0x55, 0x56};
REWRITE("Grid::SpiralStep", 0xb35000, grid_spiral_step_original, &grid_spiral_step, &g_spiral_steps);

// ---- Mesh::SetPrimitiveCount(this, n) (0xBE8BA0): a mesh part's vertex count and number of primitives -----------------------
// n = 0x40000000 takes the count from the buffers (+0x1C's +8; else, when +0x4C is set, +0x30's +0x10; else 0); others
// keep their low 28 bits. +8 = n (and without +0x1C, +0xC = 0 and +0x10 = n). By the type (word +6): 0 points n, 1 lines
// n / 2, 2 a line strip n - 1, 3 triangles n / 3, 4 and 5 a strip or fan n - 2, others 0; at least 0, into +0x48.
static volatile long g_primitive_counts;
static void __thiscall mesh_set_primitive_count(uint8_t *self, uint32_t n) {
    g_primitive_counts++;
    volatile uint32_t *const t = (volatile uint32_t *)self;
    if (n == 0x40000000) {
        if (t[0x1c / 4]) n = *(const uint32_t *)((uint8_t *)(uintptr_t)t[0x1c / 4] + 8);
        else if (t[0x4c / 4]) n = *(const uint32_t *)((uint8_t *)(uintptr_t)t[0x30 / 4] + 0x10);
        else n = 0;
    } else
        n &= 0x0fffffff;
    t[2] = n;
    if (!t[0x1c / 4]) {
        t[3] = 0;
        t[4] = n;
    }
    const int32_t v = (int32_t)n;
    int32_t p;
    switch (*(const uint16_t *)(self + 6)) {
    case 0: p = v; break;
    case 1: p = v / 2; break;
    case 2: p = v - 1; break;
    case 3: p = v / 3; break;
    case 4: case 5: p = v - 2; break;
    default: p = 0; break;
    }
    t[0x48 / 4] = (uint32_t)(0 < p ? p : 0);
}
static const uint8_t mesh_set_primitive_count_original[] = {0x51, 0x8b, 0x54, 0x24, 0x08, 0x81, 0xfa, 0x00};
REWRITE("Mesh::SetPrimitiveCount", 0xbe8ba0, mesh_set_primitive_count_original, &mesh_set_primitive_count, &g_primitive_counts);

// ---- TileCorners::Count(this, a, b, c) (0x727B00): a tile counted into one of three 16-bit totals --------------------------
// A flagged tile (byte +0x34) counts into c. By the packed corners (word +0x32): 0, 0x1111, 0x2222, 0x3333 count into a;
// 0x4444 to 0x7777 into b; anything else corner by corner (each nibble's low three bits): below 4 into a, else b.
static volatile long g_tile_counts;
static void __thiscall tile_corners_count(const uint8_t *self, uint16_t *a, uint16_t *b, uint16_t *c) {
    g_tile_counts++;
    if (self[0x34]) {
        ++*(volatile uint16_t *)c;
        return;
    }
    const uint16_t v = *(const uint16_t *)(self + 0x32);
    switch (v) {
    case 0: case 0x1111: case 0x2222: case 0x3333: ++*(volatile uint16_t *)a; return;
    case 0x4444: case 0x5555: case 0x6666: case 0x7777: ++*(volatile uint16_t *)b; return;
    default: break;
    }
    for (int shift = 0; shift < 16; shift += 4) {
        if (((*(const uint16_t *)(self + 0x32) >> shift) & 7) < 4) ++*(volatile uint16_t *)a;
        else ++*(volatile uint16_t *)b;
    }
}
static const uint8_t tile_corners_count_original[] = {0x57, 0x8b, 0xf9, 0x8a, 0x47, 0x34, 0x84, 0xc0};
REWRITE("TileCorners::Count", 0x727b00, tile_corners_count_original, &tile_corners_count, &g_tile_counts);

// ---- Entry::Compare(a, b) (0x84D2D0): a sort comparison of two entries: -1, 0 or 1 ------------------------------------------
// By kind (+8, signed) first. Kind 0: by the first value (unsigned) and nothing more. Kind 1: by the first value
// (unsigned), then a set +0x18 flag first; kind 2: by +4 (unsigned). Then by +0xC, +0x14 (reversed) and +0x10 (signed).
static volatile long g_entry_compares;
static int32_t __stdcall entry_compare(const int32_t *a, const int32_t *b) {
    g_entry_compares++;
    if (a[2] != b[2]) return a[2] < b[2] ? -1 : 1;
    switch (a[2]) {
    case 0: return (uint32_t)b[0] > (uint32_t)a[0] ? -1 : (uint32_t)b[0] < (uint32_t)a[0] ? 1 : 0;
    case 1: {
        if (a[0] != b[0]) return (uint32_t)a[0] < (uint32_t)b[0] ? -1 : 1;
        const uint8_t fa = ((const uint8_t *)a)[0x18], fb = ((const uint8_t *)b)[0x18];
        if (fa && !fb) return 1;
        if (!fa && fb) return -1;
        break;
    }
    case 2:
        if (a[1] != b[1]) return (uint32_t)a[1] < (uint32_t)b[1] ? -1 : 1;
        break;
    default: break;
    }
    if (a[3] != b[3]) return a[3] > b[3] ? 1 : -1;
    if (a[5] != b[5]) return a[5] < b[5] ? 1 : -1;
    if (a[4] != b[4]) return a[4] > b[4] ? 1 : -1;
    return 0;
}
static const uint8_t entry_compare_original[] = {0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x44, 0x24, 0x08};
REWRITE("Entry::Compare", 0x84d2d0, entry_compare_original, &entry_compare, &g_entry_compares);

// ---- Lists::Remove(this, item) (0xA6BFD0): an item taken out of the first list it is in, keeping the order ---------------
// The first list at +0x6C (count +0x74); else the second, 8-byte entries keyed by their first value, at +0x78 (count +0x80).
static volatile long g_list_removes;
static void __thiscall lists_remove(uint8_t *self, uint32_t item) {
    g_list_removes++;
    volatile int32_t *const count1 = (volatile int32_t *)(self + 0x74);
    const int32_t n1 = *count1;
    int32_t i = 0;
    if (n1 > 0)
        while ((*(const uint32_t *const *)(self + 0x6c))[i] != item && ++i < *count1) {}
    if (i < n1) {
        for (int32_t j = i; j < n1 - 1 && j < *count1 - 1; j++) {
            volatile uint32_t *const l = *(volatile uint32_t *const *)(self + 0x6c);
            l[j] = l[j + 1];
        }
        *count1 = *count1 - 1;
        return;
    }
    volatile int32_t *const count2 = (volatile int32_t *)(self + 0x80);
    const int32_t n2 = *count2;
    i = 0;
    if (n2 > 0)
        while ((*(const uint32_t *const *)(self + 0x78))[2 * i] != item && ++i < *count2) {}
    if (i >= n2)
        return;
    for (int32_t j = i; j < n2 - 1 && j < *count2 - 1; j++) {
        volatile uint32_t *const l = *(volatile uint32_t *const *)(self + 0x78);
        l[2 * j] = l[2 * j + 2];
        l[2 * j + 1] = l[2 * j + 3];
    }
    *count2 = *count2 - 1;
}
static const uint8_t lists_remove_original[] = {0x53, 0x56, 0x8b, 0x71, 0x74, 0x33, 0xc0, 0x85};
REWRITE("Lists::Remove", 0xa6bfd0, lists_remove_original, &lists_remove, &g_list_removes);

// ---- Binary searches of sorted tables by an unsigned number ---------------------------------------------------------------
// (key, table, count, place, -): *key looked for among the entries' first values (entries of the stride given), sorted
// unsigned. Gives (in the low byte) 1 and the place when found, else 0 and where it would go (0 for an empty table).
template <int Stride> static uint8_t sorted_table_find(const uint32_t *key, const uint8_t *table, int32_t count, int32_t *place) {
    if (count < 1) {
        *(volatile int32_t *)place = 0;
        return 0;
    }
    const uint32_t k = *key;
    int32_t lo = 0, hi = count - 1;
    for (;;) {
        const int32_t mid = (hi + lo) >> 1;
        const uint32_t v = *(const uint32_t *)(table + mid * Stride);
        if (v > k) {
            if (mid > lo) {
                hi = mid - 1;
                continue;
            }
            *(volatile int32_t *)place = lo;
            return 0;
        }
        if (v == k) {
            *(volatile int32_t *)place = mid;
            return 1;
        }
        if (mid < hi) {
            lo = mid + 1;
            continue;
        }
        *(volatile int32_t *)place = hi + 1;
        return 0;
    }
}
static volatile long g_table_finds;
static uint8_t __fastcall sorted_table_find20(const uint32_t *key, const uint8_t *table, int32_t count, int32_t *place, uint32_t) {
    g_table_finds++;
    return sorted_table_find<20>(key, table, count, place);
}
static uint8_t __fastcall sorted_array_find(const uint32_t *key, const uint8_t *table, int32_t count, int32_t *place, uint32_t) {
    g_table_finds++;
    return sorted_table_find<4>(key, table, count, place);
}
static uint8_t __fastcall sorted_table_find8(const uint32_t *key, const uint8_t *table, int32_t count, int32_t *place, uint32_t) {
    g_table_finds++;
    return sorted_table_find<8>(key, table, count, place);
}
static const uint8_t sorted_table_find_original[] = {0x8b, 0x44, 0x24, 0x04, 0x83, 0xf8, 0x01, 0x7d};
REWRITE("SortedTable::Find20", 0x90c5c0, sorted_table_find_original, &sorted_table_find20, &g_table_finds);
REWRITE("SortedArray::Find", 0x801450, sorted_table_find_original, &sorted_array_find, &g_table_finds);
REWRITE("SortedTable::Find8", 0x4453d0, sorted_table_find_original, &sorted_table_find8, &g_table_finds);

// ---- SortedPointers::FindBy8(key, list, count, place, -) (0xA90980): the same search over objects by their +8 (unsigned) ---
// The key is (*key)'s +8.
static uint8_t __fastcall sorted_pointers_find_by8(const uint8_t *const *key, const uint8_t *const *list, int32_t count, int32_t *place, uint32_t) {
    g_table_finds++;
    if (count < 1) {
        *(volatile int32_t *)place = 0;
        return 0;
    }
    const uint32_t k = *(const uint32_t *)(*key + 8);
    int32_t lo = 0, hi = count - 1;
    for (;;) {
        const int32_t mid = (hi + lo) >> 1;
        const uint32_t v = *(const uint32_t *)(list[mid] + 8);
        if (k < v) {
            if (mid > lo) {
                hi = mid - 1;
                continue;
            }
            *(volatile int32_t *)place = lo;
            return 0;
        }
        if (k == v) {
            *(volatile int32_t *)place = mid;
            return 1;
        }
        if (mid < hi) {
            lo = mid + 1;
            continue;
        }
        *(volatile int32_t *)place = hi + 1;
        return 0;
    }
}
static const uint8_t sorted_pointers_find_by8_original[] = {0x8b, 0x44, 0x24, 0x04, 0x83, 0xf8, 0x01, 0x7d};
REWRITE("SortedPointers::FindBy8", 0xa90980, sorted_pointers_find_by8_original, &sorted_pointers_find_by8, &g_table_finds);

// ---- SortedTable::Find64(this, key_lo, key_hi, place) (0x9F9130): binary search for a signed 64-bit key -------------------
// Pointers to 64-bit values at +0x84 (count +0x8C). Gives (in the low byte) 1 and the place when found, else 0 and the
// place it would go.
static uint8_t __thiscall sorted_table_find64(const uint8_t *self, uint32_t key_lo, int32_t key_hi, int32_t *place) {
    g_table_finds++;
    const int64_t k = (int64_t)((uint64_t)(uint32_t)key_hi << 32 | key_lo);
    const int64_t *const *const list = *(const int64_t *const *const *)(self + 0x84);
    int32_t lo = 0, hi = *(const int32_t *)(self + 0x8c) - 1;
    while (lo <= hi) {
        const int32_t mid = (hi - lo) / 2 + lo;
        const int64_t v = *list[mid];
        if (k > v)
            lo = mid + 1;
        else if (k == v) {
            *(volatile int32_t *)place = mid;
            return 1;
        } else
            hi = mid - 1;
    }
    *(volatile int32_t *)place = lo;
    return 0;
}
static const uint8_t sorted_table_find64_original[] = {0x53, 0x55, 0x56, 0x57, 0x8b, 0xb9, 0x8c, 0x00};
REWRITE("SortedTable::Find64", 0x9f9130, sorted_table_find64_original, &sorted_table_find64, &g_table_finds);

// ---- Pairs::FindSorted(list, a, b, count) (0xC22B60): a pair of numbers in a sorted list of pairs, or -1 -------------------
// Steps from the highest power of two not above the count, halving: from lo (-1 at first), the place lo + step (when
// inside) is taken when its pair is below (a, b) (signed); an equal pair gives its place.
static int32_t __thiscall pairs_find_sorted(const int32_t *list, int32_t a, int32_t b, int32_t count) {
    g_pair_finds++;
    uint32_t step = (uint32_t)count;
    step |= step >> 1, step |= step >> 2, step |= step >> 4, step |= step >> 8, step |= step >> 16;
    step &= ~(step >> 1);
    int32_t lo = -1;
    for (; step; step >>= 1) {
        const int32_t i = lo + (int32_t)step;
        if (i >= count)
            continue;
        if (a > list[2 * i])
            lo = i;
        else if (a == list[2 * i]) {
            if (b > list[2 * i + 1])
                lo = i;
            else if (b == list[2 * i + 1])
                return i;
        }
    }
    return -1;
}
static const uint8_t pairs_find_sorted_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x10, 0x8b, 0xd3, 0xd1};
REWRITE("Pairs::FindSorted", 0xc22b60, pairs_find_sorted_original, &pairs_find_sorted, &g_pair_finds);

// ---- SortedPointers::FindXY2 / Whirlpool::FindCell(this, x, y, place) (0x8FEDA0, 0x8FED20): the entry at (x, y) -----------
// FindXY2: entries (pointers at +0x14, count +0x18) sorted by x (+0) then y (+4); FindCell: cells (+0x10, count +0x18)
// sorted by row (+4) then column (+0). Gives the entry and its place, or null and where it would go (place when asked).
template <bool RowFirst> static const int32_t *find_xy(const int32_t *const *list, int32_t count, int32_t x, int32_t y, int32_t *place) {
    int32_t lo = 0, hi = count - 1;
    while (lo <= hi) {
        const int32_t mid = (hi - lo) / 2 + lo;
        const int32_t *const e = list[mid];
        const int32_t k1 = RowFirst ? y : x, v1 = RowFirst ? e[1] : e[0], k2 = RowFirst ? x : y, v2 = RowFirst ? e[0] : e[1];
        if (k1 < v1 || (k1 == v1 && k2 < v2))
            hi = mid - 1;
        else if (k1 > v1 || k2 > v2)
            lo = mid + 1;
        else {
            if (place)
                *(volatile int32_t *)place = mid;
            return e;
        }
    }
    if (place)
        *(volatile int32_t *)place = lo;
    return nullptr;
}
static const int32_t *__thiscall sorted_pointers_find_xy2(const uint8_t *self, int32_t x, int32_t y, int32_t *place) {
    g_xy_finds++;
    return find_xy<false>(*(const int32_t *const *const *)(self + 0x14), *(const int32_t *)(self + 0x18), x, y, place);
}
static const int32_t *__thiscall whirlpool_find_cell(const uint8_t *self, int32_t column, int32_t row, int32_t *place) {
    g_xy_finds++;
    return find_xy<true>(*(const int32_t *const *const *)(self + 0x10), *(const int32_t *)(self + 0x18), column, row, place);
}
static const uint8_t sorted_pointers_find_xy2_original[] = {0x51, 0x53, 0x55, 0x56, 0x57, 0x8b, 0x79, 0x18};
REWRITE("SortedPointers::FindXY2", 0x8feda0, sorted_pointers_find_xy2_original, &sorted_pointers_find_xy2, &g_xy_finds);
REWRITE("Whirlpool::FindCell", 0x8fed20, sorted_pointers_find_xy2_original, &whirlpool_find_cell, &g_xy_finds);

// ---- Frame::PickAxis(this, out, frame) (0x581530): one of a frame's axes copied: the first when +4 is set, else the second
static volatile long g_axis_picks;
static uint32_t *__thiscall frame_pick_axis(const uint8_t *self, uint32_t *out, const uint32_t *frame) {
    g_axis_picks++;
    const uint32_t *const axis = *(const uint32_t *)(self + 4) ? frame : frame + 4;
    volatile uint32_t *const o = out;
    o[0] = axis[0], o[1] = axis[1], o[2] = axis[2];
    return out;
}
static const uint8_t frame_pick_axis_original[] = {0x56, 0x8b, 0x71, 0x04, 0x33, 0xc0, 0x33, 0xd2};
REWRITE("Frame::PickAxis", 0x581530, frame_pick_axis_original, &frame_pick_axis, &g_axis_picks);

// ---- Mesh::CountForId(this, id, vertices, indices) (0x95B610): the vertex and index counts of a mesh's parts with an id ----
// Parts of 16 bytes at +0x14 (count +0x1C), id the word at +8: by the shape bits (byte +0xA & 0xE): 2, 4, 8 and 14 add 3 and
// 3; 6, 10 and 12 add 4 and 6; 0 nothing.
static volatile long g_mesh_counts;
static void __thiscall mesh_count_for_id(const uint8_t *self, uint32_t id, int32_t *vertices, int32_t *indices) {
    g_mesh_counts++;
    volatile int32_t *const v = vertices, *const x = indices;
    *v = 0;
    *x = 0;
    for (int32_t i = 0; i < *(const volatile int32_t *)(self + 0x1c); i++) {
        const uint8_t *const part = *(const uint8_t *const *)(self + 0x14) + i * 16;
        if (*(const uint16_t *)(part + 8) != id)
            continue;
        switch (part[0xa] & 0xe) {
        case 2: case 4: case 8: case 14: *v = *v + 3, *x = *x + 3; break;
        case 6: case 10: case 12: *v = *v + 4, *x = *x + 6; break;
        default: break;
        }
    }
}
static const uint8_t mesh_count_for_id_original[] = {0x8b, 0x54, 0x24, 0x08, 0x55, 0x56, 0x8b, 0x74};
REWRITE("Mesh::CountForId", 0x95b610, mesh_count_for_id_original, &mesh_count_for_id, &g_mesh_counts);

// ---- Pairs::IndexOf(this, a, b) (0xBC2A30): the number of a pair of 16-bit ids -----------------------------------------------
// For each id i (words at +0x24, count +8) equal to a: its slot (+0x18, four bytes: partner count, first partner) lists
// partners (words at +0x28); the first equal to b gives the slot's first number plus its place. Not found: 0xFFFF in the
// low half (the count's high half above it).
static volatile long g_pair_indices;
static uint32_t __thiscall pairs_index_of(const uint8_t *self, uint32_t a, uint32_t b) {
    g_pair_indices++;
    uint32_t i = 0;
    for (; i < *(const volatile uint32_t *)(self + 8); i++) {
        if ((*(const uint16_t *const *)(self + 0x24))[i] != (uint16_t)a)
            continue;
        const uint16_t *const slot = *(const uint16_t *const *)(self + 0x18) + 2 * i;
        const int32_t n = slot[0];
        const uint16_t *const partners = *(const uint16_t *const *)(self + 0x28) + slot[1];
        for (int32_t j = 0; j < n; j++)
            if (partners[j] == (uint16_t)b)
                return (uint32_t)(*(const uint16_t *const *)(self + 0x18))[2 * i + 1] + (uint32_t)j;
    }
    return (*(const volatile uint32_t *)(self + 8) & 0xffff0000u) | 0xffff;
}
static const uint8_t pairs_index_of_original[] = {0x8b, 0x41, 0x08, 0x53, 0x55, 0x56, 0x57, 0x33};
REWRITE("Pairs::IndexOf", 0xbc2a30, pairs_index_of_original, &pairs_index_of, &g_pair_indices);

// ---- Lights::ColourFor(this, out, id) (0xA35440): the colour (4 values, the light's +0x7C) of the light with an id ----------
// Ids inline at +0xF0 (count +0x108), lights at +0xD8; all 0 when none. Gives out.
static volatile long g_light_colours;
static uint32_t *__thiscall lights_colour_for(const uint8_t *self, uint32_t *out, uint32_t id) {
    g_light_colours++;
    volatile uint32_t *const o = out;
    const int32_t n = *(const int32_t *)(self + 0x108);
    for (int32_t i = 0; i < n; i++)
        if (((const uint32_t *)(self + 0xf0))[i] == id) {
            const uint32_t *const c = (const uint32_t *)(((const uint8_t *const *)(self + 0xd8))[i] + 0x7c);
            o[0] = c[0], o[1] = c[1], o[2] = c[2], o[3] = c[3];
            return out;
        }
    o[0] = 0, o[1] = 0, o[2] = 0, o[3] = 0;
    return out;
}
static const uint8_t lights_colour_for_original[] = {0x56, 0x8b, 0xb1, 0x08, 0x01, 0x00, 0x00, 0x33};
REWRITE("Lights::ColourFor", 0xa35440, lights_colour_for_original, &lights_colour_for, &g_light_colours);

// ---- Keys::RemoveAt(this, n) (0xE042B0): entry n of a list of 12-byte entries (+0x10, count +0x14) joined with the next ----
// Entry n's +8 grows by the next one's, its +4 becomes the next one's; the entries after the next move down and the
// count drops by one.
static volatile long g_key_removals;
static void __thiscall keys_remove_at(uint8_t *self, int32_t n) {
    g_key_removals++;
    volatile uint32_t *e = (volatile uint32_t *)(*(uint8_t *const *)(self + 0x10) + n * 12);
    e[2] = e[2] + e[5];
    e = (volatile uint32_t *)(*(uint8_t *const *)(self + 0x10) + n * 12);
    e[1] = e[4];
    volatile int32_t *const count = (volatile int32_t *)(self + 0x14);
    for (int32_t j = n + 1; j < *count - 1; j++) {
        volatile uint32_t *const d = (volatile uint32_t *)(*(uint8_t *const *)(self + 0x10) + j * 12);
        d[0] = d[3], d[1] = d[4], d[2] = d[5];
    }
    *count = *count - 1;
}
static const uint8_t keys_remove_at_original[] = {0x8b, 0x51, 0x10, 0x53, 0x56, 0x8b, 0x74, 0x24};
REWRITE("Keys::RemoveAt", 0xe042b0, keys_remove_at_original, &keys_remove_at, &g_key_removals);

// ---- Settings::Downgrade(this) (0x5D13C0): values only a higher setting allows lowered, in a list of ten -------------------
// 0 and 1: 7 to 3; 2: 8 to 4; 3 to 7: 2 to 0; 8: 2 to 1; 9: 3 to 2.
static volatile long g_downgrades;
static void __thiscall settings_downgrade(int32_t *self) {
    g_downgrades++;
    volatile int32_t *const s = self;
    if (s[0] == 7) s[0] = 3;
    if (s[1] == 7) s[1] = 3;
    if (s[2] == 8) s[2] = 4;
    for (int i = 3; i < 8; i++)
        if (s[i] == 2) s[i] = 0;
    if (s[8] == 2) s[8] = 1;
    if (s[9] == 3) s[9] = 2;
}
static const uint8_t settings_downgrade_original[] = {0x83, 0x39, 0x07, 0x75, 0x06, 0xc7, 0x01, 0x03};
REWRITE("Settings::Downgrade", 0x5d13c0, settings_downgrade_original, &settings_downgrade, &g_downgrades);

// ---- Window::FirstFocusable(this) (0x6ADC50): the first child of the window's lists with bit 2 of its flags set, or null ---
// The list at +0x64, then +0x4C (nodes: next +0, child +8, until a node whose next is null); flags at the child's +0x94.
static volatile long g_focusables;
static uint32_t __thiscall window_first_focusable(const uint8_t *self) {
    g_focusables++;
    static const uint8_t lists[] = {0x64, 0x4c};
    for (const uint8_t at : lists) {
        const uint32_t *node = *(const uint32_t *const *)(self + at);
        while (*node && node) {
            const uint8_t *const child = (const uint8_t *)(uintptr_t)node[2];
            if (child && (*(const uint32_t *)(child + 0x94) >> 2) & 1 && node[2])
                return node[2];
            node = (const uint32_t *)(uintptr_t)*node;
        }
    }
    return 0;
}
static const uint8_t window_first_focusable_original[] = {0x8b, 0x51, 0x64, 0x8b, 0x02, 0x56, 0x57, 0x33};
REWRITE("Window::FirstFocusable", 0x6adc50, window_first_focusable_original, &window_first_focusable, &g_focusables);

// ---- SortedPointers::FindInt(this, key, place) (0x8FD720): binary search of objects by their first value (signed) ----------
// Pointers at +0, count +4. Gives the object and its place, or null and where it would go (place when asked).
static const int32_t *__thiscall sorted_pointers_find_int(const uint8_t *self, int32_t key, int32_t *place) {
    g_pointer_finds++;
    const int32_t *const *const list = *(const int32_t *const *const *)self;
    int32_t lo = 0, hi = *(const int32_t *)(self + 4) - 1;
    while (lo <= hi) {
        const int32_t mid = (hi - lo) / 2 + lo;
        const int32_t *const o = list[mid];
        if (key > *o)
            lo = mid + 1;
        else if (key < *o)
            hi = mid - 1;
        else {
            if (place)
                *(volatile int32_t *)place = mid;
            return o;
        }
    }
    if (place)
        *(volatile int32_t *)place = lo;
    return nullptr;
}
static const uint8_t sorted_pointers_find_int_original[] = {0x53, 0x55, 0x56, 0x57, 0x8b, 0x79, 0x04, 0x33};
REWRITE("SortedPointers::FindInt", 0x8fd720, sorted_pointers_find_int_original, &sorted_pointers_find_int, &g_pointer_finds);

// ---- Description::Compare(this, other) (0x713110): whether two descriptions match ------------------
// 0 when they differ (byte +0, +4 to +0xC, the two values +0x14 points at, +0x24 to +0x2C); otherwise -1 when their
// byte +1 differs, else 1.
static volatile long g_description_compares;
static int32_t __thiscall description_compare(const uint8_t *self, const uint8_t *other) {
    g_description_compares++;
    const uint32_t *const a = (const uint32_t *)self, *const b = (const uint32_t *)other;
    if (self[0] != other[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3]) return 0;
    const uint32_t *const pa = (const uint32_t *)(uintptr_t)a[5], *const pb = (const uint32_t *)(uintptr_t)b[5];
    if (pa[0] != pb[0] || pa[1] != pb[1]) return 0;
    if (a[9] != b[9] || a[10] != b[10] || a[11] != b[11]) return 0;
    return self[1] != other[1] ? -1 : 1;
}
static const uint8_t description_compare_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8a, 0x11, 0x3a, 0x10};
REWRITE("Description::Compare", 0x713110, description_compare_original, &description_compare, &g_description_compares);

// ---- Entries::FindByKey(this, mask, a, b, c, d) (0x4CC880): the value of the active entry matching a key, or 0 ------------
// Entries of 0x2C bytes at +0x94, looked through until +0x9C active ones (byte +0) have been seen: one with flags (+4)
// meeting the mask and +8 to +0x14 equal to a-d gives its +0x1C's +0xC.
static volatile long g_key_entry_finds;
static uint32_t __thiscall entries_find_by_key(const uint8_t *self, uint32_t mask, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    g_key_entry_finds++;
    const int32_t n = *(const int32_t *)(self + 0x9c);
    const uint8_t *const base = *(const uint8_t *const *)(self + 0x94);
    if (n <= 0)
        return 0;
    int32_t active = 0;
    for (const uint8_t *e = base;; e += 0x2c) {
        const uint32_t *const v = (const uint32_t *)e;
        if (e[0]) {
            active++;
            if ((v[1] & mask) && v[2] == a && v[3] == b && v[4] == c && v[5] == d)
                return *(const uint32_t *)((const uint8_t *)(uintptr_t)v[7] + 0xc);
        }
        if (active >= n)
            return 0;
    }
}
static const uint8_t entries_find_by_key_original[] = {0x53, 0x55, 0x56, 0x8b, 0xb1, 0x9c, 0x00, 0x00};
REWRITE("Entries::FindByKey", 0x4cc880, entries_find_by_key_original, &entries_find_by_key, &g_key_entry_finds);

// ---- Math::IntSqrt(n) (0xDF88F0): the whole-number square root of a number ------------------------------------------------
// 0x8000 above 2^30; 0 for 0. A first guess c: the first power of two whose square reaches n; then three Newton steps
// c = (c c + n) / (2 c) (signed), stopping early on a negative one.
static volatile long g_int_sqrts;
static int32_t __cdecl math_int_sqrt(int32_t n) {
    g_int_sqrts++;
    if (n > 0x40000000)
        return 0x8000;
    int32_t c = 1;
    if (n > 1)
        do c += c;
        while (c * c < n);
    if (!n)
        return 0;
    for (int step = 0; step < 3; step++) {
        c = (c * c + n) / (c + c);
        if (c < 0)
            break;
    }
    return c;
}
static const uint8_t math_int_sqrt_original[] = {0x56, 0x8b, 0x74, 0x24, 0x08, 0x81, 0xfe, 0x00};
REWRITE("Math::IntSqrt", 0xdf88f0, math_int_sqrt_original, &math_int_sqrt, &g_int_sqrts);

// ---- ListNode::IsLinked(node) (0xCC21F0): whether a list node is properly in a list (in the low byte) -----------------------
// Not null, both neighbours (next +0, previous +4) set, and each pointing back at it.
static volatile long g_link_checks;
static uint8_t __thiscall list_node_is_linked(const uint32_t *node) {
    g_link_checks++;
    return node && node[1] && node[0] && ((const uint32_t *)(uintptr_t)node[0])[1] == (uint32_t)(uintptr_t)node &&
           ((const uint32_t *)(uintptr_t)node[1])[0] == (uint32_t)(uintptr_t)node;
}
static const uint8_t list_node_is_linked_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x08, 0xc7, 0x45};
REWRITE("ListNode::IsLinked", 0xcc21f0, list_node_is_linked_original, &list_node_is_linked, &g_link_checks);

// ---- Rect::AddCell(this, x, y) (0x71BFE0): a byte rectangle (+0x92: min x, min y, max x, max y) grown to take in a cell ---
static void __thiscall rect_add_cell(uint8_t *self, uint32_t x_arg, uint32_t y_arg) {
    g_path_bounds++;
    const uint8_t x = (uint8_t)x_arg, y = (uint8_t)y_arg;
    volatile uint8_t *const b = self + 0x92;
    b[0] = x < b[0] ? x : b[0];
    b[1] = y < b[1] ? y : b[1];
    b[2] = x > b[2] ? x : b[2];
    b[3] = y > b[3] ? y : b[3];
}
static const uint8_t rect_add_cell_original[] = {0x8a, 0x81, 0x92, 0x00, 0x00, 0x00, 0x53, 0x8a};
REWRITE("Rect::AddCell", 0x71bfe0, rect_add_cell_original, &rect_add_cell, &g_path_bounds);

// ---- Rect::GrowByOne(this, x0, y0, x1, y1) (0x8BCFF0): a byte rectangle grown by one cell each way, kept in the grid ------
// The low corner moves down to no less than 0, the high one up to no more than the grid's width and height (bytes +4, +5)
// less 1.
static void __thiscall rect_grow_by_one(const uint8_t *self, uint8_t *x0, uint8_t *y0, uint8_t *x1, uint8_t *y1) {
    g_path_bounds++;
    *(volatile uint8_t *)x0 = *x0 ? (uint8_t)(*x0 - 1) : 0;
    *(volatile uint8_t *)y0 = *y0 ? (uint8_t)(*y0 - 1) : 0;
    if ((int32_t)*x1 < (int32_t)self[4] - 1) *(volatile uint8_t *)x1 = (uint8_t)(*x1 + 1);
    else *(volatile uint8_t *)x1 = *x1;
    if ((int32_t)*y1 < (int32_t)self[5] - 1) *(volatile uint8_t *)y1 = (uint8_t)(*y1 + 1);
    else *(volatile uint8_t *)y1 = *y1;
}
static const uint8_t rect_grow_by_one_original[] = {0x8b, 0x54, 0x24, 0x04, 0x8a, 0x02, 0x84, 0xc0};
REWRITE("Rect::GrowByOne", 0x8bcff0, rect_grow_by_one_original, &rect_grow_by_one, &g_path_bounds);

// ---- PointerArray::RemoveSwap(this, value) (0x80EE80): a value taken out of an array (+0x8C, count +0x94) by moving the ---
// last entry (as the count was) into its place; the count drops by one.
static volatile long g_swap_removes;
static void __thiscall pointer_array_remove_swap(uint8_t *self, uint32_t value) {
    g_swap_removes++;
    volatile int32_t *const count = (volatile int32_t *)(self + 0x94);
    const int32_t n = *count;
    volatile uint32_t *const list = *(volatile uint32_t *const *)(self + 0x8c);
    for (int32_t i = 0; i < n && i < *count; i++)
        if (list[i] == value) {
            list[i] = list[n - 1];
            *count = *count - 1;
            return;
        }
}
static const uint8_t pointer_array_remove_swap_original[] = {0x57, 0x8b, 0xb9, 0x94, 0x00, 0x00, 0x00, 0x33};
REWRITE("PointerArray::RemoveSwap", 0x80ee80, pointer_array_remove_swap_original, &pointer_array_remove_swap, &g_swap_removes);

// ---- ParticleBaseDesc::RemoveLink(this, value) (0x9CEB20): a value taken out of the array at +0x70 (count +0x78), the -----
// later entries moved down
static void __thiscall particle_base_desc_remove_link(uint8_t *self, uint32_t value) {
    g_link_removals++;
    remove_from(self, 0x70, value, 8);
}
static const uint8_t particle_base_desc_remove_link_original[] = {0x56, 0x8b, 0x71, 0x78, 0x33, 0xc0, 0x85, 0xf6};
REWRITE("ParticleBaseDesc::RemoveLink", 0x9ceb20, particle_base_desc_remove_link_original, &particle_base_desc_remove_link, &g_link_removals);

// ---- Queue::HasId(this, id) (0x9465E0): whether a queue holds an active entry with an id (in the low byte) -----------------
// Nothing when +0x68 equals +0x74; otherwise the entries (pointers at +0x64) from +0x7C up to +0x80: one with its id
// (+0x3C) and active (+0x38 set).
static volatile long g_queue_ids;
static uint8_t __thiscall queue_has_id(const uint8_t *self, uint32_t id) {
    g_queue_ids++;
    const uint32_t *const t = (const uint32_t *)self;
    if (t[0x68 / 4] == t[0x74 / 4])
        return 0;
    const uint32_t *const base = (const uint32_t *)(uintptr_t)t[0x64 / 4];
    for (const uint32_t *p = base + t[0x7c / 4]; p < base + t[0x80 / 4]; p++) {
        const uint8_t *const e = (const uint8_t *)(uintptr_t)*p;
        if (e && *(const uint32_t *)(e + 0x3c) == id && *(const uint32_t *)(e + 0x38))
            return 1;
    }
    return 0;
}
static const uint8_t queue_has_id_original[] = {0x8b, 0x41, 0x68, 0x2b, 0x41, 0x74, 0x56, 0x57};
REWRITE("Queue::HasId", 0x9465e0, queue_has_id_original, &queue_has_id, &g_queue_ids);

// ---- WideString::Compare(this, other) (0xBCE970): two wide strings compared: the first difference of their characters ------
// (this's less other's), else this's length less other's.
static int32_t __thiscall wide_string_compare(const uint8_t *const *self, const uint8_t *const *other) {
    g_raw_compares++;
    const uint8_t *const a = *self, *const b = *other;
    const uint16_t *const ta = *(const uint16_t *const *)(a + 0xc), *const tb = *(const uint16_t *const *)(b + 0xc);
    const uint32_t la = *(const uint32_t *)(a + 4), lb = *(const uint32_t *)(b + 4);
    const uint32_t m = lb < la ? lb : la;
    for (uint32_t i = 0; i < m; i++)
        if (const int32_t d = (int32_t)ta[i] - (int32_t)tb[i])
            return d;
    return (int32_t)(la - lb);
}
static const uint8_t wide_string_compare_original[] = {0x8b, 0x01, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x09};
REWRITE("WideString::Compare", 0xbce970, wide_string_compare_original, &wide_string_compare, &g_raw_compares);

// ---- Vertices::SetSecond(this) (0x8EF180): a list of values (+0x10) copied into the second value of each 16-byte vertex ---
// (+0x14); (+8 + +0xC) twice over.
static volatile long g_second_sets;
static void __thiscall vertices_set_second(const uint8_t *self) {
    g_second_sets++;
    const int32_t n = (*(const int32_t *)(self + 0xc) + *(const int32_t *)(self + 8)) * 2;
    const uint32_t *const from = *(const uint32_t *const *)(self + 0x10);
    volatile uint32_t *const to = *(volatile uint32_t *const *)(self + 0x14);
    for (int32_t i = 0; i < n; i++) to[4 * i + 1] = from[i];
}
static const uint8_t vertices_set_second_original[] = {0x8b, 0x51, 0x14, 0x8b, 0x41, 0x10, 0x56, 0x8b};
REWRITE("Vertices::SetSecond", 0x8ef180, vertices_set_second_original, &vertices_set_second, &g_second_sets);

// ---- Pieces::PairAllowed(a, b) (0xBC5EB0): whether two pieces make an allowed pair ------------------------------------------
// One of kind (+0x10) 0x4A or 0x4C with the other of kind 0 and type (+8) 1, 2, 4 or 5.
static volatile long g_pair_checks;
static inline bool piece_end(int32_t kind) { return kind == 0x4a || kind == 0x4c; }
static inline bool piece_type(int32_t type) { return type == 2 || type == 5 || type == 1 || type == 4; }
static uint32_t __fastcall pieces_pair_allowed(const int32_t *a, const int32_t *b) {
    g_pair_checks++;
    if (piece_end(a[4]) && !b[4] && piece_type(b[2])) return 1;
    if (piece_end(b[4]) && !a[4] && piece_type(a[2])) return 1;
    return 0;
}
static const uint8_t pieces_pair_allowed_original[] = {0x8b, 0x41, 0x10, 0x83, 0xf8, 0x4a, 0x56, 0x74};
REWRITE("Pieces::PairAllowed", 0xbc5eb0, pieces_pair_allowed_original, &pieces_pair_allowed, &g_pair_checks);

// ---- TerrainLayers::SetFlag(this, list, flag, value) (0xAFA080): the value (+0x24) and flag (bit 1 of +0x10) of layers ----
// The layers named by the list (its start +0 and count +4 into the indices at +8; layers of 0x2C bytes at +0x14); then
// byte +0x34 = 1.
static volatile long g_layer_flags;
static void __thiscall terrain_layers_set_flag(uint8_t *self, const int32_t *list, uint32_t flag, uint32_t value) {
    g_layer_flags++;
    for (int32_t i = 0; i < ((const volatile int32_t *)list)[1]; i++) {
        const int32_t index = (*(const int32_t *const *)(self + 8))[list[0] + i];
        uint8_t *const e = *(uint8_t *const *)(self + 0x14) + index * 0x2c;
        *(volatile uint32_t *)(e + 0x24) = value;
        volatile uint32_t *const f = (volatile uint32_t *)(*(uint8_t *const *)(self + 0x14) + index * 0x2c + 0x10);
        *f = (uint8_t)flag ? *f | 2 : *f & 0xfffffffd;
    }
    ((volatile uint8_t *)self)[0x34] = 1;
}
static const uint8_t terrain_layers_set_flag_original[] = {0x56, 0x57, 0x8b, 0x7c, 0x24, 0x0c, 0x8b, 0x47};
REWRITE("TerrainLayers::SetFlag", 0xafa080, terrain_layers_set_flag_original, &terrain_layers_set_flag, &g_layer_flags);

// ---- Area::Move(this, dx, dy) (0x7339B0): an area's rectangles (+0x40, +0x48, +0x58) and items (+0x64) moved by an offset --
static volatile long g_area_moves;
static void __thiscall area_move(uint8_t *self, int32_t dx, int32_t dy) {
    g_area_moves++;
    volatile int32_t *const t = (volatile int32_t *)self;
    const int32_t a48 = t[0x48 / 4], a44 = t[0x44 / 4];
    t[0x48 / 4] = a48 + dx;
    const int32_t a4c = t[0x4c / 4];
    t[0x40 / 4] = t[0x40 / 4] + dx;
    t[0x4c / 4] = a4c + dy;
    t[0x44 / 4] = a44 + dy;
    const int32_t a58 = t[0x58 / 4], a5c = t[0x5c / 4];
    t[0x5c / 4] = a5c + dy;
    t[0x58 / 4] = a58 + dx;
    for (int32_t i = 0; i < t[0x68 / 4]; i++) {
        volatile int32_t *const item = (volatile int32_t *)(uintptr_t)t[0x64 / 4] + 2 * i;
        item[0] = item[0] + dx;
        item[1] = item[1] + dy;
    }
}
static const uint8_t area_move_original[] = {0x8b, 0x51, 0x48, 0x8b, 0x41, 0x44, 0x56, 0x8b};
REWRITE("Area::Move", 0x7339b0, area_move_original, &area_move, &g_area_moves);

// ---- FreeList::Release(this, head, n) (0x8BFEF0): entry n taken out of a chain and put on the free list --------------------
// Entries of 16 bytes at +0 (count +4), next at +4; the chain starts at *head and runs while below the count. n's
// neighbour (or *head) is joined past it; then n's next is the free list (+8) and the free list starts at n.
static volatile long g_free_releases;
static void __thiscall free_list_release(uint32_t *self, uint32_t *head, uint32_t n) {
    g_free_releases++;
    volatile uint32_t *const t = self;
    const uint32_t count = t[1];
    uint32_t prev = 0xffffffff, cur = *head;
    while (cur < count) {
        const uint32_t next = ((const uint32_t *)(uintptr_t)t[0])[cur * 4 + 1];
        if (cur == n) {
            if (prev < count) ((volatile uint32_t *)(uintptr_t)t[0])[prev * 4 + 1] = next;
            else *(volatile uint32_t *)head = next;
            break;
        }
        if (!(next < t[1]))
            break;
        prev = cur;
        cur = next;
    }
    ((volatile uint32_t *)(uintptr_t)t[0])[n * 4 + 1] = t[2];
    t[2] = n;
}
static const uint8_t free_list_release_original[] = {0x53, 0x8b, 0x59, 0x04, 0x55, 0x8b, 0x6c, 0x24};
REWRITE("FreeList::Release", 0x8bfef0, free_list_release_original, &free_list_release, &g_free_releases);

// ---- Timers::Tick55(this) (0x7A9150): 55 flagged timers of each object of a list counted down --------------------------------
// The list at +0x50C (nodes: next +0, object +8, until a node whose next is null). Timer c (+0x18 on) counts when bit
// c & 31 of +0x14 is set; one that goes below 0 is set to 0 and its bit cleared.
static volatile long g_timer_ticks;
static void __thiscall timers_tick55(const uint8_t *self) {
    g_timer_ticks++;
    const uint32_t *node = *(const uint32_t *const *)(self + 0x50c);
    if (!*node || !node)
        return;
    do {
        uint8_t *const o = (uint8_t *)(uintptr_t)node[2];
        volatile uint32_t *const flags = (volatile uint32_t *)(o + 0x14);
        volatile int32_t *const timers = (volatile int32_t *)(o + 0x18);
        for (int c = 0; c < 0x37; c++) {
            const uint32_t bit = 1u << (c & 31);
            if (!(*flags & bit))
                continue;
            const int32_t t = timers[c] - 1;
            timers[c] = t;
            if (t < 0) {
                *flags = *flags & ~bit;
                timers[c] = 0;
            }
        }
        node = (const uint32_t *)(uintptr_t)*node;
    } while (*node && node);
}
static const uint8_t timers_tick55_original[] = {0x8b, 0x81, 0x0c, 0x05, 0x00, 0x00, 0x83, 0x38};
REWRITE("Timers::Tick55", 0x7a9150, timers_tick55_original, &timers_tick55, &g_timer_ticks);

// ---- Grid::ContainsId(this, id, x0, y0, x1, y1) (0x9375C0): whether a rectangle of a grid of 16-bit ids holds an id ---------
// Ids at +0x38 (width +0x30); the rectangle inclusive, signed. Gives the answer in the low byte.
static volatile long g_grid_ids;
static uint8_t __thiscall grid_contains_id(const uint8_t *self, uint32_t id, int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
    g_grid_ids++;
    const uint16_t *const ids = *(const uint16_t *const *)(self + 0x38);
    const int32_t w = *(const int32_t *)(self + 0x30);
    for (int32_t y = y0; y <= y1; y++)
        for (int32_t x = x0; x <= x1; x++)
            if (ids[w * y + x] == (uint16_t)id)
                return 1;
    return 0;
}
static const uint8_t grid_contains_id_original[] = {0x8b, 0x44, 0x24, 0x0c, 0x3b, 0x44, 0x24, 0x14};
REWRITE("Grid::ContainsId", 0x9375c0, grid_contains_id_original, &grid_contains_id, &g_grid_ids);

// ---- Bytes3List::Equal(this, list, n) (0xC44360): whether a list of three-byte values (4 apart) matches one given ----------
// Its count (+8) must be n and +4 clear; then each value's three bytes. Gives the answer in the low byte.
static volatile long g_bytes3_compares;
static uint8_t __thiscall bytes3_list_equal(const uint32_t *self, const uint8_t *list, uint32_t n) {
    g_bytes3_compares++;
    if (self[2] != n || self[1])
        return 0;
    const uint8_t *const own = (const uint8_t *)(uintptr_t)self[0];
    for (uint32_t i = 0; i < n; i++)
        if (own[4 * i] != list[4 * i] || own[4 * i + 1] != list[4 * i + 1] || own[4 * i + 2] != list[4 * i + 2])
            return 0;
    return 1;
}
static const uint8_t bytes3_list_equal_original[] = {0x8b, 0x41, 0x08, 0x57, 0x8b, 0x7c, 0x24, 0x0c};
REWRITE("Bytes3List::Equal", 0xc44360, bytes3_list_equal_original, &bytes3_list_equal, &g_bytes3_compares);

// ---- Bytes3::Equal(this, other) (0xCC4A30): whether two three-byte values are the same (in the low byte) --------------------
static uint8_t __thiscall bytes3_equal(const uint8_t *self, const uint8_t *other) {
    g_bytes3_compares++;
    return self[2] == other[2] && self[1] == other[1] && self[0] == other[0];
}
static const uint8_t bytes3_equal_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x08, 0xc7, 0x45};
REWRITE("Bytes3::Equal", 0xcc4a30, bytes3_equal_original, &bytes3_equal, &g_bytes3_compares);

// ---- Objects::HasWithBit(this, object, bit) (0x7A8F30): whether the list at +0x50C holds an object with the same id --------
// Nodes (next +0, object +8, until a node whose next is null); same id (+4) and, when a bit is given, bit & 31 of its
// flags (+0x14) set. Gives the answer in the low byte.
static volatile long g_bit_finds;
static uint8_t __thiscall objects_has_with_bit(const uint8_t *self, const uint8_t *object, uint32_t bit) {
    g_bit_finds++;
    const uint32_t *node = *(const uint32_t *const *)(self + 0x50c);
    if (!*node || !node)
        return 0;
    const uint32_t id = *(const uint32_t *)(object + 4);
    do {
        const uint8_t *const o = (const uint8_t *)(uintptr_t)node[2];
        if (id == *(const uint32_t *)(o + 4) && (!bit || (*(const uint32_t *)(o + 0x14) & (1u << (bit & 31)))))
            return 1;
        node = (const uint32_t *)(uintptr_t)*node;
    } while (*node && node);
    return 0;
}
static const uint8_t objects_has_with_bit_original[] = {0x8b, 0x81, 0x0c, 0x05, 0x00, 0x00, 0x83, 0x38};
REWRITE("Objects::HasWithBit", 0x7a8f30, objects_has_with_bit_original, &objects_has_with_bit, &g_bit_finds);

// ---- Cars::TotalLoad(this) (0x9892A0): the sum of each car's load over 10 --------------------------------------------------
// Cars of 0x58 bytes at +0x104 (count +0x108): through the car's +0x48 and that's +0xCC, the load at +0xC4 (signed / 10).
static volatile long g_car_loads;
static int32_t __thiscall cars_total_load(const uint8_t *self) {
    g_car_loads++;
    int32_t sum = 0;
    const int32_t n = *(const int32_t *)(self + 0x108);
    const uint8_t *const cars = *(const uint8_t *const *)(self + 0x104);
    for (int32_t i = 0; i < n; i++) {
        const uint8_t *const p = *(const uint8_t *const *)(cars + i * 0x58 + 0x48);
        if (!p)
            continue;
        const uint8_t *const q = *(const uint8_t *const *)(p + 0xcc);
        if (q)
            sum += *(const int32_t *)(q + 0xc4) / 10;
    }
    return sum;
}
static const uint8_t cars_total_load_original[] = {0x8b, 0x81, 0x08, 0x01, 0x00, 0x00, 0x57, 0x33};
REWRITE("Cars::TotalLoad", 0x9892a0, cars_total_load_original, &cars_total_load, &g_car_loads);

// ---- String::Compare(this, other) (0xBCF040): two strings compared: the first difference of their characters (signed bytes,
// this's less other's), else this's length less other's -------------------------------------------------------------------
static int32_t __thiscall string_compare(const uint8_t *const *self, const uint8_t *const *other) {
    g_raw_compares++;
    const uint8_t *const a = *self, *const b = *other;
    const int8_t *const ta = *(const int8_t *const *)(a + 0xc), *const tb = *(const int8_t *const *)(b + 0xc);
    const uint32_t la = *(const uint32_t *)(a + 4), lb = *(const uint32_t *)(b + 4);
    const uint32_t m = lb < la ? lb : la;
    for (uint32_t i = 0; i < m; i++)
        if (const int32_t d = (int32_t)ta[i] - (int32_t)tb[i])
            return d;
    return (int32_t)(la - lb);
}
static const uint8_t string_compare_original[] = {0x8b, 0x01, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x09};
REWRITE("String::Compare", 0xbcf040, string_compare_original, &string_compare, &g_raw_compares);

// ---- Window::AllowedMoves(this, moves) (0xC5F510): the moves a window's flags (+0x1C) allow --------------------------------
// For style 3 (low two bits) without bit 0x40: bit 7 takes off 5, bit 8 0xA, bit 14 1, bit 12 4, bit 11 2, bit 13 8.
// Any other window allows none of the low four.
static volatile long g_allowed_moves;
static uint32_t __thiscall window_allowed_moves(const uint8_t *self, uint32_t moves) {
    g_allowed_moves++;
    const uint32_t f = *(const uint32_t *)(self + 0x1c);
    if ((f & 3) != 3 || (f & 0x40))
        return moves & 0xfffffff0;
    if (f & 0x80) moves &= ~5u;
    if (f & 0x100) moves &= ~0xau;
    if (f & 0x4000) moves &= ~1u;
    if (f & 0x1000) moves &= ~4u;
    if (f & 0x800) moves &= ~2u;
    if (f & 0x2000) moves &= ~8u;
    return moves;
}
static const uint8_t window_allowed_moves_original[] = {0x8b, 0x49, 0x1c, 0x8b, 0x44, 0x24, 0x04, 0x8b};
REWRITE("Window::AllowedMoves", 0xc5f510, window_allowed_moves_original, &window_allowed_moves, &g_allowed_moves);

// ---- SortedPointers::FindFirst / FindFirst2(this, key, place) (0x9734A0, 0x84F1A0): objects searched by their first value --
// FindFirst: pointers at +0xC (count +0x10), signed; FindFirst2: at +0 (count +4), unsigned. Gives (in the low byte) 1 and
// the place when found, else 0 and where it would go.
template <class T> static uint8_t find_first(const T *const *list, int32_t count, T key, int32_t *place) {
    int32_t lo = 0, hi = count - 1;
    while (lo <= hi) {
        const int32_t mid = (hi - lo) / 2 + lo;
        const T v = *list[mid];
        if (key > v)
            lo = mid + 1;
        else if (key == v) {
            *(volatile int32_t *)place = mid;
            return 1;
        } else
            hi = mid - 1;
    }
    *(volatile int32_t *)place = lo;
    return 0;
}
static uint8_t __thiscall sorted_pointers_find_first(const uint8_t *self, int32_t key, int32_t *place) {
    g_pointer_finds++;
    return find_first<int32_t>(*(const int32_t *const *const *)(self + 0xc), *(const int32_t *)(self + 0x10), key, place);
}
static uint8_t __thiscall sorted_pointers_find_first2(const uint8_t *self, uint32_t key, int32_t *place) {
    g_pointer_finds++;
    return find_first<uint32_t>(*(const uint32_t *const *const *)self, *(const int32_t *)(self + 4), key, place);
}
static const uint8_t sorted_pointers_find_first_original[] = {0x53, 0x55, 0x56, 0x57, 0x8b, 0x79, 0x10, 0x33};
static const uint8_t sorted_pointers_find_first2_original[] = {0x53, 0x55, 0x56, 0x57, 0x8b, 0x79, 0x04, 0x33};
REWRITE("SortedPointers::FindFirst", 0x9734a0, sorted_pointers_find_first_original, &sorted_pointers_find_first, &g_pointer_finds);
REWRITE("SortedPointers::FindFirst2", 0x84f1a0, sorted_pointers_find_first2_original, &sorted_pointers_find_first2, &g_pointer_finds);

// ---- RefList::Find(this, a, b) (0xBBCCE0): the entry of a circular list with two values, counted one more use ---------------
// Nodes (next +0, entry +8) from *this round to the sentinel at +0xC; an entry with +4 = b and +8 = a has its count (+0)
// raised and gives its value (+0xC); 0 when none.
static volatile long g_ref_finds;
static uint32_t __thiscall ref_list_find(const uint8_t *self, uint32_t a, uint32_t b) {
    g_ref_finds++;
    const uint8_t *const sentinel = self + 0xc;
    const uint32_t *node = *(const uint32_t *const *)self;
    if ((const uint8_t *)node == sentinel)
        return 0;
    for (;;) {
        volatile uint32_t *const e = (volatile uint32_t *)(uintptr_t)node[2];
        if (e[1] == b && e[2] == a) {
            e[0] = e[0] + 1;
            return e[3];
        }
        if ((const uint8_t *)node == sentinel)
            return 0;
        node = (const uint32_t *)(uintptr_t)*node;
        if ((const uint8_t *)node == sentinel)
            return 0;
    }
}
static const uint8_t ref_list_find_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x11, 0x56, 0x8d, 0x71};
REWRITE("RefList::Find", 0xbbcce0, ref_list_find_original, &ref_list_find, &g_ref_finds);

// ---- Drag::Offset(this, out) (0x682910): the drag's offset (+0x25C) while dragging (byte +0x258), otherwise (0, 0) --------
static volatile long g_drag_offsets;
static void __thiscall drag_offset(const uint8_t *self, uint32_t *out) {
    g_drag_offsets++;
    const uint32_t *const o = (const uint32_t *)(self + 0x25c);
    ((volatile uint32_t *)out)[0] = self[0x258] ? o[0] : 0;
    ((volatile uint32_t *)out)[1] = self[0x258] ? o[1] : 0;
}
static const uint8_t drag_offset_original[] = {0x8a, 0x81, 0x58, 0x02, 0x00, 0x00, 0x83, 0xec};
REWRITE("Drag::Offset", 0x682910, drag_offset_original, &drag_offset, &g_drag_offsets);

// ---- KeyCombination::Set(this, k1, k2, k3, k4) (0xBD1940): a key combination from up to four keys -------------------------
// The non-zero keys kept in order, their count at +0x10.
static volatile long g_key_combinations;
static void __thiscall key_combination_set(uint32_t *self, uint32_t k1, uint32_t k2, uint32_t k3, uint32_t k4) {
    g_key_combinations++;
    volatile uint32_t *const t = self;
    t[4] = 0;
    const uint32_t keys[4] = {k1, k2, k3, k4};
    for (const uint32_t k : keys)
        if (k) {
            t[t[4]] = k;
            t[4] = t[4] + 1;
        }
}
static const uint8_t key_combination_set_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x85, 0xc9};
REWRITE("KeyCombination::Set", 0xbd1940, key_combination_set_original, &key_combination_set, &g_key_combinations);

// ---- PhysicsWorld::RemoveBody(this, body) (0xA6BF80): a body taken out of the world's list (+0x60, count +0x68) -------------
static void __thiscall physics_world_remove_body(uint8_t *self, uint32_t body) {
    g_link_removals++;
    remove_from(self, 0x60, body, 8);
}
static const uint8_t physics_world_remove_body_original[] = {0x56, 0x8b, 0x71, 0x68, 0x33, 0xc0, 0x85, 0xf6};
REWRITE("PhysicsWorld::RemoveBody", 0xa6bf80, physics_world_remove_body_original, &physics_world_remove_body, &g_link_removals);

// ---- Ride::Count585AA0(this, n) (0x585AA0): counts (+0x90, +0x74) raised, n added to a total (+0x1F4) ----------------------
// and a running average (+0x1F8) becomes (average 19 + n) / 20 (signed).
static volatile long g_ride_counts;
static void __thiscall ride_count_585aa0(uint8_t *self, int32_t n) {
    g_ride_counts++;
    volatile int32_t *const t = (volatile int32_t *)self;
    const int32_t c = t[0x90 / 4] + 1, d = t[0x74 / 4] + 1;
    t[0x90 / 4] = c;
    const int32_t total = t[0x1f4 / 4];
    t[0x74 / 4] = d;
    t[0x1f4 / 4] = (int32_t)((uint32_t)total + (uint32_t)n);
    t[0x1f8 / 4] = (int32_t)((uint32_t)t[0x1f8 / 4] * 19u + (uint32_t)n) / 20;
}
static const uint8_t ride_count_585aa0_original[] = {0x8b, 0x91, 0x90, 0x00, 0x00, 0x00, 0x8b, 0x41};
REWRITE("Ride::Count585AA0", 0x585aa0, ride_count_585aa0_original, &ride_count_585aa0, &g_ride_counts);

// ---- Cell::StepInside(this, x, y, direction) (0x71BF70): a byte cell position moved one step, still inside the grid? -------
// Direction 0: x - 1, 1: x + 1, 2: y - 1, 3: y + 1 (bytes); 1 when x is at most +0x5C and y at most +0x5D, else 0; any
// other direction 0 and no move.
static volatile long g_cell_steps;
static uint32_t __thiscall cell_step_inside(const uint8_t *self, uint8_t *x, uint8_t *y, uint32_t direction) {
    g_cell_steps++;
    volatile uint8_t *const px = x, *const py = y;
    switch ((uint8_t)direction) {
    case 0: *px = (uint8_t)(*px - 1); break;
    case 1: *px = (uint8_t)(*px + 1); break;
    case 2: *py = (uint8_t)(*py - 1); break;
    case 3: *py = (uint8_t)(*py + 1); break;
    default: return 0;
    }
    return *px <= self[0x5c] && *py <= self[0x5d];
}
static const uint8_t cell_step_inside_original[] = {0x0f, 0xb6, 0x44, 0x24, 0x0c, 0x83, 0xf8, 0x03};
REWRITE("Cell::StepInside", 0x71bf70, cell_step_inside_original, &cell_step_inside, &g_cell_steps);

// ---- Tree::UpdateUp(node) (0xBE7A90): a node's values (+8, +0xC) copied up its parents (+4) ----------------------------------
// While the node is its parent's first child (+0x10) the parent takes the values and becomes the node; then the parent
// it is the second child of (+0x1C) gets them at +0x14, and otherwise at +0x20.
static volatile long g_tree_updates;
static void __thiscall tree_update_up(const uint32_t *node) {
    g_tree_updates++;
    for (;;) {
        volatile uint32_t *const p = (volatile uint32_t *)(uintptr_t)node[1];
        if (!p)
            return;
        if (p[4] == (uint32_t)(uintptr_t)node) {
            p[2] = node[2];
            p[3] = node[3];
            node = (const uint32_t *)(uintptr_t)node[1];
            continue;
        }
        if (p[7] == (uint32_t)(uintptr_t)node)
            p[5] = node[2], p[6] = node[3];
        else
            p[8] = node[2], p[9] = node[3];
        return;
    }
}
static const uint8_t tree_update_up_original[] = {0x8b, 0x41, 0x04, 0x85, 0xc0, 0x74, 0x48, 0x39};
REWRITE("Tree::UpdateUp", 0xbe7a90, tree_update_up_original, &tree_update_up, &g_tree_updates);

// ---- Pairs::InsertionSort(this) (0xCB2A00): pairs of 8 bytes (+0xC, count +0) sorted by their second value's low 16 bits ---
// Each pair moves down past the ones whose value is not below its own.
static volatile long g_insertion_sorts;
static void __thiscall pairs_insertion_sort(const uint32_t *self) {
    g_insertion_sorts++;
    for (uint32_t i = 1; i < *(const volatile uint32_t *)self; i++) {
        volatile uint32_t *const l = (volatile uint32_t *)(uintptr_t)self[3];
        const uint32_t a = l[2 * i], b = l[2 * i + 1];
        uint32_t j = i;
        while (j && !((uint16_t)b > (uint16_t)l[2 * j - 1])) {
            l[2 * j] = l[2 * j - 2];
            l[2 * j + 1] = l[2 * j - 1];
            j--;
        }
        l[2 * j] = a;
        l[2 * j + 1] = b;
    }
}
static const uint8_t pairs_insertion_sort_original[] = {0x8b, 0x01, 0xba, 0x01, 0x00, 0x00, 0x00, 0x3b};
REWRITE("Pairs::InsertionSort", 0xcb2a00, pairs_insertion_sort_original, &pairs_insertion_sort, &g_insertion_sorts);

// ---- Lists::ClearMarks(this) (0x9E2E60): the marks of two lists' entries cleared, and the lists emptied --------------------
// 0x30-byte entries at +0x20 (count +0x28; mark +0x2C) and 0x20-byte ones at +0x2C (count +0x34; mark +0x1C); then +0x28,
// +0x34, +0x38 and +0x40 cleared.
static volatile long g_mark_clears;
static void __thiscall lists_clear_marks(uint8_t *self) {
    g_mark_clears++;
    volatile int32_t *const t = (volatile int32_t *)self;
    for (int32_t i = 0; i < t[0x28 / 4]; i++) *(volatile uint32_t *)((uint8_t *)(uintptr_t)t[0x20 / 4] + i * 0x30 + 0x2c) = 0;
    for (int32_t i = 0; i < t[0x34 / 4]; i++) *(volatile uint32_t *)((uint8_t *)(uintptr_t)t[0x2c / 4] + i * 0x20 + 0x1c) = 0;
    t[0x28 / 4] = 0, t[0x34 / 4] = 0, t[0x38 / 4] = 0, t[0x40 / 4] = 0;
}
static const uint8_t lists_clear_marks_original[] = {0x8b, 0x51, 0x28, 0x56, 0x33, 0xf6, 0x33, 0xc0};
REWRITE("Lists::ClearMarks", 0x9e2e60, lists_clear_marks_original, &lists_clear_marks, &g_mark_clears);

// ---- List::MoveIn(this, entry) (0x73EE30): an entry's link (+0xC) taken out of its list and put last in this's (+0x304) ----
// It is taken out only when properly linked (both neighbours set and pointing back).
static volatile long g_move_ins;
static void __thiscall list_move_in(const uint8_t *self, uint8_t *entry) {
    g_move_ins++;
    volatile uint32_t *const link = (volatile uint32_t *)(entry + 0xc);
    if (link) {
        const uint32_t prev = link[1], next = link[0];
        if (prev && next && ((volatile uint32_t *)(uintptr_t)next)[1] == (uint32_t)(uintptr_t)link && *(volatile uint32_t *)(uintptr_t)prev == (uint32_t)(uintptr_t)link) {
            ((volatile uint32_t *)(uintptr_t)next)[1] = prev;
            *(volatile uint32_t *)(uintptr_t)link[1] = link[0];
            link[0] = 0;
            link[1] = 0;
        }
    }
    volatile uint32_t *const head = *(volatile uint32_t *const *)(self + 0x304);
    link[1] = head[1];
    *(volatile uint32_t *)(uintptr_t)head[1] = (uint32_t)(uintptr_t)link;
    link[0] = (uint32_t)(uintptr_t)head;
    head[1] = (uint32_t)(uintptr_t)link;
}
static const uint8_t list_move_in_original[] = {0x8b, 0x44, 0x24, 0x04, 0x83, 0xc0, 0x0c, 0x74};
REWRITE("List::MoveIn", 0x73ee30, list_move_in_original, &list_move_in, &g_move_ins);

// ---- Memory::Move(to, from, n) (0xDFE1A0): bytes copied between places that may overlap; gives to -------------------------
static volatile long g_memory_moves;
static uint8_t *__stdcall memory_move(uint8_t *to, const uint8_t *from, uint32_t n) {
    g_memory_moves++;
    volatile uint8_t *const t = to;
    if (to > from && to < from + n)
        for (uint32_t i = n; i--;) t[i] = from[i];
    else
        for (uint32_t i = 0; i < n; i++) t[i] = from[i];
    return to;
}
static const uint8_t memory_move_original[] = {0x55, 0x8b, 0xec, 0x8b, 0x45, 0x08, 0x8b, 0x4d};
REWRITE("Memory::Move", 0xdfe1a0, memory_move_original, &memory_move, &g_memory_moves);

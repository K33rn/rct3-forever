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
static inline void remove_from(uint8_t *self, int at, uint32_t object) {
    volatile int32_t *const count = (volatile int32_t *)(self + at + 4);
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

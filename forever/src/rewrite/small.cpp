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

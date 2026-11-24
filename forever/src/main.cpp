// forever.dll: RCT3 Forever inside the game. The launcher loads it into RCT3plus.exe before the game's own code
// starts. It checks the exe is the version it knows, turns on the built-in Cheats Menu and puts the
// rewritten game functions in place. Settings: RCT3Forever.ini; log: RCT3Forever.log (where: see data_path).
#include "forever.h"
#include "exe.h"
#include "rewrite/rewrite.h"
#include <stdio.h>
#include <stdarg.h>

// the Cheats Menu (cheatsmenu/src)
void peep_attach(HINSTANCE h);
extern void *peep_real_create9;
extern "C" void *__stdcall Proxy_Direct3DCreate9(unsigned sdk);

// The game's main window is titled RCT3 Forever: top-level windows the game makes with a title get ours instead.
static decltype(&CreateWindowExA) g_real_create_window;
static HWND g_game_window;       // (the game's main window and the thread that made it: watched for a freeze)
static DWORD g_game_thread;
static HWND WINAPI titled_create_window(DWORD ex, LPCSTR cls, LPCSTR title, DWORD style, int x, int y, int w, int h, HWND parent,
                                        HMENU menu, HINSTANCE inst, LPVOID param) {
    const bool main_window = !parent && title && *title && !(style & WS_CHILD);
    if (main_window) title = "RCT3 Forever";
    HWND made = g_real_create_window(ex, cls, title, style, x, y, w, h, parent, menu, inst, param);
    if (main_window && made && !g_game_window) g_game_window = made, g_game_thread = GetCurrentThreadId();
    return made;
}

static HMODULE g_self;
static char g_log[MAX_PATH], g_ini[MAX_PATH];

// Settings and logs go beside forever.dll when an RCT3Forever.ini is there (the zip, or a copy kept anywhere), otherwise
// in %APPDATA%\RCT3 Forever (an installed copy: Program Files can't be written to). RCT3Forever.exe follows the same rule.

static void data_path(char *out, const char *name) {
    char dir[MAX_PATH], ini[MAX_PATH];
    GetModuleFileNameA(g_self, dir, MAX_PATH);
    char *s = strrchr(dir, '\\');
    if (s) *s = 0;
    snprintf(ini, MAX_PATH, "%s\\RCT3Forever.ini", dir);
    if (GetFileAttributesA(ini) == INVALID_FILE_ATTRIBUTES) {
        char app[MAX_PATH];
        DWORD n = GetEnvironmentVariableA("APPDATA", app, MAX_PATH);
        if (n && n < MAX_PATH) {
            snprintf(dir, MAX_PATH, "%s\\RCT3 Forever", app);
            CreateDirectoryA(dir, nullptr);
        }
    }
    snprintf(out, MAX_PATH, "%s\\%s", dir, name);
}

void flog(const char *fmt, ...) {
    FILE *f = fopen(g_log, "a");
    if (!f) return;
    SYSTEMTIME t;
    GetLocalTime(&t);
    fprintf(f, "%02d:%02d:%02d ", t.wHour, t.wMinute, t.wSecond);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fputc('\n', f);
    fclose(f);
}

int setting(const char *section, const char *key, int def) { return GetPrivateProfileIntA(section, key, def, g_ini); }

static bool known_exe() {
    auto base = (uint8_t *)GetModuleHandleA(nullptr);
    auto nt = (IMAGE_NT_HEADERS32 *)(base + ((IMAGE_DOS_HEADER *)base)->e_lfanew);
    if ((uintptr_t)base == exe::IMAGE_BASE && nt->FileHeader.TimeDateStamp == exe::TIMESTAMP &&
        nt->OptionalHeader.SizeOfImage == exe::IMAGE_SIZE)
        return true;
    flog("unknown game version (time stamp 0x%08lx, size 0x%lx): RCT3 Forever stays off",
         nt->FileHeader.TimeDateStamp, nt->OptionalHeader.SizeOfImage);
    return false;
}

static DWORD WINAPI reporter(void *) {
    for (;;) {
        Sleep(30000);
        rewrite::report();
    }
}

// A frozen game: when its main window stops answering for 8 seconds, where its main thread is, noted in
// RCT3Forever.log (the module and offset; in forever.dll, the rewrite nearest below), with the game addresses on its
// stack. Noted again (up to three times) while it stays frozen, so a loop shows as the same places.
static void where_note(const char *what, uintptr_t at, const uint32_t *stack, int words);
static const int kStackWords = 256;
static DWORD WINAPI freeze_watch(void *) {
    int frozen = 0, noted = 0;
    for (;;) {
        Sleep(2000);
        if (!g_game_window || !IsWindow(g_game_window)) continue;
        DWORD_PTR r;
        if (SendMessageTimeoutA(g_game_window, WM_NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 2000, &r)) {
            frozen = 0;
            continue;
        }
        if (++frozen < 4 || noted >= 3 || (frozen - 4) % 5) continue;   // (8 s frozen, then every 10 s)
        HANDLE th = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, FALSE, g_game_thread);
        if (!th) continue;
        // While it is paused the game thread may hold any lock (the heap's, the C library's, the loader's): only
        // kernel calls here (no file, no allocation, no module lookup), and the note is written once it runs again.
        CONTEXT c = {};
        c.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
        static uint32_t stack[kStackWords];
        int words = 0;
        bool got = false;
        if (SuspendThread(th) != (DWORD)-1) {
            if (GetThreadContext(th, &c)) {
                got = true;
                for (; words < kStackWords; words++) {
                    SIZE_T n = 0;
                    if (!ReadProcessMemory(GetCurrentProcess(), (const uint32_t *)(uintptr_t)c.Esp + words, stack + words, 4, &n) || n != 4) break;
                }
            }
            ResumeThread(th);
        }
        CloseHandle(th);
        if (got) where_note("FREEZE", c.Eip, stack, words), noted++;
    }
}

// A crash, noted in RCT3Forever.log before the game goes down: what happened and where (the module and offset; in
// forever.dll, the rewritten function whose code is nearest below), then the game addresses on the stack (the game
// functions that led there, newest first). Only faults are noted, and only the first few (the game may handle one).
static LONG CALLBACK crash_note(EXCEPTION_POINTERS *e) {
    const DWORD code = e->ExceptionRecord->ExceptionCode;
    if (code != EXCEPTION_ACCESS_VIOLATION && code != EXCEPTION_ILLEGAL_INSTRUCTION && code != EXCEPTION_PRIV_INSTRUCTION &&
        code != EXCEPTION_INT_DIVIDE_BY_ZERO && code != EXCEPTION_STACK_OVERFLOW && code != EXCEPTION_BREAKPOINT)
        return EXCEPTION_CONTINUE_SEARCH;
    static volatile long noted;
    if (InterlockedIncrement(&noted) > 3) return EXCEPTION_CONTINUE_SEARCH;
    const uintptr_t at = (uintptr_t)e->ExceptionRecord->ExceptionAddress;
    const uintptr_t what = e->ExceptionRecord->NumberParameters >= 2 ? e->ExceptionRecord->ExceptionInformation[1] : 0;
    char head[64];
    snprintf(head, sizeof head, "CRASH %08lx (address %08x)", code, (unsigned)what);
    static uint32_t stack[kStackWords];
    int words = 0;
    for (; words < kStackWords; words++) {
        SIZE_T n = 0;
        if (!ReadProcessMemory(GetCurrentProcess(), (const uint32_t *)(uintptr_t)e->ContextRecord->Esp + words, stack + words, 4, &n) || n != 4) break;
    }
    where_note(head, at, stack, words);
    // the registers, and the first bytes where each one points (when readable): what the faulting code was working on
    const CONTEXT *c = e->ContextRecord;
    const struct { const char *name; DWORD value; } regs[] = {{"eax", c->Eax}, {"ebx", c->Ebx}, {"ecx", c->Ecx}, {"edx", c->Edx},
                                                             {"esi", c->Esi}, {"edi", c->Edi}, {"ebp", c->Ebp}, {"esp", c->Esp}};
    for (const auto &r : regs) {
        uint32_t words8[8];
        SIZE_T n = 0;
        char line[160];
        int used = snprintf(line, sizeof line, "CRASH  %s %08lx", r.name, r.value);
        if (r.value >= 0x10000 && ReadProcessMemory(GetCurrentProcess(), (const void *)(uintptr_t)r.value, words8, sizeof words8, &n) && n == sizeof words8)
            for (uint32_t w : words8) used += snprintf(line + used, sizeof line - used, " %08x", w);
        flog("%s", line);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

static void where_note(const char *what, uintptr_t at, const uint32_t *stack, int words) {
    HMODULE mod = nullptr;
    char path[MAX_PATH] = "?";
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)at, &mod))
        GetModuleFileNameA(mod, path, MAX_PATH);
    const char *file = strrchr(path, '\\') ? strrchr(path, '\\') + 1 : path;
    const char *near_rewrite = "";
    if (mod == g_self) {
        uintptr_t best = 0;
        for (const rewrite::Entry *r = rewrite::list(); r; r = r->next)
            if ((uintptr_t)r->with <= at && (uintptr_t)r->with > best) best = (uintptr_t)r->with, near_rewrite = r->name;
    }
    flog("%s at %08x (%s+0x%x)%s%s", what, (unsigned)at, file, (unsigned)(at - (uintptr_t)mod),
         *near_rewrite ? ", in or after the rewrite " : "", near_rewrite);
    char chain[400] = "";
    size_t used = 0;
    int found = 0;
    for (int k = 0; k < words && found < 16; k++) {
        const uint32_t v = stack[k];
        if (v >= 0x401000 && v < 0xe10000) {
            used += snprintf(chain + used, sizeof chain - used, " %06x", (unsigned)v);
            found++;
        }
    }
    flog("%.6s game addresses on the stack:%s", what, chain);
}

BOOL WINAPI DllMain(HINSTANCE h, DWORD why, LPVOID) {
    if (why == DLL_PROCESS_DETACH && g_self) {
        // the game's own exit work (atexit) has run by now: report the calls made since the last report
        rewrite::report();
        flog("game closed");
        return TRUE;
    }
    if (why != DLL_PROCESS_ATTACH) return TRUE;
    g_self = h;
    DisableThreadLibraryCalls(h);
    data_path(g_log, "RCT3Forever.log");
    data_path(g_ini, "RCT3Forever.ini");
    DeleteFileA(g_log);
    flog("RCT3 Forever %s in pid %lu", FOREVER_VERSION, GetCurrentProcessId());
    if (!known_exe()) return TRUE;
    AddVectoredExceptionHandler(1, crash_note);

    // [Forever] CheatsMenu (or, in a settings file from before the rename, PeepEditor)
    if (setting("Forever", "CheatsMenu", setting("Forever", "PeepEditor", 1))) {
        peep_attach(h);
        peep_real_create9 = replace_import(exe::IAT_Direct3DCreate9, (void *)Proxy_Direct3DCreate9);
        flog("Cheats Menu on");
    } else {
        flog("Cheats Menu off");
    }

    g_real_create_window = (decltype(&CreateWindowExA))replace_import(exe::IAT_CreateWindowExA, (void *)titled_create_window);
    if (setting("Forever", "Rewrites", 1)) rewrite::install_all();
    else flog("rewritten functions off (Rewrites=0): the game's own code runs");
    if (setting("Forever", "GameFixes", 1)) install_fixes();
    CloseHandle(CreateThread(nullptr, 0, reporter, nullptr, 0, nullptr));
    CloseHandle(CreateThread(nullptr, 0, freeze_watch, nullptr, 0, nullptr));
    return TRUE;
}

// forever.dll: RCT3 Forever inside the game. The launcher loads it into RCT3plus.exe before the game's own code
// starts. It checks the exe is the version it knows, turns on the built-in Cheats Menu and puts the
// rewritten game functions in place. Settings: RCT3Forever.ini beside this file; log: RCT3Forever.log.
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
static HWND WINAPI titled_create_window(DWORD ex, LPCSTR cls, LPCSTR title, DWORD style, int x, int y, int w, int h, HWND parent,
                                        HMENU menu, HINSTANCE inst, LPVOID param) {
    if (!parent && title && *title && !(style & WS_CHILD)) title = "RCT3 Forever";
    return g_real_create_window(ex, cls, title, style, x, y, w, h, parent, menu, inst, param);
}

static HMODULE g_self;
static char g_log[MAX_PATH], g_ini[MAX_PATH];

static void beside_self(char *out, const char *name) {
    GetModuleFileNameA(g_self, out, MAX_PATH);
    char *s = strrchr(out, '\\');
    strcpy(s ? s + 1 : out, name);
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
    const uintptr_t what = e->ExceptionRecord->NumberParameters >= 2 ? e->ExceptionRecord->ExceptionInformation[1] : 0;
    flog("CRASH %08lx at %08x (%s+0x%x)%s%s, address %08x", code, (unsigned)at, file, (unsigned)(at - (uintptr_t)mod),
         *near_rewrite ? ", in or after the rewrite " : "", near_rewrite, (unsigned)what);
    char chain[400] = "";
    size_t used = 0;
    int found = 0;
    const uint32_t *sp = (const uint32_t *)(uintptr_t)e->ContextRecord->Esp;
    for (int k = 0; k < 256 && found < 16; k++) {
        uint32_t v = 0;
        SIZE_T got = 0;
        if (!ReadProcessMemory(GetCurrentProcess(), sp + k, &v, 4, &got) || got != 4) break;
        if (v >= 0x401000 && v < 0xe10000) {
            used += snprintf(chain + used, sizeof chain - used, " %06x", (unsigned)v);
            found++;
        }
    }
    flog("CRASH game addresses on the stack:%s", chain);
    return EXCEPTION_CONTINUE_SEARCH;
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
    beside_self(g_log, "RCT3Forever.log");
    beside_self(g_ini, "RCT3Forever.ini");
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
    return TRUE;
}

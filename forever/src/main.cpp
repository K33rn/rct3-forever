// forever.dll: RCT3 Forever inside the game. The launcher loads it into RCT3plus.exe before the game's own code
// starts. It checks the exe is the version it knows, turns on the built-in Park & Peep Editor and puts the
// rewritten game functions in place. Settings: RCT3Forever.ini beside this file; log: RCT3Forever.log.
#include "forever.h"
#include "exe.h"
#include <stdio.h>
#include <stdarg.h>

// the Park & Peep Editor (peepeditor/src, built with FOREVER)
void peep_attach(HINSTANCE h);
extern void *peep_real_create9;
extern "C" void *__stdcall Proxy_Direct3DCreate9(unsigned sdk);

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

BOOL WINAPI DllMain(HINSTANCE h, DWORD why, LPVOID) {
    if (why != DLL_PROCESS_ATTACH) return TRUE;
    g_self = h;
    DisableThreadLibraryCalls(h);
    beside_self(g_log, "RCT3Forever.log");
    beside_self(g_ini, "RCT3Forever.ini");
    DeleteFileA(g_log);
    flog("RCT3 Forever %s in pid %lu", FOREVER_VERSION, GetCurrentProcessId());
    if (!known_exe()) return TRUE;

    // the launcher sets RCT3FOREVER_NO_PEEP when the standalone Peep Editor is installed in the game folder
    char no_peep[8];
    bool peep = setting("Forever", "PeepEditor", 1) && !GetEnvironmentVariableA("RCT3FOREVER_NO_PEEP", no_peep, 8);
    if (peep) {
        peep_attach(h);
        peep_real_create9 = replace_import(exe::IAT_Direct3DCreate9, (void *)Proxy_Direct3DCreate9);
        flog("Park & Peep Editor on");
    } else {
        flog("Park & Peep Editor off");
    }

    if (setting("Forever", "Rewrites", 1)) rewrite::install_all();
    else flog("rewritten functions off (Rewrites=0): the game's own code runs");
    CloseHandle(CreateThread(nullptr, 0, reporter, nullptr, 0, nullptr));
    return TRUE;
}

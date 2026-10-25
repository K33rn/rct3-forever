// RCT3Forever.exe: starts RollerCoaster Tycoon 3 with RCT3 Forever. It finds the game (RCT3Forever.ini, Steam, or
// asks), checks RCT3plus.exe is the version Forever knows, starts it paused at its entry point, loads forever.dll
// into it and lets it run. Nothing in the game folder is changed.
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <wchar.h>

static const DWORD KNOWN_TIMESTAMP = 0x434e4089;   // forever/src/exe.h
static const wchar_t *TITLE = L"RCT3 Forever";
static wchar_t g_dir[MAX_PATH], g_ini[MAX_PATH];

static void fail(const wchar_t *msg) { MessageBoxW(nullptr, msg, TITLE, MB_ICONERROR); }

static bool exists(const wchar_t *dir, const wchar_t *name) {
    wchar_t p[MAX_PATH];
    swprintf(p, MAX_PATH, L"%ls\\%ls", dir, name);
    return GetFileAttributesW(p) != INVALID_FILE_ATTRIBUTES;
}

// PE header of the exe on disk: time stamp and entry point
static bool read_exe(const wchar_t *path, DWORD *stamp, DWORD *entry) {
    HANDLE f = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    BYTE buf[4096];
    DWORD got = 0;
    ReadFile(f, buf, sizeof buf, &got, nullptr);
    CloseHandle(f);
    if (got < sizeof(IMAGE_DOS_HEADER)) return false;
    LONG at = ((IMAGE_DOS_HEADER *)buf)->e_lfanew;
    if (at <= 0 || at + sizeof(IMAGE_NT_HEADERS32) > got) return false;
    auto nt = (IMAGE_NT_HEADERS32 *)(buf + at);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return false;
    *stamp = nt->FileHeader.TimeDateStamp;
    *entry = nt->OptionalHeader.ImageBase + nt->OptionalHeader.AddressOfEntryPoint;
    return true;
}

// A Steam library's common folder holding RCT3plus.exe one level down
static bool find_in_steam(wchar_t *out) {
    wchar_t steam[MAX_PATH];
    DWORD n = sizeof steam;
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath", RRF_RT_REG_SZ, nullptr, steam, &n))
        return false;
    wchar_t pat[MAX_PATH];
    swprintf(pat, MAX_PATH, L"%ls\\steamapps\\common\\*", steam);
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) return false;
    bool found = false;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd.cFileName[0] == L'.') continue;
        swprintf(out, MAX_PATH, L"%ls\\steamapps\\common\\%ls", steam, fd.cFileName);
        found = exists(out, L"RCT3plus.exe");
    } while (!found && FindNextFileW(h, &fd));
    FindClose(h);
    return found;
}

static bool ask_folder(wchar_t *out) {
    BROWSEINFOW bi = {};
    bi.lpszTitle = L"Pick the RollerCoaster Tycoon 3 folder (the one with RCT3plus.exe)";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    PIDLIST_ABSOLUTE id = SHBrowseForFolderW(&bi);
    if (!id) return false;
    bool ok = SHGetPathFromIDListW(id, out);
    CoTaskMemFree(id);
    return ok;
}

static bool game_folder(wchar_t *out) {
    GetPrivateProfileStringW(L"Game", L"Folder", L"", out, MAX_PATH, g_ini);
    if (out[0] && exists(out, L"RCT3plus.exe")) return true;
    if (!find_in_steam(out)) {
        CoInitialize(nullptr);
        for (;;) {
            if (!ask_folder(out)) return false;
            if (exists(out, L"RCT3plus.exe")) break;
            fail(L"There's no RCT3plus.exe in that folder. Pick the folder RollerCoaster Tycoon 3 is installed in.");
        }
    }
    WritePrivateProfileStringW(L"Game", L"Folder", out, g_ini);
    return true;
}

static bool rw(HANDLE p, DWORD at, const void *data, BYTE *was, SIZE_T n) {
    DWORD old;
    if (!VirtualProtectEx(p, (void *)(uintptr_t)at, n, PAGE_EXECUTE_READWRITE, &old)) return false;
    bool ok = (!was || ReadProcessMemory(p, (void *)(uintptr_t)at, was, n, nullptr)) &&
              WriteProcessMemory(p, (void *)(uintptr_t)at, data, n, nullptr);
    VirtualProtectEx(p, (void *)(uintptr_t)at, n, old, &old);
    FlushInstructionCache(p, (void *)(uintptr_t)at, n);
    return ok;
}

// Wait until the main thread spins on the loop written at the entry point: Windows has loaded the game's DLLs and
// filled its imports, and none of the game's own code has run yet.
static bool wait_at_entry(HANDLE t, DWORD entry) {
    for (int i = 0; i < 1000; i++) {
        Sleep(10);
        SuspendThread(t);
        CONTEXT c = {};
        c.ContextFlags = CONTEXT_CONTROL;
        bool there = GetThreadContext(t, &c) && c.Eip == entry;
        if (there) return true;
        ResumeThread(t);
    }
    return false;
}

static bool load_forever(HANDLE p, const wchar_t *dll) {
    SIZE_T n = (wcslen(dll) + 1) * sizeof(wchar_t);
    void *mem = VirtualAllocEx(p, nullptr, n, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!mem || !WriteProcessMemory(p, mem, dll, n, nullptr)) return false;
    auto load = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    HANDLE t = CreateRemoteThread(p, nullptr, 0, load, mem, 0, nullptr);
    if (!t) return false;
    WaitForSingleObject(t, 30000);
    DWORD module = 0;
    GetExitCodeThread(t, &module);
    CloseHandle(t);
    VirtualFreeEx(p, mem, 0, MEM_RELEASE);
    return module != 0;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    GetModuleFileNameW(nullptr, g_dir, MAX_PATH);
    *wcsrchr(g_dir, L'\\') = 0;
    swprintf(g_ini, MAX_PATH, L"%ls\\RCT3Forever.ini", g_dir);
    wchar_t dll[MAX_PATH];
    swprintf(dll, MAX_PATH, L"%ls\\forever.dll", g_dir);
    if (GetFileAttributesW(dll) == INVALID_FILE_ATTRIBUTES) {
        fail(L"forever.dll is missing. Keep RCT3Forever.exe, forever.dll and RCT3Forever.ini together.");
        return 1;
    }

    wchar_t game[MAX_PATH];
    if (!game_folder(game)) return 1;
    wchar_t exe[MAX_PATH];
    swprintf(exe, MAX_PATH, L"%ls\\RCT3plus.exe", game);
    DWORD stamp, entry;
    if (!read_exe(exe, &stamp, &entry)) {
        fail(L"RCT3plus.exe couldn't be read.");
        return 1;
    }
    if (stamp != KNOWN_TIMESTAMP) {
        fail(L"This RCT3plus.exe isn't the version RCT3 Forever supports: RollerCoaster Tycoon 3 Platinum / Gold "
             L"with Soaked! and Wild!, unpatched. (Complete Edition is a different program.)");
        return 1;
    }

    // the standalone Park & Peep Editor in the game folder would draw a second menu
    if (exists(game, L"d3d9.dll") && exists(game, L"PeepEditor.ini")) {
        int r = MessageBoxW(nullptr,
                            L"The standalone Park & Peep Editor is installed in the game folder. RCT3 Forever has it "
                            L"built in, so with both you'd get two menus.\n\nDisable it in the Mod Loader (or remove "
                            L"d3d9.dll and PeepEditor.ini from the game folder) to use Forever's.\n\nStart now "
                            L"with the standalone one, and Forever's editor off?",
                            TITLE, MB_ICONWARNING | MB_YESNO);
        if (r != IDYES) return 1;
        SetEnvironmentVariableW(L"RCT3FOREVER_NO_PEEP", L"1");
    }

    // the game's command line is passed on: RCT3Forever.exe <args> -> RCT3plus.exe <args>
    wchar_t *args = GetCommandLineW();
    if (*args == L'"') args = wcschr(args + 1, L'"') ? wcschr(args + 1, L'"') + 1 : args + wcslen(args);
    else while (*args && *args != L' ') args++;
    static wchar_t cmd[4096];
    swprintf(cmd, 4096, L"\"%ls\"%ls", exe, args);

    STARTUPINFOW si = {sizeof si};
    PROCESS_INFORMATION pi;
    if (!CreateProcessW(exe, cmd, nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, game, &si, &pi)) {
        fail(L"RCT3plus.exe couldn't be started.");
        return 1;
    }
    const BYTE spin[2] = {0xeb, 0xfe};   // jmp $
    BYTE was[2];
    bool ok = rw(pi.hProcess, entry, spin, was, 2);
    ResumeThread(pi.hThread);
    ok = ok && wait_at_entry(pi.hThread, entry);
    bool loaded = ok && load_forever(pi.hProcess, dll);
    if (ok) rw(pi.hProcess, entry, was, nullptr, 2);
    if (!ok) {
        TerminateProcess(pi.hProcess, 1);
        fail(L"The game didn't start the way RCT3 Forever expects, so it was closed again.");
        return 1;
    }
    ResumeThread(pi.hThread);
    if (!loaded)
        fail(L"forever.dll couldn't be loaded into the game; it is running without RCT3 Forever.");
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return loaded ? 0 : 1;
}

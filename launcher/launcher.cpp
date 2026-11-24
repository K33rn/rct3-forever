// RCT3Forever.exe: starts RollerCoaster Tycoon 3 with RCT3 Forever. It finds the game (RCT3Forever.ini, or asks on the
// first start, opening at the game in the Steam libraries), checks RCT3plus.exe is the version Forever knows, starts it paused at its entry point, loads
// forever.dll into it and lets it run. Nothing in the game folder is changed.
//
// Settings (RCT3Forever.ini) and logs live beside the program when an RCT3Forever.ini is there (the zip, or a copy
// kept anywhere you like), otherwise in %APPDATA%\RCT3 Forever (an installed copy: Program Files can't be written to).
// Hold Shift while starting, or start with --choose-game, to pick the game folder again.
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

static const DWORD KNOWN_TIMESTAMP = 0x434e4089;   // forever/src/exe.h
static const wchar_t *TITLE = L"RCT3 Forever";
static wchar_t g_dir[MAX_PATH], g_data[MAX_PATH], g_ini[MAX_PATH];

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

// The folder settings and logs go in (see the top of this file); the defaults are copied in on the first start
static void data_folder() {
    if (exists(g_dir, L"RCT3Forever.ini")) {
        wcscpy(g_data, g_dir);
    } else {
        wchar_t app[MAX_PATH];
        DWORD n = GetEnvironmentVariableW(L"APPDATA", app, MAX_PATH);
        if (n && n < MAX_PATH) {
            swprintf(g_data, MAX_PATH, L"%ls\\RCT3 Forever", app);
            CreateDirectoryW(g_data, nullptr);
        } else {
            wcscpy(g_data, g_dir);
        }
    }
    swprintf(g_ini, MAX_PATH, L"%ls\\RCT3Forever.ini", g_data);
    if (GetFileAttributesW(g_ini) == INVALID_FILE_ATTRIBUTES) {
        wchar_t defaults[MAX_PATH];
        swprintf(defaults, MAX_PATH, L"%ls\\RCT3Forever.default.ini", g_dir);
        CopyFileW(defaults, g_ini, TRUE);
    }
}

// A Steam library's common folder holding RCT3plus.exe one level down
static bool find_in_library(const wchar_t *library, wchar_t *out) {
    wchar_t pat[MAX_PATH];
    swprintf(pat, MAX_PATH, L"%ls\\steamapps\\common\\*", library);
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) return false;
    bool found = false;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd.cFileName[0] == L'.') continue;
        swprintf(out, MAX_PATH, L"%ls\\steamapps\\common\\%ls", library, fd.cFileName);
        found = exists(out, L"RCT3plus.exe");
    } while (!found && FindNextFileW(h, &fd));
    FindClose(h);
    return found;
}

// Steam's own library, then the others listed in steamapps\libraryfolders.vdf ("path"  "D:\\SteamLibrary")
static bool find_in_steam(wchar_t *out) {
    wchar_t steam[MAX_PATH];
    DWORD n = sizeof steam;
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath", RRF_RT_REG_SZ, nullptr, steam, &n))
        return false;
    for (wchar_t *c = steam; *c; c++)
        if (*c == L'/') *c = L'\\';   // the registry has c:/program files (x86)/steam
    if (find_in_library(steam, out)) return true;
    wchar_t vdf[MAX_PATH];
    swprintf(vdf, MAX_PATH, L"%ls\\steamapps\\libraryfolders.vdf", steam);
    HANDLE f = CreateFileW(vdf, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    static char text[65536];
    DWORD got = 0;
    ReadFile(f, text, sizeof text - 1, &got, nullptr);
    CloseHandle(f);
    text[got] = 0;
    for (char *at = strstr(text, "\"path\""); at; at = strstr(at + 6, "\"path\"")) {
        char *open = strchr(at + 6, '"');
        if (!open) break;
        char path[MAX_PATH];
        int len = 0;
        for (char *c = open + 1; *c && *c != '"' && len < MAX_PATH - 1; c++) {
            if (*c == '\\' && c[1] == '\\') c++;   // the file doubles its backslashes
            path[len++] = *c;
        }
        path[len] = 0;
        wchar_t library[MAX_PATH];
        if (!MultiByteToWideChar(CP_UTF8, 0, path, -1, library, MAX_PATH)) continue;
        if (find_in_library(library, out)) return true;
    }
    return false;
}

// The Explorer folder picker, opened at `start` when given (the older tree dialog where that isn't available)
static bool ask_folder(wchar_t *out, const wchar_t *start) {
    IFileOpenDialog *d = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, (void **)&d))) {
        DWORD options = 0;
        d->GetOptions(&options);
        d->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
        d->SetTitle(L"Select your RollerCoaster Tycoon 3 folder (the one with RCT3plus.exe)");
        d->SetOkButtonLabel(L"Select folder");
        IShellItem *at = nullptr;
        if (start && SUCCEEDED(SHCreateItemFromParsingName(start, nullptr, IID_IShellItem, (void **)&at))) {
            d->SetFolder(at);
            at->Release();
        }
        bool ok = false;
        IShellItem *item = nullptr;
        if (SUCCEEDED(d->Show(nullptr)) && SUCCEEDED(d->GetResult(&item))) {
            PWSTR path = nullptr;
            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                wcsncpy(out, path, MAX_PATH - 1);
                out[MAX_PATH - 1] = 0;
                CoTaskMemFree(path);
                ok = true;
            }
            item->Release();
        }
        d->Release();
        return ok;
    }
    BROWSEINFOW bi = {};
    bi.lpszTitle = L"Pick the RollerCoaster Tycoon 3 folder (the one with RCT3plus.exe)";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    PIDLIST_ABSOLUTE id = SHBrowseForFolderW(&bi);
    if (!id) return false;
    bool ok = SHGetPathFromIDListW(id, out);
    CoTaskMemFree(id);
    return ok;
}

static const wchar_t *WRONG_VERSION =
    L"This RCT3plus.exe isn't the version RCT3 Forever supports: RollerCoaster Tycoon 3 Platinum / Gold with Soaked! "
    L"and Wild!, unpatched. (Complete Edition is a different program.)";

// Whether a folder holds the supported RCT3plus.exe; why not, when it doesn't
static bool good_game(const wchar_t *folder, const wchar_t **why) {
    wchar_t exe[MAX_PATH];
    swprintf(exe, MAX_PATH, L"%ls\\RCT3plus.exe", folder);
    DWORD stamp, entry;
    if (GetFileAttributesW(exe) == INVALID_FILE_ATTRIBUTES)
        *why = L"There's no RCT3plus.exe in that folder. Pick the folder RollerCoaster Tycoon 3 is installed in.";
    else if (!read_exe(exe, &stamp, &entry))
        *why = L"RCT3plus.exe couldn't be read.";
    else if (stamp != KNOWN_TIMESTAMP)
        *why = WRONG_VERSION;
    else
        return true;
    return false;
}

// The game folder: the one saved, else asked for. The first start always asks (explained first), with the picker
// opened at the game in the Steam libraries when it's there; `choose` asks straight away
static bool game_folder(wchar_t *out, bool choose) {
    const wchar_t *why;
    GetPrivateProfileStringW(L"Game", L"Folder", L"", out, MAX_PATH, g_ini);
    if (!choose && out[0] && exists(out, L"RCT3plus.exe")) return true;
    wchar_t found[MAX_PATH] = L"";
    bool steam = find_in_steam(found);
    if (!choose) {
        wchar_t msg[1024];
        if (steam)
            swprintf(msg, 1024,
                     L"Welcome to RCT3 Forever! Please select your RollerCoaster Tycoon 3 Platinum / Gold folder (with "
                     L"Soaked! and Wild!): the one with RCT3plus.exe.\n\nThe game was found in your Steam library, and "
                     L"the picker opens there:\n%ls\n\nPress Select folder to use it, or pick another.",
                     found);
        else
            swprintf(msg, 1024,
                     L"Welcome to RCT3 Forever! Please select your RollerCoaster Tycoon 3 Platinum / Gold folder (with "
                     L"Soaked! and Wild!): the one with RCT3plus.exe.\n\nIt wasn't found in your Steam libraries.");
        if (MessageBoxW(nullptr, msg, TITLE, MB_OKCANCEL | MB_ICONINFORMATION) != IDOK) return false;
    }
    CoInitialize(nullptr);
    for (;;) {
        if (!ask_folder(out, steam ? found : nullptr)) return false;
        if (good_game(out, &why)) break;
        fail(why);
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
    data_folder();
    wchar_t dll[MAX_PATH];
    swprintf(dll, MAX_PATH, L"%ls\\forever.dll", g_dir);
    if (GetFileAttributesW(dll) == INVALID_FILE_ATTRIBUTES) {
        fail(L"forever.dll is missing. Keep RCT3Forever.exe and forever.dll together (reinstall RCT3 Forever if you "
             L"installed it).");
        return 1;
    }

    // the game's command line is passed on: RCT3Forever.exe <args> -> RCT3plus.exe <args>, less --choose-game
    wchar_t *args = GetCommandLineW();
    if (*args == L'"') args = wcschr(args + 1, L'"') ? wcschr(args + 1, L'"') + 1 : args + wcslen(args);
    else while (*args && *args != L' ') args++;
    static wchar_t rest[4096];
    wcsncpy(rest, args, 4095);
    bool choose = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    if (wchar_t *flag = wcsstr(rest, L" --choose-game")) {
        choose = true;
        wmemmove(flag, flag + 14, wcslen(flag + 14) + 1);
    }

    wchar_t game[MAX_PATH];
    if (!game_folder(game, choose)) return 1;
    wchar_t exe[MAX_PATH];
    swprintf(exe, MAX_PATH, L"%ls\\RCT3plus.exe", game);
    DWORD stamp, entry;
    if (!read_exe(exe, &stamp, &entry)) {
        fail(L"RCT3plus.exe couldn't be read. Hold Shift while starting RCT3 Forever to pick another folder.");
        return 1;
    }
    if (stamp != KNOWN_TIMESTAMP) {
        wchar_t msg[512];
        swprintf(msg, 512, L"%ls\n\nHold Shift while starting RCT3 Forever to pick another folder.", WRONG_VERSION);
        fail(msg);
        return 1;
    }
    static wchar_t cmd[4096];
    swprintf(cmd, 4096, L"\"%ls\"%ls", exe, rest);

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

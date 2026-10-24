// Peep Editor for RollerCoaster Tycoon 3: a d3d9.dll placed beside RCT3plus.exe. It forwards Direct3DCreate9 to
// the real system d3d9.dll, hooks the device's Present / Reset to draw an ImGui menu, the window procedure and
// DirectInput to keep the game from seeing clicks and keys meant for the menu.
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <d3d9.h>
#include <dinput.h>
#include <stdio.h>
#include <math.h>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "game.h"
#include "ui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static HMODULE g_self, g_real;
static HWND g_hwnd;
static WNDPROC g_oldproc;
static IDirect3DDevice9 *g_dev;
static bool g_imgui = false;
static DWORD g_loaded_at;
static char g_log_path[MAX_PATH];

void log(const char *fmt, ...) {
    FILE *f = fopen(g_log_path, "a");
    if (!f) return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fputc('\n', f);
    fclose(f);
}

static void *patch(void **vt, int i, void *fn) {
    if (vt[i] == fn) return nullptr;
    DWORD old;
    VirtualProtect(&vt[i], 4, PAGE_EXECUTE_READWRITE, &old);
    void *o = vt[i];
    vt[i] = fn;
    VirtualProtect(&vt[i], 4, old, &old);
    return o;
}

// ---- input ------------------------------------------------------------------------------------------------

static bool block_mouse() { return ui::open() && g_imgui && ImGui::GetIO().WantCaptureMouse; }
static bool block_keys() { return ui::open() && g_imgui && ImGui::GetIO().WantCaptureKeyboard; }

static LRESULT CALLBACK wndproc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (g_imgui && ui::open()) {
        ImGui_ImplWin32_WndProcHandler(h, m, w, l);
        bool mouse = (m >= WM_MOUSEFIRST && m <= WM_MOUSELAST) || m == WM_INPUT;
        bool key = (m >= WM_KEYFIRST && m <= WM_KEYLAST);
        if ((mouse && block_mouse()) || (key && block_keys())) return 0;
    }
    if ((m == WM_KEYDOWN || m == WM_KEYUP) && (int)w == ui::toggle_key()) return 0;
    return CallWindowProcW(g_oldproc, h, m, w, l);
}

static const GUID kSysMouse = {0x6F1D2B60, 0xD5A0, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
static void *g_mouse_dev, *g_key_dev;
typedef HRESULT(WINAPI *GetDeviceState_t)(void *, DWORD, void *);
typedef HRESULT(WINAPI *GetDeviceData_t)(void *, DWORD, void *, DWORD *, DWORD);
typedef HRESULT(WINAPI *DICreateDevice_t)(void *, REFGUID, void **, void *);
static GetDeviceState_t o_state;
static GetDeviceData_t o_data;
static DICreateDevice_t o_dicreate;

static bool blocked(void *dev) { return (dev == g_mouse_dev && block_mouse()) || (dev == g_key_dev && block_keys()); }

static HRESULT WINAPI hk_state(void *dev, DWORD cb, void *data) {
    HRESULT r = o_state(dev, cb, data);
    if (SUCCEEDED(r) && blocked(dev)) memset(data, 0, cb);
    return r;
}

static HRESULT WINAPI hk_data(void *dev, DWORD cb, void *rgdod, DWORD *n, DWORD fl) {
    HRESULT r = o_data(dev, cb, rgdod, n, fl);
    if (SUCCEEDED(r) && n && blocked(dev)) *n = 0;
    return r;
}

static HRESULT WINAPI hk_dicreate(void *di, REFGUID g, void **dev, void *outer) {
    HRESULT r = o_dicreate(di, g, dev, outer);
    if (SUCCEEDED(r) && dev && *dev) {
        if (IsEqualGUID(g, kSysMouse)) g_mouse_dev = *dev; else g_key_dev = *dev;   // the game makes a mouse and a keyboard
        void **vt = *(void ***)*dev;
        void *o = patch(vt, 9, (void *)hk_state);
        if (o) o_state = (GetDeviceState_t)o;
        o = patch(vt, 10, (void *)hk_data);
        if (o) o_data = (GetDeviceData_t)o;
        log("dinput device %p (%s)", *dev, IsEqualGUID(g, kSysMouse) ? "mouse" : "other");
    }
    return r;
}

typedef HRESULT(WINAPI *DI8Create_t)(HINSTANCE, DWORD, REFIID, void **, void *);
static DI8Create_t o_di8;

static HRESULT WINAPI hk_di8(HINSTANCE h, DWORD v, REFIID riid, void **out, void *outer) {
    HRESULT r = o_di8(h, v, riid, out, outer);
    if (SUCCEEDED(r) && out && *out) {
        void *o = patch(*(void ***)*out, 3, (void *)hk_dicreate);
        if (o) o_dicreate = (DICreateDevice_t)o;
    }
    return r;
}

// Point the exe's import of DINPUT8!DirectInput8Create at our wrapper.
static void hook_dinput_iat() {
    uint8_t *base = (uint8_t *)GetModuleHandleA(nullptr);
    auto nt = (IMAGE_NT_HEADERS *)(base + ((IMAGE_DOS_HEADER *)base)->e_lfanew);
    auto &dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    for (auto d = (IMAGE_IMPORT_DESCRIPTOR *)(base + dir.VirtualAddress); d->Name; d++) {
        if (_stricmp((char *)base + d->Name, "dinput8.dll")) continue;
        auto thunk = (IMAGE_THUNK_DATA32 *)(base + d->FirstThunk);
        auto names = (IMAGE_THUNK_DATA32 *)(base + (d->OriginalFirstThunk ? d->OriginalFirstThunk : d->FirstThunk));
        for (; thunk->u1.Function; thunk++, names++) {
            if (names->u1.Ordinal & IMAGE_ORDINAL_FLAG32) continue;
            auto ibn = (IMAGE_IMPORT_BY_NAME *)(base + names->u1.AddressOfData);
            if (strcmp((char *)ibn->Name, "DirectInput8Create")) continue;
            if ((void *)thunk->u1.Function == (void *)hk_di8) return;
            o_di8 = (DI8Create_t)thunk->u1.Function;
            DWORD old;
            VirtualProtect(&thunk->u1.Function, 4, PAGE_READWRITE, &old);
            thunk->u1.Function = (DWORD)(uintptr_t)hk_di8;
            VirtualProtect(&thunk->u1.Function, 4, old, &old);
            log("DirectInput8Create hooked");
        }
    }
}

// ---- d3d9 -------------------------------------------------------------------------------------------------

typedef HRESULT(WINAPI *Present_t)(IDirect3DDevice9 *, const RECT *, const RECT *, HWND, const RGNDATA *);
typedef HRESULT(WINAPI *Reset_t)(IDirect3DDevice9 *, D3DPRESENT_PARAMETERS *);
typedef HRESULT(WINAPI *CreateDevice_t)(IDirect3D9 *, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *, IDirect3DDevice9 **);
static Present_t o_present;
static Reset_t o_reset;
static CreateDevice_t o_create;

static void draw(IDirect3DDevice9 *dev) {
    if (!g_imgui) {
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        ImGui_ImplWin32_Init(g_hwnd);
        ImGui_ImplDX9_Init(dev);
        ui::init();
        g_imgui = true;
        log("imgui ready, window %p", g_hwnd);
    }
    ui::poll_toggle(g_hwnd);
    ui::apply_locks();
    bool toast = GetTickCount() - g_loaded_at < 8000;
    if (!ui::open() && !toast && !ui::overlay_active()) return;
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::GetIO().MouseDrawCursor = ui::open();
    ImGui::NewFrame();
    ui::frame(toast);
    ImGui::EndFrame();
    if (SUCCEEDED(dev->BeginScene())) {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        dev->EndScene();
    }
}

// ---- the game's camera --------------------------------------------------------------------------------------
// The game draws the world with vertex shaders. Once a frame it uploads c0..c3 = projection (rows: 2.414 = cot of
// half the 45 degree field of view, ...; c3 = 0 0 1 0) and c4..c6 = the view matrix rows (rotation + translation).
// Other uploads carry a view without translation (camera-relative passes) or a mirrored one (water reflections):
// the first full, unmirrored one of the frame is kept.
typedef HRESULT(WINAPI *SetVSConstF_t)(IDirect3DDevice9 *, UINT, const float *, UINT);
static SetVSConstF_t o_setvsc;
static float g_cam[28], g_cam_next[28];   // 16 projection + 12 view
static bool g_cam_ok = false, g_cam_have_next = false;
static DWORD g_cam_at = 0;

static HRESULT WINAPI hk_setvsc(IDirect3DDevice9 *d, UINT start, const float *c, UINT n) {
    if (start == 0 && n >= 7 && !g_cam_have_next && c[0] > 0.1f && fabsf(c[12]) < 1e-4f && fabsf(c[13]) < 1e-4f &&
        fabsf(c[14] - 1) < 1e-4f && fabsf(c[15]) < 1e-4f) {
        const float *v = c + 16;
        float t2 = v[3] * v[3] + v[7] * v[7] + v[11] * v[11];
        float det = v[0] * (v[5] * v[10] - v[6] * v[9]) - v[1] * (v[4] * v[10] - v[6] * v[8]) + v[2] * (v[4] * v[9] - v[5] * v[8]);
        if (t2 > 1e-6f && det > 0.5f) {
            memcpy(g_cam_next, c, sizeof g_cam_next);
            g_cam_have_next = true;
        }
    }
    return o_setvsc(d, start, c, n);
}

bool world_to_screen(float x, float y, float z, float sw, float sh, float *sx, float *sy) {
    if (!g_cam_ok || GetTickCount() - g_cam_at > 1000) return false;
    const float *P = g_cam, *V = g_cam + 16;
    float vx = V[0] * x + V[1] * y + V[2] * z + V[3];
    float vy = V[4] * x + V[5] * y + V[6] * z + V[7];
    float vz = V[8] * x + V[9] * y + V[10] * z + V[11];
    float cx = P[0] * vx + P[1] * vy + P[2] * vz + P[3];
    float cy = P[4] * vx + P[5] * vy + P[6] * vz + P[7];
    float cw = P[12] * vx + P[13] * vy + P[14] * vz + P[15];
    if (cw < 0.5f) return false;   // behind (or at) the camera
    *sx = (cx / cw * 0.5f + 0.5f) * sw;
    *sy = (0.5f - cy / cw * 0.5f) * sh;
    return true;
}

static void camera_frame_done() {
    if (g_cam_have_next) {
        memcpy(g_cam, g_cam_next, sizeof g_cam);
        g_cam_ok = true;
        g_cam_at = GetTickCount();
    }
    g_cam_have_next = false;
}

static HRESULT WINAPI hk_present(IDirect3DDevice9 *dev, const RECT *a, const RECT *b, HWND w, const RGNDATA *r) {
    if (dev == g_dev) {
        camera_frame_done();
        draw(dev);
    }
    return o_present(dev, a, b, w, r);
}

static HRESULT WINAPI hk_reset(IDirect3DDevice9 *dev, D3DPRESENT_PARAMETERS *pp) {
    if (g_imgui) ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT r = o_reset(dev, pp);
    if (g_imgui && SUCCEEDED(r)) ImGui_ImplDX9_CreateDeviceObjects();
    return r;
}

static HRESULT WINAPI hk_create(IDirect3D9 *d3d, UINT ad, D3DDEVTYPE t, HWND focus, DWORD fl, D3DPRESENT_PARAMETERS *pp,
                                IDirect3DDevice9 **out) {
    HRESULT r = o_create(d3d, ad, t, focus, fl, pp, out);
    log("CreateDevice -> %08lx", (unsigned long)r);
    if (SUCCEEDED(r) && out && *out) {
        g_dev = *out;
        HWND w = pp && pp->hDeviceWindow ? pp->hDeviceWindow : focus;
        if (w && w != g_hwnd) {
            g_hwnd = w;
            g_oldproc = (WNDPROC)SetWindowLongPtrW(w, GWLP_WNDPROC, (LONG_PTR)wndproc);
        }
        void **vt = *(void ***)g_dev;
        void *o = patch(vt, 17, (void *)hk_present);
        if (o) o_present = (Present_t)o;
        o = patch(vt, 16, (void *)hk_reset);
        if (o) o_reset = (Reset_t)o;
        o = patch(vt, 94, (void *)hk_setvsc);
        if (o) o_setvsc = (SetVSConstF_t)o;
        g_loaded_at = GetTickCount();
        hook_dinput_iat();
    }
    return r;
}

typedef IDirect3D9 *(WINAPI *Create9_t)(UINT);

extern "C" IDirect3D9 *WINAPI Proxy_Direct3DCreate9(UINT sdk) {
    static Create9_t real;
    if (!real) {
        char p[MAX_PATH];
        GetSystemDirectoryA(p, MAX_PATH);
        strcat(p, "\\d3d9.dll");
        g_real = LoadLibraryA(p);
        real = g_real ? (Create9_t)GetProcAddress(g_real, "Direct3DCreate9") : nullptr;
        if (!real) { log("could not load %s", p); return nullptr; }
    }
    IDirect3D9 *d = real(sdk);
    if (d) {
        void *o = patch(*(void ***)d, 16, (void *)hk_create);
        if (o) o_create = (CreateDevice_t)o;
    }
    hook_dinput_iat();
    return d;
}

BOOL WINAPI DllMain(HINSTANCE h, DWORD why, LPVOID) {
    if (why == DLL_PROCESS_ATTACH) {
        g_self = h;
        DisableThreadLibraryCalls(h);
        GetModuleFileNameA(h, g_log_path, MAX_PATH);
        char *s = strrchr(g_log_path, '\\');
        strcpy(s ? s + 1 : g_log_path, "PeepEditor.log");
        DeleteFileA(g_log_path);
        log("Peep Editor loaded into pid %lu", GetCurrentProcessId());
        game::start_scanner();
    }
    return TRUE;
}

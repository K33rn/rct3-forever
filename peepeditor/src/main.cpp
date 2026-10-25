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
        // clicks and the wheel over the menu are the menu's; movement still reaches the game so its cursor follows
        bool mouse = m >= WM_LBUTTONDOWN && m <= WM_MOUSELAST;
        bool key = (m >= WM_KEYFIRST && m <= WM_KEYLAST);
        if ((mouse && block_mouse()) || (key && block_keys())) return 0;
    }
    if ((m == WM_KEYDOWN || m == WM_KEYUP) && (int)w == ui::toggle_key()) {
        if (m == WM_KEYDOWN && !(l & 0x40000000) && g_imgui) ui::toggle();   // not an auto-repeat
        return 0;
    }
    return CallWindowProcW(g_oldproc, h, m, w, l);
}

// DirectInput devices of different kinds can have different vtables (seen on a PC with two devices whose second
// one wasn't the same class): the originals are kept per vtable and looked up from the object, never shared.
struct VtHooks { void **vt; void *orig[128]; };
static VtHooks g_vts[16];
static int g_nvts = 0;
static CRITICAL_SECTION g_vt_lock;

static void *original(void *obj, int slot) {
    void **vt = *(void ***)obj;
    for (int i = 0; i < g_nvts; i++)
        if (g_vts[i].vt == vt) return g_vts[i].orig[slot];
    return nullptr;
}

// Patch slot of obj's vtable once, remembering that vtable's own original.
static void hook_slot(void *obj, int slot, void *fn) {
    EnterCriticalSection(&g_vt_lock);
    void **vt = *(void ***)obj;
    int i = 0;
    while (i < g_nvts && g_vts[i].vt != vt) i++;
    if (i == g_nvts && g_nvts < 16) { g_vts[g_nvts].vt = vt; memset(g_vts[g_nvts].orig, 0, sizeof g_vts[0].orig); g_nvts++; }
    if (i < g_nvts && !g_vts[i].orig[slot] && vt[slot] != fn) g_vts[i].orig[slot] = patch(vt, slot, fn);
    LeaveCriticalSection(&g_vt_lock);
}

static void *g_mouse_dev, *g_key_dev;
typedef HRESULT(WINAPI *GetDeviceState_t)(void *, DWORD, void *);
typedef HRESULT(WINAPI *GetDeviceData_t)(void *, DWORD, void *, DWORD *, DWORD);
typedef HRESULT(WINAPI *DICreateDevice_t)(void *, REFGUID, void **, void *);
typedef HRESULT(WINAPI *GetCaps_t)(void *, DIDEVCAPS *);

static bool blocked(void *dev) { return (dev == g_mouse_dev && block_mouse()) || (dev == g_key_dev && block_keys()); }

static HRESULT WINAPI hk_state(void *dev, DWORD cb, void *data) {
    auto o = (GetDeviceState_t)original(dev, 9);
    if (!o) return E_FAIL;
    HRESULT r = o(dev, cb, data);
    if (SUCCEEDED(r) && blocked(dev)) {
        // the mouse keeps moving the game's cursor (lX, lY); only the wheel and buttons are hidden from the game
        if (dev == g_mouse_dev && cb > 8) memset((char *)data + 8, 0, cb - 8);
        else if (dev != g_mouse_dev) memset(data, 0, cb);
    }
    return r;
}

static HRESULT WINAPI hk_data(void *dev, DWORD cb, void *rgdod, DWORD *n, DWORD fl) {
    auto o = (GetDeviceData_t)original(dev, 10);
    if (!o) return E_FAIL;
    HRESULT r = o(dev, cb, rgdod, n, fl);
    if (SUCCEEDED(r) && n && rgdod && blocked(dev)) {
        if (dev == g_mouse_dev && cb >= 8) {   // keep X / Y movement events, drop wheel and button events
            DWORD kept = 0;
            for (DWORD i = 0; i < *n; i++) {
                char *e = (char *)rgdod + i * cb;
                DWORD ofs = *(DWORD *)e;
                if (ofs == DIMOFS_X || ofs == DIMOFS_Y) {
                    if (kept != i) memmove((char *)rgdod + kept * cb, e, cb);
                    kept++;
                }
            }
            *n = kept;
        } else if (dev != g_mouse_dev) {
            *n = 0;
        }
    } else if (SUCCEEDED(r) && n && !rgdod && blocked(dev) && dev != g_mouse_dev) {
        *n = 0;   // a peek for the number of waiting events
    }
    return r;
}

static HRESULT WINAPI hk_dicreate(void *di, REFGUID g, void **dev, void *outer) {
    auto o = (DICreateDevice_t)original(di, 3);
    if (!o) return E_FAIL;
    HRESULT r = o(di, g, dev, outer);
    if (SUCCEEDED(r) && dev && *dev) {
        DIDEVCAPS caps;
        memset(&caps, 0, sizeof caps);
        caps.dwSize = sizeof caps;
        int type = SUCCEEDED(((GetCaps_t)(*(void ***)*dev)[3])(*dev, &caps)) ? (int)(caps.dwDevType & 0xff) : 0;
        if (type == DI8DEVTYPE_MOUSE) g_mouse_dev = *dev;
        else if (type == DI8DEVTYPE_KEYBOARD) g_key_dev = *dev;
        hook_slot(*dev, 9, (void *)hk_state);
        hook_slot(*dev, 10, (void *)hk_data);
        log("dinput device %p type %d vtable %p (%s)", *dev, type, *(void **)*dev,
            type == DI8DEVTYPE_MOUSE ? "mouse" : type == DI8DEVTYPE_KEYBOARD ? "keyboard" : "other");
    }
    return r;
}

typedef HRESULT(WINAPI *DI8Create_t)(HINSTANCE, DWORD, REFIID, void **, void *);
static DI8Create_t o_di8;

static HRESULT WINAPI hk_di8(HINSTANCE h, DWORD v, REFIID riid, void **out, void *outer) {
    HRESULT r = o_di8(h, v, riid, out, outer);
    if (SUCCEEDED(r) && out && *out) hook_slot(*out, 3, (void *)hk_dicreate);
    return r;
}

// Point the exe's import of DINPUT8!DirectInput8Create at our wrapper.
static void hook_dinput_iat() {
    static int enabled = -1;   // PeepEditor.ini [PeepEditor] HookGameInput=0 leaves DirectInput alone
    if (enabled < 0) {
        char ini[MAX_PATH];
        GetModuleFileNameA(nullptr, ini, MAX_PATH);
        char *sl = strrchr(ini, '\\');
        strcpy(sl ? sl + 1 : ini, "PeepEditor.ini");
        enabled = GetPrivateProfileIntA("PeepEditor", "HookGameInput", 1, ini) != 0;
        log("HookGameInput=%d", enabled);
    }
    if (!enabled) return;
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
    game::tick_orders();
    bool toast = GetTickCount() - g_loaded_at < 8000;
    if (!ui::open() && !toast && !ui::overlay_active()) return;
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::GetIO().MouseDrawCursor = false;   // the game's own cursor stays the only one
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
// The game draws the world with vertex shaders and uploads c0..c3 = projection (rows: cot(fov/2) / aspect, cot(fov/2),
// depth row, c3 = 0 0 1 0) and c4..c6 = view rows (rotation + translation). Several passes upload such a pair each
// frame: camera-relative passes (no translation), water reflections (mirrored), and others such as shadows with
// their own view. The screen camera is the one whose projection has the screen's aspect ratio; among those, the
// one used most often in the frame.
typedef HRESULT(WINAPI *SetVSConstF_t)(IDirect3DDevice9 *, UINT, const float *, UINT);
struct CamCand { float m[28]; int uses, src; };   // src 0 = shader constants, 1 = SetTransform
static CamCand g_cands[64];
static int g_ncands = 0;
static float g_cam[28];
static bool g_cam_ok = false;
static DWORD g_cam_at = 0, g_cam_logged = 0;
static float g_aspect = 0;   // backbuffer width / height
static char g_cam_status[160] = "not seen yet";
static void add_candidate(const float *c, int src);
const char *camera_status() { return g_cam_status; }

static HRESULT WINAPI hk_setvsc(IDirect3DDevice9 *d, UINT start, const float *c, UINT n) {
    if (start == 0 && n >= 7 && c[0] > 0.1f && c[5] > 0.1f && fabsf(c[12]) < 1e-4f && fabsf(c[13]) < 1e-4f &&
        fabsf(c[14] - 1) < 1e-4f && fabsf(c[15]) < 1e-4f) {
        const float *v = c + 16;
        float t2 = v[3] * v[3] + v[7] * v[7] + v[11] * v[11];
        float det = v[0] * (v[5] * v[10] - v[6] * v[9]) - v[1] * (v[4] * v[10] - v[6] * v[8]) + v[2] * (v[4] * v[9] - v[5] * v[8]);
        if (t2 > 1e-6f && det > 0.5f) add_candidate(c, 0);
    }
    auto o = (SetVSConstF_t)original(d, 94);
    return o ? o(d, start, c, n) : D3DERR_INVALIDCALL;
}

// Count a camera seen this frame: the same camera = same x / y scale and view (the depth row varies between passes).
static void add_candidate(const float *c, int src) {
    int i = 0;
    while (i < g_ncands && (g_cands[i].m[0] != c[0] || g_cands[i].m[5] != c[5] ||
                            memcmp(g_cands[i].m + 16, c + 16, 12 * sizeof(float))))
        i++;
    if (i < g_ncands) g_cands[i].uses++;
    else if (g_ncands < 64) {
        memcpy(g_cands[g_ncands].m, c, sizeof g_cands[0].m);
        g_cands[g_ncands].src = src;
        g_cands[g_ncands++].uses = 1;
    }
}

// Fixed-function drawing (no vertex shaders): the camera comes through SetTransform as D3D row-vector matrices;
// they are turned into the shader layout (column-vector rows) and counted the same way.
typedef HRESULT(WINAPI *SetTransform_t)(IDirect3DDevice9 *, D3DTRANSFORMSTATETYPE, const D3DMATRIX *);
static float g_ff_proj[16];
static bool g_ff_have_proj = false;

static HRESULT WINAPI hk_settransform(IDirect3DDevice9 *d, D3DTRANSFORMSTATETYPE t, const D3DMATRIX *m) {
    if (m) {
        const float *f = (const float *)m;
        if (t == D3DTS_PROJECTION && fabsf(f[11] - 1) < 1e-4f && fabsf(f[15]) < 1e-4f && f[0] > 0.1f && f[5] > 0.1f) {
            for (int r = 0; r < 4; r++)
                for (int c = 0; c < 4; c++) g_ff_proj[r * 4 + c] = f[c * 4 + r];
            g_ff_have_proj = true;
        } else if (t == D3DTS_VIEW && g_ff_have_proj) {
            float cand[28];
            memcpy(cand, g_ff_proj, sizeof g_ff_proj);
            for (int r = 0; r < 3; r++)
                for (int c = 0; c < 4; c++) cand[16 + r * 4 + c] = f[c * 4 + r];
            const float *v = cand + 16;
            float t2 = v[3] * v[3] + v[7] * v[7] + v[11] * v[11];
            float det = v[0] * (v[5] * v[10] - v[6] * v[9]) - v[1] * (v[4] * v[10] - v[6] * v[8]) + v[2] * (v[4] * v[9] - v[5] * v[8]);
            if (t2 > 1e-6f && det > 0.5f) add_candidate(cand, 1);
        }
    }
    auto o = (SetTransform_t)original(d, 44);
    return o ? o(d, t, m) : D3DERR_INVALIDCALL;
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
    int best = -1;
    for (int i = 0; i < g_ncands; i++) {
        float aspect = g_cands[i].m[5] / g_cands[i].m[0];   // projection: x scale = y scale / aspect
        bool screen = g_aspect <= 0 || fabsf(aspect / g_aspect - 1) < 0.03f;
        if (screen && (best < 0 || g_cands[i].uses > g_cands[best].uses)) best = i;
    }
    bool exact = best >= 0;
    if (best < 0)   // no projection of the screen's shape (fullscreen scaling...): the most used non-square one
        for (int i = 0; i < g_ncands; i++) {
            float aspect = g_cands[i].m[5] / g_cands[i].m[0];
            if (fabsf(aspect - 1) > 0.05f && (best < 0 || g_cands[i].uses > g_cands[best].uses)) best = i;
        }
    if (uintptr_t rd = game::renderer()) {   // the game's own camera, the same with any graphics card or settings
        float v[16], p[16];
        if (game::rd(rd + 0x790, v, sizeof v) && game::rd(rd + 0x7d0, p, sizeof p) && p[0] > 0.1f && p[5] > 0.1f &&
            fabsf(p[11] - 1) < 1e-3f) {
            for (int r = 0; r < 4; r++)
                for (int c = 0; c < 4; c++) g_cam[r * 4 + c] = p[c * 4 + r];   // row-vector -> column-vector rows
            for (int r = 0; r < 3; r++)
                for (int c = 0; c < 4; c++) g_cam[16 + r * 4 + c] = v[c * 4 + r];
            g_cam_ok = true;
            g_cam_at = GetTickCount();
            snprintf(g_cam_status, sizeof g_cam_status, "found (game memory, renderer %08x)", (unsigned)rd);
            g_ncands = 0;
            return;
        }
    }
    if (g_ncands) {
        if (best >= 0)
            snprintf(g_cam_status, sizeof g_cam_status, "found (%s, %s; used %d times this frame, %d cameras seen)",
                     g_cands[best].src ? "fixed-function" : "shaders", exact ? "screen shape" : "closest shape",
                     g_cands[best].uses, g_ncands);
        else
            snprintf(g_cam_status, sizeof g_cam_status, "not found (%d views, none screen-shaped, screen %.3f)", g_ncands,
                     g_aspect);
    } else if (GetTickCount() - g_cam_at > 2000) {
        snprintf(g_cam_status, sizeof g_cam_status, "not found (the game sent no camera)");
    }
    if (best >= 0) {
        memcpy(g_cam, g_cands[best].m, sizeof g_cam);
        g_cam_ok = true;
        g_cam_at = GetTickCount();
    }
    if (ui::overlay_active() && GetTickCount() - g_cam_logged > 15000) {   // what the choice was made from
        g_cam_logged = GetTickCount();
        log("camera: screen aspect %.3f, %d candidates, chose %d", g_aspect, g_ncands, best);
        for (int i = 0; i < g_ncands; i++) {
            const float *m = g_cands[i].m;
            log("  #%d %s uses %d proj %.3f %.3f aspect %.3f  view t %.1f %.1f %.1f", i, g_cands[i].src ? "ff" : "vs",
                g_cands[i].uses, m[0], m[5], m[5] / m[0], m[19], m[23], m[27]);
        }
    }
    g_ncands = 0;
}

static HRESULT WINAPI hk_present(IDirect3DDevice9 *dev, const RECT *a, const RECT *b, HWND w, const RGNDATA *r) {
    if (dev == g_dev) {
        IDirect3DSurface9 *bb = nullptr;   // the back buffer: what the screen camera renders to
        if (SUCCEEDED(dev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bb)) && bb) {
            D3DSURFACE_DESC sd;
            if (SUCCEEDED(bb->GetDesc(&sd)) && sd.Height) g_aspect = (float)sd.Width / sd.Height;
            bb->Release();
        }
        camera_frame_done();
        draw(dev);
    }
    auto o = (Present_t)original(dev, 17);
    return o ? o(dev, a, b, w, r) : D3DERR_INVALIDCALL;
}

static HRESULT WINAPI hk_reset(IDirect3DDevice9 *dev, D3DPRESENT_PARAMETERS *pp) {
    if (g_imgui) ImGui_ImplDX9_InvalidateDeviceObjects();
    auto o = (Reset_t)original(dev, 16);
    HRESULT r = o ? o(dev, pp) : D3DERR_INVALIDCALL;
    if (g_imgui && SUCCEEDED(r)) ImGui_ImplDX9_CreateDeviceObjects();
    return r;
}

static HRESULT WINAPI hk_create(IDirect3D9 *d3d, UINT ad, D3DDEVTYPE t, HWND focus, DWORD fl, D3DPRESENT_PARAMETERS *pp,
                                IDirect3DDevice9 **out) {
    auto o = (CreateDevice_t)original(d3d, 16);
    HRESULT r = o ? o(d3d, ad, t, focus, fl, pp, out) : D3DERR_INVALIDCALL;
    log("CreateDevice -> %08lx", (unsigned long)r);
    if (SUCCEEDED(r) && out && *out) {
        g_dev = *out;
        HWND w = pp && pp->hDeviceWindow ? pp->hDeviceWindow : focus;
        if (w && w != g_hwnd) {
            g_hwnd = w;
            g_oldproc = (WNDPROC)SetWindowLongPtrW(w, GWLP_WNDPROC, (LONG_PTR)wndproc);
        }
        hook_slot(g_dev, 17, (void *)hk_present);
        hook_slot(g_dev, 16, (void *)hk_reset);
        hook_slot(g_dev, 94, (void *)hk_setvsc);
        hook_slot(g_dev, 44, (void *)hk_settransform);
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
        hook_slot(d, 16, (void *)hk_create);
    }
    hook_dinput_iat();
    return d;
}

BOOL WINAPI DllMain(HINSTANCE h, DWORD why, LPVOID) {
    if (why == DLL_PROCESS_ATTACH) {
        g_self = h;
        InitializeCriticalSection(&g_vt_lock);
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

#include "game.h"
#include <algorithm>
#include <mutex>

namespace game {

const GuestClass kGuestClasses[2] = {
    {0xf6c948, 0xf6c888, "Guest"},
    {0xf74fa0, 0xf74ee0, "Guest (special)"},
};

bool rd(uintptr_t a, void *out, size_t n) {
    if (a < 0x10000) return false;
    SIZE_T got = 0;
    return ReadProcessMemory(GetCurrentProcess(), (LPCVOID)a, out, n, &got) && got == n;
}

bool wr(uintptr_t a, const void *in, size_t n) {
    if (a < 0x10000) return false;
    SIZE_T got = 0;
    return WriteProcessMemory(GetCurrentProcess(), (LPVOID)a, in, n, &got) && got == n;
}

static std::mutex g_mx;
static std::vector<Guest> g_list;
static std::vector<std::pair<uintptr_t, size_t>> g_excluded;
static volatile LONG g_scans = 0, g_want = 0;
static double g_ms = 0;

void exclude_region(uintptr_t base, size_t size) {
    std::lock_guard<std::mutex> l(g_mx);
    g_excluded.push_back({base, size});
}

int guest_class(uintptr_t a) {
    uint32_t v0 = get<uint32_t>(a), v4 = get<uint32_t>(a + 0x400);
    for (int i = 0; i < 2; i++)
        if (v0 == kGuestClasses[i].vt && v4 == kGuestClasses[i].vt400) return i;
    return -1;
}

static bool excluded(uintptr_t a) {
    for (auto &r : g_excluded)
        if (a >= r.first && a < r.first + r.second) return true;
    return false;
}

static void scan_once() {
    const size_t CH = 1 << 20;
    static uint8_t *buf = nullptr;
    if (!buf) {
        buf = (uint8_t *)VirtualAlloc(nullptr, CH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        exclude_region((uintptr_t)buf, CH);
    }
    LARGE_INTEGER f, t0, t1;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&t0);
    std::vector<Guest> found;
    std::vector<std::pair<uintptr_t, size_t>> skip;
    {
        std::lock_guard<std::mutex> l(g_mx);
        skip = g_excluded;
    }
    MEMORY_BASIC_INFORMATION mbi;
    uintptr_t a = 0x10000;
    while (a < 0xfffe0000 && VirtualQuery((LPCVOID)a, &mbi, sizeof mbi) == sizeof mbi) {
        uintptr_t base = (uintptr_t)mbi.BaseAddress, end = base + mbi.RegionSize;
        bool ok = mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE && (mbi.Protect & 0xff) == PAGE_READWRITE &&
                  !(mbi.Protect & PAGE_GUARD);
        for (auto &r : skip)
            if (base < r.first + r.second && r.first < end) ok = false;
        for (uintptr_t p = base; ok && p < end; p += CH) {
            size_t n = std::min<size_t>(CH, end - p);
            if (!rd(p, buf, n)) break;
            for (size_t i = 0; i + 4 <= n; i += 4) {
                uint32_t v = *(uint32_t *)(buf + i);
                if (v != kGuestClasses[0].vt && v != kGuestClasses[1].vt) continue;
                int c = guest_class(p + i);
                if (c >= 0) found.push_back({p + i, c});
            }
        }
        if (end <= a) break;
        a = end;
    }
    QueryPerformanceCounter(&t1);
    std::lock_guard<std::mutex> l(g_mx);
    found.erase(std::remove_if(found.begin(), found.end(), [](const Guest &g) { return excluded(g.addr); }), found.end());
    g_list.swap(found);
    g_ms = (t1.QuadPart - t0.QuadPart) * 1000.0 / f.QuadPart;
    InterlockedIncrement(&g_scans);
}

static DWORD WINAPI scanner(LPVOID) {
    for (;;) {
        while (!g_want) Sleep(100);   // the menu asks while it's open (every few seconds)
        g_want = 0;
        scan_once();
    }
}

void start_scanner() { CreateThread(nullptr, 0, scanner, nullptr, 0, nullptr); }
void request_scan() { g_want = 1; }
int scans_done() { return g_scans; }
double last_scan_ms() { return g_ms; }

std::vector<Guest> guests() {
    std::lock_guard<std::mutex> l(g_mx);
    return g_list;
}

std::wstring name(uintptr_t g) {
    uint32_t s = get<uint32_t>(g + NAME);
    int32_t len = get<int32_t>(s + 4, -1);
    uint32_t chars = get<uint32_t>(s + 0xc);
    if (!s || !chars || len <= 0 || len > 200) return L"";
    std::wstring w(len, L'\0');
    if (!rd(chars, &w[0], len * 2)) return L"";
    return w;
}

// The game's operator new (cdecl, size): a longer name needs a new buffer on the game's own heap.
typedef void *(__cdecl *game_new_t)(size_t);

bool set_name(uintptr_t g, const std::wstring &n) {
    uint32_t s = get<uint32_t>(g + NAME);
    if (!s || n.empty() || n.size() > 60 || guest_class(g) < 0) return false;
    int32_t cap = get<int32_t>(s + 8);
    uint32_t chars = get<uint32_t>(s + 0xc);
    if ((int32_t)n.size() >= cap || !chars) {   // the old buffer is left alone (a few bytes), never freed by us
        cap = (int32_t)n.size() + 8;
        chars = (uint32_t)(uintptr_t)((game_new_t)0xbcdfc0)(2 * cap);
        if (!chars) return false;
    }
    if (!wr(chars, n.c_str(), 2 * (n.size() + 1))) return false;
    put<int32_t>(s + 4, (int32_t)n.size());
    put<int32_t>(s + 8, cap);
    put<uint32_t>(s + 0xc, chars);
    return true;
}

uintptr_t deref(uintptr_t field) {
    uint32_t h = get<uint32_t>(field + 4);
    return h ? get<uint32_t>(h + 0xc) : 0;
}

const char *state_name(int s) {
    switch (s) {
    case 0: return "Arrived / choosing";
    case 4: return "Heading for a ride or shop";
    case 7: return "Queueing";
    case 8: return "On a ride";
    case 9: return "Getting off";
    case 10: return "At a shop / gallery / attraction";
    case 11: return "Walking to an entrance";
    case 12: return "Sitting on a bench";
    case 13: return "Waiting outside the park";
    case 14: return "At the park entrance";
    case 16: return "Walking";
    case 19: return "Watching fireworks";
    case 20: return "Watching an entertainer";
    case 256: return "Watching a ride";
    case 258: return "High dive";
    case 259: return "Swimming";
    case 260: return "Jacuzzi";
    }
    return "?";
}

static const uint32_t kThoughtTable = 0x10ede48, kThoughtCount = 256;
static char g_thoughts[kThoughtCount][64];
static bool g_thoughts_read = false;

static void read_thoughts() {
    for (uint32_t i = 0; i < kThoughtCount; i++) {
        char buf[64] = {0};
        uint32_t p = get<uint32_t>(kThoughtTable + 4 * i);
        rd(p, buf, 63);
        const char *s = strncmp(buf, "Thought_", 8) == 0 ? buf + 8 : buf;
        strncpy(g_thoughts[i], s[0] ? s : "?", 63);
    }
    g_thoughts_read = true;
}

const char *thought_name(int id) {
    if (!g_thoughts_read) read_thoughts();
    return id >= 0 && id < (int)kThoughtCount ? g_thoughts[id] : "?";
}

int thought_count() { return kThoughtCount; }

Vec3 position(uintptr_t g) { return get<Vec3>(g + WORLD_MATRIX + 0x30, Vec3{0, 0, 0}); }

Waypoints waypoints(uintptr_t g) {
    Waypoints w;
    w.ptr = get<uint32_t>(g + WAYPOINTS);
    w.count = get<int16_t>(g + WAYPOINTS + 6);
    w.next = get<int16_t>(g + WAYPOINTS + 8);
    w.last = get<int16_t>(g + WAYPOINTS + 0xa);
    for (int i = 0; w.ptr && i < w.count && i < 256; i++) w.pts.push_back(get<Vec3>(w.ptr + 16 * i));
    return w;
}

}  // namespace game

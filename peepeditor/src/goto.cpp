// Sending guests somewhere: the game's attraction list and the steps it runs when a guest picks an attraction.
#include "game.h"
#include <map>
#include "ui.h"

namespace game {

// thiscall functions in RCT3plus.exe; only ever called on the game's thread (the menu runs inside Present)
typedef void *(__thiscall *fn_p0)(void *);
typedef int(__thiscall *fn_i0)(void *);
typedef void(__thiscall *fn_v0)(void *);
typedef void(__thiscall *fn_v1)(void *, int);
typedef void(__thiscall *fn_v2)(void *, int, int);
typedef char(__thiscall *fn_c0)(void *);
typedef char(__thiscall *fn_c1)(void *, void *);

static void *vfn(uintptr_t obj, int off) { return (void *)(uintptr_t)get<uint32_t>(get<uint32_t>(obj) + off); }
static bool code_ptr(void *f) { return (uintptr_t)f >= 0x401000 && (uintptr_t)f < 0xe00000; }

static const uint32_t kAttractionMgr = 0x12f4e4c, kRideId = 0x12f51b4, kStallId = 0x12f5494, kGuestId = 0x130654c;
static const uint32_t kGroupVt = 0xf6d7c0, kAttractionId = 0x12f4c64;

// An engine object's component, found through the component table its vfunc +4 returns; 0 when it has none.
static uintptr_t component(uintptr_t obj, uint32_t id_global) {
    if (!obj || !code_ptr(vfn(obj, 4))) return 0;
    uintptr_t info = (uintptr_t)((fn_p0)vfn(obj, 4))((void *)obj);
    uint32_t id = get<uint32_t>(id_global);
    if (!info || id >= get<uint32_t>(info + 0x18)) return 0;
    uint32_t e = get<uint32_t>(get<uint32_t>(info + 0x14) + 4 * id);
    return e ? obj + get<uint32_t>(e + 0x10) : 0;
}

static std::string utf8_str(uint32_t s) {   // the game's string {refs, len, capacity, wchar_t *}
    int32_t len = get<int32_t>(s + 4, -1);
    uint32_t chars = get<uint32_t>(s + 0xc);
    if (!s || !chars || len <= 0 || len > 120) return "";
    std::wstring w(len, L' ');
    if (!rd(chars, &w[0], len * 2)) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), len, nullptr, 0, nullptr, nullptr);
    std::string o(n, ' ');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), len, &o[0], n, nullptr, nullptr);
    return o;
}

std::string attraction_name(uintptr_t obj) {
    std::string n = utf8_str(get<uint32_t>(obj + 0x68));
    if (n.empty() || n[0] == '#') {
        char b[32];
        snprintf(b, sizeof b, "Attraction %08x", (unsigned)obj);
        n = b;
    }
    return n;
}

std::vector<Attraction> attractions() {
    std::vector<Attraction> out;
    uintptr_t mgr = get<uint32_t>(kAttractionMgr);
    if (!mgr) return out;
    uint32_t node = get<uint32_t>(mgr + 0x64);
    for (int n = 0; node && get<uint32_t>(node) && n < 2000; n++) {
        uintptr_t obj = get<uint32_t>(node + 8);
        if (!obj) break;
        uintptr_t ac = component(obj, kAttractionId);
        Attraction a{obj, attraction_name(obj), component(obj, kRideId) != 0, component(obj, kStallId) != 0, 0,
                     ac && get<int32_t>(ac + 0x70) == 2};
        if (code_ptr(vfn(obj, 0x64))) a.entrances = ((fn_i0)vfn(obj, 0x64))((void *)obj);
        out.push_back(a);
        node = get<uint32_t>(get<uint32_t>(obj + 0xb0));   // the object's own list node -> the next node
    }
    return out;
}

static uintptr_t group_of(uintptr_t guest) {
    uintptr_t g = deref(guest + GUEST_GROUP);
    return g && get<uint32_t>(g) == kGroupVt ? g : 0;
}

uintptr_t group_target(uintptr_t guest) {
    uintptr_t g = group_of(guest);
    return g ? deref(g + 0x98) : 0;
}

int group_size(uintptr_t guest) {
    uintptr_t g = group_of(guest);
    return g ? get<int32_t>(g + 0x48) : 0;
}

// Point an engine reference {vtable, handle, list node {next, prev}} at obj, as the game's inlined code does.
static bool ref_set(uintptr_t ref, uintptr_t obj) {
    uint32_t *node = (uint32_t *)(ref + 8);
    uint32_t next = node[0], prev = node[1], self = (uint32_t)(uintptr_t)node;
    if (!next || !prev || get<uint32_t>(next + 4) != self || get<uint32_t>(prev) != self) return false;
    *(uint32_t *)(uintptr_t)(next + 4) = prev;
    *(uint32_t *)(uintptr_t)prev = next;
    node[0] = node[1] = 0;
    ((fn_v1)0x8055c0)((void *)(uintptr_t)get<uint32_t>(ref + 4), 0);            // drop the old handle
    uint32_t h = (uint32_t)(uintptr_t)((fn_p0)0x805700)((void *)obj);          // the object's handle
    *(uint32_t *)(ref + 4) = h;
    uint32_t list = get<uint32_t>(h + 0x20);
    node[0] = get<uint32_t>(list);
    *(uint32_t *)(uintptr_t)(node[0] + 4) = self;
    node[1] = list;
    *(uint32_t *)(uintptr_t)list = self;
    ((fn_v0)vfn(ref, 0))((void *)ref);                                         // the reference's "changed" hook
    return true;
}

// Group members (guest objects): the group's member array holds references, 0x14 bytes each.
static std::vector<uintptr_t> members(uintptr_t grp) {
    std::vector<uintptr_t> out;
    int n = get<int32_t>(grp + 0x48);
    uint32_t arr = get<uint32_t>(grp + 0x60);
    for (int i = 0; i < n && i < 32; i++) {
        uintptr_t m = deref(arr + 0x14 * i);
        if (m && guest_class(m) >= 0) out.push_back(m);
    }
    return out;
}

// Walking about (or arriving / heading somewhere): safe to redirect. Anything else - queueing, on a ride, at a shop,
// on a bench, watching something - has the game's own bookkeeping (queues, seats) that must not be cut short.
static bool free_to_go(int state) { return state == 16 || state == 0 || state == 4 || state == 11; }

static bool group_free(uintptr_t grp) {
    for (uintptr_t m : members(grp))
        if (!free_to_go(get<int32_t>(m + STATE))) return false;
    return true;
}

bool send_to(uintptr_t guest, uintptr_t attraction, int entrance, std::string *why) {
    auto fail = [&](const char *m) { if (why) *why = m; return false; };
    if (guest_class(guest) < 0) return fail("That guest is gone.");
    uintptr_t grp = group_of(guest);
    if (!grp) return fail("This guest has no group (not walking in the park?).");
    bool known = false;
    for (auto &a : attractions()) known |= a.obj == attraction;
    if (!known) return fail("That attraction no longer exists.");
    int n_ent = code_ptr(vfn(attraction, 0x64)) ? ((fn_i0)vfn(attraction, 0x64))((void *)attraction) : 0;
    if (n_ent <= 0) return fail("That attraction has no entrance yet.");
    uintptr_t ac = component(attraction, kAttractionId);
    if (!ac || get<int32_t>(ac + 0x70) != 2)   // the planner (0x772e23) only routes to open attractions
        return fail("That attraction is closed. Open it in its own window first (new shops start closed).");
    if (entrance < 0 || entrance >= n_ent) entrance = 0;
    uintptr_t leader = deref(grp + 0x4c);
    if (guest_class(leader) < 0) return fail("Couldn't find the group's leader.");
    int st = get<int32_t>(grp + 0x7c);
    if ((st != 16 && st != 11 && st != 4 && st != 0) || !group_free(grp))
        return fail("The group is busy (queueing, riding, shopping, sitting...). Try again when they're walking.");

    // what the game does after a guest picks an attraction at a junction (0x781a18 onwards); the route planner
    // (0x7778e0 -> 0x772de0) handles rides and shops alike
    ((fn_v0)0x77d970)((void *)grp);
    if (uintptr_t c = deref(grp + 0x144)) ((fn_v1)vfn(c, 0))((void *)c, 1);
    for (uintptr_t m : members(grp))
        if (uintptr_t mc = component(m, kGuestId)) ((fn_v2)0x7cd220)((void *)mc, 0, 0);
    put<int32_t>(grp + 0x7c, 11);   // FlockState: heading for an attraction
    put<int32_t>(grp + 0x80, 1);
    if (!ref_set(grp + 0x84, leader) || !ref_set(grp + 0x98, attraction)) return fail("Couldn't set the group's target.");
    put<int32_t>(grp + 0xac, entrance);
    uint8_t type = get<uint8_t>(get<uint32_t>(attraction + 0xa8));
    if ((code_ptr(vfn(attraction, 0x38)) && ((fn_c0)vfn(attraction, 0x38))((void *)attraction)) || type == 0x12)
        put<int32_t>(grp + 0x80, 4);
    if (!((fn_c1)0x7778e0)((void *)grp, (void *)leader)) {
        put<int32_t>(grp + 0x7c, 16);   // back to wandering; the group picks its next step itself
        put<int32_t>(grp + 0x80, 0);
        return fail("The game found no path to that entrance.");
    }
    ((fn_v1)0x77b750)((void *)grp, (int)leader);
    return true;
}

// ---- orders: keep a sent group on course ------------------------------------------------------------------
// While walking, the group's own decisions (an attraction it passes, a need) can replace the target. An order
// re-sends the group (at most once a second, for up to two minutes) until a member is queueing at, riding or
// using the chosen attraction.
struct Order { uintptr_t grp, guest, target; int entrance; DWORD started, last; int resent; std::string status; };
static std::vector<Order> g_orders;

void make_wanting(uintptr_t guest) {   // hungry and thirsty: a reason to buy at a shop
    uintptr_t grp = group_of(guest);
    std::vector<uintptr_t> ms = grp ? members(grp) : std::vector<uintptr_t>{guest};
    for (uintptr_t m : ms) {
        if (get<float>(m + CONSUMPTION) > 0.25f) put<float>(m + CONSUMPTION, 0.25f);
        if (get<float>(m + CONSUMPTION + 4) > 0.25f) put<float>(m + CONSUMPTION + 4, 0.25f);
    }
}

void order(uintptr_t guest, uintptr_t attraction, int entrance) {
    uintptr_t grp = group_of(guest);
    if (!grp) return;
    for (auto &o : g_orders)
        if (o.grp == grp) { o = Order{grp, guest, attraction, entrance, GetTickCount(), GetTickCount(), 0, "on the way"}; return; }
    g_orders.push_back(Order{grp, guest, attraction, entrance, GetTickCount(), GetTickCount(), 0, "on the way"});
}

static std::map<uintptr_t, std::string> g_finished;   // guest -> last order's outcome

void tick_orders() {
    DWORD now = GetTickCount();
    for (size_t i = 0; i < g_orders.size();) {
        Order &o = g_orders[i];
        std::string done;
        if (get<uint32_t>(o.grp) != kGroupVt || guest_class(o.guest) < 0) done = "the group is gone";
        else {
            for (uintptr_t m : members(o.grp)) {
                int st = get<int32_t>(m + STATE);
                if ((st == 7 || st == 8 || st == 9 || st == 10) && deref(m + STATE_TARGET) == o.target && done.empty()) {
                    done = st == 7 ? "arrived - queueing" : st == 8 ? "arrived - on the ride" : "arrived";
                    log("order %08x: member %08x state %d at %s", (unsigned)o.guest, (unsigned)m, st,
                        attraction_name(o.target).c_str());
                }
            }
            if (done.empty() && now - o.started > 120000) done = "gave up after two minutes";
            if (done.empty() && deref(o.grp + 0x98) != o.target && now - o.last > 1000) {
                o.last = now;
                if (group_free(o.grp)) {
                    std::string why;
                    if (send_to(o.guest, o.target, o.entrance, &why)) o.resent++;
                    else done = why;
                } else {
                    done = "they stopped for something else on the way";
                }
            }
        }
        if (!done.empty()) {
            g_finished[o.guest] = done + (o.resent ? " (kept on course " + std::to_string(o.resent) + "x)" : "");
            g_orders.erase(g_orders.begin() + i);
        } else {
            o.status = o.resent ? "on the way (kept on course " + std::to_string(o.resent) + "x)" : "on the way";
            i++;
        }
    }
}

std::string order_status(uintptr_t guest) {
    uintptr_t grp = group_of(guest);
    for (auto &o : g_orders)
        if (o.grp == grp || o.guest == guest) return attraction_name(o.target) + ": " + o.status;
    auto it = g_finished.find(guest);
    return it == g_finished.end() ? "" : it->second;
}

void cancel_order(uintptr_t guest) {
    uintptr_t grp = group_of(guest);
    for (size_t i = 0; i < g_orders.size(); i++)
        if (g_orders[i].grp == grp || g_orders[i].guest == guest) { g_orders.erase(g_orders.begin() + i); return; }
}

}  // namespace game

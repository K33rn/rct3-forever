// Park editor: the park's live statistics, its finances and spawning guests.
// Park object = [0x1306194] (built by 0x708520, ctor 0x707680; fields registered at 0x707b1b), finances = [0x1305dd8]
// (built by 0x6c1da0, ctor 0x6bfce0; fields registered at 0x6bff80). A graphedValue is {float *data; int index;
// int capacity; int count; bool}: the current value is data[index] (0x611af0 pushes a new month by index - 1).
#include "park.h"
#include <math.h>
#include <string.h>
#include <map>
#include <string>
#include "imgui.h"
#include "game.h"
#include "ui.h"

using namespace game;

namespace park {

enum Obj { PARK, FIN };
static uintptr_t obj(Obj o) { return get<uint32_t>(o == PARK ? 0x1306194 : 0x1305dd8); }

// Address of a value: plain fields directly, graphed values through their current slot (0 when unreadable).
static uintptr_t graph_slot(uintptr_t gv) {
    uint32_t data = get<uint32_t>(gv), idx = get<uint32_t>(gv + 4), cap = get<uint32_t>(gv + 8);
    if (!data || cap == 0 || cap > 4096 || idx >= cap) return 0;
    return data + 4 * idx;
}
static uintptr_t addr(Obj o, uint32_t off, bool graphed) {
    uintptr_t b = obj(o);
    if (!b) return 0;
    return graphed ? graph_slot(b + off) : b + off;
}

// ---- locks (re-resolved every frame: the graph slot moves each month and the park changes on load) ----------
struct Lock { Obj o; uint32_t off; bool graphed; int size; uint32_t bits; };
static std::map<uint32_t, Lock> g_locks;   // key: obj << 16 | offset
static uint32_t key(Obj o, uint32_t off) { return ((uint32_t)o << 16) | off; }

void apply_locks() {
    for (auto &kv : g_locks) {
        uintptr_t a = addr(kv.second.o, kv.second.off, kv.second.graphed);
        if (a) wr(a, &kv.second.bits, kv.second.size);
    }
}

static void lock_box(Obj o, uint32_t off, bool graphed, int size) {
    uint32_t k = key(o, off);
    bool on = g_locks.count(k) != 0;
    ImGui::SameLine();
    ImGui::PushID((int)k);
    if (ImGui::Checkbox("lock", &on)) {
        Lock l{o, off, graphed, size, 0};
        uintptr_t a = addr(o, off, graphed);
        if (on && a && rd(a, &l.bits, size)) g_locks[k] = l;
        else g_locks.erase(k);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Keep this value: the editor writes it back every frame");
    ImGui::PopID();
}

static void relock(Obj o, uint32_t off) {
    auto it = g_locks.find(key(o, off));
    if (it == g_locks.end()) return;
    uintptr_t a = addr(o, off, it->second.graphed);
    if (a) rd(a, &it->second.bits, it->second.size);
}

// ---- widgets ------------------------------------------------------------------------------------------------
static void help(const char *tip) {
    if (tip && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tip);
}

// float value, typed in (step = the +/- buttons)
static void f_input(const char *label, Obj o, uint32_t off, bool graphed, float step, const char *fmt,
                    const char *tip = nullptr) {
    uintptr_t a = addr(o, off, graphed);
    float v = a ? get<float>(a) : 0;
    ImGui::SetNextItemWidth(170);
    ImGui::BeginDisabled(!a);
    if (ImGui::InputFloat(label, &v, step, step * 10, fmt) && a) { put(a, v); relock(o, off); }
    ImGui::EndDisabled();
    help(tip);
    lock_box(o, off, graphed, 4);
}

static void f_slider(const char *label, Obj o, uint32_t off, float lo, float hi, const char *fmt,
                     const char *tip = nullptr) {
    uintptr_t a = addr(o, off, false);
    float v = a ? get<float>(a) : 0;
    ImGui::SetNextItemWidth(170);
    ImGui::BeginDisabled(!a);
    if (ImGui::SliderFloat(label, &v, lo, hi, fmt) && a) { put(a, v); relock(o, off); }
    ImGui::EndDisabled();
    help(tip);
    lock_box(o, off, false, 4);
}

// money kept in cents: int32 fields, or graphed floats
static void money(const char *label, Obj o, uint32_t off, bool graphed, const char *tip = nullptr) {
    uintptr_t a = addr(o, off, graphed);
    double cents = !a ? 0 : graphed ? get<float>(a) : get<int32_t>(a);
    float d = (float)(cents / 100.0);
    ImGui::SetNextItemWidth(170);
    ImGui::BeginDisabled(!a);
    if (ImGui::InputFloat(label, &d, 100.0f, 10000.0f, "$%.2f") && a) {
        if (graphed) put<float>(a, roundf(d * 100));
        else put<int32_t>(a, (int32_t)llround((double)d * 100));
        relock(o, off);
    }
    ImGui::EndDisabled();
    help(tip);
    lock_box(o, off, graphed, 4);
}

static void i_input(const char *label, Obj o, uint32_t off, const char *tip = nullptr) {
    uintptr_t a = addr(o, off, false);
    int v = a ? get<int32_t>(a) : 0;
    ImGui::SetNextItemWidth(170);
    ImGui::BeginDisabled(!a);
    if (ImGui::InputInt(label, &v) && a) { put<int32_t>(a, v); relock(o, off); }
    ImGui::EndDisabled();
    help(tip);
    lock_box(o, off, false, 4);
}

// minutes after midnight as hh:mm
static void time_input(const char *label, Obj o, uint32_t off) {
    uintptr_t a = addr(o, off, false);
    int m = a ? get<int32_t>(a) : 0;
    int hm[2] = {m / 60, m % 60};
    ImGui::SetNextItemWidth(170);
    if (ImGui::InputInt2(label, hm) && a) {
        int v = (hm[0] < 0 ? 0 : hm[0] > 24 ? 24 : hm[0]) * 60 + (hm[1] < 0 ? 0 : hm[1] > 59 ? 59 : hm[1]);
        put<int32_t>(a, v > 1440 ? 1440 : v);
        relock(o, off);
    }
    help("hours, minutes");
    lock_box(o, off, false, 4);
}

// ---- spawning -------------------------------------------------------------------------------------------------
// 0x781da0 (fastcall, ecx = group type) makes a guest group at a guest injection point (map edge) and sends it
// walking to an entrance - what the park's update calls (0x43e04a). Types 0-7 follow the demographic weights the
// game keeps at +0x70..0x8c of [0x1306568] (0x771910..0x771ad0); 11 = pick by those weights, as the game does.
// Returns the group, or 0 when no injection point has a route to an entrance.
static const char *kGroupTypes[] = {"Random (as the game picks)", "Lone teenager", "Lone adult", "Teenage couple",
                                    "Adult couple", "Teenage gang", "Adult party", "Family", "Day trip"};
typedef uintptr_t(__fastcall *spawn_t)(int type);

static int spawn(int type, int count, int *guests) {
    int made = 0;
    *guests = 0;
    for (int i = 0; i < count; i++) {
        uintptr_t grp = ((spawn_t)0x781da0)(type);
        if (!grp) break;
        made++;
        *guests += get<int32_t>(grp + 0x48);
    }
    request_scan();
    return made;
}

// ---- the tab --------------------------------------------------------------------------------------------------
static std::string str(uintptr_t s) {
    int32_t len = get<int32_t>(s + 4, -1);
    uint32_t chars = get<uint32_t>(s + 0xc);
    if (!s || !chars || len <= 0 || len > 200) return "";
    std::wstring w(len, L'\0');
    if (!rd(chars, &w[0], len * 2)) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string o(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &o[0], n, nullptr, nullptr);
    return o;
}

void tab() {
    uintptr_t p = obj(PARK), f = obj(FIN);
    if (!p || !f) {
        ImGui::TextWrapped("No park loaded yet. Open a park (sandbox, scenario or a saved game).");
        return;
    }
    std::string nm = str(get<uint32_t>(p + 0x1d8));
    // a park never renamed keeps a text key such as "#ME#"
    ImGui::TextColored(ImVec4(0.15f, 0.85f, 0.95f, 1), "%s", nm.empty() || nm[0] == '#' ? "This park" : nm.c_str());
    ImGui::TextDisabled("The game recalculates most of these as it runs: tick 'lock' to hold a value.");
    ImGui::SameLine();
    if (ImGui::SmallButton("Unlock all")) g_locks.clear();

    if (ImGui::BeginTabBar("parktabs")) {
        if (ImGui::BeginTabItem("Park")) {
            ImGui::BeginChild("p", ImVec2(0, 0));
            ImGui::SeparatorText("Rating");
            f_input("Park rating", PARK, 0x128, true, 10, "%.0f", "0-999; guests come faster above 200, fastest at 850");
            i_input("Rating penalty", PARK, 0x180);
            ImGui::SeparatorText("Guests");
            f_input("Guests in the park", PARK, 0x60, true, 1, "%.0f", "the game's count (spawn guests below to add real ones)");
            f_input("On their way in", PARK, 0x74, true, 1, "%.0f");
            f_input("Adults", PARK, 0x88, true, 1, "%.0f");
            f_input("Teenagers", PARK, 0x9c, true, 1, "%.0f");
            f_input("Children", PARK, 0xb0, true, 1, "%.0f");
            f_input("Guest intake (this month)", PARK, 0x100, true, 1, "%.0f");
            f_input("Ideal guests (attractions)", PARK, 0x150, true, 10, "%.0f", "how many guests the attractions draw; new guests come only while the park has fewer than ideal + publicity");
            f_input("Ideal guests (live)", PARK, 0x18c, false, 10, "%.0f", "ideal guests + publicity: the limit for new arrivals");
            ImGui::SeparatorText("Appeal and publicity");
            f_input("Adult appeal", PARK, 0xc4, true, 1, "%.1f");
            f_input("Teenager appeal", PARK, 0xd8, true, 1, "%.1f");
            f_input("Children appeal", PARK, 0xec, true, 1, "%.1f");
            f_input("Publicity", PARK, 0x114, true, 10, "%.0f", "25-400 counts: x0.25 .. x4 guest arrivals");
            ImGui::SeparatorText("New guests");
            f_slider("Generation rate", PARK, 0x178, 0, 5, "%.2f", "x arrivals (campaign parks 0.75-1)");
            f_slider("Starting happiness", PARK, 0x168, 0, 1, "%.2f");
            f_slider("Starting hunger", PARK, 0x16c, 0, 1, "%.2f", "1 = full");
            f_slider("Starting thirst", PARK, 0x170, 0, 1, "%.2f", "1 = full");
            f_slider("Ride intensity preference", PARK, 0x174, 0, 1, "%.2f");
            ImGui::SeparatorText("Opening hours");
            time_input("Opens", PARK, 0x1a8);
            time_input("Closes", PARK, 0x1ac);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Money")) {
            ImGui::BeginChild("m", ImVec2(0, 0));
            ImGui::SeparatorText("Bank");
            money("Cash", FIN, 0x58, false);
            money("Loan", FIN, 0x5c, false);
            money("Max loan", FIN, 0x60, false);
            f_input("Interest rate", FIN, 0x64, false, 0.5f, "%.2f");
            money("Park entrance fee", FIN, 0x114, false);
            money("Fee guests accept", PARK, 0x13c, true, "the assessed entrance fee: above it arrivals drop to 1/4, above twice it to 1/16");
            ImGui::SeparatorText("Values");
            money("Park value", FIN, 0x9c, true);
            money("Company value", FIN, 0xc4, true);
            money("Cash less loan", FIN, 0x88, true);
            money("Share price", FIN, 0xd8, true);
            money("Weekly profit", FIN, 0xb0, true);
            money("Income", FIN, 0xec, true);
            money("Expenditure", FIN, 0x100, true);
            ImGui::SeparatorText("Prices and guests' money");
            money("Land price (buy)", FIN, 0x6c, false);
            money("Land price (sell)", FIN, 0x70, false);
            money("Construction rights (buy)", FIN, 0x74, false);
            money("Construction rights (sell)", FIN, 0x78, false);
            money("New guests' cash", FIN, 0x7c, false);
            money("New guests' ATM money", FIN, 0x80, false);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Spawn guests")) {
            static int type = 0, count = 1;
            static std::string result;
            ImGui::TextWrapped("New guests appear where the game's own arrive (the roads and car parks at the map's "
                               "edge) and walk to the park entrance, paying the entrance fee as they go in.");
            ImGui::SetNextItemWidth(260);
            ImGui::Combo("Group", &type, kGroupTypes, IM_ARRAYSIZE(kGroupTypes));
            ImGui::SetNextItemWidth(260);
            ImGui::SliderInt("How many groups", &count, 1, 50);
            if (ImGui::Button("Spawn")) {
                int guests = 0, made = spawn(type == 0 ? 11 : type - 1, count, &guests);
                char b[160];
                if (made) snprintf(b, sizeof b, "Spawned %d group%s (%d guests).", made, made == 1 ? "" : "s", guests);
                else snprintf(b, sizeof b, "Nothing spawned: no road from the map edge reaches a park entrance.");
                result = b;
                log("spawn type %d x%d -> %d groups, %d guests", type, count, made, guests);
            }
            ImGui::SameLine();
            if (ImGui::Button("Spawn 10 random groups")) {
                int guests = 0, made = spawn(11, 10, &guests);
                char b[160];
                snprintf(b, sizeof b, made ? "Spawned %d groups (%d guests)." : "Nothing spawned: no road from the map edge reaches a park entrance.", made, guests);
                result = b;
                log("spawn random x10 -> %d groups, %d guests", made, guests);
            }
            if (!result.empty()) ImGui::TextUnformatted(result.c_str());
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

}  // namespace park

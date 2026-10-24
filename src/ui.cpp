#include "ui.h"
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "imgui.h"
#include "game.h"

using namespace game;

namespace ui {

static bool g_open = false;
static int g_key = VK_F9;
static char g_key_name[16] = "F9";
static uintptr_t g_sel = 0;
static bool g_sel_changed = false;
static char g_filter[64] = "";
static bool g_in_park_only = true;
static DWORD g_last_scan_req = 0;

// ---- settings ---------------------------------------------------------------------------------------------

static int parse_key(const char *s) {
    if ((s[0] == 'F' || s[0] == 'f') && atoi(s + 1) >= 1 && atoi(s + 1) <= 12) return VK_F1 + atoi(s + 1) - 1;
    struct { const char *n; int vk; } named[] = {{"Insert", VK_INSERT}, {"Home", VK_HOME}, {"End", VK_END},
                                                 {"Delete", VK_DELETE}, {"PageUp", VK_PRIOR}, {"PageDown", VK_NEXT},
                                                 {"Pause", VK_PAUSE}, {"ScrollLock", VK_SCROLL}};
    for (auto &k : named)
        if (!_stricmp(s, k.n)) return k.vk;
    return 0;
}

void init() {
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    char *s = strrchr(path, '\\');
    strcpy(s ? s + 1 : path, "PeepEditor.ini");
    char key[32];
    GetPrivateProfileStringA("PeepEditor", "ToggleKey", "F9", key, sizeof key, path);
    if (int vk = parse_key(key)) {
        g_key = vk;
        strncpy(g_key_name, key, sizeof g_key_name - 1);
    }
    float scale = GetPrivateProfileIntA("PeepEditor", "UiScalePercent", 110, path) / 100.0f;
    ImGuiStyle &st = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    st.WindowRounding = 6;
    st.FrameRounding = 4;
    st.GrabRounding = 4;
    st.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.55f, 0.10f, 0.55f, 1);
    st.Colors[ImGuiCol_Header] = ImVec4(0.45f, 0.15f, 0.55f, 0.6f);
    st.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.55f, 0.2f, 0.7f, 0.8f);
    st.Colors[ImGuiCol_Tab] = ImVec4(0.30f, 0.12f, 0.40f, 1);
    st.Colors[ImGuiCol_TabSelected] = ImVec4(0.60f, 0.20f, 0.70f, 1);
    st.Colors[ImGuiCol_SliderGrab] = ImVec4(0.15f, 0.85f, 0.95f, 1);
    st.Colors[ImGuiCol_CheckMark] = ImVec4(0.15f, 0.85f, 0.95f, 1);
    st.ScaleAllSizes(scale);
    ImGui::GetIO().FontGlobalScale = scale;
    log("toggle key %s (vk %d), ini %s", g_key_name, g_key, path);
}

bool open() { return g_open; }
int toggle_key() { return g_key; }

static DWORD g_toggled_at = 0;

void toggle() {
    if (GetTickCount() - g_toggled_at < 250) return;   // the same press seen by both the poll and the window message
    g_toggled_at = GetTickCount();
    g_open = !g_open;
    if (g_open) request_scan();
}

void poll_toggle(HWND w) {
    static bool was = false;
    bool down = (GetAsyncKeyState(g_key) & 0x8000) != 0 && GetForegroundWindow() == w;
    if (down && !was) toggle();
    was = down;
}

// ---- locks ------------------------------------------------------------------------------------------------

struct Lock { uintptr_t guest; uint32_t off; int size; uint32_t bits; };
static std::map<uint64_t, Lock> g_locks;   // key: guest << 16 | offset

static uint64_t lock_key(uintptr_t g, uint32_t off) { return ((uint64_t)g << 16) | off; }

void apply_locks() {
    for (auto it = g_locks.begin(); it != g_locks.end();) {
        Lock &l = it->second;
        if (guest_class(l.guest) < 0) { it = g_locks.erase(it); continue; }
        wr(l.guest + l.off, &l.bits, l.size);
        ++it;
    }
}

static void lock_box(uintptr_t g, uint32_t off, int size) {
    uint64_t k = lock_key(g, off);
    bool on = g_locks.count(k) != 0;
    ImGui::SameLine();
    ImGui::PushID((int)off);
    if (ImGui::Checkbox("lock", &on)) {
        if (on) {
            Lock l{g, off, size, 0};
            rd(g + off, &l.bits, size);
            g_locks[k] = l;
        } else {
            g_locks.erase(k);
        }
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Keep this value: the editor writes it back every frame");
    ImGui::PopID();
}

static void relock(uintptr_t g, uint32_t off) {   // an edit of a locked value updates the lock
    auto it = g_locks.find(lock_key(g, off));
    if (it != g_locks.end()) rd(g + off, &it->second.bits, it->second.size);
}

// ---- field widgets ----------------------------------------------------------------------------------------

static void f_slider(const char *label, uintptr_t g, uint32_t off, float lo, float hi, const char *fmt = "%.2f",
                     bool lockable = true) {
    float v = get<float>(g + off);
    ImGui::SetNextItemWidth(220);
    if (ImGui::SliderFloat(label, &v, lo, hi, fmt)) { put(g + off, v); relock(g, off); }
    if (lockable) lock_box(g, off, 4);
}

static void i_input(const char *label, uintptr_t g, uint32_t off, bool lockable = false) {
    int v = get<int32_t>(g + off);
    ImGui::SetNextItemWidth(160);
    if (ImGui::InputInt(label, &v)) { put<int32_t>(g + off, v); relock(g, off); }
    if (lockable) lock_box(g, off, 4);
}

static void money(const char *label, uintptr_t g, uint32_t off, bool lockable = false) {
    int cents = get<int32_t>(g + off);
    float d = cents / 100.0f;
    ImGui::SetNextItemWidth(160);
    if (ImGui::InputFloat(label, &d, 1.0f, 10.0f, "$%.2f")) {
        put<int32_t>(g + off, (int32_t)lroundf(d * 100));
        relock(g, off);
    }
    if (lockable) lock_box(g, off, 4);
}

static void b_check(const char *label, uintptr_t g, uint32_t off) {
    bool v = get<uint8_t>(g + off) != 0;
    if (ImGui::Checkbox(label, &v)) { put<uint8_t>(g + off, v ? 1 : 0); relock(g, off); }
    lock_box(g, off, 1);
}

static std::string utf8(const std::wstring &w) {
    if (w.empty()) return std::string();
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], n, nullptr, nullptr);
    return s;
}

static std::wstring wide(const char *s) {
    int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    std::wstring w(n > 0 ? n - 1 : 0, L'\0');
    if (n > 1) MultiByteToWideChar(CP_UTF8, 0, s, -1, &w[0], n);
    return w;
}

static std::string display_name(uintptr_t g, int index) {
    std::string n = utf8(name(g));
    if (n.empty()) {
        char b[32];
        snprintf(b, sizeof b, "Guest #%d", index + 1);
        n = b;
    }
    return n;
}

// ---- tracking guests on screen ---------------------------------------------------------------------------

struct Track { uintptr_t g; ImU32 col; std::vector<Vec3> trail; };
static std::vector<Track> g_tracks;
static bool g_show_route = true, g_show_trail = true, g_show_labels = true;
static const ImU32 kTrackColours[] = {IM_COL32(40, 230, 250, 255), IM_COL32(255, 80, 200, 255), IM_COL32(255, 220, 40, 255),
                                      IM_COL32(120, 255, 90, 255), IM_COL32(255, 140, 40, 255), IM_COL32(170, 120, 255, 255),
                                      IM_COL32(255, 255, 255, 255), IM_COL32(255, 70, 70, 255)};
static const int kMaxTracks = 8, kTrailPoints = 600;

static Track *find_track(uintptr_t g) {
    for (auto &t : g_tracks)
        if (t.g == g) return &t;
    return nullptr;
}

static void toggle_track(uintptr_t g) {
    for (size_t i = 0; i < g_tracks.size(); i++)
        if (g_tracks[i].g == g) { g_tracks.erase(g_tracks.begin() + i); return; }
    if ((int)g_tracks.size() >= kMaxTracks) g_tracks.erase(g_tracks.begin());
    ImU32 col = kTrackColours[0];
    for (ImU32 c : kTrackColours) {   // first colour not in use
        bool used = false;
        for (auto &t : g_tracks) used |= t.col == c;
        if (!used) { col = c; break; }
    }
    g_tracks.push_back(Track{g, col, {position(g)}});
}

bool overlay_active() { return !g_tracks.empty() || (g_open && g_sel); }

static ImU32 with_alpha(ImU32 c, float a) { return (c & 0x00ffffff) | ((ImU32)(a * 255) << 24); }

static void draw_world() {
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    ImVec2 ds = ImGui::GetIO().DisplaySize;
    auto proj = [&](const Vec3 &p, ImVec2 &o) { return world_to_screen(p.x, p.y, p.z, ds.x, ds.y, &o.x, &o.y); };
    auto line = [&](const Vec3 &a, const Vec3 &b, ImU32 c, float w) {
        ImVec2 sa, sb;
        if (proj(a, sa) && proj(b, sb)) dl->AddLine(sa, sb, c, w);
    };
    static Track sel{0, IM_COL32(255, 255, 255, 255), {}};
    bool show_sel = g_open && g_sel && guest_class(g_sel) >= 0 && !find_track(g_sel);
    if (show_sel && sel.g != g_sel) sel = Track{g_sel, IM_COL32(255, 255, 255, 255), {position(g_sel)}};
    if (show_sel) g_tracks.push_back(sel);   // drawn with the others this frame, taken out again below
    for (size_t i = 0; i < g_tracks.size();) {
        Track &t = g_tracks[i];
        if (guest_class(t.g) < 0) { g_tracks.erase(g_tracks.begin() + i); continue; }   // left the park / gone
        Vec3 p = position(t.g);
        Vec3 &last = t.trail.back();
        float dx = p.x - last.x, dy = p.y - last.y, dz = p.z - last.z;
        if (dx * dx + dy * dy + dz * dz > 0.25f * 0.25f) {
            t.trail.push_back(p);
            if ((int)t.trail.size() > kTrailPoints) t.trail.erase(t.trail.begin());
        }
        const Vec3 up{0, 0.15f, 0};
        auto lift = [&](Vec3 v) { v.x += up.x; v.y += up.y; v.z += up.z; return v; };
        if (g_show_trail) {   // where they have been, fading out with age
            size_t n = t.trail.size();
            for (size_t k = 1; k < n; k++)
                line(lift(t.trail[k - 1]), lift(t.trail[k]), with_alpha(t.col, 0.15f + 0.6f * k / n), 3.0f);
        }
        if (g_show_route) {   // where they are going: the route's remaining points
            Waypoints w = waypoints(t.g);
            Vec3 prev = lift(p);
            for (int k = std::max<int>(0, w.next); k < (int)w.pts.size(); k++) {
                Vec3 q = lift(w.pts[k]);
                line(prev, q, IM_COL32(0, 0, 0, 160), 5.0f);
                line(prev, q, t.col, 2.5f);
                ImVec2 sq;
                if (proj(q, sq)) dl->AddCircleFilled(sq, 3.5f, t.col);
                prev = q;
            }
            ImVec2 se;
            if (!w.pts.empty() && w.next < (int)w.pts.size() && proj(lift(w.pts.back()), se)) {
                Vec3 flag = w.pts.back();
                flag.y += 2.0f;
                ImVec2 sf;
                if (proj(flag, sf)) {
                    dl->AddLine(se, sf, t.col, 2.0f);
                    dl->AddTriangleFilled(sf, ImVec2(sf.x + 12, sf.y + 4), ImVec2(sf.x, sf.y + 9), t.col);
                }
            }
        }
        ImVec2 feet, head;
        Vec3 h = p;
        h.y += 2.3f;
        if (proj(p, feet) && proj(h, head)) {   // the marker: a pin over the guest's head
            dl->AddLine(feet, head, with_alpha(t.col, 0.8f), 2.0f);
            dl->AddCircleFilled(head, 7.0f, IM_COL32(0, 0, 0, 200));
            dl->AddCircleFilled(head, 5.5f, t.col);
            dl->AddCircle(feet, 6.0f, t.col, 0, 2.0f);
            if (g_show_labels) {
                char lab[96];
                snprintf(lab, sizeof lab, "%s - %s", utf8(name(t.g)).c_str(), state_name(get<int32_t>(t.g + STATE)));
                ImVec2 sz = ImGui::CalcTextSize(lab);
                ImVec2 at(head.x - sz.x / 2, head.y - 12 - sz.y);
                dl->AddRectFilled(ImVec2(at.x - 4, at.y - 2), ImVec2(at.x + sz.x + 4, at.y + sz.y + 2), IM_COL32(0, 0, 0, 170), 3.0f);
                dl->AddText(at, t.col, lab);
            }
        }
        i++;
    }
    if (show_sel && !g_tracks.empty() && g_tracks.back().g == sel.g) {   // keep its trail for next frame
        sel = g_tracks.back();
        g_tracks.pop_back();
    }
}

static void track_controls(uintptr_t g) {
    bool on = find_track(g) != nullptr;
    if (ImGui::Button(on ? "Stop tracking on screen" : "Track on screen")) toggle_track(g);
    if (on) {
        ImGui::SameLine();
        ImGui::ColorButton("##col", ImGui::ColorConvertU32ToFloat4(find_track(g)->col), ImGuiColorEditFlags_NoTooltip,
                           ImVec2(18, 18));
    }
    ImGui::SameLine();
    ImGui::Checkbox("route", &g_show_route);
    ImGui::SameLine();
    ImGui::Checkbox("trail", &g_show_trail);
    ImGui::SameLine();
    ImGui::Checkbox("labels", &g_show_labels);
}

// ---- the guest list ---------------------------------------------------------------------------------------

struct Row { uintptr_t g; int index; std::string name; float cash, happy, energy; int state; bool in_park; };
static std::vector<Row> g_rows;
static DWORD g_rows_at = 0;
static int g_rows_scan = -1;

static void refresh_rows(bool force) {
    if (!force && GetTickCount() - g_rows_at < 500 && g_rows_scan == scans_done()) return;
    g_rows_at = GetTickCount();
    g_rows_scan = scans_done();
    std::vector<Guest> gs = guests();
    std::sort(gs.begin(), gs.end(), [](const Guest &a, const Guest &b) { return a.addr < b.addr; });
    g_rows.clear();
    for (size_t i = 0; i < gs.size(); i++) {
        uintptr_t g = gs[i].addr;
        if (guest_class(g) < 0) continue;
        Row r{g, (int)i, display_name(g, (int)i), get<int32_t>(g + CASH) / 100.0f, get<float>(g + CURRENT_HEALTH + 4),
              get<float>(g + CURRENT_HEALTH), get<int32_t>(g + STATE), get<uint8_t>(g + IS_IN_PARK) != 0};
        g_rows.push_back(r);
    }
}

static bool matches(const Row &r) {
    if (g_in_park_only && !r.in_park) return false;
    if (!g_filter[0]) return true;
    std::string a = r.name, b = g_filter;
    std::transform(a.begin(), a.end(), a.begin(), ::tolower);
    std::transform(b.begin(), b.end(), b.begin(), ::tolower);
    return a.find(b) != std::string::npos;
}

static void guest_list() {
    std::vector<const Row *> shown;
    for (auto &r : g_rows)
        if (matches(r)) shown.push_back(&r);
    ImGuiTableFlags fl = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable |
                         ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable;
    if (!ImGui::BeginTable("guests", 4, fl)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Cash", ImGuiTableColumnFlags_WidthFixed, 60);
    ImGui::TableSetupColumn("Happy", ImGuiTableColumnFlags_WidthFixed, 48);
    ImGui::TableSetupColumn("Doing", ImGuiTableColumnFlags_WidthFixed, 120);
    ImGui::TableHeadersRow();
    if (ImGuiTableSortSpecs *ss = ImGui::TableGetSortSpecs()) {
        if (ss->SpecsCount) {
            int c = ss->Specs[0].ColumnIndex;
            bool asc = ss->Specs[0].SortDirection == ImGuiSortDirection_Ascending;
            std::stable_sort(shown.begin(), shown.end(), [&](const Row *a, const Row *b) {
                bool lt = c == 0 ? a->name < b->name : c == 1 ? a->cash < b->cash : c == 2 ? a->happy < b->happy
                                                                                       : a->state < b->state;
                bool gt = c == 0 ? b->name < a->name : c == 1 ? b->cash < a->cash : c == 2 ? b->happy < a->happy
                                                                                       : b->state < a->state;
                return asc ? lt : gt;
            });
        }
    }
    ImGuiListClipper clip;
    clip.Begin((int)shown.size());
    while (clip.Step()) {
        for (int i = clip.DisplayStart; i < clip.DisplayEnd; i++) {
            const Row &r = *shown[i];
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID((int)r.g);
            if (Track *t = find_track(r.g)) {
                ImGui::PushStyleColor(ImGuiCol_Text, t->col);
            }
            bool sel = ImGui::Selectable(r.name.c_str(), g_sel == r.g, ImGuiSelectableFlags_SpanAllColumns);
            if (find_track(r.g)) ImGui::PopStyleColor();
            if (sel) {
                g_sel_changed = g_sel != r.g;
                g_sel = r.g;
            }
            ImGui::PopID();
            ImGui::TableNextColumn();
            ImGui::Text("$%.0f", r.cash);
            ImGui::TableNextColumn();
            ImGui::Text("%3.0f%%", r.happy * 100);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(state_name(r.state));
        }
    }
    ImGui::EndTable();
}

// ---- the selected guest -----------------------------------------------------------------------------------

static void tab_overview(uintptr_t g) {
    static char buf[64];
    static uintptr_t buf_for = 0;
    if (buf_for != g) {
        strncpy(buf, utf8(name(g)).c_str(), sizeof buf - 1);
        buf_for = g;
    }
    ImGui::SetNextItemWidth(220);
    ImGui::InputText("##name", buf, sizeof buf);
    ImGui::SameLine();
    if (ImGui::Button("Rename") && buf[0]) set_name(g, wide(buf));
    track_controls(g);
    int cls = guest_class(g);
    ImGui::Text("Kind: %s, age group %d, sex %d", cls >= 0 ? kGuestClasses[cls].name : "?", get<uint8_t>(g + PERSON_TYPE),
                get<uint8_t>(g + SUB_TYPE));
    int st = get<int32_t>(g + STATE), sub = get<int32_t>(g + SUB_STATE);
    ImGui::Text("Doing: %s (state %d, step %d)", state_name(st), st, sub);
    Vec3 p = position(g);
    ImGui::Text("Position: %.1f, %.1f, %.1f", p.x, p.y, p.z);
    ImGui::Text("Heading for: %08x   Group: %08x", (unsigned)deref(g + STATE_TARGET), (unsigned)deref(g + GUEST_GROUP));
    int t = get<int32_t>(g + TIME_IN_PARK);
    ImGui::Text("Time in park: %d   Rides done: %d", t, get<int32_t>(g + NUM_RIDES_DONE));
    ImGui::Text("Bought: %d food, %d drinks, %d souvenirs, %d photos", get<int32_t>(g + NUM_FOOD),
                get<int32_t>(g + NUM_DRINKS), get<int32_t>(g + NUM_SOUVENIRS), get<int32_t>(g + PHOTOS_TAKEN));
    ImGui::Text("Favourite ride: %08x (rating %.2f)", (unsigned)deref(g + FAVOURITE_RIDE),
                get<float>(g + FAVOURITE_RIDE_RATING));
    ImGui::Text("Queueing time: %.1f s (feels like %.1f s)", get<float>(g + QUEUEING_TIME),
                get<float>(g + PERCEIVED_QUEUEING_TIME));
    ImGui::Separator();
    b_check("In the park", g, IS_IN_PARK);
    b_check("Wants to leave the park", g, WANT_LEAVE_PARK);
    b_check("Angry face", g, ANGRY_FACE);
    b_check("Enjoyed last ride", g, ENJOYED_LAST_RIDE);
    b_check("Needs cash (looking for an ATM)", g, NEEDS_CASH);
}

static void tab_money(uintptr_t g) {
    money("Cash", g, CASH, true);
    money("Withdrawn from ATMs", g, CASH + 0x20);
    money("Bank money (for ATMs)", g, CASH + 0x1c);
    ImGui::SeparatorText("Spent");
    money("Food", g, CASH + 0x4);
    money("Drinks", g, CASH + 0x8);
    money("Rides", g, CASH + 0xc);
    money("Park entry", g, CASH + 0x10);
    money("Souvenirs", g, CASH + 0x14);
    money("Total", g, CASH + 0x18);
    ImGui::SeparatorText("Price sense");
    f_slider("Price modifier", g, PRICE_MODIFIER, 0, 3);
}

static void tab_feelings(uintptr_t g) {
    ImGui::TextDisabled("Current values change toward the targets over time; lock a value to hold it.");
    ImGui::SeparatorText("Current");
    f_slider("Happiness##c", g, CURRENT_HEALTH + 4, 0, 1);
    f_slider("Energy##c", g, CURRENT_HEALTH, 0, 1);
    f_slider("Nausea##c", g, CURRENT_HEALTH + 8, 0, 1);
    f_slider("Sunburn##c", g, CURRENT_HEALTH + 12, 0, 1);
    ImGui::SeparatorText("Target");
    f_slider("Happiness##t", g, TARGET_HEALTH + 4, 0, 1);
    f_slider("Energy##t", g, TARGET_HEALTH, 0, 1);
    f_slider("Nausea##t", g, TARGET_HEALTH + 8, 0, 1);
    f_slider("Sunburn##t", g, TARGET_HEALTH + 12, 0, 1);
    ImGui::SeparatorText("Needs (food and drink: 1 = full; toilet: 1 = desperate)");
    f_slider("Food", g, CONSUMPTION, 0, 1);
    f_slider("Drink", g, CONSUMPTION + 4, 0, 1);
    f_slider("Toilet", g, CONSUMPTION + 8, 0, 1);
    i_input("Wetness", g, WETNESS, true);
    if (ImGui::Button("Make happy and healthy")) {
        for (uint32_t b : {CURRENT_HEALTH, TARGET_HEALTH}) {
            put(g + b, 1.0f); put(g + b + 4, 1.0f); put(g + b + 8, 0.0f); put(g + b + 12, 0.0f);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Fill needs")) { put(g + CONSUMPTION, 1.0f); put(g + CONSUMPTION + 4, 1.0f); put(g + CONSUMPTION + 8, 0.0f); }
}

static void tab_prefs(uintptr_t g) {
    f_slider("Intensity min", g, INTENSITY_MIN, -2, 14, "%.1f");
    f_slider("Intensity max", g, INTENSITY_MAX, 0, 14, "%.1f");
    i_input("Preferred ride intensity", g, PREFERED_RIDE_INTENSITY);
    int tol = get<int32_t>(g + NAUSEA_TOLERANCE);
    const char *tols[] = {"None", "Low", "Average", "High"};
    ImGui::SetNextItemWidth(160);
    if (ImGui::Combo("Nausea tolerance", &tol, tols, 4)) put<int32_t>(g + NAUSEA_TOLERANCE, tol);
    ImGui::TextDisabled("Rides above %d nausea make this guest sick.", 2 * tol + 2);
    f_slider("Pool preference", g, POOL_PREFERENCE, 0, 1);
    f_slider("Pool need", g, POOL_NEED, 0, 1);
}

static void tab_path(uintptr_t g) {
    track_controls(g);
    Vec3 p = position(g);
    Waypoints w = waypoints(g);
    ImGui::Text("Position: %.1f, %.1f, %.1f", p.x, p.y, p.z);
    ImGui::Text("Route: %d points, walking to point %d", w.count, w.next);
    float total = 0;
    Vec3 prev = p;
    if (ImGui::BeginTable("wp", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 260))) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 30);
        ImGui::TableSetupColumn("x");
        ImGui::TableSetupColumn("y");
        ImGui::TableSetupColumn("z");
        ImGui::TableSetupColumn("walk m");
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < w.pts.size(); i++) {
            const Vec3 &q = w.pts[i];
            bool ahead = (int)i >= w.next;   // points before the next one are already walked
            if (ahead) total += sqrtf((q.x - prev.x) * (q.x - prev.x) + (q.z - prev.z) * (q.z - prev.z));
            if (ahead) prev = q;
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%d%s", (int)i, (int)i == w.next ? " <" : "");
            ImGui::TableNextColumn(); ImGui::Text("%.1f", q.x);
            ImGui::TableNextColumn(); ImGui::Text("%.1f", q.y);
            ImGui::TableNextColumn(); ImGui::Text("%.1f", q.z);
            ImGui::TableNextColumn();
            if (ahead) ImGui::Text("%.1f", total); else ImGui::TextDisabled("passed");
        }
        ImGui::EndTable();
    }
    ImGui::Text("Heading for object %08x", (unsigned)deref(g + STATE_TARGET));
}

static void tab_goto(uintptr_t g) {
    static std::vector<Attraction> list;
    static DWORD list_at = 0;
    static char filter[48] = "";
    static int kind = 0;   // 0 all, 1 rides, 2 shops & stalls, 3 other
    static std::string msg;
    static bool msg_ok = false;
    static uintptr_t msg_for = 0;
    if (GetTickCount() - list_at > 2000) { list = attractions(); list_at = GetTickCount(); }
    if (msg_for != g) msg.clear();

    uintptr_t tgt = group_target(g);
    const Attraction *heading = nullptr;
    for (auto &a : list)
        if (a.obj == tgt) heading = &a;
    int n = group_size(g);
    ImGui::Text("Heading for: %s", heading ? heading->name.c_str() : "nowhere in particular (wandering the paths)");
    ImGui::TextDisabled("Guests walk in groups: sending this guest sends their whole group (%d %s).", n,
                        n == 1 ? "guest" : "guests");
    std::string os = order_status(g);
    if (!os.empty()) {
        ImGui::Text("Order: %s", os.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton("Cancel order")) cancel_order(g);
    }
    track_controls(g);
    ImGui::Separator();
    ImGui::SetNextItemWidth(180);
    ImGui::InputTextWithHint("##af", "search attractions", filter, sizeof filter);
    ImGui::SameLine();
    ImGui::RadioButton("all", &kind, 0); ImGui::SameLine();
    ImGui::RadioButton("rides", &kind, 1); ImGui::SameLine();
    ImGui::RadioButton("shops & stalls", &kind, 2); ImGui::SameLine();
    ImGui::RadioButton("other", &kind, 3);
    static bool want = true;
    ImGui::Checkbox("Make them hungry & thirsty when sent to a shop (guests only buy what they need)", &want);
    if (!msg.empty())
        ImGui::TextColored(msg_ok ? ImVec4(0.4f, 1, 0.5f, 1) : ImVec4(1, 0.45f, 0.4f, 1), "%s", msg.c_str());

    if (ImGui::BeginTable("attr", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV,
                          ImVec2(0, ImGui::GetContentRegionAvail().y - 34))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Attraction", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Kind", ImGuiTableColumnFlags_WidthFixed, 110);
        ImGui::TableSetupColumn("Entrance", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableHeadersRow();
        std::string f = filter;
        std::transform(f.begin(), f.end(), f.begin(), ::tolower);
        for (auto &a : list) {
            int k = a.ride ? 1 : a.shop ? 2 : 3;
            if (kind && kind != k) continue;
            std::string ln = a.name;
            std::transform(ln.begin(), ln.end(), ln.begin(), ::tolower);
            if (!f.empty() && ln.find(f) == std::string::npos) continue;
            ImGui::PushID((int)a.obj);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (a.obj == tgt) ImGui::TextColored(ImVec4(0.15f, 0.85f, 0.95f, 1), "%s  <- heading here", a.name.c_str());
            else ImGui::TextUnformatted(a.name.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(k == 1 ? "ride" : k == 2 ? "shop" : "other");
            if (!a.open) { ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 0.45f, 0.4f, 1), "closed"); }
            ImGui::TableNextColumn();
            static std::map<uintptr_t, int> pick;
            int &e = pick[a.obj];
            if (a.entrances > 1) {
                ImGui::SetNextItemWidth(70);
                ImGui::SliderInt("##e", &e, 1, a.entrances, "%d");
            } else if (a.entrances == 1) {
                e = 1;
                ImGui::TextDisabled("1");
            } else {
                ImGui::TextDisabled("none");
            }
            ImGui::TableNextColumn();
            ImGui::BeginDisabled(a.entrances <= 0 || !a.open);
            if (ImGui::SmallButton("Go here")) {
                std::string why;
                if (!a.ride && want) make_wanting(g);
                msg_ok = send_to(g, a.obj, std::max(e, 1) - 1, &why);
                if (msg_ok) order(g, a.obj, std::max(e, 1) - 1);
                msg = msg_ok ? "On the way to " + a.name + (a.ride ? " - they'll queue and ride." : ".") : why;
                msg_for = g;
                log("send %08x -> %s (%s): %s", (unsigned)g, a.name.c_str(), a.ride ? "ride" : "other",
                    msg_ok ? "ok" : why.c_str());
                if (msg_ok && !find_track(g)) toggle_track(g);   // show where they're going
            }
            ImGui::EndDisabled();
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (ImGui::Button("Leave the park")) {
        put<uint8_t>(g + WANT_LEAVE_PARK, 1);
        msg = "Wants to leave the park: they'll head for the exit when they next decide.";
        msg_ok = true;
        msg_for = g;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("%d attractions", (int)list.size());
}

static void tab_thoughts(uintptr_t g) {
    // RecentThoughts: {entries*, count}; each entry is 0x34 bytes: Thought, Variant, Time (-1 = empty slot), two references
    uint32_t arr = get<uint32_t>(g + RECENT_THOUGHTS);
    int32_t n = get<int32_t>(g + RECENT_THOUGHTS + 4);
    int shown = 0;
    if (arr && n > 0 && n <= 64) {
        for (int i = 0; i < n; i++) {
            uintptr_t e = arr + 0x34 * i;
            int id = get<int32_t>(e);
            float t = get<float>(e + 8);
            if (t < 0 && id == 0) continue;
            ImGui::BulletText("%s", thought_name(id));
            ImGui::SameLine();
            ImGui::TextDisabled("(#%d, variant %d, time %.0f)", id, get<int32_t>(e + 4), t);
            shown++;
        }
    }
    if (!shown) ImGui::TextDisabled("No recent thoughts yet (the game keeps the last %d).", n > 0 && n <= 64 ? n : 6);
    ImGui::SeparatorText("Give a thought");
    static int pick = 9;
    ImGui::SetNextItemWidth(260);
    if (ImGui::BeginCombo("##thought", thought_name(pick))) {
        for (int i = 0; i < thought_count(); i++)
            if (ImGui::Selectable(thought_name(i), i == pick)) pick = i;
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Put in first slot") && arr && n > 0 && n <= 64) {
        put<int32_t>(arr, pick);
        put<int32_t>(arr + 4, 0);
        if (get<float>(arr + 8) < 0) put<float>(arr + 8, 0.0f);
    }
}

static void tab_memory(uintptr_t g) {
    static int off = 0, type = 0;
    static char val[32] = "";
    ImGui::TextDisabled("Every byte of the guest. Offsets are listed in docs/research-physics-and-guests.md.");
    ImGui::SetNextItemWidth(100);
    ImGui::InputInt("offset (hex)", &off, 4, 16, ImGuiInputTextFlags_CharsHexadecimal);
    off = std::max(0, std::min(off, (int)GUEST_SIZE - 4));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(90);
    const char *types[] = {"int32", "float", "byte"};
    ImGui::Combo("##type", &type, types, 3);
    char cur[48];
    if (type == 0) snprintf(cur, sizeof cur, "%d", get<int32_t>(g + off));
    else if (type == 1) snprintf(cur, sizeof cur, "%g", get<float>(g + off));
    else snprintf(cur, sizeof cur, "%u", get<uint8_t>(g + off));
    ImGui::Text("now: %s", cur);
    ImGui::SetNextItemWidth(120);
    ImGui::InputText("new value", val, sizeof val);
    ImGui::SameLine();
    if (ImGui::Button("Write") && val[0]) {
        if (type == 0) put<int32_t>(g + off, (int32_t)strtol(val, nullptr, 0));
        else if (type == 1) put<float>(g + off, (float)atof(val));
        else put<uint8_t>(g + off, (uint8_t)strtol(val, nullptr, 0));
    }
    ImGui::BeginChild("hex", ImVec2(0, 0), ImGuiChildFlags_Borders);
    uint32_t row[4];
    for (uint32_t o = 0; o < GUEST_SIZE; o += 16) {
        if (!rd(g + o, row, 16)) break;
        float *f = (float *)row;
        ImGui::Text("+%03x  %08x %08x %08x %08x   %10.3g %10.3g %10.3g %10.3g", o, row[0], row[1], row[2], row[3], f[0],
                    f[1], f[2], f[3]);
    }
    ImGui::EndChild();
}

static void tab_all() {
    std::vector<Guest> gs = guests();
    int n = 0;
    for (auto &x : gs) n += guest_class(x.addr) >= 0 && get<uint8_t>(x.addr + IS_IN_PARK);
    ImGui::Text("%d guests in the park", n);
    auto each = [&](auto fn) {
        for (auto &x : gs)
            if (guest_class(x.addr) >= 0 && get<uint8_t>(x.addr + IS_IN_PARK)) fn(x.addr);
    };
    if (ImGui::Button("Everyone happy and healthy"))
        each([](uintptr_t g) {
            for (uint32_t b : {CURRENT_HEALTH, TARGET_HEALTH}) {
                put(g + b, 1.0f); put(g + b + 4, 1.0f); put(g + b + 8, 0.0f); put(g + b + 12, 0.0f);
            }
        });
    if (ImGui::Button("Fill everyone's needs (food, drink, toilet)"))
        each([](uintptr_t g) { put(g + CONSUMPTION, 1.0f); put(g + CONSUMPTION + 4, 1.0f); put(g + CONSUMPTION + 8, 0.0f); });
    if (ImGui::Button("Cure nausea")) each([](uintptr_t g) { put(g + CURRENT_HEALTH + 8, 0.0f); put(g + TARGET_HEALTH + 8, 0.0f); });
    static float give = 50;
    ImGui::SetNextItemWidth(120);
    ImGui::InputFloat("##give", &give, 10, 100, "$%.0f");
    ImGui::SameLine();
    if (ImGui::Button("Give everyone this much cash"))
        each([](uintptr_t g) { put<int32_t>(g + CASH, get<int32_t>(g + CASH) + (int32_t)lroundf(give * 100)); });
    static float lo = 0, hi = 14;
    ImGui::SetNextItemWidth(220);
    ImGui::DragFloatRange2("intensity range", &lo, &hi, 0.1f, -2, 14, "%.1f");
    ImGui::SameLine();
    if (ImGui::Button("Set for everyone"))
        each([](uintptr_t g) { put(g + INTENSITY_MIN, lo); put(g + INTENSITY_MAX, hi); });
    ImGui::Separator();
    ImGui::Text("Tracked on screen: %d of %d", (int)g_tracks.size(), kMaxTracks);
    ImGui::SameLine();
    if (ImGui::Button("Stop tracking all")) g_tracks.clear();
    ImGui::Text("Locked values: %d", (int)g_locks.size());
    ImGui::SameLine();
    if (ImGui::Button("Unlock all")) g_locks.clear();
}

void frame(bool toast) {
    draw_world();
    if (!g_open) {
        if (toast) {
            ImGui::SetNextWindowPos(ImVec2(12, 12));
            ImGui::SetNextWindowBgAlpha(0.75f);
            ImGui::Begin("##toast", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                                 ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings);
            ImGui::Text("Peep Editor loaded - press %s in a park", g_key_name);
            ImGui::End();
        }
        return;
    }
    if (GetTickCount() - g_last_scan_req > 4000) {
        request_scan();
        g_last_scan_req = GetTickCount();
    }
    refresh_rows(false);
    ImGui::SetNextWindowSize(ImVec2(1000, 620), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(40, 60), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Peep Editor", &g_open)) { ImGui::End(); return; }
    int in_park = 0;
    for (auto &r : g_rows) in_park += r.in_park;
    ImGui::Text("%d guests (%d in the park)", (int)g_rows.size(), in_park);
    ImGui::SameLine();
    ImGui::TextDisabled("scan %.0f ms", last_scan_ms());
    ImGui::SameLine();
    if (ImGui::SmallButton("Refresh")) { request_scan(); refresh_rows(true); }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    ImGui::InputTextWithHint("##filter", "search names", g_filter, sizeof g_filter);
    ImGui::SameLine();
    ImGui::Checkbox("in park only", &g_in_park_only);
    ImGui::SameLine();
    ImGui::TextDisabled("(%s closes)", g_key_name);

    ImGui::BeginChild("list", ImVec2(400, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
    if (g_rows.empty()) ImGui::TextWrapped(scans_done() ? "No guests found. Load a park that has guests." : "Looking for guests...");
    else guest_list();
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("detail", ImVec2(0, 0), ImGuiChildFlags_Borders);
    bool have = g_sel && guest_class(g_sel) >= 0;
    if (!have) g_sel = 0;
    if (have) {
        int idx = 0;
        for (auto &r : g_rows) if (r.g == g_sel) idx = r.index;
        ImGui::TextColored(ImVec4(0.15f, 0.85f, 0.95f, 1), "%s", display_name(g_sel, idx).c_str());
    }
    if (ImGui::BeginTabBar("tabs")) {
        if (have) {
            if (ImGui::BeginTabItem("Overview", nullptr, g_sel_changed ? ImGuiTabItemFlags_SetSelected : 0)) { tab_overview(g_sel); ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Money")) { tab_money(g_sel); ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Feelings & needs")) { tab_feelings(g_sel); ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Preferences")) { tab_prefs(g_sel); ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Path")) { tab_path(g_sel); ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Go to")) { tab_goto(g_sel); ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Thoughts")) { tab_thoughts(g_sel); ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Memory")) { tab_memory(g_sel); ImGui::EndTabItem(); }
        }
        if (ImGui::BeginTabItem("All guests")) { tab_all(); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
        g_sel_changed = false;
        if (!have) ImGui::TextDisabled("Pick a guest on the left.");
    }
    ImGui::EndChild();
    ImGui::End();
}

}  // namespace ui

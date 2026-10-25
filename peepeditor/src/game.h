// RCT3 (RCT3plus.exe, Platinum / Steam 2700) guest access. Every address and offset here comes from the
// research in docs/research-physics-and-guests.md (save-field registrations, constructors, the ride chooser).
#pragma once
#include <windows.h>
#include <stdint.h>
#include <string>
#include <vector>

namespace game {

// A guest object starts with one of these vtables and has the chooser interface at +0x400.
// 0x76aee0 / 0x7f8c80 are the two destructors that reset them (plain guest, and the derived class).
struct GuestClass { uint32_t vt, vt400; const char *name; };
extern const GuestClass kGuestClasses[2];

// Offsets inside a guest (relative to the object; from the save-field table at 0x7df8be / 0x76bd23).
enum : uint32_t {
    WORLD_MATRIX = 0x050,    // 4x4 floats, translation at +0x30
    PERSON_TYPE = 0x120, SUB_TYPE = 0x121,
    STATE = 0x190, SUB_STATE = 0x194,
    WAYPOINTS = 0x198,       // {Point *pts (16 bytes: x, y, z, FLT_MAX); int16 ?, count @+6, next @+8, last @+0xa}
    NAME = 0x1b0,            // string: {int refs; int len; int capacity; wchar_t *chars}
    STATE_TARGET = 0x1bc,    // reference
    ROUTE_CONTROLLER = 0x1ec,
    RECENT_THOUGHTS = 0x278, // {entries*, count (6)}; entry 0x34 bytes: Thought, Variant, Time (-1 = empty), refs
    FLAGS = 0x2a8,
    QUEUEING_TIME = 0x2dc, PERCEIVED_QUEUEING_TIME = 0x2e0, NUM_RIDES_DONE = 0x2e8,
    TIME_IN_PARK = 0x310,
    WANT_LEAVE_PARK = 0x404, LAST_RIDE_WAS_FAVOURITE = 0x405,
    TARGET_HEALTH = 0x408,   // Energy, Happiness, Nausea, SunBurn (floats 0..1)
    CURRENT_HEALTH = 0x418,
    CONSUMPTION = 0x428,     // Food, Drink, Toilet (1 = full)
    PREFERED_RIDE_INTENSITY = 0x434, INTENSITY_MAX = 0x438, INTENSITY_MIN = 0x43c,
    NAUSEA_TOLERANCE = 0x440, IS_IN_PARK = 0x444,
    CASH = 0x44c,            // CurrentCash, SpentOnFood, Drink, Rides, ParkEntry, Souvenirs, TotalSpent, ATMCash, WithdrawnCash (cents)
    NUM_FOOD = 0x470, NUM_DRINKS = 0x474, NUM_SOUVENIRS = 0x478,
    FAVOURITE_RIDE = 0x47c, FAVOURITE_RIDE_RATING = 0x490,
    ANGRY_FACE = 0x498, PHOTOS_TAKEN = 0x49c, BAD_FOOD_TIME = 0x4a0,
    PRICE_MODIFIER = 0x4b0, LEAVING_CLOSED_PARK = 0x4b4,
    FIREWORK_COUNT = 0x4d8, LASER_COUNT = 0x4dc, WATERJET_COUNT = 0x4e0,
    GUEST_GROUP = 0x4ec,
    WETNESS = 0x504, POOL_PREFERENCE = 0x508, POOL_NEED = 0x50c,
    IS_AVATAR = 0x544, ENJOYED_LAST_RIDE = 0x550, NEEDS_CASH = 0x558,
    GUEST_SIZE = 0x5c0,
};

// Memory access that can't crash the game: ReadProcessMemory / WriteProcessMemory on our own process fail
// cleanly on a freed or protected page.
bool rd(uintptr_t a, void *out, size_t n);
bool wr(uintptr_t a, const void *in, size_t n);
template <class T> T get(uintptr_t a, T fallback = T()) { T v; return rd(a, &v, sizeof v) ? v : fallback; }
template <class T> bool put(uintptr_t a, T v) { return wr(a, &v, sizeof v); }

// Region the scanner must skip (our own buffers can hold copies of guest bytes).
void exclude_region(uintptr_t base, size_t size);

struct Guest { uintptr_t addr; int cls; };
// Is this still a live guest (both vtables in place)? Returns the class index or -1.
int guest_class(uintptr_t a);
// Background scan of the game's heap for guest objects; the list is refreshed every few seconds.
void start_scanner();
// The game's renderer (vtable 0xfbf138, +0x210 = 0xfbf12c), found by the same scan; 0 until then. Its main camera
// block is at +0x740: view matrix at +0x790 and projection at +0x7d0 (D3D row-vector layout).
uintptr_t renderer();
void request_scan();
std::vector<Guest> guests();
int scans_done();
double last_scan_ms();

std::wstring name(uintptr_t g);
bool set_name(uintptr_t g, const std::wstring &n);
uintptr_t deref(uintptr_t ref_field);     // an engine reference field -> the object it points at (0 if none)

const char *state_name(int s);
const char *thought_name(int id);         // read from the game's own table at 0x10ede48
int thought_count();

struct Vec3 { float x, y, z; };

// Attractions (rides, shops, ATMs, toilets...) from the game's attraction manager (global 0x12f4e4c, list at +0x64).
// All of these call game code: use them on the game's thread only (the menu runs there).
struct Attraction { uintptr_t obj; std::string name; bool ride, shop; int entrances; bool open; };
std::vector<Attraction> attractions();
std::string attraction_name(uintptr_t obj);
uintptr_t group_target(uintptr_t guest);   // where the guest's group is heading (its FlockTarget), 0 when none
int group_size(uintptr_t guest);
// Send the guest's whole group to an attraction entrance, as the game does when a guest picks one at a junction
// (0x781a18..0x781c1d). Returns false with a reason when it can't.
bool send_to(uintptr_t guest, uintptr_t attraction, int entrance, std::string *why);
// Keep a sent group going there until it arrives (re-sends if the group's own AI changes its mind on the way).
void order(uintptr_t guest, uintptr_t attraction, int entrance);
void tick_orders();                         // every frame, game thread
std::string order_status(uintptr_t guest);  // "" when there's no order
void cancel_order(uintptr_t guest);
void make_wanting(uintptr_t guest);         // the whole group gets hungry and thirsty (food, drink = 0.25)
Vec3 position(uintptr_t g);
struct Waypoints { uint32_t ptr = 0; int16_t count = 0, next = 0, last = 0; std::vector<Vec3> pts; };
Waypoints waypoints(uintptr_t g);

}  // namespace game

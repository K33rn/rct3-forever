// Looking up an interface of a game object. Each object's class describes itself (its virtual function 1): the
// description holds a table, indexed by interface id, of where in the object each interface it has starts. The ids
// are numbers the game hands out at start-up and keeps in globals, one per interface.
#include "rewrite.h"

struct InterfaceEntry {
    char unknown[0x10];
    int32_t offset;   // +0x10: where the interface starts in the object
};
struct ClassInfo {
    char unknown[0x14];
    InterfaceEntry **interfaces;   // +0x14: by interface id; null where the class doesn't have one
    int32_t count;                 // +0x18
};
typedef ClassInfo *(__fastcall *ClassInfoFn)(void *self);

// The object's interface with the id, or null if the object is null or doesn't have it.
static void *find_interface(void *object, int32_t id) {
    if (!object) return nullptr;
    auto class_info = (ClassInfoFn)(*(void ***)object)[1];
    ClassInfo *info = class_info(object);
    if (!info || id >= info->count) return nullptr;
    InterfaceEntry *e = info->interfaces[id];
    return e ? (char *)object + e->offset : nullptr;
}

// Two lookups the ride choice uses (research notes: the ride's weather factor and its fit factor).
static volatile long g_weather, g_fit;

static void *__fastcall weather_interface(void *object) {
    g_weather++;
    return find_interface(object, *(int32_t *)0x13cde74);
}

static void *__fastcall fit_interface(void *object) {
    g_fit++;
    return find_interface(object, *(int32_t *)0x13d0680);
}

static const uint8_t weather_original[] = {0x56, 0x8b, 0xf1, 0x85, 0xf6, 0x74, 0x25, 0x8b};
static const uint8_t fit_original[] = {0x56, 0x8b, 0xf1, 0x85, 0xf6, 0x74, 0x25, 0x8b};

REWRITE("Object::WeatherInterface", 0x49ac80, weather_original, weather_interface, &g_weather);
REWRITE("Object::FitInterface", 0x4bd9d0, fit_original, fit_interface, &g_fit);

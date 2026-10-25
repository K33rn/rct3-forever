// GraphedValue: the history of one park statistic (park rating, guests in park, cash, park value, profit...) that
// the park windows draw as a graph and the save file stores. A ring buffer, newest value first.
#include "rewrite.h"

struct GraphedValue {
    float *data;    // capacity slots
    int index;      // slot of the newest value
    int capacity;   // 64 for the park's graphs
    int count;      // values stored so far, up to capacity
    // a flag follows (+0x10), not used by Push
};
static_assert(sizeof(GraphedValue) == 16, "the first four fields as RCT3plus.exe lays them out");

static volatile long g_pushes;

// Push: a new newest value. The newest slot moves back one (wrapping from 0 to the end), the value goes there, and
// the count grows until the buffer is full - from then on each push overwrites the oldest value.
// The game calls it as a member function (this in ecx, the value on the stack, popped by the callee): fastcall
// with an unused second register argument has the same shape.
static void __fastcall push(GraphedValue *g, void *, float value) {
    g_pushes++;
    if (--g->index < 0) g->index = g->capacity - 1;
    g->data[g->index] = value;
    if (g->count < g->capacity) g->count++;
}

static const uint8_t push_original[] = {0xff, 0x49, 0x04, 0x79, 0x07, 0x8b, 0x41, 0x08};   // dec [ecx+4]; jns; mov eax, [ecx+8]

REWRITE("GraphedValue::Push", 0x611af0, push_original, push, &g_pushes);

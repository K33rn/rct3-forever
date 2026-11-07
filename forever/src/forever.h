#pragma once
#include <windows.h>
#include <stdint.h>

void flog(const char *fmt, ...);   // RCT3Forever.log beside forever.dll
int setting(const char *section, const char *key, int def);
void install_fixes();   // fixes.cpp

// Send every call of the game's function at `at` to `with`. The function's first bytes must be `expect` (the
// known RCT3plus.exe), or nothing is changed and false comes back.
bool replace_function(const char *name, uintptr_t at, const uint8_t *expect, size_t n, void *with);

// Point an import address table slot of RCT3plus.exe at `with`; returns what was there.
void *replace_import(uintptr_t slot, void *with);

namespace rewrite {
void install_all();   // every function RCT3 Forever has rewritten (rewrite/*.cpp)
void report();        // logs how often each rewritten function ran, when that changed
}

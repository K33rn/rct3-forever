#pragma once
#include <stdint.h>

// The RCT3plus.exe RCT3 Forever runs on: RollerCoaster Tycoon 3 Platinum / Gold (with Soaked! and Wild!), Steam or
// disc, unpatched. Linked 2005-10-13 11:10:01 UTC; always loaded at 0x400000 (it has no relocations), so the
// addresses below are fixed.
namespace exe {
constexpr uint32_t TIMESTAMP = 0x434e4089;   // PE header TimeDateStamp
constexpr uint32_t IMAGE_SIZE = 0x104c000;   // PE header SizeOfImage
constexpr uintptr_t IMAGE_BASE = 0x400000;

// import address table slots
constexpr uintptr_t IAT_Direct3DCreate9 = 0xf26430;
constexpr uintptr_t IAT_CreateWindowExA = 0xf26388;
}

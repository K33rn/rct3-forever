// list_windows: one function written out 2 times (checked by tools/family.py: the same instructions, jumps by
// where they go, the exception handler set aside). Its parameters, by the instructions they appear in:
//   p0: +0x1dd mov dword ptr [esi + 0x16c], ecx
//   p1: +0x20a mov dword ptr [esi + 0x170], 0
// Do not edit by hand.
LIST_WINDOWS(0x5996a0, 0x16c, 0x170, 0x6a, 0xff, 0x68, 0x18, 0x20, 0xe2, 0x00)
LIST_WINDOWS(0x599990, 0x170, 0x16c, 0x6a, 0xff, 0x68, 0x98, 0x20, 0xe2, 0x00)

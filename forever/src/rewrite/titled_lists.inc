// titled_lists: one function written out 2 times (checked by tools/family.py: the same instructions, jumps by
// where they go, the exception handler set aside). Its parameters, by the instructions they appear in:
//   p0: +0x2e mov eax, dword ptr [0x12f4ef4], +0x170 mov ecx, dword ptr [0x12f4ef4], +0x206 mov eax, dword ptr [0x12f4ef4]
//   p1: +0x1c4 mov dword ptr [ebx], 0xf33800
//   p2: +0x1ca mov dword ptr [ebx + 4], 0xf337e8
// Do not edit by hand.
TITLED_LISTS(0x4d60a0, 0x12f4ef4, 0xf33800, 0xf337e8, 0x6a, 0xff, 0x68, 0xdc, 0x45, 0xe1, 0x00)
TITLED_LISTS(0xb80020, 0x13d014c, 0xf9bcf0, 0xf9bcd8, 0x6a, 0xff, 0x68, 0xdc, 0x84, 0xe9, 0x00)

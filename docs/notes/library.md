# Libraries linked into RCT3plus.exe

Code in the exe that isn't the game's own: Microsoft libraries that were linked in when the game was built. It stays
as it is (Forever has no reason to rewrite Microsoft's code), so `tools/progress.py` counts these ranges as library
code, not as game code to do. Ghidra recognised a few hundred of these functions by their signatures; the ranges
below cover the rest, worked out from what the code contains and how it connects to the game.

How a range was found: the game's own functions all start on 16-byte boundaries (Visual C++ 2003 pads between them),
while these libraries were compiled separately and their functions start anywhere. Each range is self-contained:
apart from the game's memory functions (`0xBCDFC0` allocate, `0xBCE000` free, `0x402B90`), which the game supplies
to every library through its own `new` and `delete`, nothing in it calls game code, and the game enters it at a
handful of places only.

| Range | Library | How we know |
|---|---|---|
| `0xCF19B4`–`0xDB334D` | D3DX 9, the Direct3D helper library (texture loading and saving, shader assembler, the HLSL compiler), with the libpng 1.0.5 and zlib it reads PNG files with | Messages `D3DX: (WARN) Overloaded ::new and ::delete...`, `Saved by D3DX`; the registry key `Software\Microsoft\Direct3D` and `d3d9d.dll` / `DebugSetMute` (D3DX's debug settings); the HLSL compiler's error messages (`intrinsic function '%s' is not yet implemented`, `vertex shader must minimally write all fields of POSITION`, a yacc parser); libpng's version string `1.0.5`; zlib's inflate messages (`invalid block type`, `invalid distance code`); Radiance HDR loading (`#?RADIANCE`, `32-bit_rle_rgbe`). 2,262 functions, 720 KB; it calls out only to the three memory functions and is entered from 21 places (the game's shader setup calls its shader assembler `0xCF6275` 132 times) |
| `0xDB334D`–`0xDF61A0` | DirectX error strings (DxErr 9) and the Microsoft C runtime | `0xDB334D` and `0xDC4E99` turn an HRESULT into its name, from tables of 1,364 names (`E_ABORT`, `CO_E_INIT_TLS`, ...; ASCII and wide-character versions), followed by their descriptions (`Operation aborted`, `Invalid pointer`, ...); then the C runtime, of which Ghidra recognised 446 functions (`_sprintf`, `__getlocaleinfo`, `__mbschr`, ... up to `0xDF6122`). 566 functions, 153 KB |

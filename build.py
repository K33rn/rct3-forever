"""Build the Peep Editor mod: d3d9.dll (32-bit) with Zig's C++ compiler, then PeepEditor.zip.

usage: uv run --no-project --with ziglang --with pefile python build.py
Dear ImGui is downloaded into third_party/ on the first build (MIT licence, shipped in the zip's licence file).
"""
import io
import os
import subprocess
import sys
import urllib.request
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
IMGUI_VER = '1.91.9'
IMGUI = os.path.join(HERE, 'third_party', 'imgui')
OUT = os.path.join(HERE, 'out')
VERSION = '1.0'


def fetch_imgui():
    if os.path.exists(os.path.join(IMGUI, 'imgui.cpp')):
        return
    url = f'https://github.com/ocornut/imgui/archive/refs/tags/v{IMGUI_VER}.zip'
    print('downloading', url)
    z = zipfile.ZipFile(io.BytesIO(urllib.request.urlopen(url).read()))
    root = f'imgui-{IMGUI_VER}/'
    for n in z.namelist():
        if n.startswith(root) and not n.endswith('/'):
            p = os.path.join(IMGUI, n[len(root):])
            os.makedirs(os.path.dirname(p), exist_ok=True)
            open(p, 'wb').write(z.read(n))


def build():
    os.makedirs(OUT, exist_ok=True)
    src = [os.path.join(HERE, 'src', f) for f in ('main.cpp', 'game.cpp', 'goto.cpp', 'ui.cpp')]
    src += [os.path.join(IMGUI, f) for f in ('imgui.cpp', 'imgui_draw.cpp', 'imgui_tables.cpp', 'imgui_widgets.cpp')]
    src += [os.path.join(IMGUI, 'backends', f) for f in ('imgui_impl_dx9.cpp', 'imgui_impl_win32.cpp')]
    dll = os.path.join(OUT, 'd3d9.dll')
    cmd = [sys.executable, '-m', 'ziglang', 'c++', '-target', 'x86-windows-gnu', '-shared', '-O2', '-std=c++17',
           '-fno-exceptions', '-fno-rtti', '-DNDEBUG', '-DIMGUI_DISABLE_DEMO_WINDOWS', '-DWIN32_LEAN_AND_MEAN', '-Wno-nullability-completeness',
           '-I', IMGUI, '-I', os.path.join(IMGUI, 'backends'), '-o', dll, *src,
           os.path.join(HERE, 'src', 'exports.def'), '-luser32', '-lgdi32', '-limm32', '-ldwmapi', '-lkernel32',
           '-s']
    print('compiling', len(src), 'files')
    subprocess.check_call(cmd)
    for junk in ('d3d9.lib', 'd3d9.pdb'):
        if os.path.exists(os.path.join(OUT, junk)):
            os.remove(os.path.join(OUT, junk))
    return dll


def check(dll):
    import pefile
    pe = pefile.PE(dll)
    assert pe.FILE_HEADER.Machine == 0x14c, 'not 32-bit'
    names = [e.name.decode() for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name]
    assert names == ['Direct3DCreate9'], names
    deps = sorted(e.dll.decode().lower() for e in pe.DIRECTORY_ENTRY_IMPORT)
    print('exports', names, 'imports', deps, os.path.getsize(dll), 'bytes')


def package(dll):
    zpath = os.path.join(OUT, 'PeepEditor.zip')
    with zipfile.ZipFile(zpath, 'w', zipfile.ZIP_DEFLATED) as z:
        z.write(dll, 'PeepEditor/d3d9.dll')
        z.write(os.path.join(HERE, 'PeepEditor.ini'), 'PeepEditor/PeepEditor.ini')
        z.write(os.path.join(HERE, 'README.txt'), 'PeepEditor/README.txt')
        z.writestr('PeepEditor/LICENSES.txt', 'Dear ImGui ' + IMGUI_VER + ' (https://github.com/ocornut/imgui)\n\n' +
                   open(os.path.join(IMGUI, 'LICENSE.txt')).read())
    print('wrote', zpath, os.path.getsize(zpath), 'bytes')
    return zpath


if __name__ == '__main__':
    fetch_imgui()
    d = build()
    check(d)
    package(d)

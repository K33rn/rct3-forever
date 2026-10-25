"""Build RCT3 Forever: forever.dll (with the Park & Peep Editor built in) and RCT3Forever.exe (32-bit, Zig's C++
compiler), then dist/RCT3Forever-<version>.zip.

usage: uv run --no-project --with ziglang --with pefile python build.py
Dear ImGui is downloaded into third_party/ on the first build (MIT licence, shipped in the zip's licence file).
"""
import glob
import io
import os
import subprocess
import sys
import urllib.request
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
VERSION = '0.1'
IMGUI_VER = '1.91.9'
IMGUI = os.path.join(HERE, 'third_party', 'imgui')
OUT = os.path.join(HERE, 'out')
DIST = os.path.join(HERE, 'dist')
ZIG = [sys.executable, '-m', 'ziglang', 'c++', '-target', 'x86-windows-gnu', '-O2', '-std=c++17', '-fno-exceptions',
       '-fno-rtti', '-DNDEBUG', '-DWIN32_LEAN_AND_MEAN', '-Wno-nullability-completeness', '-s']


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


def build_dll():
    peep = os.path.join(HERE, 'peepeditor', 'src')
    src = glob.glob(os.path.join(HERE, 'forever', 'src', '*.cpp'))
    src += glob.glob(os.path.join(HERE, 'forever', 'src', 'rewrite', '*.cpp'))
    src += [os.path.join(peep, f) for f in ('main.cpp', 'game.cpp', 'goto.cpp', 'park.cpp', 'ui.cpp')]
    src += [os.path.join(IMGUI, f) for f in ('imgui.cpp', 'imgui_draw.cpp', 'imgui_tables.cpp', 'imgui_widgets.cpp')]
    src += [os.path.join(IMGUI, 'backends', f) for f in ('imgui_impl_dx9.cpp', 'imgui_impl_win32.cpp')]
    dll = os.path.join(OUT, 'forever.dll')
    cmd = ZIG + ['-shared', '-DFOREVER', f'-DFOREVER_VERSION="{VERSION}"', '-DIMGUI_DISABLE_DEMO_WINDOWS',
                 '-I', IMGUI, '-I', os.path.join(IMGUI, 'backends'), '-o', dll, *src,
                 '-luser32', '-lgdi32', '-limm32', '-ldwmapi', '-lkernel32']
    print('compiling forever.dll:', len(src), 'files')
    subprocess.check_call(cmd)
    return dll


def build_exe():
    exe = os.path.join(OUT, 'RCT3Forever.exe')
    cmd = ZIG + ['-o', exe, os.path.join(HERE, 'launcher', 'launcher.cpp'), '-Wl,--subsystem,windows',
                 '-luser32', '-lshell32', '-lole32', '-ladvapi32', '-lkernel32']
    print('compiling RCT3Forever.exe')
    subprocess.check_call(cmd)
    return exe


def check(dll, exe):
    import pefile
    for f in (dll, exe):
        assert pefile.PE(f, fast_load=True).FILE_HEADER.Machine == 0x14c, f + ' is not 32-bit'
    for junk in glob.glob(os.path.join(OUT, '*.lib')) + glob.glob(os.path.join(OUT, '*.pdb')):
        os.remove(junk)
    print('forever.dll', os.path.getsize(dll), 'bytes; RCT3Forever.exe', os.path.getsize(exe), 'bytes')


def package(dll, exe):
    os.makedirs(DIST, exist_ok=True)
    zpath = os.path.join(DIST, f'RCT3Forever-{VERSION}.zip')
    with zipfile.ZipFile(zpath, 'w', zipfile.ZIP_DEFLATED) as z:
        z.write(exe, 'RCT3Forever/RCT3Forever.exe')
        z.write(dll, 'RCT3Forever/forever.dll')
        z.write(os.path.join(HERE, 'forever', 'RCT3Forever.ini'), 'RCT3Forever/RCT3Forever.ini')
        z.write(os.path.join(HERE, 'README.md'), 'RCT3Forever/README.md')
        z.writestr('RCT3Forever/LICENSES.txt', open(os.path.join(HERE, 'LICENSE')).read() +
                   '\n\n---- Dear ImGui ' + IMGUI_VER + ' (https://github.com/ocornut/imgui)\n\n' +
                   open(os.path.join(IMGUI, 'LICENSE.txt')).read())
    print('wrote', zpath, os.path.getsize(zpath), 'bytes')


if __name__ == '__main__':
    os.makedirs(OUT, exist_ok=True)
    fetch_imgui()
    d, e = build_dll(), build_exe()
    check(d, e)
    if os.path.exists(os.path.join(HERE, 'README.md')):
        package(d, e)

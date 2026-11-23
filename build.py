"""Build RCT3 Forever: forever.dll (with the Cheats Menu built in) and RCT3Forever.exe (32-bit, Zig's C++
compiler), then dist/RCT3Forever-<version>.zip and the installer dist/RCT3Forever-<version>-setup.exe.

usage: uv run --no-project --with ziglang --with pefile python build.py
       uv run --no-project --with ziglang --with pefile python build.py test ["<game folder>"]
Dear ImGui is downloaded into third_party/ on the first build (MIT licence, shipped in the zip's licence file), and
so is NSIS (the installer maker; zlib licence, only used to build, checked against NSIS_SHA256).
`test` builds tools/equiv and checks every rewritten function against the game's own (see tools/equiv/equiv.cpp);
the game folder defaults to the one in out/RCT3Forever.ini.
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
NSIS_VER = '3.10'
NSIS_URL = f'https://downloads.sourceforge.net/project/nsis/NSIS%203/{NSIS_VER}/nsis-{NSIS_VER}.zip'
NSIS_SHA256 = 'fcdce3229717a2a148e7cda0ab5bdb667f39d8fb33ede1da8dabc336bd5ad110'
NSIS = os.path.join(HERE, 'third_party', 'nsis')
OUT = os.path.join(HERE, 'out')
DIST = os.path.join(HERE, 'dist')
# -march=i686 (no SSE): float maths on the x87 FPU like the game (Visual C++ 2003), so rewrites round as it does
# -fno-strict-aliasing: rewrites read and write the game's memory through casts of every type, as the game's own code
# does; with strict aliasing the compiler may keep a value it read past a write of another type (a loop that never ends)
ZIG = [sys.executable, '-m', 'ziglang', 'c++', '-target', 'x86-windows-gnu', '-O2', '-std=c++17', '-fno-exceptions',
       '-fno-rtti', '-fno-strict-aliasing', '-march=i686', '-DNDEBUG', '-DWIN32_LEAN_AND_MEAN', '-Wno-nullability-completeness', '-s']


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


def fetch_nsis():
    makensis = os.path.join(NSIS, 'makensis.exe')
    if os.path.exists(makensis):
        return makensis
    import hashlib
    print('downloading', NSIS_URL)
    data = urllib.request.urlopen(urllib.request.Request(NSIS_URL, headers={'User-Agent': 'Wget/1.21'}), timeout=300).read()
    got = hashlib.sha256(data).hexdigest()
    if got != NSIS_SHA256:
        sys.exit(f'the NSIS download is not the expected file (sha256 {got})')
    z = zipfile.ZipFile(io.BytesIO(data))
    root = f'nsis-{NSIS_VER}/'
    for n in z.namelist():
        if n.startswith(root) and not n.endswith('/'):
            p = os.path.join(NSIS, n[len(root):])
            os.makedirs(os.path.dirname(p), exist_ok=True)
            open(p, 'wb').write(z.read(n))
    return makensis


def licences():
    return (open(os.path.join(HERE, 'LICENSE')).read() + '\n\n---- Dear ImGui ' + IMGUI_VER +
            ' (https://github.com/ocornut/imgui)\n\n' + open(os.path.join(IMGUI, 'LICENSE.txt')).read())


def build_installer(dll, exe, per_user=False, out_dir=DIST):
    """The installer, from the files staged in out/installer; per_user makes the no-administrator one the tests use."""
    import shutil
    makensis = fetch_nsis()
    stage = os.path.join(OUT, 'installer')
    os.makedirs(stage, exist_ok=True)
    shutil.copy(exe, os.path.join(stage, 'RCT3Forever.exe'))
    shutil.copy(dll, os.path.join(stage, 'forever.dll'))
    shutil.copy(os.path.join(HERE, 'forever', 'RCT3Forever.ini'), os.path.join(stage, 'RCT3Forever.default.ini'))
    shutil.copy(os.path.join(HERE, 'README.md'), os.path.join(stage, 'README.md'))
    open(os.path.join(stage, 'LICENSES.txt'), 'w', newline='\r\n').write(licences())
    os.makedirs(out_dir, exist_ok=True)
    cmd = [makensis, '/V2', f'/DVERSION={VERSION}', f'/DOUT={out_dir}', f'/DFILES={stage}']
    if per_user:
        cmd.append('/DPERUSER')
    cmd.append(os.path.join(HERE, 'installer', 'RCT3Forever.nsi'))
    subprocess.check_call(cmd)
    setup = os.path.join(out_dir, f'RCT3Forever-{VERSION}-setup.exe')
    print('wrote', setup, os.path.getsize(setup), 'bytes')
    return setup


def build_dll():
    peep = os.path.join(HERE, 'cheatsmenu', 'src')
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
                 '-luser32', '-lshell32', '-lole32', '-luuid', '-ladvapi32', '-lkernel32']
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
        z.writestr('RCT3Forever/LICENSES.txt', licences())
    print('wrote', zpath, os.path.getsize(zpath), 'bytes')


def test(game):
    exe = os.path.join(OUT, 'equiv.exe')
    src = [os.path.join(HERE, 'tools', 'equiv', 'equiv.cpp')]
    src += glob.glob(os.path.join(HERE, 'forever', 'src', 'rewrite', '*.cpp'))
    # the tests kept in files of their own (tools/equiv/tests/*.inc), listed for equiv.cpp to include
    tests = sorted(glob.glob(os.path.join(HERE, 'tools', 'equiv', 'tests', '*.inc')))
    listing = ''.join('#include "%s"\n' % t.replace('\\', '/') for t in tests)
    inc = os.path.join(OUT, 'equiv_tests.inc')
    if not os.path.exists(inc) or open(inc).read() != listing:
        open(inc, 'w').write(listing)
    # its own fixed base far from 0x400000, where the game is loaded, and no ASLR to move it there
    # and a 1 MB stack (Zig's default 16 MB lands right on 0x400000)
    subprocess.check_call(ZIG + ['-I', OUT, '-o', exe, *src, '-Wl,--image-base,0x30000000', '-Wl,--stack,0x100000'])
    import pefile
    pe = pefile.PE(exe)
    pe.OPTIONAL_HEADER.DllCharacteristics &= ~0x40   # IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE
    data = pe.write()
    pe.close()
    open(exe, 'wb').write(data)
    if not game:
        import configparser
        ini = configparser.ConfigParser(comment_prefixes=(';',), strict=False)
        ini.read(os.path.join(OUT, 'RCT3Forever.ini'))
        game = ini.get('Game', 'Folder', fallback='')
    if not game:
        sys.exit('give the game folder: build.py test "<game folder>"')
    print('checking the rewrites against', game)
    sys.exit(subprocess.call([exe, os.path.join(game, 'RCT3plus.exe')] + sys.argv[3:]))


if __name__ == '__main__':
    os.makedirs(OUT, exist_ok=True)
    if sys.argv[1:2] == ['test']:
        test(sys.argv[2] if len(sys.argv) > 2 else '')
    fetch_imgui()
    d, e = build_dll(), build_exe()
    check(d, e)
    if os.path.exists(os.path.join(HERE, 'README.md')):
        package(d, e)
        build_installer(d, e)

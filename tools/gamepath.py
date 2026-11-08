"""Where your RCT3plus.exe is, for the tools that read the game's code. In order: the RCT3_EXE environment variable,
the game folder in out/RCT3Forever.ini (set by build.py test or the first start of RCT3Forever.exe), then the usual
Steam folder. Run it to print the path: `uv run --no-project python tools/gamepath.py`."""
import configparser, os

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
STEAM = r'C:\Program Files (x86)\Steam\steamapps\common\Rollercoaster Tycoon 3 Gold'


def game_exe():
    if os.environ.get('RCT3_EXE'):
        return os.environ['RCT3_EXE']
    ini = configparser.ConfigParser(comment_prefixes=(';',), strict=False)
    ini.read(os.path.join(HERE, 'out', 'RCT3Forever.ini'))
    for folder in (ini.get('Game', 'Folder', fallback=''), STEAM):
        if folder and os.path.exists(os.path.join(folder, 'RCT3plus.exe')):
            return os.path.join(folder, 'RCT3plus.exe')
    raise SystemExit('RCT3plus.exe not found: set RCT3_EXE to its path, or the game folder in out/RCT3Forever.ini')


if __name__ == '__main__':
    print(game_exe())

import shutil
import os
from pathlib import Path
import sys


def PrintUsage():
    print("Usage: python copydlls.py <build type> <path to the build dir>\n"
          "<build type> must be either debug or release.\n\n"
          "Explanations:\n"
          "Azharja is dynamically linked to Qt and Scintilla. This means that "
          "in order to run, azharja.exe needs to have some dlls next to it. "
          "The purpose of this script is to copy the required dlls to the "
          "folder where azharja.exe is built.\n"
          "The script also assumes that the Qt6_ROOT environment variable is "
          "set properly (this is required in order to build Azharja anyway).")


# Set folder paths that will be used in this script.
# --------------------------------------------------

this_script_dir = Path(__file__).resolve().parent
qt_dir = os.environ.get("Qt6_ROOT")
if qt_dir is None:
    raise RuntimeError(
        "The Qt6_ROOT environment variable is not set. We need this variable"
        "in order to copy Qt dll files required for Azharja to run.")
qt_dir = Path(qt_dir)

if len(sys.argv) < 2:
    PrintUsage()
    sys.exit(1)

build_type = sys.argv[1]
if build_type not in ["debug", "release"]:
    PrintUsage()
    sys.exit(1)

build_dir = Path(sys.argv[2])


# Copy files for debug
# --------------------

if build_type == "debug":
    os.makedirs(build_dir, exist_ok=True)
    shutil.copy(Path.joinpath(this_script_dir,
                "scintilla/bin/debug/ScintillaEdit5.dll"), build_dir)
    shutil.copy(Path.joinpath(qt_dir, "bin/Qt6Core5Compatd.dll"), build_dir)
    shutil.copy(Path.joinpath(qt_dir, "bin/Qt6Core5Compatd.dll"), build_dir)
    shutil.copy(Path.joinpath(qt_dir, "bin/Qt6Cored.dll"), build_dir)
    shutil.copy(Path.joinpath(qt_dir, "bin/Qt6Guid.dll"), build_dir)
    shutil.copy(Path.joinpath(qt_dir, "bin/Qt6Networkd.dll"), build_dir)
    shutil.copy(Path.joinpath(qt_dir, "bin/Qt6Svgd.dll"), build_dir)
    shutil.copy(Path.joinpath(qt_dir, "bin/Qt6Widgetsd.dll"), build_dir)
    shutil.copytree(Path.joinpath(qt_dir, "plugins/iconengines"),
                    Path.joinpath(build_dir, "iconengines"), dirs_exist_ok=True)
    shutil.copytree(Path.joinpath(qt_dir, "plugins/imageformats"),
                    Path.joinpath(build_dir, "imageformats"), dirs_exist_ok=True)
    shutil.copytree(Path.joinpath(qt_dir, "plugins/platforms"),
                    Path.joinpath(build_dir, "platforms"), dirs_exist_ok=True)
    shutil.copytree(Path.joinpath(qt_dir, "plugins/styles"),
                    Path.joinpath(build_dir, "styles"), dirs_exist_ok=True)
    shutil.copytree(Path.joinpath(qt_dir, "translations"),
                    Path.joinpath(build_dir, "translations"), dirs_exist_ok=True)


# Copy files for Release
# ----------------------

if build_type == "release":
    os.makedirs(build_dir, exist_ok=True)
    shutil.copy(Path.joinpath(this_script_dir,
                "scintilla/bin/release/ScintillaEdit5.dll"), build_dir)
    shutil.copy(Path.joinpath(qt_dir, "bin/Qt6Core5Compat.dll"), build_dir)
    shutil.copy(Path.joinpath(qt_dir, "bin/Qt6Core.dll"), build_dir)
    shutil.copy(Path.joinpath(qt_dir, "bin/Qt6Gui.dll"), build_dir)
    shutil.copy(Path.joinpath(qt_dir, "bin/Qt6Network.dll"), build_dir)
    shutil.copy(Path.joinpath(qt_dir, "bin/Qt6Svg.dll"), build_dir)
    shutil.copy(Path.joinpath(qt_dir, "bin/Qt6Widgets.dll"), build_dir)
    shutil.copytree(Path.joinpath(qt_dir, "plugins/iconengines"),
                    Path.joinpath(build_dir, "iconengines"), dirs_exist_ok=True)
    shutil.copytree(Path.joinpath(qt_dir, "plugins/imageformats"),
                    Path.joinpath(build_dir, "imageformats"), dirs_exist_ok=True)
    shutil.copytree(Path.joinpath(qt_dir, "plugins/platforms"),
                    Path.joinpath(build_dir, "platforms"), dirs_exist_ok=True)
    shutil.copytree(Path.joinpath(qt_dir, "plugins/styles"),
                    Path.joinpath(build_dir, "styles"), dirs_exist_ok=True)
    shutil.copytree(Path.joinpath(qt_dir, "translations"),
                    Path.joinpath(build_dir, "translations"), dirs_exist_ok=True)

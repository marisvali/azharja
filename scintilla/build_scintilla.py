import shutil
import os
import subprocess
import sys
from pathlib import Path

# Set folder paths that will be used in this script.
# --------------------------------------------------

this_script_dir = Path(__file__).resolve().parent
qt_scintilla_dir = Path.joinpath(this_script_dir, "src/qt/ScintillaEdit")
qmake_dir = Path.joinpath(qt_scintilla_dir, "qmake")
src_scintilla_dir = Path.joinpath(this_script_dir, "src")
includes_dir = Path.joinpath(this_script_dir, "include")


# Run "python WidgetGen.py" as instructed by src/qt/README
# --------------------------------------------------------

os.chdir(qt_scintilla_dir)
subprocess.run([sys.executable, "WidgetGen.py"], check=True)


# Generate the release version of Scintilla
# -----------------------------------------

# Run "qmake ScintillaEdit.pro".
if os.path.exists(qmake_dir):
    shutil.rmtree(qmake_dir)
os.makedirs(qmake_dir, exist_ok=True)
os.chdir(qmake_dir)
subprocess.run(["qmake", "../ScintillaEdit.pro"], check=True)
subprocess.run(["nmake"], check=True)

# # Copy the .dll and .lib files.
shutil.copy("../../bin/ScintillaEdit5.dll",
            "../../../../bin/release/ScintillaEdit5.dll")
shutil.copy("../../bin/ScintillaEdit5.lib",
            "../../../../bin/release/ScintillaEdit5.lib")


# Generate the debug version of Scintilla
# ---------------------------------------

# Make a debug version of ScintillaEdit.pro by adding the line
# "CONFIG += debug" somewhere near the top.
# Search for where the line "VERSION = .." appears as that seems to be
# near the top and I expect in future versions of Scintilla it will still
# be present and near the top.
os.chdir(qt_scintilla_dir)
with open("ScintillaEdit.pro", "r") as file:
    lines = file.readlines()

for i, item in enumerate(lines):
    if item.startswith("VERSION"):
        lines.insert(i, "CONFIG += debug\n")
        break

with open("ScintillaEdit_debug.pro", "w") as file:
    file.writelines(lines)

# Run "qmake ScintillaEdit_debug.pro".
if os.path.exists(qmake_dir):
    shutil.rmtree(qmake_dir)
os.makedirs(qmake_dir, exist_ok=True)
os.chdir(qmake_dir)
subprocess.run(["qmake", "../ScintillaEdit_debug.pro"], check=True)
subprocess.run(["nmake"], check=True)

# Copy the .dll and .lib files.
shutil.copy("../../bin/ScintillaEdit5.dll",
            "../../../../bin/debug/ScintillaEdit5.dll")
shutil.copy("../../bin/ScintillaEdit5.lib",
            "../../../../bin/debug/ScintillaEdit5.lib")


# Copy all header files to the include directory
# ----------------------------------------------

all_headers = src_scintilla_dir.rglob("*.h", case_sensitive=False)
for src in all_headers:
    dst = Path.joinpath(includes_dir, src.name)
    shutil.copy(src, dst)

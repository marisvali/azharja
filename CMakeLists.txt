cmake_minimum_required(VERSION 3.16)

project(azharja VERSION 2.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile commands database (needed by clangd).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Network Core5Compat)
qt_standard_project_setup()

# Create a target called ScintillaEdit that just uses some .h and .lib files. This target can then be used by my other targets like any other CMake target.
add_library(ScintillaEdit STATIC IMPORTED
        ../../qt-sources/qtbase/src/corelib/global/qlogging.cpp)
set_target_properties(ScintillaEdit PROPERTIES
  IMPORTED_LOCATION_DEBUG "${CMAKE_SOURCE_DIR}/scintilla/lib/ScintillaEditd.lib"
  IMPORTED_LOCATION_RELEASE "${CMAKE_SOURCE_DIR}/scintilla/lib/ScintillaEdit.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/scintilla/include"
)

# Sources folder
add_subdirectory(src)
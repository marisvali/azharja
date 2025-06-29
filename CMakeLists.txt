cmake_minimum_required(VERSION 3.16)

project(azharja VERSION 2.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile commands database (needed by clangd).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Network Core5Compat)
qt_standard_project_setup()

# Create a target called ScintillaEdit that just uses some .h, .lib and .dll files. This target can then be used by my other targets like any other CMake target.
add_library(ScintillaEdit SHARED IMPORTED)
set_target_properties(ScintillaEdit PROPERTIES
        IMPORTED_IMPLIB_DEBUG "${CMAKE_SOURCE_DIR}/scintilla/bin/debug/ScintillaEdit5.lib"
        IMPORTED_LOCATION_DEBUG "${CMAKE_SOURCE_DIR}/scintilla/bin/debug/ScintillaEdit5.dll"
        IMPORTED_IMPLIB_RELEASE "${CMAKE_SOURCE_DIR}/scintilla/bin/release/ScintillaEdit5.lib"
        IMPORTED_LOCATION_RELEASE "${CMAKE_SOURCE_DIR}/scintilla/bin/release/ScintillaEdit5.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/scintilla/include"
)

# Sources folder
add_subdirectory(src)
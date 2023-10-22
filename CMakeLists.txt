cmake_minimum_required(VERSION 3.16)

project(azharja VERSION 2.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Network Core5Compat)
qt_standard_project_setup()

add_library(ScintillaEdit STATIC IMPORTED)
set_target_properties(ScintillaEdit PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/scintilla/lib/ScintillaEdit.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/scintilla/include"
)

# Sources folder
add_subdirectory(src)
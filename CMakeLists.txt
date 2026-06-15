cmake_minimum_required(VERSION 3.21)

project(RightClickWslPath
    VERSION 0.1.0
    DESCRIPTION "Copy as WSL path - Explorer context-menu entry"
    LANGUAGES CXX)

# Windows-only project: COM shell extension + sparse MSIX + Win32 clipboard.
if(NOT WIN32)
    message(FATAL_ERROR "RightClickWslPath targets Windows only (MSVC + Windows 10 SDK).")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Target Windows 10 2004 (10.0.19041) for IExplorerCommand + sparse-package APIs.
add_compile_definitions(
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00
)

# ---- core: shared static library (no UI, no COM) ----
add_library(core STATIC
    src/core/PathConverter.cpp
    src/core/WslProcess.cpp
    src/core/Clipboard.cpp
)
target_include_directories(core PUBLIC src/core)

# ---- handler: modern Win11 IExplorerCommand COM DLL ----
add_library(handler SHARED
    src/handler/dllmain.cpp
    src/handler/ExplorerCommand.cpp
    src/handler/handler.def
)
set_target_properties(handler PROPERTIES OUTPUT_NAME "WslPathHandler")
target_link_libraries(handler PRIVATE core shlwapi ole32 runtimeobject)

# ---- cli: classic Win10 helper exe (GUI subsystem => no console flash) ----
add_executable(cli WIN32 src/cli/main.cpp)
set_target_properties(cli PROPERTIES OUTPUT_NAME "wslpathcopy")
target_link_libraries(cli PRIVATE core shlwapi shell32)

# ---- tests (optional; built when BUILD_TESTING is ON) ----
include(CTest)
if(BUILD_TESTING)
    add_executable(core_tests test/core_tests.cpp)
    target_link_libraries(core_tests PRIVATE core)
    add_test(NAME core_tests COMMAND core_tests)
endif()

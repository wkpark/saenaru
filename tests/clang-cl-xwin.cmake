# Cross build for Windows on Linux with clang-cl and the MSVC CRT/SDK of xwin.
#
#   apt install clang lld llvm wine
#   xwin --accept-license splat --output ~/.xwin   # accepts the Microsoft license
#
# XWIN_DIR (default ~/.xwin) and WINE (default wine) can be set in the environment.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)
set(CMAKE_LINKER lld-link)
set(CMAKE_RC_COMPILER llvm-rc)
set(CMAKE_MT llvm-mt)

if(DEFINED ENV{XWIN_DIR})
    set(XWIN_DIR "$ENV{XWIN_DIR}")
else()
    set(XWIN_DIR "$ENV{HOME}/.xwin")
endif()

set(_xwin_include "/imsvc${XWIN_DIR}/crt/include /imsvc${XWIN_DIR}/sdk/include/ucrt /imsvc${XWIN_DIR}/sdk/include/um /imsvc${XWIN_DIR}/sdk/include/shared")
set(CMAKE_C_FLAGS_INIT "${_xwin_include}")
set(CMAKE_CXX_FLAGS_INIT "${_xwin_include}")

set(_xwin_lib "/libpath:${XWIN_DIR}/crt/lib/x86_64 /libpath:${XWIN_DIR}/sdk/lib/um/x86_64 /libpath:${XWIN_DIR}/sdk/lib/ucrt/x86_64")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${_xwin_lib}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_xwin_lib}")

# ctest runs the tests with wine
if(DEFINED ENV{WINE})
    set(CMAKE_CROSSCOMPILING_EMULATOR "$ENV{WINE}")
else()
    set(CMAKE_CROSSCOMPILING_EMULATOR wine)
endif()

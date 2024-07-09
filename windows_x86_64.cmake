set(CMAKE_CROSSCOMPILING TRUE)
SET(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Clang target triple
SET(TARGET x86_64-pc-win32-coff)

SET(CMAKE_C_COMPILER_TARGET ${TARGET})
SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER_TARGET ${TARGET})
SET(CMAKE_CXX_COMPILER clang++)
SET(CMAKE_ASM_COMPILER_TARGET ${TARGET})
SET(CMAKE_ASM_COMPILER clang)
SET(USE_GAS FALSE)
SET(CMAKE_BUILD_TYPE Release)
#LINK_DIRECTORIES(/Users/vaylor27/.xwin-cache/unpack/Win11SDK_10.0.26100_store_libs.msi/lib/um/x64 /Users/vaylor27/.xwin-cache/unpack/Microsoft.VC.14.40.17.10.CRT.x64.Store.base.vsix/lib/x64/uwp /Users/vaylor27/.xwin-cache/unpack/Win11SDK_10.0.26100_libs_x86_64.msi/lib/um/x64 /Users/vaylor27/.xwin-cache/unpack/ucrt.msi/lib/ucrt/x64)
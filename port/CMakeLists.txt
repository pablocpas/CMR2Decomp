cmake_minimum_required(VERSION 3.24)
project(OpenCMR2 LANGUAGES C CXX)

# The reference target is 32-bit x86 (preset linux-x86, toolchain
# cmake/linux-i686.cmake): the game's data structures then have the
# original's layout, which the golden tests compare against. The 64-bit build
# comes once every structure is pointer-size clean.
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(WARNING "64-bit build: the game code is not pointer-size clean yet")
endif()

option(OPENCMR2_SANITIZE "Build with AddressSanitizer and UBSan" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Semantics the decompiled code relies on:
#  - signed overflow wraps (16.16 fixed point overflows on purpose);
#  - type punning through casts (the code was written against MSVC6);
#  - explicit float stores use single precision, doubles use double precision;
#    the original's x87 nearest-even integer conversions are written explicitly
#    at audited physics sites (ordinary C++ casts would truncate).
add_compile_options(-fwrapv -fno-strict-aliasing)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86|i[3-6]86|AMD64")
    add_compile_options(-msse2 -mfpmath=sse)
endif()

if(OPENCMR2_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3-shared)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)

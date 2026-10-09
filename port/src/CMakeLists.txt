set(GAME_DIR ${PROJECT_SOURCE_DIR}/game)

# The decompiled game. zlib is C, as in the original (ZlibZutil.cpp is C++).
file(GLOB GAME_SOURCES CONFIGURE_DEPENDS ${GAME_DIR}/*.cpp)
file(GLOB ZLIB_C_SOURCES CONFIGURE_DEPENDS ${GAME_DIR}/Zlib*.cpp)
list(REMOVE_ITEM ZLIB_C_SOURCES ${GAME_DIR}/ZlibZutil.cpp)
set_source_files_properties(${ZLIB_C_SOURCES} PROPERTIES LANGUAGE C)

add_library(cmr2_game STATIC ${GAME_SOURCES})
target_include_directories(cmr2_game PUBLIC ${GAME_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(cmr2_game PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-x c>
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/port/types.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/port/msvcrt.h"
    -fms-extensions
    -Wno-everything
    -ferror-limit=0)
target_link_libraries(cmr2_game PUBLIC SDL3::SDL3)

# Platform layer and the executable.
add_library(opencmr2_platform STATIC
    platform/sys.cpp
    platform/keys.cpp
    platform/msvcrt.cpp)
target_include_directories(opencmr2_platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${GAME_DIR})
target_link_libraries(opencmr2_platform PUBLIC SDL3::SDL3)

add_executable(opencmr2 platform/main.cpp)
target_link_libraries(opencmr2 PRIVATE cmr2_game opencmr2_platform)

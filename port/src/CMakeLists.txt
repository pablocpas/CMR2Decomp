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
    -include ${CMAKE_CURRENT_SOURCE_DIR}/port/types.h
    -fms-extensions
    -Wno-everything
    -ferror-limit=0)
target_link_libraries(cmr2_game PUBLIC SDL3::SDL3)

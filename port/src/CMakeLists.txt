set(GAME_DIR ${PROJECT_SOURCE_DIR}/game)

# Small standalone decoder, also usable by tests without SDL or game data.
add_library(opencmr2_bink STATIC video/bink_decoder.cpp)
target_include_directories(opencmr2_bink PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

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
target_sources(opencmr2_platform PRIVATE diagnostics/observer.cpp diagnostics/timing.cpp)
target_include_directories(opencmr2_platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${GAME_DIR})
target_link_libraries(opencmr2_platform PUBLIC SDL3::SDL3)

add_executable(opencmr2 platform/main.cpp)
# The game and the platform layer call each other.
target_link_libraries(opencmr2 PRIVATE -Wl,--start-group cmr2_game opencmr2_platform -Wl,--end-group)

# ---- shaders (GLSL -> SPIR-V, embedded) -------------------------------------------
find_program(GLSLANG_VALIDATOR glslangValidator REQUIRED)
set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/render/shaders)
set(SHADER_SOURCES fixed3d.vert fixedtl.vert fixed.frag clear.vert clear.frag)
set(SPIRV_FILES)
foreach(shader IN LISTS SHADER_SOURCES)
    set(output ${CMAKE_CURRENT_BINARY_DIR}/shaders/${shader}.spv)
    add_custom_command(
        OUTPUT ${output}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders
        COMMAND ${GLSLANG_VALIDATOR} -V --target-env vulkan1.0 -o ${output} ${SHADER_DIR}/${shader}
        DEPENDS ${SHADER_DIR}/${shader} ${SHADER_DIR}/common.glsl
        COMMENT "Compiling shader ${shader}")
    list(APPEND SPIRV_FILES ${output})
endforeach()
set(SHADER_HEADER ${CMAKE_CURRENT_BINARY_DIR}/generated/shaders_spirv.h)
string(REPLACE ";" "|" SPIRV_LIST "${SPIRV_FILES}")
add_custom_command(
    OUTPUT ${SHADER_HEADER}
    COMMAND ${CMAKE_COMMAND} -DOUTPUT=${SHADER_HEADER} -DINPUTS=${SPIRV_LIST} -P ${PROJECT_SOURCE_DIR}/cmake/embed_spirv.cmake
    VERBATIM
    DEPENDS ${SPIRV_FILES} ${PROJECT_SOURCE_DIR}/cmake/embed_spirv.cmake
    COMMENT "Embedding shaders")

# ---- platform subsystems ---------------------------------------------------------
target_sources(opencmr2_platform PRIVATE
    render/gfx_core.cpp
    render/gfx_gpu.cpp
    ${SHADER_HEADER}
    audio/audio_sdl.cpp
    input/input_sdl.cpp
    video/movie.cpp
    net/net_none.cpp)
target_include_directories(opencmr2_platform PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_link_libraries(opencmr2_platform PRIVATE opencmr2_bink)

# Portable fixed-point helpers vs the original inline assembly. Needs a
# 32-bit x86 build: clang compiles the MSVC-style assembly with -fasm-blocks.
if(CMAKE_SIZEOF_VOID_P EQUAL 4 AND CMAKE_SYSTEM_PROCESSOR MATCHES "i[3-6]86|x86")
    add_executable(fixedpoint_equiv fixedpoint_equiv.cpp)
    target_include_directories(fixedpoint_equiv PRIVATE ${PROJECT_SOURCE_DIR}/game ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(fixedpoint_equiv PRIVATE
        -include ${PROJECT_SOURCE_DIR}/src/port/types.h -fms-extensions -fasm-blocks -Wno-everything)
    add_test(NAME fixedpoint_equiv COMMAND fixedpoint_equiv)
endif()

add_executable(net_session_test net_session.cpp)
target_link_libraries(net_session_test PRIVATE opencmr2_net)
target_compile_options(net_session_test PRIVATE "SHELL:-include ${PROJECT_SOURCE_DIR}/src/port/types.h")
add_test(NAME net_session COMMAND net_session_test)
set_tests_properties(net_session PROPERTIES TIMEOUT 60)

add_executable(bink_decoder_test bink_decoder.cpp)
target_link_libraries(bink_decoder_test PRIVATE opencmr2_bink)
add_test(NAME bink_decoder COMMAND bink_decoder_test)

add_executable(movie_playback_test movie_playback.cpp ${PROJECT_SOURCE_DIR}/src/video/movie.cpp)
target_link_libraries(movie_playback_test PRIVATE opencmr2_bink)
add_test(NAME movie_playback COMMAND movie_playback_test)

add_executable(movie_render_state_test movie_render_state.cpp
    ${PROJECT_SOURCE_DIR}/src/diagnostics/observer.cpp
    ${PROJECT_SOURCE_DIR}/src/video/movie.cpp ${PROJECT_SOURCE_DIR}/src/render/gfx_core.cpp)
target_link_libraries(movie_render_state_test PRIVATE opencmr2_bink SDL3::SDL3)
add_test(NAME movie_render_state COMMAND movie_render_state_test)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_executable(math_tables_test math_tables.cpp)
    target_link_libraries(math_tables_test PRIVATE
        -Wl,--start-group cmr2_game opencmr2_platform -Wl,--end-group)
    add_test(NAME math_tables COMMAND math_tables_test
        ${CMAKE_CURRENT_SOURCE_DIR}/reference/finland-stage1/math-tables.bin)
    add_executable(car_scheduler_test car_scheduler.cpp)
    target_link_libraries(car_scheduler_test PRIVATE
        -Wl,--start-group cmr2_game opencmr2_platform -Wl,--end-group)
    add_test(NAME car_scheduler COMMAND car_scheduler_test)
    add_executable(stage_probe stage_probe.cpp)
    target_link_libraries(stage_probe PRIVATE
        -Wl,--start-group cmr2_game opencmr2_platform -Wl,--end-group)
    add_executable(preview_cleanup_test preview_cleanup.cpp)
    target_include_directories(preview_cleanup_test PRIVATE ${PROJECT_SOURCE_DIR}/game)
    target_link_libraries(preview_cleanup_test PRIVATE
        -Wl,--start-group cmr2_game opencmr2_platform -Wl,--end-group)
    add_test(NAME preview_cleanup COMMAND preview_cleanup_test)
endif()

add_executable(bink_decode bink_decode.cpp)
target_link_libraries(bink_decode PRIVATE opencmr2_bink)

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_test(NAME reference_capture_format
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_capture.py)
endif()

set(OPENCMR2_BINK_TEST_DATA_DIR "" CACHE PATH "Optional original game data for movie and physics regression tests")
if(OPENCMR2_BINK_TEST_DATA_DIR)
    if(Python3_Interpreter_FOUND AND TARGET stage_probe)
        add_test(NAME physics_original_parity COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/physics_parity.py --probe $<TARGET_FILE:stage_probe>
            --data ${OPENCMR2_BINK_TEST_DATA_DIR})
    endif()
    file(GLOB_RECURSE BINK_TEST_FILES "${OPENCMR2_BINK_TEST_DATA_DIR}/*.bik" "${OPENCMR2_BINK_TEST_DATA_DIR}/*.BIK")
    foreach(movie IN LISTS BINK_TEST_FILES)
        get_filename_component(name "${movie}" NAME_WE)
        add_test(NAME "bink_file_${name}" COMMAND bink_decode "${movie}")
    endforeach()
endif()

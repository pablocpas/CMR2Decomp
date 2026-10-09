# Portable fixed-point helpers vs the original inline assembly. Needs a
# 32-bit x86 build: clang compiles the MSVC-style assembly with -fasm-blocks.
if(CMAKE_SIZEOF_VOID_P EQUAL 4 AND CMAKE_SYSTEM_PROCESSOR MATCHES "i[3-6]86|x86")
    add_executable(fixedpoint_equiv fixedpoint_equiv.cpp)
    target_include_directories(fixedpoint_equiv PRIVATE ${PROJECT_SOURCE_DIR}/game ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(fixedpoint_equiv PRIVATE
        -include ${PROJECT_SOURCE_DIR}/src/port/types.h -fms-extensions -fasm-blocks -Wno-everything)
    add_test(NAME fixedpoint_equiv COMMAND fixedpoint_equiv)
endif()

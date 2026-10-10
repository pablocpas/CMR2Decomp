# Writes a C++ header with the given SPIR-V files as byte arrays:
#   cmake -DOUTPUT=out.h -DINPUTS="a.spv|b.spv" -P embed_spirv.cmake
# Each array is named g_<file name with dots turned into underscores>,
# without the .spv extension.
string(REPLACE "|" ";" INPUTS "${INPUTS}")
set(content "// Generated from the SPIR-V shaders. Do not edit.\n#pragma once\n\n")
foreach(input IN LISTS INPUTS)
    get_filename_component(name "${input}" NAME)
    string(REGEX REPLACE "\\.spv$" "" name "${name}")
    string(REPLACE "." "_" name "${name}")
    file(READ "${input}" hex HEX)
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${hex}")
    string(APPEND content "static const unsigned char g_${name}[] = {${bytes}};\n\n")
endforeach()
file(WRITE "${OUTPUT}" "${content}")

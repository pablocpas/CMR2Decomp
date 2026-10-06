#!/usr/bin/env python3
"""Run a reccmp tool with the fixes this project's executables need.

  reccmp_compat.py reccmp --target CMR2 ...     (same as reccmp-reccmp)
  reccmp_compat.py datacmp --target CMR2        (same as reccmp-datacmp)

Both the original CMR2.exe and the rebuilt one are linked without a .reloc
section. reccmp only accepts a `jmp dword ptr [__imp_X]` stub as an import
thunk when the relocation table lists its operand, so without relocations no
import thunk is matched and every call to rand, strncmp, malloc, ... stays an
unresolved symbol. When an image has no relocation table, accept every
absolute jump whose operand is an import address instead.

Import this module (or call `apply()`) before building a reccmp Compare.
"""
import sys

from reccmp.analysis import imports as _imports
from reccmp.analysis.imports import ImportThunk, find_absolute_jumps_in_bytes
import reccmp.compare.analyze as _analyze


def find_import_thunks(image):
    import_addrs = set(imp.addr for imp in image.imports)
    if not import_addrs:
        return
    relocations = image.relocations
    for region in image.get_code_regions():
        for addr, jmp_dest in find_absolute_jumps_in_bytes(region.data, region.addr):
            if relocations and addr + 2 not in relocations:
                continue
            if jmp_dest in import_addrs:
                yield ImportThunk(addr, jmp_dest, 6)


def apply():
    _imports.find_import_thunks = find_import_thunks
    _analyze.find_import_thunks = find_import_thunks


apply()

TOOLS = {"reccmp": "reccmp.tools.asmcmp", "datacmp": "reccmp.tools.datacmp",
         "verexp": "reccmp.tools.verexp", "vtable": "reccmp.tools.vtable"}

if __name__ == "__main__":
    if len(sys.argv) < 2 or sys.argv[1] not in TOOLS:
        sys.exit("usage: reccmp_compat.py {" + ",".join(TOOLS) + "} [reccmp args]")
    import importlib
    tool = sys.argv.pop(1)
    sys.argv[0] = "reccmp-" + tool
    sys.exit(importlib.import_module(TOOLS[tool]).main())

// Temporary scaffolding so the boot-state functions (0x4d0xxx / 0x50xxxx) can be
// developed and measured before their dependencies exist. Every entry is a real
// function in the reference exe with its stdcall argument count; the bodies are
// empty. DELETE each one as its real implementation lands (layers 0-8 of
// tools/cascade/README.md).



// STUB: CMR2 0x005062d0
void FUN_005062d0(unsigned int) { }

// --- scaffolding for the FUN_0041b060 entry chain (0x401000-0x472e00) -------
// Same idea as above: the game-state machine and its callees are written before
// their own dependencies exist. Argument counts come from the call sites /
// the original's `ret N`.

// STUB: CMR2 0x00416710
void FUN_00416710(void) { }







struct Menu;








struct Menu;

// Scaffolding for the FrontendScreens batch: callees that do not exist yet.
// FUN_004dbd80 (0x004dbd80) is implemented in FrontendScreens.cpp.

// STUB: CMR2 0x004f8b30
void FUN_004f8b30(void) { }



struct FixMatrix;

// TEMPORARY (W171 GameInfo batch): these are real batch functions that other
// agents are implementing in parallel. The stubs only exist so the batch links




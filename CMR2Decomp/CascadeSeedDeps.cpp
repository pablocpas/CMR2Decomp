// Temporary scaffolding so the boot-state functions (0x4d0xxx / 0x50xxxx) can be
// developed and measured before their dependencies exist. Every entry is a real
// function in the reference exe with its stdcall argument count; the bodies are
// empty. DELETE each one as its real implementation lands (layers 0-8 of
// tools/cascade/README.md).

// STUB: CMR2 0x0040dc30
void FUN_0040dc30(void) { }

// STUB: CMR2 0x0049d3f0
void FUN_0049d3f0(int, int, void *, int, int) { }

// STUB: CMR2 0x004b9380
void FUN_004b9380(unsigned int, unsigned int, unsigned int) { }



// STUB: CMR2 0x005062d0
void FUN_005062d0(unsigned int) { }

// --- scaffolding for the FUN_0041b060 entry chain (0x401000-0x472e00) -------
// Same idea as above: the game-state machine and its callees are written before
// their own dependencies exist. Argument counts come from the call sites /
// the original's `ret N`.

// STUB: CMR2 0x0040f8d0
void FUN_0040f8d0(unsigned char *, int) { }

// STUB: CMR2 0x0040fec0
void FUN_0040fec0(int, int, int) { }

// STUB: CMR2 0x00412390
void FUN_00412390(int, int) { }


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

// STUB: CMR2 0x00416710
void FUN_00416710(void) { }

// STUB: CMR2 0x004188c0
void FUN_004188c0(void) { }

// STUB: CMR2 0x00418ff0
void FUN_00418ff0(void) { }

// STUB: CMR2 0x0041f930
unsigned char FUN_0041f930(void) { return 0; }

// STUB: CMR2 0x0040a580
void FUN_0040a580(int, int, int) { }

// STUB: CMR2 0x0040efa0
void FUN_0040efa0(void) { }

// STUB: CMR2 0x0041f560
void FUN_0041f560(void) { }

// STUB: CMR2 0x00455080
void FUN_00455080(void) { }

// STUB: CMR2 0x00475f00
void FUN_00475f00(void) { }

// STUB: CMR2 0x00403500
void FUN_00403500(void) { }

// STUB: CMR2 0x00422fe0
void FUN_00422fe0(int, int, int, int) { }

// STUB: CMR2 0x00424c50
void FUN_00424c50(void) { }

// STUB: CMR2 0x0041c5a0
void FUN_0041c5a0(unsigned char, int) { }

// STUB: CMR2 0x0041e6b0
void FUN_0041e6b0(int, int, int) { }

// STUB: CMR2 0x0046cce0
void FUN_0046cce0(int, int, int, int) { }

// STUB: CMR2 0x00420150
unsigned char FUN_00420150(void) { return 0; }


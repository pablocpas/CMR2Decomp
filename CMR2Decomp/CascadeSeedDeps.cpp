// Temporary scaffolding so the boot-state functions (0x4d0xxx / 0x50xxxx) can be
// developed and measured before their dependencies exist. Every entry is a real
// function in the reference exe with its stdcall argument count; the bodies are
// empty. DELETE each one as its real implementation lands (layers 0-8 of
// tools/cascade/README.md).

// STUB: CMR2 0x0049d3f0
void FUN_0049d3f0(int, int, void *, int, int) { }



// STUB: CMR2 0x005062d0
void FUN_005062d0(unsigned int) { }

// --- scaffolding for the FUN_0041b060 entry chain (0x401000-0x472e00) -------
// Same idea as above: the game-state machine and its callees are written before
// their own dependencies exist. Argument counts come from the call sites /
// the original's `ret N`.

// STUB: CMR2 0x00416710
void FUN_00416710(void) { }

// STUB: CMR2 0x00420630
void FUN_00420630(void) { }

// STUB: CMR2 0x00455080
void FUN_00455080(void) { }


// STUB: CMR2 0x00424c50
void FUN_00424c50(void) { }

// STUB: CMR2 0x0041c5a0
void FUN_0041c5a0(unsigned char, int) { }


// STUB: CMR2 0x0041b460
void FUN_0041b460(void) { }

// STUB: CMR2 0x00422140
void FUN_00422140(unsigned char, int) { }

// Callees of 0x0042baf0 (Car.cpp batch) that no module implements yet; they
// only keep the batch linkable. DELETE each one when its real body lands.
// STUB: CMR2 0x004877a0
void FUN_004877a0(int, short *, short) { }
// STUB: CMR2 0x00480bb0
void FUN_00480bb0(int, short *, short) { }
// STUB: CMR2 0x0046d510
void FUN_0046d510(void) { }
// STUB: CMR2 0x0046b4e0
void FUN_0046b4e0(int) { }

// STUB: CMR2 0x0044a1b0
void FUN_0044a1b0(int) { }

// STUB: CMR2 0x00455470
void FUN_00455470(int) { }

// STUB: CMR2 0x00472a30
void FUN_00472a30(void) { }


// STUB: CMR2 0x00478c40
void FUN_00478c40(void) { }

struct Menu;
// STUB: CMR2 0x0049bcb0
void FUN_0049bcb0(Menu *pMenu) { }

// STUB: CMR2 0x00473d60
void FUN_00473d60(Menu *pMenu) { }

// Scaffolding for the FrontendScreens batch: callees that do not exist yet.
// STUB: CMR2 0x004dbd80
void FUN_004dbd80(Menu *pMenu) { }

// STUB: CMR2 0x004f8b30
void FUN_004f8b30(void) { }



struct FixMatrix;

// TEMPORARY (W171 GameInfo batch): these are real batch functions that other
// agents are implementing in parallel. The stubs only exist so the batch links


# Raw accesses that change meaning on 64-bit, batch 1

Baseline: `340cb33`. All 3364 functions keep their bytes (2924 exact). Found
with OpenCMR2's `tools/audit_raw_access.py`, which resolves each raw access
against the i386 and x86-64 layouts and reports the ones that touch different
fields.

## Real 64-bit defects fixed

| Function(s) | Was | Now | On 64-bit before |
| --- | --- | --- | --- |
| Car_ShatterWindow, Car_SpawnDebris, WheelSpray/WheelSplash_Update, StageTiming_SpawnWheelParticles, CarEffect_UpdateExhaustSmokeAndBackfire | `*((BYTE *)pBodyNode + 0x17c)` | `pBodyNode->viewMask` | read `cameraDistance` |
| FixMatrix_ProjectWorldPointToView | `(FixMatrix *)(pView + 0xd8)`, `m_pTextureManager + 0x23e4` | `&pView->world`, `->fixedProjection` (pView is `SceneNode *` at every caller) | wrong matrix, wrong projection |
| StageObject_UpdateProjectedDistanceFade | `pView + 0x98` | `&pView->current` | wrong matrix |
| StageObject_UpdateMovingTransforms (FILL_PROBE_TABLE) | `pObj + 0x104..0x10c` | `pObj->probe` | read `field_0xf8` |
| StageObject_AssignMatchingCarClassSlots | `pObject[0x30]` | `pObject->flags` (Car *, SceneNode *) | read `angles.z` |
| ForceFeedback_* (g_unk0x0053937c, now `Car *`) | `+0x778`, `+0xb74`, `+0xbac`, `+0x4a4`, `+0x1a8 + i * 0xc`... | `speed`, `field_0xb74`, `cornerOnGround[]`, `cornerAxis[]`, `wheelSurfaceFx[i]`, `cornerGrip[i]` | read pointers and unrelated fields |
| CarPart_InitWheelHubSlot, StageTiming_IntegratePartSpringMotion | `g_partSet + 0x248/0x24c/0x288` | `damageValues[2]/[3]/[18]` | read `halfExtents` |
| StageTiming_LoadSelectedCarModel | `*(int *)&pCar->pAlternateBodyNode = 0` | `= NULL` | cleared half of each pointer |
| Car_BuildViewOrder, AutoGear_ApplyShiftBodyNudge | player record `+4/+0xe8/+0x10c` | `Race_GetPlayerRecordPointer` returns `ReplayStream *` (`playing`, `field_0xe8`, `field_0x10c`) | read other replay fields |
| Interpolation timers (Timer_GetValue, Graphics_StartShapedInterpolationTimer, Graphics_IsRegisteredTimerRunning, reset/stop/find) | `BYTE timers[32][0x30]` with the owner pointer at +0x20 | `InterpolationTimer` (0x30, LayoutChecks) | the 8-byte owner overwrote `delay` at +0x24 |
| In-race action menu | `BYTE g_unk0x0058cf80[0x100]` used as a `Menu` (0x1e0) | `Menu` | overflowed into the next global, on 32-bit ports too (the original reserves 0x1e0: next global 0x58d260) |
| Pause menus | `BYTE g_unk0x0058ca90[0x4d4]` with two `Menu`s at 0 and 0x1e8 | `PauseMenus` (PauseMenus.h) | the second menu overlapped the first (Menu grows to 0x308) |

## Wrong declared types (correct on 64-bit by luck, now typed)

- `g_collisionFace` and `g_pContacts0x005915e0/00591394` are `CollisionBox *`
  (`&g_unk0x00590ed0[i]`); `CollisionFaceVertices` was a 0x90-byte prefix of
  it. `CollisionBox::pad_0x28` is `built` (+0x28) and `hasSecondaryVertices` (+0x2c).
- `g_unk0x00831088[16]` is `GenericFile` (buffer, fileSize, didFileLoad), not
  three pointers.
- Option archives: getters return `GenericFile *` (storage stays `int[4]`, see
  the comment there: typing the arrays costs bytes in that TU).
- Sky/ground c3d data from `FindFile` is `void *`, not `GenericFile *`.

## Neighbour effects avoided

`PauseMenus` lives in its own header included by StageUI.cpp and
StageObjects.cpp only. `Graphics_ResetInterpolationTimers` keeps its `INT_PTR`
comparison (the original's `jl`).

## Harnesses

All `tests/check_64bit_*.py` compile with `FloatToInt.h`. Three already
failed before this batch after earlier prototype changes (address loops,
race-state pointers, route nodes); their stubs now follow the typed prototypes.

## Stage lighting records (follow-up)

`LightZone` (0x14), `LightZoneItem` (0x30) and `LightZoneMesh` (0x14) in
`LightZone.h` describe the lighting data Scene_LoadLighting relocates in place:
each holds 32-bit file offsets turned into pointers, so on 64-bit they need
native copies (OpenCMR2 does that in its Scene_LoadLighting). The consumers
(Scene_RelightSector, Scene_AttenuateSectorLight, Scene_CollectNearbyLightZones,
Scene_ProjectCasterIntoLightZone) read the fields; the int walks over items
step `sizeof(LightZoneItem) / sizeof(int)` (12 on the original). The item's
owner is a `StageObject`: the caster projection read its position, matrix and
mesh at raw offsets (pMesh at +0xc is +0x10 on 64-bit). Moving the types out of
SceneNode.h also made FrontendText_LoadRegionLanguages exact (2925).

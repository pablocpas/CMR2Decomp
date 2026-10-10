# Objetos del escenario: punteros con sus tipos y nombres

Tanda sobre `ee06f54`, siguiendo `CODEX-64BIT.md`. Dos commits: `bbd6216`
cambia tipos y forma; el siguiente solo pone nombre a lo que esa tanda dejó
claro (como pide `CLAUDE.md`, no se renombra a la vez que se iguala código).

## Corrección de comportamiento

`CarDamage_BuildRelativeVelocityHull` (50 %) recorría el casco con un puntero y
`(int)pOut < (int)&g_stageDeformHull[10]`. Escrita con un índice da 98.81 %, y
la única diferencia que queda es un salto invertido: el original eleva los
vértices 4 y 5 con `field_0x770[1]` y los 6 y 7 con `field_0x770[0]`. El código
descompilado lo tenía al revés. Con el orden del original la función es
**byte-exacta**. OpenCMR2 hereda el error hasta que se sincronice.

## Tipos

| Antes | Ahora | Evidencia |
| --- | --- | --- |
| `AI_SelectWallCollisionResponse(int)` con `*(int *)(param_1 + off)` | `(int *pState)` con `pState[i]` | Recibe `g_unk0x0058e178` (`int[0x2e]`), que los demás lectores ya indexan como `int *` |
| `StageObject_RebuildCarLightMeshes(int)`, `StageObject_UpdateCarLightFlagsAndGlows(int)`, `StageObject_SetModelSubmeshVisibility(int, ...)`, `StageTiming_RebuildDamagedPartMeshes(int)`, `CarInterior_UpdateSteeringBlendFraction(int, int)` | `Car *` | Todos reciben un coche; `+0xb1a` es `index`, `+0x720` `pBodyNode`, `+0x48c` `groundNormal` |
| `StageObject_GetCurrentObject{Pointer,Context,Values}(int *...)` | `SceneNode **` | Devuelven `g_stageSkyNode`, `g_stageGroundNode`, `g_stageCloudNode`/`g_stageCloudTopNode` |
| Sus usuarios: `(FixMatrix *)(obj + 0x98)`, `*(int **)(obj + 4)`, `*node`, `node + 4`, `node + 7`, `node[3]` | `&obj->current`, `pFirstChild`, `pNext`, `&translation`, `&angles`, `pObject` | Campos de `SceneNode` |
| `StageObjectEntry0x128::pObject` (`int *`, en sus dos copias) | `SceneNode *` | Todas sus lecturas lo convertían; `[0xcc / 4]` es `current.position.y`, `+8` `pParent`, `+0x174` `dirty` |
| `int node = (int)SceneNode_FindByType(...)`, `*(BYTE *)(node + 0x17c)` | `SceneNode *node`, `node->viewMask` | |
| `Replay_Init{ObjectList,MeshList}CarState(int, BYTE)` | `(BYTE *pLane, BYTE)` | Reciben `p->pInputs + lane * 0x114c` / `p->pStates + ...` |
| `StageObject_DispatchActiveCarObjectUpdate(BYTE *, int, int)` | `(BYTE *, FixMatrix *pRef, int)` | Recibe `Car_GetCameraReferenceMatrix` |
| `StageObject_BuildCarNodeOrientation(int, int *)` | `(BYTE *object, int *)` | Registro del objeto montado |
| `StageObject_BuildTypeTableRowOutput(int, int, int *)` | `(int, int *pControls, int *)` | Recibe los cinco canales de control |
| `Glow_Add(..., int unused1, ..., int unused2, ...)` | `FixVector *` | Las cuatro llamadas pasan direcciones de `FixVector` |
| `StageObject_SetContactLevelToUnity(BYTE *, int)`, `StageObject_BuildCarMountWorldMatrix(BYTE *, int)` | segundo argumento `void *` | No se usa y los llamadores pasan punteros de tipos distintos |

## Bucles

Los recorridos `(int)p < (int)&tabla[N]` se escriben con índice donde MSVC6
genera lo mismo: objetos móviles (dos), registros de faros, sonidos del menú,
valores de objeto y los cinco recorridos de `g_replaySlots`
(`Replay_AdvanceLiveRecordingStreams` 81.4 % -> 84.1 %).

Dos funciones exactas no admiten todavía la forma con índice:
`StageObject_ResetPairedCarValues` (el original reutiliza el `eax` a cero del
`memset`) y `CarSkid_ClearWheelTrails` (el original prepara el puntero de fila
antes de los `memset`). Siguen comparando `(int)p < (int)fin`; se quedan como
deuda en lugar de usar `INT_PTR`.

## Nombres (segundo commit)

| Antes | Ahora | Evidencia |
| --- | --- | --- |
| `StageObject_GetCurrentObjectPointer` | `StageObject_GetSkyNode` | Devuelve `g_stageSkyNode` |
| `StageObject_GetCurrentObjectContext` | `StageObject_GetGroundNode` | Devuelve `g_stageGroundNode` |
| `StageObject_GetCurrentObjectValues` | `StageObject_GetCloudNodes` | Devuelve los dos nodos de nubes |
| `g_unk0x0058e178` | `g_aiDriveState` | Estado del coche de la CPU que `AI_UpdateRouteDrivingControls` rellena y consulta |
| `g_unk0x00588d40` | `g_replaySlots` | Tabla de dieciséis punteros a las ranuras de los dos juegos de búferes de repetición |
| `g_unk0x00588e80` | `g_replayStreams` | Sus entradas se leen como `ReplayStream *` |
| `g_unk0x0058c92c`, `g_unk0x0058c928` | `g_movingObjectMeshes`, `g_movingObjectMeshIndex` | Mallas clonadas para los objetos móviles y el índice de cada variante |
| `StageObjectEntry0x128` | `MovingObject` | Entrada de `g_movingObjects` |

Las funciones renombradas quedan en `scripts/renames/004-64bit-batches.tsv`.
Tras el renombrado, `CMR2PROGRESS/bytes.json` es idéntico al anterior.

## Deuda

- `StageObject` y los datos de variantes (`pEntries + i * 8`,
  `MovingObject::field_0x0`) son registros del fichero de escenario con
  punteros reubicados en sitio; necesitan traducción.
- `StageObjectEntry0x128`/`MovingObject` está definido dos veces
  (`FixedPoint.cpp` y `StageObjects.cpp`); debería vivir en una cabecera.

## Exactitud y pruebas

- `match.py --changed`: sin regresiones.
- Build y medida completos y `prepare_fastcmp.py`: **2924** funciones
  byte-exactas (+1, `CarDamage_BuildRelativeVelocityHull`); ninguna pierde
  puntuación.
- `run_differential_suite.py`: 116 arneses, 0 fallos.
- Inventario de accesos crudos: 1382 -> 1354.
- Conversiones puntero<->entero en x64 (medidas como en
  `64bit-shadow-casters.md`): 733 -> 593. `StageObjects.cpp` 191 -> 72,
  `TrackCollision.cpp` 39 -> 29.

# Raíces de escena y cámaras como `SceneNode *`

Tanda sobre `279ccb5`, siguiendo `CODEX-64BIT.md`. Las raíces de escena, las
cámaras y los rectángulos de vista viajaban como `int` entre los que construyen
una escena y `Game_PrepareScene`/`Game_DrawSceneViewport`. En 64 bits esos
`int` truncan direcciones. Todo es estado de ejecución, salvo la imagen C3D que
reubica `Sector_RelocateStageMeshFile` (ver «Deuda»).

## Evidencia y cambio

| Antes | Ahora | Evidencia |
| --- | --- | --- |
| `Game_DrawSceneViewport(int, int, void *, int, BYTE)` | `(SceneNode *pRoot, SceneNode *pCamera, void *pRect, ...)` | Toda llamada pasa una raíz y una cámara; el cuerpo convertía `param2` a `SceneNode *` en cada uso |
| `Game_PrepareScene(..., int unused, int)` | `(..., void *pRect, int)` | Todas las llamadas pasan la dirección de un rectángulo `short[4]` |
| `Sector_CullGridAroundViewNode(SceneNode *, int unused)` | `(SceneNode *, void *pRect)` | Recibe el mismo rectángulo |
| `Particle_DrawAll(int, BYTE)` | `(SceneNode *pCamera, BYTE)` | Recibe la cámara de `Game_DrawSceneViewport` |
| `g_unk0x00536be0`, `g_unk0x00536be4`, `g_unk0x0082b1b0`, `g_unk0x0082b1b4` (`int`) | `SceneNode *` | Solo reciben `SceneNode_CreateRoot`/`SceneType2_Create` y se usan como nodos |
| `RallyData_GetChallengeRenderState`, `RallyData_GetChallengeSceneState`, `OptionMenu_GetBackgroundRoot` → `int` | `SceneNode *` | Devuelven esos globales; los receptores los comparan con `pParent` o los pasan como padre |
| `*(int *)((BYTE *)pCar->pWheelNodes[w] + 8)` | `pCar->pWheelNodes[w]->pParent` | `SceneNode +0x8` |
| `Sector_BuildC3DModelScene(unsigned int, unsigned int, unsigned int)` | `SceneNode *(void *data, SceneNode *parent, GenericFile *archive)` | Devuelve la raíz reubicada y la cuelga de `parent` |
| `Graphics_LoadTextureRecordList(..., int)` | `(..., GenericFile *archive)` | Lo pasa a `CTexture::FindLoadTexture` |
| `OptionPreview_LoadStageGeometryRecord`: siete locales `int` | `BYTE *hC3D/hL/hS`, `SceneNode *stage/root/nodeL/nodeS`, `GenericFile *pRecord` | Buffers de fichero, nodos y el registro del archivo |
| `((int *)&pTriangles[k])[1]`, `((int *)pTriangles)[1]` | `pTriangles[k].textureIndex`, `pTriangles->textureIndex` | `MeshTriangle +0x4` indexa `D3DTextureManager::textureBuffer` |

`MeshTriangle` parte `field_0x2[0x2a]` en `field_0x2[2]`, `int textureIndex`
(`0x4`) y `field_0x8[0x24]`; `LayoutChecks.cpp` comprueba el offset.

## Lo que dice el compilador

- `FrontendMenu_DrawRallySummary` (no exacta, 98.56 %) cambia de orden dos
  recargas al final de un bucle (`count` y `pMenu`) según el número de símbolos
  del TU, sin que la función cambie. Cualquier símbolo nuevo lo invierte (un
  `extern` o un miembro de más en un struct de una cabecera incluida); el orden
  de declaración de los locales no influye. El campo nuevo de `MeshTriangle` lo
  invertía. Se recupera quitando de `FrontendScreens.cpp` el prototipo
  `SavedGames_ReleaseRecords`, que ese fichero no usa.
- `g_unk0x0059be6c = pCamera`, `FixMatrix_GetPosition(..., &pCamera->world)` y
  las llamadas a `Glow_Draw`/`Billboard_Draw` generan lo mismo que con los
  casts: el original ya tenía la cámara como `SceneNode *`.

## Deuda

- `Particle_DrawAll` sigue recorriendo las partículas con un cursor de bytes y
  pasa `(int)pCamera` al callback de `ParticleType::field0x5c`.
- `Sector_RelocateStageMeshFile` y los dos primeros parámetros de
  `Graphics_LoadTextureRecordList` trabajan sobre la imagen C3D del disco, con
  offsets de 32 bits. En 64 bits hay que traducirla, no reubicarla en sitio:
  queda para su propia tanda.
- `Mesh.cpp` lee `textureIndex` con un cursor de bytes sobre `pTriangles`; va
  con la tanda de `Mesh`.

## Exactitud y pruebas

- `match.py --changed` sobre las 36 unidades afectadas: sin cambios.
- Build y medida completos y `prepare_fastcmp.py`: 2922 funciones
  byte-exactas; las 3364 filas conservan `s`, `fz` y `x` exactamente.
- `run_differential_suite.py`: 116 arneses, 0 fallos.
- Inventario de accesos crudos: 1423 -> 1417.
- Conversiones puntero<->entero en x64 (`tools/audit_64bit.py` de OpenCMR2,
  sincronizado y con esta tanda aplicada): 933 -> 816, en 313 funciones (antes
  341). `Game.cpp` pasa de 12 funciones afectadas a 5.

# Sombras proyectadas y etapas de textura con sus tipos

Tanda sobre `a60b8d0`, siguiendo `CODEX-64BIT.md`. Dos superficies de estado de
ejecución que guardaban punteros en `int`: los proyectores de sombra de
`SceneNode.cpp` y la textura de cada etapa de Direct3D en `CGraphics`.

Dos commits: `e7e56c9` cambia tipos y forma (con los nombres antiguos de los
globales) y el siguiente solo renombra los cuatro globales, como pide
`CLAUDE.md` (no renombrar a la vez que se iguala código).

## Proyectores de sombra

| Antes | Ahora | Evidencia |
| --- | --- | --- |
| `int g_sceneLightState[30]` | `ShadowCaster *g_shadowCasters[30]` | Solo `Scene_AddShadowCaster` lo escribe (`AllocateLockedBuffer(sizeof(ShadowCaster))`); todos los lectores lo convierten a `ShadowCaster *` |
| `int g_sceneLightState2[10]` | `Mesh *g_shadowCylinders[10]` | Solo `Mesh_GetShadowCylinder` lo escribe; se compara por nombre con `strcmp` y se libera como `Mesh` |
| `BYTE g_sceneLightFlag`, `g_sceneLightFlag2` | `g_shadowCasterCount`, `g_shadowCylinderCount` | Entradas usadas de cada tabla |
| `ShadowPart::field_0x0[0x30]` | `float light[12]` | Dirección de la luz y su base en el espacio de la malla (cuatro `float[3]`), que `Scene_EmitProjectedShadowMesh` pasa a `FloatMatrix_InverseRotateVector` |
| `ShadowPart::field_0x50`, `field_0x52` | `faceCount`, `vertexCount` | Caras y vértices emitidos |
| `Scene_EmitProjectedShadowMesh(float *, int)` con `BYTE *p` y offsets | `(ShadowPart *pPart, int)` con campos | `Scene_ProjectDirtyShadowParts` le pasa `pCaster->pParts + i` |

Dentro de `Scene_EmitProjectedShadowMesh`, `*(float *)(nodo + 0x148)` es
`pNode->worldF[12]` (la traslación de la matriz para Direct3D) y el cursor de
caras empieza en `&pMesh->pTriangles->vertexIndex[1]`.

## Lo que dice el compilador

- `Scene_FreeShadowCasters` (50 %) recorría las tablas con un puntero y
  `(int)p < (int)&tabla[N]`. El original compara con signo (`jl`): MSVC6 lo
  genera así al reducir un índice a puntero. Escrita como
  `for (n = 0; n < 30; n++)` sobre `g_shadowCasters[n]->...`, sin locales, es
  **byte-exacta**: el original vuelve a leer `g_shadowCasters[n]` en cada
  acceso, incluida la condición del bucle interno (`partCount`).
- `Scene_EmitProjectedShadowMesh` (exacta) sigue exacta con `ShadowPart *`.
- Los bucles de `Scene_InitLighting`, `Scene_FreeType2Object`,
  `SceneType2_ReleaseAll`, `Scene_RestoreLights` y `Scene_InitFixedMathTables`
  pasan a índice con el mismo código generado.

## Etapas de textura

| Antes | Ahora | Evidencia |
| --- | --- | --- |
| `ApplyTextureStageChange(int, int)` | `(int stage, Texture *pTexture)` | Sus 22 llamadas pasaban `(int)` de una textura o 0; leía `+0x114` (`pSurface`) |
| `ConfigureTextureStageBlendMode(int, int)` | `(int stage, Texture *pTexture)` | Lo primero que hacía era `(Texture *)param2` |
| `static int m_unk0x0065fa38` | `static Texture *` | Textura de la última etapa cambiada, comparada con la nueva |
| `SetTextureStageState(..., COLORARG2/ALPHAARG2, (DWORD)pTexture)` con textura nula | `D3DTA_DIFFUSE` | Es 0; el descompilador había reutilizado el registro. Mismo código |

## Deuda

- `Sound_NoOpMusicCallback(int)` es una función vacía que comparten llamadores
  de tipos distintos (mensajes de error de la música, nodos de escena y los
  enteros 0..4 de `StageObjects.cpp`): seguramente varias funciones vacías de
  depuración que el enlazador fundió en una. Con `void *` los enteros
  necesitarían un cast; se queda en `int`. Su argumento no se usa, así que
  truncarlo en 64 bits no tiene efecto.
- Las zonas de luz (`g_sceneLightZones`, `LightZoneVertex::pOwner`) y
  `StageObject` son imágenes del fichero de escenario con punteros reubicados
  en sitio: necesitan traducción, igual que `Sector_RelocateStageMeshFile`.
- `MeshTriangle +0x4` es una tabla de texturas indexada por capa
  (`Graphics.cpp` lee `+4 + capa * 4`); el campo `textureIndex` es la primera.
  Convertirlo en tabla toca el número de miembros, que invierte el empate de
  `FrontendMenu_DrawRallySummary` (ver `64bit-scene-roots.md`); va con la tanda
  de `Mesh`.

## Exactitud y pruebas

- `match.py --changed`: sin regresiones; `SceneNode.cpp` 51 -> 52 exactas.
- Build y medida completos y `prepare_fastcmp.py`: **2923** funciones
  byte-exactas (+1, `Scene_FreeShadowCasters`); las demás filas conservan `s`,
  `fz` y `x`.
- `run_differential_suite.py`: 116 arneses, 0 fallos.
- Inventario de accesos crudos: 1417 -> 1382.
- Conversiones puntero<->entero en x64 (`tools/audit_64bit.py` de OpenCMR2,
  sincronizado, con los conflictos puerto/decomp resueltos hacia la decomp y
  esta tanda aplicada): 816 -> 733. `SceneNode.cpp` 70 -> 13, `Graphics.cpp`
  34 -> 14.

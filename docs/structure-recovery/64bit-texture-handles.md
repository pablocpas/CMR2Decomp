# Texturas como `Texture *`

Tanda sobre `f29f401`, siguiendo `CODEX-64BIT.md`. Las texturas cargadas con
`CTexture::FindLoadTexture` se guardaban en `int` y se leían por offsets;
`Texture` contiene punteros (`pSurface`, `pArchive`), así que en 64 bits esos
offsets cambian. Todo es estado de ejecución.

## Evidencia y cambio

| Antes | Ahora | Evidencia |
| --- | --- | --- |
| 33 globales `int` (`g_unk0x0052aa60`, `g_unk0x005436b8[8]`, `g_unk0x0082ca20[9]`, `g_unk0x00590b00`, `g_unk0x00547fb4`...) | `Texture *` | Solo reciben `FindLoadTexture`, copias entre ellos o 0, y solo se usan como textura |
| `WeatherMapSet::maps[4]` | `Texture *maps[4]` | Cargados con `FindLoadTexture`, pasados como textura de la entrada |
| `InRaceMenu_Get{UpArrow,DownArrow,RoundBox,Curtain}Texture` → `int` | `Texture *` | Devuelven esos globales |
| Seis locales `int texture` | `Texture *texture` | Reciben los anteriores |
| `*(short *)(t + 0x11c/0x11e/0x120/0x122)` | `t->field_0x11c`, `field_0x11e`, `width`, `height` | Offsets de `Texture` |
| `(SpriteRect *)(t + 0x11c)`, `((SpriteRect *)(t + 0x11c))->w/h` | `(SpriteRect *)&t->field_0x11c`, `t->width/height` | Rectángulo de origen del sprite |
| `Frontend_SetObjectField118(Unk0x004a3e20 *, int)` | `(Texture *, int)`, asigna `blendMode` | `+0x118` es `Texture::blendMode`; el struct falso desaparece |
| `Billboard_Add(..., unsigned short *)` con `*pTexture` | `(..., Texture *)` con `pTexture->textureId` | Lee el primer `USHORT` de la textura |
| `ParticleType::field0x38`, `field0x4c`; `Particle::field0x60` | `Texture *texture`, `Texture **frames`, `Texture *texture` | Setters de edición, `Particle_DrawAll`, efectos del coche |
| `ParticleEdit_SetTextureParams(int, ...)`, `ParticleEdit_SetExtendedParams(int, ...)` con `(int)&g` | `(Texture *, ...)`, `(Texture **, ...)` con `g` | El primero es la textura; el segundo el array de fotogramas |
| `CarEffects_InitUVs(int, int)` | `(Texture *, Texture *)` | Recibe dos texturas (sin usarlas) |

## Lo que dice el compilador

- Las expresiones con índice anidado (`g_unk0x0082ca20[g_unk0x0082ca04[i]] + 0x120`)
  quedaron como sumas sobre `Texture *` en la primera pasada: el código generado
  leía `+0x15600` (= `0x120 * sizeof(Texture)`) y `RallyData_DrawCarSplitTimesPanel`
  dejó de ser exacta. Corregidas a campos, vuelve a serlo. Se ha comprobado que no
  queda ninguna suma sobre las expresiones tipadas.
- `GameMenu_SetupStageResults` (no exacta) solo cambiaba qué dirección constante
  reservaba en `ebp`, por un cambio en otra función del TU: el bloque de la cortina
  de `GameMenu_DrawSplitTimes`. Con `((Texture *)InRaceMenu_GetCurtainTexture())->width`
  (el cast que el descompilador había conservado) el TU genera lo mismo que antes.
  El original probablemente convertía ahí un getter de otro tipo.
- `OptionMenu_DrawResultsRallyInfo` vuelve a ser la función sensible de
  `GameInfo.cpp` (ver `64bit-menu-transitions.md`): el orden de los operandos de
  `*(int *)&g_unk0x00831660[2] + *(int *)&g_unk0x00831660[0]` se ajusta para que
  cargue como el original con el estado actual del TU.

## Deuda

`Particle_DrawAll` (40 %) recorre las partículas con un cursor de bytes; su
textura se lee como `*(Texture **)(pb + 0xb)` y el callback de dibujo de
`ParticleType::field0x5c` sigue siendo un `int`. Necesita su propia tanda con
`Particle *`.

## Exactitud y pruebas

- `match.py --changed` sobre las unidades afectadas: sin cambios.
- Build y medida completos y `prepare_fastcmp.py`: 2922 funciones
  byte-exactas; las 3364 filas conservan `s`, `fz` y `x` exactamente.
- `run_differential_suite.py`: 116 arneses, 0 fallos.
- Conversiones puntero<->entero en x64 (`tools/audit_64bit.py` de OpenCMR2,
  sincronizado y con esta tanda aplicada): 1080 -> 933.

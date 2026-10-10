# Menús con sus tipos: acciones, transiciones y menús guardados como bytes

Tanda sobre `b3fd9b4`, siguiendo `CODEX-64BIT.md`. Objetivo: que el código
de menús se lea como el original (tipos reales y campos) y no trunque
direcciones en 64 bits. Sustituye el `INT_PTR` de `b3fd9b4` por el tipo de
función real. `Menu` y `MenuItem` son estado de ejecución; ningún fichero
guarda su imagen.

## Evidencia

- **Acciones** (`MenuItem +0x10`): ver `64bit-menu-item-actions.md`. Solo
  `Menu_AddItemType*` las escriben y solo `Menu_HandleInput` las llama, como
  `void (*)(Menu *, MenuItem *)`.
- **Transiciones**: `g_menuNextAction` es el menú al que pasar. Lo escriben
  `Menu_SetNextAction` (100 llamadas, todas con un menú), `Menu_GoBack` y
  `Menu_Update` (padre o submenú del elemento); `Menu_Update` lo devuelve y sus
  receptores lo usan como `Menu *` (`StageUI_UpdatePauseMenu`,
  `GameMenu_RefreshResultsHeader`, `InRaceMenu_UpdateStateMachineFrame`,
  `FrontendMenu_UpdateAndSwitchActive`) o lo comparan con 0. Los menús del
  perfil (`g_unk0x00819030` renombrar, `g_unk0x00819124` al completar,
  `g_unk0x00819870` volver) solo guardan menús para `Menu_SetNextAction`.
- **Menús guardados como bytes** (`GameInfo.cpp`): `g_unk0x0082b488`,
  `g_unk0x0082b668`, `g_unk0x0082b848`, `g_unk0x0082ba28`, `g_unk0x0082bc08`
  eran `BYTE[0x1e0]` usados siempre como `(Menu *)`; `sizeof(Menu) == 0x1e0`.
  Sus accesos por offset son campos: `[0x1f + i * 0x14]` es `items[i].max`,
  `[0x1e]`/`[0x1f]` son `items[0].min`/`.max`, `[6]` es `itemCount` y
  `+ i * 0x14 + 0x18` es `items[i].id`.

## Cambio

- `MenuItem::action` es `MenuItemAction` y los cinco `Menu_AddItemType*` lo
  reciben así; las acciones declaradas con otra firma se registran con
  `(MenuItemAction)Acción`. Ninguna usa su segundo argumento como dirección.
- `Menu *` en `g_menuNextAction`, `Menu_SetNextAction`, el retorno y el local
  de `Menu_Update`, los tres menús del perfil y sus getters/setters, y los
  receptores. `InRaceMenu_UpdateStateMachineFrame` guarda el menú en su propio
  local (`pNext`): su `result` también lleva enteros (cámara, temblor).
- Los cinco menús son `Menu`; sus getters devuelven `Menu *`
  (`GameInfo.h` declara `struct Menu *OptionMenu_GetControlSetupMenu(void)`,
  sin añadir líneas a la cabecera compartida).
- `LayoutChecks.cpp`: `sizeof(MenuItem) == 0x14`, `action` en `0x10`,
  `Menu::items` en `0x14`, `sizeof(Menu) == 0x1e0`.

## Lo que dice el compilador sobre la forma original

- `OptionMenu_FillValueSlider` y `OptionMenu_AdvanceSelectedOption` (exactas)
  tenían `index *= 5; pMode[0x1f + index * 4]`. Escritas con un `Menu *pMode` y
  `pMode->items[index].max` pierden la exactitud (72.73 %): el original calcula
  `index * 5` antes de la segunda llamada al getter. La forma de una sola
  expresión, `Get()->items[Menu_FindItem(Get(), 1)].max`, vuelve a ser exacta:
  es la que escribió el programador.
- `OptionMenu_DrawResultsRallyInfo` (no exacta, 92.07 %) es sensible a cambios
  de su TU: lo único que varía es el orden en que carga los dos operandos de
  `*(int *)&g_unk0x00831660[0] + *(int *)&g_unk0x00831660[2]`. El orden se
  invierte o se recupera según otras expresiones de `GameInfo.cpp` (un cast
  intermedio de más o de menos), sin cambiar nada de la función. La forma final
  conserva ese orden y no lleva casts intermedios. Era la causa por la que
  `b3fd9b4` tuvo que usar `INT_PTR`.
- Tipar los menús destapó tres `*(short *)(Get() + i * 0x14 + 0x18)` en
  `OptionMenu_DrawResultsOptions`: con `Menu *` esa suma cuenta menús, no bytes.
  Son `items[i].id`.

## Exactitud y pruebas

- `match.py --changed` sobre las 31 unidades afectadas: sin cambios.
- Build y medida completos y `prepare_fastcmp.py`: 2922 funciones
  byte-exactas; las 3364 filas conservan `s`, `fz` y `x` exactamente.
- `run_differential_suite.py`: 116 arneses, 0 fallos.
- Conversiones puntero<->entero en x64 (`tools/audit_64bit.py` de OpenCMR2,
  sincronizado y con esta tanda aplicada): 1202 -> 1080 sobre `b3fd9b4`
  (1518 -> 1080 desde `d48fc00`); `FrontendMenus.cpp` 2, `Menu.cpp` y
  `StageUI.cpp` 0, `FrontendScreens.cpp` 126 -> 36.
- Ya no hay `INT_PTR` en el fuente.

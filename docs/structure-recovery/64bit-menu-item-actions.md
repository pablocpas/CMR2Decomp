# Acciones de los elementos de menú con anchura de puntero

Tanda sobre `d48fc00`, siguiendo `CODEX-64BIT.md`. `MenuItem` es estado de
ejecución (los constructores de menú lo rellenan); no es una imagen de disco.

## Evidencia

`MenuItem +0x10` (antes `int param`) guarda la acción del elemento:

- **Escritores:** solo `Menu_AddItemType1/2/3/4/6` y la inicialización a 0 de
  `Menu.cpp`. Sus 521 llamadas pasan 302 direcciones de función con
  `(int)`, 13 sin cast y 206 `0`/`NULL`; nunca un número. 89 funciones
  distintas.
- **Lector:** solo `Menu_HandleInput`, que la compara con 0 y la llama como
  `void (*)(Menu *, MenuItem *)` (`Menu.cpp`).
- **Firmas de las acciones:** 80 declaradas `(Menu *, int)`, 3
  `(Menu *, MenuItem *)`, 4 `(int, int)` y otras tres variantes. Solo 7 usan
  su segundo argumento: dos ya lo declaran `MenuItem *`; las otras cinco
  (`FrontendMenu_EnableRallyStageRows`, `FrontendMenu_SelectDifficulty`,
  `FrontendMenu_SelectRallyStage`, `FrontendProfile_ApplyStageSelectionAndAdvance`,
  `FrontendProfile_OpenPaletteEditor`) solo lo reenvían a
  `RallyData_FillStageSplitEditorRows`, `FrontendProfile_SetupNextPlayer` y
  `FrontendProfile_SelectPlayerSlot`, que no lo leen. Ninguna acción usa el
  `int` como dirección, así que en 64 bits no hay valor truncado que se use.

## Cambio

- `Menu.h`: `typedef void (*MenuItemAction)(Menu *, MenuItem *)` documenta la
  firma con la que se llama. El campo pasa a `INT_PTR action` (mismo offset
  `0x10`) y el parámetro de los cinco `Menu_AddItemType*` a `INT_PTR action`.
- `Menu_HandleInput` llama `((MenuItemAction)pItem->action)(pMenu, pItem)`.
- Las llamadas pasan `(INT_PTR)Acción` en lugar de `(int)Acción`.
  `InRaceMenu_BuildNetworkOptions` conserva su cast intermedio:
  `(INT_PTR)(MenuCallback)InRaceMenu_ApplyNetworkOptions`.
- `LayoutChecks.cpp`: `sizeof(MenuItem) == 0x14`, `action` en `0x10`,
  `Menu::items` en `0x14`.

## Por qué `INT_PTR` y no el tipo de función

`INT_PTR` es `long` con MSVC6 en 32 bits (mismo código) y de 64 bits en un
build de 64 bits, que es lo que se busca: ninguna dirección se trunca. El
tipo de función en el campo y en los parámetros sería más expresivo, pero
cambia el código de otra función de `GameInfo.cpp`:

| Variante | `OptionMenu_DrawResultsRallyInfo` (0x50a920) |
| --- | --- |
| Base | 92.07 %, fuzzy 97.41 % |
| `MenuItemAction` en campo y parámetros | 91.48 %, fuzzy 97.11 % |
| `INT_PTR`, sin el cast `(MenuCallback)` intermedio | 91.48 %, fuzzy 97.11 % |
| `INT_PTR`, con el cast intermedio (esta tanda) | 92.07 %, fuzzy 97.41 % |
| `MenuItemAction`, con el cast intermedio | 91.48 %, fuzzy 97.11 % |

Bisección: el `typedef` solo, los prototipos `INT_PTR` solos y los casts
`(INT_PTR)` del resto de `GameInfo.cpp` no cambian nada; lo que mueve esa
función es quitar el `(MenuCallback)` de la llamada de
`InRaceMenu_BuildNetworkOptions`, y con `MenuItemAction` hay además otra
causa que no se ha aislado. Ninguna función de menú cambia en ninguna variante.

## Exactitud y pruebas

- `match.py --changed` sobre las diez unidades afectadas: sin cambios.
- Build y medida completos y `prepare_fastcmp.py`: 2922 funciones
  byte-exactas; las 3364 filas conservan `s`, `fz` y `x` exactamente (ninguna
  mejora ni empeora: la tanda solo cambia tipos).
- `run_differential_suite.py`: 116 arneses, 0 fallos. Ninguno ejercita
  `Menu_HandleInput`; la equivalencia de esta tanda la da la identidad de las
  métricas de cada función, no cobertura diferencial nueva.
- Conversiones puntero<->entero en x64 (`tools/audit_64bit.py` de OpenCMR2 tras
  sincronizar y aplicar esta tanda): 1518 -> 1202; `FrontendMenus.cpp`
  277 -> 10, `GameMenus.cpp` 52 -> 27, `GameInfo.cpp` 160 -> 141.

OpenCMR2 necesita `INT_PTR`/`UINT_PTR` como `intptr_t`/`uintptr_t` en
`src/port/types.h` (se añade al sincronizar).

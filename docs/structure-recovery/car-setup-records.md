# Reglajes del coche y tablas de opciones

El registro de 7 bytes que antes aparecía como `BYTE [7]`, "SevenByteRecord",
"DriverSkillRecord" o `SavedGameController` es el reglaje del coche. `CarSetup`
(`CarSetup.h`) lo representa en todas sus copias.

## Evidencia de las columnas

Los textos del juego (`englishtext.bfl`, descomprimido con gzip, cabecera
`CMPRHS`) nombran las siete columnas de la pantalla de reglaje en el orden
`0x2b7..0x2bd`: *Tyres, Gearbox, Suspension, Power Ratio, Brake Bias,
Brake Power, Steering*. Los textos de ayuda `0x29a..0x2a0` repiten la lista
(*Steering Sensitivity*). `Car_Spawn` (`0x43c7f0`) aplica cada byte a un setter
distinto, que confirma el orden:

| Byte | Campo | Consumidor en `Car_Spawn` |
| --- | --- | --- |
| 0 | `tyres` | `Car_SetTyreType` (antes `Car_SetSurfaceDragLevel`): grupo de nueve valores de arrastre por superficie. |
| 1 | `gearbox` | `Car_SelectGearSpeedTable`. |
| 2 | `suspension` | `Car_SetSuspension` (antes `Car_SetRideHeight`): amortiguación, recorridos y altura. |
| 3 | `powerRatio` | `Car_SetDriveSplit`; 50 da exactamente `0x8000`. |
| 4 | `brakeBias` | `Car_SetBrakeBias`; 50 da exactamente `0x8000`. |
| 5 | `brakePower` | `Car_SetBrakePower` (antes `Car_SetSteeringSwingTarget`): `maxBrakeForce`. |
| 6 | `steering` | `Car_SetSteeringSensitivity` (antes `Car_SetDifficultyHandling`). |

Las cinco columnas deslizantes valen 0..100 (50 centrado); el menú muestra
el valor dividido por 10 (`OptionMenu_GetCarSetupChoice`) y guarda la
elección multiplicada por 10 (`OptionMenu_SetCarSetupChoice`). `Car_Spawn` lee
todas con extensión de signo, por eso los siete campos son `char`.

`Car_SetSteeringSensitivity` escribe tres campos del coche que ahora tienen
nombre por sus lectores: `steeringOutwardRate` (`0x7fc`) y
`steeringCentringRate` (`0x800`), que `CarPhysics_DampSurfaceSteeringAngle`
usa al girar hacia fuera o volver al centro, y `steeringForceScale` (`0x804`),
que escala las entradas del acumulador en `AutoGear_IntegrateSteeringAccumulator`.

## Copias tipadas

| Dirección | Nombre | Contenido |
| --- | --- | --- |
| `0x52f0e0` | `g_driverGroupTable.playerCarSetups[4]` | Reglaje de cada jugador (`RallyData_GetPlayerCarSetup`). |
| `SavedGameImage + 0x614` | `playerCarSetups[4]` | La misma copia en la partida guardada. |
| `0x82bee8` | `g_optionCarSetups[8]` | Filas 0..3 editadas en la pantalla; 4..7 los reglajes al entrar. |
| `0x51682c` | `g_countryCarSetupBlock.countrySetups[9]` | Reglaje por defecto de cada país (Suecia, índice 3, lleva neumático 6). |
| `0x5200e8` | `g_challengeCarSetups[9]` | Uno por desafío: el mismo índice (bits 10-11 × 3 + bits 12-13) más uno carga `chall\cha%.2d.tga`. |
| `0x520128` | `g_alternateCountryCarSetups[5]` | Países 1/2, 3, 5, 7 y 8 en el modo 4 y algunos casos del flag 25. |

`0x51682c` es un bloque: tras los nueve reglajes vienen un byte de relleno,
un entero `-1` sin lector conocido, la cadena `" "` que miden los menús y los
nombres de tiempo `SNOW..BLIZZARD` a los que apunta `g_weatherSymbolNames`.
`CountryCarSetupBlock` describe esos offsets; vive en `CountryCarSetups.h`
porque definido en `CarSetup.h` (incluido desde `SavedGame.h`) desplaza un
salto de `FrontendMenu_DrawRallySummary`. Las tablas de `0x5200e8` y `0x520128`
quedan con sus 63 y 35 bytes; el original rellena hasta el siguiente global.

## Otros nombres de la tanda

- Texturas por el archivo que cargan: efectos (`spark4`, `paint`, `glass`,
  `exhst02/03`, `WSpray`, `SnowCld`, `Wsplash`, polvo, detalle, hojas,
  hierba, grava), tiempo (`rain`, `splat`, `splat2`, `snow1..3`, `sunglow`),
  rodadas (`skids_1`, `skids_mark`, `skids_blank`), desafío (`netblob`,
  `netblobr`, `chalblob`, mapa), símbolos del menú de opciones (`DanRed/Ora/Yel`,
  `Tick`, `Cross`, `Box`, `Circle`, `Arrow`, `Ar_640A/D`, banner, piezas) y
  símbolos del tiempo. `g_wheelSprayTextures` cargaba `drop.tga` y pasa a
  `g_splashDropTexture`; `g_challengeTextures` cargaba `netblob.tga`.
- `OptionMenu_DrawCountryFlags` dibuja los símbolos y nombres del tiempo
  (`g_weatherChoices`, rellenado por `RallyData_BuildCountryWeatherIndexMap`;
  texto `0x9d + tipo`), no banderas: pasa a `OptionMenu_DrawWeatherChoices`.
- `g_pauseMenus` / `g_pPauseMenu` (`StageUI_GetCurrentPauseMenu`).

## Validación

Las 31 unidades afectadas conservan todas sus puntuaciones con `match.py`;
la compilación y medición completas conservan las 2924 funciones exactas,
`bytes.json` no cambia y la auditoría de datos no da problemas (los bytes de
las tres tablas coinciden con el original). Diez comprobaciones nuevas en
`LayoutChecks.cpp` fijan tamaños y offsets. La suite diferencial completa
pasa antes y después del renombrado de funciones.

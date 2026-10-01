# Traspaso — 2026-10-01

El usuario pide continuar la descompilación hasta byte matching, empezando por
las funciones de menor porcentaje. La integración está hecha en `main`;
el siguiente trabajo debe centrarse en las funciones pendientes.

## Punto de partida

- 2529 de 3363 funciones de fuente exactas después de resolver relocaciones.
- 834 pendientes, incluidas 14 con operandos todavía sin resolver.
- 32 funciones exactas más que la referencia `16527f6`; ninguna exacta perdida.
- Los porcentajes de reccmp son similitud; `Implemented: 100%` no significa
  byte matching completo. La prueba estricta está en `CMR2PROGRESS/bytes.json`.
- `INTEGRATION.md` recoge los orígenes de los cambios, las métricas y las pruebas.
- Las ramas y worktrees anteriores se conservan. Sus cambios recuperados están
  incorporados en main; no borrar ni resetear sus árboles de trabajo.

## Continuación

`CMR2PROGRESS/nonmatching.tsv` se ordena por porcentaje de reccmp ascendente,
desempatando por tamaño original descendente. Contiene también la puntuación
de la auditoría de bytes. Empezar por este informe actualizado, recuperando
primero flujo de control, tipos, campos de bits y llamadas del original.

El lote actual deja estas funciones mejoradas pero pendientes:

| Dirección | Función | Diferencia restante |
|---|---|---|
| 0x004069c0 | RallyData_InitKnockoutBracket | Accesos a bitfields, orden de operaciones y variables de pila; pasó del 11% al 66%. |
| 0x00505e10 | CGameInfo::FUN_00505e10 | Resultado preservado en ESI y orden de las cargas; pasó del 23% al 79%. |
| 0x004cf3f0 | FUN_004cf3f0 | `add eax,8` y `xor ecx,ecx` intercambiados; pasó del 50% al 91%. |
| 0x004ac7a0 | SceneNode_Reparent | El original retorna con `mov eax,ecx`; la recompilación recarga `dirty` del padre. Reccmp: 38% → 94%; auditoría de bytes: 98%. |

`FUN_004735a0` pasó del 41% a bytes exactos, incluidos sus caminos de selección
validados por ejecución nativa. La inicialización de eliminatorias conserva
una peculiaridad del original: en la ronda de dos pilotos el sorteo par elige
el segundo piloto. La prueba nueva detecta el error de la implementación previa.

## Compilar y medir en esta máquina

```bash
export CMR2_MSVC_ROOT=/home/pablo/colin_mcrae_linux/tools/msvc600/VC98
export WINEPREFIX=/tmp/cmr2-main-integration/wineprefix
export CMR2_TOOLS=/tmp/cmr2-main-integration/test-tools
python3 scripts/build.py
python3 scripts/measure.py
python3 tests/differential_knockout_bracket.py CMR2PROGRESS/summary.json CMR2PROGRESS/entities.json
python3 tests/differential_scene_reparent.py CMR2PROGRESS/summary.json CMR2PROGRESS/entities.json
```

Wine necesita ejecución fuera del sandbox en este entorno. El prefijo anterior
está aislado del utilizado por los otros worktrees. `README.md` explica cómo
configurar compilador y prefijo en otra máquina. El ejecutable original y las
configuraciones locales de reccmp no se versionan.

Los informes incluyen hashes de fuentes, compilador, EXE y PDB. Los campos
`build.commit` y `source_changed` describen el momento de la compilación, antes
del commit de cierre; los hashes identifican las fuentes realmente medidas.
No reutilizar informes ni mapas de símbolos con otro binario. No medir una
compilación `--windowed` como si correspondiera al original.

## Evidencias y experimentos

Los logs finales están en `/tmp/cmr2-main-integration/clean-final` y el resumen
de pruebas se guarda en `CMR2PROGRESS/validation.json`. Se ha ejecutado MSVC6
con Wine y los harnesses diferenciales; no una carrera interactiva completa ni
el CI de Windows.

En `/tmp/cmr2-main-integration/matching-lows` quedan variantes temporales y sus
diffs, muchas descartadas. No incorporarlas en bloque: algunas empeoran el
matching, cambian las opciones de optimización o sirven sólo para investigar.
Las variantes con contadores sin inicializar no están en main. La cola de
entrada `0x004b7d60` ya era exacta: su antiguo comentario del 45% estaba obsoleto
y no se cuenta como ganancia del lote.

Después de cada lote: recompilar, medir, mantener cero incidencias en datacmp,
comprobar que ninguna función antes exacta deja de serlo y ejecutar las pruebas
de comportamiento afectadas. Conservar las anotaciones de las funciones de
bajo porcentaje. Usar C++ compatible con MSVC6; no sustituir lógica por direcciones
del ejecutable original ni ensamblador artificial para aumentar el porcentaje.

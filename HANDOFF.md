# Traspaso — 2026-10-01

El usuario ha pedido cerrar el trabajo y dejar `main` limpio para otro agente.
Se detiene aquí la descompilación. El objetivo pendiente es bajar a 700 funciones
no exactas, empezando por las de menor porcentaje; después, continuar hasta
byte matching completo. La integración y los cambios verificados están en `main`.

## Punto de partida

- 2531 de 3363 funciones de fuente exactas después de resolver relocaciones.
- 832 pendientes, incluidas 14 con operandos todavía sin resolver.
- 34 funciones exactas más que la referencia `16527f6`; ninguna exacta perdida.
- Los porcentajes de reccmp son similitud; `Implemented: 100%` no significa
  byte matching completo. La prueba estricta está en `CMR2PROGRESS/bytes.json`.
- `INTEGRATION.md` recoge los orígenes de los cambios, las métricas y las pruebas.
- Las ramas y worktrees anteriores se conservan. Sus cambios recuperados están
  incorporados en main; no borrar ni resetear sus árboles de trabajo.

## Último lote cerrado

- `Font_GetTextHeight` (`0x0040b730`): del 40% a los 89 bytes originales exactos.
- `FUN_0047b870` (`0x0047b870`, controles al comenzar la etapa): del 25% a los
  254 bytes originales exactos. Usa el helper existente `FixVecScale` y conserva
  las cargas del puntero global y el orden de las escrituras del original.
- `CGameInfo::FUN_00406010`: del 16,95% al 58,70% de reccmp; sigue pendiente.
  Corrige la limpieza de tres bytes, los bits de coche/cambio y la copia de
  etiquetas. La nueva prueba `differential_record_reset.py` compara 6000 casos
  con el código original y un modelo independiente de las tablas, incluidos
  bits preservados, desbordamiento de splits y guardas. La versión previa
  falla el caso 0: deja sucio el byte intermedio que debe limpiar.
- Las 33 pruebas diferenciales nativas pasan en el binario final. Ninguna
  función anteriormente exacta se ha perdido. Datos: cero incidencias.
- Quedan 132 funciones por convertir a exactas para llegar al objetivo de 700.

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
python3 tests/differential_record_reset.py CMR2PROGRESS/summary.json CMR2PROGRESS/entities.json
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

Los logs del último cierre están en `/tmp/cmr2-goal700/clean-final`; la
compilación y la medición están en `/tmp/cmr2-goal700/*-final.log`. Los logs de
la integración previa siguen en `/tmp/cmr2-main-integration/clean-final`. El resumen
de pruebas se guarda en `CMR2PROGRESS/validation.json`. Se ha ejecutado MSVC6
con Wine y los harnesses diferenciales; no una carrera interactiva completa ni
el CI de Windows.

En `/tmp/cmr2-goal700` y `/tmp/cmr2-main-integration/matching-lows` quedan
variantes temporales y sus
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

## Pistas pendientes del último lote

No se han aplicado las variantes de rankings, cursor, eventos ni ancho de texto:
ninguna consiguió bytes exactos. El siguiente agente puede agrupar funciones
que aún usan tres `FixMul` escalares y comprobar si el original usa `FixVecScale`;
ese patrón permitió cerrar `0x0047b870`. Comprobar cada caso contra el original.
Los archivos de `/tmp` son auxiliares locales: el traspaso reproducible está
versionado en este documento, los informes y las pruebas. No incorporar las
variantes temporales sin recompilar y verificar pérdida cero de funciones exactas.

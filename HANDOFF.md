# Traspaso — 2026-10-01

Trabajo cerrado por petición del usuario para pasarlo a otro agente. La
integración y los lotes verificados están en `main`. El objetivo de bajar a
700 funciones no exactas sigue pendiente; queda pausado en este traspaso.

## Punto de partida

- 2535 de 3363 funciones de fuente exactas después de resolver relocaciones.
- 828 pendientes, incluidas 14 con operandos todavía sin resolver.
- 38 funciones exactas más que la referencia `16527f6`; ninguna exacta perdida.
- Los porcentajes de reccmp son similitud; `Implemented: 100%` no significa
  byte matching completo. La prueba estricta está en `CMR2PROGRESS/bytes.json`.
- `INTEGRATION.md` recoge los orígenes de los cambios, las métricas y las pruebas.
- Las ramas y worktrees anteriores se conservan. Sus cambios recuperados están
  incorporados en main; no borrar ni resetear sus árboles de trabajo.

## Último lote de matching

`Events_Add` (`0x0046e620`) pasa del 20% a los **121 bytes originales exactos**.
Se recupera el acceso mediante el índice global mientras se inicializa el
registro. La auditoría completa confirma una ganancia y cero pérdidas frente
a `34dd8e3`. No se cambiaron flags, estructuras ni declaraciones compartidas.
Quedan **128** ganancias exactas para alcanzar las 700 pendientes.

Los logs del cierre están en `/tmp/cmr2-goal700/handoff-build.log`,
`handoff-measure.log`, `handoff-native.log` y `handoff-tests/`.
Los metadatos de la compilación y del matching están en `CMR2PROGRESS/provenance.json`.

## Lote anterior: tres funciones de bajo porcentaje

| Dirección | Función | Reccmp antes | Resultado |
|---|---|---:|---|
| 0x00505590 | FUN_00505590, rectángulo del panel de splits | 26,20% | 588 bytes exactos |
| 0x004207f0 | RallyData_FUN_004207f0, reinicio de registros de carrera | 45,45% | 37 bytes exactos |
| 0x00403110 | FUN_00403110, sliders de cámara | 45,83% | 239 bytes exactos |

Se recuperan las divisiones/multiplicaciones de fijo mediante los helpers
existentes, el orden original de cálculo y escritura y el recorrido relativo
al campo de los registros. Compilación completa, auditoría de todas las funciones,
datacmp sin incidencias y las 33 pruebas diferenciales nativas pasaron en ese
binario. No hubo funciones exactas perdidas. Ese lote dejó 829 pendientes.

Las opciones `/G5`, `/G6`, `/Op`, `/Oa` y `/Ow` se comprobaron en copias temporales
de varias unidades. No hay una mejora común sin pérdidas: no se cambiaron los
flags del build. `/Ow` cierra `FUN_004556f0` en la copia de StageTiming pero pierde
28 funciones antes exactas; recuperar la diferencia de su orden de instrucciones
mediante fuente, sin aplicar ese flag a la unidad.

Los logs de ese lote están en `/tmp/cmr2-goal700/wave2-tests`,
`wave2-build.log`, `wave2-measure.log`, `wave2-cpu.log` y `wave2-flags.log`.
Los metadatos de la compilación y del matching están en `CMR2PROGRESS/provenance.json`.

## Lote del cierre anterior

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

Los logs del cierre anterior están en `/tmp/cmr2-goal700/clean-final`; la
compilación y la medición están en `/tmp/cmr2-goal700/*-final.log`. Los logs de
la integración previa siguen en `/tmp/cmr2-main-integration/clean-final`. El resumen
de compilación y matching está en `CMR2PROGRESS/provenance.json`; las pruebas
ahora muestran sus resultados solo en consola. Se ha ejecutado MSVC6
con Wine y los harnesses diferenciales; no una carrera interactiva completa ni
el CI de Windows.

En `/tmp/cmr2-goal700` y `/tmp/cmr2-main-integration/matching-lows` quedan
variantes temporales y sus diffs, muchas descartadas. No incorporarlas en bloque: algunas empeoran el
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

No se han aplicado las variantes de rankings, cursor ni ancho de texto:
ninguna consiguió bytes exactos. El siguiente agente puede agrupar funciones
que aún usan tres `FixMul` escalares y comprobar si el original usa `FixVecScale`;
ese patrón permitió cerrar `0x0047b870`. Comprobar cada caso contra el original.
Los archivos de `/tmp` son auxiliares locales: el traspaso reproducible está
versionado en este documento, los informes y las pruebas. No incorporar las
variantes temporales sin recompilar y verificar pérdida cero de funciones exactas.

En las variantes `wave2-*`, `FixBasis_Integrate` llega al 97,47% de la auditoría
pero sigue sin bytes exactos: dos escrituras de la parte final quedan en distinto
orden. Las variantes de cursor, color de splash, blink, limpieza de vectores y
flags de etapa tampoco cerraron funciones. No están aplicadas en main.

## Investigación posterior para evitar repetir intentos

Los experimentos `wave3-*` están en `/tmp/cmr2-goal700`. Sólo se incorpora
la variante exacta de `Events_Add`; las restantes no están en las fuentes.

- `Events_Add` (`0x0046e620`): el acceso directo
  `g_eventRecords[g_eventCount]` y el incremento `g_eventCount++` recuperan la
  promoción del contador global y sus registros originales. Conservar también
  la comprobación de `g_eventTextures` presente en el original. Un puntero local
  al registro o asignar `g_eventCount = count + 1` no da el mismo código.
- `CInput::FUN_0040bc90`: reutilizar el parámetro llega al 66,67% de la auditoría;
  los casts aislados y tipos alternativos no cierran la función.
- `FUN_004698a0`: el original accede a los datos de cada parte relativos al
  puntero de sus contadores. Las variantes corrigen las bases, pero sólo llegan
  al 33,85%; requieren reconstrucción completa y verificación de comportamiento
  antes de integrarse.
- `FUN_00455bc0`: los recorridos con direcciones enteras y offsets negativos
  quedan en el 37,5%; siguen faltando la estructura del desplazamiento de filas
  y la vida de los registros. No aplicar esos experimentos.
- `FUN_0045d1e0`: externizar globals, structs/unions y cambios de tipos de canales
  no cierran el color de splash. No se cambiaron las declaraciones compartidas.

Las cifras de experimentos aislados no se suman al progreso. La referencia para
continuar es exclusivamente la auditoría completa versionada en `main`.

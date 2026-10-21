# Perfiles TM de tiempos y orden de pilotos

El global de `0x54241c` es ahora `StageTimeProfile *g_stageTimeProfile`.
Su productor (`0x455300`) busca `<etapa>.tm0/.tm1/.tm2` y copia la variante
seleccionada por dificultad o modo de juego. Sus lectores usan cinco
coeficientes por piloto para calcular valores de ordenación y dos bancos
de muestras para construir tiempos por split. No son archivos de equipo
ni datos de pintura: se corrigen cuatro nombres de funciones que atribuían
esas funciones al recurso.

| Dirección | Nombre actual | Evidencia |
| --- | --- | --- |
| `0x455300` | `StageTiming_LoadSelectedTimeProfile` | Carga las tres variantes TM de la etapa y publica el perfil seleccionado. |
| `0x455f00` | `StageTiming_BlendDriverTimeTuning` | Combina el valor base y dos ajustes de cada piloto con pesos de posición y mezcla del workspace. |
| `0x456110` | `StageTiming_RandomizeAndSortDriverTimeTuning` | Añade el rango aleatorio y pasa los 16 valores a `RallyTiming_SortOrder`, con el array de índices de pilotos. |
| `0x455ed0` | `StageTiming_BuildAndSortDriverTimeTuning` | Crea el array temporal de valores y encadena los dos pasos anteriores. |

Los nombres describen el uso observado, sin atribuirlos al fuente original.
Cinco parámetros que llamaban coche, tiempos o `param_1` al workspace pasan
a llamarse `pSpread`. Su tipo sigue siendo el `StageTimingSpread` existente;
no se modifica su layout ni la aritmética de pesos.

## Layout del archivo

| Offset | Campo | Evidencia |
| --- | --- | --- |
| `0x000` | `drivers[16]`, registros de cinco bytes | Los dos consumidores recorren 16 valores, antes mediante un índice de bytes con paso 5. |
| `+0` por piloto | `baseValue`, byte con signo | Se convierte a fijo y suma al valor de ordenación interpolado. |
| `+1` | `randomSubtract`, byte con signo | Su negación forma uno de los extremos del rango aleatorio. |
| `+2` | `randomAdd`, byte con signo | Es el otro extremo del rango; se suma tras el producto con el aleatorio normalizado. |
| `+3` | `blend0Adjustment`, byte con signo | Ajuste ponderado por los complementos de los dos pesos de mezcla. |
| `+4` | `blend1Adjustment`, byte con signo | Ajuste ponderado por los dos pesos de mezcla. |
| `0x050` | `blend0Times[10][16]`, enteros con signo | Primer banco del interpolador de splits. |
| `0x2d0` | `blend1Times[10][16]`, enteros con signo | Segundo banco; los pesos de mezcla seleccionan sus muestras. |
| `0x550` | Tamaño total | Dos bancos de `0x280` bytes después de los 80 bytes de coeficientes. |

El tamaño y los offsets de ambos bancos se comprueban en `LayoutChecks.cpp`;
el registro de coeficientes mide cinco bytes. La prueba nativa verifica los
mismos valores con punteros de ocho bytes. El archivo no contiene punteros.

La [auditoría de recursos](stage-time-profile-assets.json) recorre 94 archivos
de pistas de alto detalle: encuentra 282 entradas TM, 255 de `0x550` bytes y
27 vacías. Conserva nombres, tamaños y SHA-256, sin copiar los datos del juego.
Se reproduce con los recursos instalados mediante:

```sh
python3 scripts/audit_stage_time_profiles.py /ruta/Data/GAME/Tracks \
  --json docs/structure-recovery/stage-time-profile-assets.json
```

Las entradas vacías no prueban que el consumidor acepte un perfil vacío.
El loader mantiene su selección y sus supuestos previos, sin añadir validación
de longitudes o reglas nuevas para variantes ausentes.

## Cambios de acceso y límites

Los consumidores de coeficientes usan `drivers[i].campo`, con índices de piloto,
en lugar de aritmética de bytes. Los dos tiempos de la ruta especial de
`StageTiming_InitRaceDriverRecords` se leen como `blend1Times[0][0]` y
`blend1Times[1][0]`; esas lecturas eliminan dos accesos crudos.

Las seis lecturas del interpolador siguen visibles mediante un cursor de bytes.
Las variantes con índices tipados introducen otra inducción y otro slot de
stack en MSVC6, y pierden puntuación frente al 93,37 % byte y 98,90 % fuzzy
existente. Se retiran esas variantes; no se ocultan los accesos ni se añaden
vistas de array sin consumidores para alterar el compilador.

Los extremos aleatorios son `randomAdd` y `-randomSubtract`, aunque el primero
sea mayor que el segundo. No se renombran como mínimo/máximo, no se fuerzan
positivos y no se les asignan unidades no demostradas por los lectores.

## Validación

La compilación, el gate de las tres unidades afectadas y la medición completa
conservan todas las puntuaciones fuzzy y las 2924 funciones exactas de 3364.
El consumidor aleatorio mejora de 87,94 a 90,78 % de bytes, manteniendo
95,04 % fuzzy; las demás puntuaciones permanecen iguales. No hay errores de
comparación de datos. Los 142 harnesses diferenciales pasan sobre el mismo
ejecutable medido.

El nuevo harness añade 168 escenarios con modelos aritméticos independientes:
coeficientes de −128 a 127, mezcla en extremos y fuera del intervalo habitual,
bancos con valores distintos y wrap de enteros, splits −1/0/1/4/9, extremos
del RNG y conteos de pilotos 0/1/8/16. Comprueba el heap completo, el perfil,
el workspace, salidas, orden/argumentos de proveedores y ABI. La ordenación
final es una hoja controlada; no se afirma que esas pruebas ejecuten una carrera.

El fixture nativo existente se actualiza a los nombres/tipos de producción y
a las dependencias actuales de sus cabeceras. Incorpora los mismos 168 casos,
incluidos RNG y ordenación controlados, y pasa 716 escenarios totales con
ASan/UBSan, punteros por encima de 4 GB y comparación con el original x86.

El inventario pasa de 92 a 90 candidatos de acceso crudo. Sus 1391 campos
detectados sin nombre semántico permanecen: estos tipos sustituyen la vista
de un buffer, sin borrar declaraciones antiguas para mejorar el contador.

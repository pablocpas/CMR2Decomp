# Revisión de lógica — 2026-10-03

Se ha revisado el inventario de las **3363 funciones anotadas del fuente**,
contrastando matching, cuerpos triviales, marcadores de trabajo pendiente,
símbolos sin resolver, pruebas existentes y, en las 793 no exactas, llamadas,
saltos y limpieza de pila del código original y recompilado. Se han ejecutado
los **71 harnesses diferenciales: cero fallos**. La cobertura adicional incluye
superficies, geometría, rutas, empaquetado y recepción de red, decisión de
colisión, ABI, matrices de objetos y restauración física,
instantáneas de replay, inicialización de contactos, estimaciones y orden de
carrera, y avance por checkpoints.

Esto deja un inventario completo y evidencia concreta; **no demuestra todavía
que toda la lógica del juego sea equivalente**. Los comentarios que afirman
haber transcrito una función y su porcentaje de matching no sustituyen pruebas
de comportamiento. Las diferencias estáticas tampoco demuestran un defecto.

| Estado | Funciones | Interpretación |
| --- | ---: | --- |
| `local_byte_exact` | 2570 | Bytes locales equivalentes tras resolver relocaciones; sus dependencias pueden seguir pendientes |
| `fixtures_pass_matching_pending` | 59 | No exactas, pero pasan pruebas directas en los casos registrados |
| `logic_unverified` | 723 | No exactas y sin prueba directa registrada que pase |
| `unresolved_and_untested` | 11 | Igual que la anterior, además con símbolos/constantes sin resolver |
| Total | 3363 | Todas las funciones del inventario de fuente |

Quedan **793 funciones para matching**, de las cuales **734 necesitan ampliar
la validación directa de comportamiento**. Son tareas pendientes de validación,
no errores demostrados. Los 71 harnesses registran 112 entradas principales
distintas; los proveedores simulados no se cuentan como funciones probadas.

Hay 106 funciones no exactas por debajo del 50% en la comparación de bytes;
15 ya pasan pruebas. Con el porcentaje de reccmp son 101 por debajo del 50%,
18 de ellas probadas. También hay 146 no exactas con comparación de bytes de
al menos el 90% que todavía carecen de prueba directa. El matching alto no
demuestra corrección. En los módulos críticos del auditor hay 287 funciones
no exactas: 34 pasan pruebas directas y 253 aún las necesitan.

La prioridad actual es **matching manual por función**, guiado por ASM,
complementando al otro worktree con candidatos de matching alto y pruebas
existentes. En esta pasada se integran tres cuerpos exactos: `0x456330`
(97,79% → 100%), `0x41f930` (91,99% → 100%) y `0x50c130` (96,68% → 100%).
No se pierde ningún exacto ni baja ningún porcentaje. Recibo:
[matching-manual-high-2026-10-03.json](matching-manual-high-2026-10-03.json).

## Defecto confirmado y corregido

`FUN_0046e780` (`0x46e780`) tenía una firma sin argumentos en el fuente.
El original recibe un argumento sin uso y termina con `ret 4`; el llamador
original `FUN_0040f0c0` empuja `0` antes de llamarlo. Se han corregido definición,
declaraciones y llamada en `StageObjects.cpp` y `RallyData.cpp` para conservar
ese contrato.

La prueba detectó que, con el argumento presente, el original retiraba ocho
bytes de la pila —retorno más argumento— y la compilación anterior solo cuatro.
El llamador anterior también omitía el argumento, por lo que esto no demuestra
por sí solo un fallo de toda llamada recompilada; sí confirma la discrepancia
de ABI frente al original. La regresión actual verifica cinco salidas tempranas
y rechaza una mutación que elimina la limpieza del argumento. No cubre todavía
las ramas de dibujo activo.

Los cuerpos pequeños `0x4057a8` y `0x4057ab` también se han comprobado:
el original devuelve cero y retorna sin operación, respectivamente. Son noops
válidos de release, no lógica omitida. Sus cuerpos son ahora exactos: se ha corregido el comparador para eliminar
el padding de COFF antes de calcular cuántos bytes leer del original. El tamaño
del inventario original era correcto; la lectura anterior incluía código vecino.

## Restauración física y revisión de lógica de carrera

`FUN_00426d80` (`0x426d80`), que estaba al 30,42% en bytes y 31,07% en reccmp,
queda **100% exacta**. El original usa `rep movsd` para copiar los nueve
componentes de orientación anterior; el bucle escalar se ha sustituido por
`memcpy` entre regiones contiguas de 36 bytes sin solapamiento. La nueva prueba
compara 960 restauraciones, con helpers reales, matriz, memoria completa del
coche, ambas ramas de suspensión y entrada aliada o separada.

La misma revisión de copias del original mejora `FUN_0046d610` (`0x46d610`)
del 26,63% al 69,27% en bytes (70,50% reccmp). Pasan 1921 ticks de replay,
incluidos cambios de fase y salidas tempranas. Los proveedores de coche,
frames, eventos, geometría e interpolación están controlados: sus
implementaciones siguen fuera de esta prueba.

`FUN_00448d50` (`0x448d50`) conserva un matching reccmp del 25%, pero pasa
2592 fixtures de orden de carrera: cero a ocho coches, empates, progreso
firmado, contadores de byte y pasadas repetidas. Ejecuta el comparador y las
consultas reales; además se compara con un modelo independiente.

`FUN_00458e00` (`0x458e00`, 42,94% reccmp) pasa 3072 fixtures de avance por
checkpoints: ambas direcciones, vueltas, umbrales ±50, límite de recorrido,
contadores firmados y máximos sin signo. Se comprueban registros completos
con guardas y orden de eventos contra un modelo independiente; los cinco
helpers de eventos y actualización están controlados.

La restauración física queda cerrada localmente. Las otras tres conservan
matching pendiente y cobertura limitada a los casos descritos. Ninguna mejora
ha perdido una función que antes fuera exacta.

## Recepción por red y decisión de colisión

`FUN_00425c40` (`0x425c40`) pasa de 27,62% a **57,06% reccmp** y de
44,60% a 54,12% en bytes al reproducir los dos arrays de ángulos y el cursor
de filas del original. La prueba ejecuta el receptor completo y el helper de
movimiento remoto reales: **4096 paquetes** con secuencias aceptadas, iguales,
antiguas, saltos de 100 y wrap, red desactivada, sectores inválidos, límites
quantizados y cuatro modos de redondeo x87. Compara los registros completos,
steering, paquete conservado y guardas. Inicializa tablas sine/sqrt compartidas
y evita velocidades tan pequeñas que desborden la división del original.
No valida transporte ni sesiones de red completas.

`FUN_00490b90` (`0x490b90`) pasa de 44,44% a **60,19% reccmp** y de
37,11% a 57,14% en bytes al conservar el resultado durante las llamadas y
expresar el árbol de decisión con una salida común. Sus **7200 fixtures**
comprueban decisiones con modelo independiente, selección fallida/aceptada,
contadores unsigned, ambas caras, memoria con guardas, argumentos y limpieza
de pila. Los proveedores de selección, transformación y clasificación
geométrica están controlados; sus implementaciones continúan pendientes.

Ambas siguen pendientes de matching exacto. Las versiones anteriores también
superaban estos casos: se han mejorado sus estructuras de código, sin detectar
un defecto de comportamiento en los fixtures. El registro
[de red y colisión](matching-net-face-2026-10-03.json) conserva las mediciones,
los hashes y la ausencia de pérdidas de exactos.

## Inicialización de contactos y estimación de carrera

`FUN_00494bb0` (`0x494bb0`) pasa de 42,79% a **57,69% reccmp**
y de 41,41% a 55,45% en bytes al reproducir los incrementos secuenciales
del cursor de manejo. Sus **2560 fixtures** comprueban los registros de
contacto, cinco cachés, slots intactos, ocho modos de carrera y cuatro
estados de rally. Ejecutan todas las consultas reales; no cubren la
simulación completa de contactos.

`FUN_004483e0` (`0x4483e0`) pasa de 43,14% a **47,76% reccmp**
y de 42,06% a 46,01% en bytes al conservar el cursor durante el recorrido
y convertir el tiempo antes de calcular su escala. Sus **2160 fixtures**
comprueban orden, posiciones y tiempos con guardas, prefijos ya clasificados,
progreso cero/firmado, empates y actualización del resumen de rally antes
del ajuste por empate. Usan un modelo entero independiente, todos los helpers
reales y cuatro modos de redondeo x87. Exigen listas válidas de pilotos únicos
y denominadores distintos de cero.

Las versiones anteriores también pasan estos casos: estas dos mejoras no
corrigen un fallo de comportamiento demostrado. Conservan matching pendiente.
El [registro de contactos y tiempos](matching-contact-times-2026-10-03.json)
conserva los hashes, resultados y ausencia de pérdidas de exactos.

## Steering automático exacto

`FUN_00494110` (`0x494110`) pasa de 47,96% en bytes y 45,51% reccmp
a **bytes locales exactos**. Conserva las dos fuerzas intermedias por separado,
los límites cero materializados por `FixMul(0, v)`, el árbol de decisiones
por los dos flags y las comparaciones de magnitudes/ángulos del original.
No se han añadido instrucciones ASM ni cambiado flags de compilación.

La nueva regresión compara **9984 casos** con modelo independiente de enteros
y de redondeo x87 mediante racionales. Comprueba toda la memoria del coche
con guardas, rampa, fuerzas, acumulador, ángulos de 16 bits, wrap firmado de
32 bits, orden de helpers y pila. Ejecuta los tres helpers reales de alineación,
torque y rampa, sin proveedores simulados, en los cuatro modos de redondeo.
La versión anterior también supera estos casos; la mejora cierra matching,
no un defecto de comportamiento demostrado. No valida todo el ciclo de caja
automática ni una simulación de conducción.

[matching-steering-2026-10-03.json](matching-steering-2026-10-03.json) conserva
la compilación, hashes, pruebas y ausencia de pérdidas de exactos.

## Matriz y posicionamiento de objetos

`FUN_00486910` (`0x486910`) pasa de 45,16% a **75,17% en bytes**
y de 45,03% a **73,83% reccmp**. Las vistas `FixMatrix` de los datos fuente
y destino, y la elección directa de 10 o 0 grados, reproducen mejor los
registros del original y su máscara del ángulo. Conserva matching pendiente.

`FUN_004869e0` (`0x4869e0`) pasa de 94% en bytes a **100% exacta** al guardar
los ceros desde la base del objeto y conservar la lectura de la escala y el
orden de los seis campos finales. No se han añadido instrucciones ASM ni
cambiado las opciones de compilación.

La nueva prueba ejecuta **12288 casos**: 6144 entradas directas por cada función,
con matrices afines, cuatro slots válidos, modos 0..2, cuatro variantes de
estado de juego/carrera y cuatro modos x87. El modelo independiente calcula
los ejes reflejados, la rotación con racionales y el desplazamiento por split;
compara toda la memoria del objeto/referencia con guardas, tablas intactas,
contador de productos y pila. Ejecuta todos los helpers de rotación,
posicionamiento y consultas reales, sin proveedores simulados.

Las versiones anteriores también pasan estos casos. Los buffers de matriz
son distintos y no se valida todo el ciclo de actualización/renderizado de
objetos. El [registro de esta tanda](matching-object-matrix-2026-10-03.json)
conserva hashes, resultados y ausencia de pérdidas de exactos.

## Transformaciones de coche y defecto de suspensión

`Car_StoreRenderTransforms` (`0x42af50`) pasa del **44,78% al 91,45% en bytes**
y del **45,01% al 94,52% reccmp**. La revisión detectó un defecto lógico:
el ángulo de rueda usa el grupo `setup +0x240`, mientras la suspensión usa
`setup +0x250`; el fuente anterior reutilizaba `+0x240` para ambos cálculos.
Se ha corregido la lectura de suspensión y reconstruido el orden de consultas,
las ramas de escala y los temporales de rueda del original.

La nueva prueba compara **1920 casos** de ambas pasadas contra un modelo
independiente: ocho coches, IDs distintos de slots, listas permutadas/repetidas,
clamps de suspensión, límites enteros y cuatro modos x87. Comprueba matrices,
ruedas, memoria de coches, guardas, tablas, orden de consultas y pila, con todos
los helpers reales. El original pasa los 1920 casos; la compilación anterior
falla el caso `(0, 1, 0, 0)` y la corregida pasa. Esta diferencia confirma el
defecto frente al original, incluso aunque otros casos lo oculten por el clamp.

Los tipos válidos son 0..13 y las cantidades -1..8. No se simula el renderizado
ni el ciclo completo de física. El matching local sigue pendiente. El
[registro de esta tanda](matching-render-transforms-2026-10-03.json) conserva
la evidencia del fallo anterior, hashes, resultados y ausencia de exactos perdidos.

## Manejador de carrera: punteros, argumentos y transiciones

`FUN_0041e8d0` (`0x41e8d0`) pasa del **43,59% al 59,60% en bytes** y del
**43,97% al 61,35% reccmp**. Se han confirmado y corregido diferencias de
comportamiento frente al original: la consulta del contador de tiempo perdía
el puntero a jugadores; una salida pasaba el argumento inicial a teardown en
lugar de la fase guardada; y varias salidas de los estados 0 y 4 omitían o
añadían transiciones. El restore de las etapas 2/4/6/8/10 también debe poner
el estado de juego a 2 antes de la restauración común.

La nueva prueba compara **11003 casos** y verifica que se ejecutan todos los
índices de las cinco tablas originales de despacho: **11/5/13/7/4**. Incluye
modos 0..12 y default, estados 0..4 y default, etapas 0..11, flags combinados,
listas de 0/1/2/8 slots, salidas tempranas, ambos finales del progreso en modo 5
y argumentos con bits altos. Las consultas de juego/rally/fase y la entrega
del puntero al contador son reales; un modelo independiente comprueba tiempo,
fase, progreso y guardas. La comparación diferencial comprueba además toda
la secuencia y argumentos de las acciones controladas y la limpieza de pila.

El ejecutable anterior falla los dos invariantes de puntero/argumento y las
cinco secuencias del estado 4 guardadas en el
[registro de esta tanda](matching-race-handler-2026-10-03.json).
El original y el ejecutable corregido pasan todos los casos. Teardown, vistas,
replay, sonido/render, reloj y selección externa están controlados: no se
validan sus implementaciones ni una carrera completa. Conserva matching pendiente.

## Huella de contacto exacta y búsqueda agrupada

`FUN_004962c0` (`0x4962c0`, 2869 bytes) pasa del **95,51% a bytes locales
exactos** al agrupar las escrituras de cada esquina, conservando el orden de
las llamadas. Sus **13440 casos** usan todos los helpers reales y un modelo
entero independiente: coches normales/ghost, normales inclinadas/desconocidas,
umbral de alineación ±1, 14 etapas, dimensiones límite y memoria con guardas.
La versión anterior también pasa los casos: se cierra matching, sin detectar
un nuevo defecto de comportamiento. No se valida aquí el tick completo de
contacto, sombras o skid trails; los buffers de entrada son distintos.

La tanda adicional integra **42 mejoras estrictas**, ninguna caída de matching
ni pérdida de exactos. Entre ellas, `Car_UpdateTyreForces` (`0x4387a0`) pasa del
55,52% al 64,36% en bytes y `Car_UpdateSurfaceParams` (`0x4781d0`) del 46,24%
al 54,84%. El lote de bajo matching probó 6281 variantes en 220 segundos.
Se excluyeron cuatro candidatos ganadores que reordenaban cálculos flotantes
o inicializadores de punteros float. Los 68 harnesses pasan sobre el lote final.

El motor `--packed` prueba variantes de varias funciones en una compilación
por archivo, las mide por separado y vuelve a comprobar los cuerpos ganadores
juntos. Además, reutiliza el objeto COFF: una comparación de 40 funciones
pasa de 0,448 a 0,105 segundos, con resultados idénticos. Ese factor 4,26 se
refiere a la comparación, no a todo el proceso. La implementación definitiva
pasa una comprobación nativa adicional de 189 variantes y 37 pruebas unitarias.
Los resultados, exclusiones y límites quedan en
[matching-packed-2026-10-03.json](matching-packed-2026-10-03.json).

## Funciones de lógica que priorizar

Todas las siguientes siguen sin prueba directa registrada. Los porcentajes de
esta tabla son **reccmp**, no la comparación de bytes. Son una cola de revisión,
no una lista de fallos confirmados.

| Dirección | Función/área | Matching | Revisión que falta |
| --- | --- | ---: | --- |
| `0x496e00` | Suelo y contactos, `CarPhysics.cpp` | 65,33% | Contactos, alturas, coche conducido/ghost y constantes sin resolver |

La cola completa está en [logic-pending.tsv](logic-pending.tsv), ordenada por
prioridad estática. La prioridad por módulo es aproximada: un mismo archivo
mezcla cálculo, presentación y efectos. La selección anterior usa el cometido
concreto de las funciones. [logic-audit.json](logic-audit.json) conserva por
función el motivo, ubicación, señales, pruebas y notas de revisión.

## Porcentajes bajos con comportamiento comprobado

| Entrada | Matching reccmp | Evidencia |
| --- | ---: | --- |
| `0x4781d0` `Car_UpdateSurfaceParams` | 48,92% | 2304 casos: 48 superficies, mezclas, compresión y resistencia; helpers reales |
| `0x489060` `Collision_RayQuad` | 40,00% | 1800 casos, 407 impactos y 1393 fallos; helpers reales |
| `0x491550` `Track_FindTriangle` | 47,57% | 420 casos de quadtree, hojas y límites; proveedor final de triángulo controlado |
| `0x424f20` `NetRace_PackCarState` | 48,38% | 960 casos, cuatro modos x87 y los 30 bytes del paquete; consultas de juego controladas |
| `0x420a30` `RallyData_UpdateCarRoute` | 47,00% | 961 casos, avance/retroceso, caché, rutas abiertas/cerradas y límite de ciclos; helpers reales |
| `0x456250` `StageTiming_RebuildSplitPositions` | 25,32% | Ya cubierta por el harness existente de rankings; no se clasifica como lógica sin prueba |

Los límites de los proveedores y los casos están en los propios harnesses y
en [tests/README-gameplay.md](../tests/README-gameplay.md). No se ha validado una
partida interactiva completa ni una sesión completa de red.

## Alcance adicional del inventario original

`scripts/functions.tsv` contiene 3646 entradas. Hay **298 direcciones originales
fuera del inventario de fuente**, y 15 direcciones de fuente que no están en
ese TSV. La unión comprende 3661 direcciones; no todas representan funciones
independientes de lógica del juego.

[logic-outside-source.tsv](logic-outside-source.tsv) lista esas 298 entradas:
57 comienzan con un salto y requieren revisar su destino, cuatro están medidas
como biblioteca enlazada, una (`0x41f788`) es una entrada interna del bloque de
rutas `0x41f560`, y 236 siguen sin clasificación concluyente. Entre ellas hay
entradas de SDK/CRT y artefactos del análisis. **No se dan por implementadas ni
se etiquetan todas como lógica de juego ausente.** Esta clasificación sigue
pendiente fuera de las 737 funciones de fuente sin prueba directa.

## Evidencia reproducible

La compilación conserva **2570 funciones exactas**, ninguna exacta perdida,
y **cero incidencias de datos**. Las 47 comprobaciones unitarias del compilador,
los límites de función y el análisis de instrucciones pasan.

El EXE probado tiene SHA-256
`87cf2afb650f2d66e20f24b71bb334378f91963022b5705ffd25ee279709cc44`.
[logic-tests.json](logic-tests.json) conserva hashes, salidas y resultados de
los 71 harnesses; [logic-audit.json](logic-audit.json) liga el inventario a esa
evidencia y al manifiesto de compilación. [logic-all.tsv](logic-all.tsv) permite
consultar las 3363 funciones, incluidas las exactas. Las notas manuales están
en [logic-notes.json](logic-notes.json).

Con el entorno de compilador/Wine configurado y después de compilar y medir:

```sh
python3 tests/run_differential_suite.py --jobs 3
python3 scripts/audit_logic.py
```

El análisis recorre instrucciones alcanzables para no confundir padding o
código vecino con lógica. La equivalencia exacta incluye ahora las tablas
embebidas de los `switch`; una regresión rechaza destinos intercambiados
aunque las instrucciones anteriores a la tabla sean idénticas. Marca 84 funciones con despacho indirecto cuyo flujo
no recorre completamente; requieren casos de sus ramas. Las diferencias de
llamadas y saltos pueden deberse a inlining o decisiones del compilador. No
quedan diferencias detectadas de limpieza de pila tras corregir el caso
confirmado, pero este análisis parcial no garantiza que no existan otras.

## Avance de matching del 2026-10-03

Se han integrado mejoras estrictas en 46 funciones de fuente mediante cinco
lotes y una corrección manual. Los lotes compilaron 6339 candidatos. Dos
funciones de resultados del frontend (`0x4e5c90` y `0x4e63d0`) pasan a exactas;
los otros dos nuevos exactos (`0x4057a8` y `0x4057ab`) corresponden a cuerpos
que ya coincidían y cuya comprobación estaba afectada por el padding. No se
atribuyen esos dos a una mejora del fuente.

El nuevo mutador intercambia ramas sin llaves con llamadas repartidas en
varias líneas y conserva la evaluación de la condición mediante negación
lógica. Se han ampliado los lotes a funciones de 801–10000 bytes. El registro
[matching de esta tanda](matching-search-2026-10-03.json) conserva el build,
los cambios por función, los resultados de los lotes y la ausencia de pérdidas
de exactos. Tras la revisión adicional de restauración física, quedan 801 funciones no exactas: el objetivo completo continúa
pendiente.

El registro [de restauración y lógica](matching-logic-2026-10-03.json) conserva
las mejoras de esa tanda, sus hashes y pruebas. La tanda posterior de red y
colisión se registra por separado arriba. Las mejoras posteriores de contactos
y tiempos se registran en [matching-contact-times-2026-10-03.json](matching-contact-times-2026-10-03.json).

## Cómo comprobar la lógica pendiente

De las 801 funciones no exactas, 58 pasan pruebas directas y 743 todavía
necesitan esa validación. Para ampliar la evidencia, cada grupo comparte un
estado válido y ejecuta original/recompilado con iguales entradas; se comparan
retornos, memoria, guardas, llamadas, argumentos y pila. Se añaden casos límite,
casos aleatorios reproducibles y, cuando haya captura de partidas, estados
reales. Hay que registrar las ramas ejercitadas, especialmente los 84 cuerpos
con despacho indirecto, y validar también las implementaciones de los helpers
que hoy son proveedores controlados. Ningún porcentaje sustituye ese trabajo.

Un fallo diferencial proporciona un caso concreto para corregir. Pasar todos
los casos ejecutados no demuestra equivalencia para toda entrada posible.
Para una garantía universal hace falta una prueba de equivalencia con contratos
de entrada, memoria, enteros, x87 y dependencias definidos; si un análisis
queda incompleto, el resultado debe seguir marcado como no demostrado.

## Prioridad actual y evidencia de confianza

La cola `validation-queue.json`/`.tsv` conserva las comprobaciones abiertas por
función. Las revisiones explícitas de matrices, integración de orientación y
esquinas del coche delimitan dominios y comprueban sensibilidad a fallos; su
vigencia depende de hashes de fuentes, compilador y harnesses. La cobertura no
prueba todos los caminos ni los proveedores controlados. La búsqueda principal
ahora compara ASM por función; las variantes se prueban después de identificar
una diferencia concreta, sin esperar a validar todo el juego.

`FUN_0050c130` tenía un defecto incluso al 96,27% en bytes: el último preview
consultaba el tag 1 del menú; el original consulta el 0. Una prueba con tags
distintos observa `preview(0, 1)` en el original y `preview(1, 1)` en la
compilación anterior. Corregido: pasan 756 fixtures del panel completo con
consultas reales de menú/juego/rally y hojas de texto/formato/render/clima/preview
controladas. Se comparan sus argumentos y orden, posiciones, colores, guardas
y ABI. Su matching local queda ahora al 100% exacto tras expresar las dos ramas de dibujo.

`Car_BuildViewOrder` (`0x428810`) y `FUN_0044b270` quedan exactas: se conserva
el retorno de byte de la consulta de flags y la forma de las comparaciones de
byte; el temporal del sorter usa la anchura del original. Las 69 pruebas pasan,
no cae ningún porcentaje ni se pierde ningún cuerpo exacto. Estrategia y
referencias: `validation-strategy.md`; recibo: `matching-validation-2026-10-03.json`.

## Revisión del cargador y matching manual

En `0x41f930`, la llamada final de iluminación tenía sus dos argumentos
invertidos. El original pasa primero el global `0x538238` y después `0x538234`.
La regresión de nueve estados de punteros falla en el ejecutable anterior y
pasa en el nuevo; también comprueba llamadas siguientes, retorno BYTE y pila.
Se conserva la prueba de 2688 casos de rutas. El cuerpo compilado completo
queda exacto, incluidos los cambios de strcat y orden de argumentos.

En `0x456330`, la escritura del cociente debe preceder a la recarga del primer
valor de la tabla. Un acceso volatile conserva esa programación de MSVC6 y
reproduce el cuerpo entero. Sus regresiones de tiempos permanecen registradas.
Las tres funciones nuevas se miden junto con las 3363 del inventario y se
verifican con toda la batería de 71 harnesses.

## Fusión de los cuatro commits de decomp/match3

Se han integrado los commits hasta `13c1f8b`, conservando el trabajo de main.
El resultado pasa de 2567 a **2570 cuerpos exactos**, con **793 pendientes**.
Quedan exactas `Sector_BuildCorners` (`0x4b8b90`), `FUN_0046c750` y
`FUN_0046cce0`. Mejoran once funciones en total, incluidas alturas y replay.
No se pierde ningún exacto ni baja ningún porcentaje: una compensación local
en `0x47aa70` conserva el orden de llamadas y sus instrucciones anteriores.
Pasan los 71 harnesses y las 47 pruebas unitarias; no hay incidencias de datos.
Las revisiones de matrices y esquinas se mantienen tras revisar los dos cambios
de cabecera y comprobar que sus cuerpos de máquina siguen idénticos.
Recibo: [merge-match3-2026-10-03.json](merge-match3-2026-10-03.json).

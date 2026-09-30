# Revisión de fidelidad — 29 de septiembre de 2026

Base: `decomp/callbacks`, commit `adfc56d`. Trabajo de esta revisión:
`decomp/matching-review`, en `/home/pablo/colin_mcrae_linux/matching-review`.

## Estado medido del lote inicial

Se compiló la base con MSVC6 y las opciones de `tools/build.sh`. Las medidas se
compararon contra ese HEAD, usando informes separados de los JSON compartidos.
La compilación existente de `/home/pablo/cmr2-callbacks` dio los mismos porcentajes
que la compilación limpia de la base.

| Medida | Base | Después del lote |
|---|---:|---:|
| Entradas medidas del inventario con matching >= 90 % | 2505 | 2507 |
| Entradas medidas del inventario con matching < 90 % | 841 | 839 |
| Entradas del inventario al 100 % | 2095 | 2098 |
| Matching medio de las implementadas | 89,43 % | 89,48 % |
| Funciones del juego sin empezar según `funcstat.py` | 0 | 0 |
| Incidencias de datos iniciales | 0 | 0 |

El inventario contiene 3646 entradas: incluye bibliotecas y artefactos del
análisis. `funcstat.py` identifica 3343 funciones del juego. La categoría de
matching inferior al 90 % suma unos **620 KB**, por lo que la media por función
oculta la dificultad de las rutinas grandes. La cobertura del inventario no
demuestra que todas las funciones tengan la lógica correcta ni que el análisis
del ejecutable haya descubierto todo el código.

## Cambios de este lote

| Dirección | Función | Antes | Después | Corrección |
|---|---|---:|---:|---|
| `0x431c10` | Reinicio con fade del coche | 8,00 % | 100,00 % | Callback recompilado y color empaquetado en cuatro bytes |
| `0x43ecd0` | Actualización de ejes del coche | 99,23 % | 100,00 % | Leer `Car::field_0xb1a`, evitando multiplicar el offset por `sizeof(Car)` |
| `0x49fd90` | `CInput::DInputReleaseDevices` | 94,02 % | 100,00 % | Recorrer cuatro slots y reproducir la comparación con signo del original |
| `0x48df50` | Deslizamiento y amortiguación del coche | 42,46 % | 82,35 % | Accesos en bytes y forma original de los valores absolutos |
| `0x402f90` | Constructor de opciones de red | 88,34 % | 90,80 % | Referencias reales a los callbacks |
| `0x4035e0` | Constructor de menú | 94,52 % | 95,89 % | Referencia real a su callback |
| `0x404000` | Constructor de menú | 79,00 % | 80,00 % | Referencia real a su callback |
| `0x457000` | Preparación de modelos del coche | 68,68 % | 70,91 % | Compartir la tabla de detalle que modifica Graphics |
| `0x4f8b30` | Tablas de identificadores del perfil | 60,58 % | 60,96 % | Límites relativos a los arrays recompilados |

También se corrigieron:

- `Sector.cpp`, `0x4b93c0`: el bucle de texturas avanzaba 10 + 19 dwords entre
  triángulos. El original avanza 19 en total, es decir, `0x4c` bytes.
- `GameInfo.cpp`, `0x5029b0`: el límite del bucle dependía de una dirección del
  ejecutable original. Ahora depende del array correspondiente.
- Global `0x537f34`: GameInfo definía un entero independiente del array de Race.
  GameInfo y NetRace usan ahora el array definido en Race.
- Global `0x51a3d0`: StageTiming leía una copia independiente de la tabla que
  Graphics modifica al cambiar la calidad. Ahora usa la misma tabla.
- Anotaciones repetidas de declaraciones `extern` y del callback `0x484310`:
  se conserva la anotación de cada definición. `check_dupes.py` queda limpio.

Parte de las correcciones de lógica ya existía en `port/silentpatch-window`,
especialmente en `4d59d62`; se recuperaron y verificaron para esta rama de
fidelidad. El color de `0x431c10`, la forma del cálculo de `0x48df50`, la
comparación de `0x49fd90` y la unificación de `0x51a3d0` se ajustaron durante esta
revisión contra el original.

## Validación y límites

- Compilación completa con MSVC6: correcta.
- Comparación completa contra la base: ninguna función medida desaparece.
- `reccmp-datacmp`: cero incidencias antes y después.
- `check_dupes.py`: limpio, 3361 direcciones de funciones y 3196 de globales.
- `git diff --check`: correcto.
- `tests/differential_slip.py`: ejecuta el código máquina original y recompilado
  de `0x48df50` con 6000 registros de coche. Compara el registro entero, sus
  guardas y el puntero global. Cero diferencias. Cambiar deliberadamente la
  constante de amortiguación de `0xf851` a `0xf850` se detecta en 5917 casos.

Cuatro porcentajes bajan al recolocar el binario. Se compararon las instrucciones
de las compilaciones base y final, preservando opcodes, registros, offsets de
campos y constantes; se resolvieron únicamente las direcciones de símbolos y
saltos. Las cuatro funciones conservan el mismo código tras esa reubicación:

| Dirección | Antes | Después | Instrucciones verificadas |
|---|---:|---:|---:|
| `0x428bf0` | 59,96 % | 59,78 % | 560 |
| `0x45f6d0` | 83,98 % | 83,12 % | 115 |
| `0x4984b0` | 85,45 % | 81,82 % | 55 |
| `0x505b40` | 61,86 % | 61,44 % | 236 |

Son variaciones de la resolución de operandos del comparador; no se cambió su
lógica. El porcentaje de reccmp compara ensamblado normalizado, y no certifica
igualdad literal de todos los bytes del ejecutable. La prueba diferencial cubre
una rutina de física; **no se ha validado una partida completa** con este lote.

## Siguiente trabajo, por riesgo funcional

1. **Vectores sobre la pila.** `SceneNode_Rotate` (`0x4ac820`) pasa direcciones
   de enteros sueltos a `FixVecScaleRecip`, que escribe tres componentes.
   `StageTiming.cpp`, `0x483100` y `0x484f40`, contiene el mismo patrón. Los
   escalares independientes no garantizan memoria contigua. Recuperar las
   correcciones de vectores reales de `4d59d62` y probar las rotaciones con un
   harness diferencial.
2. **Carga de geometría.** `GameInfo.cpp`, `0x5062d0`, todavía escribe en
   `0x831088` como dirección absoluta, usa el registro incorrecto del cargador y
   pierde una rama de recursos por comprobar una variable inicializada a cero.
   `4d59d62` contiene las correcciones del cargador y de sus nodos.
3. **Datos y arrays que se leen fuera de su objeto.** Entre los casos ya
   corregidos en la rama de ejecución: `0x82c070`, las curvas y nombres de audio,
   los slots de luces `0x547d00` y las tablas por jugador. Recuperar los cambios
   por grupos y comparar datos, límites y consumidores. Un `datacmp` limpio no
   detecta un array truncado ni una lectura del objeto vecino.
4. **Layout de Car y globals vecinos.** `overlap.py` sigue avisando de tamaños
   que cruzan el siguiente global (`g_cars`, `g_carOrder`, `g_carViewScale` y
   otros). Algunas vistas son intencionadas; hay que comprobar cada caso con el
   ensamblado y modelar el almacenamiento compartido explícitamente.
5. **Matching residual.** Después de cerrar estos riesgos, atacar la banda
   85–99 % y las rutinas grandes con diferencias de llamadas, ramas, anchuras y
   offsets. Reservar las diferencias exclusivamente de registros o slots de
   pila para una pasada posterior.

## Precauciones con las herramientas locales

- La carpeta inicialmente abierta está en `decomp/frontend`, no en `callbacks`.
  Los handoffs antiguos tienen cifras y prioridades anteriores a terminar la
  implementación del inventario.
- `tools/current.json` y `baseline.json` son compartidos. Usar un informe propio
  del HEAD que se está revisando; no refrescar el baseline de otra rama.
- `opsim.py` y `callcount.py` todavía fijan la ruta del repositorio antiguo y no
  respetan `CMR2_REPO`. Comprobar la imagen usada antes de aceptar sus resultados.
- `check.sh` y `match.sh` pueden continuar tras un error de compilación si existe
  un `.exe` anterior. Exigir éxito de `build.sh` antes de medir.
- El aviso de un global duplicado puede ser una declaración `extern` repetida,
  o dos objetos reales. Distinguirlos antes de unificar datos.

Reproducir las comprobaciones desde esta carpeta:

```bash
export CMR2_REPO="$PWD"
export WINEPREFIX=/home/pablo/colin_mcrae_linux/tools/wineprefix
export WINEDEBUG=-all
/home/pablo/colin_mcrae_linux/tools/build.sh && \
  reccmp-reccmp --target CMR2 --no-color --silent \
    --json /tmp/cmr2-review.json --json-diet
reccmp-datacmp --target CMR2
python3 /home/pablo/colin_mcrae_linux/tools/check_dupes.py
python3 tests/differential_slip.py /tmp/cmr2-review.json
```

## Hito: menos de 700 funciones por debajo del 100 %

Se cuenta `matching < 1.0` en **todas** las 3364 funciones del informe de
reccmp, sin excluir rutinas ni aceptar `effective_accuracy` como coincidencia
exacta. Este recuento es distinto del inventario inferior al 90 % de arriba.

| Lote | Inferiores al 100 % | Nuevas al 100 % | Regresiones |
|---|---:|---:|---:|
| Inicio del hito (`c11fb4d`) | 1256 | — | — |
| Condiciones y campos de menús (`a9247e9`) | 1233 | 23 | 0 |
| Funciones bajas, vectores y cargador | 1227 | 6 | 2 parciales, verificadas diferencialmente |
| Tablas, colas y funciones bajas | 1221 | 6 | 1 parcial, verificada diferencialmente |

El primer lote reproduce las condiciones con signo y sin signo, el ancho del
retorno de `0x407270`, la resta posterior al cálculo de longitud y el límite
de siete jugadores. En seis constructores (`0x4f6e50`, `0x4f6f10`, `0x4f6fd0`,
`0x4f7090`, `0x4f7150`, `0x4f9490`), el original escribe la selección en
`Menu::items[0].max` (offset `0x1f`), mientras el fuente escribía en `cursor`
(offset `7`). Todos llegan al 100 % con la corrección.

Por indicación del usuario, los lotes siguientes priorizan las funciones de
porcentaje bajo y errores funcionales, conservando el criterio estricto del
hito. Compilación completa correcta; informe propio en
`/tmp/cmr2-milestone-batch1.json`; ninguna función desaparece.

### Lote de funciones bajas

| Dirección | Antes | Después | Cambio |
|---|---:|---:|---|
| `0x416710` | 0 % (stub) | 100 % | Recuperar el salto a la inicialización de sonidos del copiloto |
| `0x477ac0` | 18,18 % | 100 % | Treinta escrituras de palabras desenrolladas como en el original |
| `0x4923d0` | 14,71 % | 100 % | Color empaquetado con una vista de bytes |
| `0x492470` | 13,33 % | 100 % | Vista de bytes y estado de iluminación externo |
| `0x492520` | 13,89 % | 100 % | Vista de bytes y estado de iluminación externo |
| `0x4aacf0` | 22,22 % | 100 % | Cargar externamente el contador de conexiones como byte |
| `0x4657d0` | 9,09 % | 67,36 % | `FixVecScale` y copia de vector en los puntos de escape |
| `0x4814d0` | 13,11 % | 76,47 % | Inicialización y ramas de los tres ángulos |
| `0x483050` | 5,41 % | 36,59 % | Bases, comparación y escritura compartida de la oscilación |
| `0x456bb0` | 16,67 % | 80 % | Limpiar la tabla en grupos de dos registros |
| `0x40d520` | 18,84 % | 37,04 % | Bucle exterior por puntero y switch de dirección |
| `0x5062d0` | 46,21 % | 56,01 % | Recuperar el cargador, los recursos L/S y los slots correctos |
| `0x483100` | 40,19 % | 44,84 % | Vectores contiguos en los cálculos de piezas del coche |
| `0x484f40` | 25,37 % | 26,87 % | Vectores contiguos para la normal y la tangente |

`TrackLighting.cpp` contiene las tres rutinas de color; `GameNetwork.cpp`, el
acceso al contador de conexiones. Las definiciones de datos siguen compartidas
con sus módulos. Separar estos consumidores reproduce las lecturas de byte
externo de MSVC6; ambos ficheros están registrados en el proyecto y sus filtros.

`StageSplitRankings` modela las cuatro tablas adyacentes en `0x541fac..0x542197`.
La eliminación de un piloto (`0x455bc0`) utiliza el bloque común como el original.
La reconstrucción (`0x456250`) cambia de 25,61 % a 22,36 % por el código generado
con el almacenamiento compartido. `SceneNode_Rotate` (`0x4ac820`) pasa de 65,69 %
a 65,32 % al sustituir los enteros sueltos por vectores contiguos. Estas dos
bajadas **no se ocultan**: las pruebas ejecutan el código máquina original y
recompilado con las mismas entradas y comparan la memoria completa.

Pruebas diferenciales de este lote:

- `tests/differential_rotation.py`: 6000 rotaciones, ángulos límite, seis bases y
  cadenas de uno a tres padres; compara nodos enteros, entradas, guardas y globals
  de trabajo. Cero diferencias.
- `tests/differential_rankings.py`: 6000 eliminaciones y 6000 reconstrucciones,
  tablas permutadas, 0–9 splits y tamaños límite; cero diferencias y guardas
  intactas. El único callee simulado devuelve el número de splits elegido.
- `tests/differential_sort.py`: 6000 ordenaciones con empates, direcciones válidas
  e inválidas y tamaños -2..16; cero diferencias, tiempos y guardas intactos.

Validación final: compilación completa correcta, `reccmp-datacmp` con cero
incidencias, `check_dupes.py` limpio (3362 funciones y cero stubs). Las pruebas
diferenciales del build final suman 24.000 casos, todos sin diferencias.

Informes separados de los compartidos: `/tmp/cmr2-low-final.json`,
`/tmp/cmr2-low-final-entities.json`. Todas las 3364 funciones siguen medidas;
1227 inferiores al 100 %, 837 inferiores al 90 %. El hito de menos de 700 al
100 % sigue pendiente: faltan al menos **528** mejoras netas al 100 %.

La corrección del cargador se recuperó de `4d59d62` y se contrastó con el
ensamblado. No se ha ejecutado una partida completa en esta rama. Las pruebas
de rotación y rankings validan sus rutinas concretas; no certifican todo el motor.

Reproducir las pruebas desde esta carpeta después de compilar y medir:

```bash
python3 tests/differential_rotation.py /tmp/cmr2-low-final.json
python3 tests/differential_rankings.py /tmp/cmr2-low-final.json
python3 tests/differential_sort.py /tmp/cmr2-low-final.json
```

Los dos primeros scripts resuelven los símbolos del build actual; opcionalmente
aceptan un mapa JSON como segundo argumento. Rechazan un mapa cuya dirección de
la función no coincida con el informe.

### Tablas, colas y funciones bajas

Última medida completa: **3364 funciones**, **2143 al 100 %**, **1221 por debajo
del 100 %** y **830 por debajo del 90 %**. Son 35 coincidencias exactas más
desde el inicio del hito; quedan 522 adicionales para llegar a 699.

| Dirección | Antes de este lote | Después | Cambio |
|---|---:|---:|---|
| `0x4b9ff0` | 39,25 % | 100 % | Matrices locales 4×4, índices, operando B materializado y retorno del puntero de salida |
| `0x4eb340` | 43,48 % | 100 % | Obtener la ruta del perfil antes de preparar la escritura |
| `0x4789d0` | 43,48 % | 100 % | Conservar base, siguiente superficie y diferencia antes de `FixMul` |
| `0x46c4e0` | 52,63 % | 100 % | Obtener el coche antes de preparar el buffer de replay |
| `0x46ed40` | 41,38 % | 100 % | Retorno BYTE, comparación con `0x100` y una sola tabla de eventos |
| `0x4b7ca0` | 69,57 % | 100 % | Límite propio de 30 casillas, forma del bucle y bloque contiguo de colas |
| `0x4b7d10` | 59,26 % | 92,86 % | Misma forma de inserción para las teclas |
| `0x46e530` | 66,67 % | 71,79 % | Eliminar los alias independientes de la tabla |
| `0x46e6a0` | 96,20 % | 98,73 % | La misma tabla compartida; queda un orden de instrucciones distinto |
| `0x46ea80` | 45,12 % | 48,68 % | Leer ancho y alto de los eventos que realmente se inicializan |
| `0x40cc60` | 44,44 % | 24,14 % | Limpiar los ocho registros y tablas reales, sin depender del objeto vecino |

Bugs corregidos:

- La cola de caracteres usaba el array de teclas como límite. En el build
  anterior ese array estaba **antes** del de caracteres (`0x544f60` frente a
  `0x546654`): una casilla ocupada bastaba para abandonar la inserción. Las dos
  colas de 30 enteros se almacenan ahora en `InputQueues`, como su región
  contigua original, y cada bucle respeta su límite propio.
- El reset del campeonato hacía `p[-14]` desde la tabla de tiempos y terminaba
  en la dirección de una tabla de posiciones independiente. En el build anterior
  los tiempos empezaban en `0x5c5764`, pero la tabla que se quería limpiar estaba
  en `0x5c579c`; el límite `0x5c58a0` tampoco equivalía a ocho registros.
  Se accede por nombre a la tabla correcta y se recorren ocho posiciones.
  Su bajada de porcentaje se conserva: el nuevo código pasa la comparación
  diferencial y respeta todos los buffers y guardas.
- `0x588edc` y `0x588f16` eran arrays separados que duplicaban los campos de
  `g_eventRecords`. Polvo y derrapes leían dimensiones distintas de las que
  escribe `Events_Add`. Ahora se usan `EventRec::a` y `EventRec::b` en la tabla
  compartida. La capacidad real es 11 registros de `0x1c` bytes, que terminan
  antes de `g_eventDraws` (`0x589010`), no 29 registros que se solapan con ella.
- Las dos texturas y las dos escalas de eventos estaban declaradas como
  escalares y se indexaban a través de su dirección. Ahora tienen arrays de
  dos elementos, que respaldan los accesos por slot del original.

`Font_GetTextWidth` se detiene en el primer separador también en el original.
Se corrigió el comentario que prometía calcular la línea más ancha.

Validación:

- Build completo con MSVC6 correcto, ninguna función desaparece y ningún
  matching exacto anterior pasa a parcial.
- `reccmp-datacmp`: **3189 variables, cero incidencias**. Eliminar variables
  duplicadas no altera el denominador de funciones del hito.
- `tests/differential_tables.py`: 6000 resets, 6000 secuencias de colas y 6000
  avances de eventos; código máquina original y recompilado, conservando las
  distancias entre globals de cada build. Cero diferencias y guardas intactas.
  Dos mutaciones que reintroducen direcciones erróneas sí se detectan.
- `check_dupes.py` y `git diff --check`: correctos.

Informes privados: `/tmp/cmr2-tables-final.json` y
`/tmp/cmr2-tables-final-entities.json`. Para reproducir la prueba:

```bash
python3 tests/differential_tables.py /tmp/cmr2-tables-final.json \
  /tmp/cmr2-tables-final-entities.json
```

La partida completa sigue sin validarse en esta rama. El hito de menos de 700
funciones inferiores al 100 % permanece pendiente.

## Lote de llamadas, índices y control de flujo

Base `c1cbbe6`. Medición completa con el campo estricto `matching`:
**3364 funciones, 2149 al 100 %, 1215 por debajo del 100 %, 824 por debajo
 del 90 %**. Se ganan seis exactas en este lote y 41 desde la base del hito
(`c11fb4d`, 1256 pendientes). Faltan 516 mejoras netas para llegar a 699.

| Función | Antes | Después | Cambio confirmado |
|---|---:|---:|---|
| `0x4765e0` | 39,34 % | 100 % | Releer el slot del objeto después de cada llamada |
| `0x45af00` | 40,37 % | 100 % | Conservar el índice del coche al empaquetar el color |
| `0x455620` | 42,42 % | 100 % | Contador entero extendido desde BYTE y orden original de ramas |
| `0x408bd0` | 44 % | 100 % | Índice int; actualizar también declaración y llamador |
| `0x419b50` | 53,33 % | 100 % | Conservar el módulo de rand y devolverlo sin recalcular |
| `0x459320` | 60,87 % | 100 % | Separar índice y resultado del checkpoint anterior |
| `0x423900` | 52,78 % | 63,89 % | Orden original de los cuerpos del switch; registros pendientes |

El bug más relevante del lote estaba en `0x45af00`: escribía cuatro bytes de
color sobre `car` y luego lo usaba como índice en todas las llamadas siguientes.
El original conserva el índice en ESI y usa la pila para el color. La corrección
reproduce exactamente esa separación, incluido el camino sin color disponible.

Validación del lote:

- Build completo MSVC6 correcto. No desaparecen funciones; ninguna exacta
  anterior pasa a parcial. Las siete variaciones del informe son las previstas.
- `tests/differential_effect_calls.py`: 6000 ejecuciones del código máquina
  original y recompilado con callees simulados. Mismos argumentos y orden de
  llamadas; datos del coche y guardas intactos. El código anterior guardado
  antes del build produce 480 diferencias: la prueba detecta el bug corregido.
- `reccmp-datacmp`: 3189 variables y cero incidencias.
- `check_dupes.py`: limpio; `git diff --check`: correcto.

Informes privados: `/tmp/cmr2-calls-final.json` y
`/tmp/cmr2-calls-final-entities.json`. Reproducir:

```bash
python3 tests/differential_effect_calls.py /tmp/cmr2-calls-final.json \
  /tmp/cmr2-calls-final-entities.json
```

La prueba verifica este dispatcher, no toda la física de las funciones que
llama ni una partida completa. Las técnicas confirmadas quedan recogidas en
`tools/CONOCIMIENTO.md`, apartado 12.4. El hito de menos de 700 sigue pendiente.

## Lote de dispatchers y activación de billboards

Base `90f69b6`. Informe completo estricto: **3364 funciones, 2152 al 100 %,
1212 por debajo del 100 % y 821 por debajo del 90 %**. Tres exactas nuevas,
ninguna desaparecida y ninguna exacta anterior regresada. Son 44 mejoras
netas desde `c11fb4d`; faltan 513 para alcanzar 699 pendientes.

| Función | Antes | Después | Cambio |
|---|---:|---:|---|
| `0x486b90` | 51,28 % | 100 % | Asignar el valor en ambas ramas y compartir el store en el fuente |
| `0x476540` | 62,86 % | 100 % | Siete stores sobre el global e incrementos acumulados del puntero |
| `0x486b20` | 68,35 % | 100 % | El mismo patrón del dispatcher de tipos |
| `0x4b1150` | 44,90 % | 75,86 % | Activar billboards y recuperar la estructura del generador de índices |

`0x4b1150` omitía el store `g_billboardsEnabled = 1` que el original hace
antes de registrar el callback. Sin él, `Billboard_Add` seguía rechazando
solicitudes porque el sistema figuraba como desactivado. Se conserva la
capacidad de 800 quads y los 4800 índices, y se restauran el contador
descendente y los incrementos de vértice. La diferencia restante corresponde
al punto elegido por el compilador para el cursor dentro de cada grupo.

Validación:

- Build completo MSVC6 correcto; reccmp mide las 3364 funciones. Solo cambian
  las cuatro funciones previstas; las tres coincidencias exactas se confirman
  en el ejecutable completo, además de la compilación aislada.
- `tests/differential_billboard_init.py`: 6000 inicializaciones con código
  máquina real de ambas imágenes, conservando las distancias entre globals.
  Todos los índices y los tres flags/contadores tienen los valores esperados;
  la identidad del callback y los valores visibles al registrarlo coinciden.
  Guardas intactas, incluidos los huecos entre globals. Una mutación que quita
  el store de activación falla en 4000 casos.
- `reccmp-datacmp`: **3189 variables, cero incidencias**.
- `check_dupes.py` y `git diff --check`: correctos.

Informes: `/tmp/cmr2-dispatch-final.json` y
`/tmp/cmr2-dispatch-final-entities.json`. El volcado detallado de diferencias
`/tmp/cmr2-dispatch-final-scan.json` permite buscar causas compartidas sin
volver a usar el antiguo scan de `c11fb4d` como si fuese el estado actual.

```bash
python3 tests/differential_billboard_init.py /tmp/cmr2-dispatch-final.json \
  /tmp/cmr2-dispatch-final-entities.json
```

Técnicas y variantes descartadas: `tools/CONOCIMIENTO.md`, apartado 12.5.
La presentación gráfica completa sigue sin validarse y el hito permanece activo.

## Lote de búsqueda de menú, slots de replay y rectángulos de pantalla

Base `d1736c6`. Medida completa: **3364 funciones, 2159 al 100 %, 1205
por debajo del 100 % y 816 por debajo del 90 %**. Siete exactas nuevas,
sin funciones desaparecidas ni regresiones de exactas anteriores. Son **51
mejoras netas desde `c11fb4d`**; faltan 506 para alcanzar 699 pendientes.

| Función | Antes | Después | Corrección |
|---|---:|---:|---|
| `0x464b60` | 20,93 % | 100 % | Releer dimensiones y calcular las mitades antes de los stores de cero |
| `0x4d0820` | 70,59 % | 100 % | Retorno BYTE del callback de destrucción del splash |
| `0x4ebee0` | 81,82 % | 100 % | Retorno BYTE y bloque de copia/liberación en la rama no nula |
| `0x4a1610` | 91,53 % | 100 % | Índice BYTE, incluida la declaración del llamador |
| `0x502b10` | 91,49 % | 100 % | Resultado BYTE de la comparación de opciones |
| `0x4b7da0` | 94,44 % | 100 % | Comparación unsigned que conserva el salto del original |
| `0x4a0380` | 95 % | 100 % | Buscar en el campo del item situado en +8 |
| `0x46d270` | 88,24 % | 94,12 % | Recorrer los dieciséis slots, con límite y comparación con signo |
| `0x46d5e0` | 88,24 % | 94,12 % | El mismo arreglo del dispatcher de grabación |
| `0x46d510` | 95,24 % | 98,41 % | Dieciséis slots y comparación unsigned del flag BYTE |
| `0x46e440` | 97,14 % | 98,57 % | Recorrer también el segundo grupo de slots |

Dos causas de bugs quedan corregidas:

- `Menu_FindItem` buscaba en +4, aunque el original busca en +8. Los seis
  constructores ya coincidían y escribían correctamente ambos campos; no se
  cambia el tamaño del item ni se intercambian los campos de sus constructores.
- Los slots de replay de `0x588d40` y `0x588d60` eran arrays independientes de
  ocho entradas. Cuatro recorridos solo procesaban el primer grupo, mientras
  `0x46c8e0` recorría dieciséis sobre un array de ocho. Ahora existe un único
  array de dieciséis, con una vista del segundo grupo en +8, y los cinco
  recorridos usan su límite real. El inicializador conserva el 100 %. El
  stepping `0x46c8e0` mantiene su matching parcial, con el acceso fuera del
  array eliminado.

Los cuatro replays mejorados siguen pendientes: el límite one-past del array
coincide en la imagen recompilada con otro símbolo global, y reccmp muestra
ese nombre en lugar del límite original sin símbolo. No se cuentan como
exactos ni se modifica el comparador para ocultar esa diferencia.

Validación:

- Build completo MSVC6 correcto. El informe completo solo cambia las once
  funciones de la tabla; las 3364 entradas originales siguen medidas.
- `tests/differential_menu_lookup.py`: 6000 búsquedas con campos +4/+8
  distintos, tags repetidos, WORD con signo y consultas ausentes. Cero
  diferencias; datos y guardas intactos. Restaurar el acceso a +4 produce
  **1692** diferencias.
- `tests/differential_replay_slots.py`: 6000 inicializaciones y 6000 llamadas
  a cada dispatcher con código máquina real. Verifica los dieciséis punteros,
  resets, orden/argumentos de llamadas y memoria protegida, conservando las
  distancias entre globals de cada imagen. Cero diferencias. Reducir el límite
  a ocho falla en **6000 casos de playback y 6000 de grabación**. Sus callees
  están simulados; no verifica la decodificación ni la física del replay.
- `reccmp-datacmp`: **3188 variables, cero incidencias**. La segunda mitad
  del array deja de anotarse como un objeto independiente; los 64 bytes se
  comprueban en el objeto único.
- `check_dupes.py` limpio; `git diff --check` correcto.

Informes privados completos: `/tmp/cmr2-menu-replay-view-final.json`,
`/tmp/cmr2-menu-replay-view-final-entities.json` y
`/tmp/cmr2-menu-replay-view-final-scan.json`. Reproducir las pruebas:

```bash
python3 tests/differential_menu_lookup.py /tmp/cmr2-menu-replay-view-final.json
python3 tests/differential_replay_slots.py /tmp/cmr2-menu-replay-view-final.json \
  /tmp/cmr2-menu-replay-view-final-entities.json
```

Técnicas y variantes descartadas: `tools/CONOCIMIENTO.md`, apartado 12.6.
El hito de menos de 700 funciones sin matching exacto sigue activo.

## Segundo lote de 50 exactas — 30 de septiembre de 2026

Base: `d19e7a9`, misma rama `decomp/matching-review`. Medición completa de
ambas imágenes, con informes privados para no confundir la base del lote con
el baseline compartido de check.sh.

| Medida | Base | Final |
|---|---:|---:|
| Funciones medidas | 3364 | 3364 |
| Al 100 % | 2159 | **2209** |
| Por debajo del 100 % | 1205 | **1155** |
| Por debajo del 90 % | 816 | **803** |
| Exactas previas que dejan de serlo | — | **0** |
| Funciones desaparecidas | — | **0** |
| Incidencias de datos iniciales | 0 | **0** |

Son **50 nuevas exactas**, trece desde menos del 90 %. El detalle completo,
con dirección, nombre y porcentaje inicial, está en
[tests/matching_next50.tsv](tests/matching_next50.tsv).

Se corrigen accesos de índice cero a tablas de red que habían quedado
independientes, un flag de Race duplicado dentro de un array, el return de
«atrás» que faltaba y el puntero a medio rectángulo de una pantalla. También
se ajustan bytes, comparaciones, máscaras, registros y orden de stores.
Las técnicas y variantes descartadas se documentan en `tools/CONOCIMIENTO.md`
§12.7 (ruta compartida `/home/pablo/colin_mcrae_linux/tools/CONOCIMIENTO.md`).

Validación:

- Compilación completa y `tools/check.sh` correctos. Los avisos de regresión
  de su baseline antiguo no son el delta de este lote; el informe completo
  contra d19e7a9 confirma 50 nuevas, cero exactas perdidas y cero desaparecidas.
- `reccmp-datacmp`: **3173 variables, cero incidencias**. Persisten avisos de
  aliases GLOBAL que ya estaban en la base.
- `check_dupes.py`: 3362 FUNCTION, cero STUB, 3174 globals; limpio según el
  criterio del script. `overlap.py`: **28 -> 27**, ningún solapamiento nuevo.
- **6000** casos de getters/actualizaciones de red: cero diferencias en
  ambas regiones completas y guardas; mutaciones detectadas en 6000 y 5571.
- **6000** ensamblados de resultados de red: cero diferencias; comparadores
  nativos, qsort real, contenido completo, llamadas y guardas comprobados.
- **6000** callbacks de atrás: cero diferencias; la mutación sin return
  produce llamadas adicionales en 4800 casos.
- `git diff --check`: correcto.

Hay cuatro descensos entre funciones que ya eran parciales: `0x40a580`,
89,57 -> 82,46 %; `0x40b880`, 27,13 -> 26,53 %; `0x4d6f10`, 60,11 -> 56,71 %;
`0x4d7380`, 62,79 -> 59,52 %. La prueba nueva de resultados cubre 0x40a580.
Los otros son llamadores afectados por el layout de red y el índice BYTE de
Font_DrawText y continúan pendientes. Las pruebas no validan transporte de
red, todos los renderers ni una partida completa. El 100 % aquí es el matching
normalizado de reccmp; no igualdad literal de dos archivos PE reubicados.

Informes finales: `/tmp/cmr2-next50-final.json`,
`/tmp/cmr2-next50-final-scan.json` y `/tmp/cmr2-next50-final-entities.json`.
Logs con el mismo prefijo: check, data, tables-test, results-test y back-test.

## Lote de 100 exactas — 30 de septiembre de 2026

Base **`0ec1923`**, rama `decomp/matching-review`. El cierre añade **101
funciones al 100 % estricto**: cien del lote y el retorno del diálogo de CD.
Catorce partían de menos del 90 %. La lista completa está en
[tests/matching_next100.tsv](tests/matching_next100.tsv).

| Medida | Base | Final |
|---|---:|---:|
| Funciones medidas | 3364 | 3364 |
| Al 100 % estricto | 2209 | **2310** |
| Por debajo del 100 % | 1155 | **1054** |
| Por debajo del 90 % | 803 | **782** |
| Exactas anteriores perdidas | — | **0** |
| Funciones desaparecidas | — | **0** |
| Incidencias de datos iniciales | 0 | **0** |

La estrategia fue agrupar causas compartidas: tipos BYTE/WORD y ABI de los
llamadores, bloques de datos realmente contiguos, salidas de APIs COM,
callbacks con sus firmas reales y tablas de zlib. Se probaron variantes por
unidad de compilación y se aceptaron tras una comparación completa de las
imágenes enlazadas. No se modificó el criterio del comparador ni se retiraron
funciones parciales de la medición.

### Correcciones de comportamiento

- La caché de opciones tenía solo los 48 bytes de flags, pero copiaba cuatro
  fichas de 0x148 bytes detrás. `PlayerOptionCache` proporciona sus **0x550
  bytes reales**. Backup y restore también quedan exactos.
- `0x505a60` intercambiaba solo un campo entre jugadores. Ahora intercambia
  los **diez** del original, conservando el orden de stores y el sentinel -1.
- `0x40d010` usaba +65536 al aplicar penalizaciones. El original usa el
  DOUBLE **-65536** y penalizaciones char con signo; la prueba nativa cubre
  los valores -128..127. La función sigue parcialmente emparejada.
- El parpadeo `0x5004c0` devuelve solo el **bit 0**; faltaba `& 1`. Su matching
  baja por instrucciones redundantes del original, pero la lógica ya coincide.
- Tres constructores de menús apuntaban a sobrecargas vacías. Se eliminan
  esas copias y se usan `0x4ecaf0`, `0x4de1d0` y `0x4f0da0` con sus firmas
  reales. Se verifican los destinos en el PE final y sus RET 4/4/8.
- DirectPlay recibe un `DPNAME` real y **la dirección** del DPID de salida,
  en vez de su valor. La ordenación de proveedores compara **IPX**, cuyo
  GUID original está en 0x511a38, en lugar de TCP/IP. Solo anotar el GUID
  equivocado daba 100 % de instrucciones: datacmp detectó la diferencia.
- Se restablece el paso del HRESULT de `SetCurrentPosition(0)` por el helper
  de sonido, se limpian los **100** contadores al liberar vertex buffers y
  el filtro de modo gráfico lee `dwRGBBitCount`, en vez de `dwFlags`.
- La propiedad de joystick usa `sizeof(DIPROPHEADER)` (**16**), no el tamaño
  completo de DIPROPRANGE (24). Los valores iniciales 1/2/3 del menú 0x4035e0
  van al argumento `value`, no a `param`.
- El diálogo de CD solo devuelve TRUE para **IDRETRY**; Cancel cierra y sale,
  y cualquier otro resultado devuelve FALSE. Queda al 100 %.
- Se elimina el segundo borrado de Y del eje de partículas `0x498370`, que
  anulaba el resultado de la normalización y no existe en el original.

Los bloques de standings/splits, clasificación de ocho entradas, archivos de
stage, nodos/flags, replay, dispositivos/gains, rankings, rastro de puntos y
timers conservan sus campos vecinos reales. zlib conserva sus tablas CRC,
Huffman, longitudes, distancias, configuración, máscaras y mensajes; se
emparejan sus datos originales y sus promociones enteras, sin añadir asm.

### Validación

- Build MSVC6 y `tools/check.sh`: correctos. El conteo anterior usa
  `Compare.compare_all().accuracy`, no el progreso agregado ni la precisión
  efectiva que acepta algunos cambios de registros.
- `reccmp-datacmp`: **3201 variables, cero incidencias**. Persisten los
  aliases GLOBAL que ya avisaban en la base.
- `check_dupes.py`: **3362 FUNCTION, cero STUB, 3195 globals**, limpio según
  el criterio del script. También se revisaron las anotaciones de las tablas
  dentro de `zlib/trees.h`, que el script no recorre.
- `overlap.py`: **27 -> 27**, listado idéntico al de la base, sin nuevos
  solapamientos. `git diff --check`: correcto.

| Prueba nativa | Casos | Resultado |
|---|---:|---|
| Texto y coordenadas | 6000 | Cero diferencias, 134013 dibujos de glyphs |
| DirectPlay, sonido, vertex buffers | 18000 | Cero diferencias en outputs, HRESULTs, llamadas y guardas |
| Caché, intercambio, penalizaciones, parpadeo | 30000 | Cero diferencias y modelo independiente de memoria |
| Getters/updates de tablas de red | 6000 | Cero diferencias; mutaciones detectadas en 6000/5571 |
| Resultados de red y comparador nativo | 6000 | Cero diferencias; qsort, bloque completo y guardas |
| Replay | 6000 por rutina | Inicializador y dos dispatchers correctos en los 16 slots; mutaciones fallan 6000/6000 |

Las pruebas ejecutan los cuerpos de ambas imágenes. Simulan proveedores de
estado, métodos COM y operaciones de rasterización; no validan una partida
completa ni transporte de red. La prueba de intercambio cubre los **cuatro
jugadores configurables** y -1; la declaración antigua de 16 records sigue
solapándose con controles en slots superiores, y no se da por validada esa
zona. Las coordenadas de DrawText son WORD con signo; un llamador conserva
sus slots DWORD del ABI x86, consumidos por sus 16 bits bajos.

### Funciones parciales cuyo porcentaje baja

No se pierde ninguna exacta. Estas diez parciales siguen pendientes, y no se
ocultan de la medición:

| Dirección | Base | Final |
|---|---:|---:|
| `0x40a820` | 96.33 % | 87.27 % |
| `0x40d820` | 64.16 % | 62.46 % |
| `0x452be0` | 92.48 % | 91.02 % |
| `0x4d3360` | 73.61 % | 71.62 % |
| `0x4dfe20` | 51.67 % | 49.70 % |
| `0x4e1d70` | 51.67 % | 35.71 % |
| `0x4e7ed0` | 59.34 % | 58.96 % |
| `0x4e8500` | 65.64 % | 64.92 % |
| `0x4fccb0` | 57.19 % | 56.95 % |
| `0x5004c0` | 83.87 % | 75.86 % |

Las variaciones proceden de los layouts y firmas corregidos; el parpadeo
incluye una corrección de lógica probada. Los renderers afectados aún no
tienen prueba completa de sus llamadores.

Informes reproducibles y base inmutable en `tools/matching-next100-base*` y
`tools/matching-next100-final*` (JSON completo, entidades y scan). Logs de
verificación con el prefijo `tools/matching-next100-final-`. La copia de
trabajo de esos informes también está en `/tmp/cmr2-next100-final*`.

```bash
python3 tests/differential_font_coordinates.py /tmp/cmr2-next100-final.json /tmp/cmr2-next100-final-entities.json
python3 tests/differential_com_outputs.py /tmp/cmr2-next100-final.json /tmp/cmr2-next100-final-entities.json
python3 tests/differential_option_cache.py /tmp/cmr2-next100-final.json /tmp/cmr2-next100-final-entities.json
python3 tests/differential_network_tables.py /tmp/cmr2-next100-final.json /tmp/cmr2-next100-final-entities.json
python3 tests/differential_network_results.py /tmp/cmr2-next100-final.json /tmp/cmr2-next100-final-entities.json
python3 tests/differential_replay_slots.py /tmp/cmr2-next100-final.json /tmp/cmr2-next100-final-entities.json
```

Técnicas, variantes descartadas y próximos candidatos: `tools/CONOCIMIENTO.md`
**§12.8**, en la carpeta compartida del proyecto.

## Comparación byte a byte y permuter — 30 de septiembre de 2026

Se adopta el flujo de trabajo de las descompilaciones de referencia (objdiff /
asm-differ para iterar por unidad de compilación, decomp-permuter para las
diferencias de registros y orden) adaptado a MSVC6. Las herramientas viven en
`tools/fastcmp/`, fuera del repositorio, como el resto del entorno local.

| Herramienta | Uso |
|---|---|
| `fastcmp.py 0xADDR [--diff]` | Compila **solo** el TU (~0,7 s, mismos flags que `build.sh`), extrae la función del `.obj`, traduce sus relocaciones a direcciones del original y compara **bytes**. |
| `bytecount.py OUT.json [BASE.json]` | Las 3362 funciones en ~10 s, en paralelo, con diferencias frente a una base. |
| `tryv.py 0xADDR v.py` | Compila N variantes del fuente en paralelo y las puntúa (`--pick N` aplica una). |
| `permute.py 0xADDR [--apply-exact]` | Permuter: mover sentencias, conmutar operandos, invertir `if/else`, separar variables, sustituir temporales; hill climbing. |
| `rollscan.py`, `origrefs.py`, `tiebreak.py`, `classify.py`, `callorder.py` | Barridos de patrones concretos (ver abajo). |

`fastcmp` coincide con reccmp en 3288 de 3357 funciones y descubrió **67
funciones idénticas byte a byte que reccmp puntúa por debajo del 100 %**:
reccmp nombra cada operando con la tabla de símbolos de su propia imagen
(la constante `0x800000` coincide con otro símbolo, un puntero de fin de array
cae en el global vecino). Un resultado EXACT de `fastcmp` es código idéntico
al original, así que no requiere revisión semántica; las mejoras parciales
del permuter sí se revisan antes de aplicarlas.

| Medida | Inicio (`4c421e7`) | Ahora |
|---|---:|---:|
| Byte a byte exactas (`fastcmp`) | 2391 | **2423** |
| reccmp estricto (`matching == 1`) | 2333 | **2354** |
| reccmp por debajo del 90 % | 759 | 747 |
| `reccmp-datacmp` | 0 incidencias | 0 incidencias |

### Reglas de MSVC6 confirmadas con TUs mínimos

1. **Orden de declaración.** Con dos llamadas en una misma expresión binaria
   (también `-` o `<`), se evalúa primero la función *declarada después*
   (cuenta la primera declaración). Las cabeceras deben listar las funciones
   en orden de dirección, como el original: `RallyData.h`.
2. **Bucles pequeños.** Un bucle constante de 2–4 iteraciones se desenrolla y
   su constante va a otro registro; `a[0]=0; a[1]=0;` no genera lo mismo.
3. **`memset` en línea.** Un store de cero escrito *antes* del `memset` obtiene
   su propio `xor` y se retrasa tras el `rep stosd`.
4. **Símbolos distintos.** MSVC6 solo reordena stores que sabe disjuntos, es
   decir, a *símbolos distintos*; dentro de un blob o de un struct global no.
   `g_saveData` eran cinco globals: registros de coche (8×0xc4), flags de
   edición, categorías (4×0x650), 8 bytes y slots (16×0x30). Tras separarlos,
   los accesos que llegaban a flags y categorías a través del alias de los
   registros de coche se corrigieron: en nuestra imagen ya no son contiguos.
5. **Campos de bits.** `(x & M) | (v << n)`, `((a ^ b) & m) ^ a` y
   `((a ^ b) & m) == 0` son stores y comparaciones de *bitfields*. Solo los
   bitfields reales reproducen el orden. Se añaden vistas en uniones anónimas
   (mismo layout) a `NetPlayerInfo` y a los registros de `GameInfo0xa4`.
6. **Variables y huecos de pila.** Reutilizar una variable para dos vidas
   independientes cambia los huecos; y los índices de array deben escribirse
   como tales (el compilador crea los desplazamientos en bytes).
7. **Flags por TU.** El original no llama nunca a `_ftol`: `Race.cpp`,
   `StageUI.cpp` y `TimingUtils.cpp` también llevan `/QIfist` (`build.sh`).
   `/Ob2`, `/Op`, `/Oa` y `/Ow` empeoran en todos los TUs.
8. **Desempates.** Añadir un tipo a una cabecera puede cambiar la asignación
   de registros de funciones no relacionadas; un barrido de 0–15
   declaraciones ficticias por TU no vuelve exacta ninguna, así que no es lo
   que falta en las restantes.

### Bugs encontrados por el matching

- `0x4cfb30`: la escritura de nivel y extra usaba la máscara `0xffff803f`,
  que borraba el coche, el cambio y el nivel recién escritos. El original solo
  limpia los bits 11–14 (`and ch, 0x87`). Corregido con bitfields.
- El split de `g_saveData` habría dejado accesos fuera de `g_saveCarRecords`;
  todos nombran ahora su global real.

Funciones exactas nuevas en esta tanda: `0x409150`, `0x40d4b0`, `0x4188c0`,
`0x418e70`, `0x41a0a0`, `0x420190`, `0x420820`, `0x420850`, `0x4246a0`,
`0x445db0`, `0x456ca0`, `0x45f890`, `0x46e530`, `0x472e00`, `0x480380`,
`0x4a0c60`, `0x4a8040`, `0x4b1500`, `0x4b2610`, `0x4b3940`, `0x4b4100`,
`0x4b4910`, `0x4b5ee0`, `0x4bae10`, `0x4cf740`, `0x4cf8e0`, `0x4cfa10`,
`0x4e3340`, `0x4eb0c0`, `0x4ec2b0`, `0x4f02e0`, `0x505e70`. Las variantes
descartadas de las que se resisten están en `tools/fastcmp/work/hard.txt`.

## Continuación desde la auditoría del otro agente: primer lote bajo 700

Rama aislada `decomp/matching-low-review`, base **a6ff60a**. El trabajo de
`matching-review` ya incluía los cambios anteriores; no se vuelven a contar.
El hito de menos de 700 funciones pendientes **sigue sin alcanzarse**.

| Medida | Base auditada | Este lote |
|---|---:|---:|
| Funciones en reccmp | 3364 | 3364 |
| 100% estricto reccmp | 2357 | **2362** |
| Pendientes según reccmp | 1007 | **1002** |
| Exactas por bytes reubicados | 2435 | **2441** |
| Pendientes de las 3362 anotadas por el comparador | 927 | **921** |
| Variables / incidencias de datacmp | 3203 / 0 | **3201 / 0** |

**Seis nuevas exactas por bytes, cinco al 100% en reccmp, ninguna pérdida:**

| Función | reccmp base | Cambio |
|---|---:|---|
| `0x411e40` | 41.10% | Ajuste porcentual antes de buscar; salida por el contador; outputs tras FixDiv. |
| `0x4176b0` | 34.09% | Rama de sample activo primero y desplazamiento por índices de los 20 slots. |
| `0x431d80` | 71.88% | Ayudantes existentes FixVecScaleRecip/FixVecLength y salida por el contador. |
| `0x4668d0` | 71.70% | Dos arrays de timestamps para los dos jugadores locales. |
| `0x466920` | 67.69% | Dos grupos reales de 4 y 3 DWORDs; bucles pequeños desenrollados. |
| `0x48dca0` | 25.00% | Incremento con clamp usando la expresión de asignación. |

`0x4176b0` queda al 98% en reccmp porque el puntero de fin del array se
nombra como un global vecino distinto. La comparación independiente verifica
los **170 bytes completos**, incluidos ambos caminos. Se mantienen los cinco
errores de resolución de símbolos del comparador, excluidos de las exactas.

### Bugs corregidos que todavía no son matches completos

- Los timestamps de `0x588a80` y `0x588a88` se indexaban como arrays, pero
  estaban declarados como escalares independientes. Ahora cada uno posee sus
  **8 bytes** reales; ambos llamadores usan el array correspondiente.
- `0x4a12d0` y `0x4a13b0` eran `void`, aunque el original devuelve **0, 1,
  -1 o -2**. La firma y el llamador de `0x4ecaf0` ya conservan ese resultado.
  Se elimina la escritura de busy=1 que no existe en el original y se
  corrige el GUID de `0x4a13b0`, que tenía dos palabras intercambiadas.
  Sus porcentajes reccmp pasan de 79.52/67.47 a **67.96/66.67**; siguen
  pendientes y no se cuentan como nuevas exactas.

### Verificación y artefactos

Build MSVC6 correcto; datacmp **3201 variables / cero incidencias**;
check_dupes **3362 FUNCTION / cero STUB / 3195 GLOBAL**;
overlap idéntico a la base y diff --check limpio. Las dos anotaciones GLOBAL
menos son miembros de los arrays recuperados, no funciones eliminadas.

- Reset de replay: **12000** casos nativos, cero diferencias; record completo,
  timestamps, guardas, orden inverso y proveedor que cambia el puntero y count.
- Enumeración de sesiones: **12000**, cero diferencias; resultados HRESULT,
  GUID, descriptor, llamadas COM, rutas busy/null y guardas.
- Lógica previa: **42000**, cero diferencias. El harness ahora reubica por
  separado los cinco globals reales del save, conservando su modelo de memoria.
- Control negativo: la prueba de enumeración falla con el PE de la base
  auditada, detectando el retorno indefinido en el primer caso.

Los mocks de proveedores no validan transporte de red ni una partida completa.
Informes inmutables, entidades, scans, lista de ganancias y logs en
`tools/matching-below700-01/`. Las pruebas nuevas son reproducibles con
`tests/differential_replay_reset.py` y `tests/differential_session_enumeration.py`,
pasando `current.json` y `current-entities.json` de esa carpeta.

## Segundo lote aislado bajo 700: clasificación, viento y sonido de etapa

Base **9ff8098**, rama `decomp/matching-low-review`. Se han comparado las
3362 funciones anotadas por bytes: **2447 exactas**, **915 pendientes**,
**seis ganancias y ninguna pérdida**. Faltan **216** para llegar a 699.
El hito sigue abierto. reccmp mide 3364 funciones: **2367** al 100%, cinco
ganancias estrictas y ninguna pérdida; 730 quedan por debajo del 90%.

| Función | reccmp antes | reccmp después | Corrección |
|---|---:|---:|---|
| `0x40e5e0` | 25.88% | 100% | Desplazamiento de leaderboards por índice, conservando la copia de cada registro. |
| `0x417780` | 72.00% | 100% | Reinicio de los cinco slots por índice. |
| `0x45f6d0` | 83.98% | 100% | Contador con signo; terminar la búsqueda de nieve asignándole el count. |
| `0x493a40` | 36.62% | 100% | Par base calculado una vez; mejor marcha BYTE; orden original de guardas y asignaciones. |
| `0x4b8b10` | 70.27% | 100% | Capturar el primer nodo del sector antes de decidir si hay lista. |
| `0x4ec020` | 86.84% | 97.44% | Recorrido de perfiles con índice unsigned. Los 103 bytes son exactos. |

La diferencia normalizada de `0x4ec020` afecta al nombre del final de un
array. Se verifica por separado el código completo; no se cambian los
owners para alterar la puntuación. Los cinco símbolos que el comparador
no resuelve siguen contándose como pendientes.

### Reinicio de sonido: corrección funcional que sigue parcial

En `0x418f20`, el original avanza el puntero **0xb4 bytes antes** de escribir
los estados WORD y el índice de patrón. La fuente anterior usaba el puntero
sin avanzar para los offsets `-0x140`, `-0x13e` y `-0xb0`: dejaba estados sin
reiniciar y escribía 25 en otro campo. Ahora usa los ocho registros existentes
`RaceCarSoundState`: `state/stateOld=-1`, `pattern=25`, contadores a cero,
handles/ids y los primeros cuatro slotState a -1. El recorrido cabe completo
en g_raceBlock; conserva el registro único del callback.

reccmp pasa de **56.00% a 56.86%**. Esta corrección **no** cuenta entre las
seis exactas. El harness nativo compara todo el arena de 64 KiB con el
original y un modelo independiente: **6000 casos, cero diferencias**,
incluidos los ocho registros, campos vecinos y callback que modifica memoria
antes de la asignación final del flag. El control negativo falla en la
compilación anterior, caso 0, por el patrón incorrecto antes del callback.

### Verificación y reproducción

Build MSVC6 correcto; datacmp **3201 variables / cero incidencias**;
check_dupes **3362 funciones / cero STUB / 3195 globals**; overlap idéntico
al primer lote y `git diff --check` limpio. No se han cambiado cabeceras,
owners globales ni firmas públicas.

Informes, scans, entidades, bytes, ganancias, logs y compilaciones congeladas
en `tools/matching-below700-02/`. Prueba:

```sh
python3 tests/differential_stage_sound_reset.py ../tools/matching-below700-02/current.json ../tools/matching-below700-02/current-entities.json
```

El tercer argumento opcional permite comprobar el PE anterior usando sus
informes y entidades. Los mocks no validan una partida completa.

Siguiente grupo identificado: `g_unk0x00537248/4c` se usan como un array de
dos jugadores en el HUD y el detector de sentido contrario, pero aún son
dos escalares independientes. Revisar su ownership y el reset `0x416670`
antes de continuar con variantes de registros. No está corregido en este lote.

## Tercer lote aislado bajo 700: intervalos, nodos y flags de dos jugadores

Base **f4db191**, rama `decomp/matching-low-review`. Auditoría completa:
**2453/3362 exactas por bytes**, **909 pendientes**, seis ganancias y ninguna
pérdida. Faltan **210** para llegar a 699; el hito sigue abierto. reccmp:
**2372/3364** estrictas, cinco ganancias y ninguna pérdida; **725** bajo 90%.

| Función | reccmp antes | Después | Corrección |
|---|---:|---:|---|
| `0x4692b0` | 50.00% | 100% | Buscar el tipo del nodo por índice, sin pelar la primera iteración. |
| `0x48caa0` | 30.77% | 100% | Publicar el count antes de capturarlo y guardar el puntero de la lista. |
| `0x48f400` | 27.85% | 100% | Capturar ambos extremos; dos loops indexados y una salida común. |
| `0x492b50` | 50.00% | 100% | Recorrer los triángulos por índice inverso; conservar la textura leída en cada vuelta. |
| `0x49cd90` | 95.83% | 100% | Capturar pObject antes de comprobar el flag de dibujo. |
| `0x4aa880` | 77.19% | 96.77% | memset del Data4 del GUID: los 96 bytes completos son exactos. |

La diferencia normalizada en ClearConnections es el nombre del puntero final
de un array, que coincide con otro símbolo en la imagen recompilada. No se
modifican owners para cambiar la puntuación. Los cinco errores de resolución
heredados del comparador siguen contándose como pendientes.

### Flag de sentido contrario del segundo jugador

`g_unk0x00537248/4c` eran dos escalares, separados **368 bytes** en el PE
anterior. El HUD y el detector accedían al segundo mediante `[player]` desde
el primero, fuera de ese objeto, mientras el reset limpiaba el otro scalar.
Se recupera el owner real **g_raceWrongWayFlags[2]**, de ocho bytes, y sus
usuarios nombran el array. Se elimina la anotación redundante del miembro
+4. No hay cambios de cabeceras ni de firmas públicas.

El reset `0x416670` sigue parcial (37.21%); este arreglo no cuenta entre las
seis exactas. `tests/differential_race_state_reset.py` ejecuta original y
recompilado con un modelo independiente del arena completo: **6000 casos,
cero diferencias**, dos flags, diez call records, cinco slots y guardas.
El control negativo con el PE anterior preserva la distancia real de los
escalares y falla en el caso 0 por el flag del segundo jugador (offset 4544).
Reubicar ambos escalares juntos en un harness habría ocultado este bug.

Build MSVC6 correcto; datacmp **3200 variables / cero incidencias**;
check_dupes **3362 funciones / cero STUB / 3194 globals**; los 27 overlaps
heredados son idénticos al segundo lote; `git diff --check` limpio.
Artefactos inmutables, informes, entidades, scans, ganancias, hashes, logs y
ambas compilaciones completas: `tools/matching-below700-03/`.

```sh
python3 tests/differential_race_state_reset.py ../tools/matching-below700-03/current.json ../tools/matching-below700-03/current-entities.json ../tools/matching-below700-03/current-build/CMR2.exe
```

El tercer argumento permite comprobar otro PE con sus informes y entidades.
La prueba cubre el reset; no valida una partida completa.

## Cuarto lote aislado bajo 700: clasificación viva y progreso con signo

Base **76c0a6a**, rama `decomp/matching-low-review`. Auditoría completa:
**2455/3362 exactas por bytes**, **907 pendientes**; dos ganancias y ninguna
pérdida. Faltan **208** para bajar a 699. reccmp: **2374/3364** estrictas,
dos ganancias sin pérdidas, **722** bajo 90%. El hito sigue abierto.

| Función | reccmp antes | Después | Corrección |
|---|---:|---:|---|
| `0x408d80` | 56.91% | 100% | Consultar count en cada vuelta; recorrer los usados en orden inverso y capturar el destino de los demás antes del proveedor. |
| `0x418c30` | 57.69% | 100% | Llamadas de sonido dentro de cada rama, heavy primero; nivel de shake como operando izquierdo de la comparación. |

Los **172 bytes** de clasificación y los **152 bytes** de sonido son exactos.
En clasificación, la fuente previa capturaba una vez el count del primer
bucle y calculaba dos veces el destino del segundo, con llamadas a proveedores
entre ambas consultas. El original consulta el count durante el primer bucle
y usa el mismo destino capturado para ambos campos del segundo.
La macro de shake tiene otro usuario `0x418ba0`: mejora a **84.78%**, todavía
parcial; la auditoría completa confirma que no se pierde ninguna exacta.

### Progreso negativo interpretado como recorrido completo

`0x421470` comparaba un valor int con un producto unsigned porque
RallyData_FUN_00406990 devuelve unsigned. El original usa **cmp/jl con signo**.
Se convierte el count en int antes de multiplicar y se captura el valor del
registro dentro de la rama correspondiente, después de consultar el modo.
reccmp pasa de **63.16% a 94.25%**; sigue parcial y no cuenta como ganancia
exacta. La diferencia restante incluye el save/restore adelantado de ESI.

`tests/differential_route_progress.py`: **6000 casos nativos / cero
diferencias** con original, recompilado y modelo independiente. Comprueba
los ocho registros, umbrales con signo, clamps, retorno, número de llamadas,
64 KiB completos y proveedor que cambia el límite, denominador, registro e
índice del coche después de capturar el progreso. El control negativo con el
PE anterior falla en el caso 1: devuelve **65536** para progreso **-1**, con
umbral positivo, cuando el original devuelve cero. No prueba una partida.

Build MSVC6 correcto; datacmp **3200 variables / cero incidencias**;
check_dupes **3362 funciones / cero STUB / 3194 globals**; los 27 overlaps
heredados siguen idénticos y `git diff --check` limpio. No cabeceras, firmas
públicas ni owners modificados. Informes, compilaciones completas, hashes,
ganancias y logs: `tools/matching-below700-04/`.

```sh
python3 tests/differential_route_progress.py ../tools/matching-below700-04/current.json ../tools/matching-below700-04/current-entities.json ../tools/matching-below700-04/current-build/CMR2.exe
```

## Quinto lote bajo 700: integrar el avance del otro agente y matrices vivas

Merge local de **c7422b2** sobre **de55abe**, conservando los commits de ambas
ramas. No se modifica matching-review ni sus cambios pendientes. Sus cuatro
commits desde a6ff60a aportan **16 nuevas exactas únicas**; 417780, 4692b0 y
48dca0 ya estaban exactas aquí y no se cuentan de nuevo. Dos conflictos de
fuente (4692b0 y 48dca0) se resuelven conservando nuestros cuerpos exactos.

Se añade además **0x4813b0**, 50% -> 100%: guardas positivas anidadas y
lecturas de la matriz desde el objeto publicado en g_unk0x00590c20.
Las llamadas GetRight/GetUp/GetForward pueden cambiar el contexto: conservar
un puntero local de la primera lectura ocultaba esos cambios. Los **279
bytes completos** coinciden, incluidas ambas salidas y las tres llamadas.

Auditoría de las 3362 funciones: **2472 exactas por bytes**, **890 pendientes**,
**17 ganancias y ninguna pérdida**. Faltan **191** para bajar a 699. reccmp:
**2388/3364** estrictas, +14 sin pérdidas, **710** por debajo del 90%.
El hito sigue abierto. Las ganancias y sus porcentajes individuales están
en `tools/matching-below700-05/gains.tsv`.

No se heredan los porcentajes declarados en los mensajes de otros commits:
4a1940 queda en **88.24%** en nuestra compilación, 466030 en **91.43%**,
4da710/4daf90 en **69.54%/57.36%**; ninguna cuenta como nueva exacta. Tres
nuevas exactas por bytes tampoco llegan al 100% normalizado: 408340 **96.43%**,
477f30 **93.75%**, 4f0e80 **99.26%**, por nombres de límites/operandos.
Las cinco resoluciones fallidas heredadas siguen contándose como pendientes.

Build MSVC6 correcto, datacmp **3200 variables / cero incidencias**,
check_dupes **3362 funciones / cero STUB / 3194 globals**, los 27 overlaps
heredados idénticos. Progreso de ruta y reset de carrera pasan **12000 casos
nativos / cero diferencias** con el PE integrado. No hay cambios de cabeceras
ni owners; el cuarto argumento BYTE de 46cce0 queda consistente en ambos TUs.
Informes y compilaciones completas congeladas en **tools/matching-below700-05/**.

## Sexto lote bajo 700: ordenación y precisión del mínimo de escala

Base **0181991**, rama aislada `decomp/matching-low-review`. Resultado completo:
**2474/3362 exactas por bytes**, **888 pendientes**; +2 y ninguna pérdida.
Faltan **189** para bajar a 699. reccmp **2390/3364** estrictas, +2 sin
pérdidas, **708** bajo 90%. El hito sigue abierto.

| Función | reccmp antes | Después | Corrección |
|---|---:|---:|---|
| `0x40d520` | 37.04% | 100% | Índices de coche int, guard positivo y sesgo del siguiente índice capturado después del cursor. |
| `0x427580` | 68.89% | 100% | Mínimo con scaleY como operando izquierdo; constantes FLOAT reales del HUD. |

La ordenación conserva count, inicialización opcional, direcciones 0/1,
comparaciones estrictas y stores BYTE del orden. Sus **154 bytes** son
exactos; no hay cambios de firma ni del número de argumentos (ret20).

### Comparación de escalas sin redondeo anticipado

El original compara scaleY vivo en x87 con scaleX ya guardado en float.
`if (scaleY < scaleX) ... else ...` recupera esa comparación. Antes se
comparaba scaleX con una copia de scaleY redondeada a float; dos valores
distintos podían parecer iguales y seleccionar el mínimo equivocado.

Se recuperan tres constantes observadas directamente en el PE original:
**0x511388=40.0f**, **0x51138c=1000.0f**, **0x511390=30.0f**. Cada owner
ocupa cuatro bytes y se inicializa con esos valores. One y Zero usan los
globals existentes. Los **145 bytes** completos de 427580 coinciden.

`tests/differential_network_font.py`: **6000 casos nativos / cero diferencias**,
modelo entero independiente del umbral, vecinos exactos, counts 1..128 y
64 KiB completos. El control negativo falla en el PE anterior con
width=494000, height=49399999, players=100: scaleX=25 y scaleY ligeramente
menor. Al redondear Y antes de decidir, la fuente anterior escogía 25 y
la fuente grande; original y corregida escogen la pequeña. El count ampliado
sirve para distinguir ambas precisiones; no afirma que una carrera tenga
100 jugadores ni valida una partida real.

Build correcto; datacmp **3203 variables / cero incidencias**; check_dupes
**3362 funciones / cero STUB / 3197 globals**, tres constantes reales nuevas;
los 27 overlaps heredados idénticos y git diff --check limpio. No cabeceras.
Informes, hashes y compilaciones completas en **tools/matching-below700-06/**.

```sh
python3 tests/differential_network_font.py ../tools/matching-below700-06/current.json ../tools/matching-below700-06/current-entities.json ../tools/matching-below700-06/current-build/CMR2.exe
```

## Séptimo lote bajo 700: integración, cámara y orientación

Base **423365c**, merge de los cinco commits hasta **5c90390** del otro
worktree, sin incluir sus cambios sin confirmar. Se conserva nuestra copia
exacta de tiempos **408d80** al resolver el conflicto; su vista SaveSlot
queda junto a los globals de guardado. No se suma una función ya exacta.

Auditoría completa: **2479/3362 exactas por bytes**, **883 pendientes**,
**5 ganancias y ninguna pérdida**. Faltan **184** para llegar a 699.
reccmp: **2394/3364** estrictas (+4, sin pérdidas), **703** bajo 90%.
Las cinco resoluciones fallidas heredadas siguen contándose pendientes.

| Función | reccmp antes | Después | Procedencia y cambio |
|---|---:|---:|---|
| `0x421720` | 63.60% | 100% | Otro agente: accesos indexados a cada record de cámara. |
| `0x4eb3e0` | 58.82% | 97.22% | Otro agente: perfiles indexados directamente, sin puntero local. Bytes exactos. |
| `0x40d010` | 82.05% | 100% | Otro agente: índices de tiempos y penalizaciones. |
| `0x459250` | 55.07% | 100% | Recargar el objeto gráfico después de proveedores, ramas independientes y argumento BYTE. |
| `0x45f5d0` | 44.74% | 100% | Orientación WORD, dirección completa, FixVecDot y actualización del pitch en el record. |

La cámara **459250** reproduce los **200 bytes / 67 instrucciones** completos.
El original vuelve a leer g_pGraphics antes de los stores y después de
FUN_00423f30; los punteros pNear/pFar capturados antes de la primera llamada
escribían y limitaban en un objeto anterior si un proveedor cambiaba el global.
Se conserva el clamp de distancia **far** incluso tras publicar **near**,
tal como hace el original. La firma BYTE del setter 422f90 y su declaración
son coherentes; sigue siendo exacto y conserva ret8.

La orientación **45f5d0** reproduce **242 bytes / 84 instrucciones**, incluidas
ambas ramas de envoltura. La salida de 421fe0 ocupa un WORD: una variable
unsigned short permite que MSVC reutilice el argumento sin ampliar el signo
antes de las máscaras de 12 bits. Se inicializa direction.y, luego x y z;
FixVecDot recibe la dirección primero y vec segundo. Publicar p[5] con += y
leer después el ángulo recupera el acceso al record. No se usa el local int
parcialmente escrito que había dado 100% preliminar: leer sus bytes altos
sin inicialización no era una solución válida.

### Corrección parcial de entrada de nombres

El commit 99bb7a2 cambia **4f1040** de strlen-2 a **strlen-1**. El original
cuenta también el terminador y usa base-2: eso equivale a strlen-1. Con un
nombre de un carácter, la fuente anterior escribía antes del buffer. Esta
función queda **76.68%**; es una corrección de lógica y no cuenta entre las
cinco exactas. Las otras parciales de cámara no se presentan como exactas:
421e20 queda **68.86%**, 423b20 **26.47%**, 47bad0 **96.77%**. Se sincronizan
las notas bajo90 de las funciones editadas, manteniendo FUNCTION.

### Verificación

- `tests/differential_camera_clip.py`: **6000 casos nativos / 0 diferencias**.
  Proveedores reemplazan el objeto gráfico durante las llamadas; modelo
  independiente de memoria y trazas, límites con signo, clamp de nodo sin
  signo y arena completa de 64 KiB. El PE anterior falla en caso1 en la traza.
- `tests/differential_name_entry.py`: **6000 casos nativos / 0 diferencias**.
  Longitudes 0..3, todas las columnas de las tres filas, borrar y confirmar,
  cursor y arena de 64 KiB. El PE anterior falla en caso1, nombre de un
  carácter, escribiendo el byte anterior al buffer (offset16383).
- Build MSVC6 correcto; datacmp **3203 variables / 0 incidencias**;
  check_dupes **3362 funciones / 0 STUB / 3197 globals**; los **27 overlaps
  heredados** son idénticos. Git diff --check limpio. No nuevas cabeceras.

Los proveedores de estos harnesses están simulados; estas pruebas no validan
una partida completa. Informes, hashes, gains.tsv y compilaciones completas
antes/después congelados en **tools/matching-below700-07/**. El hito sigue abierto.

```sh
python3 tests/differential_camera_clip.py ../tools/matching-below700-07/current.json ../tools/matching-below700-07/current-entities.json ../tools/matching-below700-07/current-build/CMR2.exe
python3 tests/differential_name_entry.py ../tools/matching-below700-07/current.json ../tools/matching-below700-07/current-entities.json ../tools/matching-below700-07/current-build/CMR2.exe
```

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

# Clima, iluminación y escenas de etapa — tandas 07–09

Se recupera una familia completa de registros y se actualizan sus escritores,
lectores y declaraciones de función en `StageTiming.cpp` y `StageObjects.cpp`.
Los tipos están en `CMR2Decomp/StageWeather.h` y
`CMR2Decomp/StageWeatherParticle.h`; `LayoutChecks.cpp` comprueba
sus ocho tamaños, 87 offsets y dos capacidades. Se conserva el orden de las
declaraciones de función, importante para la generación de código de MSVC6.

## Tanda 07: registros de vista y rampas

| Registro | Tamaño | Evidencia |
| --- | --- | --- |
| `ViewWeatherState` | `0x178` | Reserva en `0x45e5b0`; inicialización de partículas en `0x45eed0`; simulación en `0x45f9d0`; dibujo en `0x460390`. |
| `ViewLightingRamp` | `0x2c` | Reserva en `0x45e5b0`; configuración en `0x45f3d0`; progreso en `0x45e8b0`; integración de yaw en `0x45f5d0`; interpolación en `0x461bb0`; lectura en `0x461c30`. |
| `CarSurfaceRamp` | `0x0c` | Reserva en `0x45e5b0`; configuración en `0x45e710`; actualización por posición de carrera en `0x45e7f0`; lectura en `0x460c80` para actualizar los parámetros de superficie del coche. |
| `WeatherWindTransition` | `0x0c` | Reset en `0x45eea0`; actualización en `0x45f6d0`: objetivo, velocidad de cambio y espera; las tres palabras no son componentes de una posición. |

`ViewWeatherState` distingue el estado actual/deseado, centro de la caja de
precipitación, posición anterior de cámara, velocidad absoluta/relativa,
eje de oscilación de nieve, desplazamiento para salpicaduras y su recíproco,
intensidad, número actual/deseado de partículas, velocidad de cambio,
atenuación bajo refugios, visibilidad solar y posiciones de ruta.

La distribución reserva a cada vista una parte del pool de 400 partículas,
identificada por dos `short` firmados en `+0x74` y `+0x76`. Inicialización y
simulación establecen tres caches de 25 elementos: triángulos en `+0x78`,
alturas de suelo en `+0xac` y alturas relativas en `+0x110`. Las alturas
relativas se calculan como `centre.y - groundHeight - 0x61999`.

El parámetro de `0x45e9a0`, previamente presentado como `SceneNode *`, es
el registro de clima: comprueba intervalos de ruta y atenúa la precipitación
en `+0x64`. Se corrige el tipo y se conserva su identidad FUNCTION.

## Tanda 08: tabla de iluminación y partículas

| Registro | Tamaño | Evidencia |
| --- | --- | --- |
| `StageLightPreset` | `0x48` | Lectores de `0x460da0`: parámetros de altura de malla, tamaño del icono solar, inicio/final de niebla y once colores de cuatro bytes. |
| `StageLightingValues` | `0xb4` | `0x460da0` expande cada preset a valores firmados 16.16; `0x461c30` interpola los pares y envía colores, pesos, alphas y distancias a sus consumidores. |
| `StageLightingState` | `0x178` | Dos bloques anteriores, blend en `+0x168`, intensidad en `+0x16c`, sectores de relámpago en `+0x170/+0x172` y activación de niebla en `+0x174`. |
| `StageWeatherParticle` | `0x24` | Pool de 400 entradas en `0x543fb0`; inicialización en `0x45eed0`, simulación en `0x45f9d0` y dibujo en `0x460390`. |

La antigua descripción de las dos palabras de `+0x170` como estados del clima
era incorrecta: `0x45f890` escribe el sector iluminado por el relámpago y
conserva el del frame anterior. La última palabra activa el cálculo de
niebla; no es un indicador de cambio del clima.

El campo compartido de la partícula en `+0x14` es una unión: durante la lluvia
contiene la altura del suelo para situar la salpicadura; durante la nieve,
una fase angular en grados. En `+0x10` se establece una amplitud aleatoria
que escala el seno de la fase. El byte de `+0x1d` selecciona una de tres
texturas de nieve. Se conservan sus anchos de lectura y escritura.

## Tanda 09: cielo, suelo y nubes

Los resultados de construir las escenas en `0x46f060` son `SceneNode *`:

| Dirección | Nombre recuperado | Identidad observada |
| --- | --- | --- |
| `0x589438` | `g_stageSkyNode` | Escena construida desde `TEMP.SKY`. |
| `0x58943c` | `g_stageGroundNode` | Escena construida desde `TEMP.GRO`. |
| `0x589440` | `g_stageCloudNode` | C3D del archivo seleccionado por el nivel de nubes. |
| `0x589444` | `g_stageCloudTopNode` | Escena `top.c3d` del mismo archivo de nubes. |
| `0x589448` | `g_stageCloudArchive` | Archivo cargado y liberado junto con las escenas de nubes. |

La carga usa `pObject`, `visible`, `viewMask`, `pFirstChild` y `pNext`.
Posicionamiento y lectura usan `current`; desaparecen las desreferencias
numéricas correspondientes. Los getters históricos que devuelven direcciones
por `int *` mantienen explícitamente ese contrato para sus consumidores.

## Exactitud, límites y deuda

Se mantienen los cambios locales anteriores del audit numérico. El baseline
se obtiene compilando y midiendo ese árbol antes de modificar estos registros.
Cada ampliación se compara por módulo y la validación final recompila el
proyecto completo, mide todas las funciones y ejecuta la suite registrada.
Los resultados y hashes se guardan en
`weather-and-stage-scenes-matching.json`.

La nueva prueba `differential_weather_records.py` añade 3.827 casos con modelos
independientes de multiplicación firmada 16.16, registros y stack envenenados,
guardas del heap completo y comprobaciones de ABI. Cubre el escalado con
capacidades `short` negativas/extremas, transiciones entre tipos, límites de
refugio y posiciones de ruta unsigned, y factores extremos de interpolación.
El proveedor de integración de cámara está controlado en esta prueba; su
cuerpo completo y las consultas de terreno se verifican por separado en
`differential_camera_deformation.py`. Las pruebas existentes de iluminación
comprueban también modelos independientes, memoria y orden de llamadas.

La cabecera completa de clima se limita a sus consumidores directos.
`StageTiming.h` conserva el acceso al pool mediante una cabecera mínima de
partículas, con coordenadas escalares y sin importar los helpers inline de
`FixedPoint.h` antes de su posición original. Alterar este alcance cambiaba
la asignación de registros de `GameMenu_SetupStageResults` o
`OptionMenu_DrawResultsRallyInfo`; la versión final conserva ambas puntuaciones
y evita añadir declaraciones sin uso para forzar coincidencias.
Los mapas de símbolos de fastcmp se regeneran después
de medir; los de un EXE anterior no sirven para validar las relocalizaciones
de un nuevo build.

Persisten dos recorridos con punteros sesgados entre las alturas contiguas:
`pValue[-25]` en la inicialización y `pHeight[25]` en la simulación. Sustituirlos
por dos índices independientes empeora la coincidencia de esas funciones.
Los tamaños y la contigüidad quedan comprobados y los recorridos documentados
en el código; siguen siendo deuda concreta para una representación portable.
También se conserva la lectura DWORD histórica sobre el contador BYTE de
vistas, así como la lectura BYTE del tipo DWORD en su getter.

Los bytes `ViewWeatherState::field_0xaa`,
`StageWeatherParticle::field_0x1e` y `StageLightPreset::field_0xc` siguen sin
semántica demostrada. El byte de partícula `+0x1c` se inicializa a `0x4b`,
pero no se le atribuye una función sin un lector que la justifique.

El inventario pasa de 2.078 a 1.987 accesos crudos, de 102 a 99 tamaños
literales y de 1.429 a 1.389 identificadores desconocidos distintos.
Se nombran 40 identificadores de globals y vistas globales; 339 apariciones de identificadores desconocidos
desaparecen. Hay seis tipos nombrados adicionales (ocho layouts de esta
familia, dos ya existían) y 1.512 declaraciones de campos sin semántica
demostrada. La recuperación del proyecto completo sigue abierta.

Resultado final: las 3364 entradas de la auditoría de bytes son idénticas al
baseline (mismo SHA-256 del informe). Se conservan 2922 funciones exactas,
56,10 % perfect y 94,50 % fuzzy, sin cambios de puntuación ni incidencias de
datos. Pasan los 107 harnesses registrados, sin fallos. Los hashes del build,
fuentes y pruebas quedan en el JSON de evidencia del lote. El resultado
completo de la suite está en `weather-and-stage-scenes-tests.txt`.

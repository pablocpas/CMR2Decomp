# Historial de esquinas y preparación del paso físico

Los dos recorridos de estado de `Car_StepAll` (`0x433890`) y `Car_PrepareStep`
(`0x444c10`) usan ahora los arrays de `Car`, conservando el orden de las copias:

| Acceso anterior | Representación recuperada |
| --- | --- |
| Cursor `0x300..0x330`, con referencias a `+0x30` y `-0x90` | Para las cuatro esquinas inferiores: `cornerPrev2[i] = cornerPrev[i]`, seguido de `cornerPrev[i] = corners[i]`. |
| Cursor `0x42c..0x48c` en pasos de `0xc` | `cornerVelocity[i] = velocity`, para las ocho esquinas. |

Los nombres locales distinguen `orderIndex` de `cornerIndex`, el array de
sectores vecinos, la normal del suelo en el marco del cuerpo, media gravedad
y masa. Los parámetros de ambos pases identifican el array de coches y el
orden. En preparación, el cursor es `pCarIndex` y el contador `remaining`.

## Campos con productores y consumidores identificados

| Campo | Evidencia |
| --- | --- |
| `wheelLeanHeight[4]`, `+0x998` | `Car_UpdateSuspension` (`0x43b100`) reconstruye una base desde `wheelLean`, rota las cuatro esquinas horizontales y guarda su componente Y más `suspensionDamageOffset`. `Car_UpdateWheelLoads` (`0x43c640`) limita el máximo absoluto a `0x4000` y lo usa en el reparto de masa. |
| `wheelTravelLoadBias[4]`, `+0x808` | `Car_IntegrateWheelTravel` (`0x42e450`) calcula el recorrido relativo a la media, lo multiplica por el factor `+0x9c0` y normaliza los cuatro valores si el máximo absoluto supera uno. El cálculo de cargas suma este sesgo al término anterior antes de aplicar `suspensionLoadScale`. |

Ambos arrays conservan tamaño, signo y posición. Cuatro comprobaciones nuevas
verifican offsets y capacidades en el build Win32; la prueba nativa usa sus
posiciones reales tras la expansión de los punteros del coche.

## Capacidad del estado de contactos

La prueba nativa nueva detectó un acceso fuera de límites en el código previo:
dos ramas de `Car_PrepareStep` y el reset de `Car_RestorePhysicsFromRecord`
(`0x426d80`) recorrían `cornerOnGround[4+i]` con `i = 0..7`. Ese array contiene
ocho elementos, por lo que la segunda mitad de ese recorrido excedía su límite.

El layout ya recuperado ofrece `objectContactState[8]` en `+0xbbc`: las cuatro
primeras posiciones aliasan `cornerOnGround[4..7]` y las otras cuatro ocupan
`+0xbcc..0xbd8`. Los tres resets usan ahora `objectContactState[i]`. Conservan
las mismas posiciones de memoria, anchos, efectos y puntuaciones; se elimina
el acceso inválido a través del array más corto. Sus checks de offset y tamaño
ya existían y permanecen activos.

## Validación

| Función | Coincidencia de bytes | Fuzzy |
| --- | --- | --- |
| `Car_StepAll` | 87,00 % → 88,65 % | 96,45 % → 97,40 % |
| `Car_PrepareStep` | 81,39 % → 81,61 % | 89,98 % → 89,98 % |

El gate abarca 18 unidades, incluidas las que reciben `Car.h`. La medición
completa conserva las otras puntuaciones, las 2924 funciones exactas y cero
problemas de comparación de datos. Los bytes perfectos globales permanecen en
56,28 % y el fuzzy redondeado en 94,46 %.
Los 138 harnesses diferenciales pasan sobre ese mismo build medido.

`differential_car_step.py` conserva 800 escenarios del scheduler y 288 del
integrador, y añade 160 preparaciones completas. Estas últimas incluyen
cuentas negativas/cero, 1/2/8 coches, ambos órdenes y todas las combinaciones
de flags de preparación; ejecutan los helpers de matrices y longitud reales.
Comprueban las ocho velocidades, las dos generaciones de esquinas, el orden
de proveedores, el heap completo contra el original, escrituras globales y ABI.
La consulta de terreno y los pases posteriores de física son hojas controladas.

`check_64bit_car_step.py` ejecuta ambos pases de producción y nueve helpers
de matrices: 144 escenarios con ASan/UBSan, punteros nativos mayores de 4 GB y
comparación del heap proyectado y globals contra el original x86. Solo la
fixture traduce los dos tramos sin punteros del coche; el código de producción
usa `Car *`, arrays y tamaños nativos. Se guardan los quince punteros del
coche, el padding nativo, los registros de coches no usados y el prefijo de
punteros de los conjuntos de piezas.

La prueba nativa de estado inicial mantiene sus 354 escenarios; se actualiza
su copia de headers para incluir `CameraRecord.h` y `X87Single.h`, dependencias
del header actual del coche que faltaban en la fixture.

## Deuda que permanece

El recorrido sesgado de `cornerWeight` en `Car_UpdateWheelLoads` sigue pendiente.
La variante directa por índices pierde bytes (89,84 % → 85,27 %) y fuzzy
(99,22 % → 96,90 %), por lo que se retira sin afectar a las otras mejoras.
No se sustituye por un wrapper que esconda el desplazamiento.

El inventario reproducible pasa de 118 a 113 candidatos crudos y de 1407 a
1405 declaraciones de campos sin nombre semántico. Los campos restantes del
coche, los otros subsistemas y el objetivo general continúan abiertos.

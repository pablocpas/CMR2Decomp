# Montaje de glows y asociación de vértices

`CarLight_BuildPointGlows` (`0x463fe0`, antes
`StageObject_RebuildCarLightMeshes`) crea un glow por punto CIN, configura las
dos texturas de su canal y asocia los puntos de tipo 9 con los dos emisores de
escape. Para el coche cero también crea los dos nodos de luz de vista trasera.
Después guarda en cada punto el índice de la pieza/vértice deformable más
cercano. No construye ninguna `Mesh`.

Los recorridos usan índices de `CarLightPoint`, piezas y vértices. Se eliminan
ambos cursores de bytes de puntos y el acceso a `geometry.vertices` calculado
desde un puntero sesgado a `vertexCount`, incluidos sus casts, `offsetof` y
factores que compensaban el tamaño del puntero en x64. La lectura es ahora
`pParts->geometry.vertices[meshIndex][vertexIndex].pos`.

La evidencia de las variables queda en las operaciones originales:

| Nombre | Evidencia |
| --- | --- |
| `nearestDistanceSquared[20]` | Se inicializa con `0x3e80000` y se compara con `FixVecDot(position, position)`: distancia al cuadrado en 16.16, con límite inicial de 1000. |
| `meshIndex`, `vertexIndex` | Recorren `count`/`vertexCount` y se guardan como los dos shorts de enlace del punto. |
| `lightIndex` | Recorre los puntos al crear glows y al buscar su vértice; se reutiliza en ambas fases. |
| `exhaustSide` | Solo aumenta para puntos de tipo 9 y se pasa a ambos setters de escape. |
| `projectedBrightness` | Vale cero o `0xff` para el tipo 9; ocupa ese argumento de `Glow_Add`. |
| `pNode` | Selecciona el nodo de pieza o el nodo de carrocería, incluido el fallback cuando falta la pieza. |

El orden de lectura es significativo para el matching: primero se obtiene el
punto, después se carga la posición del vértice y se calcula la diferencia.
Con ese orden, los bucles por índices recuperan las cargas del original y ya
no necesitan el acceso sesgado. La comparación sigue siendo estricta (`<`):
un empate conserva el primer vértice, y si ninguno mejora el límite se conservan
los shorts de enlace anteriores. Se mantienen la tabla temporal de 20 entradas,
los reloads del índice del coche y el orden de llamadas de recursos.

La coincidencia de bytes mejora de **90,85 % a 96,73 %**, y la estructural de
**93,46 % a 98,04 %**. La medición completa mantiene los demás scores `s/fz/x`,
las 2924 funciones exactas de 3364 y cero errores de datos. La similitud
estructural global pasa de 94,45 % a 94,46 %.

Otros tres nombres completan interfaces ya identificadas:

| Dirección | Nombre | Evidencia |
| --- | --- | --- |
| `0x46b400` | `CarDamage_SetModelEnabled` | Escribe el entero `enabled` de `g_carDamageModelEnabled[carIndex]`, leído por el getter y la deformación. |
| `0x46b420` | `CarDamage_ClearModelEnabled` | Limpia los ocho enteros al reiniciar el estado de carrera. |
| `0x480980` | `CarPart_ResetMovingStateTables` | Limpia los cuatro arrays de `PartState`, los nodos de slots y los índices de modelo (a `0xff`). |

Los nombres descriptivos se documentan por dirección en
`scripts/renames/017-functions.tsv`; no se presentan como símbolos originales.
El reset móvil conserva su recorrido de bytes basado en `sizeof(PartState)` y
la comparación con signo mediante `INT_PTR`: las conversiones directas a
índices/punteros cambian sus instrucciones. Sigue siendo exacto y opera con
strides nativos; no se afirma que ese cursor haya desaparecido.

`differential_car_glow_build.py` añade 128 montajes completos contra el original
y un modelo independiente. Ejecuta el allocator del pool, los getters de daño y
piezas, el setter de glow y ambos setters de escape reales. Solo controla las
hojas de recursos CIN, creación de nodos de luz y configuración de texturas.
Comprueba heap completo, globals y vecinos, anchos de escritura, orden de
proveedores y ABI. Cubre coches 0/7, daño desactivado/entero no canónico,
cuentas negativas/cero/20, piezas sin vértices, empates, límite de distancia,
pool ocupado/agotado y enlaces de escape con glow nulo.

`check_64bit_car_glow_build.py` repite esos 128 casos comparando con el original
x86: nueve cuerpos de producción, headers reales de coche/piezas/glow y
punteros mayores de 4 GB bajo ASan/UBSan. La fixture traduce direcciones y spans
serializados; los cuerpos de producción usan sus layouts nativos. Se guardan
también punteros y padding que la proyección no escribe.

La prueba nativa de piezas pasa 466 casos con quince cuerpos; incluye cuentas
negativas/cero y ocho coches al limpiar las tablas móviles, con conservación
de modos y punteros de tabla. Los 137 harnesses diferenciales pasan sin fallos
sobre el mismo build medido.

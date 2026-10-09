# Escenas, buffers y vínculos del coche — tanda 06

Se reconstruye el bloque que la descompilación representaba como
`g_unk0x00542630[0x24 * 32]`. Sus usos establecen 16 registros de escena
seguidos de varias tablas independientes; el tamaño de aquella declaración
no demostraba 32 registros del mismo tipo. Las declaraciones recuperadas
están en `CMR2Decomp/CarResources.h` y sus comprobaciones en `LayoutChecks.cpp`.

## Registro de escena

`CarSceneRecord` mide `0x24` y la tabla tiene 16 entradas. El getter
`0x456be0` calcula ese stride; la inicialización `0x456d90` y la liberación
`0x457ed0` recorren exactamente 16 elementos. La carga `0x457000` establece
los objetos y las letras de selección de los assets.

| Offset | Miembro y tipo | Evidencia |
| --- | --- | --- |
| `0x00` | `BYTE modelClass` | La selección de modelo obtiene la categoría del archivo frontend, la guarda como byte y la pasa al posicionamiento/física del coche. |
| `0x01..03` | Bytes desconocidos | Sin lectores/escritores identificados. No se declaran como padding demostrado. |
| `0x04` | `SceneNode *bodyNode` | Resultado de buscar el nodo con id 5 en la escena principal; se separa para situar la carrocería. |
| `0x08` | `SceneNode *rootNode` | Resultado de construir la escena C3D principal; contiene los nodos de rueda 1..4. |
| `0x0c` | `SceneNode *alternateWheelScene` | Escena opcional de ruedas de nieve/limpias. Sus objetos sustituyen los de la escena principal. |
| `0x10..1f` | `void *originalWheelObjects[4]` | La carga conserva `pObject` de las cuatro ruedas; la liberación los restaura antes de destruir la escena principal. El tipo concreto del objeto queda pendiente. |
| `0x20` | `BYTE detailCode` | Letra empleada en las rutas del modelo/texturas y selección del modelo con daños. |
| `0x21` | `BYTE variantCode` | Letra usada para cargar la carrocería alternativa. |
| `0x22..23` | Bytes desconocidos | Sin semántica demostrada. |

Las búsquedas «ByType» consultan el byte bajo de `SceneNode::flags`; el id
buscado no es `SceneNode::type`, que controla la clase del objeto.

## Regiones contiguas del ejecutable original

| Dirección original | Declaración | Evidencia |
| --- | --- | --- |
| `0x542630` | `g_carScenes[16]` | Tabla anterior, `0x240` bytes. |
| `0x542870` | `BYTE *g_carStartData` | Buffer CSP cargado por `0x456c10`; usado para obtener posiciones de salida. El formato completo sigue pendiente. |
| `0x542874` | `FixAngles *g_carStartAngles` | Vista del mismo buffer CSP desde `+0x18`, indexada por ángulos de salida. |
| `0x542878` | `FixVector g_carStartPosition` | Scratch de posición devuelto por `0x456c70`; se actualizan X/Z y se conserva Y. |
| `0x542884` | `void *g_carAuxiliaryBuffers[16]` | Se pone a cero y se libera. No se ha identificado un escritor de payload; el nombre solo expresa propiedad. |
| `0x5428c4` | `SceneNode *g_carAlternateBodyScenes[16]` | Raíces construidas al cargar la variante de carrocería y destruidas al finalizar. |
| `0x542904` | `void *g_carWheelModelBuffers[16]` | Buffers C3D de ruedas alternativas. |
| `0x542944` | `void *g_carAlternateBodyBuffers[16]` | Buffers C3D de carrocería alternativa. |
| `0x542984` | `void *g_carModelBuffers[16]` | Buffers C3D principales. |
| `0x5429c4..c5` | Dos `BYTE` de calidad de coche | Letras de detalle de jugador/oponentes para knockout a pantalla dividida. |
| `0x5429c6..c7` | Dos bytes desconocidos | Se conserva la región sin atribuirle significado. |
| `0x5429c8` | `void *g_carInfoBuffers[8]` | Buffers CIN por coche. Sus entradas relativas contienen perfiles de física/contacto; no son datos de cinemáticas. Formatos internos pendientes. |
| `0x5429e8..542aaf` | `0xc8` bytes desconocidos | Cola de la antigua declaración, conservada sin reinterpretarla. |

Las tablas son globals independientes con sus anotaciones originales. El
compilador y linker pueden reubicarlas de forma independiente: ningún
consumidor nuevo alcanza una tabla desde la dirección de otra. No se infiere
que toda esta región correspondiera a un único objeto C++ en el fuente original.

Un agregado único producía código distinto en la inicialización y liberación
(80,89 % y 66,67 % de coincidencia, respectivamente), incluso conservando todos
los offsets. Separar las tablas reproduce ambas funciones byte por byte. Esto
es evidencia del comportamiento del compilador; la comprobación funcional
adicional verifica los límites y las relaciones entre tablas reubicadas.

La inicialización conserva un cursor apuntando al miembro `bodyNode`, escribe
`rootNode/bodyNode` en ese orden y avanza por `sizeof(CarSceneRecord)`. Su límite
numérico expresa la misma posición de miembro un registro después del array.
Esta forma mantiene las instrucciones originales; sustituirla por un cursor
al inicio del registro cambiaba su generación de código. Queda identificada
como una restricción concreta de matching.

## Vínculos y recarga

En `Car`, `pNode0x71c/720/724` se recuperan como `pSceneRoot`, `pBodyNode` y
`pAlternateBodyNode`. La carga y `Car_BindModel` (`0x43e5a0`) establecen estos
vínculos, y los consumidores de transforms los usan de forma consistente.
`physicsMatrix` y `bodyMatrix` sustituyen los nombres de dirección de las dos
matrices en `+0x00/+0x40`: la inicialización hace que `pWorld/pBodyMatrix`
apunten a ellas, respectivamente.

Las siete tablas temporales de `Car_ReloadModels` (`0x42b800`) ahora son arrays
`SceneNode *` con nombres que describen lo que guardan/restauran: raíz,
carrocería, carrocería alternativa, vista cercana/lejana, cuatro ruedas y
cuatro nodos adicionales por coche. Se conservan sus capacidades de ocho
coches y los offsets originales de las anotaciones.

## Validación y límites

La comparación parcial de los 45 archivos afectados y la reconstrucción
completa conservan las 2922 funciones byte-exactas de 3364. No hay pérdidas
ni cambios de puntuación frente a la tanda 05; perfect 56,10 %, fuzzy 94,50 %.
Los hashes y resultados constan en [car-resources-matching.json](car-resources-matching.json).

La nueva prueba `differential_car_resources.py` ejecuta 128 casos contra los
dos ejecutables, con las 16 entradas, búsquedas/getters reales, restauración
de objetos antes de destruir, orden de liberación, combinaciones de nulos,
guardas de toda la imagen/heap y registros preservados por la ABI. Solo las
operaciones finales de destrucción/liberación y caché son leaves controladas.
La fixture de contactos usa la anotación de la tabla CIN independiente,
en lugar de su antiguo desplazamiento desde el blob.

El inventario pasa de 2149 a 2078 candidatos de acceso crudo y de 108 a 102
tamaños literales. Detecta 292 tipos y 1514 declaraciones de campos todavía
sin nombre semántico: exponer los huecos desconocidos del nuevo registro
impide presentar el mero cambio de representación como recuperación completa.

Los 106 harnesses registrados pasan sobre el build medido, sin fallos.
También pasa la prueba de publicación concurrente del mapa de anotaciones.

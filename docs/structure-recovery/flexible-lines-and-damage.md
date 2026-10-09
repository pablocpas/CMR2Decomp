# Líneas flexibles y snapshots de daños: tercera tanda

## Registros de líneas del coche

`CarFlexibleLineDescriptor` recupera el descriptor serializado de `0x20` bytes:
posición local en `+0x00`, eje de reposo en `+0x0c`, longitud en `+0x18` y color
RGBA en `+0x1c`. `CarFlexibleLineState` recupera el estado de `0x3c` bytes:
posición/deflexión, posición anterior, deflexión interpolada, extremo del
segmento apoyado en el suelo, dos velocidades de resorte y flag de contacto.

La reserva de `0xb4` bytes equivale a tres estados por coche. El descriptor y
su contador proceden del archivo del modelo; el código usa el contador del
modelo, sin añadir un clamp ni inferir nuevos límites de comportamiento.
Los tipos se llaman líneas flexibles por su representación y simulación:
los nombres históricos de las funciones no establecen si representan una
antena, un rastro u otro objeto del modelo.

| Global | Dirección original | Tipo y evidencia |
| --- | --- | --- |
| `g_carLineStates` | `0x00590c6c` | Array de punteros a estados, reservado por coche; reset, integración, interpolación y dibujo indexan los mismos registros. |
| `g_carLineDescriptors` | `0x00590c00` | Ocho punteros a los descriptores de modelos; contacto y dibujo usan posición, eje, longitud y color. |
| `g_carLineCounts` | `0x00590b30` | Ocho punteros al contador serializado; las pasadas leen el contador, no su dirección numérica. |
| `g_carLineImpulse` | `0x00590b50` | Velocidad del coche en su marco menos el impulso aleatorio; se proyecta respecto al eje antes de integrar el resorte. |
| `g_carPartNodes` | `0x00590b7c` | `SceneNode *[4][8]`, asignados por clase del nodo, usados al activar piezas. |
| `g_carPartModelIndices` | `0x00590c24` | `BYTE[4][8]`, índices del modelo asignados junto con el nodo; seleccionan centros y extensiones. |

`StageTiming_AttachObjectToCurrentCar` ahora expresa los enlaces y copias de
matrices mediante `PartState` y `SceneNode`. El tamaño de copias y reservas
se expresa con `sizeof`.

`SceneNode::viewMask` sustituye `field_0x17c`: los dos setters de máscaras y
los lectores de render prueban bits por vista. `PartState::boundsRadius`+sustituye `field_0x15c`: se calcula como longitud de las semiextensiones y se
usa como radio en la prueba de corrección de movimiento.

## Registros de daño

Las reservas, las copias desde registros de pilotos, el reinicio y la
restauración establecen este layout de `CarDamageRecord`, de `0x290` bytes:

| Campo | Offset/tamaño | Evidencia |
| --- | --- | --- |
| `impacts` | `+0x000`, `0x106` | Pool activo de veinte links de trece bytes, seguido de count y firstLink de un byte. Restauración recorre la cadena por índice. |
| `stageImpacts` | `+0x106`, `0x106` | Pool preservado que acumula impactos hasta capturar el snapshot; se copia al pool activo al reiniciar. |
| `damage` | `+0x20c`, `0x40` | Intensidades cuantizadas de 34 partes, dos bytes de alineación, cuatro flags de piezas ocultas y tres de líneas apoyadas en el suelo. |
| `stageDamage` | `+0x24c`, `0x40` | Snapshot preservado con el mismo layout; se copia entero al estado activo. |
| `stageSnapshotCaptured` | `+0x28c`, `4` | Evita capturar otra vez y detiene la acumulación de impactos para el snapshot. |

`CarDamageLinkPool` y `CarDamageSnapshot` describen esos dos bloques repetidos.
El antiguo `hasLinks` contenía un contador, aunque la restauración solo
comprobaba si era distinto de cero. Los doce bytes de carga de cada impacto
siguen pendientes de recuperar; no se han presentado como padding.

`CarPartSet::partHidden[4]` y `lineGrounded[3]` sustituyen los campos finales
`field_0x4b0` y `field_0x4c0`. Los flags se guardan en el snapshot y se restauran
mediante los setters que ocultan nodos o reinician el estado de líneas.
`g_carDamageRecords` (`0x00588b98`) es ahora un array tipado de esos registros.

Todos los tamaños, límites entre bloques y offsets anteriores se verifican
en `LayoutChecks.cpp`, incluidos los pools packed de `0x106` bytes.

## Validación y pendientes

La reconstrucción y auditoría completas conservan las 2922 funciones exactas.
La única variación de puntuación frente a la segunda tanda es una mejora en
`CarPart_ResolveLocalPointGroundContact`; no hay pérdidas de exactitud ni
problemas de datos globales.

`differential_car_flexible_lines.py` añade 4320 casos con modelos enteros
independientes para reset, resorte y contacto. Comprueba ocho coches, tres
slots, índices enmascarados, retornos anticipados, normales alineadas y casos
degenerados, writes en todo el heap y ABI. Los helpers de punto fijo y rotación
son reales; solo la consulta de terreno es una hoja controlada. El dibujo,
interpolación y enlace de matrices conservan su coincidencia byte-exacta.

El inventario queda en 2172 desreferencias crudas y 1581 declaraciones de
campos sin nombre semántico detectadas. La recuperación sigue abierta:
quedan la carga de impactos, consumidores que usan índices de enteros en
conjuntos de piezas, campos de estado móvil y los demás subsistemas del
[inventario](inventory.json).

La suite completa de 104 harnesses pasa sin fallos, incluido el nuevo. Los
hashes y métricas se conservan en
[flexible-lines-and-damage-matching.json](flexible-lines-and-damage-matching.json).

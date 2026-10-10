# Perfiles CIN — investigación previa a las tandas 16–18

Esta nota conserva el estado previo a la conversión. La evidencia actual, los
tipos y sus límites se documentan en [perfiles CIN y cockpit](cin-and-cockpit-records.md).
La sección 4 se ha rastreado hasta sus offsets de cámara; los tamaños totales
serializados y los campos sin semántica siguen pendientes.

Los buffers `g_carInfoBuffers[8]` de la tanda 06 proceden de los archivos
`%s.cin`. `StageTiming_GetStartArchiveRelativeEntry` (`0x457e10`) obtiene el
buffer según el `signed char` de `Car::index`, lee un offset `unsigned short`
y lo suma a la base. Para índices de sección mayores que cero avanza dos
bytes por índice; para cero o negativos lee la primera entrada. La llamada
al getter se repite dos veces y hay que conservar esos accesos al convertir
la API a un tipo recuperado.

Se han observado seis índices de sección; todavía no se declara que sean
los únicos ni se fija el tamaño total del directorio sin inspeccionar los
assets y los demás lectores.

| Índice | Consumidores observados | Layout establecido hasta ahora |
| --- | --- | --- |
| 0 | `StageObject_RebuildCarLightMeshes`, `0x463fe0` | Un `int` de cantidad seguido de `CarLightPoint` de `0x28` bytes. Pendiente recuperar el contenedor y los globals que apuntan a él. |
| 1 | `CarContact_Initialize`, `0x494bb0`, y consumidores de skid/contacto | Byte de cantidad, tres bytes de rangos, `int` en `+4`, `int` en `+8` y perfil desde `+0xc`. Deben rastrearse los consumidores del perfil para fijar todos los campos y su extensión. |
| 2 | `CarEffects` | Tres grupos de cuatro bytes usados como tintes/escalas; vértices `FixVector` desde `+0xc`. Pendiente establecer cantidad y variantes de los vértices. |
| 3 | `StageTiming_CacheListedCarTimingPointers`, `0x4809e0` | `FixVector` en `+0`, cantidad `int` en `+0xc`, descriptores `CarFlexibleLineDescriptor` desde `+0x10`. El vector se usa en `CarPart_UpdateFlexibleLines`; los descriptores fueron recuperados en la tanda 03. |
| 4 | `StageObject_CacheCarSplitVectorPointers` | Se conserva un puntero por coche. Hay que seguir todos los lectores de la tabla antes de dar nombre a su payload. |
| 5 | `StageObject_CacheCarClassAndTimingPointers`, `0x476540` | Siete vistas cacheadas a offsets `+0`, `+8`, `+0xc`, `+0x18`, `+0x1c`, `+0x24`, `+0x2c`. La tabla de punteros tiene stride `0x1c`; varios consumidores prueban ángulos y pares de shorts, sin que eso permita aún nombrar todas las vistas. |

Para la siguiente tanda: recuperar contenedores de secciones y el registro
de siete punteros; sustituir vistas desde buffers por campos y arrays;
comprobar offsets/ancho/signedness y capacidad con assets y ejecutable;
validar todas las funciones de los archivos implicados y la suite completa.
Los perfiles con longitud variable requieren conservar el encabezado y la
cardinalidad observada, sin inventar arrays fijos a partir de un solo lector.

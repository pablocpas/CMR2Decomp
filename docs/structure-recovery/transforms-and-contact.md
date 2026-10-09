# Transforms, temporización y contactos: quinta tanda

Los snapshots `CarTransforms` y los contactos `CarContact` tienen ahora todos
sus miembros expresados mediante tipos y nombres asociados a sus usos.
Se conservan la identidad de los globals, sus capacidades declaradas y el ABI
de las funciones. Los nombres históricos de accessors y funciones se mantienen
para que la auditoría siga identificando las mismas funciones.

## Layouts y tablas

| Registro o tabla | Layout | Evidencia |
| --- | --- | --- |
| `CarWheelRecord` | `0x18`: posición `+0`, inclinación `+0xc`, dirección `+0x10`, giro `+0x14` | `Car_StoreRenderTransforms` (`0x42af50`) escribe los cuatro registros; `Car_InterpolateRenderTransforms` (`0x42bf70`) interpola cada componente y construye las matrices de rueda. |
| `CarTransforms::wheels[4]` | `+0x80`, ocupa `0x60`; registro completo `0xfc` | Copias y reinicio por cuatro ruedas; siguen el normal en `+0xe0` y cuatro alturas en `+0xec`. El antiguo buffer opaco y la macro `CAR_WHEEL` desaparecen. |
| `g_carPhysicsTransforms[8]` | Global `0x53a3a8`, stride `sizeof(CarTransforms)` | Se restaura del shadow, copia y usa como extremo anterior de la interpolación. Ocho registros de `0xfc` terminan en el siguiente global anotado, `0x53ab88`. |
| `g_carTransformsInvalid[8]` | Global `0x53acf0`, ocho int | Los setters de `0x42c840/0x42c870` marcan el estado; `0x42bf70` copia el shadow a la fila anterior antes de interpolar y limpia la marca. Corrige el comentario que los llamaba flags de «dibujado». |
| `g_carRenderBoxCorners[10][8]` | Global `0x53c5a0`, ocho `FixVector` por fila: `0x60` | `Collision_BuildOrientedBoxWorldCorners` escribe las ocho esquinas; `CarPhysics_UpdateBodyContactAndSkidTrail` usa sus puntos. Se conserva la capacidad previamente declarada de diez filas. Los consumidores examinados justifican los ocho slots de coche; el uso de las dos filas adicionales sigue sin establecerse. |
| `g_carContacts`, `g_carContactCount` | Globals `0x592734/0x592738`, pool de registros `0x2a4` | Reserva `count * sizeof(CarContact)`, puesta a cero, callback de liberación, indexación, contactos de ruedas/cuerpo y sombras. `StageObject_GetMotionRecord` devuelve `CarContact *`. |
| `g_carTyreTrailCenters[8][4]` | Global `0x549c20`, cuatro centros por coche | `0x4657d0` calcula los puntos medios de `wheelFrontMid` y `wheelRearMid` y eleva Y en `0xccc`. Los escritores de huellas obtienen los extremos izquierdo/derecho de estos centros. Corrige la interpretación anterior como puntos de escape. |

Los ángulos de rueda son int con signo en grados 16.16. El primer ángulo deriva
los primeros cuatro `CarPartSet::damageValues`; el segundo usa la dirección
suavizada y queda a cero en las ruedas traseras; el tercero usa
`g_wheelRotation`, integrado por `Car_IntegrateWheelRotation` (`0x42b4a0`).
Por eso se llaman `tiltDegrees`, `steeringDegrees` y `spinDegrees`, en lugar de
la interpretación previa que atribuía el giro al primer componente.

Las copias de matrices y ruedas usan `sizeof`; las alturas y normales usan
miembros directos. `Car_GetPhysicsRow` devuelve `CarTransforms *` y
`Car_GetRendererRecord` devuelve `FixVector *`. Los consumidores de cámara y
contactos reciben esos tipos. Los daños de rueda leídos desde `+0x240/+0x250`
usan el array recuperado `damageValues` sin cambiar el orden de las llamadas.

## Temporización del coche

| Campo | Offset | Contrato observado |
| --- | --- | --- |
| `simulationInterpolation` | `Car +0xa90`, int | Fracción del paso en 16.16; interpola los transforms y las cámaras. |
| `simulationRateHz` | `Car +0xa98`, float | Valor inicial 25; convierte los milisegundos transcurridos a pasos de simulación. Replay puede sustituirlo por el valor del archivo. |
| `simulationStepsRemaining` | `Car +0xb43`, BYTE | Número de pasos pendientes del frame; se programa, limita a cinco y consume en el bucle de carrera. La conversión original a char y el almacenamiento BYTE se conservan. |

La función históricamente llamada `Car_UpdateEngineNoteFalloff` (`0x42c890`)
programa los pasos, guarda su fracción y devuelve el mayor contador. El bucle
de carrera usa ese máximo y solo simula un coche mientras le queden pasos.
El comentario que describía `+0xb43` como indicador de coche de jugador se
corrige en todos los consumidores. Se nombran también el mask de inicialización,
época, frame actual, frame anterior y factor de conversión de la simulación:
`g_carSimulationClockFlags`, `g_carSimulationEpochMs`, `g_carSimulationFrameMs`,
`g_carSimulationPreviousMs`, `g_carMillisecondsToSeconds`.

Los cuatro bytes `Car +0xa94` permanecen opacos. No hay evidencia para llamarlos
relleno ni para inventarles un tipo o función: se conservan como
`BYTE field_0xa94[4]` al separar el bloque de temporización.

## Campos restantes del contacto

| Campo | Offset | Lectores/escritores |
| --- | --- | --- |
| `shadowLevel` | `+0x250`, int | Inicializado a `0x9999`; `CarShadow_SetLevel` (`0x4984b0`) modifica el nivel y las alphas. La sombra de derrape lo usa también para su contorno suave. |
| `ghostContact` | `+0x294`, int | En modos 5/6/7 lo activa el inicializador para coches distintos del primero. Selecciona el cuerpo proyectado simplificado, sombra opaca y ausencia de trail de derrape. |
| `wheelPatchesEnabled` | `+0x298`, int | Condiciona actualización, dibujo y desplazamiento hacia cámara de los parches de rueda. |
| `skidIndexOffset` | `+0x29c`, int | El constructor guarda 0/1 según la esquina de salida. El test de visibilidad resta uno cuando está activado. |
| `skidRangeAscending` | `+0x2a0`, int | El constructor lo determina del signo lateral; el test de visibilidad invierte el índice cuando vale cero. |

Los cinco miembros mantienen su ancho int; no se estrechan a bool/byte.
La reserva, liberación, accessor y los consumidores del pool emplean el tipo
`CarContact`; la macro con cast del pool desaparece.

## Exactitud y deuda concreta

`Car_ResetRenderTransforms` conserva un cursor int situado en el segundo word
de cada rueda. Cambiarlo por un cursor `CarWheelRecord *` cambia la dirección
base y los desplazamientos de las seis stores y pierde su coincidencia exacta
(98,02 %). Se conserva el cursor, con origen en el miembro `position.y`, stride
calculado a partir de `sizeof(CarWheelRecord)` y comentario que explica el
motivo. Los demás accesos de la función usan los miembros y tamaños establecidos.

La escritura de los centros de huella usa el índice de rueda y campos de
vector. Un cursor de salida tipado independiente reducía la coincidencia de
72,16 % a 67,03 %; el acceso directo al array la aumenta a 89,01 % y conserva
las coincidencias exactas del resto del archivo.

`LayoutChecks.cpp` comprueba los tamaños y offsets de snapshots, ruedas,
contactos y temporización. La prueba nueva
`tests/differential_car_contact_centers.py` cubre 352 casos de coches/ruedas,
diferencias impares, wrap de enteros con signo, returns por límite superior y
slot cero nulo, contactos de solo lectura, guardas de salida y ABI. Ejecuta el
getter y los helpers reales en ambas imágenes; el modelo del punto medio es
independiente. La prueba existente de transforms cubre las copias y generación
de ruedas con listas repetidas, daños, suspensión y cuatro modos de x87.

Resultados y hashes: [transforms-and-contact-matching.json](transforms-and-contact-matching.json).

La reconstrucción completa conserva las 2922 funciones byte-exactas de 3364,
sin pérdidas; la única variación de puntuación es la mejora del cálculo de
centros. No hay problemas de datos globales. Los 105 harnesses diferenciales
pasan y los hashes de fuentes/ejecutable corresponden al build medido.
El inventario pasa de 2168 a 2149 accesos crudos, de 116 a 108 tamaños literales
y de 1521 a 1514 declaraciones de campos sin nombre semántico.

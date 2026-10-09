# Piezas y daños del coche: segunda tanda

Se conserva el ABI de las funciones y la identidad original de sus globals.
Los nombres de funciones que históricamente dicen `ReplayRecord` siguen
existiendo para no mezclar esta recuperación con el cambio de todos sus
consumidores. Sus buffers contienen conjuntos de geometría y daños del coche.

## Layouts y campos establecidos

| Tipo o campo | Offset/tamaño original | Evidencia |
| --- | --- | --- |
| `CarPartStateTables` | `0x14`, modes en `+0x10`; global `0x00590d7c` | Cuatro arrays de `PartState`, uno por slot, asignados y liberados juntos. Selección por índice de coche; cuatro bytes usados por transforms de objetos. |
| `PartState` | `0x1a0`; matrices `+0xc/+0x4c/+0x8c`, callback `+0x110`, flags `+0x150` | Reserva, reinicio, actualización e interpolación de piezas móviles. Los consumidores usan los miembros existentes. |
| `g_carPartSets` | `CarPartSet *`, registros `0x4d0`; global `0x00588b94` | Reserva de `count * 1232`, indexación por coche y funciones de deformación, rotura y geometría. |
| `g_carPartSetLookup` | `CarPartSet *[8]`; global `0x00588990` | Entradas inicializadas con direcciones de registros sucesivos de ese buffer. |
| `g_physicsPartSet`, `g_autoGearPartSet` | Punteros a `CarPartSet`; globals `0x0053cc1c/0x00592270` | Reciben el mismo accessor por índice de coche; los consumidores leen sus campos de daños. |
| `damageValues[34]` | `+0x240` | `StageObject_RebuildDamagePartValues` deriva los valores de `damageGrid`, los limita a 16.16 y actualiza los efectos mecánicos. |
| `damageScales[34]`, `damageBiases[34]` | `+0x2c8/+0x350` | Multiplicación de cada valor, suma del sesgo y límite superior. Restauración de sesgos a partir del daño serializado. |
| `steeringWobble` | `+0x3d8` | Calculado del daño; `Car_UpdateSteering` lo usa para escalar ruido de dirección. |
| `suspensionDamageOffset[4]` | `+0x3dc` | Sumado al desplazamiento de suspensión de cada rueda en las dos variantes de integración. |
| `wheelDamageDrag[4]` | `+0x3ec` | Torque que se opone al giro de cada rueda, con signo elegido según la carga. |
| `frontBrakeScale`, `rearBrakeScale` | `+0x3fc/+0x400` | La integración de frenos selecciona el factor delantero o trasero. Se inicializan a `0x10000` y disminuyen con el daño. |
| `engineTorqueScale` (inicialmente `steeringFollowScale`) | `+0x404` | Multiplica el par del acelerador. La investigación del motor de la [cuarta tanda](car-controls.md) corrige la interpretación inicial de este miembro y de `steerFollowRate`. |
| `bodyDamageDrag` | `+0x408` | Incremento del coeficiente base de resistencia multiplicado por velocidad absoluta. |
| `breakPartIndex[8]` | `+0x460` | El slot de rotura selecciona el id de modelo y las ventanas que se rompen. |
| `gearShiftDamage` | `+0x468`, byte | Cuantización de `damageValues[14]` y lectura por el cambio automático; se conserva la conversión explícita a `char`. |
| `glassCooldown`, `glassDebrisEmitted` | `+0x469`, byte; `+0x46c`, int | `Car_SpawnDebris` elige partículas, aumenta el contador y marca la emisión. El paso del coche reduce el contador cuando no hubo emisión y limpia la marca. |
| `partDamaged[8]`, `partBroken[8]` | `+0x470/+0x490` | `StageTiming_FlagCarPartBreaks` aplica las apariencias 1/2 una sola vez. Antes se expresaban como índices negativos desde `+0x490`. |

Se han nombrado 14 declaraciones antes basadas en offsets; una de ellas,
`field_0x3fc[3]`, se descompone en tres campos con funciones distintas. Los
offsets y tamaños recuperados se comprueban en `LayoutChecks.cpp`.

`CarDamage_BuildRelativeVelocityHull` tenía mal interpretado su segundo
argumento como `Car *`. Los tres callers pasan el conjunto de piezas; las
lecturas en `+0x410/+0x414/+0x418/+0x41c` son `maxX/minX/maxZ/minZ`. El tipo y
los accesos se corrigen conservando las direcciones y anchos de las lecturas.

## Exactitud y trabajo pendiente

El reinicio de estados conserva el avance de un cursor en bytes usando
`sizeof(PartState)`: sustituirlo por un contador de elementos altera el código
generado. La activación de slots también conserva el cursor de bytes inverso
para compartir el desplazamiento de las tablas de estado y umbrales. Estas
dos conversiones directas perdían la coincidencia exacta y se ajustaron.

Quedan campos ambiguos en el conjunto de piezas, en los registros de daño y en
el estado móvil. También quedan consumidores de geometría que ven el registro
como `int *` y las tablas de puntos de contacto de `0x3c`, con descriptores
serializados de `0x20`. Su recuperación requiere seguir todos sus lectores y
escritores; no basta con cambiar el nombre del puntero.

El comparador paralelo presentaba una carrera al publicar el mapa de globals
antes de terminar de leer las anotaciones. Se publica ahora el mapa completo;
`tests/test_fastcmp_annotations.py` comprueba el caso con un lector concurrente.
Esto evita falsas pérdidas cuando se renombran globals entre builds.

El inventario pasa de 2237 a 2196 accesos crudos, de 129 a 123 tamaños literales
y de 1605 a 1591 declaraciones de campos sin nombre semántico detectadas.
Son métricas sintácticas, no una certificación de recuperación completa.

La reconstrucción completa conserva las 2922 funciones byte-exactas de 3364,
sin cambios de puntuación respecto al inicio de esta tanda y con cero problemas
de datos globales. Los 103 harnesses diferenciales pasan sin fallos. Los hashes
y métricas constan en [car-parts-matching.json](car-parts-matching.json).

# Activación y nombres de piezas móviles

Esta tanda continúa la [interfaz de piezas](car-part-table-interface.md).
Los nombres describen los usos observados; no se atribuyen al fuente original.
Las cuatro tablas contienen piezas móviles de carrocería asociadas a clases de
nodo 9/10/12/13 por `StageObject_AssignMatchingCarClassSlots`, no registros de bujes.

| Dirección | Nombre recuperado | Evidencia |
| --- | --- | --- |
| `0x480a60` | `CarPart_ResetDamageThresholds` | Reseed de RNG a 400; umbrales `0x8000/0x8000/0x8000/0x3333` e índices de daño `22/20/21/18`. |
| `0x480bb0` | `CarPart_UpdateOrderedCars` | Recorre el orden de coches desde el final, activa piezas por daño, calcula vibración y ejecuta los callbacks de las piezas activas. |
| `0x480cb0` | `CarPart_UpdateVibrationAngle` | Usa velocidad y `abs((position.x + position.z) % 1024)` para limitar un escalar por coche y convertirlo a ángulo. No lee la dirección. |
| `0x480de0` | `CarPart_AttachDamagedSlots` | Recorre slots 3..0; exige flag oculto cero, nodo no asociado y daño estrictamente superior al umbral. |
| `0x480e50` | `CarPart_InitMovingSlot` | Selecciona modelo, centro, semiextensiones, pivote e integrador por slot/tipo de carrocería; limpia el estado dinámico. |
| `0x4813b0` | `CarPart_AttachSlotNode` | Asocia el nodo, copia sus matrices y base, selecciona el padre de escena y la matriz de fuerzas del cuerpo. |
| `0x481560` | `CarPart_StoreRotatedBasis` | Copia la base, rota la copia solo si la velocidad supera `0x7ae` y guarda sus tres ejes en `pWorld`. |
| `0x4816f0` | `CarPart_IntegrateSwingAndDrag` | Integra balanceo, fuerza elástica y arrastre tangencial del suelo; maneja desprendimiento. |
| `0x483050` | `CarPart_StartDamageSwing` | Daño del slot superior a `0xcccc`; los pares de daño 2/3 o 0/1 eligen el signo del pivote Z. |
| `0x483100` | `CarPart_EmitGroundDebris` | Usa superficie de la esquina, semiextensiones y velocidad vectorial para construir ejes de residuos y llamar a `Car_SpawnDebris`. |

El mapa de renombres registra además cuatro globals: umbrales e índices de
daño (`0x590c44/0x590c60`), escalares de vibración por coche (`0x590b10`) y
ángulo compartido (`0x590c68`). Este último conserva almacenamiento `int`:
el productor solo escribe la palabra baja y los consumidores usan lecturas
con y sin signo de 16 bits. No se altera la palabra alta.

El recorrido de activación reemplaza el offset decreciente y los casts por
`parts[slot][car.index]` y `thresholds[slot]`, conservando bytes exactos con
MSVC6. El emisor recibe `FixVector *`, sin conversión por `int *` ni puente
`Car * -> BYTE * -> Car *`. La orientación lee `basis.right.x` directamente.
Se nombran `modelIndex`, `pivotOffset` y los índices de daño por sus usos.

`PartState::field_0x154` y `field_0x160` siguen sin nombre semántico: solo se
ha encontrado su puesta a cero. No se confunden con relleno. El getter de
ocultación sigue leyendo el array contiguo, mientras el daño se consulta en
`g_partSet`; las pruebas usan conjuntos distintos para conservar esa diferencia.

## Validación

La compilación y medición completas conservan las 3364 puntuaciones `s/fz/x`,
2924 funciones exactas, 56,28 % de bytes perfectos y 94,46 % de similitud fuzzy,
con cero problemas de comparación de datos. El gate comprueba nueve unidades.
Pasan los 138 harnesses diferenciales sobre ese mismo build medido.

`differential_car_moving_parts.py` añade 1584 escenarios contra el original:
asociación e inicializadores reales, cuatro slots, coches 0/7, tipos 0/8/9/11,
ambos sentidos de daño y empate, nodo ausente/existente, estados ocultos no
canónicos, límites con signo, igualdad de umbral y orden inverso. Verifica el
heap completo envenenado, el callback instalado, las escrituras globales y ABI.
Los callbacks se instalan pero no se ejecutan en esta prueba.

La prueba nativa de tablas ejecuta 16 cuerpos de producción en 2966 casos con
ASan/UBSan, headers reales y punteros mayores de 4 GB. Añade todas las
combinaciones de filtros/umbrales de los cuatro slots; solo la inicialización
del slot es una hoja controlada en esos casos nativos de activación.

El inventario se regenera con hashes actuales. El archivo anterior estaba
atrasado (749 candidatos crudos): al auditar de nuevo el HEAD de partida hay
120, frente a 118 después de esta tanda. Este lote elimina dos candidatos
sintácticos, no los 631 acumulados desde la última actualización del archivo.
Los identificadores desconocidos distintos pasan de 772 a 770; los 1407 campos
sin nombre semántico permanecen pendientes. El objetivo general sigue abierto.

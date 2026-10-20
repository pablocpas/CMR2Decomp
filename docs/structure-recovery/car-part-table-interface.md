# Interfaz de piezas y visibilidad del coche

Los nueve nombres de esta tanda se apoyan en los accesos del ejecutable y sus
productores/consumidores. Son nombres descriptivos recuperados por evidencia;
no proceden de símbolos originales. El mapa está en
`scripts/renames/017-functions.tsv`.

| Dirección | Nombre | Evidencia |
| --- | --- | --- |
| `0x4667c0` | `CarPart_AllocateTables` | Reserva `CarPartSet`, `CarDamageRecord` y las tablas de copias de vértices/tipos de nodo; inicializa el lookup y registra su liberación. |
| `0x466680` | `CarPart_FreeTables` | Libera primero los vértices de cada pieza, luego las tablas por coche y las tablas exteriores, y finalmente ambos arrays de registros. |
| `0x469680` | `CarPart_GetSet` | Devuelve `g_carPartSetLookup[carIndex]`; los consumidores leen geometría, daños y flags de piezas. El antiguo nombre hablaba de replay. |
| `0x469bc0` | `CarPart_GetHiddenState` | Lee `g_carPartSets[pCar->index].partHidden[slot]`, el dword de uno de los cuatro slots en `+0x4b0`. |
| `0x469bf0` | `CarPart_MarkHidden` | Escribe uno en ese flag y llama a la reconstrucción de daños derivados. `0x483010` lo invoca al desactivar la pieza móvil. |
| `0x480ac0` | `CarPart_HideNode` | Si el nodo existe y el flag es distinto de cero, escribe cero en `SceneNode::viewMask`. Un flag cero conserva la máscara. |
| `0x484d10` | `CarPart_GetModelIndex` | Lee el byte `[slot][carIndex]` que `0x480af0` guarda junto al nodo al reconocer su clase. Conserva el retorno `unsigned int` y valores como `0xff`. |
| `0x484de0` | `CarPart_GetNode` | Devuelve el `SceneNode *` guardado por `0x480af0`; el montaje de luces lo usa como nodo. |
| `0x46b670` | `CarPart_ApplyHiddenStates` | Obtiene el conjunto del coche y aplica sus cuatro flags con `CarPart_HideNode`. |

Las declaraciones y llamadas usan ahora `Car *`, y el getter de nodos devuelve
`SceneNode *`, sin puentes por `BYTE *`/`void *`. Se nombran los parámetros
`carIndex`, `slot`, `hidden` y los recorridos de liberación por coche/pieza.
Los flags siguen siendo enteros y el índice del coche conserva su tipo con
signo. No se introducen conversiones a booleano ni comprobaciones de límites.

El lookup se construye con `&g_carPartSets[carIndex]`: desaparecen el cursor en
bytes, el cast y la suma de strides. Tanto ese recorrido como los ocho helpers
restantes conservan sus bytes originales con MSVC6. Se mantiene la diferencia
entre el getter por lookup y el acceso directo al array; no se supone que ambas
tablas sean intercambiables después de la inicialización.

La medición completa conserva las 2924 funciones exactas de 3364, todos los
scores `s/fz/x` y cero errores de datos. El gate comprueba las 16 unidades
afectadas, incluidas las que reciben el header compartido.

`differential_car_part_records.py` conserva sus 494 casos y añade 72 de reserva
y liberación contra el ejecutable original: cuentas negativas/cero, ocho coches,
tablas ausentes, piezas dispersas, cuentas por coche de -1/0/1/15, tamaños de
reserva, orden de liberación, callback, heap completo, escrituras globales y ABI.
Solo el allocator, la liberación de buffers y el registro del callback son hojas
controladas en estos casos de ciclo de vida.

`check_64bit_car_part_tables.py` ejecuta los nueve cuerpos de producción y los
headers reales con ASan/UBSan: 196 casos, direcciones mayores de 4 GB, tamaños
nativos de registros y punteros, guardas de registros/nodos y lookup con alias
distintos del array contiguo. El cálculo de daños del setter es una hoja que
comprueba que el flag ya esté escrito al recibir la llamada.

Los 512 casos de `check_64bit_address_loops.py` también pasan. Su fixture usa
ahora el allocator renombrado, el nombre vigente `StageObject_SetFrameRate` y
el tipo opaco `ReplayStream *` que ya empleaba el cuerpo de producción.

La batería completa de 136 harnesses diferenciales pasa sin fallos.

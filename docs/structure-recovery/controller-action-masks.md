# Máscaras de acciones del controlador

`ControllerData` conserva sus `0x2f0` bytes y su formato sin punteros. Las diez
palabras desde `+0x128` forman `actionMasks[10]`; una vista con campos nombrados
expresa los usos individuales de inicialización y composición de máscaras.
Ambas vistas se usan en producción y comparten los mismos veinte bytes.
Trece comprobaciones fijan el tamaño del registro, el offset y cardinalidad del
array y los diez offsets individuales.

| Acción | Offset | Campo | Evidencia del consumidor |
| --- | --- | --- | --- |
| 0 | `0x128` | `steerLeftMask` | Indicador de dirección izquierda; el eje X negativo activa el mismo control. |
| 1 | `0x12a` | `steerRightMask` | Indicador de dirección derecha. |
| 2 | `0x12c` | `throttleMask` | Indicador de acelerador. |
| 3 | `0x12e` | `brakeMask` | Indicador de freno. |
| 4 | `0x130` | `handbrakeMask` | Indicador de freno de mano. |
| 5 | `0x132` | `shiftUpMask` | Petición de cambio `1`, consumida como marcha actual más uno. |
| 6 | `0x134` | `shiftDownMask` | Petición de cambio `0xff`, consumida como marcha actual menos uno. |
| 7 | `0x136` | `cameraCycleMask` | Llama a `View_UpdateDriverCameraCycle`. |
| 8 | `0x138` | `driverCycleMask` | Activa `StageObject_CycleDriverCameraSelection` (`0x47bad0`). |
| 9 | `0x13a` | `backMask` | `InRaceMenu_IsBackPressed` (`0x404e10`) consulta esta acción para volver. |

`StageObject_ReadCarControllerMapping` (`0x47bca0`) copia las acciones 0–8 a
`g_carButtonMasks`. `StageObject_UpdatePlayerControlIndicators` enlaza esos
índices con los controles y las dos operaciones de cámara. El consumidor de
peticiones de marcha en `TrackCollision.cpp` distingue subir y bajar. Estos
nombres describen la evidencia disponible; no se atribuyen al fuente original.

`GetButtonMapping` (`0x40c2a0`) resuelve un slot lógico mediante la tabla de
ocho entradas y accede a uno de los seis registros mediante el tipo. El cursor
`pActionMask` sustituye diez desreferencias con stride y offsets crudos; el
puntero al registro se obtiene después, cuando lo necesita el predicado.
Esta estructura conserva las 157 instrucciones originales. Los consumidores
del editor usan el array real en lugar de recorrer campos escalares adyacentes.

El almacenamiento sigue siendo `unsigned short` y el resultado `short`:
`0x8000` y `0xffff` conservan sus resultados con signo. Los defaults dependen
del slot lógico 0/1, no del índice de registro al que apunta. Las máscaras
físicas iniciales varían por dispositivo; no se sustituyen por `1 << acción`.
La composición `usedMasks` de `AssignUnmappedControllerActions` excluye la
acción 8 e incluye `0x400`, exactamente como el original.

## Validación

La compilación y el gate de las 18 unidades afectadas por las cabeceras no
pierden puntuaciones byte/fuzzy ni funciones exactas. La medición completa
conserva las 2924 exactas de 3364 funciones y no informa errores de datos.
Solo mejora `GameMenu_SetupStageResults` (`0x44a1b0`): 72,76 → 74,24 % de
bytes y 86,58 → 87,15 % fuzzy, por el efecto de las declaraciones compartidas
sobre MSVC6; su fuente y comportamiento no cambian.

El nuevo harness diferencial compara 1944 casos con el original y el build
medido: slots primarios y alias, tres registros, diez acciones y defaults,
máscaras cero/positivas/con signo y predicados cero/positivos/negativos.
Comprueba resultados, argumentos de la llamada, registros completos, tabla
de slots, heap, escrituras globales y ABI. De esos casos, 216 ejecutan el
predicado real para sus nueve entradas de teclado declaradas.

La prueba nativa extrae las dos funciones y las declaraciones de producción.
Sus 1944 casos comparan resultados y llamadas con el original x86 bajo
ASan/UBSan, con punteros por encima de 4 GB y ambas vistas de máscaras en los
mismos offsets. También pasan los 512 casos nativos existentes de copias de
registros y la suite completa de 140 harnesses sobre el mismo build medido.

## Límites pendientes

Las diez máscaras no justifican ampliar el array de teclas: `field_0x13e[9]`
termina justo donde empieza `keyNames[0]`, en `+0x147`. El predicado y algunos
recorridos heredados usan también el índice 9 y cruzan ese límite declarado.
Esta tanda conserva el layout y ese comportamiento x86, sin inventar una
décima tecla ni desplazar los nombres. Para probar todos los caminos del
getter se controla su predicado como proveedor; el predicado real se ejecuta
solo con índices 0–8. No se establece que todo DirectInput o esos recorridos
sean ya seguros en nativo.

El inventario reproducible baja de 109 a 99 candidatos de acceso crudo y de
1401 a 1391 declaraciones de campos sin nombre semántico. Siguen pendientes
la frontera de teclado, los registros de ejes opacos y el resto del inventario.

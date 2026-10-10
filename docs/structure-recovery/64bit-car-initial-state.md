# Estado inicial, bases y suspensión del coche con punteros nativos

Tanda sobre `1a08864`, siguiendo `CODEX-64BIT.md`. Se convierten juntos
los recorridos de inicio/reset, los accesos de suspensión y sus proveedores
de rampa y copia de ruido. `Car` es estado de ejecución; no es una imagen
de disco. No se cambia el formato de ningún fichero.

## Evidencia de campos

| Offset Win32 | Tipo / miembro | Evidencia |
| --- | --- | --- |
| `0x360..0x383` | `right`, `up`, `forward` / `bodyAxes[9]` | Tres vectores y nueve words copiados juntos al guardar/restaurar la base |
| `0x384..0x3a7` | `targetRight`, `targetUp`, `targetForward` / `targetAxes[9]` | Copia completa desde la base y restauración inversa durante el fade; el primer vector estaba declarado como bytes |
| `0x270`, `0x300`, `0x330` | `corners`, `cornerPrev`, `cornerPrev2` | Inicio copia exactamente cuatro vectores a las dos historias, en el orden original |
| `0x4a4`, `0x504` | `cornerAxis`, `cornerNormal` | Ocho vectores copiados uno a uno |
| `0xa74`, `0xa78` | `int`, nombre provisional conservado | Nivel de ruido actual/objetivo: productor de superficies y copia de un word completo |
| `0xa9c`, `0xa9e` | `steepTime`, `cornerTriangle[8]` | Borrado de un short y de ocho shorts |
| `0xb10`, `0xb12`, `0xb18` | Ángulos `short` | Borrados originales de dos bytes |
| `0xb1a`, `0xb1e`, `0xb28`, `0xb45` | Índice, marcha, bytes provisionales | Lectura signed del índice; stores de un byte, manteniendo semántica desconocida de los dos flags |
| `0xb2c`, `0xaae` | `cornerFlags[8]`, `wheelSurface[8]` | Tests de byte por esquina y selección signed de short por rueda |

Las bases tienen vistas de array reales mediante las uniones del `Car`;
se evita caminar desde el primer subobjeto vector hasta los siguientes.
Se añaden 26 comprobaciones de offsets/tamaños en `LayoutChecks.cpp`; el
tamaño original `0xc24` continúa comprobado. Los nombres provisionales
no se sustituyen por interpretaciones sin evidencia.

`Car_ResetBodyBasis` recibe `Car *`. Los getters de rampa y copia de ruido,
sus declaraciones repetidas y todos sus consumidores reciben también
`Car *`. El getter de efecto por rueda usa `wheelSurface[wheel]`. Estos
campos siguen a quince punteros en `Car`: sus antiguos offsets dejan de
ser válidos con punteros de ocho bytes. El código nuevo usa miembros.

## Exactitud y pruebas

- Build y medida completos, `prepare_fastcmp.py` y gate de las 18 unidades
  afectadas: 2922 funciones byte-exactas; ninguna baja en `s`, `fz` o `x`.
- Solo mejora `Car_PlaceAtStart`: score 74,77 → 79,45 % y fuzzy
  92,05 → 93,11 %. Las otras 3363 filas conservan las tres métricas.
- Las cuatro copias de base de `Car_UpdateGroundContact`,
  `Car_PrepareStep` y `Car_StepGroundContact` solo cambian la expresión
  del mismo bloque de 36 bytes. Se verifican idénticos sus flujos completos
  de instrucciones reubicadas respecto del objeto anterior: 885, 905 y
  1281 instrucciones respectivamente, sin símbolos desconocidos. No se
  les atribuye cobertura diferencial nueva.
- `differential_car_initial_state.py`: 3456 casos de los seis cuerpos
  completos, heap de 64 KiB envenenado, ABI stdcall, registros preservados,
  retornos y orden de proveedores. Incluye alias entre matrices externas
  y matrices del propio coche. Rampa y copia de ruido ejecutan sus cuerpos
  reales también dentro del inicio/reset; terreno, actualización de escena
  y parámetros de superficie son hojas controladas. La suspensión tiene
  además un modelo entero independiente, incluidos clamps y fases.
- `check_64bit_car_initial_state.py`: los seis cuerpos actuales se ejecutan
  en x64, 354 escenarios bajo ASan/UBSan. Los quince punteros del coche y
  la tabla de rampas tienen direcciones superiores a 4 GB y se conservan.
  Una proyección **solo del fixture** traduce los dos tramos sin punteros
  al layout de la referencia y compara todo el heap con el original.
  No introduce offsets de 32 bits en el acceso del juego.
- El fixture de rampas reserva 256 registros y sitúa el puntero en el
  medio para probar los 256 índices signed sin salir de la reserva. No
  afirma que el juego reserve esa capacidad.
- Se refuerza `match.py`: una mejora de score ya no puede ocultar una
  pérdida de fuzzy. Tres tests prueban rechazo de esa combinación,
  aceptación de mejoras y rechazo de pérdidas previas de score/exactitud.

La suite completa pasa: **116 harnesses, cero fallos**. Se verifican además
manifest, procedencia y hashes de las fuentes del ejecutable medido.

## Medición y deuda

El inventario pasa de 1489 a **1452 accesos crudos**. De los 37 candidatos
eliminados, 16 eran definiciones de macros sin usos, verificadas en todo
su lifetime; se distinguen de los 21 accesos reales sustituidos. También
desaparecen cinco tamaños constantes y un multiplicador hexadecimal.
No se relaja el auditor ni se ocultan patrones.

Quedan los accesos activos de `Car_Spawn`, los buffers de cámara, el mezclador
`Surface_BlendWheelContactParameters` y el recorrido interior del otro
productor de superficies. El consumidor de glows todavía lleva su coche
en un `int`: la llamada se adapta al prototipo y ese cuerpo sigue pendiente.
Las regiones de significado desconocido del coche conservan su nombre
provisional. **Esta tanda no completa el objetivo global.**

```sh
python3 tests/differential_car_initial_state.py CMR2PROGRESS/entities.json
python3 tests/check_64bit_car_initial_state.py
python3 tests/check_matching_gate.py
```

# Cursores de parámetros de superficie sobre miembros tipados

Tanda sobre `949ac3b`, con referencia actual de **2924 funciones exactas**.
Incluye los avances previos hasta `2687b82` y conserva la corrección byte-exacta
de `CarDamage_BuildRelativeVelocityHull`: vértices 4–5 con `field_0x770[1]`,
6–7 con `[0]`. No sincroniza ni modifica OpenCMR2.

## Evidencia

Los dos productores de superficie trabajan sobre un `Car` completo. El mezclador
pareado (`0x4786b0`) escribía con offsets sobre `BYTE *pWheel`, incluidos
`0xb29`, `0xa74` y `0xa78`, que están después de los quince punteros del coche.
Ahora recibe `Car *` en su definición, declaración repetida y llamador.

| Región Win32 | Tipo / vista | Uso demostrado |
| --- | --- | --- |
| `0x80..0x19f` | `CarCornerGrip[8]` | Nueve words por esquina; el mezclador pareado copia 3→2 y 1→0 |
| `0x1a0..0x1cf` | `CarWheelSurface[4]` | Dos bytes de efecto, drag y extraGrip; se conservan los dos bytes de padding de cada registro |
| `0x880..0x8d7` | `surfaceCompressionWords[22]` | Caminata original continua por los arrays existentes, cornerMass, tyreGrip y field_0x8b8; no se renombra su semántica |
| `0x890`, `0x8a0` | Los dos arrays de wheelSpin ya existentes | Compresión que los productores usan al construir las cuatro esquinas inferiores |
| `0xb29` | Byte provisional existente | Selección de grupo de nueve entradas en g_surfaceDrag |
| `0xa74`, `0xa78` | Words existentes | Ruido actual/objetivo y clamp de 0x3333 por actualización |

Se añaden 15 comprobaciones de offsets/tamaños. Las vistas de ejecución no
son formatos de disco y no contienen punteros reubicados.

## Forma que conserva MSVC6

Cambiar directamente los cursores interiores a `CarCornerGrip *` y
`CarWheelSurface *` bajó ambos scores; se descartó esa variante. La forma que
pasa conserva las direcciones interiores que reutiliza el original y obtiene
la vista del registro con `offsetof`. Todas las lecturas y escrituras del
registro son miembros, incluidos los nueve campos de grip y sus copias.
El avance usa `sizeof(CarCornerGrip)` o `sizeof(CarWheelSurface)`.

Dos uniones de `Car` dan arrays reales a las caminatas que cruzaban subobjetos:
el prefijo fijo de matrices/grip/efectos y el tramo de inputs de compresión.
No se almacenan direcciones en sus words. Los cursores se inicializan mediante
`offsetof(Car, miembro)` y tamaños de los tipos; no mediante offsets de Win32.
El curso de superficies de ocho esquinas obtiene el último id mediante
`&pCar->wheelSurface[7]`.

Se conserva el orden de stores, las copias de esquinas/ruedas pareadas y el
store transitorio del ruido antes de su escala. No se cambian las tablas,
las capacidades ni los valores de suavizado.

## Pruebas y exactitud

- Build, medida completa, prepare_fastcmp y gate de las 18 unidades afectadas.
- 2924 funciones byte-exactas y ninguna pérdida de score, fuzzy o exactitud.
  Solo cambian dos filas: `Car_UpdateSurfaceParams` 59,40→60,03 %,
  fuzzy 72,75→72,85 %; el pareado 82,46→86,62 %,
  fuzzy 83,37→92,97 %. Las otras 3362 filas conservan sus métricas.
- `differential_surface_contacts.py`: 14112 casos del cuerpo pareado completo,
  sin proveedores interceptados. Tablas y matemáticas reales, modelo entero
  independiente, 48 superficies, siete grupos de drag, límites de compresión
  y ruido, heap envenenado completo, padding, esquinas superiores y ABI.
- El productor de ocho esquinas mantiene su arnés completo existente,
  `differential_surface_params.py`.
- `check_64bit_surface_contacts.py`: 480 casos de ambos cuerpos actuales en
  x64, tablas del original, ASan/UBSan y quince punteros mayores que 4 GB
  intactos. Compara el heap primitivo completo con la referencia Win32;
  la traducción de los dos spans sin punteros pertenece solo al fixture.

La suite completa pasa: **117 harnesses, cero fallos**, con manifest, hashes
de fuentes y procedencia verificados. También vuelven a pasar las pruebas
nativas de contacto e inicio/reset sobre el Car de esta tanda.

El inventario baja de **1354 a 1341 accesos crudos**, sin cambiar el auditor.
Los cursores que ahora escriben miembros dejan además de usar índices relativos
de words para expresar los campos del registro; ese avance no se cuenta como
una disminución adicional del inventario.

## Revisión de lo pendiente

La lista inicial de 2455 casts estaba desactualizada. Una copia temporal del
port, con la decomp actual aplicada y los conflictos resueltos hacia la decomp,
produce 593 diagnósticos puntero/entero en 249 contextos. Esa medición **no es
un build completo del juego**: también hay 328 errores de compatibilidad del
snapshot (306 en Graphics, 13 en Game, cuatro en SceneNode, tres en Sprite y
dos en StageTiming). Se conservan los errores y no se interpreta el contador
de warnings como certificación de compilación. Las pruebas nativas anteriores
sí compilan y ejecutan los cuerpos exactos de esta tanda.

Quedan 1341 candidatos sintácticos, entre ellos estructuras runtime, lectores
de formatos fijos y relocadores de disco. Hay que seguir por familias con sus
layouts y pruebas: no hay evidencia para declarar todos ellos un único struct
ni para dar por terminado el trabajo de 64 bits.

```sh
python3 tests/differential_surface_contacts.py CMR2PROGRESS/entities.json
python3 tests/check_64bit_surface_contacts.py
```

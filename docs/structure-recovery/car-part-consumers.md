# Consumidores de piezas del coche: tandas 10–11

La cadena que recupera `StageTiming_GetCarReplayRecord` devuelve ahora
`CarPartSet *` en todos sus consumidores y declaraciones. El nombre histórico
se conserva; el registro contiene geometría y daños, no muestras de replay.
El lote modifica 43 definiciones/cuerpos de función en siete unidades y los
headers compartidos, nombra 12 globals y dos campos, y añade 34 verificaciones
de tamaño, offset y capacidad.

## Evidencia de los tipos y nombres

| Recuperación | Evidencia original |
| --- | --- |
| `CarPartSet::nodes` contiene `SceneNode *` | `0x466e90` guarda el nodo completo. `0x4692b0` compara el byte bajo de `flags`; `0x460ca0` usa `world` en `+0xd8`. `CarPartNode` era una vista parcial redundante. |
| `brokenLightFlickerTimer`, `+0x40c` | `0x46b4e0` compara el dword con cero con signo, escribe duraciones en 16.16 y resta `0x10000`. |
| `brokenLightFlickerPhase`, `+0x4cc` | La misma función alterna 0/1 al vencer el temporizador. El nombre describe esa alternancia sin atribuirle efectos de render no demostrados. |
| `damageValues[23]`, `+0x29c` | Tanto el parpadeo (`0x46b4e0`) como las luces (`0x4643f0`) consultan ese dword. |
| `g_carOriginalPartVertices`, `0x588b9c` | `0x46afe0` reserva una tabla por coche, otra por pieza y arrays de `CarPartVertex`; `0x46acb0` asigna la copia por tipo de nodo; `0x466680` libera las tres capas. Tipo `CarPartVertex ***`. |
| `g_carOriginalPartNodeTypes`, `0x588ba0` | Se reserva un byte por pieza, se copia el byte bajo de `SceneNode::flags` y se compara para asociar vértices originales. Tipo `BYTE **`. |
| `g_carPartSetCount`, `0x588a90` | Cuenta los coches reservados; limita los recorridos de inicialización y liberación. |
| `g_carDamageModelEnabled`, `0x588970` | Escritor/lector del estado que habilita reconstrucción y deformación de las mallas de daño. |
| `g_carPairValuePending`, `0x588bb4` | El setter `0x46b760` marca o limpia el pendiente; `0x46bb40` lo consulta al seleccionar los valores de la pareja por vista. Se conserva la capacidad y el reset originales. |
| `g_carDamageHazardTimer`, `0x547ce0` | `0x4643f0` acumula el paso temporal y alterna el bit 4 de las luces en los umbrales `0xa0000/0x140000`, cuando hay daño. |
| `g_carLightGlowSlots`, `0x547d00` | `0x463fe0` guarda resultados de `Glow_Add`; `0x4643f0` los habilita y posiciona. Tipo `GlowLight *[160]`, 20 slots por coche. |
| Offsets, nodos y atenuación de luces de vista trasera | `0x463fe0` crea ambos nodos; `0x464960` transforma sus offsets con `SceneNode::current`, actualiza sus posiciones y memoriza la atenuación; `0x457ed0` los destruye. Se nombran las globals `0x547fa0/0x547fe0/0x547fec/0x547ff0/0x547ff4`. |

## Consumidores convertidos

- Registro/búsqueda de piezas y todas las declaraciones del getter compartido.
- Montaje ordenado, cierre de huecos, cajas, escalas, daños y snapshots guardados.
- Construcción, asociación y liberación de copias de vértices originales.
- Materiales del modelo, reinicio de nodos, alturas de ruedas y ocultación en sombras.
- Parpadeo, asignación del vértice más cercano y posición de los glows.
- Selección de triángulo y sus tres bordes con `MeshTriangle::vertexIndex`;
  vértices con stride de `CarPartFloatVertex`; chispas usando `SceneNode::world`.
- Consumidores ya tipados de física, transmisión y efectos: se retiran casts
  redundantes y se mantiene la firma coherente entre unidades.

La rama de montaje usa `Car::pBodyNode` (`+0x720`). El root de la escena está
en `+0x71c`; se comprueban ambos offsets. Se conservan la ordenación inversa,
el número de slots y los recorridos/copias originales, incluidos los límites
que aún requieren investigación.

El recorrido de luces sigue sesgado en `vertexCount`: obtener los vértices por
`vertices[i]` introducía otro puntero de inducción y empeoraba el matching.
La relación entre los arrays se expresa con tipos y `offsetof`, sin el índice
mágico `pObj[-0xea]`. Se conserva la carga de vértices y el recorrido original.

## Validación

La reconstrucción completa y la medición mantienen las 2922 funciones exactas
de 3364 y todos los scores `s/fz/x`, incluidos los de funciones aproximadas.
El informe de bytes es idéntico a la referencia anterior; su SHA-256 es
`a2e517a3d8e88d04eda10a863133bdf2447dc17a0f8a7c4d3f3b310f20fc1aa3`.
No hay problemas de comparación de datos globales. El gate de las unidades
afectadas y los 108 harnesses diferenciales pasan sin fallos.

`differential_car_part_records.py` añade 494 casos con modelos independientes,
heap completo envenenado/guardado, orden de proveedores y ABI. Ejecuta los
cuerpos completos de registro, búsqueda, parpadeo, muestreo de bordes y montaje;
en este último también ejecuta el registro de nodos y las copias/compactación.
Cubre índices extremos de slots, cuentas negativas/cero, dwords con signo,
bordes de umbral y overflow, fases no canónicas, sorteos extremos y snapshots
con/sin impactos. Los proveedores de recursos y efectos son hojas controladas.

Los resultados de la suite y de las unidades afectadas se adjuntan en los
archivos de [medición](car-part-consumers-matching.json) y
[pruebas](car-part-consumers-tests.txt).

## Inventario y siguiente bloque

El inventario sintáctico pasa de 1987 a 1964 desreferencias crudas y de 1389 a
1377 identificadores desconocidos distintos. Los campos sin nombre semántico
pasan de 1512 a 1510 y los tamaños literales de 99 a 95. La cuenta de tipos pasa
de 298 a 297 porque se elimina la vista parcial `CarPartNode` a favor del
`SceneNode` ya recuperado. Estos contadores no incluyen todos los índices
literales de arrays de enteros que han sido sustituidos por miembros.

Queda un bloque coherente de deformación: `0x467700/0x467e90` aún caminan por
slots de `CarPartSet` como enteros; el clamp `0x508740` también recibe registros
de preview `0x82d220`. Comparten arrays iniciales de meshes/nodos/vértices, pero
sus cuentas tienen anchos y offsets distintos (`int` frente a `WORD/BYTE`).
Hay que recuperar esa interfaz común y el registro de preview de `0x2ac`, con
sus lectores y escritores, antes de cambiar las firmas. También quedan los
payloads de impactos de 13 bytes y campos de `PartState`. El objetivo completo
continúa abierto.

Las tandas 12–13 resuelven el prefijo compartido y los registros de preview
descritos arriba. Las fuentes de vertices se llaman ahora `DeformVertex` y
`DeformFloatVertex`, compartidas con el preview, y las tres tablas iniciales
de `CarPartSet` están en `geometry`. Véase
[preview-deform-records.md](preview-deform-records.md); siguen pendientes los
impactos de 13 bytes y los campos restantes de estado.

# Geometría compartida y preview: tandas 12–13

El lote recupera los registros de meshes, escenas y puntos de control del
preview y la interfaz geométrica común con los daños del coche. Sustituye
accesos a enteros y offsets en conversión, asignación, búsqueda, liberación,
variantes de ruedas, transformaciones y deformación. Nombra 23 globals y
elimina vistas duplicadas; añade 57 comprobaciones de layout y capacidad.

## Layouts y evidencia

| Registro | Tamaño | Evidencia de lectores y escritores |
| --- | ---: | --- |
| `DeformMeshSources` | `0xb4` | `0x466e90/0x507290` registran las tres tablas de 15 meshes, nodos y fuentes en `+0/+0x3c/+0x78`. `0x508740` consume solo este prefijo. Ambos registros contienen ahora `geometry` en `+0`. |
| `DeformVertex` / `DeformFloatVertex` | `0x20` / `0x30` | Conversores `0x46afe0/0x506bb0`, reconstrucciones `0x4698a0/0x5074d0` y clamp `0x508740`. Se conservan los seis componentes, bytes con signo y bytes de relleno de las vistas de coche anteriores. |
| `OptionPreviewMeshRecord` | `0x2ac` | Registro y conversión `0x507290/0x506bb0`, asociación `0x506fc0` y consumidores de daño `0x507650/0x508890`. Cuenta de vértices `WORD[15]` en `+0x24c`; cuenta de meshes `BYTE` en `+0x26a`. Difieren de los `int` del coche y no se unifican con ellos. |
| `OptionPreviewSceneRecord` | `0x54` | Carga `0x5062d0`, liberación `0x505f10` y selección de ruedas `0x506080`: cuatro nodos de ruedas, tres tablas de cuatro objetos y raíces normal/L/S. `modelClass` se escribe como byte; los otros tres bytes conservan su nombre desconocido. |
| `OptionPreviewDeformGeometry` | `0x138` | Construcción `0x507a10`, animación `0x509dc0`, plano de deformación `0x507fe0` y transformaciones de esquinas. Ocho esquinas en `+4`, doce puntos de control en `+0x64`, cuatro anchors en `+0xf4`; las fases en `+0x124/+0x128` usan grados en 16.16. |

`cornerHeightOffsets[4]` procede de los canales de daño 6..9 y alimenta las
transformaciones de esquinas (`0x509be0/0x5091c0`).
`partDamageFractions[4]` procede de los canales 0..3 y se lee en la animación
del preview (`0x509d30/0x509dc0`); ese lector conserva el cálculo cuyo resultado
descarta el original. Los escritores y lectores justifican los nombres;
los comentarios anteriores que hablaban de colores del cielo se corrigen.
Los nombres históricos de esas funciones se conservan en este lote.

Las 23 globals nombradas comprenden las tablas de geometría/escena, fuentes
convertidas y tipos de nodo, buffers de los modelos normal/L/S, selección de
ruedas, registro de la sesión, umbrales de daño y parámetros del impacto.
Los parámetros de deformación permanecen como globals independientes: unirlos
en una struct cambiaría las hipótesis de aliasing y las recargas de MSVC6.
La lista de direcciones y nombres está en el informe de medición adjunto.

Las tablas de fuentes y tipos de nodo de `0x831198/0x82d1dc` tienen 16 slots.
Estaban declaradas con solo 2: las dos direcciones abarcan 64 bytes hasta la
siguiente global y la liberación recorre 16 entradas. Se corrigen las dos
capacidades sin cambiar el recorrido, las reservas ni la liberación. La
prueba de vida de buffers ya cubre los slots 0, 1, 7, 8 y 15.

## Formas necesarias para el matching

Los recorridos de coche conservan su puntero de inducción de cuatro bytes
sobre `geometry.meshes`. Las tablas paralelas se expresan con miembros de
las vistas tipadas sesgadas y se mantiene la cuenta `int` del registro real.
No se añade un segundo cursor ni se cambia la frecuencia de las lecturas.

En `0x467e90/0x508890`, indexar directamente un `DeformFloatVertex[]` modifica
la planificación de instrucciones. Los macros locales mantienen el cursor
por bytes, calculan el stride con `sizeof(DeformFloatVertex)` y acceden a
`pos` y `normal` por sus nombres. Los macros no se exportan a headers.
El conversor conserva las tres escrituras de `0x81` como `BYTE`; sus posiciones
se derivan de `offsetof(DeformVertex, rawNormal)`. Asignar esa constante al
miembro con signo alteraba el código generado. Las lecturas posteriores siguen
siendo con signo y las pruebas comparan los bytes completos del buffer.

En la selección de variantes, la vista `PREVIEW_SCENE_AT_OFFSET` conserva
el cursor original por bytes con stride `sizeof(OptionPreviewSceneRecord)`.
El cuerpo accede a nodos, meshes y clase por sus nombres. Estos casos quedan
identificados; no se cuentan como eliminación completa de los recorridos por
bytes. El include de `stddef.h` necesario para `offsetof` también se verifica
con toda la unidad: introducirlo cambia decisiones de MSVC6 en otras funciones.

## Validación

La reconstrucción completa y la medición conservan las 2922 funciones exactas
de 3364 y todos los scores `s/fz/x`. El informe de bytes es idéntico a la
referencia anterior (SHA-256
`a2e517a3d8e88d04eda10a863133bdf2447dc17a0f8a7c4d3f3b310f20fc1aa3`).
No hay problemas de datos globales y el gate de las 18 unidades afectadas pasa.
Los 109 harnesses diferenciales pasan sin fallos sobre el mismo build medido.
La medición, la comparación por unidades y la suite se registran en
[medición](preview-deform-records-matching.json) y
[pruebas](preview-deform-records-tests.txt).

`differential_deform_mesh_records.py` añade 954 casos: 494 de clamp con modelo
independiente y extremos de enteros/bytes con signo; 32 búsquedas por tipo de
nodo; 108 combinaciones de visibilidad; 320 ejecuciones de los cuatro cuerpos
completos de deformación. Compara heap completo envenenado, globals, orden y
contenido de uploads, ancho de escrituras, registros preservados y limpieza
de argumentos stdcall. Los consumidores ejecutan su clamp y helpers reales;
solo las hojas de upload, actualización de mesh y marcado de sombras están
controladas. Para las rutinas completas de deformación se contrasta el
original con el reconstruido y se comprueban guardas/clamp; no se afirma un
modelo matemático independiente de toda su geometría.

Las pruebas existentes de carga y vida de los buffers cubren los conversores,
las tablas de 16 slots, liberación y los cuatro modos de redondeo x87. Los
954 casos nuevos usan el modo x87 original `0x037f`.

## Inventario y siguiente bloque

El inventario pasa de 1964 a 1858 accesos crudos detectados y de 1377 a 1346
identificadores desconocidos distintos. Los campos sin nombre semántico pasan
de 1510 a 1498, los tamaños literales de 95 a 93 y los multiplicadores
hexadecimales de 2294 a 2278. El número de tipos definidos pasa de 297 a 294
al retirar las vistas auxiliares duplicadas y compartir las ya recuperadas.
Los contadores son sintácticos; no equivalen al número total de accesos
convertidos ni demuestran por sí solos el sentido de los nombres.

El siguiente bloque son los impactos de 13 bytes: sus escritores en
`CarDamage_AllocateImpactDeformRecord` (`0x468520`) y lectores en `StageObject_BuildDeformationVectors` (`0x4688b0`)
y `OptionPreview_LoadSkyColourRecord` distinguen fuerza/modo/radio, normal,
axis, posición y vínculo siguiente. Falta recuperar la vista del payload,
los límites de la lista y sus consumidores de buffers de rally. También
quedan las tablas de orientación del preview, campos de `PartState` y los
registros de carga. El inventario completo del proyecto sigue pendiente.

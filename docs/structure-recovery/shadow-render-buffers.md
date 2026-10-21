# Buffers de normales, vértices y referencias de sombras

`ShadowPart` conserva su layout Win32 de `0x58` y su layout nativo de `0x78`.
Las declaraciones identifican ahora sus elementos y los recorridos usan los
campos de esos tipos:

| Offset | Campo/tipo | Evidencia |
| --- | --- | --- |
| `0x38` | `pSourceNormals`, `float (*)[3]` | `Scene_ProjectDirtyShadowParts` (`0x4b4490`) copia `MeshVertexF::nx/ny/nz`. |
| `0x3c` | `pBiasedPositions`, `float (*)[3]` | El mismo productor guarda `position + normal * 0.005`, con el orden de operaciones original. |
| `0x40` | `pLightDotProducts`, `float *` | `Scene_EmitProjectedShadowMesh` (`0x4b4180`) calcula el producto entre cada normal y la dirección de luz transformada al espacio de la malla. |
| `0x44` | `pEmittedPositions`, `float (*)[3]` | Copia las posiciones sesgadas que se emiten; el proyector de zona (`0x4b5770`) las usa al calcular UV. |
| `0x48` | `pVertices`, `MeshVertexF *` | Reserva registros de `sizeof(MeshVertexF)`, inicializa `specularColour`, y después escribe `diffuseColour` y posición mundial. |
| `0x4c` | `pFaceVertices`, `MeshVertexF **` | Guarda tres referencias nativas por cara emitida; los vértices compartidos se obtienen del origen del buffer mediante el remapeo. |

Los cinco nombres nuevos describen los usos observados, sin atribuirlos a
símbolos originales. `MeshVertexF` ya representaba el formato de `0x30` bytes;
el header de sombras solo lo declara por adelantado, sin duplicar el tipo ni
introducir su definición en headers sensibles de otras unidades.

El dato en `+0x40` no es una tabla de flags ni una distancia geométrica al
plano. El emisor rechaza una cara cuando los tres productos normal/luz son
negativos. Para los vértices aceptados, el mismo producto controla la rampa
de alpha bajo `0.1`. Se corrigen los comentarios anteriores que lo confundían
con la posición respecto a un plano.

El productor recorre `MeshVertexF` y arrays de tres floats; el emisor usa campos
de posición/color y el proyector escribe `textureCoordinates[0][0..1]` mediante
un cursor de vértices. Desaparecen el cursor UV sesgado a `+0x24`, los strides
de 12/48 bytes de esos recorridos y los casts de formato redundantes.
Las reservas usan los tamaños de sus elementos. Las copias de posiciones del
emisor siguen transportando sus tres palabras sin conversiones de float.

Los checks existentes conservan todos los offsets de `ShadowPart` y del
vértice. Tres checks adicionales fijan las dimensiones de los buffers de
normales, posiciones sesgadas y posiciones emitidas. El allocator nativo
comprueba las seis reservas por pieza, incluido el tamaño de las referencias
de triángulos con punteros de ocho bytes, y todas sus guardas.

## Validación

La compilación, el gate de las dos unidades afectadas y la medición completa
conservan las 3364 puntuaciones `s/fz/x`, 2924 funciones exactas y cero problemas
de comparación de datos. Los porcentajes globales siguen en 56,28 % de bytes
perfectos y 94,46 % fuzzy.
Los 139 harnesses diferenciales pasan sobre ese mismo build medido.

La prueba diferencial de buffers pasa de 80 a 160 escenarios. Los casos nuevos
ejecutan el productor de caché real antes del emisor/proyector, con normales
distintas de las posiciones y valores fraccionarios. Modelos independientes
comprueban las normales, `position + normal * 0.005`, las caras admitidas y las
referencias compartidas. Una segunda llamada con caché limpia e intensidad
distinta debe conservar todo el heap. Se guardan fuentes, buffers completos,
reservas, lotes y ABI.

El oráculo ejecuta con el control x87 `0x007f`, correspondiente a la precisión
simple del juego. La prueba nativa declara `CMR2_QIFIST`, como el build MSVC6
de `SceneNode.cpp`: los alphas fraccionarios ejercitan el redondeo a par, que
los casos anteriores de alpha entero no distinguían. Los helpers de matrices
se extraen ahora de este checkout de producción.

Los 160 escenarios nativos pasan con ASan/UBSan y comparación exacta contra
el original x86. Se preservan los tres controles negativos anteriores y se
añade otro: reservas de referencias a mitad de tamaño, referencias de cuatro
bytes, origen avanzado al reutilizar un vértice y usar posición en lugar de
normal deben detectarse. Los cuatro fallan como se espera.

## Accesos que quedan pendientes

El emisor aún calcula el desplazamiento entre el cursor del array local de
índices y el de referencias. Las variantes por índices y por cursores paralelos
retroceden desde 72,18 % de bytes y 77,42 % fuzzy; no se incorporan. Se conserva
la corrección de ancho nativo, con sus dos accesos crudos visibles. Tampoco se
ocultan los accesos restantes del loader de iluminación ni los otros subsistemas.

El inventario reproducible pasa de 113 a 109 candidatos crudos y de 33 a 29
tamaños literales de reserva/copia. Los 1401 campos sin nombre semántico siguen
pendientes: estos buffers ya tenían identificadores, aunque tipos insuficientes
y nombres que no identificaban su contenido. El objetivo general sigue abierto.

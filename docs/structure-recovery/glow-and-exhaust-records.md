# Renderer de glows y emisores de escape: tanda 22

Se comparte `GlowLight` entre el renderer y sus consumidores de etapa/coche.
La reserva y los recorridos demuestran un stride de `0x5c`. Se tipan la tabla,
los setters, los argumentos de textura/nodo y los dos quads. La interfaz queda
en `Glow.h`; `CarExhaust.h` declara las tablas independientes de puntos y glows
que antes se describían incorrectamente como texturas de trails.

## Evidencia de tipos y nombres

- `0x4ae170/0x4ae200/0x4ae260/0x4ae2f0` reservan, liberan, reinician y ocupan
  entradas. `g_glowCapacity` y `g_glowAllocatedCount` describen usos distintos:
  el segundo aumenta incluso al añadir una entrada de tipo cero y no se
  reinicia al reservar/liberar. No se interpreta como un recuento recalculado
  de entradas activas.
- `0x4ae3d0` escribe un byte de activación en `+0x50`; `0x4ae3f0` escribe
  intensidad de 32 bits en `+0x3c`. Son `Glow_SetEnabled/Glow_SetIntensity`.
- `0x4ae950` usa `+0x40` como multiplicador de `sizeX` para ambos ejes del
  quad proyectado. `projectedSizeScale` expresa ese uso. El byte `+0x51`
  multiplica el brillo del RGB; cero también desactiva la llamada desde
  `0x4af120`. Se llama `projectedBrightness`, sin reducirlo a un booleano.
- Las cargas de texturas y los argumentos de `Glow_Add` en `0x463fe0` y en
  el montaje de luces de etapa proporcionan `Texture *`. `Billboard_Add`
  (`0x4b11c0`) lee su primer `WORD`; `Texture.textureId` ocupa ese prefijo.
  El quad consume la misma textura como registro completo. Se conserva la
  vista del prefijo para la API de billboards.
- `0x4ae230` escribe los cuatro límites de `BillboardDef`; recibe ese tipo.
  Se preservan sus miembros y signos originales (`top=-x`, `left=y`,
  `bottom=x`, `right=-y`), sin reinterpretar el sistema de ejes.
- `0x463fe0` registra los `CarLightPoint` de tipo 9 como emisores de escape
  y sus `GlowLight`. `0x45af90` y `0x45d2d0` consumen el vector `pos` del
  punto y los setters de glow. Las tablas son `g_carExhaustPoints[8][2]`
  y `g_carExhaustGlows[8][2]`; siguen siendo arrays independientes.
- Los lectores/escritores de `0x543380/0x543cf8/0x5439b0` identifican la
  posición transformada del escape, el lado alternado por el callback y
  los frames restantes de la ráfaga. Se conserva el contador compartido
  de lado y la truncación del contador de frames a ocho bits.

Se añaden 22 comprobaciones de tamaño, offset y capacidad. Los campos
`field_0x52` y `field_0x58` de `GlowLight` siguen sin significado demostrado.
No se deduce la declaración original del fuente solo por recuperar su layout.

## Comportamiento conservado y límites

El reinicio limpia únicamente el tipo de cada entrada y el contador.
`Glow_Add` conserva los campos que no inicializa el original; los parámetros
sin uso siguen siendo parámetros sin uso. La liberación/reserva mantiene el
contador anterior. Las tablas de escape mantienen sus comprobaciones de
índice originales, sin introducir nuevos límites ni cambios de ownership.

Los quads conservan la aritmética fixed-point, el orden de stores, colores,
UV y la cola de triángulos. La división por distancia del layer no recibe
una nueva protección. `Glow_Draw` conserva su puntuación previa; no se
presenta como una función que acaba de alcanzar coincidencia exacta.

Esta tanda no añade un harness independiente de reserva/proyección. La
suite registrada existente incluye el ciclo de glows compartidos con setters
reales y los efectos de escape; controla sus proveedores externos. La
reserva y los quads se validan aquí por la auditoría completa de bytes.
La futura ampliación de cobertura de esas rutas sigue pendiente.

La medición, el gate y la suite del punto estable se guardan junto al informe
JSON de esta tanda. El inventario queda en 1663 accesos crudos y 1285
identificadores desconocidos distintos; ambos son candidatos sintácticos,
no pruebas de recuperación semántica. El objetivo completo sigue pendiente.

## Validación del punto estable

El build y la medición completos conservan los scores `s/fz/x` de las 3364
funciones respecto al último commit y a la tanda anterior: 2922 byte-exactas,
56.10 % perfect match y 94.50 % fuzzy match, sin problemas de datos globales.
El gate de las 38 unidades afectadas pasa. Los 112 harnesses registrados pasan
sin fallos sobre el mismo build medido, con sus fuentes y EXE/PDB verificados.

Véanse [informe de matching](glow-and-exhaust-records-matching.json),
[gate](glow-and-exhaust-records-gate.txt) y
[suite completa](glow-and-exhaust-records-tests.txt).

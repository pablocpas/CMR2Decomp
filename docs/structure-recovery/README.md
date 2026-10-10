# Recuperación de estructuras y campos

Objetivo del proyecto: reconstruir los tipos, arrays y campos que puedan justificarse
con el ejecutable y sus usos, y expresar sus accesos mediante esos tipos.
El trabajo comprende todo el proyecto, incluidos los registros todavía
representados por buffers y los campos con nombres basados en direcciones.

Los nombres recuperados describen la función observada; no se presentan como
los identificadores del código fuente original. Un nombre provisional o un
campo sin semántica demostrada sigue siendo trabajo pendiente. También hay
que distinguir los bytes de relleno de los campos todavía desconocidos.

## Inventario reproducible

```sh
python3 scripts/audit_readability.py --json docs/structure-recovery/inventory.json
```

El inventario excluye comentarios y literales, conserva archivo, línea,
anotación de función más cercana y hashes del código analizado. Sus resultados
son candidatos sintácticos: un multiplicador hexadecimal puede ser una escala
numérica, un acceso crudo puede corresponder a un formato serializado y la
anotación más cercana no necesariamente pertenece a la declaración señalada.
No se usa la desaparición de `unk` como prueba de recuperación.

Punto de partida después de la primera limpieza:

| Indicador | Cantidad |
| --- | ---: |
| Tipos con definición nombrada | 287 |
| Declaraciones de campos sin nombre semántico detectadas | 1605 |
| Identificadores desconocidos distintos | 1457 |
| Desreferencias con aritmética explícita detectadas | 2237 |
| Tamaños literales en asignaciones/copias detectados | 129 |

El inventario completo se actualiza con cada tanda. Los indicadores iniciales
se conservan aquí para que reducirlos no borre la evidencia del trabajo pendiente.

## Criterios de recuperación

Para cada registro hay que establecer su tamaño, alineación, offsets, tipos,
signedness y cardinalidad de arrays a partir de reservas, inicialización,
copias, lectores y escritores. Los formatos serializados o compartidos por
varios usos requieren evidencia específica de sus variantes.

Una sustitución debe conservar el orden y ancho de los accesos, la aritmética,
los efectos y todas las coincidencias byte-exactas existentes. Los tamaños y
offsets establecidos se verifican en `CMR2Decomp/LayoutChecks.cpp`, evitando
añadir declaraciones de comprobación a cabeceras compartidas sensibles a MSVC6.

Cada tanda se compila y compara por archivo y después con el ejecutable entero.
Se ejecuta la suite diferencial registrada sobre el mismo build
medido. Una pérdida de exactitud obliga a ajustar o retirar la transformación.
Los recorridos por bytes que MSVC6 exige para reproducir el binario quedan
identificados como deuda concreta; no se ocultan detrás de nombres nuevos.

## Orden de trabajo y evidencia

| Bloque | Estado y siguiente trabajo |
| --- | --- |
| Coche, callbacks y jugadores de sesión | Véanse [auditoría inicial](../audits/2026-10-09/readability.md), [motor, controles y cambio](car-controls.md) [transforms/contactos](transforms-and-contact.md) y [recursos/vínculos](car-resources.md). Snapshots, contactos y recursos tipados; quedan campos del coche, perfiles serializados y registros de vista. |
| Piezas móviles, geometría y daños del coche | Recuperación en curso. Véanse [piezas](car-parts.md) y [líneas/snapshots](flexible-lines-and-damage.md). [Consumidores de piezas](car-part-consumers.md), [geometría de preview](preview-deform-records.md) e [impactos persistentes](impact-and-preview-state.md) tipados. Quedan campos de estado móvil y longitudes serializadas de CIN; véase [perfiles y cockpit](cin-and-cockpit-records.md). |
| Objetos de etapa y nodos de escena | [Clima, iluminación y escenas](weather-and-stage-scenes.md): registros de vista, partículas, presets y nodos de cielo/suelo/nubes tipados; quedan variantes de objetos móviles y otros buffers; [interior y luces](interior-and-light-state.md) recupera sus recursos, pose y pools compartidos. |
| Tiempos, replay y estado de carrera | Distinguir registros persistentes, instantáneas y tablas de ranking antes de asignar tipos. |
| Información de juego y datos del rally | Rastrear opciones empaquetadas, tablas de ruta y registros por jugador. |
| Colisión, sectores y gráficos | Recuperar formatos de malla, árbol de colisión, nodos y vértices con tamaños y variantes verificados. |
| Resto de subsistemas | Recorrer el inventario completo: entrada, red, sonido, frontend, archivos y efectos. |

El objetivo sigue abierto hasta resolver el inventario de layouts y documentar
con evidencia los casos que no permitan una interpretación única.

Después de estas tandas, el inventario detecta 2078 desreferencias crudas y
1514 declaraciones de campos sin nombre semántico. Los recursos del coche
incluyen un registro de escena de 0x24 bytes, tablas independientes y vínculos
con nombres comprobados por sus escritores/lectores. La nueva prueba de
liberación añade 128 casos y lleva la suite registrada a 106 harnesses.
Los campos desconocidos y formatos de los buffers conservan su estado
pendiente. La siguiente investigación preparada es la de perfiles CIN,
registros de vista y variantes de los objetos/nodos de etapa.

Las tandas 07–09 recuperan la familia de clima, iluminación y escenas de etapa
con 97 comprobaciones de layouts y capacidad. El inventario actualizado tiene
1987 accesos crudos, 1389 identificadores desconocidos distintos y 1512 campos
sin nombre semántico. Véanse [evidencia y deuda concreta](weather-and-stage-scenes.md).
La suite incorpora una prueba de registros de clima con 3827 casos y tiene
107 harnesses registrados. El objetivo del proyecto completo sigue abierto.

Las tandas 10–11 completan la interfaz tipada del getter y sus consumidores de
montaje, recursos, luces, sombras y chispas en 43 funciones, con 12 globals y
dos campos nombrados y 34 comprobaciones de layout. El inventario queda en
1964 accesos crudos y 1377 identificadores desconocidos distintos.
Véanse [evidencia y siguiente bloque de deformación](car-part-consumers.md).
La nueva prueba de registros aporta 494 casos y la suite completa tiene
108 harnesses, todos aprobados. La auditoría de bytes permanece idéntica.

Las tandas 12–13 recuperan la geometría compartida de coche/preview y los
registros de meshes (`0x2ac`), escenas (`0x54`) y puntos de control (`0x138`).
Nombran 23 globals y añaden 57 comprobaciones de layout. El inventario pasa a
1858 accesos crudos y 1346 identificadores desconocidos distintos. Véanse
[evidencia y límites del tipado](preview-deform-records.md). La medición
completa conserva todas las puntuaciones de 3364 funciones y las 2922 exactas;
la auditoría de bytes es idéntica. Las 109 pruebas registradas pasan sin fallos.
El objetivo de todo el proyecto sigue abierto.

Las tandas 14–15 recuperan las cargas de impactos y el registro persistente
`0x148`, los ángulos con signo y las tablas de referencia/proyección del
preview. Nombran 11 globals y corrigen 16 nombres de funciones; añaden 40
comprobaciones de layout. El inventario queda en 1850 accesos crudos, 1334
identificadores desconocidos distintos y 1495 campos sin semántica.
Véanse [layouts, comportamiento y límites](impact-and-preview-state.md).
Todos los scores de las 3364 funciones y las 2922 exactas se conservan;
los 110 harnesses pasan sin fallos, incluidos 1861 casos nuevos.
Los perfiles CIN, los campos móviles con solo un escritor de inicialización
y el resto del inventario siguen pendientes dentro del objetivo completo.

Las tandas 16–18 recuperan las seis familias CIN observadas y sus caches,
el cockpit y los archivos del coche. Añaden 101 comprobaciones de layout,
nombran 14 globals/vistas y corrigen nueve nombres de funciones. El inventario
queda en 1781 accesos crudos, 1319 identificadores desconocidos distintos y
1496 campos sin semántica. Los tipos de cola variable y prefijo conocido no
establecen tamaños completos de archivo. Véanse [evidencia y límites](cin-and-cockpit-records.md).
Todos los scores de las 3364 funciones y las 2922 exactas se conservan; los
111 harnesses pasan sin fallos, incluidos 17323 casos nuevos. Los recursos
restantes, registros de luces, estado móvil y el resto del proyecto siguen
pendientes dentro del objetivo completo.

Las tandas 19–21 recuperan los recursos y texturas del cockpit, pose del
conductor, mezcla de dirección, niveles de luces y pool compartido de glows.
Convierten 41 cuerpos de funciones en conjunto, recuperan 27 nombres de
globals/vistas y corrigen nueve nombres de funciones. Añaden 60 comprobaciones
de layout y capacidad. El inventario queda en 1677 accesos crudos, 1292
identificadores desconocidos distintos y 1498 campos sin semántica. Véanse
[evidencia, comportamiento y deuda](interior-and-light-state.md). Los 112
harnesses pasan sin fallos, incluidos 5691 casos nuevos. Todos los scores de
3364 funciones y las 2922 exactas se conservan, sin problemas de datos.
La siguiente familia preparada es `GlowLight`, su registry y los quads;
el resto del inventario mantiene el objetivo completo abierto.


La tanda 22 comparte el registro del renderer `GlowLight`, su interfaz y
los quads de layer/proyección. Tipa las tablas de emisores/glows de escape,
recupera nueve nombres de globals (siete antes desconocidos) y corrige cuatro
nombres de funciones. Añade 22 comprobaciones de layout/capacidad. El inventario
queda en 1663 accesos crudos, 1285 identificadores desconocidos distintos y
1497 campos sin semántica. Véanse [evidencia y límites](glow-and-exhaust-records.md).
El punto estable conserva todas las puntuaciones de las 3364 funciones respecto
al último commit: 2922 exactas y cero problemas de datos; las 38 unidades del
gate y los 112 harnesses pasan. No añade un nuevo harness de proyección/reserva;
la cobertura independiente de esas rutas y el resto del objetivo siguen pendientes.

La preparación de 64 bits parte del commit estable `c9c8b9c`. Su primera
tanda convierte las cuatro respuestas de contacto del coche y 600 checks
existentes, añadiendo 25 checks de offsets y una macro común compatible con
MSVC6. Elimina 188 avisos de casts nativos de esos cuerpos y 159 accesos
crudos; el inventario queda en 1504. Véanse [evidencia, disco y deuda
pendiente](64bit-car-contacts-and-layouts.md). Se conservan las 2922 funciones
byte-exact y todas las puntuaciones de las 3364 filas; los 113 harnesses
pasan, incluidos 4446 casos nuevos. Cuatro cuerpos se ejecutan además con
punteros de ocho bytes por encima de 4 GB. El port completo sigue pendiente.

La segunda tanda para 64 bits tipa las diez texturas de opciones y su array
de doce piezas junto con todos sus consumidores. Corrige además cuatro
getters que devolvían ids/flags numéricos como `void *`. Elimina 76 casts
de direcciones/números y once accesos crudos; el inventario queda en 1493.
Véanse [evidencia, cobertura y API pendiente](64bit-option-textures-and-indices.md).
Los 114 harnesses pasan, incluidos 10640 casos nuevos, y la auditoría de
3364 funciones continúa idéntica con 2922 exactas. Cinco cuerpos actuales
se prueban también en x64 con punteros no nulos mayores que 4 GB.

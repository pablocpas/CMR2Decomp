# Recursos del interior y estado de luces: tandas 19–21

El lote recupera una familia completa de carga, reinicio, actualización y
liberación: recursos del interior, nodos, texturas del cuadro, pose del
conductor y niveles de luces. Comparte el registro del pool de 100 glows
entre creación, spawn, movimiento, colisión e interpolación. Cambia 41 cuerpos
de funciones en conjunto, recupera 27 identificadores de globals/vistas y
corrige nueve nombres de funciones. Añade 60 comprobaciones de layout y
capacidad. Se conservan las anotaciones y su orden.

## Layouts y evidencia

| Registro o array | Disposición | Evidencia |
| --- | --- | --- |
| `CarDriverPoseState` | Dos filas de `0x24`: fase de 12 bits, rotación, desplazamiento filtrado y vector de montaje | `0x476e00` integra rotación/desplazamiento; `0x4778b0` usa el vector de montaje; `0x475f80` reinicia sus componentes. La parte en `+0x18` deja de ser un tail desconocido porque se encuentra su lector como `FixVector`. |
| `CarInteriorDashTextures` | Dos pares `revCounter/digit`, `0x08` cada par | `0x477340` selecciona por los nombres de textura `REVCT/DIGIT`; `0x477460/0x4775f0` actualizan y `0x4779e0` limpia ambos punteros. |
| Arrays de niveles del cuadro | Dos filas de 12 `unsigned short` para revs; dos de siete `WORD` para el dígito, con copia solicitada/aplicada | Los escritores, comparadores y remapeos demuestran la forma y el ancho. Se mantienen como arrays; agruparlos en un struct no añade información. |
| Archivos y buffers del interior | 16 `GenericFile`, dos buffers de modelo y ocho raíces de escena | Carga `0x4760a0`, reinicio `0x475f80` y liberación `0x4779e0`. Los buffers C3D siguen siendo `void *`: se demuestra su propiedad y uso, no su formato serializado completo. |
| Nodos de detalle/default | Dos arrays de ocho `SceneNode *` | Montaje y copia de payload eligen el nodo según el detalle; la actualización de máscaras de vista los consume como nodos. Se conserva la capacidad original. |
| Mezcla de dirección | Tres arrays separados de dos elementos: marcha previa `BYTE`, modo `int` y frame `BYTE` | `0x476850` responde a marcha solicitada/freno de mano; `0x476640` usa su fracción para interpolar el nodo de dirección. El reinicio escribe dos bytes de marcha y dos de frame, sin ampliar esos stores. |
| `CarLightTextureState` | Ocho registros `0x48`: dos texturas; niveles, copia aplicada y niveles filtrados como tres arrays `[2][5]`; dos máscaras y un modo de combinación | `0x477ac0/0x477b60/0x477c20/0x477c80/0x477ce0/0x477f30` y consumidores de point glows. Los dos conjuntos no reciben una interpretación espacial no demostrada. |
| `CarHeadlightGlowRecord` | 100 registros `0x5c`: velocidad, posición/normal, copias previas, fade, altura, vida, triángulo, glow, actividad y coche | `0x47d510/0x47d5a0/0x47d850/0x47dd70/0x47e1e0`; las antiguas vistas interiores desde `+0x0c/+0x18` comparten la misma memoria. El puntero de glow se tipa como `GlowLight *`, distinto del registro del pool. |

El estado de luces explica por qué los nombres históricos de «damage record»
y «lane shorts» resultaban engañosos: se reinician niveles, se cambian máscaras
por canal y se devuelven niveles a los lectores de luces. Los nuevos nombres
siguen esa evidencia. El display de marcha incluye el glyph de reverse y el
segmento central de la rama por defecto; no se reinterpreta como un generador
matemático de dígitos decimales.

Los bytes de `CarDriverPoseState.field_0xa`, de `CarHeadlightGlowRecord.field_0x4e`
y de `field_0x59` siguen sin semántica demostrada. Su conservación no justifica
inventar nombres. La estructura del registro no demuestra la declaración
original del fuente. El almacenamiento del bloque de etapa permanece contiguo
para conservar sus alias y recorridos.

## Comportamiento y deuda concreta

La animación conserva la diferencia de signedness entre los dos conjuntos:
el primero guarda el valor como `short`; el segundo lo trunca a `BYTE` antes
de guardarlo como `short`. La combinación de los primeros canales y las
divisiones truncadas permanecen iguales. La copia aplicada solo cambia al
repaint; se actualiza incluso si las dos texturas faltan.

La mezcla de dirección conserva sus cuatro comprobaciones consecutivas:
puede avanzar por varias fases en una misma llamada. La marcha solicitada
es `char` con signo y la copia previa es `BYTE`; no se homogeneizan esos tipos.
Los frames hacen wrap de ocho bits y los límites de 7/3 permanecen estrictos.

Liberar el interior conserva las raíces, los nodos de reposo/mezcla y los
flags que el original deja intactos. Solo los cinco últimos campos de una
fila de nodos se limpian cuando su raíz estaba presente. Los buffers y los
16 registros de archivo se liberan y limpian en el orden original. No se
convierte esta recuperación en un cambio de ownership.

El inicializador sigue necesitando cinco cursores sesgados en MSVC6. Sus
accesos ahora se expresan con macros locales de miembros, `offsetof` y pasos
`sizeof`; conserva el límite de la fila de nodos, que cae en una dirección
del scratch vecino sin escribir en ella. La creación, interpolación y
colisión de glows conservan también sus cursores originales con vistas
tipadas. Es deuda visible, aunque ya no exige interpretar números para
identificar cada campo. Los modelos C3D siguen siendo buffers opacos.

## Validación

El build y la medición completos conservan las 2922 funciones byte-exactas de
3364 y todos los scores `s/fz/x` por dirección, sin problemas de datos globales.
El gate de las 38 unidades afectadas conserva sus puntuaciones. Cambian los
nombres del informe JSON, por lo que no se afirma que su hash permanezca igual.

La nueva prueba ejecuta 5691 casos contra modelos independientes: 768 de
animación, ocho resets, una invalidación completa, 576 de máscaras, 50 de
getters con salidas nulas/aliased, 3360 de mezcla de dirección, 112 de glyphs,
624 de umbrales/repaint del cuadro y 192 de liberación. Ejecuta los cuerpos
originales y reconstruidos; compara registros y bloque completos, heap
inicialmente envenenado, guards, anchos, trazas de argumentos/orden, ABI y
registros preservados. Las hojas de render/destrucción/liberación están
controladas. Los casos nuevos usan x87 `0x037f`.

La suite anterior del pool compartido ejecuta su ciclo completo de creación,
spawn, movimiento e interpolación con setters reales; controla solo sus
proveedores externos. La rutina de colisión conserva su auditoría de bytes,
sin atribuirle casos nuevos de comportamiento que no se han añadido aquí.

Véanse [medición y nombres](interior-and-light-state-matching.json) y
[prueba enfocada](interior-and-light-state-focused-tests.txt). Los 112 harnesses
pasan sin fallos sobre el mismo build medido; véase [suite completa](interior-and-light-state-tests.txt).

## Inventario y siguiente familia

Los accesos crudos pasan de 1781 a 1677 y los identificadores desconocidos
distintos de 1319 a 1292. Los tipos pasan de 309 a 311 al compartir dos layouts
existentes y añadir dos más. Los tamaños literales pasan de 90 a 84 y los
multiplicadores hexadecimales de 2258 a 2249. Los campos sin semántica pasan de
1496 a 1498 al expresar bytes antes llamados `pad` sin adjudicarles una función.
Estos contadores son candidatos sintácticos, no pruebas de layouts ni de nombres.

La siguiente familia conectada es el registro del renderer `GlowLight`, su
reserva/liberación, setters y quads de layer/proyección. El objetivo sigue
abierto en todo el proyecto, incluidos los formatos serializados, registros
móviles, carrera/rally, frontend y otros subsistemas.

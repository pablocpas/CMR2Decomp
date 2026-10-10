# Impactos persistentes y orientación del preview: tandas 14–15

El lote tipa los impactos empaquetados y el estado de daño persistente que
comparten el coche y el preview. Sustituye las vistas de bytes en escritores,
lectores, listas enlazadas, copia de snapshots y selección de daños. Recupera
además los registros de orientación, puntos de referencia por modelo y quads
proyectados. Nombra 11 globals y corrige 16 nombres de funciones respaldados por
sus consumidores; añade 40 comprobaciones de layout y capacidad.

## Layouts y evidencia

| Registro | Tamaño | Evidencia |
| --- | ---: | --- |
| `CarImpactPayload` | `0x0c` | Escritor `0x468520`, lectores `0x4688b0/0x507710`: fuerza, modo y radio como bytes sin signo; normal, eje y posición como tres componentes con signo cada uno. |
| `CarDamageLink` / `CarDamageLinkPool` | `0x0d` / `0x106` | Escritor y replay `0x469690/0x507650`: 20 entradas, cuenta en `+0x104`, cabeza en `+0x105`, siguiente con signo en `+0x0c` y terminador `-1`. Las declaraciones existentes se comparten desde `CarImpact.h`. |
| `RallyCarDamageState` | `0x148` | Carga `0x4669f0`, exportación `0x469b50` y consumidores del preview: pool en `+0`, snapshot en `+0x108`; los dos bytes intermedios no se copian en la exportación. Se mantiene separado del registro activo `CarDamageRecord` de `0x290`. |
| `PreviewSignedAngles` | `0x08` | Setter `0x506930`, animación `0x506720` y rotación de la raíz: tres `short` con signo, último word copiado pero sin significado demostrado. No se cambia el tipo sin signo del formato general `FixAngles`. |
| `PreviewModelReferenceGeometry` | `0x240` | Proyección `0x50f120`: 14 modelos, 12 partes por modelo y 4 `FixVector` por parte. Los 672 vectores mantienen sus valores originales, incluido el componente implícito cero del último inicializador. |
| `PreviewScreenPoint` / `PreviewProjectedQuad` | `0x08` / `0x20` | Proyección y consumidores del menú: dos coordenadas `int`, cuatro puntos por quad y 12 quads. La API externa sigue recibiendo la dirección de la primera coordenada. |

El radio del impacto solo se escribe y lee cuando el modo es 1. Los otros
modos conservan el byte previo; el scratch del escritor tampoco inicializa
ese byte. El modo de coche es un `int` que el decoder escribe solo como byte;
el modo del preview es una global `BYTE`. Se conservan esos anchos y los bytes
vecinos. El escritor compara el modo completo con 1 antes de empaquetarlo.
Las componentes vectoriales usan cuantización con signo, sin convertir el
valor `-128` a un valor saturado durante la lectura. La normalización calcula
primero el recíproco truncado y después multiplica cada componente; sustituirlo
por divisiones exactas altera el redondeo.

Los canales de daño 0..3 alimentan las fracciones de las piezas, 6..9 las
alturas de esquinas y 26..33 la selección de materiales. Los campos y sus
nombres siguen esos lectores; se retiran nombres históricos que describían
colores del cielo, tiempos de carrera o rotaciones exclusivas de ruedas.
El informe adjunto conserva el mapa completo de nombres anteriores y nuevos.

Las tablas de ángulos tienen cuatro slots. La tabla de tiempos conserva sus
20 entradas declaradas, aunque esta familia solo consulta cuatro; no se reduce
una capacidad por observar únicamente estos lectores. Se mantienen las globals
independientes y sus anotaciones de dirección.

## Comportamiento y formas necesarias para el matching

La elección del impacto más débil conserva la primera entrada ante empates,
su comparación estricta con la fuerza nueva y el movimiento de la entrada al
final de la lista. La capacidad se expresa con `sizeof` de la tabla. La carga
inicial conserva el cursor sesgado entre las dos tablas paralelas de impactos;
las vistas usan los miembros del registro real, sin añadir otro cursor.
El scratch del escritor sigue siendo el buffer de 20 bytes de la implementación
anterior, interpretado mediante el registro tipado.

Los ángulos se leen con signo. El setter convierte la diferencia a grados
16.16; si supera 180 grados en valor absoluto, refleja una diferencia positiva
con `360 - delta` o suma 360 a una negativa, una sola vez. No es una
normalización general del ángulo. El cuarto word del input no se consume y
el cuarto word previo del destino sí se propaga en las copias de 8 bytes.
La animación conserva el cálculo temporal sin signo, el fin estrictamente
posterior al tiempo límite y los truncamientos de la interpolación.

`field_0x6` sigue desconocido: la evidencia demuestra su conservación, no una
semántica nueva. Los layouts recuperados no convierten en resueltas las otras
variantes de registros persistentes, formatos CIN ni campos del coche.

## Validación

El build completo conserva las 2922 funciones byte-exactas de 3364 y todos
los scores `s/fz/x` por dirección, con cero problemas de datos globales.
Los nombres de símbolos del informe cambian por los renombrados, por lo que
no se afirma identidad del fichero JSON de auditoría. La suite completa tiene
110 harnesses y pasa sobre el mismo build medido. Véanse
[medición y mapas de nombres](impact-and-preview-state-matching.json),
[prueba enfocada](impact-and-preview-state-focused-tests.txt) y
[suite completa](impact-and-preview-state-tests.txt).

`differential_impact_and_preview_records.py` añade 1861 casos con modelos
independientes: 692 decodificaciones de ambos destinos, 288 escrituras de
pools, 3 exportaciones, 864 ciclos de orientación y 14 recorridos completos de
modelos de referencia. Ejecuta los cuerpos originales y reconstruidos;
compara heap envenenado completo, regiones globales, bytes preservados,
ancho de escrituras, orden de proveedores, registros preservados y stdcall.
Los helpers del encoder/decoder son reales. Las hojas de render, menú,
búsqueda de registro y proyección están controladas; no se afirma verificar
la implementación matemática de esas hojas mediante esta prueba. El fixture
de los modelos y presets se toma del original y se compara con el reconstruido.
Estos casos usan x87 `0x037f`; las pruebas anteriores de conversión cubren
los cuatro modos de redondeo.

## Inventario y siguiente bloque

El inventario sintáctico pasa de 1858 a 1850 accesos crudos, de 1346 a 1334
identificadores desconocidos distintos y de 1498 a 1495 declaraciones de
campos sin semántica. Los tamaños literales pasan de 93 a 91 y los
multiplicadores hexadecimales de 2278 a 2274. Los tipos definidos pasan de
294 a 299 al retirar la vista local de ángulos y añadir seis registros.
Estos contadores no miden el número total de accesos convertidos.

El objetivo sigue abierto en todo el repositorio. El siguiente bloque son
los campos del estado de piezas móviles, los perfiles CIN y los registros
compartidos de vista/carga; la prioridad sigue siendo recuperar familias de
lectores y escritores en lotes manteniendo la exactitud.

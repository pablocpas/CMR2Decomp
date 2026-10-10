# Perfiles CIN y cockpit: tandas 16–18

El lote recupera las seis familias de secciones CIN observadas y sus caches:
luces, contacto/skid, vidrio, líneas flexibles, posiciones de cámara e interior.
Los buffers siguen siendo memoria cargada de archivos; las vistas tipadas
expresan la disposición demostrada por sus lectores y escritores, sin afirmar
que el fuente original declarara estas mismas estructuras. Se añaden 101
comprobaciones de tamaño, offset, ancho y capacidad.

## Layouts y límites de la evidencia

| Vista | Tamaño o parte conocida | Evidencia |
| --- | --- | --- |
| `CarInfoDirectory` | Entradas `unsigned short`, longitud variable | `0x457e00/0x457e10`: offsets relativos al archivo; seis índices observados, sin establecer el tamaño total del directorio. |
| `CarLightPoint` / `CarLightProfile` | Punto `0x28`; encabezado de 4 bytes y cola variable | `0x463fe0`: cuenta y pasos entre puntos; parte con signo, selección de objeto/vértice y lectores de luces. |
| `CarSkidProfilePoint` / `CarContactProfile` | Punto `0x08`; encabezado `0x0c` y cola variable | `0x494bb0/0x496e00`: cantidad y rango visible, desplazamiento longitudinal, longitud de huella y pares distancia/anchura. |
| `CarGlassProfile` | Prefijo consumido `0x54` | `CarEffects`: tres grupos de cuatro bytes y seis vértices elegidos por los descriptores de ventanas. No prueba el tamaño completo de la sección. |
| `CarFlexibleLineDescriptor` / `CarFlexibleLineProfile` | Descriptor `0x20`; encabezado `0x10` y cola variable | `0x4809e0` y `CarPart_UpdateFlexibleLines`: eje de restricción, cantidad y descriptores de posición/eje/longitud/color. |
| Sección de cámara | Array de `FixVector`, extensión total desconocida | `0x486700/0x4869e0`: cache por coche y elección de offset por modo de cámara; se comparten los vectores ya establecidos. |
| `CarInteriorProfile` | Vista `0x30`, última lectura hasta `0x2e` | `0x476540`, montaje, dirección, pose del conductor y limpiaparabrisas: rotaciones, límites con signo, posición y yaw de reposo. La alineación final no demuestra el tamaño del archivo. |
| `CarInteriorProfilePointers` / `CarInteriorNodes` | `0x1c` cada uno | Siete punteros cacheados y siete nodos: lectores, tipos de nodo, creación, reparentado y liberación. |
| `CarWiperState` | `0x0c` | `0x476c70`: modo y dirección como `int`, ángulo/paso como `short`, dos slots. |
| `CarInteriorRuntimeTables` | `0x90` | Región contigua del bloque de etapa: ocho clases, dos estados, dos filas de perfiles y dos filas de nodos. Se conserva el almacenamiento original y sus alias vecinos. |
| `StageArchiveTables` | `0x188` | Reserva, carga, reinicio y liberación: 32 `GenericFile` de `0x0c` y dos contadores. Se reutiliza el tipo real en lugar de la antigua vista de tres punteros. |

Las colas `[1]` son vistas de longitud variable, no capacidades de una sola
entrada. `field_0x3`, `field_0xa` y `field_0x1a` siguen sin semántica demostrada.
El cuarto byte de los tintes no recibe un significado nuevo. Los nombres
recuperados describen usos observados, no identificadores originales.

La fila de nodos distingue el reposo, la mezcla y la referencia de dirección
por `0x476640`. El nodo de referencia copia `current` y `flags` de la mezcla;
la bandera copiada está en `+0x30`, no en `useParentWorld`. La liberación
conserva los punteros que el original deja intactos, incluidos los de reposo
y mezcla. Las capacidades declaradas de las tablas de vista son 16 índices
de coche y 64 niveles; no se reducen por ver solo algunos consumidores.

## Comportamiento y deuda de matching

El resolvedor conserva las dos llamadas al getter: lee el offset de la primera
base y lo suma a la segunda. Las secciones negativas usan la primera entrada.
El índice del coche sigue teniendo signo y el offset del archivo no lo tiene.

Los limpiaparabrisas conservan los truncamientos a `short`, los límites
estrictos y las dos comprobaciones consecutivas de intensidad. Un modo apagado
puede activar ambas comprobaciones y llamar dos veces a `rand`. El modo 1
termina el barrido; los modos 2 y 3 mantienen sus pasos lentos/rápidos.
Las direcciones de los modelos 7, 8, 9 y 13 se obtienen del cache observado.

La cámara conserva la multiplicación fixed-point real, la suma de la posición
de referencia y la selección especial 0→3 / 2→4 condicionada por los tres
getters de modo. No se sustituye la aritmética por una fórmula en coma flotante.

Quedan formas concretas requeridas por MSVC6 para mantener sus instrucciones:
los inicializadores de contacto y líneas conservan cursores de bytes, pero sus
vistas y pasos usan miembros, `offsetof` y `sizeof`; el reinicio de archivos
mantiene un cursor sesgado al segundo campo; el wrapper de limpiaparabrisas
mantiene el índice en bytes mediante macros locales de vistas tipadas. La
cámara reutiliza una variable `int` para la dirección y después para el nivel.
El bloque de etapa sigue siendo almacenamiento contiguo de bytes porque otros
recorridos cruzan sus tablas. Estas formas siguen siendo deuda visible.

## Validación

El build y la medición completos conservan las 2922 funciones byte-exactas de
3364, sin cambios de `s/fz/x` por dirección ni problemas de datos globales.
Los renombrados cambian los nombres del JSON, no sus resultados por dirección.

La nueva prueba enfocada aporta 17323 casos: 288 del directorio y su doble
recarga, 15 de líneas, 16 de cache interior, 6 de cache de cámara, 10800 de
barrido, 192 de cámara, 6 de archivos y 6000 del wrapper de limpiaparabrisas.
Ejecuta los cuerpos originales y reconstruidos contra modelos independientes,
heap envenenado completo, regiones globales, bytes preservados, anchos de
escritura, orden de proveedores y ABI. La cámara usa los helpers reales de
matrices y comprueba también el contador global de multiplicaciones.
Las hojas geométricas del wrapper están controladas: se comprueban argumentos,
alias de destino, traducción y resultado de scratch, sin afirmar validar su
implementación matemática. El parámetro angular de esas hojas se consume en
sus 12 bits bajos; no se atribuye significado a los bits superiores de un
slot cuyo argumento original es estrecho. Los casos nuevos usan x87 `0x037f`.

Los 111 harnesses pasan sin fallos sobre el mismo build medido. El gate de
22 unidades conserva sus puntuaciones. Véanse [medición y nombres](cin-and-cockpit-records-matching.json),
[prueba enfocada](cin-and-cockpit-records-focused-tests.txt) y
[suite completa](cin-and-cockpit-records-tests.txt).

## Inventario y alcance pendiente

El inventario pasa de 1850 a 1781 accesos crudos y de 1334 a 1319 identificadores
desconocidos distintos. Los tipos pasan de 299 a 309, los tamaños literales de
91 a 90 y los multiplicadores hexadecimales de 2274 a 2258. Los campos sin
semántica pasan de 1495 a 1496: retirar la vista incorrecta de archivos y
expresar tres campos CIN todavía desconocidos no justifica inventar nombres
para disminuir el contador.

Sigue pendiente el objetivo completo del repositorio: otros registros de
recursos/interior, estado móvil, vistas, carrera/rally, frontend y los demás
subsistemas. La extensión serializada total de las secciones CIN y los bytes
sin consumidores suficientes siguen documentados como incertidumbre.

# Preparación de 64 bits: contactos del coche y comprobaciones de layout

Primera tanda de `CODEX-64BIT.md`, sobre el punto estable `c9c8b9c`.
Se convierte una familia completa de respuestas de colisión a parámetros
`Car *` y miembros existentes, conservando nombres de locales, orden de
expresiones, signedness y las regiones cuya semántica sigue desconocida.
No se modifica el tamaño de ningún registro serializado.

## Alcance y evidencia

| Función original | Cambio | Avisos de conversión a punteros nativos, antes → después |
| --- | --- | ---: |
| `0x489750` `Collision_CarVsBox` | Argumento `Car *`, matriz y estado por miembros | 3 → 0 |
| `0x48a5f0` `Collision_SeparateCarBoxes` | Dos argumentos `Car *`; centros obtenidos mediante `CollisionBox::pVertex` | 27 → 0 |
| `0x48ae90` `Collision_ResolveCarContactImpulse` | Dos argumentos `Car *`; extents, masa, matrices, velocidades y correcciones por miembros | 95 → 0 |
| `0x48be20` `Collision_ResolveStaticObstacleContact` | `Car *`; vista `StageObject **` del prefijo de la entrada y flags tras el miembro de malla | 63 → 0 |

Los escritores/lectores de `Car.h`, el montaje de los scratch boxes en
`0x48a1f0` y los callers de `Collision2D.cpp`/`StageObjects.cpp` proporcionan
los tipos. Se actualizan todas las declaraciones repetidas y todos los callers.
`0x471dd0` construye las entradas de objetos estáticos guardando en su primer
slot el `StageObject *`; el segundo slot guarda el box. Esta tanda tipa únicamente
la vista del primer puntero, sin convertir todavía el constructor de esa tabla.

El recuento se obtiene compilando los cuerpos actuales y los del commit base
con Clang para x64. Son **188 avisos eliminados de estos cuerpos**, no un
recuento actualizado de todo OpenCMR2. La lista externa es anterior a las
últimas tandas; sus cifras por función no coinciden necesariamente con el
fuente de este commit. El informe guarda el compilador, hashes y diagnósticos.

## Offsets originales comprobados

| Registro / offset Win32 | Tipo o región | Evidencia en esta familia |
| --- | --- | --- |
| `Car +0x204` | `FixVector halfExtents` | Clamp por componentes |
| `Car +0x270` | `FixVector corners[8]` | Índices de esquina publicados por overlap/deformación |
| `Car +0x2d0 / +0x2e8` | Posición actual / anterior | Promedio y llamada de overlap |
| `Car +0x360 / +0x36c / +0x378` | Ejes `right/up/forward` | Componentes verticales del desplazamiento de separación |
| `Car +0x408 / +0x420` | Velocidad lineal / angular | Velocidad del contacto y producto vectorial |
| `Car +0x5c4 / +0x5d0 / +0x5dc` | Tres `FixVector` de nombre provisional | Correcciones acumuladas y offset de contacto; nombres existentes conservados |
| `Car +0x750 / +0x75c` | `FixMatrix *pWorld` / masa | Transformación del impulso y promedio ponderado |
| `Car +0x96c` | `int field_0x96c` | Escritura de `0x10000` al separar; sin reinterpretación adicional |
| `Car +0xad6 / +0xae0` | Vista `short *` de `field_0xad6` | Lista previa / lista actual de cinco ids de contacto |
| `Car +0xb35 / +0xb3e / +0xb3f` | Vista `char *` de `field_0xb35` | Estado y contadores con signo; no se convierten en booleanos/BYTE |
| `Car +0xb64 / +0xb70` | Campos `int` de nombre provisional | Condiciones de separación y emisión de debris |
| `Car +0xc00 / +0xc04` | Selección de corners superiores / `field_0xc04[0]` | Flags escritos por la respuesta estática |
| `CollisionBox +0x90 / +0x94` | `int *pArray / *pVertex` | Arrays de ocho vectores y centro; los miembros siguen el ancho nativo |
| `StageObject +0x0c / +0x10 / +0x98`, tamaño `0xa0` | Malla, prefijo de flags y siguiente objeto | Loader de etapa y primer slot de la tabla de contactos |

Se añaden **21 comprobaciones de offsets de Car y cuatro de StageObject** en
`LayoutChecks.cpp`. Los tamaños y offsets existentes siguen comprobándose en
MSVC6. No se añaden campos ni se inventan significados para los buffers.
La vista con signo del índice del otro coche y la vista BYTE del índice propio
conservan los contratos originales de `StageObject_ApplyRecursiveFrameDelta`.

## Clasificación de memoria y disco

- **Car:** instancia de ejecución con recursos y matrices nativos. Estos
  consumidores deben seguir los miembros cuando los punteros ensanchan el tipo.
- **CollisionBox:** scratch de ejecución. La selección de `pVertex` por miembro
  corrige el acceso fijo a `+0x94` cuando `pArray` ocupa ocho bytes.
- **Tabla de entradas estáticas:** tabla construida en ejecución. Su constructor
  todavía escribe punteros en slots `int` y recorre pares de ocho bytes; queda
  pendiente convertir también el almacenamiento y los demás consumidores.
- **StageObject:** **respaldado por disco**. `0x4b93c0` recorre registros de `0xa0`
  bytes en el archivo de malla, reubica el offset de malla de `+0x0c` contra el
  array de meshes y el enlace de `+0x98` contra el array de objetos, in situ.
  Se marca con `// ON-DISK:` en `Sector.h`. OpenCMR2 deberá separar esa forma
  de disco de un objeto nativo; desactivar una aserción no soluciona el loader.

El acceso a los flags usa `(*entry)->field_0x10`, que sigue al puntero nativo
de malla. Se conserva la lectura de un dword y la máscara `0x2001000` del
original. El smoke test usa ese miembro en un objeto nativo deliberadamente;
no interpreta bytes de un archivo como un `StageObject` de 64 bits.

## Macro común y compatibilidad con MSVC6

`LayoutChecks.h` define `CMR2_LAYOUT_CHECK(nombre, condición)` como el typedef
original cuando está activo `_M_IX86` o `__i386__`, e inactivo en x64.
Se convierten **600 comprobaciones existentes** de quince archivos; con las
25 nuevas quedan **625** comprobaciones bajo la misma interfaz.

Añadir un include nuevo a las siete cabeceras compartidas que contenían
typedefs alteraba la puntuación de `GameMenus.cpp:0x44a1b0`, aunque la expansión
del typedef era idéntica. Se aisló cambiando solo el grafo de includes.
Para MSVC6 (`_MSC_VER <= 1200`) esas siete cabeceras proporcionan la misma
definición de compatibilidad, protegida contra redefinición, sin introducir
el include nuevo. Los compiladores modernos incluyen el header común.
Esta disposición recupera la puntuación anterior de todas las funciones;
las comprobaciones de MSVC6 permanecen activas. No usa pragmas ni cambios
de código para forzar instrucciones. Una condición deliberadamente falsa
falla en x86 y compila en x64 en la prueba nativa.

## Validación y límites

- Build completo MSVC6, medición completa, `prepare_fastcmp.py` y gate de las
  **33 unidades afectadas** aprobados. Las 3364 filas de `bytes.json` son
  idénticas al punto estable, incluidos `s/fz/x`; **2922 byte-exactas**,
  perfect **56,10 %**, fuzzy **94,50 %**, cero incidencias de datos.
- Nuevo harness con **4446 casos**: cuerpos originales/recompilados completos,
  heap envenenado, ancho de stores globales, trazas de proveedores, índices
  con signo y ABI/stdcall. Los clamps y las matrices son reales. La respuesta
  estática tiene además un modelo entero independiente; overlap/sphere,
  daño, frame recursivo, cheats y spawn de debris son hojas controladas.
- Cuatro cuerpos actuales compilados y ejecutados en x64 con direcciones
  mayores que `UINT32_MAX`, estructuras actuales, primitivas portables de
  OpenCMR2 y ASan/UBSan. Graphics/mesh son declaraciones opacas de fixture.
  LeakSanitizer se desactiva porque el sandbox usa ptrace; ASan/UBSan siguen
  activos. Esta prueba no es un build de todo el juego.
- Suite completa: véase el resultado guardado junto a esta tanda.

El inventario pasa de **1663 a 1504 accesos crudos** y de 2248 a 2243
multiplicadores hexadecimales. Los 1285 identificadores desconocidos distintos
no cambian: nombrarlos por estética está fuera del objetivo de esta tanda.

Siguen pendientes el pool `g_unk0x00590ed0[8][0x98]`, sus constructores,
los globals scratch declarados como `FixVector *`, la tabla estática de pares
de dwords y otras rutas de la familia. También se conservan recorridos por
bytes de arrays que contienen exclusivamente enteros fixed-point. No se
presentan las cuatro respuestas como una conversión completa del subsistema.

Reproducción de la comprobación nativa y de su auditoría:

```sh
python3 tests/check_64bit_car_contacts.py --baseline-commit c9c8b9c \
  --audit-json /tmp/car-contacts-cast-audit.json
python3 tests/differential_car_collision_access.py CMR2PROGRESS/entities.json
```

La primera orden requiere Clang y el checkout hermano de OpenCMR2 (se puede
indicar otra ubicación con `--port-root`). Solo lo lee; no sincroniza ni
modifica el port. Ningún commit de esta tanda se publica en origin.

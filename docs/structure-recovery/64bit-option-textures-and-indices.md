# Preparación de 64 bits: texturas de opciones e índices de frontend

Segunda tanda de `CODEX-64BIT.md`, sobre `53b60c1`. Las diez referencias
de textura y el array de doce piezas pasan de `int` a `Texture *`.
Se convierten juntos su carga y todos sus consumidores, conservando los
nombres y las declaraciones independientes de los globals. Cuatro getters
que transportaban números bajo un retorno `void *` pasan a retornar `int`.

## Evidencia y alcance

`OptionMenu_LoadPartTextures` (`0x50a080`) asigna los retornos `Texture *`
de `CTexture::FindLoadTexture`. Sus rutas de archivo distinguen las tres
imágenes Dan, el banner de país, las dos flechas, tick/cross/box/circle
y las doce piezas. El renderer utiliza dimensiones signed-short y una
vista de cuatro shorts del registro de textura. No se deduce el tipo solo
de los offsets: coincide con el retorno de la carga y el parámetro de
`Sprite_Queue`.

| Dirección original | Almacenamiento ahora tipado |
| --- | --- |
| `0x831360 / 0x831364 / 0x831368` | Tres `Texture *`, imágenes Dan |
| `0x831668` | `Texture *`, banner de país |
| `0x83166c / 0x831670` | Dos `Texture *`, flechas |
| `0x8313ac / 0x831648 / 0x8313b0 / 0x831674` | Cuatro `Texture *`, tick/cross/box/circle |
| `0x83137c` | `Texture *[12]`, piezas |

Los consumidores de esta familia son `0x50b1c0` (filas animadas), `0x50e780`
(filas de valores), `0x50a920` (resultados/pieza), `0x50bfd0` (strip/banner)
y `0x500550` (separadores). Se preserva el uso del rect de **DanRed** también
cuando se dibuja DanOra o DanYel. Las referencias nulas conservan las guardas
del original; no se introducen comprobaciones en caminos que no las tenían.

La tabla compilada de frontend `0x516b40` ya separa `int ids[28]`,
`const char *dirs[22]` e `int flags[28]`. Su inicializador contiene ids
pequeños, nombres de directorio y flags 0/1. Los cuatro métodos leen
los arrays numéricos y sus callers los usan como índices/flags, no como
direcciones. Se corrigen los retornos en el header, las declaraciones
repetidas de `Car.cpp` y todos los callers del proyecto:

| Función | Lectura y retorno correcto |
| --- | --- |
| `0x40ee70` `GetArchivePrimaryFlagEntry` | `flags[index]`, `int` |
| `0x40ee80` `GetArchiveSecondaryFlagEntry` | `flags[index + 14]`, `int` |
| `0x40ee90` `GetArchivePrimaryIDEntry` | `ids[index]`, `int` |
| `0x40eea0` `GetArchiveSecondaryIDEntry` | `ids[index + 22]`, `int` |

Se mantiene el signedness de los enteros y la conversión a unsigned del
caller que la necesita. Los flags no se estrechan a `bool`; la prueba
comprueba también valores de 32 bits distintos de 0/1. El getter de
directorios conserva su retorno de dirección y queda fuera de este cambio.

## Layout y clasificación

| Texture, Win32 | Campo comprobado | Uso |
| --- | --- | --- |
| Tamaño `0x130` | `sizeof(Texture)` | Registro de textura de ejecución |
| `+0x114` | `pSurface` | Recurso nativo que ensancha el layout en x64 |
| `+0x11c` | `field_0x11c` | Inicio de los cuatro shorts del rect fuente |
| `+0x120 / +0x122` | `width / height` | Dimensiones con signo |
| `+0x12c` | `pArchive` | Archivo de ejecución del que procede la textura |

Las seis comprobaciones se añaden a `LayoutChecks.cpp`. Se conserva la vista
`SpriteRect *` del prefijo de shorts; los offsets de dimensión y de rect
siguen ahora los miembros cuando ensancha `pSurface`. No se cambia el tipo
Texture ni se añade un miembro que pueda alterar el layout.

**Texture y la tabla de frontend son registros de ejecución**. El primero
contiene la superficie y el archivo de recursos activos; la tabla de
frontend contiene punteros de un inicializador compilado, no offsets
reubicados de un fichero. DDS/TGA son formatos separados. No se cambia
ninguna representación en disco ni se marca un buffer como struct sin
evidencia. Los campos `field_0x11c/field_0x11e` conservan sus nombres: esta
tanda prepara el acceso, sin una recuperación estética del resto del tipo.

## Validación

- Build/medición completos, `prepare_fastcmp.py` y gate de **24 unidades**
  aprobados. Las **3364 filas** de `bytes.json` son idénticas a `53b60c1`:
  **2922 byte-exactas**, perfect **56,10 %**, fuzzy **94,50 %**, cero
  incidencias de datos. Los hashes de fuentes, EXE y PDB se verifican
  contra el manifest y la procedencia de la medición.
- Nuevo harness: **10640 casos**, con un modelo independiente de los
  cuatro getters, 22 cargas por escenario y todos los consumidores de la
  familia ejecutados completos. Compara heap envenenado, globals, stores
  de ancho original, rects/dimensiones con signo, textos y trazas/ABI.
  Rendering, selección, formato y proveedores de recursos son hojas
  controladas; el strip ejecuta las filas anidadas reales.
- Prueba nativa: **cinco cuerpos actuales** en x64 con ASan/UBSan,
  **32 escenarios de carga / 704 stores** y **76 casos de getters**.
  Las referencias no nulas son mayores que `UINT32_MAX`; se comprueba la
  identidad completa de cada puntero, incluyendo los doce slots del array.
  Los headers Texture/GenericFile son actuales; las declaraciones de Win32/
  DDraw y los proveedores se sustituyen por fixtures. LeakSanitizer se
  desactiva por el ptrace del sandbox, manteniendo ASan/UBSan.
- Resultado de la suite completa guardado junto a este documento.

Se eliminan **76 expresiones de cast** que convertían direcciones/números:
40 casts a `int`, 32 a `Texture *` y cuatro a `void *`. Son expresiones del
fuente, no el número de instrucciones ni un audit global de OpenCMR2.
Se eliminan también once vistas `short *` de offsets crudos al usar campos.
El inventario pasa de **1504 a 1493 accesos crudos**; no cambian los nombres
desconocidos ni el tamaño de los formatos serializados.

## Pendiente

La API `Sprite_FillRect` todavía recibe como `int` una dirección que no
consume. Las seis llamadas de las filas conservan esa API y el proyecto
tiene más de 300 callers que deberán convertirse juntos, incluyendo el
wrapper de ancho escalado. No se afirma que el cuerpo completo de las filas
compile ya en x64. Hay además parámetros que transportan direcciones en
otras pantallas y allocators/copies gráficos que usan tamaños literales.
El siguiente bloque de ese contexto necesita tipar toda la interfaz y
seguir el miembro nativo de Graphics, con cobertura de sus consumidores.

```sh
python3 tests/differential_option_texture_records.py CMR2PROGRESS/entities.json
python3 tests/check_64bit_option_records.py
```

La comprobación nativa requiere Clang y solo escribe fixtures temporales.
No modifica OpenCMR2 ni publica commits.

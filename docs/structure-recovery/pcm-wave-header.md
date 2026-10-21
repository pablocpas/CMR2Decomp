# Cabecera fija de las muestras WAV

`Sound_LoadWave` (`0x4a1f50`) interpreta el archivo como una cabecera de 44
bytes seguida de los datos de muestra. `PcmWaveHeader` representa ese formato
sin punteros; conserva el tamaño tanto en Win32 como en x64. El cargador usa
campos para los metadatos y obtiene el inicio de datos mediante `pWave + 1`.
Desaparecen siete desreferencias crudas y los desplazamientos de esos accesos.

| Offset | Campo utilizado | Evidencia |
| --- | --- | --- |
| `0x08` | `waveId[4]` | Comparación de cuatro bytes con `WAVE`. |
| `0x16` | `channels`, palabra sin signo | Argumento `channels` de `Sound_CreatePcmSampleBuffer`. |
| `0x18` | `sampleRate`, DWORD | Argumento `rate`, escrito después en `nSamplesPerSec`. |
| `0x22` | `bitsPerSample`, palabra sin signo | Argumento `bits`, escrito en `wBitsPerSample`. |
| `0x28` | `dataSize`, DWORD | Tamaño de creación del buffer y de copia de datos. |
| `0x2c` | Final de cabecera | Puntero de origen pasado a `Sound_CopyBufferData`. |

Los campos intermedios describen la cabecera RIFF/PCM fija; los tipos del
formato de audio se corresponden con `WAVEFORMAT` y `PCMWAVEFORMAT` del SDK
usado por el proyecto. Seis comprobaciones fijan el tamaño y todos los offsets
que lee esta función. La declaración queda en una cabecera específica, incluida
solo por el cargador y los checks, para evitar afectar a otras unidades con
declaraciones compartidas sensibles a MSVC6.

## Comportamiento conservado

El cargador comprueba únicamente `WAVE`: no recorre chunks ni valida los IDs
RIFF/fmt/data, el tag PCM o las longitudes de esos chunks. Una firma diferente
omite la carga y devuelve éxito después de la liberación que corresponda.
No se convierte esta interpretación fija en un lector general de WAV.

El bit 0 de `flags`, junto con `g_sound3DEnabled`, selecciona la ruta 3D.
La ruta normal ignora el resultado de la copia. En 3D, un fallo de
`QueryInterface` omite copia y modo, pero no convierte el resultado final en
fallo. Un fallo de copia 3D devuelve falso; un fallo de `SetMode` no lo hace.
Los fallos de creación y de copia 3D retornan antes de liberar el archivo.
El final común libera solo los archivos que no pertenecen al archivo agregado.
Estos comportamientos existentes se conservan, sin introducir correcciones de
gestión de recursos en esta tanda de recuperación.

`dataSize` se vuelve a leer después del proveedor de creación. No se cachea
todo el formato o tamaño como parte del cambio de tipos. Los índices de las
tablas de muestras también conservan sus consultas al getter original.

## Validación y límites

El gate de las dos unidades afectadas conserva las 73 funciones exactas de
`Sound.cpp` y todas sus puntuaciones. La compilación y medición completas
conservan las puntuaciones byte/fuzzy de las 3364 funciones, incluidas las
2924 exactas, sin errores de comparación de datos. La suite completa tiene
141 harnesses y pasa sobre ese mismo ejecutable medido.

El nuevo harness compara 1296 casos del original y el recompilado con un
modelo independiente de rutas, llamadas y propiedad del archivo. Incluye
cuatro formatos, flags altos, valores cero/negativos del flag global, tres
marcas de pertenencia al archivo agregado y nueve escenarios de resultados.
Los metadatos que no valida el loader están envenenados. Se comprueban los
anchos sin signo, HRESULT positivo/negativo, BOOL negativo como éxito,
tablas completas de 200 punteros, slots 0/7/199/3, registros/stack del ABI y
el heap completo. Un proveedor modifica `dataSize` entre creación y copia
para comprobar la relectura.

Los mismos 1296 casos pasan en x64 con ASan/UBSan. Se extraen la cabecera,
el cargador y el predicado HRESULT de producción; los proveedores verifican
punteros por encima de 4 GB y tablas con punteros nativos de ocho bytes.
La prueba controla las hojas de archivo y audio: no establece reproducción
real en DirectSound ni seguridad ante archivos truncados o formatos variables.

El inventario baja de 99 a 92 candidatos de acceso crudo. Los 1391 campos
detectados sin nombre semántico siguen pendientes; esta cabecera sustituye
accesos a un buffer, no declaraciones previas con nombres desconocidos.

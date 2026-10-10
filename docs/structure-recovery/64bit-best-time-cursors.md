# Cursores nativos de las cinco tablas de récords

Tanda sobre `e6c36f3`, dentro del trabajo de `CODEX-64BIT.md`.
`FrontendRecords_BuildScrambledBestTimeTables` (`0x4f8b30`) deja de
transportar direcciones en `int` y números en `BYTE *`. Se convierten juntas
las cinco pasadas, sin dividir sus variables reutilizadas ni sus lifetimes.

## Evidencia y tipos

| Slot original | Uso numérico | Uso de dirección |
| --- | --- | --- |
| `iVar2` | Contador XOR / índice de evento | Resultado del proveedor, cuarta pasada |
| `iVar3` | Contador XOR | Cola del tiempo de rally / flags del evento |
| `local_c` | Cantidad de etapas / offset serializado | Cursor de salida, cuarta pasada |
| `local_10` | País / offset serializado | Cursor de salida, segunda y quinta pasadas |
| `iVar4` | Ninguno | Dirección de los flags de rally |

Los cuatro slots mixtos usan `BestTimeCursor`, una unión con miembros
numérico, byte pointer, word pointer y pointer a la cola del tiempo. Solo
se lee el miembro activo de cada pasada. La unión mide cuatro bytes en
Win32 y ocho en el ensayo nativo. `iVar4` pasa a `unsigned int *`.
Las cinco caminatas XOR avanzan mediante `BYTE *`, sin convertir la
dirección a entero de 32 bits.

`BestTimeRallyTail` describe exclusivamente los ocho bytes a partir de
`save + 0x34 + country*0x24 + difficulty*0xc`. El primer word conserva
`field_0x0`: esta función no demuestra su semántica. El segundo es
`centiseconds`, probado por las divisiones 6000/100 y los módulos 60/100
de los tres campos del identificador. No se afirma recuperar todo el save.

Ambos tipos tienen `sizeof` y los offsets de sus miembros comprobados
en `LayoutChecks.cpp`. La cola pertenece a un registro guardado de words
fijos, sin punteros reubicados: no cambia entre 32 y 64 bits. La unión
de cursores es almacenamiento de ejecución y no forma parte del fichero.

Se conservan el orden de declaraciones y las 250 llamadas al proveedor,
incluyendo la llamada descartada y el `puVar7` arrastrado desde la tercera
pasada a la cuarta. No se corrigen esas peculiaridades del original.

## Comparación del helper

`FrontendProfile_ScrambleIdentifier` (`0x4fb8d0`) comparaba dos direcciones
convertidas a `int`. Una comparación directa entre pointers cambió el
salto signed por unsigned y redujo el score de 95,95 a 94,59 %. La resta
de pointers empeoró también el código. La comparación de direcciones en
`ptrdiff_t` conserva el ancho nativo y la comparación signed original,
con las mismas puntuaciones. No se usa el entero para almacenar ni
reconstruir un pointer.

Una prueba de recorrido sin cursor anterior al array en
`FrontendText_FormatCodeGroups` perdió su exactitud. Se descarta por completo
ese cambio: esta función queda intacta. El arnés Win32 ejecuta su cuerpo real;
el ensayo nativo usa una hoja de formato por índices, documentada en el fixture,
para no atribuir a esta tanda una reparación del formatter.

## Validación

- Build completo, medida completa y `prepare_fastcmp.py`.
- Las **3364 filas de `bytes.json` son idénticas** a `e6c36f3`:
  **2922 byte-exactas**, perfect 56,10 %, fuzzy 94,50 %, sin incidencias de datos.
  Se verifican manifest, procedencia y hashes de las fuentes.
- Gate de las tres unidades afectadas: ninguna pérdida de puntuación.
- El arnés existente `differential_best_time_codes.py` ejecuta la función y
  los builders reales: 128 casos, dos envenenados de stack, tabla completa,
  celdas sin usar, límites, guardas y la cuenta de llamadas. Detecta además
  la mutación del umbral 16 a 17 minutos.
- `check_64bit_best_time_records.py` compila **tres cuerpos actuales**:
  builder, scramble y guest identifier. Son 64 escenarios y 16000 llamadas
  con el save en stack por encima de 4 GB, bajo ASan/UBSan. Compara con el
  ejecutable original todos los bytes de las cinco tablas y el save intacto.
  El proveedor y el formatter son hojas controladas.
- **114 harnesses, cero fallos**. Resultado de la suite completa y del gate
  guardado junto al documento.

El inventario pasa de **1493 a 1489 accesos crudos**, al sustituir las cuatro
lecturas del tiempo de rally por el miembro. No se altera la expresión de
los masks de los otros formatos ni los buffers de salida de capacidades
distintas: no hay evidencia de que sean arrays completos del mismo tamaño.

## Accesos pendientes de esta familia

Los offsets de selección dentro del save y los strides 8/12/16/25/75 siguen
explícitos. Ya no contienen direcciones truncadas: son posiciones en formatos
de words o caracteres fijos. Queda recuperar el layout completo del save y
las vistas de cada tabla para convertir también esos recorridos. Este
documento registra esa deuda; **la tanda no termina el objetivo global**.

```sh
python3 tests/differential_best_time_codes.py CMR2PROGRESS/entities.json
python3 tests/check_64bit_best_time_records.py
```

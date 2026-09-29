# Revisión de fidelidad — 29 de septiembre de 2026

Base: `decomp/callbacks`, commit `adfc56d`. Trabajo de esta revisión:
`decomp/matching-review`, en `/home/pablo/colin_mcrae_linux/matching-review`.

## Estado medido

Se compiló la base con MSVC6 y las opciones de `tools/build.sh`. Las medidas se
compararon contra ese HEAD, usando informes separados de los JSON compartidos.
La compilación existente de `/home/pablo/cmr2-callbacks` dio los mismos porcentajes
que la compilación limpia de la base.

| Medida | Base | Después del lote |
|---|---:|---:|
| Entradas medidas del inventario con matching >= 90 % | 2505 | 2507 |
| Entradas medidas del inventario con matching < 90 % | 841 | 839 |
| Entradas del inventario al 100 % | 2095 | 2098 |
| Matching medio de las implementadas | 89,43 % | 89,48 % |
| Funciones del juego sin empezar según `funcstat.py` | 0 | 0 |
| Incidencias de datos iniciales | 0 | 0 |

El inventario contiene 3646 entradas: incluye bibliotecas y artefactos del
análisis. `funcstat.py` identifica 3343 funciones del juego. La categoría de
matching inferior al 90 % suma unos **620 KB**, por lo que la media por función
oculta la dificultad de las rutinas grandes. La cobertura del inventario no
demuestra que todas las funciones tengan la lógica correcta ni que el análisis
del ejecutable haya descubierto todo el código.

## Cambios de este lote

| Dirección | Función | Antes | Después | Corrección |
|---|---|---:|---:|---|
| `0x431c10` | Reinicio con fade del coche | 8,00 % | 100,00 % | Callback recompilado y color empaquetado en cuatro bytes |
| `0x43ecd0` | Actualización de ejes del coche | 99,23 % | 100,00 % | Leer `Car::field_0xb1a`, evitando multiplicar el offset por `sizeof(Car)` |
| `0x49fd90` | `CInput::DInputReleaseDevices` | 94,02 % | 100,00 % | Recorrer cuatro slots y reproducir la comparación con signo del original |
| `0x48df50` | Deslizamiento y amortiguación del coche | 42,46 % | 82,35 % | Accesos en bytes y forma original de los valores absolutos |
| `0x402f90` | Constructor de opciones de red | 88,34 % | 90,80 % | Referencias reales a los callbacks |
| `0x4035e0` | Constructor de menú | 94,52 % | 95,89 % | Referencia real a su callback |
| `0x404000` | Constructor de menú | 79,00 % | 80,00 % | Referencia real a su callback |
| `0x457000` | Preparación de modelos del coche | 68,68 % | 70,91 % | Compartir la tabla de detalle que modifica Graphics |
| `0x4f8b30` | Tablas de identificadores del perfil | 60,58 % | 60,96 % | Límites relativos a los arrays recompilados |

También se corrigieron:

- `Sector.cpp`, `0x4b93c0`: el bucle de texturas avanzaba 10 + 19 dwords entre
  triángulos. El original avanza 19 en total, es decir, `0x4c` bytes.
- `GameInfo.cpp`, `0x5029b0`: el límite del bucle dependía de una dirección del
  ejecutable original. Ahora depende del array correspondiente.
- Global `0x537f34`: GameInfo definía un entero independiente del array de Race.
  GameInfo y NetRace usan ahora el array definido en Race.
- Global `0x51a3d0`: StageTiming leía una copia independiente de la tabla que
  Graphics modifica al cambiar la calidad. Ahora usa la misma tabla.
- Anotaciones repetidas de declaraciones `extern` y del callback `0x484310`:
  se conserva la anotación de cada definición. `check_dupes.py` queda limpio.

Parte de las correcciones de lógica ya existía en `port/silentpatch-window`,
especialmente en `4d59d62`; se recuperaron y verificaron para esta rama de
fidelidad. El color de `0x431c10`, la forma del cálculo de `0x48df50`, la
comparación de `0x49fd90` y la unificación de `0x51a3d0` se ajustaron durante esta
revisión contra el original.

## Validación y límites

- Compilación completa con MSVC6: correcta.
- Comparación completa contra la base: ninguna función medida desaparece.
- `reccmp-datacmp`: cero incidencias antes y después.
- `check_dupes.py`: limpio, 3361 direcciones de funciones y 3196 de globales.
- `git diff --check`: correcto.
- `tests/differential_slip.py`: ejecuta el código máquina original y recompilado
  de `0x48df50` con 6000 registros de coche. Compara el registro entero, sus
  guardas y el puntero global. Cero diferencias. Cambiar deliberadamente la
  constante de amortiguación de `0xf851` a `0xf850` se detecta en 5917 casos.

Cuatro porcentajes bajan al recolocar el binario. Se compararon las instrucciones
de las compilaciones base y final, preservando opcodes, registros, offsets de
campos y constantes; se resolvieron únicamente las direcciones de símbolos y
saltos. Las cuatro funciones conservan el mismo código tras esa reubicación:

| Dirección | Antes | Después | Instrucciones verificadas |
|---|---:|---:|---:|
| `0x428bf0` | 59,96 % | 59,78 % | 560 |
| `0x45f6d0` | 83,98 % | 83,12 % | 115 |
| `0x4984b0` | 85,45 % | 81,82 % | 55 |
| `0x505b40` | 61,86 % | 61,44 % | 236 |

Son variaciones de la resolución de operandos del comparador; no se cambió su
lógica. El porcentaje de reccmp compara ensamblado normalizado, y no certifica
igualdad literal de todos los bytes del ejecutable. La prueba diferencial cubre
una rutina de física; **no se ha validado una partida completa** con este lote.

## Siguiente trabajo, por riesgo funcional

1. **Vectores sobre la pila.** `SceneNode_Rotate` (`0x4ac820`) pasa direcciones
   de enteros sueltos a `FixVecScaleRecip`, que escribe tres componentes.
   `StageTiming.cpp`, `0x483100` y `0x484f40`, contiene el mismo patrón. Los
   escalares independientes no garantizan memoria contigua. Recuperar las
   correcciones de vectores reales de `4d59d62` y probar las rotaciones con un
   harness diferencial.
2. **Carga de geometría.** `GameInfo.cpp`, `0x5062d0`, todavía escribe en
   `0x831088` como dirección absoluta, usa el registro incorrecto del cargador y
   pierde una rama de recursos por comprobar una variable inicializada a cero.
   `4d59d62` contiene las correcciones del cargador y de sus nodos.
3. **Datos y arrays que se leen fuera de su objeto.** Entre los casos ya
   corregidos en la rama de ejecución: `0x82c070`, las curvas y nombres de audio,
   los slots de luces `0x547d00` y las tablas por jugador. Recuperar los cambios
   por grupos y comparar datos, límites y consumidores. Un `datacmp` limpio no
   detecta un array truncado ni una lectura del objeto vecino.
4. **Layout de Car y globals vecinos.** `overlap.py` sigue avisando de tamaños
   que cruzan el siguiente global (`g_cars`, `g_carOrder`, `g_carViewScale` y
   otros). Algunas vistas son intencionadas; hay que comprobar cada caso con el
   ensamblado y modelar el almacenamiento compartido explícitamente.
5. **Matching residual.** Después de cerrar estos riesgos, atacar la banda
   85–99 % y las rutinas grandes con diferencias de llamadas, ramas, anchuras y
   offsets. Reservar las diferencias exclusivamente de registros o slots de
   pila para una pasada posterior.

## Precauciones con las herramientas locales

- La carpeta inicialmente abierta está en `decomp/frontend`, no en `callbacks`.
  Los handoffs antiguos tienen cifras y prioridades anteriores a terminar la
  implementación del inventario.
- `tools/current.json` y `baseline.json` son compartidos. Usar un informe propio
  del HEAD que se está revisando; no refrescar el baseline de otra rama.
- `opsim.py` y `callcount.py` todavía fijan la ruta del repositorio antiguo y no
  respetan `CMR2_REPO`. Comprobar la imagen usada antes de aceptar sus resultados.
- `check.sh` y `match.sh` pueden continuar tras un error de compilación si existe
  un `.exe` anterior. Exigir éxito de `build.sh` antes de medir.
- El aviso de un global duplicado puede ser una declaración `extern` repetida,
  o dos objetos reales. Distinguirlos antes de unificar datos.

Reproducir las comprobaciones desde esta carpeta:

```bash
export CMR2_REPO="$PWD"
export WINEPREFIX=/home/pablo/colin_mcrae_linux/tools/wineprefix
export WINEDEBUG=-all
/home/pablo/colin_mcrae_linux/tools/build.sh && \
  reccmp-reccmp --target CMR2 --no-color --silent \
    --json /tmp/cmr2-review.json --json-diet
reccmp-datacmp --target CMR2
python3 /home/pablo/colin_mcrae_linux/tools/check_dupes.py
python3 tests/differential_slip.py /tmp/cmr2-review.json
```

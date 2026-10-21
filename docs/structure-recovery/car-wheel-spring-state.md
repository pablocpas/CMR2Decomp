# Estado y ajuste de los resortes de rueda

La integración de `CarPhysics_IntegrateWheelSuspension` (`0x42e8e0`) y sus
consumidores permiten nombrar siete campos más. Son nombres descriptivos
justificados por el ejecutable y sus usos, no símbolos del fuente original.

| Campo y offset | Productores y consumidores |
| --- | --- |
| `wheelRideHeight[4]`, `0x988` | `Car_SetRideHeight` (`0x43dff0`) escribe `FixMul(0x1999, 1-setting)`. Se suma al desplazamiento al construir el cuerpo (`0x432cc0`), levanta su matriz (`0x444a70`) y define el límite de compresión de impactos (`0x46a500`, con mínimo `0x51e`). |
| `bodyLeanHeight[4]`, `0x9a8` | `Car_UpdateSuspension` (`0x43b100`) rota las esquinas en la base derivada de `lean`, añade el offset de daño de suspensión y guarda su Y. `Car_UpdateBodyMatrix` lo suma a la altura y desplazamiento del resorte antes de construir la base del cuerpo. |
| `wheelTravelLoadScale`, `0x9c0` | El setter guarda el inverso del rango dependiente de la altura. `Car_IntegrateWheelTravel` (`0x42e450`) lo aplica al recorrido relativo a la media para producir `wheelTravelLoadBias`. |
| `wheelSpringVelocity[4]`, `0x9c8` | Recibe el impulso por segunda diferencia de posición y la fuerza restauradora/amortiguadora integrada; a continuación se integra en `wheelSpringOffset`. Los impactos leen su magnitud. |
| `wheelSpringOffset[4]`, `0x9d8` | Recibe velocidad por paso temporal y se limita inferiormente a `-0x3333`. Se suma a la altura visual y se compara con el límite de impacto. |
| `wheelSpringStiffness[4]`, `0x9e8` | El setter escribe `0x4ccc`; la dinámica multiplica este coeficiente por el desplazamiento. |
| `wheelSpringDamping[4]`, `0x9f8` | El setter escribe `FixMul(setting, 0x5999) + 0x4000`; la dinámica multiplica este coeficiente por la velocidad. |

Los comentarios anteriores intercambiaban velocidad y desplazamiento. El
orden de operaciones del original establece la distinción:

```text
v += dt * -(stiffness * offset + damping * v)
offset += dt * v
```

Las operaciones reales conservan todos los `FixMul`, su orden, sus redondeos
y el wrap de 32 bits. Si el desplazamiento cruza `-0x3333`, se satura y se
elimina solo la velocidad negativa. La entrada desde las esquinas es la
segunda diferencia de sus tres posiciones, transformada a Y del coche;
no es simplemente la velocidad vertical. Solo se inyecta el término positivo,
con distancia absoluta al suelo estrictamente menor que `0x4ccc` e historial
anterior distinto del vector cero.

Los locales identifican `secondDifference`, `previousStep`, `currentStep`,
`bodySecondDifference`, `groundDistance`, `velocityStep` y `wheel`. El setter
nombra `heightSetting`, su complemento y los dos rangos cuyos inversos guarda.
Se conserva el override del cheat 6, que fuerza `-0x6666`, sin añadir clamps.

## Accesos y layout

El setter recorría `((int *)field_0x9e8)[wheel + 4]` para escribir los cuatro
amortiguadores. Ese acceso atravesaba el límite del array de cuatro rigideces.
Ahora escribe `wheelSpringDamping[wheel]` en el array contiguo ya declarado,
sin alterar posiciones, anchos ni orden de escrituras. Se retiran también el
cast del array de rigidez y las vistas redundantes `(&array[0])[wheel]` del
integrador. Trece checks verifican los siete offsets y las seis capacidades.

`Car_RestorePhysicsFromRecord` (`0x426d80`) copia ahora de `bodyAxes` a
`targetAxes` con su tamaño declarado, en lugar de sumar `0x360/0x384` al coche
y expresar el tamaño como nueve enteros. Esa copia conserva bytes exactos.

## Validación

El gate de 18 unidades y la medición completa conservan las 3364 puntuaciones
`s/fz/x`, las 2924 funciones exactas y cero problemas de datos. Setter,
integrador, montaje del cuerpo y restore mantienen sus coincidencias exactas.
Los porcentajes globales permanecen en 56,28 % de bytes perfectos y 94,46 % fuzzy.
Pasan los 139 harnesses diferenciales sobre ese mismo build medido y los
354 escenarios nativos existentes de estado inicial del coche.

`differential_car_spring_state.py` incorpora 602 escenarios con modelos
independientes de enteros: 242 ajustes y 360 integraciones completas. Cubre
valores de altura dentro y fuera del rango nominal, override del cheat, masas
con signo y truncado de división, las cuatro ruedas, ambos lados y la igualdad
de los umbrales de suelo/saturación, historial ausente, matrices rotadas o
embebidas en el coche y pasos temporales cero, fraccionarios y con signo.
El único proveedor controlado es la consulta del cheat; la rotación inversa
es real. Se comparan el heap completo envenenado, las escrituras globales,
el orden de consulta y ABI contra el original y el rebuild.

`check_64bit_car_spring_state.py` ejecuta los mismos 602 casos con los dos
cuerpos de producción y la rotación inversa, headers reales, punteros nativos
mayores de 4 GB y ASan/UBSan. Compara el heap proyectado completo contra el
original x86, guardando los quince punteros del coche y el padding nativo.
Solo la fixture traduce los dos tramos sin punteros del coche original.

El inventario sigue en 113 candidatos crudos: su detector no cuenta los
subíndices que cruzan arrays ni estos casts redundantes. Las declaraciones
que detecta sin nombre semántico bajan de 1405 a 1401; los otros tres campos
renombrados tenían nombres parciales `wheel0x...`, no contabilizados en esa
métrica. No se presenta esa cuenta como cobertura completa del código.

Permanecen pendientes los demás campos de suspensión, el recorrido sesgado
de cargas y el resto del inventario del proyecto. El objetivo sigue abierto.

# Motor, controles y cambio: cuarta tanda

Se han establecido o corregido los nombres de 64 miembros: 59 de `Car`,
cuatro de `CarNetRecord` y uno de `CarPartSet`. Se conservan sus tipos, anchos,
signedness, cardinalidad y offsets. El inventario registra 60 declaraciones
menos con nombres basados en offsets; cuatro miembros ya tenían nombres
semánticos que necesitaban corregirse o propagarse al registro de red.

## Evidencia de los campos

| Campos de `Car` | Offsets | Lectores y escritores que establecen su función |
| --- | --- | --- |
| `bodySize`, `upperCornersLocal[4]`, `collisionRadius`, `mass`, `inverseMass` | `0x1f8`, `0x240`, `0x758..0x760` | `Car_Spawn` (`0x43c7f0`) obtiene `halfExtents` de las dimensiones completas, calcula las cuatro esquinas superiores y el radio. Colisión usa el radio; las fuerzas y cargas de esquinas usan masa y su inversa. |
| `engineNetTorque`, `engineDragCoefficient`, `maxThrottleTorque`, `baseThrottleTorque`, `throttleRampStep`, `engineSpeedLimit`, `throttleTorque`, `throttlePhase`, `engineSpeed`, `revLimiterTorque` | `0x780..0x7b0`, con huecos conservados | `Car_UpdateEngineTorque` (`0x437dc0`), `Car_SplitEngineTorque` (`0x442fc0`) y `Car_UpdateEngineSpeed` (`0x4380c0`) derivan y distribuyen el par. `AutoGear_UpdateRollingFollowRate` (`0x4946c0`) integra el pedal y su fase. La configuración conserva el par base y adapta el máximo. |
| `baseDriveSplit`, `gearRatio[8]` | `0x7b8`, `0x7bc` | Reserva de ocho marchas, cocientes recíprocos de la tabla de velocidades y distribución del par por marcha; la relación de la marcha atrás es negativa. |
| `steeringInput`, `steeringAccumulator`, `steeringReturnRate`, `steeringTorqueScale`, `steeringSpeedScale` | `0x818..0x828` | Integración del control, retorno hacia cero y cálculo del par de dirección; `AutoGear_IntegrateSteeringAccumulator` (`0x494110`) obtiene el ángulo objetivo de este acumulador. |
| `maxBrakeForce`, `brakeRampStep`, `brakePhase`, `handbrakeRampStep`, `maxHandbrakeForce`, `handbrakePhase` | `0x82c`, `0x834`, `0x83c..0x844`, `0x84c` | `AutoGear_UpdateSteeringSwing` (`0x494880`) integra el freno; `AutoGear_UpdateSecondarySwing` (`0x494960`) integra el freno de mano. Los consumidores aplican sus fuerzas a las ruedas. |
| `wheelSpinForWheelLean[4]`, `wheelSpinForBodyLean[4]`, `wheelLeanDamping`, `bodyLeanDamping` | `0x890`, `0x8a0`, `0x9b8`, `0x9bc` | Giro filtrado por rueda para las dos integraciones de inclinación; sus coeficientes amortiguan los respectivos vectores. El segundo coeficiente también participa en amortiguación de contacto. |
| `groundRightDot`, `groundUpDot`, `groundForwardDot`, `groundHeightCorrection`, `deepestCorner` | `0x91c..0x924`, `0x958`, `0xb2b` | Productos escalares del normal con los ejes del cuerpo. `Car_UpdateGroundContact` (`0x42eae0`) elige la esquina con mayor penetración, o `-1`; la corrección vertical conserva valores positivos y negativos. |
| `cheatWheelDrop`, `cheatBodyLift` | `0xa88`, `0xa8c` | `Car_Spawn` fija la geometría del cheat 6; `View_GetCarBodyMatrix` (`0x423a30`) y `View_PlaceCarCamera` (`0x423b20`) aplican la elevación en el eje vertical del cuerpo. |
| `previousWheelSurface[4]`, `engineStartTimer`, `engineRestartPending` | `0xabe`, `0xafe`, `0xb4c` | Copia de las superficies del paso anterior y temporización/reinicio del motor. |
| `wheelSteeringAngle`, `targetSteeringAngle`, `renderSteeringAngle`, `maxSteeringAngleDegrees` | `0xb10..0xb16` | `AutoGear_IntegrateSteeringAccumulator` produce el ángulo de las ruedas; `Car_UpdateSteering` orienta los ejes de rodadura delanteros; `Car_StoreRenderTransforms` (`0x42af50`) usa la versión suavizada. Red y replay lo convierten según el máximo configurado. |
| `requestedGear`, `autoShiftDelay`, `lastShiftDirection`, `damageShiftDelay`, `reversing`, `shiftInProgress`, `automaticReverse`, `gearAtOrBelowBest`, `automaticGearbox` | `0xb20..0xb24`, `0xb5c`, `0xb84`, `0xb94..0xb9c` | `Car_UpdateAutomaticGear` (`0x493b30`), petición de marcha adyacente y `AutoGear_UpdateCarGearState` (`0x493520`). Dirección del cambio: 1 bajar, 2 subir. El retraso por daños se captura de `gearShiftDamage` y se descuenta en punto muerto. La marcha atrás automática intercambia la ruta de los pedales. |
| `braking`, `firstCameraBlocked`, `revLimiterActive`, `useUpperCollisionCorners` | `0xb54`, `0xb68`, `0xb78`, `0xc00` | Umbral del pedal de freno, rechazo de primera cámara, limitador de revoluciones y selección de cuatro/ocho esquinas de colisión. |

`CarNetRecord` conserva el ángulo de ruedas en `+0xc4`, el par en `+0xb8`,
el freno en `+0xd0` y la selección de esquinas en `+0xd4`.
`Car_RestorePhysicsFromRecord` (`0x426d80`) copia estos campos a sus equivalentes
en `Car`; el decoder de red escala el pedal normalizado por el par máximo del
modelo antes de restaurarlo. El registro completo mide `0xec`.

## Correcciones de interpretación

`steerFollowRate` (`Car +0x79c`) pasa a `throttleTorque`. Aunque influye en el
seguimiento de la dirección de rodadura trasera, su escritor integra el pedal
acelerador y sus lectores lo convierten en par de motor y ruedas.
`steeringFollowScale` (`CarPartSet +0x404`) pasa a `engineTorqueScale`: multiplica
ese par y determina el efecto de escape en `CarEffect_UpdateExhaustSmokeAndBackfire`
(`0x45af90`). Esta evidencia corrige el nombre propuesto en la segunda tanda.

`heading` (`Car +0xb10` y `CarNetRecord +0xc4`) pasa a `wheelSteeringAngle`.
Los campos `heading` de rutas y otros tipos mantienen sus nombres y usos.
El comentario de `Car +0xa8c` que decía «camera shake» se corrige: se trata de
una elevación fija del cheat 6. El campo de marcha atrás automática tampoco
se describe ya como un bloqueo genérico del cambio.

Se conservan los nombres históricos de funciones como
`Car_GetScaledSteerFollowRate` y `AutoGear_UpdateSteeringSwing`; sus nombres
no se usan como prueba del significado de los campos que tocan.

## Accesos y comprobación

La posición local de los emisores de rueda se obtiene mediante
`wheelEmitter[i]` y `pWheelNodes[i]->local`; la caída del cheat opera sobre
`wheelEmitter[i].y`. El calor del escape lee `CarPartSet::engineTorqueScale`
mediante el tipo recuperado. Se elimina la macro cruda de acceso al coche que
ya no tenía consumidores. Son cuatro candidatos crudos menos en el inventario.

`LayoutChecks.cpp` comprueba los offsets de todos estos miembros, además del
tamaño y offsets recuperados de `CarNetRecord`. Quedan campos de `Car` sin
interpretación demostrada, los registros de transforms todavía parcialmente
representados como bytes y las cargas serializadas de red/replay.

Los resultados del build completo, la comparación y la suite diferencial
constan en [car-controls-matching.json](car-controls-matching.json).

La reconstrucción completa conserva las 2922 funciones byte-exactas de 3364,
sin cambios de puntuación ni problemas de datos globales respecto a la tercera
tanda. Los 104 harnesses diferenciales pasan; la comprobación concurrente del
mapa de anotaciones también pasa. El inventario pasa de 2172 a 2168 accesos
crudos y de 1581 a 1521 declaraciones de campos sin nombre semántico.

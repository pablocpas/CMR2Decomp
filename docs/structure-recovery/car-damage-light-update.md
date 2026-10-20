# Daño, resets y actualización de luces

Esta tanda sustituye ocho nombres históricos por nombres respaldados por los
campos y llamadas del ejecutable. No son símbolos recuperados del código fuente
original. El mapa con evidencia está en `scripts/renames/017-functions.tsv`.

| Dirección | Nombre | Evidencia |
| --- | --- | --- |
| `0x466920` | `CarDamage_ResetRecord` | Reinicia el pool de impactos de etapa y las intensidades/flags de su snapshot, y los copia al estado activo. Conserva el padding de etapa y `stageSnapshotCaptured`. |
| `0x4668d0` | `CarDamage_ResetAllRecords` | Recorre los registros de daño en orden inverso y limpia los dos arrays de cooldown de deformación. No manipula streams de replay. |
| `0x4669b0` | `CarPart_ClearSetRange` | Pone a cero los conjuntos `CarPartSet` del rango de coches; es una operación distinta del reset de `CarDamageRecord`. |
| `0x466870` | `CarLight_GetBrokenFlags` | Lee `partBroken[4]` y `[5]`. El montaje asigna a esos slots los tipos de nodo 8/9, y las luces usan las salidas para habilitar sus dos canales. |
| `0x46b4c0` | `CarDamage_GetModelEnabled` | Devuelve `g_carDamageModelEnabled[pCar->index]`, que decide si las luces consultan los vértices deformados. |
| `0x480b40` | `CarPart_ResetAttachedNodes` | Reseed con 400, selección de `g_partCar`, restauración `SceneNode::current = local` y limpieza del puntero de nodo, en slots 3..0. |
| `0x483010` | `CarPart_DeactivateAndMarkHidden` | Limpia el bit activo de `PartState::flags` y marca oculto el slot de su nibble alto. |
| `0x4643f0` | `CarLight_UpdateFlagsAndGlows` | Máscaras de canales desde controles/roturas, temporizador de daño y actualización de cada glow desde niveles y vértices del modelo. |

El getter de daño, el reset de nodos y el parpadeo reciben `Car *`, sin puentes
por `BYTE *`. Las dos salidas de rotura se llaman `pBroken0/pBroken1` y sus
consumidores `broken0/broken1`: se conserva el significado de los canales sin
asignarles un lado espacial no demostrado.

El recorrido de luces usa `CarLightPoint` por índice. Desaparecen el cursor en
bytes, su incremento literal `0x28` y el cast del punto. `Mesh_ReadVertexFixed`
escribe ahora un `FixVector *`; el consumidor usa `meshPosition.x/y/z`, en lugar
de tres palabras sin tipo. `meshIndex`, `vertexIndex`, `originalX/Y/Z`,
`wheelCorner`, `channelLevels`, `glowIntensity` y `previousHazardTime` describen
los datos observados. Los campos `CarLightPoint::intensity` y los demás miembros
conservan sus nombres y layout.

Se conserva el orden de los getters y setters, incluida la salida aliased del
canal 4 (la segunda escritura reemplaza la primera), los flags enteros con signo,
el uso de la posición anterior del glow para elegir superficie y los umbrales
del temporizador. La llamada al callback vacío conserva su ABI de bytes.

La actualización mejora de **70,09 % a 73,84 %** en bytes y de **76,43 % a
77,75 %** en similitud estructural. El gate de las cuatro unidades y la medición
completa conservan los demás scores `s/fz/x`, las **2924 funciones exactas de
3364** y cero errores de datos. El reset de nodos permanece en 93,33 % de bytes
y 99,99 % estructural; no se fuerza su asignación de registros.

`differential_car_light_state.py` añade 512 ejecuciones completas del actualizador
contra el original, además de sus 5691 casos previos. Ejecuta los getters,
máscaras, niveles, conversión de vértices y setters reales; solo controla las
hojas de meteorología y superficie. Comprueba heap y registros completos, anchos de
escritura, ABI, glows ausentes, cuentas negativas/cero, coches 0/7, flags de
rotura no canónicos, canales con niveles negativos y overflow/umbrales del timer.

`check_64bit_car_glow_update.py` compara esos 512 casos con el original x86 usando
doce cuerpos de producción y headers reales en x64, punteros mayores de 4 GB y
ASan/UBSan. Traduce direcciones y spans de la fixture; el código de producción
opera sobre los layouts nativos de `Car`, `CarPartSet`, `Mesh`, `GlowLight` y
`CarLightTextureState`. Incluye empates de conversión 16.16, alias de canales y
guardas de punteros/padding además de las salidas completas.

La prueba nativa de piezas se amplía a 462 casos y catorce cuerpos, incluidos
los resets de daño, rangos de conjuntos, desactivación y nodos adjuntos/RNG.
El harness existente de resets de daño conserva sus 12000 casos contra el
original, incluidos proveedores que cambian la base durante el recorrido.
Los 136 harnesses diferenciales pasan sin fallos sobre el build medido.

Estas regresiones ejecutan el código x86 del ejecutable original y el recompilado
mediante Unicorn. Requieren `pefile`, `unicorn` y, para generar las direcciones,
las dependencias locales de reccmp/Wine. No se deben mezclar el mapa de símbolos
y un ejecutable de otra compilación.

Generar el mapa de la compilación actual desde la raíz del repositorio:

```sh
WINEPREFIX=/tmp/cmr2-main-integration/wineprefix python3 - <<'PYTHON'
import json, sys
sys.path.insert(0, "tests")
from matching_entities import load_entities
with open("/tmp/cmr2-entities.json", "w") as output:
    json.dump(load_entities(), output)
PYTHON
```

Ejecutar cada prueba con `python3 tests/differential_<nombre>.py
/tmp/cmr2-entities.json [build/CMR2.exe]`:

| Nombre | Casos | Comprobación |
| --- | ---: | --- |
| menu_list | 2664 | Llamadas de dibujo, primera fila, rectángulos y colores |
| car_basis | 300 | Bases completas del coche sobre terreno plano e inclinado |
| stage_grid | 42 | Los siete patrones de la cuenta atrás y sus rectángulos |
| ai_controls | 384 | Cinco canales, controles aplicados y estado del piloto |
| ai_network | 256 | Red neuronal completa, pesos, proyecciones y cinco salidas |
| ai_telemetry | 288 | Ángulos de ruta, interpolación y estado completo |
| car_spawn | 288 | Selección de ajustes y estado completo al colocar el coche |
| service_menus | 16 | Registros completos de menú, callbacks y ausencia de solapamientos |
| surface_collision | 208 | Los 26 tipos de superficie, impulsos, desplazamientos y efectos |
| car_contacts | 288 | Contadores y listas de los ocho coches y temporizadores de separación |
| menu_colour | 240 | El DWORD RGBA que leen los dibujantes, incluido el alfa |
| stage_paths | 2688 | Seis rutas y nombre final de la textura de carga, todos los países/etapas y variantes hi/lo |
| fireworks | 1200 | Memoria de los cohetes, destello, llamadas y consumo aleatorio en los cuatro modos de redondeo x87 |
| surface_params | 2304 | 48 superficies, mezclas, compresión y resistencia; helpers y tablas reales |
| track_geometry | 2220 | 1800 rayos/cuadriláteros y 420 recorridos del quadtree; comprobación independiente de hoja |
| net_car_state | 960 | Los 30 bytes del estado de coche, flags, contador de backfire y cuatro modos de redondeo x87 |
| car_route_update | 961 | Avance/retroceso, rutas abiertas/cerradas, caché y límite de 25 ciclos; helpers reales |
| net_car_receive | 4096 | Paquetes aceptados y descartados, registros y steering completos; helpers reales y cuatro modos de redondeo x87 |
| collision_face_decision | 7200 | Contadores unsigned, selección de cara, decisiones y ABI; modelo independiente y proveedores geométricos controlados |
| car_restore | 960 | Estado físico guardado, matriz y coche con guardas; helpers reales, dos ramas de suspensión y entrada aliada/separada |
| replay_snapshot | 1921 | Instantáneas de matriz, contadores, estado del coche y orden de consultas; carga de frames e interpolación controladas |
| object_matrix | 12288 | Dos entradas: ejes reflejados, tilt y posición/split con guardas; modelo afín/racional y todos los helpers reales |
| render_transforms | 1920 | Dos pasadas de matrices y ruedas, suspensión, listas repetidas y IDs distintos de slots; modelo independiente, guardas, helpers reales y cuatro modos x87 |
| auto_steering | 9984 | Fuerzas, rampa y ángulos con guardas, límites de 32/16 bits y cuatro modos x87; modelo entero/racional y helpers reales |
| contact_init | 2560 | Registros de contacto, cinco cachés de manejo, modos de carrera y slots intactos; helpers reales |
| body_patch | 13440 | Huella normal/ghost, inclinación y normal desconocida, umbral ±1 y 14 etapas; modelo entero independiente, helpers reales y memoria con guardas |
| race_estimates | 2160 | Orden/posición/tiempos con guardas, prefijos clasificados, empates y resumen; modelo entero independiente y helpers reales |
| race_handler | 11003 | Cinco tablas de modos/estados, etapas 0..11, reanudación y argumentos de salida; consultas reales y acciones finales controladas |
| race_order | 2592 | De cero a ocho coches, empates y pasadas repetidas; helpers reales y modelo independiente |
| checkpoint_advance | 3072 | Direcciones, vueltas, umbrales ±50, límite de recorrido y contadores; modelo independiente y eventos controlados |
| event_draw_abi | 11 | Cinco salidas tempranas, limpieza del argumento en pila y seis casos de noops de release; mutación de ret detectada |

Las pruebas controlan los proveedores de selección, ruta, geometría y las
operaciones finales de dibujo/impulso que se especifican en cada script. La
red neuronal se ejecuta completa. Comparan lógica y efectos observables en
fixtures válidos; no sustituyen una partida real ni prueban toda la física,
todos los archivos de instalación o todos los estados del juego.

`tests/logic-targets.json` registra las entradas principales comprobadas por los
68 harnesses disponibles. El proveedor final de triángulo más cercano de
`track_geometry` y las consultas de juego/secuencia/progreso de `net_car_state`
están controlados: esas funciones proveedoras no se cuentan como validadas.
`event_draw_abi` no comprueba las ramas de dibujo activo. En `replay_snapshot`,
las consultas de coche, carga de frames, eventos, geometría e interpolación
están controladas. En `checkpoint_advance`, los cinco helpers de actualización
y eventos están controlados: se comprueba su orden y argumentos, no sus
implementaciones. `auto_steering`, `object_matrix`, `car_restore`, `race_order`, `contact_init`
y `race_estimates` ejecutan sus helpers reales. Los casos de matriz usan cuatro
slots válidos, matrices afines y buffers distintos. Las estimaciones usan listas de pilotos únicos y distancias
distintas de cero; no prueban una carrera completa.

Para ejecutar todos los harnesses, conservar sus resultados con hashes de la
compilación y actualizar el inventario de lógica:

```sh
python3 tests/run_differential_suite.py --jobs 3
python3 scripts/audit_logic.py
```

El entorno debe tener `WINEPREFIX`, `CMR2_MSVC_ROOT` y `CMR2_TOOLS` configurados
como en la compilación y las pruebas existentes. El informe en
`CMR2PROGRESS/logic-tests.json` conserva salidas, fallos, duraciones y entradas
comprobadas; no se admite como evidencia vigente si cambian los binarios, el
mapa, el registro de pruebas o los harnesses.

Para registrar los destinos de ramas originales en los harnesses que cargan
la imagen completa en Unicorn, usar `--coverage-disassembly` con el JSON del
auditor. `scripts/validation_queue.py --disassembly ...` genera una cola por
función con cobertura y comprobaciones abiertas. Las pruebas nativas/Wine se
conservan como evidencia de fixtures, sin atribuirles cobertura instrumentada.
Véase `CMR2PROGRESS/validation-strategy.md` para criterios y limitaciones.

`rally_menu_selection` ejecuta el panel completo con el menú y las consultas
reales de juego/rally. Sus 756 casos distinguen tags 0/1, orden de elementos,
modo, país, etapa, selección y resolución; comprueban texto, posiciones,
colores, memoria inmutable, guardas y ABI. Detecta el preview incorrecto de
la compilación anterior. Texto/formato/render, clima y preview están controlados.

`net_car_receive` inicializa tablas sine/sqrt compartidas y ejecuta el receptor
y el helper de movimiento reales. Evita velocidades tan pequeñas que la división
del original desborde; no valida transporte ni sesiones completas.
`collision_face_decision` comprueba la decisión de la entrada y las llamadas,
con selección, transformación y clasificación geométrica controladas.

`render_transforms` usa ocho coches y setups válidos, tipos 0..13 y cantidades
-1..8. Distingue los inputs de ángulo (+0x240) de los de suspensión (+0x250),
compara matrices y ruedas completas, guardas, tablas intactas y consultas reales.
Detecta el offset incorrecto de suspensión de la compilación anterior; no
simula el renderizado ni el ciclo completo de física.

`race_handler` recorre todas las entradas de las cinco tablas originales de
modos/estados (11/5/13/7/4). Las consultas de juego/rally/fase y la entrega real
del puntero al contador se ejecutan en ambos binarios; el modelo comprueba
tiempo, fase, progreso y guardas. La comparación completa comprueba orden y
argumentos de acciones de teardown, vistas, replay y sonido/render controladas.
Incluye etapas 0..11, salidas tempranas, ambos finales de progreso en modo 5,
flags combinados, slots 0/1/2/8 y argumentos con bits altos. Detecta los defectos
de puntero/argumento y transiciones de la versión anterior; no valida las
implementaciones de esas acciones ni una carrera completa.

`stage_paths` también ejecuta el bloque final del cargador `0x41f930`. Sus nueve
casos distinguen los dos punteros de iluminación, incluidos nulos y alias:
comprueban que el primer argumento es el global `0x538238` y el segundo
`0x538234`, además del orden de las llamadas, retorno BYTE y pila. Las hojas
de inicialización se controlan para observar los argumentos. Esta regresión
fallaba con el orden de argumentos del ejecutable anterior.

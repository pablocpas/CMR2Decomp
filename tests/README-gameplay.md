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

Las pruebas controlan los proveedores de selección, ruta, geometría y las
operaciones finales de dibujo/impulso que se especifican en cada script. La
red neuronal se ejecuta completa. Comparan lógica y efectos observables en
fixtures válidos; no sustituyen una partida real ni prueban toda la física,
todos los archivos de instalación o todos los estados del juego.

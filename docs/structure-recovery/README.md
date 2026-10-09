# Recuperación de estructuras y campos

Objetivo activo: reconstruir los tipos, arrays y campos que puedan justificarse
con el ejecutable y sus usos, y expresar sus accesos mediante esos tipos.
El trabajo comprende todo el proyecto, incluidos los registros todavía
representados por buffers y los campos con nombres basados en direcciones.

Los nombres recuperados describen la función observada; no se presentan como
los identificadores del código fuente original. Un nombre provisional o un
campo sin semántica demostrada sigue siendo trabajo pendiente. También hay
que distinguir los bytes de relleno de los campos todavía desconocidos.

## Inventario reproducible

```sh
python3 scripts/audit_readability.py --json docs/structure-recovery/inventory.json
```

El inventario excluye comentarios y literales, conserva archivo, línea,
anotación de función más cercana y hashes del código analizado. Sus resultados
son candidatos sintácticos: un multiplicador hexadecimal puede ser una escala
numérica, un acceso crudo puede corresponder a un formato serializado y la
anotación más cercana no necesariamente pertenece a la declaración señalada.
No se usa la desaparición de `unk` como prueba de recuperación.

Punto de partida después de la primera limpieza:

| Indicador | Cantidad |
| --- | ---: |
| Tipos con definición nombrada | 287 |
| Declaraciones de campos sin nombre semántico detectadas | 1605 |
| Identificadores desconocidos distintos | 1457 |
| Desreferencias con aritmética explícita detectadas | 2237 |
| Tamaños literales en asignaciones/copias detectados | 129 |

El inventario completo se actualiza con cada tanda. Los indicadores iniciales
se conservan aquí para que reducirlos no borre la evidencia del trabajo pendiente.

## Criterios de recuperación

Para cada registro hay que establecer su tamaño, alineación, offsets, tipos,
signedness y cardinalidad de arrays a partir de reservas, inicialización,
copias, lectores y escritores. Los formatos serializados o compartidos por
varios usos requieren evidencia específica de sus variantes.

Una sustitución debe conservar el orden y ancho de los accesos, la aritmética,
los efectos y todas las coincidencias byte-exactas existentes. Los tamaños y
offsets establecidos se verifican en `CMR2Decomp/LayoutChecks.cpp`, evitando
añadir declaraciones de comprobación a cabeceras compartidas sensibles a MSVC6.

Cada tanda se compila y compara por archivo y después con el ejecutable entero.
Se ejecuta la suite diferencial registrada sobre el mismo build
medido. Una pérdida de exactitud obliga a ajustar o retirar la transformación.
Los recorridos por bytes que MSVC6 exige para reproducir el binario quedan
identificados como deuda concreta; no se ocultan detrás de nombres nuevos.

## Orden de trabajo y evidencia

| Bloque | Estado y siguiente trabajo |
| --- | --- |
| Coche, callbacks y jugadores de sesión | Véanse [auditoría inicial](../audits/2026-10-09/readability.md), [motor, controles y cambio](car-controls.md) [transforms/contactos](transforms-and-contact.md) y [recursos/vínculos](car-resources.md). Snapshots, contactos y recursos tipados; quedan campos del coche, perfiles serializados y registros de vista. |
| Piezas móviles, geometría y daños del coche | Recuperación en curso. Véanse [piezas](car-parts.md) y [líneas/snapshots](flexible-lines-and-damage.md). Quedan consumidores que tratan el conjunto como `int *`, cargas de impactos y campos de estado móvil. |
| Objetos de etapa y nodos de escena | Prioridad por volumen de accesos crudos; reconstruir las variantes y la propiedad de los buffers. |
| Tiempos, replay y estado de carrera | Distinguir registros persistentes, instantáneas y tablas de ranking antes de asignar tipos. |
| Información de juego y datos del rally | Rastrear opciones empaquetadas, tablas de ruta y registros por jugador. |
| Colisión, sectores y gráficos | Recuperar formatos de malla, árbol de colisión, nodos y vértices con tamaños y variantes verificados. |
| Resto de subsistemas | Recorrer el inventario completo: entrada, red, sonido, frontend, archivos y efectos. |

El objetivo sigue abierto hasta resolver el inventario de layouts y documentar
con evidencia los casos que no permitan una interpretación única.

Después de estas tandas, el inventario detecta 2078 desreferencias crudas y
1514 declaraciones de campos sin nombre semántico. Los recursos del coche
incluyen un registro de escena de 0x24 bytes, tablas independientes y vínculos
con nombres comprobados por sus escritores/lectores. La nueva prueba de
liberación añade 128 casos y lleva la suite registrada a 106 harnesses.
Los campos desconocidos y formatos de los buffers conservan su estado
pendiente. La siguiente investigación preparada es la de perfiles CIN,
registros de vista y variantes de los objetos/nodos de etapa.

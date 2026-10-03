# Matching manual y validación — 2026-10-03

La prioridad actual es resolver funciones individuales comparando el ensamblador
original y recompilado con MSVC6. Otro agente trabaja en funciones de bajo
matching en su propio worktree; este trabajo complementa esa búsqueda con
funciones de matching alto y pruebas existentes. Cada candidato se compila en
una copia aislada, y únicamente se integran los cuerpos revisados y verificados.

El inventario de lógica, las pruebas diferenciales y las revisiones de confianza
se mantienen como evidencia y regresiones. Ya no son una fase que deba terminar
para las 799 funciones del punto de partida antes de avanzar en matching. Las
permutaciones se reservan para diferencias concretas identificadas en ASM;
el lote genérico permanece detenido.

## Qué respaldan los proyectos de referencia

[LEGO Island](https://github.com/isledecomp/isle/blob/master/CONTRIBUTING.md)
prioriza reproducir instrucciones con el compilador original, revisar cambios
pequeños y comprobar sus efectos en otras funciones. Su documentación advierte
que el estado interno del compilador puede hacer que cambios aparentemente
ajenos alteren el código generado. Es una referencia relevante para nuestro
C++/x86/MSVC, aunque usa MSVC 4.2 y CMR2 usa MSVC 6.

[Super Mario 64](https://github.com/n64decomp/sm64/blob/master/README.md)
compara el hash de la ROM completa y distingue las implementaciones
funcionalmente equivalentes que aún no coinciden. El objetivo de matching
trasciende que el juego parezca funcionar.

[decomp-permuter](https://github.com/simonlindholm/decomp-permuter#faq)
recomienda la búsqueda al final, especialmente para diferencias de asignación
de registros. Las diferencias funcionales y de orden suelen necesitar revisión
manual; las mejoras aleatorias se deben revisar. Su soporte publicado es
MIPS, PowerPC y ARM32: no se puede asumir que sirva directamente para nuestro
C++/MSVC6/x86. Podemos adaptar sus ideas al compilador y comparador existentes.

Ninguna de estas fuentes establece que haya que demostrar formalmente la
lógica de todo un juego antes de trabajar en matching. La fase de validación
que adoptamos responde a la prioridad del usuario y a los defectos reales
encontrados en CMR2; las referencias respaldan mantener el compilador original,
comparar ensamblador, revisar cambios y usar el permutador como complemento.

## Qué significa confianza por función

| Evidencia | Qué permite afirmar | Qué sigue pendiente |
| --- | --- | --- |
| Inventario y revisión estática | Función identificada y señales para revisar | Equivalencia de comportamiento |
| Pruebas diferenciales | Original y recompilado coinciden en los casos ejecutados | Otros estados, límites, aliasing y secuencias |
| Ramas conocidas ejercitadas | Se observaron ambos destinos de los condicionales identificados | Combinaciones de caminos, bucles y tablas indirectas incompletas |
| Revisión de alta confianza | Evidencia de dominio, límites, dependencias y regresiones documentada | No es una demostración universal ni valida automáticamente los callees |
| Bytes locales exactos | Cuerpo de máquina equivalente con relocaciones resueltas | Dependencias, datos, entorno y comportamiento del programa completo |

Antes de cerrar una revisión de alta confianza se debe delimitar el dominio
válido y documentar los casos de cero/máximo, signos, wrap, aliasing y estados
relevantes. Se comparan retorno, memoria completa con guardas, efectos y orden
de llamadas; también ABI y precisión/rounding x87 cuando corresponda. Las
tablas de estados requieren casos explícitos, incluidos defaults. Una rama
inalcanzable necesita una justificación basada en el dominio, no un porcentaje.

Hay que indicar qué helpers se ejecutan realmente y cuáles se controlan.
Una hoja simulada verifica sus argumentos y orden, no su implementación.
Las regresiones deben detectar fallos representativos: por ejemplo un offset,
signo, argumento o transición incorrectos. Después se añaden secuencias reales
de ticks/replay/carrera/red para comprobar la composición entre funciones.

La comparación diferencial, incluso con todas las ramas conocidas, no prueba
todos los caminos ni todas las entradas. La equivalencia formal puede ser útil
en helpers pequeños de enteros, pero no sustituye automáticamente las pruebas
de 799 funciones con punteros, x87, callbacks y estado externo.

## Cómo avanzar más rápido

`validation-queue.json` y `validation-queue.tsv` incluyen **cada función
pendiente**, sus pruebas directas, ramas observadas y comprobaciones abiertas.
La prioridad favorece módulos críticos y helpers llamados por varias funciones
pendientes; el porcentaje de matching se conserva como información, no como
criterio de corrección. El grafo es de llamadas directas identificadas; no
incluye todas las llamadas indirectas ni dependencias de funciones ya exactas.

Las revisiones directas de `Car_UpdateCorners`, `FixMatrixMultiply` y
`Car_IntegrateBasisStep` delimitan entradas válidas, aliasing, signos y wrap,
y contienen regresiones sensibles a errores. Esas pruebas se conservan durante
la búsqueda manual. Un porcentaje alto no excluye un defecto: en el cargador
`0x41f930`, al 91,99%, el ASM reveló argumentos de iluminación invertidos.

La nueva instrumentación ejecuta los harnesses existentes y registra destinos
de saltos en el **binario original** cargado íntegramente en Unicorn. El
ejecutable recompilado se sigue contrastando mediante los resultados de la
prueba. Las ejecuciones nativas/Wine no reciben cobertura de ramas de este
instrumento. Las funciones sin cobertura medible se marcan como tales.
Las tres revisiones explícitas de `confidence-reviews.json` son válidas solo
para el dominio y los hashes que documentan. La cola comprueba esas revisiones;
no inicia búsquedas ni convierte la cobertura de ramas en una demostración.

```sh
python3 scripts/audit_logic.py --disassembly /tmp/cmr2-disassembly.json
python3 tests/run_differential_suite.py --jobs 3 --coverage-disassembly /tmp/cmr2-disassembly.json
python3 scripts/audit_logic.py --disassembly /tmp/cmr2-disassembly.json
python3 scripts/validation_queue.py --disassembly /tmp/cmr2-disassembly.json
```

Estos comandos presuponen compilación, medición e informe diferencial vigentes;
si se ha cambiado el registro o un harness, primero se ejecuta la suite sin
cobertura para refrescar la evidencia que exige el auditor. Los informes se
vinculan mediante hashes de binarios, fuentes, registro y pruebas.

## Permutaciones como complemento de la revisión manual

El código de búsqueda por lotes se conserva, pero no se ejecuta como estrategia
principal. Después de identificar una diferencia en ASM, se prueban variantes
concretas de tipos, temporales, orden de sentencias o forma de la expresión
en candidatos aislados. La búsqueda de mesetas y varias representaciones prometedoras puede
añadirse cuando el buscador y las regresiones estén preparados; no hace falta
introducir cruces arbitrarios de cuerpos de función.

Una mutación que cambie el comportamiento se rechaza aunque suba el matching.
Primero se comprueba compilación y comportamiento del candidato; después se
usa la diferencia de instrucciones como puntuación y se verifica el resultado
combinado con toda la suite. Se conserva la semilla, el fuente, hashes y motivos
de aceptación. El destino es equivalencia exacta, no maximizar un porcentaje
a costa de reproducir peor el original. Las reglas textuales de seguridad por
sí solas no demuestran independencia, aliasing ni equivalencia de x87.

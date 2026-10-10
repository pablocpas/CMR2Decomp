# Contexto nativo de la API de rectángulos

Tanda sobre `55143d4`. Se convierten en conjunto `Sprite_FillRect`
(`0x4a5e40`), `StageObject_FillWidthScaledRectangle` (`0x475970`), todas
sus declaraciones repetidas y **315 llamadas en 16 archivos**.

## Evidencia y layout

313 llamadas a `Sprite_FillRect` y dos al wrapper entregaban como primer
argumento `(int)g_pGraphics + 0x150`. La función no consume ese argumento;
el wrapper lo transmite sin leerlo y retorna el resultado de la hoja.
Es una dirección, nunca un id numérico. El tipo pasa de `int` a `BYTE *`.
La dirección se obtiene con `&g_pGraphics->field309_0x150`, miembro que ya
existe en el header. Su semántica sigue desconocida: no se inventa un
nombre ni se reconstruye un struct adicional para un argumento ignorado.

| Registro Graphics, Win32 | Comprobación central |
| --- | --- |
| `sizeof == 0x3cc` | Extensión actual del registro |
| `resX +0x0 / resY +0x4` | Dimensiones que usa el renderer |
| `field309_0x150 +0x150` | Dirección que se transmite a esta API |

Graphics es un registro de ejecución con referencias nativas DirectDraw.
La superficie situada antes del contexto ensancha/desplaza el miembro
en x64. La expresión anterior truncaba además la dirección completa.
Se corrigen ambos problemas, sin cambiar superficies, ventanas, rendering
ni formatos en disco. No se cambia el layout declarado de Graphics.

## Alcance y comprobación de comportamiento

La firma nueva exige actualizar los nombres decorados y todas las
declaraciones/callers. Los 313 callers directos y los dos del wrapper
mantienen exactamente el orden de argumentos y expresiones. Los rects
siguen siendo buffers de cuatro shorts con signo; esta tanda no cambia
su representación ni los casts de colores de otras familias.

Los **18 cuerpos compilados de Sprite.cpp** se comparan con el objeto
del punto estable antes del build: sus instrucciones y bytes reubicados
son idénticos. Esto verifica que cambiar el tipo del argumento ignorado
no cambia la lógica previa, además de la comprobación de scores global.

El nuevo arnés `differential_sprite_rectangle_context.py` ejecuta el
wrapper completo en original y reconstruido: **960 casos**, 20 rects,
seis layers y ocho escalas, incluyendo extremos y valores negativos.
Compara un modelo signed 32/16 bits de FixMul, dirección de contexto,
rect, color, layer, retorno de 32 bits de la hoja, heap íntegro, writes
globales y ABI. `Sprite_FillRect` es una hoja controlada; no se afirma
validar su recorte con este arnés.

`check_64bit_sprite_context.py` ejecuta el cuerpo actual del wrapper
con el **Graphics actual** y la firma actual del renderer. Las referencias
al Graphics y al miembro superan 4 GB, el offset del miembro ya no es
`0x150` y el proveedor comprueba que la dirección completa llega intacta.
Son los mismos 960 escenarios bajo ASan/UBSan, con entradas intactas y
retornos arbitrarios propagados. DirectDraw es opaco y FixMul/dibujo son
hojas del fixture; el arnés Win32 conserva las operaciones reales del wrapper.

## Deuda de recorte encontrada, excluida de esta tanda

El original tiene un `dec edi` a `0x4a5f04` **dentro** de la rama de
recorte de altura (`jle 0x4a5f05` en `0x4a5efa`). El fuente previo hace
`h -= 1` de forma incondicional. Un modelo independiente ejecutado contra
el original en 3240 casos reprodujo esa rama, incluido el uso original
de **resX** al recortar altura. Es una diferencia previa del decompilado,
no un bug del original que se deba corregir intencionadamente.

Se probaron bloque condicional, asignación compuesta, vista SpriteRect,
promoción/estrechamiento de temporales, temporales separados y extremos
guardados en locales. El bloque correcto bajó s de 25,60 a 24,80 % y
fz de 74,40 a 73,60 %. Un temporal short de altura mejoró s a 27,89 %
pero bajó fz a 69,32 %; también se rechaza. Las variantes de nombres
con semántica demostrada no resolvieron ambas métricas. **Todas esas
ediciones están descartadas**: no entran cambios de recorte ni nombres.
La reparación queda pendiente bajo la misma condición de no regresión.

## Validación y siguiente alcance

El gate de **23 unidades** pasa. Build y medición completos,
`prepare_fastcmp.py` y verificación de manifest/procedencia/hashes:
las **3364 filas de `bytes.json` son idénticas**, con **2922 byte-exactas**,
perfect 56,10 %, fuzzy 94,50 % y cero incidencias de datos. La suite
completa pasa: **115 harnesses, cero fallos**. Los resultados se guardan
junto a este documento.

Se eliminan **315 conversiones de dirección a int** y sus offsets
literales `+0x150`. El inventario de desreferencias crudas sigue en
**1489**: ese detector cuenta desreferencias por offset, no direcciones
de argumentos. No se modifica el detector para reducir artificialmente
la cifra. El objetivo global de eliminar las desreferencias pendientes,
tipar sus registros y documentar las excepciones sigue abierto.

```sh
python3 tests/differential_sprite_rectangle_context.py CMR2PROGRESS/entities.json
python3 tests/check_64bit_sprite_context.py
```

#!/usr/bin/env python3
"""check_dupes.py — busca simbolos DUPLICADOS en las anotaciones de CMR2Decomp.

Tres fallos que hoy (28/09/2026) han costado horas y que NO dan error de compilacion ni de enlazado:

  1. Dos definiciones de la misma funcion con firmas distintas -> C++ les da nombres manglados distintos,
     conviven, y reccmp mide la PRIMERA y descarta la segunda ("Dropped duplicate address"). Caso real:
     0x447530, donde el 56% que reportabamos pertenecia a una version SIN LLAMADORES y la real estaba al 29,79%.
  2. Un `// STUB:` y un `// FUNCTION:` para la misma direccion -> el stub ensombrece el cuerpo real. Caso real:
     0x426fc0, que paso de 78,89% a 0% sin que el enlazador dijera nada.
  3. Dos `// GLOBAL:` para la misma direccion -> el segundo se descarta en silencio. Caso real: un global
     duplicado en un bloque recuperado que tapaba al existente (0x8188a4).

Uso:  python3 tools/check_dupes.py [ruta_al_repo]
Salida: 0 si limpio, 1 si encuentra algo. Pensado para meterlo en check.sh.
"""
import collections
import glob
import os
import re
import sys

# CMR2_REPO = RAIZ del repo (la que contiene CMR2Decomp/). Si tools/ es un symlink a un tools/
# compartido entre worktrees, la deduccion por $TOOLS/.. apunta al arbol equivocado: por eso el
# guard de abajo falla ruidosamente en vez de reportar "limpio" con 0 funciones.
REPO = sys.argv[1] if len(sys.argv) > 1 else os.environ.get('CMR2_REPO') or \
    os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
SRC = os.path.join(REPO, 'CMR2Decomp')

fn = collections.defaultdict(list)
st = collections.defaultdict(list)
gl = collections.defaultdict(list)

for f in sorted(glob.glob(SRC + '/*.cpp') + glob.glob(SRC + '/*.h')):
    base = os.path.basename(f)
    for i, line in enumerate(open(f, errors='replace'), 1):
        m = re.search(r'//\s*FUNCTION:\s*CMR2\s*0x([0-9a-fA-F]+)', line)
        if m:
            fn[m.group(1).lower()].append((base, i))
        m = re.search(r'//\s*STUB:\s*CMR2\s*0x([0-9a-fA-F]+)', line)
        if m:
            st[m.group(1).lower()].append((base, i))
        m = re.search(r'//\s*GLOBAL:\s*CMR2\s*0x([0-9a-fA-F]+)', line)
        if m:
            gl[m.group(1).lower()].append((base, i))

bad = 0

dup_fn = {a: v for a, v in fn.items() if len(v) > 1}
if dup_fn:
    bad = 1
    print("== FUNCTION anotada mas de una vez (reccmp medira solo la primera):")
    for a, v in sorted(dup_fn.items()):
        print(f"   0x{a}: " + ", ".join(f"{f}:{l}" for f, l in v))

both = sorted(set(st) & set(fn))
if both:
    bad = 1
    print("== STUB y FUNCTION para la misma direccion (el stub ensombrece el cuerpo):")
    for a in both:
        print(f"   0x{a}: STUB {st[a]}  FUNCTION {fn[a]}")

# GLOBAL duplicado: es LEGITIMO cuando uno de los dos es un extern en cabecera. Solo avisamos si los dos
# son .cpp (dos definiciones reales) o si ambos estan en el mismo fichero.
suspicious = {}
for a, v in gl.items():
    if len(v) < 2:
        continue
    cpps = [x for x in v if x[0].endswith('.cpp')]
    if len(cpps) > 1 or len({x[0] for x in v}) < len(v):
        suspicious[a] = v
if suspicious:
    bad = 1
    print("== GLOBAL anotado mas de una vez en ficheros de implementacion (el segundo se descarta):")
    for a, v in sorted(suspicious.items()):
        print(f"   0x{a}: " + ", ".join(f"{f}:{l}" for f, l in v))

if len(fn) == 0:
    print(f"check_dupes: NO ENCUENTRO FUENTES en {SRC}")
    print("  (exporta CMR2_REPO apuntando a la RAIZ del repo, la que contiene CMR2Decomp/)")
    sys.exit(2)

if not bad:
    print(f"check_dupes: limpio ({len(fn)} funciones, {len(st)} stubs, {len(gl)} globales anotados)")
sys.exit(bad)

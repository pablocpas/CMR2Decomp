#!/usr/bin/env python3
"""fastcmp.py: compile ONE translation unit and compare one function byte-for-byte
with the original exe, in ~1 s (vs ~27 s for build + reccmp).

  fastcmp.py 0xADDR [--src FILE.cpp] [--diff] [--obj OUT.obj] [--nocompile]

Relocations in the fresh .obj are translated to ORIGINAL addresses using a
symbol map learned from the last full build (build/*.obj vs build/CMR2.exe,
then recomp->orig through ent.json). The patched recompiled bytes are then
compared with the original bytes: equal bytes == true byte-exact match.
Score = difflib ratio over the disassembled instruction texts.
"""
from collections import Counter
import sys, os, re, json, struct, subprocess, difflib, pickle, argparse, tempfile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

REPO = os.environ.get('CMR2_REPO', os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TOOLS = os.path.dirname(os.path.abspath(__file__))
HERE = os.environ.get('FASTCMP_WORK', os.path.dirname(os.path.abspath(__file__)) + '/work')
MSVC = os.environ.get('CMR2_MSVC_ROOT') or next(
    (p for p in (REPO + '/msvc600/VC98', os.path.dirname(REPO) + '/msvc600/VC98') if os.path.isdir(p)),
    REPO + '/msvc600/VC98')
QIFIST = set("Race.cpp StageUI.cpp TimingUtils.cpp Frontend.cpp FrontendScreens.cpp Game.cpp GameInfo.cpp Graphics.cpp Sprite.cpp Car.cpp Sound.cpp CarPhysics.cpp HudDash.cpp CarEffects.cpp TrackCollision.cpp RallyData.cpp Mesh.cpp Sector.cpp StageTiming.cpp StageObjects.cpp SceneNode.cpp FixedPoint.cpp RallyTiming.cpp NetRace.cpp".split())
CRT_O1 = set("CrtTypeInfo.cpp".split())
IMGBASE = 0x400000

# ---------------------------------------------------------------- PE / COFF
class PE:
    def __init__(self, path):
        self.d = open(path, 'rb').read()
        pe = struct.unpack_from('<I', self.d, 0x3c)[0]
        nsec = struct.unpack_from('<H', self.d, pe + 6)[0]
        opt = struct.unpack_from('<H', self.d, pe + 20)[0]
        self.base = struct.unpack_from('<I', self.d, pe + 24 + 28)[0]
        self.secs = []
        o = pe + 24 + opt
        for i in range(nsec):
            name, vs, va, rs, rp = struct.unpack_from('<8sIIII', self.d, o + 40 * i)
            self.secs.append((va + self.base, max(vs, rs), rp, rs))
    def read(self, addr, n):
        for va, vs, rp, rs in self.secs:
            if va <= addr < va + vs:
                off = addr - va
                b = self.d[rp + off: rp + min(off + n, rs)]
                return b + b'\0' * (n - len(b))
        raise KeyError(hex(addr))
    def text(self):
        va, vs, rp, rs = self.secs[0]
        return va, self.d[rp:rp + rs]

class COFF:
    def __init__(self, path):
        d = self.d = open(path, 'rb').read()
        mach, nsec, _, symp, nsym, opt, _ = struct.unpack_from('<HHIIIHH', d, 0)
        self.secs = []
        for i in range(nsec):
            o = 20 + opt + 40 * i
            name = d[o:o + 8].rstrip(b'\0').decode('latin1')
            sz = struct.unpack_from('<I', d, o + 16)[0]
            rp, relp = struct.unpack_from('<II', d, o + 20)
            nrel = struct.unpack_from('<H', d, o + 32)[0]
            rels = [struct.unpack_from('<IIH', d, relp + 10 * k) for k in range(nrel)]
            self.secs.append(dict(name=name, data=d[rp:rp + sz] if rp else b'', rels=rels))
        strt = symp + 18 * nsym
        self.syms = []
        k = 0
        while k < nsym:
            o = symp + 18 * k
            nm = d[o:o + 8]
            if nm[:4] == b'\0\0\0\0':
                so = struct.unpack_from('<I', nm, 4)[0]
                nm = d[strt + so: d.index(b'\0', strt + so)]
            else:
                nm = nm.rstrip(b'\0')
            val, sec, typ, cls, naux = struct.unpack_from('<IhHBB', d, o + 8)
            self.syms.append(dict(name=nm.decode('latin1'), val=val, sec=sec, typ=typ, cls=cls))
            for _ in range(naux): self.syms.append(None)
            k += 1 + naux

# ------------------------------------------------------------- symbol map
def ent_map():
    """recomp addr -> orig addr for every matched entity of the last full build."""
    p = HERE + '/ent.json'
    rows = json.load(open(p))
    return {r['r']: r['o'] for r in rows}

def plain(mangled):
    """'?Name@Cls@@...' -> 'Cls::Name', '_Name@8' -> 'Name'."""
    if mangled.startswith('?') and not mangled.startswith('??'):
        head = mangled[1:].split('@@')[0].split('@')
        return '::'.join(reversed(head[1:])) + ('::' if len(head) > 1 else '') + head[0]
    if mangled.startswith('_'):
        return mangled[1:].split('@')[0]
    return mangled

_NM = None
def name_maps():
    """plain name -> (orig, recomp) for names that are unique in ent.json."""
    global _NM
    if _NM is None:
        cnt = Counter(); m = {}
        for r in json.load(open(HERE + '/ent.json')):
            n = r['n']
            if not n: continue
            cnt[n] += 1; m[n] = (r['o'], r['r'])
        _NM = {n: v for n, v in m.items() if cnt[n] == 1}
    return _NM

def text_sections(c):
    return [i for i, s in enumerate(c.secs) if s['name'] == '.text' and s['data']]

def learn_symbols(objpath, exe, r2o, cache={}):
    """For every symbol a .text section of the full-build obj relocates against,
    find its recomp address in the linked exe and translate it to orig."""
    c = COFF(objpath)
    tva, tdata = exe.text()
    votes = {}
    for si in text_sections(c):
        s = c.secs[si]; data = s['data']
        mask = bytearray(len(data))
        for off, sym, typ in s['rels']:
            for j in range(4): mask[off + j] = 1
        # anchor: longest reloc-free run
        best = (0, 0); st = None
        for i in range(len(data) + 1):
            if i < len(data) and not mask[i]:
                if st is None: st = i
            else:
                if st is not None and i - st > best[1] - best[0]: best = (st, i)
                st = None
        a, b = best
        # anchor by the section's own function symbol when reccmp knows it
        fsym = [x for x in c.syms if x and x['sec'] == si + 1 and x['val'] == 0 and x['cls'] == 2]
        nm = name_maps()
        if fsym and plain(fsym[0]['name']) in nm:
            hits = [nm[plain(fsym[0]['name'])][1]]
        elif b - a < 8: continue
        else:
          hits = []
          pat = data[a:min(b, a + 64)]
          pos = tdata.find(pat)
          while pos >= 0 and len(hits) < 2:
            cand = pos - a
            ok = all(mask[j] or tdata[cand + j] == data[j] for j in range(len(data))) if cand >= 0 else False
            if ok: hits.append(tva + cand)
            pos = tdata.find(pat, pos + 1)
        if len(hits) != 1: continue
        base = hits[0]
        eb = exe.read(base, len(data))
        if any(not mask[j] and eb[j] != data[j] for j in range(len(data))): continue
        for off, sym, typ in s['rels']:
            name = c.syms[sym]['name']
            if c.syms[sym]['cls'] == 3 and c.syms[sym]['sec'] > 0:  # static/section symbol: key by name+sec? skip
                name = None
            if name is None: continue
            field = struct.unpack_from('<i', data, off)[0]
            v = struct.unpack_from('<i', exe.read(base + off, 4))[0]
            if typ == 6:    # DIR32
                r = (v - field) & 0xffffffff
            elif typ == 20:  # REL32
                r = (base + off + 4 + v - field) & 0xffffffff
            else: continue
            if r in r2o: votes.setdefault(name, Counter())[r2o[r]] += 1
    return {n: v.most_common(1)[0][0] for n, v in votes.items()}

# ---------------------------------------------------------------- compile
def compile_tu(src, objout):
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=os.environ.get('WINEPREFIX', TOOLS + '/wineprefix'))
    W = lambda p: 'Z:' + os.path.abspath(p).replace('/', '\\')
    env['INCLUDE'] = W(MSVC + '/Include') + ';' + W(REPO + '/third_party/dx7sdk-7001/include')
    env['WINEPATH'] = W(MSVC + '/Bin') + ';' + W(os.path.dirname(MSVC) + '/Common/MSDev98/Bin')
    b = os.path.basename(src)
    fl = ['/nologo', '/c', '/O2', '/DNDEBUG', '/D_CRTIMP=', '/Gz', '/MD', '/GX']
    qifist = b in QIFIST or (os.environ.get('FASTCMP_QIFIST_ALL') and not b.startswith('Zlib'))
    if os.environ.get('FASTCMP_TOGGLE_QIFIST'): qifist = not qifist   # flag experiments
    if qifist: fl.append('/QIfist')
    if b in CRT_O1: fl.append('/O1')   # runtime-library code, built for size
    if b.startswith('Zlib'): fl.append('/Ob2')
    if b.startswith('Zlib') and b != 'ZlibZutil.cpp': fl.append('/TC')   # zlib was built as C (Rich header: C objects)
    fl += os.environ.get('FASTCMP_EXTRA', '').split()
    fl.append('/Zi'); fl.append('/Fd' + W(objout[:-4] + '.pdb'))
    fl.append('/I' + W(REPO + '/CMR2Decomp'))   # temp copies live outside the tree
    # A failed compiler invocation must not reuse an object from an earlier run.
    if os.path.exists(objout):
        os.unlink(objout)
    # Redirect to a file, not a pipe: the wineserver keeps inherited pipe ends
    # open after CL.EXE exits, which deadlocks communicate() forever.
    timeout = float(os.environ.get('FASTCMP_TIMEOUT', '180'))
    with tempfile.TemporaryFile('w+') as log:
        try:
            r = subprocess.run(['wine', MSVC + '/Bin/CL.EXE'] + fl + ['/Fo' + W(objout), W(src)],
                               stdout=log, stderr=subprocess.STDOUT, text=True, env=env,
                               cwd=REPO, timeout=timeout)
            status = r.returncode
        except subprocess.TimeoutExpired:
            status = f'timeout after {timeout:g}s'
        log.seek(0)
        out = log.read()
    errs = [l for l in out.splitlines() if ' error ' in l or 'fatal error' in l]
    if status or errs or not os.path.exists(objout):
        raise RuntimeError('\n'.join(errs[:10]) or out[-2000:] or
                           f'Compiler exited with status {status}; no fresh object produced.')

# ------------------------------------------------------------ function lookup
def mangled_candidates(name):
    parts = name.split('::')
    fn = parts[-1]; cls = parts[:-1]
    pre = '?' + fn + '@' + ''.join(p + '@' for p in reversed(cls)) + '@'
    return pre, '_' + fn + '@', '_' + fn

def find_func(c, name):
    pre, s1, s2 = mangled_candidates(name)
    for i, s in enumerate(c.syms):
        if not s or s['sec'] <= 0: continue
        n = s['name']
        if n.startswith(pre) or n.startswith(s1) or n == s2:
            if c.secs[s['sec'] - 1]['name'] == '.text':
                return i, s
    return None, None

def func_extent(c, sym):
    """[start,end) in its section: up to the next function symbol (keeps switch tables)."""
    sec = sym['sec']; st = sym['val']
    nxt = len(c.secs[sec - 1]['data'])
    for s in c.syms:
        if s and s['sec'] == sec and s['val'] > st and (s['typ'] & 0x20 or s['cls'] == 2):
            nxt = min(nxt, s['val'])
    return st, nxt

# ------------------------------------------------------------------ compare
md = Cs(CS_ARCH_X86, CS_MODE_32)
def dis(code, addr):
    out = []
    for i in md.disasm(code, addr):
        out.append((i.address, i.size, f"{i.mnemonic} {i.op_str}".strip()))
    return out

def load_meta():
    names = {}
    for e in json.load(open(HERE + '/cur.json'))['data']:
        names[int(e['address'], 16)] = e['name']
    sizes = {}
    for l in open(TOOLS + '/functions.tsv').readlines()[1:]:
        f = l.split('\t'); sizes[int(f[0], 16)] = int(f[1])
    return names, sizes

SRCMAP = None
def src_for(addr):
    tag = f'CMR2 0x{addr:08x}'
    for fn in sorted(os.listdir(REPO + '/CMR2Decomp')):
        if fn.endswith('.cpp') and not fn.startswith('.'):
            if tag in open(REPO + '/CMR2Decomp/' + fn, encoding='latin1').read():
                return REPO + '/CMR2Decomp/' + fn
    raise KeyError(hex(addr))

def symmap_for(src):
    """cached learned symbol map for this TU (from the last full build)."""
    b = os.path.basename(src)[:-4]
    cp = HERE + f'/symcache/{b}.pkl'
    objp = REPO + f'/build/{b}.obj'
    if not os.path.exists(objp):   # a TU added since the last full build
        return {}
    st = (os.path.getmtime(objp), os.path.getmtime(REPO + '/build/CMR2.exe'), os.path.getmtime(HERE + '/ent.json'))
    if os.path.exists(cp):
        d = pickle.load(open(cp, 'rb'))
        if d['st'] == st: return d['m']
    exe = PE(REPO + '/build/CMR2.exe')
    m = learn_symbols(objp, exe, ent_map())
    os.makedirs(HERE + '/symcache', exist_ok=True)
    pickle.dump(dict(st=st, m=m), open(cp, 'wb'))
    return m

ORIG = None
# FASTCMP_SYMS="g_name=0x5894b8,..." maps data symbols that the last full build does not have yet
EXTRA_SYMS = {k: int(v, 0) for k, v in (e.split('=') for e in os.environ.get('FASTCMP_SYMS', '').split(',') if e)}

_ANNOT = None
def annotated_globals():
    """name -> original address from the `// GLOBAL: CMR2 0x...` annotations, so a
    global added or renamed since the last full build still resolves."""
    global _ANNOT
    if _ANNOT is None:
        annotations = {}
        pat = re.compile(r'//\s*GLOBAL:\s*CMR2\s+(0x[0-9a-fA-F]+)[^\n]*\n((?:[ \t]*//[^\n]*\n)*)([^\n]*)')
        for fn in sorted(os.listdir(REPO + '/CMR2Decomp')):
            if not fn.endswith(('.cpp', '.h')): continue
            with open(REPO + '/CMR2Decomp/' + fn, encoding='latin1') as source:
                text = source.read()
            for m in pat.finditer(text):
                decl = re.split(r'[\[=;(]', m.group(3))[0]
                ids = re.findall(r'[A-Za-z_][\w:]*', decl)
                if ids and ids[-1] not in ('extern', 'static', 'const'):
                    annotations.setdefault(ids[-1], int(m.group(1), 16))
        # match.py compares TUs concurrently. Publish only the complete map:
        # another worker must never resolve against a partially scanned tree.
        _ANNOT = annotations
    return _ANNOT

def real_bytes(nm):
    """__real@<size>@<80-bit hex>  ->  the float/double bytes of the constant"""
    m = re.match(r'__real@(4|8)@([0-9a-f]{20})$', nm)
    if not m: return None
    v = int(m.group(2), 16)
    sign = v >> 79; exp = (v >> 64) & 0x7fff; mant = v & ((1 << 64) - 1)
    if exp == 0 and mant == 0: val = 0.0
    else: val = mant / float(1 << 63) * 2.0 ** (exp - 16383)
    if sign: val = -val
    return struct.pack('<f' if m.group(1) == '4' else '<d', val)

CRT = ('__alldiv', '__allrem', '__allmul', '__allshr', '__allshl', '__aulldiv', '__aullrem',
       '__aullshr', '__chkstk', '__ftol', '__CIsqrt', '__CIatan2', '__CIasin', '__CIacos', '__CIcos', '__CIsin')

def resolve_extra(nm, typ, code, o, addr):
    """symbols missing from the learned map: named addresses, float constants by
    value, CRT helpers and imports by trusting the original's operand."""
    m = re.search(r'g_unk0x([0-9a-f]{8})', nm)
    if m and not nm.startswith('_?'):
        return int(m.group(1), 16) + (struct.unpack_from('<i', code, o)[0] if typ == 6 else 0)
    ob = ORIG.read(addr + o, 4)
    ov = struct.unpack('<I', ob)[0]
    rb = real_bytes(nm)
    if rb is not None and typ == 6:
        try:
            if ORIG.read(ov, len(rb)) == rb: return ov
        except Exception: pass
        return None
    if nm.lstrip('_') in [c.lstrip('_') for c in CRT] or nm.startswith(('_Direct', '_IID_', '__imp_')):
        if typ == 20: return (addr + o + 4 + struct.unpack('<i', ob)[0]) & 0xffffffff
        return ov
    return None
FILLERS = (bytes.fromhex('8da42400000000'), bytes.fromhex('8d642400'), bytes.fromhex('8d4900'),
           bytes.fromhex('8bff'), b'\x90', b'\xcc')
def strip_pad(b):
    while True:
        for f in FILLERS:
            if b.endswith(f): b = b[:-len(f)]; break
        else: return b
def compare(addr, obj, src, name, osize, verbose=False, coff=None):
    global ORIG
    if ORIG is None: ORIG = PE(REPO + '/cmr2bin/CMR2.exe')
    # Batch callers reuse the parsed object; relocation writes use a fresh copy.
    c = COFF(obj) if coff is None else coff
    si, sym = find_func(c, name)
    if sym is None: raise KeyError('symbol not found for ' + name)
    st, en = func_extent(c, sym)
    sec = c.secs[sym['sec'] - 1]
    code = bytearray(sec['data'][st:en])
    smap = symmap_for(src)
    unknown = set()
    for off, s, typ in sec['rels']:
        if not (st <= off < en): continue
        o = off - st
        field = struct.unpack_from('<i', code, o)[0]
        ss = c.syms[s]; nm = ss['name']
        if ss['sec'] == sym['sec'] and ss['cls'] in (3, 6):   # label in own section
            tgt = addr + ss['val'] - st + field if typ == 6 else None
        elif plain(nm) in EXTRA_SYMS:
            tgt = EXTRA_SYMS[plain(nm)] + field if typ == 6 else EXTRA_SYMS[plain(nm)]
        elif nm in smap:
            tgt = smap[nm] + field if typ == 6 else smap[nm]
        elif plain(nm) in name_maps():
            o_ = name_maps()[plain(nm)][0]
            tgt = o_ + field if typ == 6 else o_
        elif plain(nm) in annotated_globals():
            o_ = annotated_globals()[plain(nm)]
            tgt = o_ + field if typ == 6 else o_
        else:
            tgt = resolve_extra(nm, typ, code, o, addr)
            if tgt is None and typ == 6 and ss['sec'] > 0:
                # data defined in this obj (string literals, constants): same bytes?
                dsec = c.secs[ss['sec'] - 1]
                blob = dsec['data'][ss['val']:ss['val'] + 64] if dsec['data'] else b''
                if blob and not dsec['rels']:
                    if nm.startswith('??_C@'):
                        blob = blob[:blob.index(b'\0') + 1] if b'\0' in blob else blob
                    ov = struct.unpack('<I', ORIG.read(addr + o, 4))[0] - field
                    try:
                        if len(blob) >= 2 and ORIG.read(ov, len(blob)) == blob: tgt = ov + field
                    except Exception: pass
            if tgt is None:
                unknown.add(nm); tgt = 0xEE000000 + (hash(nm) & 0xffff) * 16
        if typ == 6: struct.pack_into('<I', code, o, tgt & 0xffffffff)
        elif typ == 20:
            rel = (tgt - (addr + o + 4)) & 0xffffffff
            struct.pack_into('<I', code, o, rel)
    # code ends where the first forward-referenced table (switch) starts
    cend = len(code)
    for off, s_, typ in sec['rels']:
        ss = c.syms[s_]
        if typ == 6 and st <= off < en and ss['sec'] == sym['sec'] and ss['cls'] in (3, 6):
            t = ss['val'] + struct.unpack_from('<i', sec['data'], off)[0] - st
            if off - st < t < cend: cend = t
    # Disassemble instructions separately, but exactness includes switch data.
    full_rebuilt = strip_pad(bytes(code))
    code = code[:cend]
    rb = bytes(code)
    # trailing padding (int3/nop/alignment fillers) is not part of the function
    rb_t = strip_pad(rb)
    # COFF alignment may exceed the whole original function. Reading that
    # padding's length from the original can pull in its next function.
    # Keep all actual rebuilt instructions, including a longer wrong body.
    osz = max(osize or 0, len(full_rebuilt))
    full_original = ORIG.read(addr, osz)
    ob_t = strip_pad(full_original[:max(osize or 0, len(rb_t))])
    exact = full_rebuilt == strip_pad(full_original)
    oi = dis(ob_t, addr); ri = dis(rb_t, addr)
    ot = [t for _, _, t in oi]; rt = [t for _, _, t in ri]
    sm = difflib.SequenceMatcher(None, ot, rt, autojunk=False)
    score = 1.0 if exact else min(sm.ratio(), 0.9999)
    if verbose:
        print(f"{name} @{addr:#x}: orig {len(ob_t)}B/{len(ot)}i  ours {len(rb_t)}B/{len(rt)}i  score {100*score:.2f}%{'  EXACT' if exact else ''}")
        if rb_t == ob_t and not exact: print("  instruction bytes agree; trailing switch data differ")
        if unknown: print("  unmapped symbols:", ', '.join(sorted(unknown))[:300])
    return score, exact, (oi, ri, sm), unknown

def show_diff(oi, ri, sm, ctx=2, full=False):
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == 'equal' and not full:
            n = i2 - i1
            if n > 2 * ctx:
                for k in range(ctx): print(f"  {oi[i1+k][2]:46.46} | {ri[j1+k][2]}")
                print(f"   ... {n - 2*ctx} equal ...")
                for k in range(n - ctx, n): print(f"  {oi[i1+k][2]:46.46} | {ri[j1+k][2]}")
                continue
        for k in range(max(i2 - i1, j2 - j1)):
            a = oi[i1 + k][2] if i1 + k < i2 else ''
            b = ri[j1 + k][2] if j1 + k < j2 else ''
            print(f"{' ' if tag == 'equal' else '*'} {a:46.46} | {b}")

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('addr'); ap.add_argument('--src'); ap.add_argument('--obj')
    ap.add_argument('--diff', action='store_true'); ap.add_argument('--full', action='store_true')
    ap.add_argument('--nocompile', action='store_true')
    a = ap.parse_args()
    addr = int(a.addr, 16)
    names, sizes = load_meta()
    src = a.src or src_for(addr)
    obj = a.obj or HERE + '/fc_' + os.path.basename(src)[:-4] + '.obj'
    if not a.nocompile: compile_tu(src, obj)
    score, exact, (oi, ri, sm), _ = compare(addr, obj, src, names[addr], sizes.get(addr), True)
    if a.diff: show_diff(oi, ri, sm, full=a.full)

if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Check native registry lookup success and failure at each API step.

Usage: report.json entities.json [rebuilt.exe]. Registry APIs are simulated.
Validate paths, output pointer, buffer contents and guards. The original
closes all four handles even after failed opens; unset close arguments are
not compared, because their stack values are unspecified.
"""
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

import capstone
import pefile
from matching_entities import entity_address

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT.parent / "tools"


def extract(pe, address, regions, sprintf_address):
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    data = pe.get_data(address-pe.OPTIONAL_HEADER.ImageBase, 4096)
    instructions = []
    for ins in md.disasm(data, address):
        instructions.append(ins)
        if ins.group(capstone.CS_GRP_RET):
            assert ins.op_str == "4"
            break
    else:
        raise AssertionError("Missing registry lookup return")
    end = instructions[-1].address+instructions[-1].size
    code, calls = bytearray(data[:end-address]), []
    for ins in instructions:
        if ins.group(capstone.CS_GRP_JUMP):
            assert address <= ins.operands[0].imm < end
            continue
        if ins.group(capstone.CS_GRP_CALL):
            if ins.operands[0].type == capstone.x86.X86_OP_IMM:
                assert ins.operands[0].imm == sprintf_address
                calls.append(ins.address-address+ins.imm_offset)
                continue
        for op in ins.operands:
            if op.type == capstone.x86.X86_OP_MEM:
                value, offset, size = op.mem.disp, ins.disp_offset, ins.disp_size
            elif op.type == capstone.x86.X86_OP_IMM:
                value, offset, size = op.imm, ins.imm_offset, ins.imm_size
            else:
                continue
            target = next((dest+value-start for start, length, dest in regions
                           if start <= value < start+length), None)
            if target is not None:
                assert size == 4
                struct.pack_into("<I", code, ins.address-address+offset, target)
            elif op.type == capstone.x86.X86_OP_MEM:
                assert not 0x400000 <= value < 0x900000, hex(value)
    return code, calls


DRIVER = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef char *(__stdcall *RegistryFn)(char *);
static BYTE *arena,expected[65536],original[65536];
static int caseId,image,failedStep,opens,queries,closes;
static unsigned int seed;
static const char *paths[]={"SOFTWARE","Codemasters","Colin McRae Rally 2"};
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static void fail(const char *why,int position) {
    printf("%s case=%d image=%d position=%d\n",why,caseId,image,position);exit(3);
}
static LONG __stdcall openKey(HKEY parent,const char *name,DWORD options,REGSAM access,HKEY *out) {
    int step=opens++;
    if(step>3||options||access!=KEY_EXECUTE)fail("registry open flags",step);
    if(!step){if(parent!=HKEY_LOCAL_MACHINE||name!=NULL)fail("registry root",step);}
    else if(parent!=(HKEY)(0x12340000+step)||!name||strcmp(name,paths[step-1]))fail("registry path",step);
    *out=(HKEY)(0x12340001+step);
    return failedStep==step?ERROR_ACCESS_DENIED:ERROR_SUCCESS;
}
static LONG __stdcall queryKey(HKEY parent,const char *name,DWORD *reserved,DWORD *type,BYTE *data,DWORD *size) {
    int i;queries++;
    if(parent!=(HKEY)0x12340004||strcmp(name,(char *)arena+0xa000)||reserved||data!=arena+0x8000||*size!=100)fail("registry query arguments",0);
    *type=REG_SZ;
    for(i=0;i<12;i++)data[i]=(BYTE)('a'+(caseId+i)%26);
    data[12]=0;*size=13;
    return failedStep==4?ERROR_MORE_DATA:ERROR_SUCCESS;
}
static LONG __stdcall closeKey(HKEY handle) {closes++;return ERROR_SUCCESS;}
static void *load(const BYTE *bytes,int size,const int *calls,int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i,disp;if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++){disp=(BYTE *)sprintf-(p+calls[i]+4);memcpy(p+calls[i],&disp,4);}
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    int i;seed=caseId+153;for(i=0;i<65536;i++)arena[i]=(BYTE)(next()>>24);
    failedStep=caseId%6;opens=queries=closes=0;
    strcpy((char *)arena+0x1000,paths[0]);strcpy((char *)arena+0x1100,paths[1]);
    strcpy((char *)arena+0x1200,paths[2]);strcpy((char *)arena+0x1300,"%s");
    sprintf((char *)arena+0xa000,"PathValue%d",caseId);
    memset(arena+0x8000,0xa5,100);
    *(void **)(arena+0xf000)=openKey;*(void **)(arena+0xf004)=queryKey;*(void **)(arena+0xf008)=closeKey;
    memcpy(expected,arena,65536);
    strcpy((char *)expected+0x6000,(char *)expected+0xa000);
    if(failedStep>=4){for(i=0;i<12;i++)expected[0x8000+i]=(BYTE)('a'+(caseId+i)%26);expected[0x800c]=0;}
    if(failedStep<5)expected[0x8000]=0;
}
int main(void) {
    RegistryFn fns[2];char *answer;int i,differences=0;
    arena=(BYTE *)VirtualAlloc((void *)0x32000000,65536,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x32000000)exit(2);
'''
TAIL = r'''
    for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();answer=fns[image]((char *)arena+0xa000);
        if(answer!=(char *)arena+0x8000)fail("registry returned pointer",0);
        if(opens!=(failedStep<4?failedStep+1:4)||queries!=(failedStep>=4)||closes!=4)fail("registry API counts",opens);
        for(i=0;i<65536;i++)if(arena[i]!=expected[i])fail("registry output memory",i);
        if(!image)memcpy(original,arena,65536);else if(memcmp(original,arena,65536))differences++;
    }
    printf("6000 native registry cases: %d differences; each failed open/query, success, paths, output buffer, API counts and full arena checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    rebuilt = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / "build/CMR2.exe"
    address = next(int(e["recomp"], 16) for e in report["data"]
                   if int(e["address"], 16) == 0x4aa720)
    source, loads = "#include <windows.h>\n", ""
    for image in range(2):
        pe = pefile.PE(str(rebuilt if image else ROOT / "cmr2bin/CMR2.exe"))
        def entity(a):
            return entity_address(entities, a) if image else a
        regions = [(entity(a), size, 0x32000000+offset) for a, size, offset in
                   [(0x520f88, 9, 0x1000), (0x520f7c, 12, 0x1100),
                    (0x520f68, 20, 0x1200), (0x516180, 3, 0x1300),
                    (0x663b60, 260, 0x6000), (0x663ee4, 100, 0x8000)]]
        names = {b"RegOpenKeyExA": 0xf000, b"RegQueryValueExA": 0xf004, b"RegCloseKey": 0xf008}
        found = set()
        for dll in pe.DIRECTORY_ENTRY_IMPORT:
            for entry in dll.imports:
                if entry.name in names:
                    regions.append((entry.address, 4, 0x32000000+names[entry.name]))
                    found.add(entry.name)
        assert found == set(names)
        code, calls = extract(pe, address if image else 0x4aa720, regions, entity(0x405620))
        source += "static BYTE code%d[]={%s};\n" % (image, ",".join(map(str, code)))
        source += "static int calls%d[]={%s};\n" % (image, ",".join(map(str, calls)))
        loads += "    fns[%d]=(RegistryFn)load(code%d,sizeof(code%d),calls%d,%d);\n" % (image, image, image, image, len(calls))
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=str(TOOLS / "wineprefix"))
    def win(path):
        return "Z:"+str(path).replace("/", "\\")
    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin")+";"+win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-registry-value-") as tmp:
        (Path(tmp) / "registry.c").write_text(source+DRIVER+loads+TAIL)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "registry.c", "/Feregistry.exe"],
                       cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "registry.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()

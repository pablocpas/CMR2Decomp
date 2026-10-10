#!/usr/bin/env python3
"""Build the observer DLL and stage isolated, fingerprinted Windows captures."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
FUNCTIONS = {
    "dispatch": 0x4d0780, "race": 0x41c5a0, "car_step": 0x42baf0, "end_tick": 0x42bc80, "read_keyboard": 0x49f300,
    "mode": 0x4057d0, "active_menu": 0x4ea5d0, "menu_main": 0x4f8410, "menu_modes": 0x4f84a0,
    "menu_difficulty": 0x4f8300, "menu_country": 0x4f8340, "menu_stage": 0x4f8350,
    "menu_alternate": 0x4f8360, "menu_name": 0x4f83c0, "set_name": 0x4eae90, "profile_slot": 0x4f2be0,
    "country": 0x406910, "stage": 0x406930,
}
DATA = {
    "cars": 0x53c9a4, "parts": 0x588b94, "damage": 0x588b98, "contacts": 0x592734,
    "timing": 0x53d968, "checkpoints": 0x542e78, "transforms": 0x53b560, "shadow": 0x53ad10,
    "wheels": 0x53bda0, "order": 0x53b500, "order_count": 0x53a3a0, "physics_step": 0x519c8c,
    "physics_scale": 0x519c90, "stage_clock": 0x53d1b0, "frame_time": 0x663ee0,
    "keyboard": 0x59f7c8, "garage_menu": 0x831778,
    "sqrt_table": 0x6e0ef4, "sin_table": 0x6e2ef4, "acos_table": 0x6e6ef4,
    "atan_table": 0x6e8ff4, "tan_table": 0x6e93f4,
}
DETOURS = {"dispatch", "race", "car_step", "end_tick", "read_keyboard"}
ORIGINAL_SHA = "21b2dd13798724bcfe0c64b44216fb3e772c86addd330e821e53d5962fca3817"


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def win(path):
    return "Z:" + str(path.resolve()).replace("/", "\\")


def create_map(executable, output, entities=None):
    import capstone
    import pefile
    pe = pefile.PE(str(executable))
    image, base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
    disassembler = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    disassembler.detail = True
    records = []
    for name, original in {**FUNCTIONS, **DATA}.items():
        address = entities[hex(original)][0] if entities else original
        length, prefix = 0, ""
        if name in DETOURS:
            code = image[address-base:address-base+32]
            for instruction in disassembler.disasm(code, address):
                if instruction.group(capstone.CS_GRP_JUMP) or instruction.group(capstone.CS_GRP_CALL):
                    if length or instruction.size != 5 or instruction.bytes[0] not in (0xe8, 0xe9):
                        raise ValueError(f"unsupported relative detour prologue: {name} {instruction.mnemonic} {instruction.op_str}")
                length += instruction.size
                if length >= 5:
                    break
            if length < 5:
                raise ValueError(f"incomplete function signature: {name}")
            prefix = code[:length].hex()
        records.append(f"{name} {address:08x} {length} {prefix}".rstrip())
    output.write_text("\n".join(records) + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data", type=Path, required=True)
    parser.add_argument("--decomp", type=Path, default=ROOT.parent / "CMR2Decomp")
    parser.add_argument("--toolchain", type=Path, default=ROOT.parent / "tools")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--wineprefix", type=Path, required=True)
    args = parser.parse_args()
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    if (args.output / "manifest.json").exists():
        raise ValueError("capture preparation exists; use a new output directory")
    decomp, tools, data = args.decomp.resolve(), args.toolchain.resolve(), args.data.resolve()
    original, rebuilt = decomp / "cmr2bin/CMR2.exe", decomp / "build/CMR2.exe"
    if sha(original) != ORIGINAL_SHA:
        raise ValueError("original reference hash does not match reccmp-project.yml")
    build = json.loads((decomp / "build/manifest.json").read_text())
    if sha(rebuilt) != build["exe_sha256"] or sha(decomp / "build/CMR2.pdb") != build["pdb_sha256"]:
        raise ValueError("decompilation binary/PDB does not match its build manifest")
    for name, expected in build["source_sha256"].items():
        if sha(decomp / name) != expected:
            raise ValueError(f"decompilation source changed since build: {name}")
    env = dict(os.environ, WINEPREFIX=str(args.wineprefix.resolve()), WINEDEBUG="-all",
               INCLUDE=win(tools / "msvc600/VC98/Include"), LIB=win(tools / "msvc600/VC98/Lib"),
               WINEPATH=win(tools / "msvc600/VC98/Bin") + ";" + win(tools / "msvc600/Common/MSDev98/Bin"))
    source = ROOT / "tools/reference_capture/capture.cpp"
    obj, dll = args.output / "capture.obj", args.output / "SPCMR2.dll"
    subprocess.run(["wine", str(tools / "msvc600/VC98/Bin/CL.EXE"), "/nologo", "/c", "/O2", "/Gz", "/MD", "/Fo" + win(obj), win(source)], env=env, check=True)
    subprocess.run(["wine", str(tools / "msvc600/VC98/Bin/LINK.EXE"), "/nologo", "/DLL", "/BASE:0x11000000", "/ENTRY:CaptureEntry@12",
                    "/DEF:" + win(ROOT / "tools/reference_capture/capture.def"), "/OUT:" + win(dll), "/INCREMENTAL:NO", win(obj), "msvcrt.lib", "kernel32.lib", "user32.lib"], env=env, check=True)
    entities = json.loads((decomp / "CMR2PROGRESS/entities.json").read_text())
    patch = tools / "hybrid/silentpatch/SPCMR2.dll"
    manifest = {"schema": 1, "original_sha256": sha(original), "decompilation_build": build,
                "silentpatch_sha256": sha(patch), "observer_sha256": sha(dll),
                "observer_source_sha256": sha(source), "writer_sha256": sha(ROOT / "src/diagnostics/state_capture.h"),
                "entities_sha256": sha(decomp / "CMR2PROGRESS/entities.json"), "runs": {}}
    for side, exe in (("original", original), ("decomp", rebuilt)):
        run = args.output / side
        run.mkdir()
        for folder in data.iterdir():
            if folder.is_dir() and folder.name.lower() not in ("configuration", "cmr2saves", "build"):
                (run / folder.name).symlink_to(folder, target_is_directory=True)
        (run / "Configuration").mkdir()
        (run / "CMR2Saves").mkdir()
        shutil.copyfile(exe, run / "CMR2.exe")
        shutil.copyfile(dll, run / "SPCMR2.dll")
        for name in ("binkw32.dll", "Msvcrt.dll", "Msvcrt40.dll", "Msadp32.acm"):
            candidate = next((p for p in data.iterdir() if p.name.lower() == name.lower()), None)
            if candidate:
                shutil.copyfile(candidate, run / name)
        for name in ("DDraw.dll", "D3DImm.dll", "dgVoodoo.conf"):
            shutil.copyfile(tools / "hybrid/dgvoodoo" / name, run / name)
        if side == "original":
            shutil.copyfile(patch, run / "SPCMR2_sp.dll")
            shutil.copyfile(tools / "hybrid/silentpatch/SPCMR2.ini", run / "SPCMR2.ini")
        else:
            # The observer's ordinal-1 proxy replaces DirectSoundCreate import.
            # SilentPatch's fixed original addresses cannot be applied blindly
            # to the relocated standalone decompilation.
            import pefile
            pe = pefile.PE(str(run / "CMR2.exe"))
            for entry in pe.DIRECTORY_ENTRY_IMPORT:
                if entry.dll.lower() == b"dsound.dll":
                    position = pe.get_offset_from_rva(entry.struct.Name)
                    payload = bytearray((run / "CMR2.exe").read_bytes())
                    payload[position:position+11] = b"SPCMR2.dll\0"
                    (run / "CMR2.exe").write_bytes(payload)
                    break
            else:
                raise ValueError("decompilation has no expected DirectSound import")
        create_map(exe, run / "capture.map", entities if side == "decomp" else None)
        manifest["runs"][side] = {"executable_sha256": sha(run / "CMR2.exe"), "address_map_sha256": sha(run / "capture.map"),
                                  "silentpatch": side == "original",
                                  "runtime_sha256": {p.name: sha(p) for p in run.iterdir() if p.is_file() and p.name != "CMR2.exe"}}
    (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"Recorder and isolated runs prepared: {args.output}")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        print(f"prepare failed: {error}", file=sys.stderr)
        sys.exit(2)

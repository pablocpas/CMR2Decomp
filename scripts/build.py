"""Build the matching MSVC6 executable on Windows or through Wine."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess


ROOT = Path(__file__).resolve().parents[1]
QIFIST = set("Race.cpp StageUI.cpp TimingUtils.cpp Frontend.cpp FrontendScreens.cpp "
             "Game.cpp GameInfo.cpp Graphics.cpp Sprite.cpp Car.cpp Sound.cpp "
             "CarPhysics.cpp HudDash.cpp CarEffects.cpp TrackCollision.cpp RallyData.cpp "
             "Mesh.cpp Sector.cpp StageTiming.cpp StageObjects.cpp SceneNode.cpp "
             "FixedPoint.cpp RallyTiming.cpp NetRace.cpp".split())


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    default_msvc = next((p for p in (ROOT / "msvc600/VC98", ROOT.parent / "msvc600/VC98")
                         if p.is_dir()), ROOT / "msvc600/VC98")
    parser.add_argument("--msvc-root", type=Path, default=Path(
        os.environ.get("CMR2_MSVC_ROOT", default_msvc)))
    parser.add_argument("--windowed", action="store_true")
    args = parser.parse_args()
    msvc = args.msvc_root.resolve()
    compiler = msvc / "Bin/CL.EXE"
    dependencies = [ROOT / "third_party/dx7sdk-7001/include/ddraw.h",
                    ROOT / "third_party/bink-sdk-1.0p/lib/binkw32.lib", compiler]
    for path in dependencies:
        if not path.is_file():
            parser.error("Missing build dependency: " + str(path))
    native = os.name == "nt"

    def win(path):
        absolute = str(Path(path).resolve())
        return absolute if native else "Z:" + absolute.replace("/", "\\")

    env = dict(os.environ)
    env["INCLUDE"] = ";".join(map(win, [msvc / "Include", ROOT / "third_party/dx7sdk-7001/include"]))
    env["LIB"] = ";".join(map(win, [msvc / "Lib", ROOT / "third_party/dx7sdk-7001/lib"]))
    compiler_dirs = [msvc / "Bin", msvc.parent / "Common/MSDev98/Bin"]
    if native:
        env["PATH"] = os.pathsep.join(map(str, compiler_dirs)) + os.pathsep + env.get("PATH", "")
        prefix = [str(compiler)]
    else:
        env["WINEDEBUG"] = "-all"
        env["WINEPATH"] = ";".join(map(win, compiler_dirs))
        prefix = ["wine", str(compiler)]
    build = ROOT / "build"
    build.mkdir(exist_ok=True)
    # Failed compilation must never leave an older executable available for measurement.
    for path in [*build.glob("*.obj"), build / "CMR2.exe", build / "CMR2.pdb", ROOT / "vc60.pdb"]:
        path.unlink(missing_ok=True)
    flags = ["/O2", "/DNDEBUG", "/D_CRTIMP=", "/Zi", "/Gz", "/MD", "/GX"]
    flags += shlex.split(os.environ.get("CMR2_CFLAGS", ""))
    if args.windowed:
        flags.append("/DCMR2_WINDOWED")
    sources = sorted((ROOT / "CMR2Decomp").glob("*.cpp"))
    groups = [([p for p in sources if p.name in QIFIST], ["/QIfist"]),
              ([p for p in sources if p.name.startswith("Zlib")], ["/Ob2"]),
              ([p for p in sources if p.name not in QIFIST and not p.name.startswith("Zlib")], [])]
    for paths, extra in groups:
        if paths:
            subprocess.run(prefix + ["/c"] + flags + extra + ["/Fo" + win(build) + "\\"] +
                           [win(p) for p in paths], cwd=ROOT, env=env, check=True)
    libraries = ["user32.lib", "gdi32.lib", "advapi32.lib", "ole32.lib", "msacm32.lib", "winmm.lib"]
    libraries += [win(ROOT / "third_party/dx7sdk-7001/lib" / name)
                  for name in ["dxguid.lib", "dinput.lib", "ddraw.lib"]]
    libraries.append(win(ROOT / "third_party/bink-sdk-1.0p/lib/binkw32.lib"))
    subprocess.run(prefix + [win(build / (p.stem + ".obj")) for p in sources] + flags +
                   ["/Fe" + win(build / "CMR2.exe"), "/link"] + libraries +
                   ["/INCREMENTAL:NO", "/DEBUG", "/PDB:" + win(build / "CMR2.pdb"), "/SUBSYSTEM:WINDOWS"] +
                   shlex.split(os.environ.get("CMR2_LINKFLAGS", "")), cwd=ROOT, env=env, check=True)
    commit = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip()
    source_changed = subprocess.run(["git", "diff", "--quiet", "HEAD", "--", "CMR2Decomp"],
                                    cwd=ROOT).returncode != 0
    manifest = {"commit": commit, "source_changed": source_changed,
                "windowed": args.windowed, "flags": flags,
                "qifist_files": sorted(QIFIST), "zlib_flags": ["/Ob2"], "compiler_sha256": sha256(compiler),
                "source_sha256": {str(p.relative_to(ROOT)): sha256(p)
                                  for p in sorted((ROOT / "CMR2Decomp").rglob("*"))
                                  if p.suffix in (".cpp", ".h")},
                "exe_sha256": sha256(build / "CMR2.exe"), "pdb_sha256": sha256(build / "CMR2.pdb")}
    (build / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print("OK -> build/CMR2.exe")


if __name__ == "__main__":
    main()

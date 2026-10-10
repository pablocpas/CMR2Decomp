# OpenCMR2

A native, modernised port of **Colin McRae Rally 2.0** (PC, 2000), built from
the CMR2Decomp decompilation in the parent directory of this repository. It
replaces DirectDraw/Direct3D 7, DirectSound, DirectInput, DirectPlay and Bink
with SDL3 and the SDL GPU API (Vulkan, Direct3D 12, Metal) and our own code,
so the game runs natively on Linux, Windows and macOS.

OpenCMR2 contains no game data. You need your own copy of the original game.

> [!NOTE]
> Work in progress: the game code compiles and an initial SDL_GPU renderer is
> implemented. Gameplay parity and simulation fidelity are still being verified.
> See [docs/PLAN.md](docs/PLAN.md) for the roadmap,
> [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for the architecture and
> [docs/MODERNIZATION.md](docs/MODERNIZATION.md) for the design of graphics
> modernisation with the original simulation.

## Goals

- **Faithful by default.** The target is the original simulation, bit for bit
  in the reference build. Fixed-point helpers have assembly equivalence tests;
  500 Finland stage 1 updates now match original + SilentPatch at six render
  cadences, with stable 25 Hz scheduling. Full-game parity remains open;
  see [the comparison](docs/PHYSICS_PARITY.md).
- **Modern where it helps, always optional.** Any resolution and aspect ratio,
  high refresh rates with the physics at its original rate, better image
  quality and lighting, modern controllers and force feedback, and
  quality-of-life improvements. The original look is one preset away.

## Building (Linux)

Fedora packages for the 32-bit reference build:

    sudo dnf install glibc-devel.i686 libstdc++-devel.i686 SDL3-devel.i686 ninja-build glslang

Then:

    cmake --preset linux-x86
    cmake --build --preset linux-x86
    ctest --test-dir build/linux-x86

To record and replay a real stage at different render cadences, or measure CPU
frame/tick/presentation timings, see [docs/BASELINE.md](docs/BASELINE.md).
For frozen Windows original + SilentPatch recordings and field-by-field
comparison with the port, see [docs/ORIGINAL_REFERENCE.md](docs/ORIGINAL_REFERENCE.md).
Vehicle physics and game-speed checks at different render FPS are summarized
in [docs/PHYSICS_PARITY.md](docs/PHYSICS_PARITY.md).

    python3 tools/stage_baseline.py run --data ~/cmr2game --output build/baseline-first

## Updating the game source

`game/` is a copy of `../CMR2Decomp/` with the port's edits. To bring in the
decomp's changes since the commit in `UPSTREAM`:

    tools/sync_upstream.py

The goal is to fold those edits back into `../CMR2Decomp/` (behind
`#ifndef OPENCMR2` where they replace platform code, which keeps the matching
build byte-identical) until `game/` is no longer needed.

## Licence

GPL-3.0, like CMR2Decomp.

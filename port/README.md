# OpenCMR2

A native, modernised port of **Colin McRae Rally 2.0** (PC, 2000), built from
the [CMR2Decomp](https://github.com/pablocpas/CMR2Decomp) decompilation. It
replaces DirectDraw/Direct3D 7, DirectSound, DirectInput, DirectPlay and Bink
with SDL3 and the SDL GPU API (Vulkan, Direct3D 12, Metal) and our own code,
so the game runs natively on Linux, Windows and macOS.

OpenCMR2 contains no game data. You need your own copy of the original game.

> [!NOTE]
> Work in progress: the port does not run yet. See [docs/PLAN.md](docs/PLAN.md)
> for the roadmap and [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for how it is
> built.

## Goals

- **Faithful by default.** The simulation is the original's, bit for bit in
  the reference build, checked by golden replay tests against the original
  executable.
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

## Updating the game source

`game/` follows CMR2Decomp. To bring in upstream changes:

    git remote add upstream https://github.com/pablocpas/CMR2Decomp   # once
    tools/sync_upstream.py

## Licence

GPL-3.0, like CMR2Decomp.

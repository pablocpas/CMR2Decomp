# OpenCMR2 roadmap

Milestones, in order. Each phase lists what "done" means.

## F0: Portable build of the game code

- [x] Repository, `game/` imported from CMR2Decomp, `tools/sync_upstream.py`.
- [x] CMake presets: `linux-x86` (32-bit reference), `linux-x86-debug` (ASan/UBSan).
- [x] `src/port/types.h` replaces `<windows.h>` (type names only).
- [x] Fixed-point helpers in portable C, equal to the original assembly
      (`tests/fixedpoint_equiv`); no inline assembly left in `game/`.
- [ ] Platform API headers (`src/port/*.h`) and game headers free of
      DirectX/Win32 types.
- [ ] Every platform-free TU compiles with Clang and GCC.
- [ ] `/QIfist` TUs: float-to-int conversions made explicit (`lrintf`),
      found with the Clang AST.

## F1: Platform layer (SDL3)

- [ ] `main()`, window, event loop, focus handling (replaces WinMain/WndProc).
- [ ] Time, files (case-insensitive game data lookup, user directory for
      saves and settings), configuration (replaces the registry and the CD
      check), message boxes.
- [ ] Input: keyboard (DIK codes), mouse, joysticks, force feedback, gamepads.
- [ ] Audio: mixer with DirectSound semantics (3D voices, streaming music),
      WAV and MS-ADPCM decoding.

## F2: Renderer (SDL_GPU)

- [ ] `gfx.h` API and the ~134 rendering functions ported to it.
- [ ] Uber-shader reproducing the fixed-function pipeline (lighting, fog,
      texture stages, bump env map, reflection texgen), HLSL compiled to
      SPIR-V/DXIL/MSL.
- [ ] Textures (BGRA8, BC1, BC3, bump), render targets, cube maps.
- [ ] 2D path (sprites, triangles, lines, fonts).

## F3: Parity (0.1)

- [ ] Menus, rallies, arcade, replays, split screen, saves.
- [ ] Bink 1 (`BIKi`) decoder of our own for the videos (no FFmpeg).
- [ ] Headless build (null renderer and audio) and golden replay tests
      against traces from the original executable.

## F4: 64-bit and other platforms

- [ ] Remaining raw-offset accesses typed; pointer-holding structures loaded
      from disk split into on-disk and in-memory forms; size/offset
      `static_assert`s in the 32-bit build.
- [ ] Linux x86-64, Windows x64, macOS ARM64 builds and CI.

## F5: Modernisation (every item optional, "Original" preset)

- [ ] Fixed-rate simulation (25 Hz, as the original's physics scale assumes)
      with interpolated rendering at any refresh rate.
- [ ] Any resolution, widescreen (Hor+ FOV), HUD anchored to the screen
      edges, UI scaling, borderless fullscreen, VSync options.
- [ ] MSAA, anisotropic filtering, reversed-Z, longer draw and fog
      distance, per-pixel lighting, cube-map car reflections, optional
      tonemapping and bloom, texture packs.
- [ ] Controls: rebinding, dead zones and response curves, hot-plug,
      gamepad glyphs, wheel force feedback.
- [ ] Quality of life: skip intros, quick stage restart, pause on focus
      loss, per-category volume, camera and FOV options, mods through an
      overlay data directory, `FIX_BUGS` fixes for original bugs.
- [ ] Network play over UDP (ENet-style), replacing DirectPlay.

## F6: Distribution

- [ ] Flatpak, AppImage, Windows and macOS packages; first-run setup that
      locates and checks the original game data.

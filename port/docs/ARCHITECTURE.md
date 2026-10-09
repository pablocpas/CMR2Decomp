# OpenCMR2 architecture

OpenCMR2 is a native port of Colin McRae Rally 2.0 built from the
[CMR2Decomp](https://github.com/pablocpas/CMR2Decomp) decompilation. It runs on
SDL3 and the SDL GPU API (Vulkan, Direct3D 12, Metal) and needs the original
game data.

## Layout

| Path | Contents |
| --- | --- |
| `game/` | The decompiled game, synced from CMR2Decomp (`tools/sync_upstream.py`, last commit in `UPSTREAM`). |
| `src/port/` | The platform API the game calls: `types.h`, `sys.h`, `gfx.h`, `audio.h`, `input.h`, `movie.h`, `net.h`. Plain C-style functions and our own types; no Win32 or DirectX names. |
| `src/platform/` | `main()`, window and event loop, time, files, configuration (SDL3). |
| `src/render/` | The SDL_GPU renderer behind `gfx.h`, and its shaders. |
| `src/audio/` | Mixer behind `audio.h` (SDL3 audio streams), WAV and MS-ADPCM decoding. |
| `src/input/` | Keyboard, mouse, joysticks, gamepads and force feedback (SDL3). |
| `src/video/` | Bink 1 (`BIKi`) decoder behind `movie.h`. No FFmpeg. |
| `src/net/` | Network sessions behind `net.h` (replaces DirectPlay). |
| `tests/` | Unit tests and the golden replay tests. |

## How the game is ported

About 300 of the 3369 game functions call Win32 or DirectX; the rest is
platform-free logic. Those 300 are rewritten in place in `game/` against the
`src/port` API. They keep their `// FUNCTION: CMR2 0x...` annotation, so each
one can still be compared with the original, and get a `// PORT:` line that
says what changed. Game data structures that embedded DirectX types use our
types instead (`GfxTexture *` for `IDirectDrawSurface7 *`, `GfxTLVertex` for
`D3DTLVERTEX`); in the 32-bit build every structure keeps its size and layout.

Everything else in `game/` is changed only for portability (inline assembly,
MSVC-only syntax, undefined behaviour) or for real bugs, so upstream syncs stay
easy. Keep those edits small and local.

The game source is compiled with `-include port/types.h`, which provides the
Win32 type names the decompilation uses (`BYTE`, `DWORD`, `BOOL`, `RECT`) with
Win32's sizes. It declares no Win32 functions.

## The renderer

CMR2 renders with Direct3D 7's fixed-function pipeline:

- one vertex format for 3D (FVF `0x2d2`: position, normal, diffuse,
  specular, two UV sets; 48 bytes), in vertex buffers the game fills itself,
  drawn with indexed triangle lists and strips;
- pre-transformed vertices for 2D (`GfxTLVertex`, the `D3DTLVERTEX` layout);
- hardware lighting with up to 8 lights (directional, point, spot), material
  colours taken from the vertex colour, ambient light;
- linear vertex/table fog;
- two texture stages (modulate, select, add-signed, bump env map), clamp or
  wrap, point/linear filtering with mipmaps;
- camera-space reflection vectors with a texture transform (car
  reflections), cube maps rendered at runtime;
- alpha blending, alpha test, depth test and write, culling.

`gfx.h` exposes exactly that model as state-setting calls on our own types
(`Gfx_SetTransform`, `Gfx_SetLight`, `Gfx_SetStage`, `Gfx_DrawIndexed`, ...).
The SDL_GPU backend turns the current state into a pipeline key and a uniform
block, and draws with an "uber" shader that reproduces the fixed-function
equations. Device enumeration, display-mode switching, lost surfaces and
COM disappear: the game asks for what it needs and the backend provides it.

Rendering upgrades (resolution, widescreen, MSAA, anisotropic filtering,
per-pixel lighting) live in the backend and shaders, each behind an option,
with the original look as the default preset.

Textures: the game decodes TGA/DDS itself and writes pixels through
`Gfx_LockTexture`, using the pixel format the backend reports. The backend
offers 32-bit BGRA8, BC1 and BC3 (the DDS formats on the disc), and a dU/dV/L
bump format.

## Audio

The game uses DirectSound buffers: static samples, duplicated voices, 3D
voices (position, distances), frequency, volume in hundredths of a decibel,
pan, and a streamed music buffer refilled by regions. `audio.h` keeps those
semantics (sample, voice, stream) on our own types; `src/audio` mixes all
voices into one SDL3 audio stream and applies DirectSound's 3D attenuation.
WAV files and MS-ADPCM are decoded by our own code (the game used mmio and
ACM).

## Input

Key codes stay DirectInput scan codes (`DIK_*` values): the game stores them
in its binding tables and save files. `src/input` maps SDL scancodes to them.
Joysticks are read through SDL3 into a `DIJOYSTATE2`-like structure of our
own (axes, sliders, POVs, buttons). Force-feedback effects (constant, spring,
damper, friction, inertia) map to SDL haptic effects, with rumble as the
fallback on gamepads.

## Determinism and the golden tests

Vehicle physics is integer fixed point, so a 32-bit build of the port must
reproduce the original simulation exactly. The golden tests replay recorded
inputs through a headless build (null renderer, null audio) and compare a hash
of the car states every frame against traces recorded from the original
executable.

The compiler flags encode what the decompiled code relies on:

- `-fwrapv`: 16.16 fixed point overflows on purpose;
- `-fno-strict-aliasing`: the code reinterprets memory freely;
- `-msse2 -mfpmath=sse`: float arithmetic in single precision. The original
  ran the x87 in 24-bit precision once Direct3D was initialised;
- float-to-int conversions: the original TUs built with `/QIfist` round to
  nearest instead of truncating. Those conversions use `lrintf` explicitly.

## Builds

- `linux-x86` (CMake preset): 32-bit x86, the reference build. Data
  structures have the original layout.
- `linux-x86-debug`: the same with AddressSanitizer and UBSan.
- 64-bit builds (Linux, Windows, macOS ARM) follow once the game structures
  are pointer-size clean.

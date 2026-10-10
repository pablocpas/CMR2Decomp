# Movie playback

`src/video/bink_decoder.cpp` decodes the game's original `BIKi` files directly.
It builds as `opencmr2_bink`, a small static library using only the C++ standard
library. The executable does not link FFmpeg or invoke a conversion utility.
Game data stays outside the repository.

The decoder reads the frame index and track metadata, validates packet lengths,
and decodes sequentially with padded YUV420 reference planes. It supports all ten
Bink block types, including scaled, motion, residue and integer DCT blocks.
Output is cropped to the original dimensions and converted to BGRX for the
dynamic movie texture. Alpha planes are decoded when present, but full-screen
playback uses opaque output. Other Bink revisions and Bink 2 are rejected.

Sound is decoded to interleaved float PCM, including RDFT and DCT transforms,
overlap between audio blocks and the valid sample count in the final packet.
`Movie_Open` selects the requested Bink track ID; missing IDs fall back to the
first track and `-1` disables sound. The game's intro files have one stereo RDFT
track; country videos have five language tracks.

The audio layer resamples queued movie PCM into its existing SDL3 mixer. Sound
starts at the first presentation, alongside the millisecond playback clock.
Closing or skipping the movie discards its audio queue. The movie loop retains
the final frame and waits for its display interval before closing. Corrupt input
logs an error, stops movie audio and ends playback; audio-device failure permits
silent video playback with a log message.

The blocking movie loop dispatches SDL events both before decoding and while
waiting for the next frame. Enter (including keypad Enter) skips the movie;
the skip key is consumed rather than queued as a menu confirmation. Joystick
confirmation still uses the game's normal input mapping. Intro frames stretch
across the entire window/full-screen drawable, including when the internal game
resolution is 4:3 and the monitor is widescreen. Explicit movie rectangles are
honoured. Ordinary game presentation retains its existing aspect-ratio policy.
Each movie pass saves and restores the renderer's drawing state and the caller
restores its viewport. This keeps the game's cached state setters consistent:
fonts and menu sprites retain their texture operations and transparency after
the intro or a country video.

## Tests

The default CTest suite needs no game assets or codec tools. It exercises padded
dimensions, fill/skip/motion blocks, fractional frame rates, RDFT/DCT waveforms,
stereo interleaving, overlap, track selection, truncation and bitstream mutations.
The movie integration test checks texture pitch, duplicate decode calls, audio
startup, clock wrap, the final frame and cleanup after skipping or open failure.
`movie_render_state` runs the movie pass through the real frame recorder and
checks the following menu draw without reapplying its cached render states.

Validation on 2026-10-09 used the ten original movies in the local game copy:
all 8,589 frames matched a reference decode pixel for pixel after applying the
same YUV-to-BGRX conversion. The intro audio and all five UK language tracks
were compared as float PCM; the maximum measured intro difference was
`2.39e-7`. Reference transform padding beyond the final packet's declared sample
count is excluded. Synthetic tests and a complete `cm.bik` decode also ran with
AddressSanitizer/UBSan, and queued output was exercised with SDL's dummy audio
device. Reference comparison used an installed FFmpeg executable only as a
development check; it is not a build, test-suite or runtime requirement.

To also decode every original movie through the standalone decoder:

```sh
cmake --preset linux-x86 -DOPENCMR2_BINK_TEST_DATA_DIR="$HOME/cmr2game"
cmake --build --preset linux-x86
ctest --test-dir build/linux-x86 --output-on-failure
```

`build/linux-x86/tests/bink_decode FILE [TRACK_ID]` decodes a whole movie and
reports frame/sample counts, audio peak and a video hash. For reference comparison,
optional arguments `BGRX_FILE F32_FILE [FRAME_LIMIT]` dump raw pixels and audio;
an empty filename suppresses that output. Test dumps are not game assets to ship.

## Source attribution

The video bundle/coefficient decoding, integer inverse DCT and format tables
are adapted from the narrowly scoped Bink routines in FFmpeg n6.1:

- [bink.c](https://github.com/FFmpeg/FFmpeg/blob/n6.1/libavcodec/bink.c),
  Konstantin Shishkov and Peter Ross.
- [binkdata.h](https://github.com/FFmpeg/FFmpeg/blob/n6.1/libavcodec/binkdata.h) and
  [binkdsp.c](https://github.com/FFmpeg/FFmpeg/blob/n6.1/libavcodec/binkdsp.c),
  Konstantin Shishkov.
- [binkaudio.c](https://github.com/FFmpeg/FFmpeg/blob/n6.1/libavcodec/binkaudio.c),
  Peter Ross and Daniel Verkamp, used as the audio format reference.

Those routines retain their LGPL-2.1-or-later notices, with the licence in
`src/video/COPYING.LGPLv2.1`. OpenCMR2's surrounding implementation remains GPL-3.0.
The file reader, checked bitstream, reference buffers, colour conversion, FFT,
audio queue and platform integration are local code; no FFmpeg infrastructure,
headers or libraries are needed to build or run them.

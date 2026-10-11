# Float to integer conversions under /QIfist

24 of the original's translation units are compiled with `/QIfist`
(`scripts/build.py`). There a float to integer conversion is an x87 `FISTP`,
which rounds to nearest (even) under the control word the game runs with
(0x027f, measured by OpenCMR2's physics recorder), where C requires truncation.
Any other compiler truncates, so a port changes every one of these values by
up to one unit: fixed-point positions, angles, colours, random spreads.

Every conversion in those TUs is now written `FTOI(type, value)`
(`FloatToInt.h`): 425 statements found by Clang over the MSVC6-parsed source
plus the bodies of 14 macros (random spreads, map coordinates, `FIX_ANGLE`,
`VIEW_EASE`...). The existing `(int)(__int64)x` idiom stays as
`(int)FTOI(__int64, x)`: the 64-bit FISTP followed by the narrowing.

- MSVC6: `FTOI(type, x)` is `((type)(x))`; the TU's `/QIfist` makes it FISTP,
  as before. All 3364 functions keep their bytes (2924 exact).
- Other compilers with `CMR2_QIFIST` defined (OpenCMR2 defines it for the same
  24 TUs): round to nearest even like FISTP, with the integer-indefinite value
  for NaN or out-of-range input, through a 64-bit store for 64-bit and unsigned
  32-bit targets as MSVC6 does.
- Without `CMR2_QIFIST`: the plain cast (truncation, as `_ftol` in the other TUs).
  Inline code in headers follows the TU that includes it, as in the original.

The value's own parentheses become the macro's: MSVC6 compiled
`(int)((__int64)((p[0] * -65536.0)))` differently from
`(int)(__int64)(p[0] * -65536.0)` (Sector_NearestCornerHeight lost its exact
match with the extra pair).

The rewrite is OpenCMR2's `tools/ftoi_rewrite.py`; run with `--dry-run` it
lists any conversion added later without FTOI.

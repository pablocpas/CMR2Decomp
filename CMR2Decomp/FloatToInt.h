#ifndef _FLOATTOINT_H
#define _FLOATTOINT_H

#include "X87Single.h"

// Float to integer conversions. Most of the game is built with /QIfist
// (scripts/build.py): there a conversion is an x87 FISTP, which rounds to
// nearest (even) under the game's control word instead of truncating (0x027f
// at startup, 0x007f once Direct3D has set single precision: see X87Single.h).
// FTOI(type, value) marks each one. MSVC6 compiles it as the plain cast, so
// the TU's /QIfist decides, as in the original. Other compilers round like
// FISTP in the TUs built with CMR2_QIFIST (the same list) and truncate in the
// rest, so header code follows the TU that includes it.
#if defined(_MSC_VER) && _MSC_VER < 1300

#define FTOI(type, x) ((type)(x))

#elif defined(CMR2_QIFIST)

#include <math.h>

// FISTP of a 64-bit or 32-bit integer: round to nearest even; NaN or a value
// out of range stores the "integer indefinite" (the lowest value).
static inline long long Ftoi_Fist64(double x)
{
    return x >= -9223372036854775808.0 && x < 9223372036854775808.0 ? llrint(x) : (long long)(1ULL << 63);
}

static inline long long Ftoi_Fist32(double x)
{
    long long r = x > -2147483649.0 && x < 2147483648.0 ? llrint(x) : (long long)1 << 31;
    return r >= -2147483647LL - 1 && r <= 2147483647LL ? r : -2147483647LL - 1;
}

// 64-bit and unsigned 32-bit targets go through a 64-bit store, as MSVC6 does.
#define FTOI(type, x) \
    ((type)(sizeof(type) == 8 || (sizeof(type) == 4 && (type)-1 > (type)0) ? Ftoi_Fist64(x) : Ftoi_Fist32(x)))

#else

#define FTOI(type, x) ((type)(x))

#endif

#endif

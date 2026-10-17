#ifndef _X87SINGLE_H
#define _X87SINGLE_H

// x87 arithmetic at single precision. Direct3D 7 sets the precision control
// of the x87 to 24 bits (control word 0x007f) when the device is created,
// unless the application asks it to preserve the FPU state (DDSCL_FPUPRESERVE),
// which the game does not. From then on every x87 add, subtract, multiply,
// divide and square root rounds its result to a 24-bit mantissa, whatever the
// declared types; loads of double constants and the results of FSIN, FPTAN and
// FPATAN keep their full precision, and the exponent keeps its 15-bit range.
// The fixed-point math tables are computed this way after the device is
// created, and the CPU runs of the super special stages were recorded with
// them: they replay their controls open loop, so with tables computed at
// double precision the opponents leave their recorded lines and get stuck.
//
// X87S_ADD/SUB/MUL/DIV(a, b) and X87S_SQRT(x) mark these operations, and
// X87S_FILD(float, i) (an explicit cast) or X87S_LOAD(i) (an implicit
// conversion) an integer converted to float as one of their operands, which
// the x87 loads exactly (the conversion is not stored). MSVC6 compiles them as
// the plain operators and casts, so the code is the original's. Other compilers
// round the exact result once to a 24-bit mantissa and return it as a double,
// as the x87 does; the exact result is reconstructed with error-free
// transformations, without fused multiply-add.
#if defined(_MSC_VER) && _MSC_VER < 1300

// The operands are written as in the original expression, so these expand
// to its exact tokens: MSVC6 does not reassociate floating-point operations
// across parentheses, and an extra pair changes its code.
#define X87S_ADD(a, b) a + b
#define X87S_SUB(a, b) a - b
#define X87S_MUL(a, b) a * b
#define X87S_DIV(a, b) a / b
#define X87S_SQRT(x) sqrt(x)
#define X87S_FILD(type, x) (type)x
#define X87S_LOAD(x) x

#else

#include <math.h>

// Rounds an exact result to a 24-bit mantissa, nearest even. hi is the double
// nearest to it and lo has the sign of the rest (exact result - hi).
static inline double X87s_Round(double hi, double lo)
{
    int exponent;
    double m, rounded, other;
    float r;

    if (hi == 0.0 || !isfinite(hi))
        return hi;
    // Scale into [0.5, 1): the x87 exponent range does not overflow a float.
    m = frexp(hi, &exponent);
    r = (float)m;
    rounded = r;
    if (rounded != m && lo != 0.0) {
        // When hi is halfway between two floats, the rest decides.
        other = nextafterf(r, m > rounded ? 2.0f : -2.0f);
        if (fabs(m - rounded) == fabs(other - m))
            rounded = lo > 0.0 ? fmax(rounded, other) : fmin(rounded, other);
    }
    return ldexp(rounded, exponent);
}

// Dekker's exact product: a * b == *pHi + *pLo.
static inline void X87s_TwoProduct(double a, double b, double *pHi, double *pLo)
{
    const double split = 134217729.0; // 2^27 + 1
    double c, aHi, aLo, bHi, bLo;

    *pHi = a * b;
    c = split * a;
    aHi = c - (c - a);
    aLo = a - aHi;
    c = split * b;
    bHi = c - (c - b);
    bLo = b - bHi;
    *pLo = ((aHi * bHi - *pHi) + aHi * bLo + aLo * bHi) + aLo * bLo;
}

static inline double X87s_Add(double a, double b)
{
    double s = a + b;
    double bb = s - a;
    return X87s_Round(s, (a - (s - bb)) + (b - bb));
}

static inline double X87s_Mul(double a, double b)
{
    double hi, lo;

    X87s_TwoProduct(a, b, &hi, &lo);
    return X87s_Round(hi, lo);
}

static inline double X87s_Div(double a, double b)
{
    double q = a / b;
    double hi, lo;

    // The rest of the quotient has the sign of (a - q * b) / b.
    X87s_TwoProduct(q, b, &hi, &lo);
    return X87s_Round(q, ((a - hi) - lo) * (b < 0.0 ? -1.0 : 1.0));
}

static inline double X87s_Sqrt(double x)
{
    double s = sqrt(x);
    double hi, lo;

    X87s_TwoProduct(s, s, &hi, &lo);
    return X87s_Round(s, (x - hi) - lo);
}

#define X87S_ADD(a, b) X87s_Add((double)(a), (double)(b))
#define X87S_SUB(a, b) X87s_Add((double)(a), -(double)(b))
#define X87S_MUL(a, b) X87s_Mul((double)(a), (double)(b))
#define X87S_DIV(a, b) X87s_Div((double)(a), (double)(b))
#define X87S_SQRT(x) X87s_Sqrt((double)(x))
#define X87S_FILD(type, x) ((double)(x))
#define X87S_LOAD(x) ((double)(x))

#endif

#endif

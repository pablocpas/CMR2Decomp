#ifndef _FIXED_POINT_H
#define _FIXED_POINT_H

// 16.16 fixed point helpers. The original game used inline assembly for the
// 64-bit intermediates (MSVC 6 would otherwise call _allmul/_allshr/_alldiv).
// PORT: the helpers are portable C with the assembly's exact results: 64-bit
// products, results truncated to their low 32 bits, wrapping sums.

// Bits 16..47 of a * b (what "imul; shrd eax, edx, 16" and "shld edx, eax,
// 16" leave in a register).
inline int FixMulBits(int a, int b)
{
    return (int)(long long)(((long long)a * b) >> 16);
}

inline int FixMul(int a, int b)
{
    return FixMulBits(a, b);
}

// (a * b) >> 32, mirroring the original's "imul edx / shrd eax, edx, 16 /
// sar eax, 16" sequence.
inline int FixMulShift32(int a, int b)
{
    return FixMulBits(a, b) >> 16;
}

inline int FixDiv(int a, int b)
{
    return (int)((long long)a * 65536 / b);
}


// GLOBAL: CMR2 0x006e0ef4
extern unsigned short g_sqrtTable[4096];
// GLOBAL: CMR2 0x006e2ef4
extern int g_sinTable[4096];
// GLOBAL: CMR2 0x006e93f4
extern int g_tanTable[4096];
// arctan(i / 512) as a 12-bit angle, 512 entries
// GLOBAL: CMR2 0x006e8ff4
extern unsigned short g_atanTable[512];

struct FixVector {
    int x;
    int y;
    int z;
};

// 4x4 16.16 matrix, row vectors: right / up / forward / position (w unused).
struct FixMatrix {
    FixVector right;    int rw;
    FixVector up;       int uw;
    FixVector forward;  int fw;
    FixVector position; int pw;
};

// Rotation angles: 12-bit (0x400 = 90 degrees), about right / up / forward.
struct FixAngles {
    unsigned short x;
    unsigned short y;
    unsigned short z;
    unsigned short pad;
};

// Orthonormal basis without padding (right / up / forward).
struct FixBasis {
    FixVector right;
    FixVector up;
    FixVector forward;
};

void FixBasis_Rotate(FixBasis *pBasis, unsigned short *pAngles);

// Rodrigues rotation scratch values (defined in SceneNode.cpp)
extern int g_rotSin;
extern int g_rotCos;
extern int g_rotOneMinusCos;
extern int g_rotAxisXX;
extern int g_rotAxisYY;
extern int g_rotAxisZZ;
extern int g_rotAxisXY;
extern int g_rotAxisXZ;
extern int g_rotAxisYZ;

void FixMatrix_Interpolate(FixMatrix *pOut, FixMatrix *pA, FixMatrix *pB, int tRight, int tAxis, int tPos, int mode);
void FixMatrix_FromAxisAngle(FixMatrix *pOut, FixVector *pAxis, int angle);
int FixMatrix_RotateVector(FixVector *pOut, FixVector *pV, FixMatrix *pM);
int FixMatrix_InverseRotateVector(FixVector *pOut, FixVector *pV, FixMatrix *pM);
FixMatrix *FixMatrix_Multiply(FixMatrix *pOut, FixMatrix *pA, FixMatrix *pB);
void FixMatrix_Identity(FixMatrix *pOut);
void FixMatrix_RotationZ(FixMatrix *pOut, short angle);
void FixMatrix_TransformAboutPivot(FixVector *pOut, FixVector *pIn, FixVector *pPivot, FixMatrix *pM);
void FixMatrix_Invert(FixMatrix *pOut, FixMatrix *pIn);
void FixMatrix_TransformPoint(FixVector *pOut, FixVector *pIn, FixMatrix *pM);
void FixMatrix_CopyRotation(FixMatrix *pSrc, FixMatrix *pDst);
void FixMatrix_CopyRotationFrom(FixMatrix *pDst, FixMatrix *pSrc);
void FixMatrix_GetPosition(FixVector *pOut, FixMatrix *pM);
void FixMatrix_GetRight(FixVector *pOut, FixMatrix *pM);
void FixMatrix_GetUp(FixVector *pOut, FixMatrix *pM);
void FixMatrix_GetForward(FixVector *pOut, FixMatrix *pM);
void FixMatrix_SetPosition(FixVector *pV, FixMatrix *pM);
void FixMatrix_SetRight(FixVector *pV, FixMatrix *pM);
void FixMatrix_SetUp(FixVector *pV, FixMatrix *pM);
void FixMatrix_SetForward(FixVector *pV, FixMatrix *pM);
void FixVec_Normalize(FixVector *pOut, FixVector *pIn);
unsigned int FixVec_Length(FixVector *pV);

// Angles are 12-bit (0x1000 = 360 degrees)
#define FixSin(a) g_sinTable[(unsigned short)(a) & 0xfff]
#define FixCos(a) g_sinTable[(unsigned short)(0x400 - (a)) & 0xfff]

// Square root through g_sqrtTable of a non-zero unsigned 16.16 value: the
// table is indexed by the top 11-12 bits, with an even exponent.
inline int FixSqrtFromTable(unsigned int v)
{
    int top = 31 - __builtin_clz(v);
    int exponent = top - 15;
    unsigned int index;
    unsigned int root;

    if (exponent & 1)
        exponent++;
    index = exponent + 4 >= 0 ? v >> (exponent + 4) : v << -(exponent + 4);
    root = g_sqrtTable[index];
    exponent >>= 1;
    return (int)(exponent >= 0 ? root << exponent : root >> -exponent);
}

// Length of a 16.16 vector via a 4096-entry square root table.
inline int FixVecLength(FixVector *v)
{
    unsigned int sum = (unsigned int)FixMulBits(v->x, v->x) + (unsigned int)FixMulBits(v->y, v->y) +
                       (unsigned int)FixMulBits(v->z, v->z);

    return sum == 0 ? 0 : FixSqrtFromTable(sum);
}

// Square root of a 16.16 value via the same table as FixVecLength.
inline int FixSqrt(int v)
{
    return v == 0 ? 0 : FixSqrtFromTable((unsigned int)v);
}

// out = src * t
inline void FixVecScale(FixVector *out, FixVector *src, int t)
{
    out->x = FixMulBits(src->x, t);
    out->y = FixMulBits(src->y, t);
    out->z = FixMulBits(src->z, t);
}

// a . b
inline int FixVecDot(FixVector *a, FixVector *b)
{
    return (int)((unsigned int)FixMulBits(a->x, b->x) + (unsigned int)FixMulBits(a->y, b->y) +
                 (unsigned int)FixMulBits(a->z, b->z));
}

// out = a x b
inline void FixVecCross(FixVector *out, FixVector *a, FixVector *b)
{
    // Component by component, reading after each store as the original
    // does (out may alias a or b).
    out->x = FixMulBits(a->y, b->z) - FixMulBits(a->z, b->y);
    out->y = FixMulBits(a->z, b->x) - FixMulBits(a->x, b->z);
    out->z = FixMulBits(a->x, b->y) - FixMulBits(a->y, b->x);
}

// Angle of the vector (x, y) as a 12-bit angle (0x400 = 90 degrees).
inline short FixAtan2(int y, int x)
{
    int flags = 0;
    int num;
    int den;
    unsigned int index;
    unsigned short angle;

    // As in the original, y == 0 gives 0 and x == 0 gives 0x400 whatever
    // the other sign.
    if (y == 0)
        return 0;
    if (x == 0)
        return 0x400;
    if (y < 0) {
        y = -y;
        flags ^= 1;
    }
    if (x < 0) {
        x = -x;
        flags ^= 1;
    }
    if (y > x) {
        num = x;
        den = y;
        flags |= 2;
    } else {
        num = y;
        den = x;
    }
    index = (unsigned int)(int)((long long)num * 65536 / den) >> 7;
    // |y| == |x| reads one entry past g_atanTable, the low word of
    // g_tanTable[0] in the original's memory layout.
    angle = index < 512 ? g_atanTable[index] : (unsigned short)g_tanTable[0];
    if (flags & 2)
        angle = (unsigned short)(0x400 - angle);
    if (flags & 1)
        angle = (unsigned short)-angle;
    return (short)angle;
}

// out = in * (1 / len)
inline void FixVecScaleRecip(FixVector *out, FixVector *src, int len)
{
    int recip = (int)((1LL << 32) / len);

    out->x = FixMulBits(src->x, recip);
    out->y = FixMulBits(src->y, recip);
    out->z = FixMulBits(src->z, recip);
}

#ifndef FIX_ABS
#define FIX_ABS(x) ((x) < 0 ? -(x) : (x))

#define FIX_NORMALIZE_INTO(out, v)                                                  \
    {                                                                               \
        int len = FixVecLength(&v);                                                 \
        if (len == 0) {                                                             \
            out.x = 0;                                                              \
            out.y = 0;                                                              \
            out.z = 0;                                                              \
        } else {                                                                    \
            FixVecScaleRecip(&out, &v, len);                                        \
        }                                                                           \
    }
#endif

#endif

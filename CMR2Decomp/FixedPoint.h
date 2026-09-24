#ifndef _FIXED_POINT_H
#define _FIXED_POINT_H

// 16.16 fixed point helpers. The original game used inline assembly for the
// 64-bit intermediates (MSVC 6 would otherwise call _allmul/_allshr/_alldiv).

inline int FixMul(int a, int b)
{
    __asm mov eax, a
    __asm mov edx, b
    __asm imul edx
    __asm shrd eax, edx, 16
}

// (a * b) >> 32, mirroring the original's "imul edx / shrd eax, edx, 16 /
// sar eax, 16" sequence.
inline int FixMulShift32(int a, int b)
{
    __asm mov eax, a
    __asm mov edx, b
    __asm imul edx
    __asm shrd eax, edx, 16
    __asm sar eax, 16
}

inline int FixDiv(int a, int b)
{
    __asm mov eax, a
    __asm mov ecx, b
    __asm cdq
    __asm shld edx, eax, 16
    __asm shl eax, 16
    __asm idiv ecx
}


// GLOBAL: CMR2 0x006e0ef4
extern unsigned short g_sqrtTable[4096];
// GLOBAL: CMR2 0x006e2ef4
extern int g_sinTable[4096];
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

void FixMatrix_FromAxisAngle(FixMatrix *pOut, FixVector *pAxis, int angle);
void FixMatrix_Identity(FixMatrix *pOut);
void FixMatrix_RotationZ(FixMatrix *pOut, unsigned int angle);
void FixMatrix_TransformPoint(FixVector *pOut, FixVector *pIn, FixMatrix *pM);
void FixMatrix_TransformAboutPivot(FixVector *pOut, FixVector *pIn, FixVector *pPivot, FixMatrix *pM);
void FixMatrix_CopyRotationFrom(FixMatrix *pDst, FixMatrix *pSrc);
unsigned int FixVec_Length(FixVector *pV);
void FixVec_Normalize(FixVector *pOut, FixVector *pIn);
void FixMatrix_Interpolate(FixMatrix *pOut, FixMatrix *pA, FixMatrix *pB, int tRight, int tAxis, int tPos, int mode);
void FixMatrix_Multiply(FixMatrix *pOut, FixMatrix *pA, FixMatrix *pB);
int FixMatrix_RotateVector(FixVector *pOut, FixVector *pV, FixMatrix *pM);
int FixMatrix_InverseRotateVector(FixVector *pOut, FixVector *pV, FixMatrix *pM);
void FixMatrix_CopyRotation(FixMatrix *pSrc, FixMatrix *pDst);
void FixMatrix_GetPosition(FixVector *pOut, FixMatrix *pM);
void FixMatrix_GetRight(FixVector *pOut, FixMatrix *pM);
void FixMatrix_GetUp(FixVector *pOut, FixMatrix *pM);
void FixMatrix_GetForward(FixVector *pOut, FixMatrix *pM);
void FixMatrix_SetPosition(FixVector *pV, FixMatrix *pM);
void FixMatrix_SetRight(FixVector *pV, FixMatrix *pM);
void FixMatrix_SetUp(FixVector *pV, FixMatrix *pM);
void FixMatrix_SetForward(FixVector *pV, FixMatrix *pM);

// Angles are 12-bit (0x1000 = 360 degrees)
#define FixSin(a) g_sinTable[(unsigned short)(a) & 0xfff]
#define FixCos(a) g_sinTable[(unsigned short)(0x400 - (a)) & 0xfff]

// Length of a 16.16 vector via a 4096-entry square root table.
inline int FixVecLength(FixVector *v)
{
    __asm {
        mov ecx, v
        mov eax, [ecx]
        imul eax
        shld edx, eax, 16
        mov ebx, edx
        mov eax, [ecx + 4]
        imul eax
        shld edx, eax, 16
        add ebx, edx
        mov eax, [ecx + 8]
        imul eax
        shld edx, eax, 16
        add ebx, edx
        or ebx, ebx
        mov eax, ebx
        jnz nonzero
        mov eax, 0
        jmp done
    nonzero:
        xor ecx, ecx
        cmp eax, 0x10000
        jb l1
        shr eax, 16
        add cl, 16
    l1:
        cmp eax, 0x100
        jb l2
        shr eax, 8
        add cl, 8
    l2:
        cmp eax, 0x10
        jb l3
        shr eax, 4
        add cl, 4
    l3:
        cmp eax, 4
        jb l4
        shr eax, 2
        add cl, 2
    l4:
        cmp eax, 2
        jb l5
        inc ecx
    l5:
        mov eax, ebx
        sub cl, 15
        test cl, 1
        jz l6
        inc cl
    l6:
        mov bl, cl
        add cl, 4
        jns l7
        neg cl
        shl eax, cl
        jmp l8
    l7:
        shr eax, cl
    l8:
        sar bl, 1
        mov ax, word ptr [eax * 2 + g_sqrtTable]
        or bl, bl
        mov cl, bl
        js l9
        shl eax, cl
        jmp done
    l9:
        neg cl
        shr eax, cl
    done:
    }
}

// Square root of a 16.16 value via the same table as FixVecLength.
inline int FixSqrt(int v)
{
    __asm {
        mov eax, v
        or eax, eax
        mov ebx, eax
        jnz nonzero
        mov eax, 0
        jmp done
    nonzero:
        xor ecx, ecx
        cmp eax, 0x10000
        jb l1
        shr eax, 16
        add cl, 16
    l1:
        cmp eax, 0x100
        jb l2
        shr eax, 8
        add cl, 8
    l2:
        cmp eax, 0x10
        jb l3
        shr eax, 4
        add cl, 4
    l3:
        cmp eax, 4
        jb l4
        shr eax, 2
        add cl, 2
    l4:
        cmp eax, 2
        jb l5
        inc ecx
    l5:
        mov eax, ebx
        sub cl, 15
        test cl, 1
        jz l6
        inc cl
    l6:
        mov bl, cl
        add cl, 4
        jns l7
        neg cl
        shl eax, cl
        jmp l8
    l7:
        shr eax, cl
    l8:
        sar bl, 1
        mov ax, word ptr [eax * 2 + g_sqrtTable]
        or bl, bl
        mov cl, bl
        js l9
        shl eax, cl
        jmp done
    l9:
        neg cl
        shr eax, cl
    done:
    }
}

// out = src * t
inline void FixVecScale(FixVector *out, FixVector *src, int t)
{
    __asm {
        mov esi, src
        mov ebx, t
        mov ecx, out
        mov eax, [esi]
        imul ebx
        shld edx, eax, 16
        mov [ecx], edx
        mov eax, [esi + 4]
        imul ebx
        shld edx, eax, 16
        mov [ecx + 4], edx
        mov eax, [esi + 8]
        imul ebx
        shld edx, eax, 16
        mov [ecx + 8], edx
    }
}

// a . b
inline int FixVecDot(FixVector *a, FixVector *b)
{
    __asm {
        mov ecx, a
        mov esi, b
        mov eax, [esi]
        mov edx, [ecx]
        imul edx
        shrd eax, edx, 16
        mov ebx, eax
        mov eax, [esi + 4]
        mov edx, [ecx + 4]
        imul edx
        shrd eax, edx, 16
        add ebx, eax
        mov eax, [esi + 8]
        mov edx, [ecx + 8]
        imul edx
        shrd eax, edx, 16
        add eax, ebx
    }
}

// out = a x b
inline void FixVecCross(FixVector *out, FixVector *a, FixVector *b)
{
    __asm {
        mov edx, a
        mov ecx, out
        mov edi, b
        mov eax, [edx + 4]
        mov esi, edx
        mov edx, [edi + 8]
        imul edx
        shld edx, eax, 16
        mov ebx, edx
        mov eax, [esi + 8]
        mov edx, [edi + 4]
        imul edx
        shld edx, eax, 16
        sub ebx, edx
        mov [ecx], ebx
        mov eax, [esi + 8]
        mov edx, [edi]
        imul edx
        shld edx, eax, 16
        mov ebx, edx
        mov eax, [esi]
        mov edx, [edi + 8]
        imul edx
        shld edx, eax, 16
        sub ebx, edx
        mov [ecx + 4], ebx
        mov eax, [esi]
        mov edx, [edi + 4]
        imul edx
        shld edx, eax, 16
        mov ebx, edx
        mov eax, [esi + 4]
        mov edx, [edi]
        imul edx
        shld edx, eax, 16
        sub ebx, edx
        mov [ecx + 8], ebx
    }
}

// Angle of the vector (x, y) as a 12-bit angle (0x400 = 90 degrees).
inline short FixAtan2(int y, int x)
{
    __asm {
        mov ecx, x
        mov edx, y
        cmp edx, 0
        jnz nonzero
        xor eax, eax
        jmp done
    nonzero:
        cmp ecx, 0
        jnz quadrant
        mov eax, 0x400
        jmp done
    quadrant:
        xor ebx, ebx
        test edx, 0x80000000
        jz ypos
        neg edx
        xor ebx, 1
    ypos:
        test ecx, 0x80000000
        jz xpos
        neg ecx
        xor ebx, 1
    xpos:
        cmp edx, ecx
        jg steep
        mov eax, edx
        jmp divide
    steep:
        mov eax, ecx
        mov ecx, edx
        or ebx, 2
    divide:
        cdq
        shld edx, eax, 16
        shl eax, 16
        idiv ecx
        mov edx, offset g_atanTable
        shr eax, 7
        add edx, eax
        add edx, eax
        mov ax, word ptr [edx]
        test ebx, 2
        jz notsteep
        mov dx, ax
        mov ax, 0x400
        sub ax, dx
    notsteep:
        test ebx, 1
        jz done
        neg ax
    done:
    }
}

// out = in * (1 / len)
inline void FixVecScaleRecip(FixVector *out, FixVector *src, int len)
{
    __asm {
        mov ecx, out
        mov esi, src
        mov ebx, len
        mov edx, 1
        xor eax, eax
        idiv ebx
        mov ebx, eax
        mov eax, [esi]
        imul ebx
        shld edx, eax, 16
        mov [ecx], edx
        mov eax, [esi + 4]
        imul ebx
        shld edx, eax, 16
        mov [ecx + 4], edx
        mov eax, [esi + 8]
        imul ebx
        shld edx, eax, 16
        mov [ecx + 8], edx
    }
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

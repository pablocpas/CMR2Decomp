// Checks the portable fixed-point helpers in game/FixedPoint.h against the
// original inline assembly (tests/fixedpoint_asm_reference.h, the upstream
// header), on random and edge-case inputs. 32-bit x86 only: clang compiles
// the MSVC-style assembly with -fasm-blocks.

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <random>

namespace port {
#include "FixedPoint.h"
unsigned short g_sqrtTable[4096];
int g_sinTable[4096];
int g_tanTable[4096];
unsigned short g_atanTable[512];
}

#undef _FIXED_POINT_H
#undef FIX_ABS
namespace ref {
#include "fixedpoint_asm_reference.h"
unsigned short g_sqrtTable[4096];
int g_sinTable[4096];
int g_tanTable[4096];
unsigned short g_atanTable[512];
}

static std::mt19937 rng(12345);
static int failures = 0;

static int RandomInt()
{
    static const int edges[] = { 0, 1, -1, 2, -2, 0x7fff, 0x8000, 0xffff, 0x10000, -0x10000, 0x7fffffff,
                                 (int)0x80000000, 0x40000000, -0x40000000, 0x12345, -0x12345 };
    switch (rng() % 4) {
    case 0:
        return edges[rng() % (sizeof(edges) / sizeof(edges[0]))];
    case 1:
        return (int)rng() >> (rng() % 31);
    case 2:
        return (int)(rng() % 0x200000) - 0x100000;
    default:
        return (int)rng();
    }
}

#define CHECK(name, a, b, fmt, ...)                                                          \
    do {                                                                                     \
        if ((a) != (b)) {                                                                    \
            if (failures++ < 20)                                                             \
                std::printf("%s mismatch: port %d ref %d (" fmt ")\n", name, (int)(a), (int)(b), \
                            __VA_ARGS__);                                                    \
        }                                                                                    \
    } while (0)

int main()
{
    for (int i = 0; i < 4096; i++) {
        port::g_sqrtTable[i] = ref::g_sqrtTable[i] = (unsigned short)rng();
        port::g_tanTable[i] = ref::g_tanTable[i] = (int)rng();
    }
    for (int i = 0; i < 512; i++)
        port::g_atanTable[i] = ref::g_atanTable[i] = (unsigned short)rng();

    const int iterations = 2000000;
    for (int n = 0; n < iterations; n++) {
        int a = RandomInt(), b = RandomInt(), c = RandomInt();
        int d = RandomInt(), e = RandomInt(), f = RandomInt();

        CHECK("FixMul", port::FixMul(a, b), ref::FixMul(a, b), "%d, %d", a, b);
        CHECK("FixMulShift32", port::FixMulShift32(a, b), ref::FixMulShift32(a, b), "%d, %d", a, b);
        // idiv faults on a zero divisor or an overflowing quotient; the
        // original never divides like that, so neither does the test.
        long long q = b ? (long long)a * 65536 / b : 0;
        if (b != 0 && q >= INT32_MIN && q <= INT32_MAX)
            CHECK("FixDiv", port::FixDiv(a, b), ref::FixDiv(a, b), "%d, %d", a, b);
        CHECK("FixSqrt", port::FixSqrt(a), ref::FixSqrt(a), "%d", a);

        port::FixVector pa = { a, b, c }, pb = { d, e, f }, po;
        ref::FixVector ra = { a, b, c }, rb = { d, e, f }, ro;
        CHECK("FixVecLength", port::FixVecLength(&pa), ref::FixVecLength(&ra), "%d %d %d", a, b, c);
        CHECK("FixVecDot", port::FixVecDot(&pa, &pb), ref::FixVecDot(&ra, &rb), "%d %d %d", a, b, c);

        port::FixVecScale(&po, &pa, d);
        ref::FixVecScale(&ro, &ra, d);
        CHECK("FixVecScale", po.x ^ po.y ^ po.z, ro.x ^ ro.y ^ ro.z, "%d", d);

        port::FixVecCross(&po, &pa, &pb);
        ref::FixVecCross(&ro, &ra, &rb);
        CHECK("FixVecCross", po.x ^ (po.y * 3) ^ (po.z * 7), ro.x ^ (ro.y * 3) ^ (ro.z * 7), "%d", a);
        // In place, out aliasing a.
        po = pa;
        ro = ra;
        port::FixVecCross(&po, &po, &pb);
        ref::FixVecCross(&ro, &ro, &rb);
        CHECK("FixVecCross aliased", po.x ^ (po.y * 3) ^ (po.z * 7), ro.x ^ (ro.y * 3) ^ (ro.z * 7), "%d", a);

        long long recip = d ? (1LL << 32) / d : 0;
        if (d != 0 && recip >= INT32_MIN && recip <= INT32_MAX) {
            port::FixVecScaleRecip(&po, &pa, d);
            ref::FixVecScaleRecip(&ro, &ra, d);
            CHECK("FixVecScaleRecip", po.x ^ po.y ^ po.z, ro.x ^ ro.y ^ ro.z, "%d", d);
        }

        // |y| == |x| reads past g_atanTable, which only the original's
        // memory layout defines; it is checked separately below.
        int y = RandomInt(), x = RandomInt();
        if (y != INT32_MIN && x != INT32_MIN && std::abs(y) != std::abs(x))
            CHECK("FixAtan2", port::FixAtan2(y, x), ref::FixAtan2(y, x), "%d, %d", y, x);
    }

    CHECK("FixAtan2 diagonal", port::FixAtan2(5, 5), (short)(unsigned short)port::g_tanTable[0], "%d", 5);
    CHECK("FixAtan2 diagonal", port::FixAtan2(-7, 7), (short)-(unsigned short)port::g_tanTable[0], "%d", 7);

    std::printf("%s: %d mismatches over %d iterations\n", failures ? "FAIL" : "ok", failures, iterations);
    return failures ? 1 : 0;
}

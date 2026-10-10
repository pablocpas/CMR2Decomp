// MSVC 6 C runtime behaviour, see port/msvcrt.h.

#include "port/types.h"
#include "port/msvcrt.h"

#include <ctype.h>

static unsigned int s_randSeed = 1;

extern "C" int Crt_Rand(void)
{
    s_randSeed = s_randSeed * 214013u + 2531011u;
    return (int)((s_randSeed >> 16) & 0x7fff);
}

extern "C" void Crt_Srand(unsigned int seed)
{
    s_randSeed = seed;
}

extern "C" unsigned int Crt_GetRandState(void) { return s_randSeed; }

extern "C" char *Crt_Strlwr(char *s)
{
    for (char *p = s; *p; p++)
        *p = (char)tolower((unsigned char)*p);
    return s;
}

extern "C" char *Crt_Strupr(char *s)
{
    for (char *p = s; *p; p++)
        *p = (char)toupper((unsigned char)*p);
    return s;
}

extern "C" char *Crt_Itoa(int value, char *buffer, int radix)
{
    char digits[34];
    int n = 0;
    char *out = buffer;
    // MSVC prints negative values with a sign only in base 10.
    unsigned int v = (radix == 10 && value < 0) ? 0u - (unsigned int)value : (unsigned int)value;

    if (radix == 10 && value < 0)
        *out++ = '-';
    do {
        unsigned int d = v % (unsigned int)radix;
        digits[n++] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
        v /= (unsigned int)radix;
    } while (v != 0);
    while (n > 0)
        *out++ = digits[--n];
    *out = 0;
    return buffer;
}

static void CopyPart(char *dst, size_t size, const char *begin, const char *end)
{
    if (dst == NULL)
        return;
    size_t n = (size_t)(end - begin);
    if (n >= size)
        n = size - 1;
    memcpy(dst, begin, n);
    dst[n] = 0;
}

extern "C" void Crt_SplitPath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
    const char *p = path;
    const char *lastSlash = NULL;
    const char *lastDot = NULL;

    if (p[0] && p[1] == ':') {
        CopyPart(drive, _MAX_DRIVE, p, p + 2);
        p += 2;
    } else if (drive != NULL) {
        drive[0] = 0;
    }
    for (const char *q = p; *q; q++) {
        if (*q == '\\' || *q == '/')
            lastSlash = q;
        else if (*q == '.')
            lastDot = q;
    }
    const char *nameStart = lastSlash ? lastSlash + 1 : p;
    CopyPart(dir, _MAX_DIR, p, nameStart);
    if (lastDot != NULL && lastDot >= nameStart) {
        CopyPart(fname, _MAX_FNAME, nameStart, lastDot);
        CopyPart(ext, _MAX_EXT, lastDot, lastDot + strlen(lastDot));
    } else {
        CopyPart(fname, _MAX_FNAME, nameStart, nameStart + strlen(nameStart));
        if (ext != NULL)
            ext[0] = 0;
    }
}

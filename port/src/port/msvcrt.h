/*
 * The MSVC 6 C runtime behaviour the decompiled game depends on.
 *
 * Force-included after types.h. rand() must be the MSVC generator: the game
 * seeds it (srand(400) in stage objects) and draws gameplay values from it,
 * and glibc's rand() is a different generator with a different RAND_MAX.
 * The other names are MSVC spellings of C library functions.
 */
#ifndef OPENCMR2_PORT_MSVCRT_H
#define OPENCMR2_PORT_MSVCRT_H

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MSVC's linear congruential generator, 15-bit results. */
int Crt_Rand(void);
void Crt_Srand(unsigned int seed);
/* Read-only diagnostic view; does not draw a random value. */
unsigned int Crt_GetRandState(void);
#undef RAND_MAX
#define RAND_MAX 0x7fff
#define rand Crt_Rand
#define srand Crt_Srand

char *Crt_Strlwr(char *s);
char *Crt_Strupr(char *s);
char *Crt_Itoa(int value, char *buffer, int radix);
/* Splits a DOS path ('\\' or '/' separators, optional drive letter). */
void Crt_SplitPath(const char *path, char *drive, char *dir, char *fname, char *ext);

#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define stricmp strcasecmp
#define _strlwr Crt_Strlwr
#define _strupr Crt_Strupr
#define _itoa Crt_Itoa
#define _splitpath Crt_SplitPath
#define _snprintf snprintf
#define _vsnprintf vsnprintf

#define _MAX_PATH 260
#define _MAX_DRIVE 3
#define _MAX_DIR 256
#define _MAX_FNAME 256
#define _MAX_EXT 256

#ifdef __cplusplus
}
#endif

#endif

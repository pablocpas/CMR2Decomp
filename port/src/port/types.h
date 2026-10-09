/*
 * Plain data types the decompiled game uses under their Win32 names.
 *
 * This replaces <windows.h> in the game source. It only carries type names
 * and a few constants with Win32's sizes (DWORD and LONG are 32 bits on
 * every target); there is no Win32 API here. Platform services live in the
 * other src/port headers.
 */
#ifndef OPENCMR2_PORT_TYPES_H
#define OPENCMR2_PORT_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t BYTE, UCHAR;
typedef char CHAR;
typedef int16_t SHORT;
typedef uint16_t WORD, USHORT;
typedef int32_t LONG, INT32;
typedef uint32_t DWORD, ULONG, UINT32;
typedef int INT, BOOL;
typedef unsigned int UINT;
typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;
typedef float FLOAT;

typedef void *PVOID, *LPVOID;
typedef const void *LPCVOID;
typedef char *LPSTR, *PSTR;
typedef const char *LPCSTR, *PCSTR;
typedef BYTE *PBYTE, *LPBYTE;
typedef WORD *LPWORD;
typedef DWORD *LPDWORD, *PDWORD;
typedef LONG *PLONG;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#define MAX_PATH 260

typedef struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *LPRECT;

typedef struct tagPOINT {
    LONG x;
    LONG y;
} POINT;

#define MAKEFOURCC(a, b, c, d) \
    ((DWORD)(BYTE)(a) | ((DWORD)(BYTE)(b) << 8) | ((DWORD)(BYTE)(c) << 16) | ((DWORD)(BYTE)(d) << 24))

/* MSVC spellings used by the decompiled code. */
#if !defined(_MSC_VER)
#define __int64 long long
#define __stdcall
#define __cdecl
#endif

#endif

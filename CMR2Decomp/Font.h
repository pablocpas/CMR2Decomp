#ifndef _FONT_H
#define _FONT_H

#include "GenericFileLoader.h"
#include "Texture.h"

// Bitmap font (.pcf data + .tga texture). The .pcf file is a header, a
// 256-entry character map and the glyph table; kerning pairs are relocated
// in place at load time.

struct FontKernPair {
    BYTE ch;                // 0x0  following character
    BYTE pad;
    short offset;           // 0x2
};

struct FontGlyph {
    BYTE field_0x0[4];
    short width;            // 0x4
    short height;           // 0x6
    BYTE field_0x8;
    BYTE kernCount;         // 0x9
    BYTE field_0xa[2];
    FontKernPair *pKern;    // 0xc  file offset until Font_Setup relocates it
};

struct FontHeader {
    int field_0x0;
    unsigned int glyphCount; // 0x4
    int lineHeight;         // 0x8
    int spaceWidth;         // 0xc
    int charSpacing;        // 0x10
    int lineGap;            // 0x14
};

struct FontSlot {
    FontHeader *pHeader;    // 0x0
    FontGlyph *pGlyphs;     // 0x4
    Texture *pTexture;      // 0x8
    short *pCharMap;        // 0xc  char -> glyph index, -1 = missing
    void *pData;            // 0x10 the .pcf file buffer
    BYTE loaded;            // 0x14
    BYTE ownsData;          // 0x15
    BYTE pad[2];
};

// Text flags for Font_DrawText
#define FONT_ALIGN_RIGHT    0x4
#define FONT_ALIGN_CENTRE   0x2
#define FONT_ORIGIN_MIDDLE  0x10
#define FONT_ORIGIN_BOTTOM  0x20

// GLOBAL: CMR2 0x0053223c
extern FontSlot *g_fonts;
// GLOBAL: CMR2 0x00532240
extern unsigned int g_fontCount;
// GLOBAL: CMR2 0x00532244
extern FontSlot *g_pCurrentFont;
// GLOBAL: CMR2 0x00532248
extern int g_fontColour[2];
// GLOBAL: CMR2 0x00532138
extern int g_fontBlendMode;

int Font_InitTable(unsigned int count);
BYTE Font_ReleaseAll(void);
void Font_Release(BYTE index);
void Font_Setup(void *pData, Texture *pTexture, unsigned int index, char bInArchive);
void Font_Load(char *name, GenericFile *pFile, unsigned int index);
void Font_Reload(char *name, GenericFile *pFile, unsigned int index);
void Font_Select(unsigned int index, int *pColour);
int Font_GetTextWidth(unsigned int index, BYTE *text);
int Font_GetTextHeight(unsigned int index, char *text);
int Font_GetLineHeight(unsigned int index);
void Font_DrawChar(unsigned int ch, short x, short y);
void Font_DrawText(unsigned int index, char *text, int x, unsigned int y, int *pColour, unsigned int flags);
void Font_SetBlendMode(int mode);
int Font_Unused(int unused1, int unused2);

#endif

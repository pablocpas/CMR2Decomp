#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Font.h"
#include "FileBuffer.h"
#include "Game.h"
#include "Graphics.h"
#include "InstallInfo.h"
#include "Sprite.h"

FontSlot *g_fonts;
unsigned int g_fontCount;
FontSlot *g_pCurrentFont;
int g_fontColour[2];
int g_fontBlendMode;

// Destination rectangle of the glyph being drawn (x, y, w, h)
// GLOBAL: CMR2 0x00532130
short g_fontCharRect[4];

// GLOBAL: CMR2 0x005168dc
char g_fontTgaFormat[12] = "%s\\%s.tga";
// GLOBAL: CMR2 0x005168d4
char g_fontPcfExtension[8] = ".pcf";
// GLOBAL: CMR2 0x005168e8
char g_fontPcfFormat[12] = "%s\\%s.pcf";

// FUNCTION: CMR2 0x0040b220
int Font_InitTable(unsigned int count)
{
    g_fontBlendMode = 1;
    g_fonts = (FontSlot *)CFileBuffer::AllocateLockedBuffer((count & 0xff) * sizeof(FontSlot));
    if (g_fonts != NULL) {
        memset(g_fonts, 0, (unsigned char)count * sizeof(FontSlot));
        g_fontCount = count & 0xff;
        return CGame::RegisterCallback(Font_ReleaseAll, NULL);
    }
    return -1;
}

// Exit callback registered by Font_InitTable.
// FUNCTION: CMR2 0x0040b290
BYTE Font_ReleaseAll(void)
{
    BYTE i;

    for (i = 0; i < g_fontCount; i++)
        Font_Release(i);
    CFileBuffer::FreeGenericFileBuffer(g_fonts);
    g_fonts = NULL;
    g_fontCount = 0;
    return 1;
}

// FUNCTION: CMR2 0x0040b2f0
void Font_Release(BYTE index)
{
    if (g_fonts[index & 0xff].ownsData != 0) {
        CFileBuffer::FreeGenericFileBuffer(g_fonts[index & 0xff].pData);
        g_fonts[index & 0xff].pData = NULL;
        g_fonts[index & 0xff].pGlyphs = NULL;
        return;
    }
    g_fonts[index & 0xff].pGlyphs = NULL;
}

// Binds a loaded .pcf buffer and its texture to a font slot and relocates
// the kerning pointers of every glyph.
// FUNCTION: CMR2 0x0040b350
void Font_Setup(void *pData, Texture *pTexture, unsigned int index, char bInArchive)
{
    unsigned int i;
    char *p;

    if (index < g_fontCount && pData != NULL && pTexture != NULL) {
        g_fonts[index].pData = pData;
        g_fonts[index].pTexture = pTexture;
        g_fonts[index].pHeader = (FontHeader *)g_fonts[index].pData;
        g_fonts[index].ownsData = bInArchive == 0;
        if (g_fonts[index].pHeader->glyphCount >= 1) {
            p = (char *)g_fonts[index].pData + sizeof(FontHeader);
            g_fonts[index].pCharMap = (short *)p;
            p += 256 * sizeof(short);
            g_fonts[index].pGlyphs = (FontGlyph *)p;
            i = 0;
            if (g_fonts[index].pHeader->glyphCount > 0) {
                do {
                    g_fonts[index].pGlyphs[(unsigned short)i].pKern = (FontKernPair *)((char *)g_fonts[index].pGlyphs[(unsigned short)i].pKern + (int)g_fonts[index].pData);
                    i++;
                } while ((i & 0xffff) < (unsigned int)g_fonts[index].pHeader->glyphCount);
            }
            g_fonts[index].loaded = 1;
        }
    }
}

// FUNCTION: CMR2 0x0040b430
void Font_Load(char *name, GenericFile *pFile, unsigned int index)
{
    Texture *pTexture;
    void *pData;
    BYTE bInArchive;
    char path[260];

    if (index < g_fontCount) {
        sprintf(path, g_fontTgaFormat, CInstallInfo::GetFontsDir(), name);
        pTexture = CTexture::FindLoadTexture(pFile, path, NULL, NULL, 0, 0);
        if (pTexture != NULL) {
            sprintf(path + strlen(path) - 4, g_fontPcfExtension);
            pData = CGenericFileLoader::FindFile(pFile, path, &bInArchive, NULL, 0);
            if (pData != NULL)
                Font_Setup(pData, pTexture, index, bInArchive);
        }
    }
}

// Reloads only the .pcf data of a font, keeping its texture.
// FUNCTION: CMR2 0x0040b4f0
void Font_Reload(char *name, GenericFile *pFile, unsigned int index)
{
    Texture *pTexture;
    void *pData;
    BYTE bInArchive;
    char path[260];

    if (index < g_fontCount) {
        pTexture = g_fonts[index].pTexture;
        Font_Release(index);
        sprintf(path, g_fontPcfFormat, CInstallInfo::GetFontsDir(), name);
        pData = CGenericFileLoader::FindFile(pFile, path, &bInArchive, NULL, 0);
        if (pData != NULL)
            Font_Setup(pData, pTexture, index, bInArchive);
    }
}

// FUNCTION: CMR2 0x0040b580
void Font_Select(unsigned int index, int *pColour)
{
    g_pCurrentFont = &g_fonts[index & 0xff];
    g_fontColour[0] = *pColour;
}

// Width in pixels of the widest line of text (lines end at '\n' or '^').
// match 26%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040b5b0
int Font_GetTextWidth(unsigned int index, BYTE *text)
{
    short maxWidth;
    int charSpacing;
    short advance;
    int i;
    BYTE *p;
    short spaceWidth;
    FontGlyph *pGlyphs;
    short *pCharMap;
    FontSlot *pFont;
    short lineWidth;
    short prevWidth;
    short glyph;
    BYTE ch;
    FontGlyph *pGlyph;
    FontKernPair *pKern;
    int k;
    int kernCount;

    lineWidth = 0;
    prevWidth = 0;
    maxWidth = 0;
    pFont = &g_fonts[index & 0xff];
    pCharMap = pFont->pCharMap;
    spaceWidth = (short)pFont->pHeader->spaceWidth;
    charSpacing = pFont->pHeader->charSpacing;
    pGlyphs = pFont->pGlyphs;
    if (pFont->loaded != 0) {
        ch = *text;
        i = 0;
        if (ch != 0) {
            p = text;
            do {
                if (ch == '\n' || ch == '^')
                    break;
                if (ch == ' ') {
                    lineWidth += spaceWidth;
                    prevWidth = lineWidth;
                } else {
                    glyph = pCharMap[ch];
                    if (glyph != -1) {
                        pGlyph = &pGlyphs[glyph];
                        advance = (short)charSpacing;
                        if (text[i + 1] != 0 && text[i + 1] != '\n' && text[i + 1] != '^' && pCharMap[text[i + 1]] != -1) {
                            pKern = pGlyph->pKern;
                            kernCount = pGlyph->kernCount;
                            k = 0;
                            if (kernCount > 0) {
                                do {
                                    if (pKern->ch >= p[1]) {
                                        if (pKern->ch == text[i + 1])
                                            advance = pGlyph->pKern[k].offset + (short)charSpacing;
                                        break;
                                    }
                                    k++;
                                    pKern++;
                                } while (k < kernCount);
                            }
                        }
                        lineWidth = prevWidth + pGlyph->width + advance;
                        prevWidth = lineWidth;
                    }
                }
                if ((text[i + 1] == 0 || text[i + 1] == '\n' || text[i + 1] == '^') && lineWidth > maxWidth)
                    maxWidth = lineWidth;
                i++;
                p = text + i;
                ch = *p;
            } while (ch != 0);
        }
        return maxWidth;
    }
    return 0;
}

// match 40%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040b730
int Font_GetTextHeight(unsigned int index, char *text)
{
    int height;
    int lines;
    char ch;
    FontHeader *pHeader;

    height = 0;
    lines = 1;
    if (g_fonts[index & 0xff].loaded != 0) {
        for (ch = *text; ch != 0; ch = *++text) {
            if (ch == '\n' || ch == '^')
                lines++;
        }
        pHeader = g_fonts[index & 0xff].pHeader;
        height = (pHeader->lineHeight + 1) * lines - 1 + pHeader->lineGap;
    }
    return height;
}

// FUNCTION: CMR2 0x0040b790
int Font_GetLineHeight(unsigned int index)
{
    FontSlot *pFont;

    pFont = &g_fonts[index & 0xff];
    if (pFont->loaded != 0)
        return pFont->pHeader->lineHeight;
    return 0;
}

// FUNCTION: CMR2 0x0040b7c0
void Font_DrawChar(unsigned int ch, short x, short y)
{
    FontSlot *pFont;
    FontGlyph *pGlyph;
    Texture *pTexture;
    int centre[3];

    pFont = g_pCurrentFont;
    pGlyph = &pFont->pGlyphs[pFont->pCharMap[ch & 0xff]];
    pTexture = pFont->pTexture;
    centre[0] = 0;
    centre[1] = 0;
    centre[2] = 0;
    g_fontCharRect[0] = x;
    g_fontCharRect[1] = y;
    g_fontCharRect[2] = pGlyph->width;
    g_fontCharRect[3] = pGlyph->height;
    centre[0] = g_fontCharRect[2] / 2 + x;
    centre[1] = g_fontCharRect[3] / 2 + y;
    Sprite_Queue((SpriteRect *)pGlyph, (SpriteRect *)g_fontCharRect, pTexture, g_fontBlendMode, 0, centre, NULL, (BYTE *)g_fontColour, 8);
}

// Draws text at (x, y). flags: 2 = centre, 4 = right-align (both per line),
// 0x10 = y is the vertical middle of the first line, 0x20 = y is its bottom.
// match 27%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040b880
void Font_DrawText(unsigned int index, char *text, int x, unsigned int y, int *pColour, unsigned int flags)
{
    FontSlot *pFont;
    int length;
    int i;
    int k;
    int penX;
    short advance;
    short glyph;
    BYTE ch;
    FontGlyph *pGlyph;
    FontKernPair *pKern;

    pFont = &g_fonts[index & 0xff];
    rand();
    rand();
    rand();
    if (pFont->loaded != 0 && (index & 0xff) <= g_fontCount) {
        length = strlen(text);
        penX = x;
        if (flags & (FONT_ALIGN_CENTRE | FONT_ALIGN_RIGHT)) {
            penX = Font_GetTextWidth(index, (BYTE *)text);
            if (flags & FONT_ALIGN_CENTRE)
                penX /= 2;
            penX = x - penX;
        }
        if (flags & FONT_ORIGIN_MIDDLE)
            y -= pFont->pGlyphs->field_0xa[0];
        if (flags & FONT_ORIGIN_BOTTOM)
            y = (unsigned short)((short)y - (short)pFont->pHeader->lineHeight);
        Font_Select(index, pColour);
        for (i = 0; i < length; i++) {
            ch = text[i];
            if (ch == ' ') {
                penX += (short)pFont->pHeader->spaceWidth;
            } else if (ch == '\n' || ch == '^') {
                penX = x;
                if (flags & (FONT_ALIGN_CENTRE | FONT_ALIGN_RIGHT)) {
                    penX = Font_GetTextWidth(index, (BYTE *)text + i + 1);
                    if (flags & FONT_ALIGN_CENTRE)
                        penX /= 2;
                    penX = x - penX;
                }
                y += (short)(pFont->pHeader->lineGap + pFont->pHeader->lineHeight);
            } else {
                glyph = pFont->pCharMap[ch];
                if (glyph != -1) {
                    advance = (short)pFont->pHeader->charSpacing;
                    pGlyph = &pFont->pGlyphs[glyph];
                    if (i < length - 1 && pFont->pCharMap[(BYTE)text[i + 1]] != -1) {
                        k = 0;
                        if (pGlyph->kernCount != 0) {
                            pKern = pGlyph->pKern;
                            do {
                                if ((BYTE)text[i + 1] <= pKern->ch) {
                                    if (pGlyph->pKern[k].ch == (BYTE)text[i + 1])
                                        advance += pGlyph->pKern[k].offset;
                                    break;
                                }
                                k++;
                                pKern++;
                            } while (k < pGlyph->kernCount);
                        }
                    }
                    Font_DrawChar(text[i], (short)penX, (short)y);
                    penX += (short)(pGlyph->width + advance);
                }
            }
        }
    }
}

// FUNCTION: CMR2 0x0040bab0
void Font_SetBlendMode(int mode)
{
    g_fontBlendMode = mode;
}

// FUNCTION: CMR2 0x0040bac0
int Font_Unused(int unused1, int unused2)
{
    return 0;
}

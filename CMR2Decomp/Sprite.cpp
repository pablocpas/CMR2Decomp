#include <windows.h>
#include "Sprite.h"
#include "Game.h"
#include "Graphics.h"

Sprite g_spriteLayer1[SPRITE_LAYER_MAX];
Sprite g_spriteLayer2[SPRITE_LAYER_MAX];
Sprite g_spriteLayer3[SPRITE_LAYER_MAX];
Sprite g_spriteLayer4[SPRITE_LAYER_MAX];
unsigned int g_spriteCount1;
unsigned int g_spriteCount2;
unsigned int g_spriteCount3;
unsigned int g_spriteCount4;
// GLOBAL: CMR2 0x0065a958
int g_spriteEnabled;

Tri2D g_tri2DLayer1[TRI2D_LAYER_MAX];
Tri2D g_tri2DLayer2[TRI2D_LAYER_MAX];
Tri2D g_tri2DLayer3[TRI2D_LAYER_MAX];
Tri2D g_tri2DLayer4[TRI2D_LAYER_MAX];
int g_tri2DEnabled;
unsigned int g_tri2DCount1;
unsigned int g_tri2DCount2;
unsigned int g_tri2DCount3;
unsigned int g_tri2DCount4;
// GLOBAL: CMR2 0x0081616c
int g_unk0x0081616c;
// GLOBAL: CMR2 0x00816170
int g_unk0x00816170;

// Exit callback of Sprite_Init.
// FUNCTION: CMR2 0x004a3640
int Sprite_Shutdown(void)
{
    g_spriteEnabled = 0;
    return 1;
}

// FUNCTION: CMR2 0x004a3260
void Sprite_Init(void)
{
    g_spriteEnabled = 1;
    g_spriteCount1 = 0;
    g_spriteCount2 = 0;
    g_spriteCount3 = 0;
    g_spriteCount4 = 0;
    CGame::RegisterCallback(Sprite_Shutdown, NULL);
}

// Exit callback of Tri2D_Init.
// FUNCTION: CMR2 0x004bc090
int Tri2D_Shutdown(void)
{
    g_tri2DCount1 = 0;
    g_tri2DCount2 = 0;
    g_tri2DCount4 = 0;
    g_unk0x00816170 = 0;
    g_unk0x0081616c = 0;
    g_tri2DEnabled = 0;
    return 1;
}

// FUNCTION: CMR2 0x004bb610
void Tri2D_Init(void)
{
    g_tri2DEnabled = 1;
    g_tri2DCount1 = 0;
    g_tri2DCount2 = 0;
    g_tri2DCount3 = 0;
    g_tri2DCount4 = 0;
    g_unk0x0081616c = 0;
    g_unk0x00816170 = 0;
    CGame::RegisterCallback(Tri2D_Shutdown, NULL);
}

// pPos is a 16.16 screen position; the colour bytes are r, g, b, a.
// FUNCTION: CMR2 0x004bb800
void Tri2D_SetVertex(D3DTLVERTEX *pVertex, int *pPos, BYTE *pColour)
{
    pVertex->sx = (float)(pPos[0] * CGraphics::m_oneOver65536);
    pVertex->sy = (float)(pPos[1] * CGraphics::m_oneOver65536);
    pVertex->color = RGBA_MAKE(pColour[0], pColour[1], pColour[2], pColour[3]);
}

// FUNCTION: CMR2 0x004bb650
void Tri2D_Queue(int *pA, int *pB, int *pC, BYTE *pColour, int layer)
{
    unsigned int n;

    if (g_tri2DEnabled != 0 && g_tri2DCount4 < TRI2D_LAYER_MAX && g_tri2DCount2 < TRI2D_LAYER_MAX && g_tri2DCount1 < TRI2D_LAYER_MAX) {
        switch (layer) {
        case 2:
            n = g_tri2DCount2;
            Tri2D_SetVertex(&g_tri2DLayer2[n].v[0], pA, pColour);
            Tri2D_SetVertex(&g_tri2DLayer2[n].v[1], pB, pColour);
            Tri2D_SetVertex(&g_tri2DLayer2[n].v[2], pC, pColour);
            g_tri2DCount2++;
            break;
        case 3:
            n = g_tri2DCount3;
            Tri2D_SetVertex(&g_tri2DLayer3[n].v[0], pA, pColour);
            Tri2D_SetVertex(&g_tri2DLayer3[n].v[1], pB, pColour);
            Tri2D_SetVertex(&g_tri2DLayer3[n].v[2], pC, pColour);
            g_tri2DCount3++;
            break;
        case 4:
            n = g_tri2DCount4;
            Tri2D_SetVertex(&g_tri2DLayer4[n].v[0], pA, pColour);
            Tri2D_SetVertex(&g_tri2DLayer4[n].v[1], pB, pColour);
            Tri2D_SetVertex(&g_tri2DLayer4[n].v[2], pC, pColour);
            g_tri2DCount4++;
            break;
        default:
            n = g_tri2DCount1;
            Tri2D_SetVertex(&g_tri2DLayer1[n].v[0], pA, pColour);
            Tri2D_SetVertex(&g_tri2DLayer1[n].v[1], pB, pColour);
            Tri2D_SetVertex(&g_tri2DLayer1[n].v[2], pC, pColour);
            g_tri2DCount1++;
            break;
        }
    }
}

// The original repeats the same block for each of the four layers.
#define QUEUE_SPRITE(arr, count, n)                                             \
    if (count >= SPRITE_LAYER_MAX)                                              \
        return;                                                                 \
    p = &arr[count];                                                            \
    count++;                                                                    \
    p->src = *pSrc;                                                             \
    p->dst = *pDst;                                                             \
    if (pUv2 != NULL) {                                                         \
        p->uv2 = *pUv2;                                                         \
    } else {                                                                    \
        p->uv2.h = 0;                                                           \
        p->uv2.w = 0;                                                           \
        p->uv2.y = 0;                                                           \
        p->uv2.x = 0;                                                           \
    }                                                                           \
    p->pTexture = pTexture;                                                     \
    p->colour[0] = pColour[0];                                                  \
    p->colour[1] = pColour[1];                                                  \
    p->colour[2] = pColour[2];                                                  \
    p->colour[3] = pColour[3];                                                  \
    p->angle = (short)((angleDeg << 12) / 360);                           \
    p->layer = n;                                                               \
    if (pCentre != NULL) {                                                      \
        p->centre[0] = pCentre[0];                                              \
        p->centre[1] = pCentre[1];                                              \
        p->centre[2] = pCentre[2];                                              \
    } else {                                                                    \
        p->centre[0] = 0;                                                       \
        p->centre[1] = 0;                                                       \
        p->centre[2] = 0;                                                       \
    }                                                                           \
    p->param = param;

// Appends a sprite to one of the four 2D layers.
// FUNCTION: CMR2 0x004a3290
void Sprite_Queue(SpriteRect *pSrc, SpriteRect *pDst, Texture *pTexture, int layer, short angleDeg, int *pCentre, SpriteRect *pUv2, BYTE *pColour, int param)
{
    Sprite *p;

    if (layer == 1) {
        QUEUE_SPRITE(g_spriteLayer1, g_spriteCount1, 1)
        return;
    }
    if (layer == 2) {
        QUEUE_SPRITE(g_spriteLayer2, g_spriteCount2, 2)
        return;
    }
    if (layer == 3) {
        QUEUE_SPRITE(g_spriteLayer3, g_spriteCount3, 3)
        return;
    }
    if (layer == 4) {
        QUEUE_SPRITE(g_spriteLayer4, g_spriteCount4, 4)
        return;
    }
}

// GLOBAL: CMR2 0x005112e8
double g_minus65536 = -65536.0;

// Fills a screen rectangle (x, y, w, h in pixels) with two 2D triangles,
// clipping it to the screen first.
// FUNCTION: CMR2 0x004a5e40
int Sprite_FillRect(int unused, short *pRect, BYTE *pColour, int layer)
{
    short x;
    short y;
    short w;
    short h;
    int c[2];
    int a[2];
    int b[2];
    int x0;
    int y0;
    int x1;
    int y1;

    w = pRect[2];
    x = pRect[0];
    y = pRect[1];
    h = pRect[3];
    if (w > 0 && h > 0 && (x >= 0 || w + x >= 0) && (y >= 0 || h + y >= 0)) {
        if (x < (int)g_pGraphics->resX && y < (int)g_pGraphics->resY) {
            if (x < 0) {
                w += x;
                x = 0;
            }
            x0 = x;
            if (w + x0 > (int)g_pGraphics->resX)
                w = (short)g_pGraphics->resX - x - 1;
            if (y < 0) {
                h += y;
                y = 0;
            }
            y0 = y;
            if (h + y0 > (int)g_pGraphics->resY)
                h = (short)g_pGraphics->resX - y - 1;
            a[0] = (int)(__int64)((double)x0 * CGraphics::m_65536);
            c[0] = a[0];
            a[1] = (int)(__int64)((double)y0 * CGraphics::m_65536);
            x1 = 0x8000 - (int)(__int64)((double)(w - 1 + x0) * g_minus65536);
            b[0] = x1;
            b[1] = a[1];
            y1 = 0x8000 - (int)(__int64)((double)(h - 1 + y0) * g_minus65536);
            c[1] = y1;
            Tri2D_Queue(a, b, c, pColour, layer);
            a[0] = x1;
            a[1] = y1;
            Tri2D_Queue(b, a, c, pColour, layer);
        }
    }
    return 1;
}

// 2D lines (up to 200): two 16.16 endpoints and two RGBA colours.
struct Line2D {
    int a[3];               // 0x0
    int b[3];               // 0xc
    BYTE colourA[4];        // 0x18
    BYTE colourB[4];        // 0x1c
    BYTE pad[4];
};

// GLOBAL: CMR2 0x0072d680
Line2D g_line2D[200];
// GLOBAL: CMR2 0x00730fc8
unsigned int g_line2DCount;
// GLOBAL: CMR2 0x00730fcc
int g_line2DInitialised;
// GLOBAL: CMR2 0x0072f2a0
int g_unk0x0072f2a0;

// Exit callback of Line2D_Init.
// FUNCTION: CMR2 0x004bb5f0
int Line2D_Shutdown(void)
{
    if (g_line2DInitialised == 0)
        return 0;
    g_line2DCount = 0;
    g_line2DInitialised = 0;
    return 1;
}

// FUNCTION: CMR2 0x004bb280
void Line2D_Init(void)
{
    if (g_line2DInitialised == 0) {
        g_line2DCount = 0;
        g_unk0x0072f2a0 = 0;
        CGame::RegisterCallback(Line2D_Shutdown, NULL);
        g_line2DInitialised = 1;
    }
}

// FUNCTION: CMR2 0x004bb430
void Line2D_Queue(int *pA, int *pB, BYTE *pColourA, BYTE *pColourB)
{
    Line2D *p;

    if (g_line2DCount < 200) {
        p = &g_line2D[g_line2DCount];
        p->a[0] = pA[0];
        p->a[1] = pA[1];
        p->a[2] = pA[2];
        p->b[0] = pB[0];
        p->b[1] = pB[1];
        p->b[2] = pB[2];
        p->colourA[0] = pColourA[0];
        p->colourA[1] = pColourA[1];
        p->colourA[2] = pColourA[2];
        p->colourA[3] = pColourA[3];
        p->colourB[0] = pColourB[0];
        p->colourB[1] = pColourB[1];
        p->colourB[2] = pColourB[2];
        p->colourB[3] = pColourB[3];
        g_line2DCount++;
    }
}

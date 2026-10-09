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
unsigned int g_unk0x0081616c;
// GLOBAL: CMR2 0x00816170
unsigned int g_unk0x00816170;

// Screen-space quads of each sprite layer, four vertices per sprite.
// GLOBAL: CMR2 0x005a2858
D3DTLVERTEX g_spriteVerts1[1024 * 4];
// GLOBAL: CMR2 0x005c2858
D3DTLVERTEX g_spriteVerts2[1024 * 4];
// GLOBAL: CMR2 0x005fe858
D3DTLVERTEX g_spriteVerts4[1024 * 4];
// GLOBAL: CMR2 0x0061e858
D3DTLVERTEX g_spriteVerts3[1024 * 4];

void Graphics_InvalidateTextureStageCache(void);

#define SPRITE_VERTEX(v, px, py, u, vv)                                             \
    (v)->sx = (float)(px);                                                          \
    (v)->sy = (float)(py);                                                          \
    (v)->sz = 0.0f;                                                                 \
    (v)->rhw = 1.0f;                                                                \
    (v)->color = colour;                                                            \
    (v)->specular = 0xff000000;                                                     \
    (v)->tu = (u);                                                                  \
    (v)->tv = (vv);                                                                 \
    if (angle != 0) {                                                               \
        p.x = (int)(__int64)(v)->sx;                                                \
        p.y = (int)(__int64)(v)->sy;                                                \
        p.z = (int)(__int64)(v)->sz;                                                \
        FixMatrix_TransformAboutPivot(&out, &p, (FixVector *)centre, &rotation);    \
        (v)->sx = (float)out.x;                                                     \
        (v)->sy = (float)out.y;                                                     \
        (v)->sz = (float)out.z;                                                     \
    }

// Builds the quads of one sprite layer (1..4), rotating them about their
// centre when needed, draws them with point filtering and empties the layer.
// match 92%: remaining diff is register allocation and scheduling of the integer temporaries.
// FUNCTION: CMR2 0x004a3650
void Sprite_DrawLayer(int layer)
{
    D3DTLVERTEX *pVert;
    Sprite *pSprite;
    unsigned int count;
    unsigned int n;
    float w, h;
    float u0, u1, v0, v1;
    float t;
    D3DCOLOR colour;
    int centre[3];
    short angle;
    FixMatrix rotation;
    FixVector p;
    FixVector out;

    CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_CLIPPING, FALSE);
    switch (layer) {
    case 2:
        count = g_spriteCount2;
        g_spriteCount2 = 0;
        pVert = g_spriteVerts2;
        pSprite = g_spriteLayer2;
        break;
    case 3:
        count = g_spriteCount3;
        g_spriteCount3 = 0;
        pVert = g_spriteVerts3;
        pSprite = g_spriteLayer3;
        break;
    case 4:
        count = g_spriteCount4;
        g_spriteCount4 = 0;
        pVert = g_spriteVerts4;
        pSprite = g_spriteLayer4;
        break;
    default:
        count = g_spriteCount1;
        g_spriteCount1 = 0;
        pVert = g_spriteVerts1;
        pSprite = g_spriteLayer1;
        break;
    }
    for (n = 0; n < count; n++) {
        // A sprite without texture is skipped without advancing (as the original does).
        if (pSprite->pTexture == NULL)
            continue;
        w = (float)(unsigned int)pSprite->pTexture->width;
        u0 = (float)pSprite->src.x / w;
        u1 = (float)(pSprite->src.x + pSprite->src.w) / w;
        h = (float)(unsigned int)pSprite->pTexture->height;
        v0 = (float)pSprite->src.y / h;
        v1 = (float)(pSprite->src.y + pSprite->src.h) / h;
        if (CGraphics::GetSelectedDisplayDriverIndex() == 0) {
            u0 += 0.5f / w;
            u1 += 0.5f / w;
            v0 += 0.5f / h;
            v1 += 0.5f / h;
        }
        if (pSprite->param & 5) {
            t = u0;
            u0 = u1;
            u1 = t;
        }
        if (pSprite->param & 6) {
            t = v0;
            v0 = v1;
            v1 = t;
        }
        colour = RGBA_MAKE(pSprite->colour[0], pSprite->colour[1], pSprite->colour[2], pSprite->colour[3]);
        *(FixVector *)centre = *(FixVector *)pSprite->centre;
        if (centre[0] == -1 && centre[1] == -1 && centre[2] == -1) {
            centre[0] = pSprite->dst.w / 2 + pSprite->dst.x;
            centre[1] = pSprite->dst.h / 2 + pSprite->dst.y;
        }
        angle = pSprite->angle;
        if (angle != 0)
            FixMatrix_RotationZ(&rotation, angle);
        SPRITE_VERTEX(pVert, pSprite->dst.x, pSprite->dst.y, u0, v0);
        pVert++;
        SPRITE_VERTEX(pVert, pSprite->dst.x + pSprite->dst.w, pSprite->dst.y, u1, v0);
        pVert++;
        SPRITE_VERTEX(pVert, pSprite->dst.x, pSprite->dst.y + pSprite->dst.h, u0, v1);
        pVert++;
        SPRITE_VERTEX(pVert, pSprite->dst.x + pSprite->dst.w, pSprite->dst.y + pSprite->dst.h, u1, v1);
        pVert++;
        pSprite++;
    }

    switch (layer) {
    case 2:
        pVert = g_spriteVerts2;
        pSprite = g_spriteLayer2;
        break;
    case 3:
        pVert = g_spriteVerts3;
        pSprite = g_spriteLayer3;
        break;
    case 4:
        pVert = g_spriteVerts4;
        pSprite = g_spriteLayer4;
        break;
    default:
        pVert = g_spriteVerts1;
        pSprite = g_spriteLayer1;
        break;
    }
    CGraphics::m_pTextureManager->pD3D->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
    CGraphics::m_pTextureManager->pD3D->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
    for (n = 0; n < count; n++) {
        CGraphics::ApplyTextureStageChange(0, (int)pSprite[n].pTexture);
        CGraphics::m_pTextureManager->pD3D->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, &pVert[n * 4], 4, 0);
        CGame::m_unk0x0059ce20 += 2;
    }
    CGraphics::m_pTextureManager->pD3D->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
    CGraphics::m_pTextureManager->pD3D->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
    CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_CLIPPING, TRUE);
    Graphics_InvalidateTextureStageCache();
    CGraphics::InvalidateBlendStateCache();
}

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

// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004bb650
void Tri2D_Queue(int *pA, int *pB, int *pC, BYTE *pColour, int layer)
{
    unsigned int n;
    unsigned int i3;

    if (g_tri2DEnabled != 0 && g_tri2DCount4 < TRI2D_LAYER_MAX && g_tri2DCount2 < TRI2D_LAYER_MAX && g_tri2DCount1 < TRI2D_LAYER_MAX) {
        switch (layer) {
        case 2:
            n = g_tri2DCount2;
            i3 = n * 3;
            Tri2D_SetVertex(&((D3DTLVERTEX *)g_tri2DLayer2)[i3 + 0], pA, pColour);
            Tri2D_SetVertex(&((D3DTLVERTEX *)g_tri2DLayer2)[i3 + 1], pB, pColour);
            Tri2D_SetVertex(&((D3DTLVERTEX *)g_tri2DLayer2)[i3 + 2], pC, pColour);
            g_tri2DCount2++;
            break;
        case 3:
            n = g_tri2DCount3;
            i3 = n * 3;
            Tri2D_SetVertex(&((D3DTLVERTEX *)g_tri2DLayer3)[i3 + 0], pA, pColour);
            Tri2D_SetVertex(&((D3DTLVERTEX *)g_tri2DLayer3)[i3 + 1], pB, pColour);
            Tri2D_SetVertex(&((D3DTLVERTEX *)g_tri2DLayer3)[i3 + 2], pC, pColour);
            g_tri2DCount3++;
            break;
        case 4:
            n = g_tri2DCount4;
            i3 = n * 3;
            Tri2D_SetVertex(&((D3DTLVERTEX *)g_tri2DLayer4)[i3 + 0], pA, pColour);
            Tri2D_SetVertex(&((D3DTLVERTEX *)g_tri2DLayer4)[i3 + 1], pB, pColour);
            Tri2D_SetVertex(&((D3DTLVERTEX *)g_tri2DLayer4)[i3 + 2], pC, pColour);
            g_tri2DCount4++;
            break;
        default:
            n = g_tri2DCount1;
            i3 = n * 3;
            Tri2D_SetVertex(&((D3DTLVERTEX *)g_tri2DLayer1)[i3 + 0], pA, pColour);
            Tri2D_SetVertex(&((D3DTLVERTEX *)g_tri2DLayer1)[i3 + 1], pB, pColour);
            Tri2D_SetVertex(&((D3DTLVERTEX *)g_tri2DLayer1)[i3 + 2], pC, pColour);
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
// match 22%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
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

    x = pRect[0];
    y = pRect[1];
    h = pRect[3];
    w = pRect[2];
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
                h = (short)g_pGraphics->resX - y;
                h -= 1;
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

// Screen-space lines, drawn on their layer (see ScreenLine2D_Draw).
struct ScreenLine2D {
    int a[3];               // 0x0
    int b[3];               // 0xc
    BYTE colourA[4];        // 0x18
    BYTE colourB[4];        // 0x1c
    int layer;              // 0x20
};

// GLOBAL: CMR2 0x0072f2a8
ScreenLine2D g_screenLine2D[200];

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

// Vertex of a 3D line (D3DFVF_XYZ | NORMAL | DIFFUSE | SPECULAR | TEX2).
struct Line2DVertex {
    float x, y, z;
    float nx, ny, nz;
    D3DCOLOR diffuse;
    D3DCOLOR specular;
    float u, v;
    float u2, v2;
};

// Draws the queued screen lines of one layer (none for layer 4); layer 1
// also empties the queue.
// match 64%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004bb2b0
void ScreenLine2D_Draw(int layer)
{
    D3DTLVERTEX v[2];
    ScreenLine2D *p;
    unsigned int i;

    if (g_unk0x0072f2a0 == 0 || layer == 4)
        return;
    CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_CLIPPING, FALSE);
    CGraphics::ApplyTextureStageChange(0, 0);
    for (i = 0, p = g_screenLine2D; i < (unsigned int)g_unk0x0072f2a0; i++, p++) {
        if (layer != p->layer)
            continue;
        v[0].sx = (float)p->a[0] * CGraphics::m_oneOver65536;
        v[0].color = RGBA_MAKE(p->colourA[0], p->colourA[1], p->colourA[2], p->colourA[3]);
        v[0].rhw = 1.0f;
        v[0].sy = (float)p->a[1] * CGraphics::m_oneOver65536;
        v[0].specular = RGBA_MAKE(p->colourB[0], p->colourB[1], p->colourB[2], p->colourB[3]);
        v[0].sz = (float)p->a[2] * CGraphics::m_oneOver65536;
        v[0].tv = 0.0f;
        v[0].tu = 0.0f;
        v[1].color = RGBA_MAKE(p->colourB[0], p->colourB[1], p->colourB[2], p->colourB[3]);
        v[1].sx = (float)p->b[0] * CGraphics::m_oneOver65536;
        v[1].rhw = 1.0f;
        v[1].tv = 0.0f;
        v[1].tu = 0.0f;
        v[1].sy = (float)p->b[1] * CGraphics::m_oneOver65536;
        v[1].sz = (float)p->b[2] * CGraphics::m_oneOver65536;
        CGraphics::m_pTextureManager->pD3D->DrawPrimitive(D3DPT_LINELIST, D3DFVF_TLVERTEX, v, 2, 0);
    }
    if (layer == 1)
        g_unk0x0072f2a0 = 0;
    CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_CLIPPING, TRUE);
}

// Draws and empties the queued 3D lines with alpha blending.
// match 76%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004bb4c0
void Line2D_Draw(void)
{
    Line2DVertex v[2];
    Line2D *p;
    unsigned int i;

    if (g_line2DCount == 0)
        return;
    CGraphics::ApplyTextureStageChange(0, 0);
    CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
    CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
    for (i = 0, p = g_line2D; i < g_line2DCount; i++, p++) {
        v[0].x = (float)p->a[0] * CGraphics::m_oneOver65536;
        v[0].y = (float)p->a[1] * CGraphics::m_oneOver65536;
        v[0].diffuse = RGBA_MAKE(p->colourA[0], p->colourA[1], p->colourA[2], p->colourA[3]);
        v[0].z = (float)p->a[2] * CGraphics::m_oneOver65536;
        v[1].diffuse = RGBA_MAKE(p->colourB[0], p->colourB[1], p->colourB[2], p->colourB[3]);
        v[1].x = (float)p->b[0] * CGraphics::m_oneOver65536;
        v[1].y = (float)p->b[1] * CGraphics::m_oneOver65536;
        v[1].z = (float)p->b[2] * CGraphics::m_oneOver65536;
        CGraphics::m_pTextureManager->pD3D->DrawPrimitive(D3DPT_LINELIST, 0x2d2, v, 2, 0);
    }
    g_line2DCount = 0;
}

// Draws and empties one layer (1..4) of queued 2D triangles.
// FUNCTION: CMR2 0x004bb850
void Tri2D_DrawLayer(int layer)
{
    CGraphics::SetCachedSourceDestinationBlend(5, 6);
    switch (layer) {
    case 2:
        if (g_tri2DCount2 > 0u) {
            CGraphics::ApplyTextureStageChange(0, 0);
            CGraphics::m_pTextureManager->pD3D->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, g_tri2DLayer2,
                                                              g_tri2DCount2 * 3, 0);
            CGame::m_unk0x0059ce20 += g_tri2DCount2;
            g_tri2DCount2 = 0;
        }
        break;
    case 3:
        if (g_tri2DCount3 > 0u) {
            CGraphics::ApplyTextureStageChange(0, 0);
            CGraphics::m_pTextureManager->pD3D->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, g_tri2DLayer3,
                                                              g_tri2DCount3 * 3, 0);
            CGame::m_unk0x0059ce20 += g_tri2DCount3;
            g_tri2DCount3 = 0;
        }
        break;
    case 4:
        if (g_tri2DCount4 > 0u) {
            CGraphics::ApplyTextureStageChange(0, 0);
            CGraphics::m_pTextureManager->pD3D->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, g_tri2DLayer4,
                                                              g_tri2DCount4 * 3, 0);
            CGame::m_unk0x0059ce20 += g_tri2DCount4;
            g_tri2DCount4 = 0;
        }
        break;
    default:
        if (g_tri2DCount1 > 0u) {
            CGraphics::ApplyTextureStageChange(0, 0);
            CGraphics::m_pTextureManager->pD3D->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, g_tri2DLayer1,
                                                              g_tri2DCount1 * 3, 0);
            CGame::m_unk0x0059ce20 += g_tri2DCount1;
            g_tri2DCount1 = 0;
        }
        break;
    }
    Graphics_InvalidateTextureStageCache();
    CGraphics::InvalidateBlendStateCache();
}

struct Quad2DRenderVertex {
    float x;
    float y;
    float z;
    BYTE pad0x0c[0xc];
    DWORD colour;
    DWORD specular;
    float u;
    float v;
    BYTE pad0x28[8];
};

// Three 0x30-byte render vertices plus texture and flags.
struct Quad2DVertices {
    Quad2DRenderVertex v[3];
};

struct Quad2D {
    Quad2DVertices verts;   // 0x0
    Texture *pTexture;      // 0x90
    unsigned int flags;     // 0x94
};

// GLOBAL: CMR2 0x007dd168
Quad2D g_quad2DLayerA[0x400];
// GLOBAL: CMR2 0x00803168
Quad2D g_quad2DLayerB[0x200];
// GLOBAL: CMR2 0x00730fd0
Quad2D g_quad2DLayerC[0x800];
// GLOBAL: CMR2 0x007acfd0
Quad2D g_quad2DLayerD[1];
// GLOBAL: CMR2 0x00816174
unsigned int g_quad2DCountC;
// GLOBAL: CMR2 0x00816178
unsigned int g_quad2DCountD;
// GLOBAL: CMR2 0x0081618c
BYTE g_quad2DConvertOverflow;
// GLOBAL: CMR2 0x0081618d
BYTE g_quad2DOverflow;

// Queues a quad into the layer selected by the low bits of pDest (8, 0x10,
// 0x20, 0x40); with none of those bits set pDest is the destination itself.
// match 88%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004bbc60
void Quad2D_Queue(Quad2DVertices *pVerts, Texture *pTexture, Quad2D *pDest)
{
    Quad2D *p;

    if ((unsigned int)pDest & 8) {
        if (g_unk0x0081616c >= 0x400) {
            if (g_quad2DOverflow == 0)
                g_quad2DOverflow = 1;
            return;
        }
        p = &g_quad2DLayerA[g_unk0x0081616c];
        g_unk0x0081616c++;
    } else if ((unsigned int)pDest & 0x10) {
        if (g_unk0x00816170 >= 0x200) {
            if (g_quad2DOverflow == 0)
                g_quad2DOverflow = 1;
            return;
        }
        p = &g_quad2DLayerB[g_unk0x00816170];
        g_unk0x00816170++;
    } else if ((unsigned int)pDest & 0x20) {
        if (g_quad2DCountC >= 0x800) {
            if (g_quad2DOverflow == 0)
                g_quad2DOverflow = 1;
            return;
        }
        p = &g_quad2DLayerC[g_quad2DCountC];
        g_quad2DCountC++;
    } else if ((unsigned int)pDest & 0x40) {
        if (g_quad2DCountD >= 1) {
            if (g_quad2DOverflow == 0)
                g_quad2DOverflow = 1;
            return;
        }
        p = &g_quad2DLayerD[g_quad2DCountD];
        g_quad2DCountD++;
    } else {
        p = (Quad2D *)((unsigned int)pDest);
    }
    p->flags = (unsigned int)pDest;
    p->pTexture = pTexture;
    p->verts = *pVerts;
}

struct Unk0x004a3e20;
void Frontend_SetObjectField118(Unk0x004a3e20 *pObject, int value);
void Graphics_InvalidateTextureStageCache(void);

// Applies the texture (with its blend setup) and the z/cull flags of a queued quad.
#define QUAD2D_SET_STATE(q)                                                          \
    if ((q)->pTexture != NULL) {                                                    \
        Frontend_SetObjectField118((Unk0x004a3e20 *)(q)->pTexture, (q)->pTexture->blendMode);      \
        CGraphics::ApplyTextureStageChange(0, (int)(q)->pTexture);                              \
    } else {                                                                        \
        CGraphics::ApplyTextureStageChange(0, 0);                                               \
    }

#define QUAD2D_SET_FLAGS(q)                                                          \
    if ((q)->flags & 1)                                                              \
        CGraphics::SetZEnable(0);                                                    \
    else                                                                             \
        CGraphics::SetZEnable(1);                                                    \
    if ((q)->flags & 2)                                                              \
        CGraphics::SetZWriteEnable(0);                                               \
    else                                                                             \
        CGraphics::SetZWriteEnable(1);                                               \
    if ((q)->flags & 4)                                                              \
        CGraphics::SetCullMode(1);                                                   \
    else                                                                             \
        CGraphics::SetCullMode(CGame::GetSectorDrawState());

// Copies one queued layer (8, 0x10, 0x20 or other) into the shared vertex
// buffer and draws it, flushing whenever the texture or flags change.
// match 82%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004bbd80
void Quad2D_DrawLayer(unsigned int layer)
{
    IDirect3DVertexBuffer7 *pVB;
    Quad2D *pQuad;
    Quad2D *pLast;
    Texture *pLastTexture;
    unsigned int lastFlags;
    unsigned int count;
    unsigned int i;
    unsigned int n;
    BYTE *pData;

    count = 0;
    n = 0;
    pLastTexture = (Texture *)1;
    lastFlags = 0xffffffff;
    CGraphics::m_pTextureManager->pVertexBuffer1->Lock(DDLOCK_WAIT | DDLOCK_WRITEONLY, (LPVOID *)&pData, NULL);
    if (layer & 8) {
        count = g_unk0x0081616c;
        pQuad = g_quad2DLayerA;
        g_unk0x0081616c = 0;
    } else if (layer & 0x10) {
        count = g_unk0x00816170;
        pQuad = g_quad2DLayerB;
        g_unk0x00816170 = 0;
    } else if (layer & 0x20) {
        count = g_quad2DCountC;
        pQuad = g_quad2DLayerC;
        g_quad2DCountC = 0;
    } else {
        count = g_quad2DCountD;
        pQuad = g_quad2DLayerD;
        g_quad2DCountD = 0;
    }
    for (i = count; i > 0; i--) {
        if (pLastTexture != pQuad->pTexture || lastFlags != pQuad->flags) {
            pVB = CGraphics::m_pTextureManager->pVertexBuffer1;
            pVB->Unlock();
            if (n > 0) {
                CGraphics::m_pTextureManager->pD3D->DrawPrimitiveVB(D3DPT_TRIANGLELIST,
                                                                    CGraphics::m_pTextureManager->pVertexBuffer1, 0, n, 0);
                n = 0;
            }
            CGraphics::m_pTextureManager->pVertexBuffer1->Lock(DDLOCK_WAIT | DDLOCK_WRITEONLY, (LPVOID *)&pData, NULL);
            if (pLastTexture != pQuad->pTexture) {
                QUAD2D_SET_STATE(pQuad);
            }
            if (lastFlags != pQuad->flags) {
                QUAD2D_SET_FLAGS(pQuad);
            }
            pLastTexture = pQuad->pTexture;
            lastFlags = pQuad->flags;
        }
        *(Quad2DVertices *)(pData + n * 0x30) = pQuad->verts;
        n += 3;
        CGame::m_unk0x0059ce20++;
        pQuad++;
    }
    CGraphics::m_pTextureManager->pVertexBuffer1->Unlock();
    if (n > 0) {
        if (layer & 8)
            pLast = &g_quad2DLayerA[count - 1];
        else if (layer & 0x10)
            pLast = &g_quad2DLayerB[count - 1];
        else if (layer & 0x20)
            pLast = &g_quad2DLayerC[count - 1];
        else
            pLast = &g_quad2DLayerD[count - 1];
        QUAD2D_SET_STATE(pLast);
        QUAD2D_SET_FLAGS(pLast);
        CGraphics::m_pTextureManager->pD3D->DrawPrimitiveVB(D3DPT_TRIANGLELIST,
                                                            CGraphics::m_pTextureManager->pVertexBuffer1, 0, n, 0);
    }
    CGraphics::ApplyTextureStageChange(0, 0);
    CGraphics::SetZEnable(1);
    CGraphics::SetZWriteEnable(1);
    CGraphics::SetCullMode(CGame::GetSectorDrawState());
    Graphics_InvalidateTextureStageCache();
    CGraphics::InvalidateBlendStateCache();
}

// Converts three fixed-point vertices and queues them in the selected layer.
// FUNCTION: CMR2 0x004bba00
void Quad2D_QueueFixedTriangle(int, Quad2DInputVertex *pA,
                               Quad2DInputVertex *pB,
                               Quad2DInputVertex *pC, Texture *pTexture,
                               Quad2D *pDest)
{
    Quad2D *p;

    if ((unsigned int)pDest & 8) {
        if (g_unk0x0081616c >= 0x400) {
            if (g_quad2DConvertOverflow == 0)
                g_quad2DConvertOverflow = 1;
            return;
        }
        p = &g_quad2DLayerA[g_unk0x0081616c];
        g_unk0x0081616c++;
    } else if ((unsigned int)pDest & 0x10) {
        if (g_unk0x00816170 >= 0x200) {
            if (g_quad2DConvertOverflow == 0)
                g_quad2DConvertOverflow = 1;
            return;
        }
        p = &g_quad2DLayerB[g_unk0x00816170];
        g_unk0x00816170++;
    } else if ((unsigned int)pDest & 0x20) {
        if (g_quad2DCountC >= 0x800) {
            if (g_quad2DConvertOverflow == 0)
                g_quad2DConvertOverflow = 1;
            return;
        }
        p = &g_quad2DLayerC[g_quad2DCountC];
        g_quad2DCountC++;
    } else if ((unsigned int)pDest & 0x40) {
        if (g_quad2DCountD >= 1) {
            if (g_quad2DConvertOverflow == 0)
                g_quad2DConvertOverflow = 1;
            return;
        }
        p = &g_quad2DLayerD[g_quad2DCountD];
        g_quad2DCountD++;
    } else {
        p = (Quad2D *)((unsigned int)pDest);
    }

    p->pTexture = pTexture;
    p->flags = (unsigned int)pDest;

    p->verts.v[0].x = (float)(pA->x * CGraphics::m_oneOver65536);
    p->verts.v[0].y = (float)(pA->y * CGraphics::m_oneOver65536);
    p->verts.v[0].z = (float)(pA->z * CGraphics::m_oneOver65536);
    p->verts.v[0].u = (float)(pA->u * CGraphics::m_oneOver65536);
    p->verts.v[0].v = (float)(pA->v * CGraphics::m_oneOver65536);
    p->verts.v[0].colour = RGBA_MAKE(pA->colour[0], pA->colour[1],
                                      pA->colour[2], pA->colour[3]);
    p->verts.v[0].specular = 0xff000000;

    p->verts.v[1].x = (float)(pB->x * CGraphics::m_oneOver65536);
    p->verts.v[1].y = (float)(pB->y * CGraphics::m_oneOver65536);
    p->verts.v[1].z = (float)(pB->z * CGraphics::m_oneOver65536);
    p->verts.v[1].u = (float)(pB->u * CGraphics::m_oneOver65536);
    p->verts.v[1].v = (float)(pB->v * CGraphics::m_oneOver65536);
    p->verts.v[1].colour = RGBA_MAKE(pB->colour[0], pB->colour[1],
                                      pB->colour[2], pB->colour[3]);
    p->verts.v[1].specular = 0xff000000;

    p->verts.v[2].x = (float)(pC->x * CGraphics::m_oneOver65536);
    p->verts.v[2].y = (float)(pC->y * CGraphics::m_oneOver65536);
    p->verts.v[2].z = (float)(pC->z * CGraphics::m_oneOver65536);
    p->verts.v[2].u = (float)(pC->u * CGraphics::m_oneOver65536);
    p->verts.v[2].v = (float)(pC->v * CGraphics::m_oneOver65536);
    p->verts.v[2].colour = RGBA_MAKE(pC->colour[0], pC->colour[1],
                                      pC->colour[2], pC->colour[3]);
    p->verts.v[2].specular = 0xff000000;
}

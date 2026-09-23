#ifndef _SPRITE_H
#define _SPRITE_H

#include "../third_party/dx7sdk-7001/include/d3d.h"
#include "Texture.h"

struct SpriteRect {
    short x;
    short y;
    short w;
    short h;
};

// Queued 2D sprite (0x38 bytes); four layers of up to 0x3ff sprites each.
struct Sprite {
    SpriteRect src;         // 0x0  u, v, w, h in the texture
    SpriteRect dst;         // 0x8  x, y, w, h on screen
    SpriteRect uv2;         // 0x10 optional second rect (zeroed when NULL)
    Texture *pTexture;      // 0x18
    BYTE colour[4];         // 0x1c
    int layer;              // 0x20
    short angle;            // 0x24 12-bit angle (from degrees in 16.16)
    BYTE pad[2];
    int centre[3];          // 0x28
    int param;              // 0x34
};

#define SPRITE_LAYER_MAX 0x3ff

// GLOBAL: CMR2 0x005f0858
extern Sprite g_spriteLayer1[SPRITE_LAYER_MAX];
// GLOBAL: CMR2 0x0064c958
extern Sprite g_spriteLayer2[SPRITE_LAYER_MAX];
// GLOBAL: CMR2 0x005e2858
extern Sprite g_spriteLayer3[SPRITE_LAYER_MAX];
// GLOBAL: CMR2 0x0063e958
extern Sprite g_spriteLayer4[SPRITE_LAYER_MAX];
// GLOBAL: CMR2 0x0065a95c
extern unsigned int g_spriteCount1;
// GLOBAL: CMR2 0x0065a960
extern unsigned int g_spriteCount2;
// GLOBAL: CMR2 0x0065a964
extern unsigned int g_spriteCount3;
// GLOBAL: CMR2 0x0065a968
extern unsigned int g_spriteCount4;

// Fixed-point vertex handed to Quad2D_QueueFixedTriangle.
struct Quad2DInputVertex {
    int x;
    int y;
    int z;
    BYTE colour[4];
    int u;
    int v;
};

struct Quad2D;
void Quad2D_QueueFixedTriangle(int, Quad2DInputVertex *pA, Quad2DInputVertex *pB, Quad2DInputVertex *pC,
                               Texture *pTexture, Quad2D *pDest);

// Queued 2D triangle (three transformed vertices); four layers of 0x400.
struct Tri2D {
    D3DTLVERTEX v[3];
};

#define TRI2D_LAYER_MAX 0x400

// GLOBAL: CMR2 0x007ad068
extern Tri2D g_tri2DLayer1[TRI2D_LAYER_MAX];
// GLOBAL: CMR2 0x0077cfd0
extern Tri2D g_tri2DLayer2[TRI2D_LAYER_MAX];
// GLOBAL: CMR2 0x007c5068
extern Tri2D g_tri2DLayer3[TRI2D_LAYER_MAX];
// GLOBAL: CMR2 0x00794fd0
extern Tri2D g_tri2DLayer4[TRI2D_LAYER_MAX];
// GLOBAL: CMR2 0x00816168
extern int g_tri2DEnabled;
// GLOBAL: CMR2 0x0081617c
extern unsigned int g_tri2DCount1;
// GLOBAL: CMR2 0x00816180
extern unsigned int g_tri2DCount2;
// GLOBAL: CMR2 0x00816184
extern unsigned int g_tri2DCount3;
// GLOBAL: CMR2 0x00816188
extern unsigned int g_tri2DCount4;

void Tri2D_SetVertex(D3DTLVERTEX *pVertex, int *pPos, BYTE *pColour);
void Tri2D_Queue(int *pA, int *pB, int *pC, BYTE *pColour, int layer);
void Line2D_Init(void);
int Line2D_Shutdown(void);
void Line2D_Queue(int *pA, int *pB, BYTE *pColourA, BYTE *pColourB);
void Sprite_Init(void);
int Sprite_Shutdown(void);
void Tri2D_Init(void);
int Tri2D_Shutdown(void);
int Sprite_FillRect(int unused, short *pRect, BYTE *pColour, int layer);
void Sprite_Queue(SpriteRect *pSrc, SpriteRect *pDst, Texture *pTexture, int layer, short angleDeg, int *pCentre, SpriteRect *pUv2, BYTE *pColour, int param);

#endif

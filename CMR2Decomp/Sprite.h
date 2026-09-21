#ifndef _SPRITE_H
#define _SPRITE_H

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

void Sprite_Queue(SpriteRect *pSrc, SpriteRect *pDst, Texture *pTexture, int layer, short angleDeg, int *pCentre, SpriteRect *pUv2, BYTE *pColour, int param);

#endif

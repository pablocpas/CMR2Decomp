#include <windows.h>
#include "Sprite.h"

Sprite g_spriteLayer1[SPRITE_LAYER_MAX];
Sprite g_spriteLayer2[SPRITE_LAYER_MAX];
Sprite g_spriteLayer3[SPRITE_LAYER_MAX];
Sprite g_spriteLayer4[SPRITE_LAYER_MAX];
unsigned int g_spriteCount1;
unsigned int g_spriteCount2;
unsigned int g_spriteCount3;
unsigned int g_spriteCount4;

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

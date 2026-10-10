#ifndef _GLOW_H
#define _GLOW_H

#include "FixedPoint.h"

struct Texture;
struct SceneNode;
struct BillboardDef;

// Renderer records: allocation/reset use a 0x5c stride. Unknown bytes retain
// offset names. Texture readers use its first WORD textureId for billboards
// and the whole Texture record for projected triangles.
struct GlowLight {
    int type;                   // 0x0  0 free, 2 seen from behind, 3 seen from both sides
    FixVector pos;              // 0x4  local to pNode when set
    FixVector dir;              // 0x10
    FixVector planePoint;       // 0x1c
    FixVector planeNormal;      // 0x28
    int sizeX;                  // 0x34
    int sizeY;                  // 0x38
    int intensity;              // 0x3c
    int projectedSizeScale;      // 0x40 multiplies sizeX for both projected axes
    int layerIntensity;         // 0x44 draw the ground layer quad when non-zero
    Texture *pTexture;            // 0x48
    Texture *pLayerTexture;     // 0x4c
    unsigned char enabled;               // 0x50
    unsigned char projectedBrightness;               // 0x51 zero disables; otherwise greyscale brightness
    unsigned char field_0x52[2];
    SceneNode *pNode;           // 0x54
    int field_0x58;
};


extern GlowLight *g_glowEntries;
extern int g_glowCapacity;
extern int g_glowAllocatedCount;

void Glow_AllocateEntryTable(int capacity);
int Glow_FreeTable(void);
void Glow_ResetEntries(void);
GlowLight *Glow_Add(int type, FixVector *pos, FixVector *dir, int unused1,
                   int sizeX, int sizeY, Texture *billboardTexture, Texture *layerTexture,
                   int intensity, SceneNode *node, unsigned char projectedBrightness, int unused2,
                   int projectedSizeScale);
void Glow_SetEnabled(GlowLight *pLight, unsigned char value);
void Glow_SetIntensity(GlowLight *pLight, int value);
void Glow_SetPosition(GlowLight *pLight, FixVector *pPos, FixVector *pDir);
void Glow_SetLayerPlane(GlowLight *pLight, FixVector *pPoint, FixVector *pNormal, int layerIntensity);
void Glow_SetSymmetricBounds(BillboardDef *p, int x, int y);
void Glow_Draw(SceneNode *pCamera, unsigned char view);

#endif

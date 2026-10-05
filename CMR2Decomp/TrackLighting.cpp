#include <windows.h>
#include "FixedPoint.h"
#include "Mesh.h"

// Mesh lighting reads the shared stage state defined in TrackCollision.cpp.
extern Mesh *g_stageMesh4Copy;
extern Mesh *g_stageMesh6Copy;
extern BYTE *g_unk0x005920f0;
extern short g_stageMesh4Count;
extern short g_stageMesh5Count;
extern short g_stageMesh6Count;
extern BYTE g_stageColourAlpha;
extern int g_stageLightReady;
extern int g_stageColourValue;
extern int g_stageColourState;

// The input is packed bytes; keeping a byte view reproduces the original loads.
#define PACK_STAGE_COLOUR(c) (((((DWORD)(c).bytes[3] << 8 | (c).bytes[0]) << 8 | (c).bytes[1]) << 8) | (c).bytes[2])

// Sets the diffuse colour of every vertex of the stage light mesh.
// FUNCTION: CMR2 0x004923d0
void TrackLighting_SetLightMeshDiffuseColour(DWORD *pColour)
{
    union { DWORD value; BYTE bytes[4]; } colour;
    int i;

    colour.value = *pColour;

    for (i = g_stageMesh4Count - 1; i >= 0; i--) {
        *(DWORD *)((BYTE *)g_stageMesh4Copy->pVertexData + i * 0x30 + 0x18) = PACK_STAGE_COLOUR(colour);
        *(DWORD *)((BYTE *)g_stageMesh4Copy->pVertexData + i * 0x30 + 0x1c) = 0xff000000;
    }
    g_stageLightReady = 1;
}

// Sets the diffuse colour and alpha of every vertex of stage mesh 5.
// FUNCTION: CMR2 0x00492470
void TrackLighting_SetMesh5DiffuseColour(DWORD *pColour)
{
    union { DWORD value; BYTE bytes[4]; } colour;
    int i;

    colour.value = *pColour;

    for (i = g_stageMesh5Count - 1; i >= 0; i--) {
        *(DWORD *)((BYTE *)((Mesh *)g_unk0x005920f0)->pVertexData + i * 0x30 + 0x18) = PACK_STAGE_COLOUR(colour);
        *(DWORD *)((BYTE *)((Mesh *)g_unk0x005920f0)->pVertexData + i * 0x30 + 0x1c) = (DWORD)g_stageColourAlpha << 24;
    }
    g_stageColourValue = 1;
}

// Sets the diffuse colour and alpha of every vertex of the stage sky mesh.
// FUNCTION: CMR2 0x00492520
void TrackLighting_SetSkyDiffuseColour(DWORD *pColour)
{
    union { DWORD value; BYTE bytes[4]; } colour;
    int i;

    if (g_stageMesh6Copy != NULL) {
        colour.value = *pColour;
        for (i = g_stageMesh6Count - 1; i >= 0; i--) {
            *(DWORD *)((BYTE *)g_stageMesh6Copy->pVertexData + i * 0x30 + 0x18) = PACK_STAGE_COLOUR(colour);
            *(DWORD *)((BYTE *)g_stageMesh6Copy->pVertexData + i * 0x30 + 0x1c) = (DWORD)g_stageColourAlpha << 24;
        }
        g_stageColourState = 1;
    }
}

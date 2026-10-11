#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "GenericFileLoader.h"
#include "../third_party/dx7sdk-7001/include/ddraw.h"

struct Texture {
    USHORT                  textureId;
    BYTE                    field_0x2[12];
    char                    name[122];
    BYTE                    field_0x134[140];
    IDirectDrawSurface7*    pSurface;
    int                     blendMode;      // 0x118 texture stage setup, see CGraphics::ConfigureTextureStageBlendMode
    short                   field_0x11c;
    short                   field_0x11e;
    short                   width;
    short                   height;
    short                   bitsPerPixel;
    BYTE                    field_0x126[2];
    unsigned int            flags;
    void*                   pArchive;       // 0x12c GenericFile the texture was found in
};

// An entry of D3DTextureManager::textureBuffer2: a cube map, one texture per
// face (textureBuffer2[i] points at the first) and the faces' z-buffers.
struct RenderTexture {
    Texture faces[6];                   // 0x0
    IDirectDrawSurface7 *pZBuffers[6];  // 0x720
};

struct DDSFile {
    DWORD magic;            // 'DDS '
    DDSURFACEDESC2 desc;
    BYTE data[1];
};

// Parsed TGA header, see CGraphics::ParseTGAHeader
struct TGAImageInfo {
    unsigned short bytesPerPixel;
    unsigned short field_0x2;
    unsigned int width;
    unsigned int height;
    BYTE *pixels;
};

struct LockedTexture {
    Texture *pTexture;
    DDSURFACEDESC2 desc;
    DWORD masks[4];     // R, G, B, A bit masks (16 bpp only)
    WORD shifts[4];     // position of the lowest set bit of each mask
    WORD depths[4];     // bits in each mask minus 8
};

class CTexture {
public:
    static Texture* FindLoadTexture(GenericFile* pFile, char* textureName, bool *didLoadTexture, LPVOID param4, bool param5, unsigned int flag);
};

#endif

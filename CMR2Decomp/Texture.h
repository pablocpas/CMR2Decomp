#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "GenericFileLoader.h"
#include "../third_party/dx7sdk-7001/include/ddraw.h"

struct Texture {
    USHORT                  textureId;
    BYTE                    field_0x2[134];
    BYTE                    field_0x134[140];
    IDirectDrawSurface7*    pSurface;
    BYTE                    field_0x118[8];
    short                   width;
    short                   height;
    short                   bitsPerPixel;
    BYTE                    field_0x126[598];
    void*                   buffer;
};

// Layout used by the entries of D3DTextureManager::textureBuffer2 (cube maps):
// six face surfaces 0x130 apart starting at 0x114, and six z-buffers at 0x720.
struct RenderTextureFace {
    IDirectDrawSurface7 *pSurface;
    BYTE field_0x4[0x12c];
};

struct RenderTexture {
    BYTE field_0x0[0x114];
    RenderTextureFace faces[5];
    IDirectDrawSurface7 *pFace5Surface;
    BYTE field_0x708[0x18];
    IDirectDrawSurface7 *pZBuffers[6];
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
    BYTE field_0x80[0x20];
};

struct Unk0x0065aee8 {
    BYTE field_0x0[0x4c];
    IDirectDrawSurface7 *pSurface;
    BYTE field_0x50[0xe0];
};

class CTexture {
public:
    static Texture* FindLoadTexture(GenericFile* pFile, char* textureName, bool *didLoadTexture, LPVOID param4, bool param5, unsigned int flag);
};

#endif

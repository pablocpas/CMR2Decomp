#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "GenericFileLoader.h"
#include "port/gfx.h"

struct Texture {
    USHORT                  textureId;
    BYTE                    field_0x2[12];
    char                    name[122];
    BYTE                    field_0x134[140];
    GfxTexture*             pSurface;       // PORT: the renderer's texture
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

// Layout used by the entries of D3DTextureManager::textureBuffer2 (cube maps):
// six face surfaces 0x130 apart starting at 0x114, and six z-buffers at 0x720.
struct RenderTextureFace {
    GfxTexture *pSurface;               // PORT: Gfx_GetCubeFace of faces[0]
    BYTE field_0x4[0x12c];
};

struct RenderTexture {
    BYTE field_0x0[0x114];
    RenderTextureFace faces[5];
    GfxTexture *pFace5Surface;
    BYTE field_0x708[0x18];
    void *pZBuffers[6];                 // PORT: unused (render targets have their own depth)
};

// PORT: the header of a DDS file, which is a 32-bit DDSURFACEDESC2.
struct DDSPixelFormat {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    DWORD dwRGBBitCount;
    DWORD dwRBitMask;
    DWORD dwGBitMask;
    DWORD dwBBitMask;
    DWORD dwRGBAlphaBitMask;
};

struct DDSHeader {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwHeight;
    DWORD dwWidth;
    union {
        LONG lPitch;
        DWORD dwLinearSize;
    };
    DWORD dwBackBufferCount;
    DWORD dwMipMapCount;
    DWORD dwAlphaBitDepth;
    DWORD dwReserved;
    DWORD lpSurface;
    DWORD colourKeys[8];
    DDSPixelFormat ddpfPixelFormat;
    DWORD ddsCaps[4];
    DWORD dwTextureStage;
};
typedef char DDSHeaderSize[sizeof(DDSHeader) == 124 ? 1 : -1];

#define DDS_LINEARSIZE 0x00080000

struct DDSFile {
    DWORD magic;            // 'DDS '
    DDSHeader desc;
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
    GfxLockedRect desc;     // PORT: the renderer's lock instead of a DDSURFACEDESC2
    DWORD masks[4];     // R, G, B, A bit masks (16 bpp only)
    WORD shifts[4];     // position of the lowest set bit of each mask
    WORD depths[4];     // bits in each mask minus 8
};

class CTexture {
public:
    static Texture* FindLoadTexture(GenericFile* pFile, char* textureName, bool *didLoadTexture, LPVOID param4, bool param5, unsigned int flag);
};

#endif

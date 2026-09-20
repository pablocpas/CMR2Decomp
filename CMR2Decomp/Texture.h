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
    BYTE                    field_0x124[600];
    void*                   buffer;
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

#include <stdio.h>
#include <string.h>
#include "Texture.h"
#include "Graphics.h"

// GLOBAL: CMR2 0x00520c98
char g_ddsSuffix[] = ".dds";
// GLOBAL: CMR2 0x00519484
char g_tgaSuffix[] = ".tga";
extern char g_emptyString[4];

// Replaces the extension of the file name part of path with the empty string.
// match 86%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a9f50
void FUN_004a9f50(char *pOut, char *path)
{
    ((int (__cdecl *)(char *, const char *))sprintf)(pOut, strrchr(path, '\\') + 1);
    sprintf(strrchr(pOut, '.'), g_emptyString);
}

// Finds a texture in the archive pFile (as .dds, then .tga) and creates it,
// or falls back to loading it from disk by name.
// match 78%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a9e60
Texture* CTexture::FindLoadTexture(GenericFile* pFile, char* textureName, bool *didLoadTexture, LPVOID param4, bool param5, unsigned int flag) {
    char name[64];
    void *pData;
    Texture *pTexture;

    if (didLoadTexture != NULL)
        *didLoadTexture = false;
    if (pFile != NULL && pFile->didFileLoad && !param5) {
        FUN_004a9f50(name, textureName);
        sprintf(name + strlen(name), g_ddsSuffix);
        pData = CGenericFileLoader::FindFileInArchive(pFile, name, (DWORD *)param4);
        if (pData == NULL) {
            sprintf(name + strlen(name) - 4, g_tgaSuffix);
            pData = CGenericFileLoader::FindFileInArchive(pFile, name, (DWORD *)param4);
        }
        if (pData != NULL) {
            if (didLoadTexture != NULL)
                *didLoadTexture = true;
            pTexture = CGraphics::FUN_004a48c0(textureName, pData, flag);
            pTexture->pArchive = pFile;
            return pTexture;
        }
    }
    return CGraphics::FUN_004a49c0(textureName, flag);
}

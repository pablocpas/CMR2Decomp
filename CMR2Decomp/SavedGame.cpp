#include "SavedGame.h"
#include <string.h>

bool SavedGames_IsValidSessionImage(const void *buffer, size_t size)
{
    if (!buffer || size != sizeof(SavedGameImage))
        return false;
    const SavedGameImage &image = *(const SavedGameImage *)buffer;
    if (image.options.playerCount < 1 || image.options.playerCount > 4 ||
        image.options.mode > 10 || image.options.difficulty > 2 ||
        image.stage.country >= 8 || image.stage.stage >= 11 ||
        !memchr(image.name, 0, sizeof(image.name)))
        return false;
    for (unsigned i = 0; i < image.options.playerCount; i++)
        if (!memchr(image.playerIdentifiers[i].name, 0, sizeof(image.playerIdentifiers[i].name)))
            return false;
    return true;
}

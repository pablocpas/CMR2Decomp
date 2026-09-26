#include <windows.h>
#include "FrontendMenus.h"
#include "GameInfo.h"
#include "Graphics.h"
#include "Frontend.h"
#include "FrontendDraw.h"
#include "Input.h"
#include "Font.h"
#include "Sprite.h"
#include "Texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "Game.h"

// GLOBAL: CMR2 0x00525c30
BYTE g_eventEntries[288] = {
    0x01, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x03, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x03, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x08, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x02, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// GLOBAL: CMR2 0x00525d50
BYTE g_stageEntries[3168] = {
    0x05, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x09, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x4b, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x56, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x5a, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x5a, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x01, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x09, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x03, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x08, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x09, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x07, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x08, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x02, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x09, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x03, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x09, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x08, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x06, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x04, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x01, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x63, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x09, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x03, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x09, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x09, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x5a, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x04, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x5a, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x03, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x09, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x04, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x5a, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x07, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x01, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x08, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x08, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x09, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x2d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x06, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x4b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x02, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x02, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// GLOBAL: CMR2 0x005269b0
const char *g_eventNames[8] = {
    (const char *)0x00526de8, (const char *)0x00526de0, (const char *)0x00526dd8, (const char *)0x00526dd0,
    (const char *)0x00526dc8, (const char *)0x00526dc0, (const char *)0x00526db4, (const char *)0x00526dac,
};

// GLOBAL: CMR2 0x005269d0
const char *g_stageNames[88] = {
    (const char *)0x00526da4, (const char *)0x00526d9c, (const char *)0x00526d94, (const char *)0x00526da4,
    (const char *)0x00526d88, (const char *)0x00526d7c, (const char *)0x00526d74, (const char *)0x00526d88,
    (const char *)0x00526d6c, (const char *)0x00526d64, (const char *)0x00526d5c, (const char *)0x00526d54,
    (const char *)0x00526d48, (const char *)0x00526d40, (const char *)0x00526d38, (const char *)0x00526d2c,
    (const char *)0x00526d24, (const char *)0x00526d18, (const char *)0x00526d10, (const char *)0x00526d38,
    (const char *)0x00526d10, (const char *)0x00526d08, (const char *)0x00526d00, (const char *)0x00526cf8,
    (const char *)0x00526cf0, (const char *)0x00526ce8, (const char *)0x00526ce8, (const char *)0x00526cf0,
    (const char *)0x00526ce0, (const char *)0x00526cd4, (const char *)0x00526ccc, (const char *)0x00526cc4,
    (const char *)0x00526d5c, (const char *)0x00526cbc, (const char *)0x00526cb4, (const char *)0x00526ca8,
    (const char *)0x00526ca0, (const char *)0x00526c94, (const char *)0x00526c8c, (const char *)0x00526c84,
    (const char *)0x00526cbc, (const char *)0x00526c7c, (const char *)0x00526c74, (const char *)0x00526c6c,
    (const char *)0x00526c60, (const char *)0x00526c54, (const char *)0x00526c4c, (const char *)0x00526c40,
    (const char *)0x00526c34, (const char *)0x00526c28, (const char *)0x00526c40, (const char *)0x00526c34,
    (const char *)0x00526c20, (const char *)0x00526c18, (const char *)0x00526d5c, (const char *)0x00526c0c,
    (const char *)0x00526c04, (const char *)0x00526bf8, (const char *)0x00526c0c, (const char *)0x00526bf0,
    (const char *)0x00526be8, (const char *)0x00526be0, (const char *)0x00526bf0, (const char *)0x00526bd8,
    (const char *)0x00526bcc, (const char *)0x00526bc4, (const char *)0x00526bbc, (const char *)0x00526bb4,
    (const char *)0x00526bac, (const char *)0x00526ba4, (const char *)0x00526b9c, (const char *)0x00526b94,
    (const char *)0x00526ba4, (const char *)0x00526b8c, (const char *)0x00526b84, (const char *)0x00526b7c,
    (const char *)0x00526d5c, (const char *)0x00526b70, (const char *)0x00526b68, (const char *)0x00526b5c,
    (const char *)0x00526b70, (const char *)0x00526b54, (const char *)0x00526b4c, (const char *)0x00526b54,
    (const char *)0x00526b4c, (const char *)0x00526b44, (const char *)0x00526b38, (const char *)0x00526b30,
};

// FUNCTION: CMR2 0x004f89b0
BYTE FUN_004f89b0(BYTE **out, int row, int column)
{
    int offset = (column + row * 11) * 36;
    *out = g_stageEntries + offset + 4;
    return g_stageEntries[offset + 2];
}

// FUNCTION: CMR2 0x004f89e0
const char *FUN_004f89e0(int row, int column)
{
    return g_stageNames[column + row * 11];
}

// FUNCTION: CMR2 0x004f8a00
BYTE *FUN_004f8a00(int row, int column)
{
    return g_stageEntries + (column + row * 11) * 36;
}

// FUNCTION: CMR2 0x004f8a20
BYTE FUN_004f8a20(BYTE **out, int index)
{
    int offset = index * 36;
    *out = g_eventEntries + offset + 4;
    return g_eventEntries[offset + 2];
}

// FUNCTION: CMR2 0x004f8a40
const char *FUN_004f8a40(int index)
{
    return g_eventNames[index];
}

// FUNCTION: CMR2 0x004f8a50
BYTE *FUN_004f8a50(int index)
{
    return g_eventEntries + index * 36;
}

// GLOBAL: CMR2 0x00825398
BYTE g_unk0x00825398[0x4c];
// GLOBAL: CMR2 0x008253e4
BYTE g_unk0x008253e4[0x98];
// GLOBAL: CMR2 0x0082547c
BYTE g_unk0x0082547c[0x898];
// GLOBAL: CMR2 0x00825d14
BYTE g_unk0x00825d14[0x258];
// GLOBAL: CMR2 0x00825f6c
BYTE g_unk0x00825f6c[0x1cc];

// FUNCTION: CMR2 0x004f9240
BYTE *FUN_004f9240(int row, int column)
{
    return g_unk0x0082547c + (column + row * 11) * 25;
}

// FUNCTION: CMR2 0x004f9260
BYTE *FUN_004f9260(int row, int column)
{
    return g_unk0x00825d14 + (column + row * 3) * 25;
}

// FUNCTION: CMR2 0x004f9280
BYTE *FUN_004f9280(int index)
{
    return g_unk0x00825398 + index * 25;
}

// FUNCTION: CMR2 0x004f92a0
BYTE *FUN_004f92a0(int row, int column)
{
    return g_unk0x008253e4 + (column + row * 3) * 25;
}

// FUNCTION: CMR2 0x004f92c0
BYTE *FUN_004f92c0(int index)
{
    return g_unk0x00825f6c + index * 25;
}

// Working copy of the 6 controller configurations edited by the controls
// menus (copied back to CInput::m_controllerInfo when leaving).
// GLOBAL: CMR2 0x00829448
ControllerData g_controlsCopy[6];

// Calibration of one axis as shown by the controls menu.
struct AxisBinding {
    int field_0x0;
    int field_0x4;
    int deadzone;       // 0x8  0..10000
    int saturation;     // 0xc  0..10000
    int position;       // 0x10 16.16, -1..1
};
// GLOBAL: CMR2 0x0082a5e8
AxisBinding g_axisBindings[8];

void FUN_004fcc50(ControllerData *p, int index);
void Input_GetLastKeyName(LPSTR pName, unsigned int size);
unsigned int FUN_0040bc30(unsigned short slot);
unsigned int FUN_0040bc00(unsigned short slot);
DWORD FUN_0040bcd0(unsigned short slot);
void FUN_0040bd00(unsigned short slot, unsigned int value);
void FUN_0040bc60(unsigned short slot, unsigned int value);
void FUN_0040bbe0(unsigned short slot, unsigned short index);
unsigned short FUN_0040bbc0(unsigned short slot);
unsigned short FUN_0040bba0(void);

// Separator line under the rows of the controls pages.
// GLOBAL: CMR2 0x0082af70
short g_controlsLine[4];
// Controller configuration used by each of the 8 player slots.
// GLOBAL: CMR2 0x0082a7c8
unsigned short g_unk0x0082a7c8[8];
// Configuration the device settings page was last showing.
// GLOBAL: CMR2 0x0082a7d8
unsigned short g_unk0x0082a7d8;
// Set when the devices must be re-read (the game lost the focus).
// GLOBAL: CMR2 0x0082a7dc
int g_unk0x0082a7dc;
// GLOBAL: CMR2 0x0082a7e4
int g_unk0x0082a7e4;
// GLOBAL: CMR2 0x0082a7e8
int g_unk0x0082a7e8;
// GLOBAL: CMR2 0x0082a7ec
int g_unk0x0082a7ec;

// Configuration of the device selected in the controls menu.
#define CONTROLS_SEL (g_unk0x0082a7c8[g_unk0x0082a7ec & 0xffff])

// FUNCTION: CMR2 0x004fba80
short FUN_004fba80(void)
{
    return (short)g_unk0x0082a7ec;
}

// Item callback of the controls list: selects the device of this entry.
// FUNCTION: CMR2 0x004fbea0
void FUN_004fbea0(Menu *pMenu, int param)
{
    *(short *)&g_unk0x0082a7ec = pMenu->cursor;
}

// Enables and shows the 8 entries of the controls list.
// FUNCTION: CMR2 0x004fba90
void FUN_004fba90(Menu *pMenu, int param)
{
    MenuItem *pItem;
    int i;

    i = 8;
    pItem = pMenu->items;
    do {
        i--;
        pItem->enabled = 1;
        pItem->visible = 1;
        pItem++;
    } while (i != 0);
}

// GLOBAL: CMR2 0x00829428
int g_unk0x00829428[8];
// GLOBAL: CMR2 0x0082a7e0
int g_unk0x0082a7e0;

void FUN_0040bad0(void);
void FUN_004b7d40(void);

// Starts redefining a control: freezes the menu, waits for the keys to be
// released and snapshots the axes of the selected joystick/mouse.
// TODO: CMR2 0x004fbec0 (implemented, match 81%)
void FUN_004fbec0(Menu *pMenu, int param)
{
    BYTE *pDevice;
    int *pOut;
    int i;

    g_unk0x0082a7e4 = 1;
    Menu_SetFlags(pMenu, 0, 0, 0, 0);
    CInput::FUN_0049eab0();
    FUN_0040bad0();
    g_unk0x0082a7e0 = CInput::GetFirstPressedKey();
    FUN_004b7d40();
    pDevice = (BYTE *)CInput::FUN_0049ead0(g_unk0x0082a7c8[g_unk0x0082a7ec & 0xffff]);
    for (i = 0; i < 8; i++)
        g_unk0x00829428[i] = 0;
    if (*(int *)pDevice == 3 || *(int *)pDevice == 2) {
        pOut = g_unk0x00829428;
        pDevice += 0x47c;
        do {
            if (*(int *)(pDevice - 0x10) != 0)
                *pOut = *(int *)pDevice;
            pOut++;
            pDevice += 0x14;
        } while (pOut < &g_unk0x00829428[8]);
    }
}

// Menu callback of the device page: shows the entries the selected device
// supports (no calibration entry for keyboard/mouse or fewer than 4 axes, no
// axis entries for the mouse).
// TODO: CMR2 0x004fbf60 (implemented, match 82%)
void FUN_004fbf60(Menu *pMenu, char param)
{
    DeviceInfo *pDevice;

    if (param == 0) {
        pDevice = CInput::FUN_0049ead0(g_unk0x0082a7c8[g_unk0x0082a7ec & 0xffff]);
        pMenu->items[1].enabled = 1;
        pMenu->items[1].visible = 1;
        pMenu->items[0].enabled = 1;
        pMenu->items[0].visible = 1;
        pMenu->items[9].enabled = 1;
        pMenu->items[9].visible = 1;
        if (pDevice->field_0x0 == 1 || pDevice->field_0x0 == 2 || pDevice->field_0x14 < 4) {
            pMenu->items[9].enabled = 0;
            pMenu->items[9].visible = 0;
        }
        if (pDevice->field_0x0 == 2) {
            pMenu->items[0].enabled = 0;
            pMenu->items[0].visible = 0;
            pMenu->items[1].enabled = 0;
            pMenu->items[1].visible = 0;
        }
    }
}

ControllerData *FUN_0040bbb0(void);

// Leaving the device page: "back" goes to the controls menu; otherwise the
// edited configuration is stored and a joystick goes on to its calibration.
// TODO: CMR2 0x004fbff0 (implemented, match 89%)
void FUN_004fbff0(Menu *pMenu, char back)
{
    if (back != 0) {
        Menu_SetNextAction((int)FUN_004fa530());
        return;
    }
    memcpy(FUN_0040bbb0(), g_controlsCopy, sizeof(g_controlsCopy));
    CInput::FUN_0040be90(g_unk0x0082a7ec & 0xffff);
    if (CInput::FUN_0049ead0(g_unk0x0082a7c8[g_unk0x0082a7ec & 0xffff])->field_0x0 == 3)
        Menu_SetNextAction((int)FUN_004fa500());
}

// Whether the entry under the cursor is bound: entries 0/1 by field_0x110,
// 2/3 by field_0x114 of the selected configuration.
// FUNCTION: CMR2 0x004fc490
int FUN_004fc490(Menu *pMenu)
{
    if (g_controlsCopy[g_unk0x0082a7c8[g_unk0x0082a7ec & 0xffff]].field_0x110 != 0 && (pMenu->cursor == 0 || pMenu->cursor == 1))
        return 1;
    if (g_controlsCopy[g_unk0x0082a7c8[g_unk0x0082a7ec & 0xffff]].field_0x114 != 0 && (pMenu->cursor == 2 || pMenu->cursor == 3))
        return 1;
    return 0;
}

// Menu callback while a control is being redefined: waits for a new key,
// button or axis movement (more than 0.3 of the range), stores it for the
// entry under the cursor and unfreezes the menu.
// TODO: CMR2 0x004fc070 (implemented, match 77%)
void FUN_004fc070(Menu *pMenu)
{
    DeviceInfo *pDevice;
    int *pPosition;
    int *pReference;
    unsigned int buttons;
    BOOL done;
    BOOL axisPair;
    short axis;
    char count;
    int key;
    int i;

    done = FALSE;
    if (g_unk0x0082a7e4 == 0)
        return;
    Menu_SetFlags(pMenu, 0, 0, 0, 0);
    pDevice = CInput::FUN_0049ead0(CONTROLS_SEL);
    key = CInput::GetFirstPressedKey();
    if (g_unk0x0082a7e0 != -1) {
        g_unk0x0082a7e0 = key;
    } else if (key != -1 && FUN_004fc490(pMenu) == 0) {
        done = TRUE;
        Input_GetLastKeyName(g_controlsCopy[CONTROLS_SEL].keyNames[pMenu->cursor], 20);
        CInput::FUN_0040c550(g_controlsCopy[CONTROLS_SEL].field_0x13e, pMenu->cursor, key);
        if (pDevice->field_0x0 != 1)
            CInput::FUN_0040c130(&g_controlsCopy[CONTROLS_SEL].field_0x128, pMenu->cursor, 0);
    }
    axisPair = g_controlsCopy[CONTROLS_SEL].field_0x110 != 0 && (pMenu->cursor == 0 || pMenu->cursor == 1);
    if (((g_controlsCopy[CONTROLS_SEL].field_0x114 != 0 && (pMenu->cursor == 2 || pMenu->cursor == 3)) || axisPair)
        && (pDevice->field_0x0 == 3 || pDevice->field_0x0 == 2)) {
        axis = 0;
        pPosition = (int *)((BYTE *)pDevice + 0x47c);
        pReference = g_unk0x00829428;
        do {
            if (pPosition[-4] != 0 && abs(*pReference - *pPosition) > 0x4ccc) {
                done = TRUE;
                g_controlsCopy[CONTROLS_SEL].field_0x210[pMenu->cursor] = *(ControllerDataUnk0x210 *)(pPosition - 4);
                g_controlsCopy[CONTROLS_SEL].field_0x2d8[pMenu->cursor] = axis;
                CInput::FUN_0040c130(&g_controlsCopy[CONTROLS_SEL].field_0x128, pMenu->cursor, 0);
                CInput::FUN_0040c550(g_controlsCopy[CONTROLS_SEL].field_0x13e, pMenu->cursor, 0);
                *pReference = *pPosition;
            }
            pReference++;
            axis++;
            pPosition += 5;
        } while (pReference < &g_unk0x00829428[8]);
    }
    if (pDevice->field_0x0 == 3 || pDevice->field_0x0 == 2) {
        if (g_controlsCopy[CONTROLS_SEL].field_0x110 != 0)
            pDevice->field_0x8 &= ~3;
        if (g_controlsCopy[CONTROLS_SEL].field_0x114 != 0)
            pDevice->field_0x8 &= ~0xc;
    }
    buttons = pDevice->field_0x8;
    count = 0;
    for (i = 32; i != 0; i--) {
        if (buttons & 1)
            count++;
        buttons >>= 1;
    }
    if (count == 1 && pDevice->field_0x0 != 1 && FUN_004fc490(pMenu) == 0) {
        CInput::FUN_0040c130(&g_controlsCopy[CONTROLS_SEL].field_0x128, pMenu->cursor, (unsigned short)pDevice->field_0x8);
        CInput::FUN_0040c550(g_controlsCopy[CONTROLS_SEL].field_0x13e, pMenu->cursor, 0);
    } else if (!done) {
        return;
    }
    CInput::FUN_0049eab0();
    FUN_0040bad0();
    g_unk0x0082a7e4 = 0;
    Menu_SetFlags(pMenu, 1, 1, 1, 1);
    FUN_004fcc50(&g_controlsCopy[CONTROLS_SEL], pMenu->cursor);
}

// Menu callback of the calibration page: one entry per axis the device has,
// with the stored calibration of the axis; the last entry leads to the
// pad or the joystick page.
// TODO: CMR2 0x004fc500 (implemented, match 63%)
void FUN_004fc500(Menu *pMenu, int param)
{
    ControllerData *pData;
    unsigned int dev;
    unsigned int axis;
    AxisBinding *pBinding;
    MenuItem *pItem;
    int *pFlag;
    int j;

    dev = CONTROLS_SEL;
    pFlag = (int *)((BYTE *)CInput::FUN_0049ead0(dev) + 0x46c);
    axis = 0;
    pBinding = g_axisBindings;
    pItem = pMenu->items;
    do {
        if (*pFlag == 0) {
            pItem->enabled = 0;
            pItem->visible = 0;
        } else {
            pData = &FUN_0040bbb0()[dev];
            pItem->enabled = 1;
            pItem->visible = 1;
            for (j = 0; j < 10; j++) {
                if (pData->field_0x2d8[j] == axis && pData->field_0x210[j].field_0x0 != 0)
                    *(ControllerDataUnk0x210 *)pBinding = pData->field_0x210[j];
            }
        }
        pBinding++;
        axis++;
        pFlag += 5;
        pItem++;
    } while (pBinding < &g_axisBindings[8]);
    if (g_controlsCopy[dev].field_0x118 == 0)
        pMenu->items[axis].pSubMenu = FUN_004fa4f0();
    else
        pMenu->items[axis].pSubMenu = FUN_004fa520();
}

// Menu callback while calibrating an axis: left/right (shift: saturation)
// move the deadzone in steps of 50; the confirm button applies every axis to
// the device and the reset button takes the device's current values.
// TODO: CMR2 0x004fc620 (implemented, match 68%)
void FUN_004fc620(Menu *pMenu)
{
    DeviceInfo *pKeys;
    ControllerData *pData;
    AxisBinding *pBinding;
    BYTE *pDevice;
    unsigned int held;
    unsigned int axis;
    int j;

    if (g_unk0x0082a7e8 != 0) {
        pKeys = CInput::FUN_0049ead0(0);
        if (pKeys->field_0x8 & 0x10) {
            pData = &FUN_0040bbb0()[CONTROLS_SEL];
            pBinding = g_axisBindings;
            axis = 0;
            do {
                CInput::SetJoystickAxisSaturation(CONTROLS_SEL, axis, pBinding->saturation);
                CInput::SetJoystickAxisDeadzone(CONTROLS_SEL, axis, pBinding->deadzone);
                for (j = 0; j < 10; j++) {
                    if (pData->field_0x2d8[j] == axis && pData->field_0x210[j].field_0x0 != 0)
                        pData->field_0x210[j] = *(ControllerDataUnk0x210 *)pBinding;
                }
                pBinding++;
                axis++;
            } while (pBinding < &g_axisBindings[8]);
            g_unk0x0082a7e8 = 0;
        }
        if (pKeys->field_0x8 & 0x20) {
            pDevice = (BYTE *)CInput::FUN_0049ead0(CONTROLS_SEL);
            g_axisBindings[pMenu->cursor].deadzone = *(int *)(pDevice + pMenu->cursor * 0x14 + 0x474);
            g_unk0x0082a7e8 = 0;
            g_axisBindings[pMenu->cursor].saturation = *(int *)(pDevice + pMenu->cursor * 0x14 + 0x478);
        }
        held = pKeys->field_0x4;
        if (CInput::IsShiftPressed() == 0) {
            if (held & 1) {
                if (g_axisBindings[pMenu->cursor].saturation < 0x26de)
                    g_axisBindings[pMenu->cursor].saturation += 0x32;
            } else if (held & 2) {
                if (g_axisBindings[pMenu->cursor].saturation > 0x32)
                    g_axisBindings[pMenu->cursor].saturation -= 0x32;
            }
        } else if (held & 1) {
            if (g_axisBindings[pMenu->cursor].deadzone > 0x32)
                g_axisBindings[pMenu->cursor].deadzone -= 0x32;
        } else if (held & 2) {
            if (g_axisBindings[pMenu->cursor].deadzone < 0x26de)
                g_axisBindings[pMenu->cursor].deadzone += 0x32;
        }
        if (g_unk0x0082a7e8 != 0)
            return;
    }
    Menu_SetFlags(pMenu, 1, 1, 1, 1);
}

// Loads the pad page from the slot's settings: two sensitivities (0..10)
// and whether the pad vibrates.
// FUNCTION: CMR2 0x004fc880
void FUN_004fc880(Menu *pMenu, int param)
{
    pMenu->items[0].max = (char)((int)(FUN_0040bc30((unsigned short)g_unk0x0082a7ec) * 10) / 0x10000);
    pMenu->items[1].max = (char)((int)(FUN_0040bc00((unsigned short)g_unk0x0082a7ec) * 10) / 0x10000);
    if (FUN_0040bcd0((unsigned short)g_unk0x0082a7ec) != 0)
        pMenu->items[2].max = 0;
    else
        pMenu->items[2].max = 1;
}

// Leaving the pad page: stores the settings unless backing out.
// TODO: CMR2 0x004fc8f0 (implemented, match 86%)
void FUN_004fc8f0(Menu *pMenu, char back)
{
    if (back == 0) {
        FUN_0040bd00((unsigned short)g_unk0x0082a7ec, (pMenu->items[0].max << 16) / 10);
        FUN_0040bc60((unsigned short)g_unk0x0082a7ec, (pMenu->items[1].max << 16) / 10);
        CInput::FUN_0040bc90((unsigned short)g_unk0x0082a7ec, 1 - pMenu->items[2].max);
    }
}

// Leaving the device settings page: stores the slot assignments and the
// edited configurations unless backing out.
// FUNCTION: CMR2 0x004fc970
void FUN_004fc970(Menu *pMenu, char back)
{
    unsigned short *pSlot;
    int i;

    if (back == 0) {
        i = 0;
        pSlot = g_unk0x0082a7c8;
        do {
            FUN_0040bbe0(i, *pSlot);
            pSlot++;
            i++;
        } while (pSlot < &g_unk0x0082a7c8[8]);
        memcpy(FUN_0040bbb0(), g_controlsCopy, sizeof(g_controlsCopy));
    }
}

// Fills the device settings page from the configuration of the slot.
// TODO: CMR2 0x004fcb30 (implemented, match 43%)
void FUN_004fcb30(void)
{
    Menu *pMenu;
    DeviceInfo *pDevice;
    unsigned int dev;
    unsigned short slotDev;

    pMenu = FUN_004fa530();
    slotDev = CONTROLS_SEL;
    pMenu->items[0].max = (BYTE)slotDev;
    pDevice = CInput::UpdateDevice(slotDev);
    dev = slotDev;
    if (pDevice == NULL) {
        dev = g_unk0x0082a7ec;
        CONTROLS_SEL = (unsigned short)g_unk0x0082a7ec;
        pDevice = CInput::UpdateDevice(g_unk0x0082a7ec & 0xffff);
    }
    pMenu->items[1].enabled = pDevice->field_0x0 == 3;
    pMenu->items[1].max = (BYTE)g_controlsCopy[dev & 0xffff].field_0x110;
    pMenu->items[2].enabled = pDevice->field_0x0 == 3;
    pMenu->items[2].max = (BYTE)g_controlsCopy[dev & 0xffff].field_0x114;
    pMenu->items[4].enabled = pDevice->field_0x0 == 3;
    pMenu->items[5].enabled = pDevice->field_0x0 == 3;
    pMenu->items[3].enabled = *(int *)((BYTE *)pDevice + 0x464) != 0;
    pMenu->items[3].max = g_controlsCopy[dev & 0xffff].field_0x118 == 0;
    pMenu->items[4].max = g_controlsCopy[dev & 0xffff].field_0x120 == 0;
    pMenu->items[5].max = g_controlsCopy[dev & 0xffff].field_0x124 == 0;
}

// Entering the device settings page: copies the slot assignments and every
// configuration.
// FUNCTION: CMR2 0x004fc9b0
void FUN_004fc9b0(Menu *pMenu, int param)
{
    unsigned short *pSlot;
    int i;

    CInput::FUN_0040c050();
    i = 0;
    pSlot = g_unk0x0082a7c8;
    do {
        *pSlot = FUN_0040bbc0(i);
        pSlot++;
        i++;
    } while (pSlot < &g_unk0x0082a7c8[8]);
    memcpy(g_controlsCopy, FUN_0040bbb0(), sizeof(g_controlsCopy));
    FUN_004fcb30();
}

// Update callback of the device settings page: leaves when no device is
// left, re-reads the devices after the game was inactive, then either stores
// the options or switches the slot to the configuration chosen in entry 0.
// TODO: CMR2 0x004fc9f0 (implemented, match 59%)
void FUN_004fc9f0(Menu *pMenu)
{
    unsigned int dev;

    if (FUN_0040bba0() == 0) {
        Menu_SetNextAction((int)FUN_004f82c0());
        return;
    }
    if (CGame::IsActive() == 0) {
        if (g_unk0x0082a7dc != 0) {
            CInput::FUN_0049eb50();
            CInput::FUN_0040c050();
            FUN_004fc9b0(pMenu, 0);
            FUN_0040bba0();
            g_unk0x0082a7dc = 0;
        }
    } else if (g_unk0x0082a7dc != 0) {
        return;
    }
    if (pMenu->items[0].max == g_unk0x0082a7d8) {
        dev = CONTROLS_SEL;
        g_controlsCopy[dev].field_0x110 = pMenu->items[1].max;
        g_controlsCopy[dev].field_0x114 = pMenu->items[2].max;
        g_controlsCopy[dev].field_0x118 = 1 - pMenu->items[3].max;
        g_controlsCopy[dev].field_0x120 = 1 - pMenu->items[4].max;
        g_controlsCopy[dev].field_0x124 = 1 - pMenu->items[5].max;
        return;
    }
    if (pMenu->items[0].max == 0 && (short)g_unk0x0082a7ec == 1)
        pMenu->items[0].max = (char)g_unk0x0082a7ec;
    if (pMenu->items[0].max == 1 && (short)g_unk0x0082a7ec == 0)
        pMenu->items[0].max = g_unk0x0082a7d8 != 0 ? 0 : 2;
    CONTROLS_SEL = pMenu->items[0].max;
    FUN_004fcb30();
    g_unk0x0082a7d8 = pMenu->items[0].max;
}

// Starts calibrating: freezes the menu and waits for the keys to be released.
// FUNCTION: CMR2 0x004fc850
void FUN_004fc850(Menu *pMenu, int param)
{
    g_unk0x0082a7e8 = 1;
    Menu_SetFlags(pMenu, 0, 0, 0, 0);
    CInput::FUN_0049eab0();
    FUN_0040bad0();
}

// Clears every other binding of the configuration that uses the same button
// or key as binding `index`.
// FUNCTION: CMR2 0x004fcc50
void FUN_004fcc50(ControllerData *p, int index)
{
    unsigned short *pButton;
    char *pName;
    int i;

    i = 0;
    pName = p->keyNames[0];
    pButton = &p->field_0x128;
    do {
        if (i != index) {
            if (*pButton == (&p->field_0x128)[index])
                *pButton = 0;
            if (p->field_0x13e[i] == p->field_0x13e[index]) {
                p->field_0x13e[i] = 0;
                *pName = 0;
            }
        }
        i++;
        pButton++;
        pName += 20;
    } while (i < 10);
}

// Current position of axis `index` of the selected device.
// FUNCTION: CMR2 0x004fbe60
AxisBinding *FUN_004fbe60(int index)
{
    BYTE *pDevice = (BYTE *)CInput::FUN_0049ead0(g_unk0x0082a7c8[g_unk0x0082a7ec & 0xffff]);

    g_axisBindings[index].position = *(int *)(pDevice + index * 0x14 + 0x47c);
    return &g_axisBindings[index];
}


// GLOBAL: CMR2 0x0082a788
char g_bindingText[64];

// Text shown for binding `index` of the selected device: the key name, the
// button name, "axis N", or the direction names of a pad/mouse.
// TODO: CMR2 0x004fbae0 (implemented, match 71%)
char *FUN_004fbae0(int index)
{
    DeviceInfo *pDevice;
    unsigned short button;
    unsigned int dev;
    int i;

    pDevice = CInput::FUN_0049ead0(CONTROLS_SEL);
    if (pDevice->field_0x0 == 1)
        return g_controlsCopy[CONTROLS_SEL].keyNames[index];
    if (CInput::FUN_0040c270(index, &g_controlsCopy[CONTROLS_SEL]) != 0)
        return g_controlsCopy[CONTROLS_SEL].keyNames[index];
    dev = CONTROLS_SEL;
    if (g_controlsCopy[dev].field_0x210[index].field_0x0 == 0 || pDevice->field_0x0 != 3) {
        button = (&g_controlsCopy[dev].field_0x128)[index];
        if (pDevice->field_0x0 == 0) {
            if (button == 1)
                return CFrontend::GetTextString(0x1f9);
            if (button == 2)
                return CFrontend::GetTextString(0x1fa);
            if (button == 4)
                return CFrontend::GetTextString(0x1fb);
            if (button == 8)
                return CFrontend::GetTextString(0x1fc);
        }
        if (pDevice->field_0x0 == 2) {
            if (index == 0)
                return CFrontend::GetTextString(0x1f9);
            if (index == 1)
                return CFrontend::GetTextString(0x1fa);
        }
        i = CInput::GetButtonIndexFromMask(button);
        if (i != -1)
            return pDevice->field_0x284[i];
    } else if (g_controlsCopy[dev].field_0x110 == 0 && (index == 0 || index == 1)) {
        button = (&g_controlsCopy[dev].field_0x128)[index];
        i = CInput::GetButtonIndexFromMask(button);
        if (i != -1) {
            if (strcmp(pDevice->field_0x284[i], CMain::m_logFileBlankLine) != 0)
                return pDevice->field_0x284[i];
            if (button == 1)
                return CFrontend::GetTextString(0x1f9);
            return CFrontend::GetTextString(0x1fa);
        }
    } else if (g_controlsCopy[dev].field_0x114 != 0 || (index != 2 && index != 3)) {
        sprintf(g_bindingText, CFrontend::GetTextString(0x77), g_controlsCopy[dev].field_0x2d8[index]);
        return g_bindingText;
    } else {
        button = (&g_controlsCopy[dev].field_0x128)[index];
        i = CInput::GetButtonIndexFromMask(button);
        if (i != -1) {
            if (strcmp(pDevice->field_0x284[i], CMain::m_logFileBlankLine) != 0)
                return pDevice->field_0x284[i];
            if (button == 4)
                return CFrontend::GetTextString(0x1fb);
            return CFrontend::GetTextString(0x1fc);
        }
    }
    return g_controlsCopy[CONTROLS_SEL].keyNames[index];
}

// FUNCTION: CMR2 0x004fbab0
BYTE *FUN_004fbab0(void)
{
    return (BYTE *)g_controlsCopy[g_unk0x0082a7c8[g_unk0x0082a7ec & 0xffff]].name;
}

// FUNCTION: CMR2 0x004fc060
int FUN_004fc060(void)
{
    return g_unk0x0082a7e4;
}

// FUNCTION: CMR2 0x004fc610
int FUN_004fc610(void)
{
    return g_unk0x0082a7e8;
}

// Draw callback of the controls menu: title, one row (icon + name) per
// visible entry, separators around the selected row, and the carousel.
// TODO: CMR2 0x004fccb0 (implemented, match 57%)
void FUN_004fccb0(Menu *pMenu)
{
    short icon[4];
    short line[4];
    BYTE *pLineShadow;
    BYTE *pLineColour;
    BYTE *pShadow;
    BYTE *pColour;
    MenuItem *pItem;
    Texture *pTexture;
    int resX;
    short y0;
    short row;
    int i;

    icon[1] = 0;
    row = 0;
    icon[0] = (int)(g_pGraphics->resX * 100) / 640;
    icon[2] = CFrontend::m_pAr640ATexture->width;
    icon[3] = CFrontend::m_pAr640ATexture->height;
    FrontendDraw_PlayTime();
    FrontendDraw_BreadcrumbItem((int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480,
                                g_colourWhite0x00524968, 1, CFrontend::GetTextString(pMenu->field_0x4));
    resX = g_pGraphics->resX;
    y0 = (int)(g_pGraphics->resY * 170) / 480;
    line[0] = resX * 99 / 640;
    line[3] = 1;
    line[2] = resX * 282 / 640;
    if (pMenu->cursor == 0) {
        pColour = g_colourWhite0x00524968;
        pShadow = g_colourShadowWhite0x00524974;
    } else {
        pColour = g_colourText0x0052496c;
        pShadow = g_colourShadowText0x00524978;
    }
    line[1] = y0;
    Sprite_FillRect((int)g_pGraphics + 0x150, line, pShadow, 1);
    line[1]++;
    Sprite_FillRect((int)g_pGraphics + 0x150, line, pColour, 1);
    for (i = 0; i < pMenu->itemCount; i++) {
        pItem = &pMenu->items[i];
        if (pItem->visible) {
            icon[1] = (int)(g_pGraphics->resY * 20) / 480 + y0 + (int)(g_pGraphics->resY * 36) / 480 * row
                      - CFrontend::m_pAr640ATexture->height / 2;
            if (pMenu->cursor == i) {
                pShadow = g_colourWhite0x00524968;
                pColour = g_colourWhite0x00524968;
                pTexture = CFrontend::m_pAr640ATexture;
            } else {
                pShadow = g_colourText0x0052496c;
                pColour = g_colourText0x0052496c;
                pTexture = CFrontend::m_pAr640DTexture;
            }
            Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, (SpriteRect *)icon, pTexture, 1, 0, NULL, NULL, pColour, 8);
            if (i < 2)
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(100), i + 1);
            else
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x67));
            Font_DrawText(1, CFrontend::m_stringDest, resX * 0x7a / 640, (int)(g_pGraphics->resY * 24) / 480 + line[1],
                          (int *)pShadow, 0x11);
            if (pMenu->cursor == i || pMenu->cursor == i + 1) {
                pLineColour = g_colourWhite0x00524968;
                pLineShadow = g_colourShadowWhite0x00524974;
            } else {
                pLineColour = g_colourText0x0052496c;
                pLineShadow = g_colourShadowText0x00524978;
            }
            line[1] = (int)(g_pGraphics->resY * 36) / 480 * (row + 1) + y0;
            Sprite_FillRect((int)g_pGraphics + 0x150, line, pLineShadow, 1);
            line[1]++;
            Sprite_FillRect((int)g_pGraphics + 0x150, line, pLineColour, 1);
            row++;
        }
    }
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

extern char g_standingsRowFormat[];
extern BYTE g_colourShadowDim0x0052497c[4];

// Draw callback of the device page: "Controller N | <device>" title and one
// row per visible binding with its current assignment.
// TODO: CMR2 0x004fd080 (implemented, match 44%)
void FUN_004fd080(Menu *pMenu)
{
    short icon[4];
    short line[4];
    char *text[2];
    BYTE *pLineShadow;
    BYTE *pLineColour;
    BYTE *pShadow;
    BYTE *pColour;
    Texture *pTexture;
    MenuItem *pItem;
    int resX;
    short y0;
    short row;
    int i;

    icon[1] = 0;
    icon[0] = (int)(g_pGraphics->resX * 100) / 640;
    icon[2] = CFrontend::m_pAr640ATexture->width;
    icon[3] = CFrontend::m_pAr640ATexture->height;
    FrontendDraw_PlayTime();
    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(100), FUN_004fba80() + 1);
    text[0] = CFrontend::m_stringDest;
    text[1] = CFrontend::GetTextString(pMenu->field_0x4);
    FrontendDraw_Breadcrumb((int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, text, 2);
    resX = g_pGraphics->resX;
    y0 = (int)(g_pGraphics->resY * 65) / 480;
    line[0] = resX * 99 / 640;
    line[3] = 1;
    line[2] = resX * 282 / 640;
    if (pMenu->cursor == 0) {
        pColour = g_colourWhite0x00524968;
        pShadow = g_colourShadowWhite0x00524974;
    } else if (!pMenu->items[0].enabled) {
        pColour = g_colourDim0x00524970;
        pShadow = g_colourShadowDim0x0052497c;
    } else {
        pColour = g_colourText0x0052496c;
        pShadow = g_colourShadowText0x00524978;
    }
    line[1] = y0;
    Sprite_FillRect((int)g_pGraphics + 0x150, line, pShadow, 1);
    line[1]++;
    Sprite_FillRect((int)g_pGraphics + 0x150, line, pColour, 1);
    row = 0;
    for (i = 0; i < pMenu->itemCount; i++) {
        pItem = &pMenu->items[i];
        if (pItem->enabled) {
            icon[1] = (int)(g_pGraphics->resY * 20) / 480 + y0 + (int)(g_pGraphics->resY * 36) / 480 * row
                      - CFrontend::m_pAr640ATexture->height / 2;
            if (pMenu->cursor == i) {
                pShadow = g_colourWhite0x00524968;
                pColour = g_colourWhite0x00524968;
                pTexture = CFrontend::m_pAr640ATexture;
            } else {
                pShadow = g_colourText0x0052496c;
                pColour = g_colourText0x0052496c;
                pTexture = CFrontend::m_pAr640DTexture;
            }
            Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, (SpriteRect *)icon, pTexture, 1, 0, NULL, NULL, pColour, 8);
            if (i < 10 && (FUN_004fc060() == 0 || pMenu->cursor != i))
                sprintf(CFrontend::m_stringDest, g_standingsRowFormat, CFrontend::GetTextString(pItem->id), FUN_004fbae0(i));
            else
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(pItem->id));
            Font_DrawText(1, CFrontend::m_stringDest, resX * 0x7a / 640, (int)(g_pGraphics->resY * 24) / 480 + line[1],
                          (int *)pShadow, 0x11);
            if (pMenu->cursor == i || pMenu->cursor == i + 1) {
                pLineColour = g_colourWhite0x00524968;
                pLineShadow = g_colourShadowWhite0x00524974;
            } else {
                pLineColour = g_colourText0x0052496c;
                pLineShadow = g_colourShadowText0x00524978;
            }
            line[1] = (int)(g_pGraphics->resY * 36) / 480 * (row + 1) + y0;
            Sprite_FillRect((int)g_pGraphics + 0x150, line, pLineShadow, 1);
            line[1]++;
            Sprite_FillRect((int)g_pGraphics + 0x150, line, pLineColour, 1);
            row++;
        }
    }
}

void FUN_004ff060(short x, short y, Menu *pMenu, int index);

// GLOBAL: CMR2 0x00526ec8
char g_strPercentFormat[8] = "%s %d%%";
// GLOBAL: CMR2 0x00526ed0
char g_strSlashFormat[8] = "%s / %s";
// GLOBAL: CMR2 0x00526ed8
char g_strAngleFormat[8] = "< %s >";

// Draws the two choices of an entry after its label at x: the active one
// (A when value is 0) in pBright, the other one in pDim.
inline void FrontendMenus_DrawChoice(int x, int y, BYTE value, int idA, int idB, BYTE *pBright, BYTE *pDim)
{
    int xB;

    Font_DrawText(1, CFrontend::GetTextString(idA), x, y, (int *)(value == 0 ? pBright : pDim), 0x11);
    xB = (int)(g_pGraphics->resX * 10) / 640 + x + Font_GetTextWidth(1, (BYTE *)CFrontend::GetTextString(idA));
    Font_DrawText(1, CFrontend::GetTextString(idB), xB, y, (int *)(value == 0 ? pDim : pBright), 0x11);
}

// Draws a label at x and returns where its choices start.
inline int FrontendMenus_DrawLabel(char *text, int x, int y, BYTE *pColour)
{
    Font_DrawText(1, text, x, y, (int *)pColour, 0x11);
    return Font_GetTextWidth(1, (BYTE *)text) + (int)(g_pGraphics->resX * 10) / 640 + x;
}

// Draw callback of the calibration page: one row per axis ("axis N" and its
// calibration bar), "back", and the key help at the bottom.
// TODO: CMR2 0x004fd480 (implemented, match 77%)
void FUN_004fd480(Menu *pMenu)
{
    short icon[4];
    char *text[2];
    BYTE *pShadow;
    BYTE *pColour;
    Texture *pTexture;
    int resX;
    int maxWidth;
    int width;
    short y0;
    int i;

    icon[1] = 0;
    icon[0] = (int)(g_pGraphics->resX * 100) / 640;
    icon[2] = CFrontend::m_pAr640ATexture->width;
    icon[3] = CFrontend::m_pAr640ATexture->height;
    FrontendDraw_PlayTime();
    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(100), FUN_004fba80() + 1);
    text[0] = CFrontend::m_stringDest;
    text[1] = CFrontend::GetTextString(pMenu->field_0x4);
    FrontendDraw_Breadcrumb((int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, text, 2);
    if (pMenu->cursor == 0) {
        pColour = g_colourWhite0x00524968;
        pShadow = g_colourShadowWhite0x00524974;
    } else {
        pColour = g_colourText0x0052496c;
        pShadow = g_colourShadowText0x00524978;
    }
    y0 = (int)(g_pGraphics->resY * 75) / 480;
    g_controlsLine[0] = (int)(g_pGraphics->resX * 99) / 640;
    g_controlsLine[3] = 1;
    g_controlsLine[2] = (int)(g_pGraphics->resX * 282) / 640;
    g_controlsLine[1] = y0;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_controlsLine, pShadow, 1);
    g_controlsLine[1]++;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_controlsLine, pColour, 1);
    maxWidth = 0;
    for (i = 0; i < pMenu->itemCount; i++) {
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x77), i);
        width = Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
        if (maxWidth < width)
            maxWidth = width;
    }
    resX = g_pGraphics->resX;
    for (i = 0; i < pMenu->itemCount; i++) {
        icon[1] = (int)(g_pGraphics->resY * 2) / 480 + (int)(g_pGraphics->resY * 18) / 480 + y0
                  + ((short)((int)(g_pGraphics->resY * 36) / 480) * (short)i - CFrontend::m_pAr640ATexture->height / 2);
        if (pMenu->cursor == i) {
            pColour = g_colourWhite0x00524968;
            pTexture = CFrontend::m_pAr640ATexture;
        } else {
            pColour = g_colourText0x0052496c;
            pTexture = CFrontend::m_pAr640DTexture;
            if (!pMenu->items[i].enabled)
                pColour = g_colourDim0x00524970;
        }
        Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, (SpriteRect *)icon, pTexture, 1, 0, NULL, NULL, pColour, 8);
        if (i < pMenu->itemCount - 1) {
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x77), i);
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x7a) / 640,
                          (short)((int)(g_pGraphics->resY * 24) / 480 + g_controlsLine[1]), (int *)pColour, 0x11);
            FUN_004ff060((int)(g_pGraphics->resX * 0x7a) / 640 + maxWidth + resX * 5 / 640 + (int)(g_pGraphics->resX * 200) / 1280,
                         (int)(g_pGraphics->resY * 10) / 480 + icon[1], pMenu, i);
        } else {
            Font_DrawText(1, CFrontend::GetTextString(pMenu->items[i].id), (int)(g_pGraphics->resX * 0x7a) / 640,
                          (short)((int)(g_pGraphics->resY * 24) / 480 + g_controlsLine[1]), (int *)pColour, 0x11);
        }
        if (pMenu->cursor == i + 1 || pMenu->cursor == i) {
            pColour = g_colourWhite0x00524968;
            pShadow = g_colourShadowWhite0x00524974;
        } else {
            pColour = g_colourText0x0052496c;
            pShadow = g_colourShadowText0x00524978;
        }
        g_controlsLine[1] = (short)((int)(g_pGraphics->resY * 36) / 480) * ((short)i + 1) + y0;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_controlsLine, pShadow, 1);
        g_controlsLine[1]++;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_controlsLine, pColour, 1);
    }
    if (FUN_004fc610() == 0) {
        Font_DrawText(1, CFrontend::GetTextString(0x20a), (int)(g_pGraphics->resX * 24) / 640,
                      (int)(g_pGraphics->resY * 420) / 480, (int *)g_colourText0x0052496c, 9);
    } else {
        Font_DrawText(1, CFrontend::GetTextString(0x207), (int)(g_pGraphics->resX * 24) / 640,
                      (int)(g_pGraphics->resY * 420) / 480, (int *)g_colourText0x0052496c, 9);
        Font_DrawText(1, CFrontend::GetTextString(0x208), (int)(g_pGraphics->resX * 24) / 640,
                      (int)(g_pGraphics->resY * 440) / 480, (int *)g_colourText0x0052496c, 9);
        Font_DrawText(1, CFrontend::GetTextString(0x209), (int)(g_pGraphics->resX * 24) / 640,
                      (int)(g_pGraphics->resY * 460) / 480, (int *)g_colourText0x0052496c, 9);
    }
}

// Picks the colours of row i of a settings page: icon/label, active choice,
// inactive choice; disabled rows are all dim.
#define CONTROLS_ROW_COLOURS(pMenu, i, pLabel, pBright, pDim, pTexture)       \
    if ((pMenu)->cursor == (i)) {                                             \
        pLabel = g_colourWhite0x00524968;                                     \
        pBright = g_colourWhite0x00524968;                                    \
        pDim = g_colourText0x0052496c;                                        \
        pTexture = CFrontend::m_pAr640ATexture;                               \
    } else {                                                                  \
        pTexture = CFrontend::m_pAr640DTexture;                               \
        if (!(pMenu)->items[i].enabled) {                                     \
            pLabel = g_colourDim0x00524970;                                   \
            pBright = g_colourDim0x00524970;                                  \
            pDim = g_colourDim0x00524970;                                     \
        } else {                                                              \
            pLabel = g_colourText0x0052496c;                                  \
            pBright = g_colourWhite0x00524968;                                \
            pDim = g_colourText0x0052496c;                                    \
        }                                                                     \
    }

// Draw callback of the pad page: the two sensitivities (%), the vibration
// on/off choice and "back", vertically centred.
// TODO: CMR2 0x004fdb10 (implemented, match 48%)
void FUN_004fdb10(Menu *pMenu)
{
    short icon[4];
    char *text[2];
    BYTE *pLabel;
    BYTE *pBright;
    BYTE *pDim;
    BYTE *pShadow;
    BYTE *pColour;
    Texture *pTexture;
    MenuItem *pItem;
    short y0;
    int x;
    int y;
    int i;

    icon[1] = 0;
    icon[0] = (int)(g_pGraphics->resX * 100) / 640;
    icon[2] = CFrontend::m_pAr640ATexture->width;
    icon[3] = CFrontend::m_pAr640ATexture->height;
    FrontendDraw_PlayTime();
    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(100), FUN_004fba80() + 1);
    text[0] = CFrontend::m_stringDest;
    text[1] = CFrontend::GetTextString(pMenu->field_0x4);
    FrontendDraw_Breadcrumb((int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, text, 2);
    y0 = (short)(((int)(g_pGraphics->resY * 8) / 480 + (int)(g_pGraphics->resY * 38) / 480 + (int)(g_pGraphics->resY * 384) / 480) / 2)
         - (short)((int)(g_pGraphics->resY * 36) / 480 * pMenu->itemCount / 2);
    if (pMenu->cursor == 0) {
        pColour = g_colourWhite0x00524968;
        pShadow = g_colourShadowWhite0x00524974;
    } else {
        pColour = g_colourText0x0052496c;
        pShadow = g_colourShadowText0x00524978;
    }
    g_controlsLine[0] = (int)(g_pGraphics->resX * 99) / 640;
    g_controlsLine[3] = 1;
    g_controlsLine[2] = (int)(g_pGraphics->resX * 282) / 640;
    g_controlsLine[1] = y0;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_controlsLine, pShadow, 1);
    g_controlsLine[1]++;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_controlsLine, pColour, 1);
    for (i = 0; i < pMenu->itemCount; i++) {
        pItem = &pMenu->items[i];
        icon[1] = (int)(g_pGraphics->resY * 2) / 480 + (int)(g_pGraphics->resY * 18) / 480 + y0
                  + ((short)((int)(g_pGraphics->resY * 36) / 480) * (short)i - CFrontend::m_pAr640ATexture->height / 2);
        CONTROLS_ROW_COLOURS(pMenu, i, pLabel, pBright, pDim, pTexture)
        Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, (SpriteRect *)icon, pTexture, 1, 0, NULL, NULL, pLabel, 8);
        x = (int)(g_pGraphics->resX * 0x7a) / 640;
        y = (short)((int)(g_pGraphics->resY * 24) / 480 + g_controlsLine[1]);
        switch (pItem->value) {
        case 0:
        case 1:
            sprintf(CFrontend::m_stringDest, g_strPercentFormat, CFrontend::GetTextString(pItem->id), pItem->max * 10);
            Font_DrawText(1, CFrontend::m_stringDest, x, y, (int *)pLabel, 0x11);
            break;
        case 2:
            x = FrontendMenus_DrawLabel(CFrontend::GetTextString(pItem->id), x, y, pLabel);
            FrontendMenus_DrawChoice(x, y, pItem->max, 0x134, 0x133, pBright, pDim);
            break;
        case 3:
            Font_DrawText(1, CFrontend::GetTextString(pItem->id), x, y, (int *)pLabel, 0x11);
            break;
        }
        if (pMenu->cursor == i + 1 || pMenu->cursor == i) {
            pColour = g_colourWhite0x00524968;
            pShadow = g_colourShadowWhite0x00524974;
        } else {
            pColour = g_colourText0x0052496c;
            pShadow = g_colourShadowText0x00524978;
        }
        g_controlsLine[1] = (short)((int)(g_pGraphics->resY * 36) / 480) * ((short)i + 1) + y0;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_controlsLine, pShadow, 1);
        g_controlsLine[1]++;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_controlsLine, pColour, 1);
    }
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// Draw callback of the device settings page: the configuration chosen for
// the slot and its on/off options.
// TODO: CMR2 0x004fe240 (implemented, match 33%)
void FUN_004fe240(Menu *pMenu)
{
    short icon[4];
    short line[4];
    char *text[2];
    BYTE *pLabel;
    BYTE *pBright;
    BYTE *pDim;
    BYTE *pShadow;
    BYTE *pColour;
    Texture *pTexture;
    MenuItem *pItem;
    short y0;
    int x;
    int y;
    int i;

    icon[1] = 0;
    icon[0] = (int)(g_pGraphics->resX * 100) / 640;
    icon[2] = CFrontend::m_pAr640ATexture->width;
    icon[3] = CFrontend::m_pAr640ATexture->height;
    FrontendDraw_PlayTime();
    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(100), FUN_004fba80() + 1);
    text[0] = CFrontend::m_stringDest;
    text[1] = CFrontend::GetTextString(pMenu->field_0x4);
    FrontendDraw_Breadcrumb((int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, text, 2);
    y0 = (int)(g_pGraphics->resY * 100) / 480;
    line[0] = (int)(g_pGraphics->resX * 99) / 640;
    line[3] = 1;
    line[2] = (int)(g_pGraphics->resX * 282) / 640;
    if (pMenu->cursor == 0) {
        pColour = g_colourWhite0x00524968;
        pShadow = g_colourShadowWhite0x00524974;
    } else if (!pMenu->items[0].enabled) {
        pColour = g_colourDim0x00524970;
        pShadow = g_colourShadowDim0x0052497c;
    } else {
        pColour = g_colourText0x0052496c;
        pShadow = g_colourShadowText0x00524978;
    }
    line[1] = y0;
    Sprite_FillRect((int)g_pGraphics + 0x150, line, pShadow, 1);
    line[1]++;
    Sprite_FillRect((int)g_pGraphics + 0x150, line, pColour, 1);
    for (i = 0; i < pMenu->itemCount; i++) {
        pItem = &pMenu->items[i];
        icon[1] = (int)(g_pGraphics->resY * 20) / 480 + y0 + (int)(g_pGraphics->resY * 36) / 480 * (short)i
                  - CFrontend::m_pAr640ATexture->height / 2;
        CONTROLS_ROW_COLOURS(pMenu, i, pLabel, pBright, pDim, pTexture)
        Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, (SpriteRect *)icon, pTexture, 1, 0, NULL, NULL, pLabel, 8);
        x = (int)(g_pGraphics->resX * 0x7a) / 640;
        y = (int)(g_pGraphics->resY * 24) / 480 + line[1];
        switch (pItem->value) {
        case 0:
            sprintf(CFrontend::m_stringDest, g_strAngleFormat, FUN_004fbab0());
            Font_DrawText(1, CFrontend::m_stringDest, x, y, (int *)pLabel, 0x11);
            break;
        case 1:
            x = FrontendMenus_DrawLabel(CFrontend::GetTextString(0x197), x, y, pLabel);
            FrontendMenus_DrawChoice(x, y, pItem->max, 0x69, 0x68, pBright, pDim);
            break;
        case 2:
            sprintf(CFrontend::m_stringDest, g_strSlashFormat, CFrontend::GetTextString(0x6d), CFrontend::GetTextString(0x6e));
            x = FrontendMenus_DrawLabel(CFrontend::m_stringDest, x, y, pLabel);
            FrontendMenus_DrawChoice(x, y, pItem->max, 0x69, 0x68, pBright, pDim);
            break;
        case 3:
            x = FrontendMenus_DrawLabel(CFrontend::GetTextString(pItem->id), x, y, pLabel);
            FrontendMenus_DrawChoice(x, y, pItem->max, 0x134, 0x133, pBright, pDim);
            break;
        case 4:
        case 5:
            x = FrontendMenus_DrawLabel(CFrontend::GetTextString(pItem->id), x, y, pLabel);
            FrontendMenus_DrawChoice(x, y, pItem->max, 5, 4, pBright, pDim);
            break;
        case 6:
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(pItem->id));
            Font_DrawText(1, CFrontend::m_stringDest, x, y, (int *)pLabel, 0x11);
            break;
        }
        if (pMenu->cursor == i || pMenu->cursor == i + 1) {
            pColour = g_colourWhite0x00524968;
            pShadow = g_colourShadowWhite0x00524974;
        } else {
            pColour = g_colourText0x0052496c;
            pShadow = g_colourShadowText0x00524978;
        }
        line[1] = (int)(g_pGraphics->resY * 36) / 480 * ((short)i + 1) + y0;
        Sprite_FillRect((int)g_pGraphics + 0x150, line, pShadow, 1);
        line[1]++;
        Sprite_FillRect((int)g_pGraphics + 0x150, line, pColour, 1);
    }
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

int FUN_004ff420(int a, int b);

// Draws the calibration bar of an axis centred on (x, y): the bar, the
// deadzone and saturation marks and the current position.
// TODO: CMR2 0x004ff0f0 (implemented, match 62%)
void FUN_004ff0f0(short x, short y, DWORD colour, AxisBinding *pAxis)
{
    short deadzone[4];
    short saturation[4];
    short bar[4];
    short position[4];
    short half;
    int v;

    bar[0] = x - (short)((int)(g_pGraphics->resX * 200) / 1280);
    bar[1] = y - (short)((int)(g_pGraphics->resY * 10) / 960);
    bar[2] = (int)(g_pGraphics->resX * 200) / 640;
    bar[3] = (int)(g_pGraphics->resY * 10) / 480;
    Sprite_FillRect((int)g_pGraphics + 0x150, bar, (BYTE *)&colour, 1);
    if (pAxis != NULL) {
        half = (short)(FUN_004ff420(pAxis->deadzone, (int)(g_pGraphics->resX * 200) / 640) / 2);
        deadzone[0] = x - half;
        deadzone[2] = 2;
        deadzone[1] = y - (short)((int)(g_pGraphics->resY * 10) / 960) - 2;
        deadzone[3] = (int)(g_pGraphics->resY * 10) / 480 + 4;
        Sprite_FillRect((int)g_pGraphics + 0x150, deadzone, g_colourWhite0x00524968, 1);
        v = FUN_004ff420(pAxis->saturation, (int)(g_pGraphics->resX * 200) / 640);
        deadzone[0] = half + x;
        saturation[2] = deadzone[2];
        half = (short)(v / 2);
        saturation[0] = x - half;
        saturation[1] = deadzone[1];
        saturation[3] = deadzone[3];
        position[3] = deadzone[3];
        v = (int)(g_pGraphics->resX * 200) / 640 * pAxis->position / 2;
        position[0] = (short)(v / 0x10000) + x;
        position[1] = deadzone[1];
        position[2] = deadzone[2];
        Sprite_FillRect((int)g_pGraphics + 0x150, deadzone, g_colourWhite0x00524968, 1);
        Sprite_FillRect((int)g_pGraphics + 0x150, saturation, g_colourWhite0x00524968, 1);
        saturation[0] = half + x;
        Sprite_FillRect((int)g_pGraphics + 0x150, saturation, g_colourWhite0x00524968, 1);
        Sprite_FillRect((int)g_pGraphics + 0x150, position, g_colourWhite0x00524968, 1);
    }
}

// Draws row `index` of the calibration page: the axis bar in white when
// selected (red while calibrating), dim when the entry is hidden.
// TODO: CMR2 0x004ff060 (implemented, match 65%)
void FUN_004ff060(short x, short y, Menu *pMenu, int index)
{
    AxisBinding *pAxis;
    DWORD colour;
    BYTE red[4];

    red[1] = 0x14;
    red[2] = 0x14;
    red[0] = 0xf0;
    red[3] = 0xff;
    if (!pMenu->items[index].visible) {
        pAxis = NULL;
        colour = *(DWORD *)g_colourDim0x00524970;
    } else {
        pAxis = FUN_004fbe60(index);
        if (pMenu->cursor == index) {
            colour = *(DWORD *)g_colourWhite0x00524968;
            if (FUN_004fc610() != 0)
                colour = *(DWORD *)red;
        } else {
            colour = *(DWORD *)g_colourText0x0052496c;
            if (!pMenu->items[index].enabled)
                return;
        }
    }
    FUN_004ff0f0(x, y, colour, pAxis);
}

// FUNCTION: CMR2 0x004ff420
int FUN_004ff420(int a, int b)
{
    return (a * b) / 10000;
}

// GLOBAL: CMR2 0x0081b158
Menu g_menu0x0081b158;
// GLOBAL: CMR2 0x0081b338
Menu g_menu0x0081b338;
// GLOBAL: CMR2 0x0081b518
Menu g_menu0x0081b518;
// Cheats menu.
// GLOBAL: CMR2 0x0081be78
Menu g_menu0x0081be78;
// GLOBAL: CMR2 0x0081bab8
Menu g_menu0x0081bab8;
// GLOBAL: CMR2 0x0081bc98
Menu g_menu0x0081bc98;
// GLOBAL: CMR2 0x0081c058
Menu g_menu0x0081c058;
// GLOBAL: CMR2 0x0081c418
Menu g_menu0x0081c418;
// GLOBAL: CMR2 0x0081c5f8
Menu g_menu0x0081c5f8;
// GLOBAL: CMR2 0x0081c7d8
Menu g_menu0x0081c7d8;
// GLOBAL: CMR2 0x0081cb98
Menu g_menu0x0081cb98;
// GLOBAL: CMR2 0x0081cd78
Menu g_menu0x0081cd78;
// GLOBAL: CMR2 0x0081cf58
Menu g_menu0x0081cf58;
// GLOBAL: CMR2 0x0081d138
Menu g_menu0x0081d138;
// GLOBAL: CMR2 0x0081d318
Menu g_menu0x0081d318;
// GLOBAL: CMR2 0x0081d4f8
Menu g_menu0x0081d4f8;
// GLOBAL: CMR2 0x0081d6d8
Menu g_menu0x0081d6d8;
// GLOBAL: CMR2 0x0081d8b8
Menu g_menu0x0081d8b8;
// GLOBAL: CMR2 0x0081dc78
Menu g_menu0x0081dc78;
// GLOBAL: CMR2 0x0081de58
Menu g_menu0x0081de58;
// GLOBAL: CMR2 0x0081e218
Menu g_menu0x0081e218;
// GLOBAL: CMR2 0x0081e3f8
Menu g_menu0x0081e3f8;
// GLOBAL: CMR2 0x0081e5d8
Menu g_menu0x0081e5d8;
// GLOBAL: CMR2 0x0081e998
Menu g_menu0x0081e998;
// GLOBAL: CMR2 0x0081eb78
Menu g_menu0x0081eb78;
// GLOBAL: CMR2 0x0081ed58
Menu g_menu0x0081ed58;
// GLOBAL: CMR2 0x0081ef38
Menu g_menu0x0081ef38;
// GLOBAL: CMR2 0x0081f118
Menu g_menu0x0081f118;
// GLOBAL: CMR2 0x0081f2f8
Menu g_menu0x0081f2f8;
// GLOBAL: CMR2 0x0081f4d8
Menu g_menu0x0081f4d8;
// GLOBAL: CMR2 0x0081f6b8
Menu g_menu0x0081f6b8;
// GLOBAL: CMR2 0x0081fa78
Menu g_menu0x0081fa78;
// GLOBAL: CMR2 0x0081fc58
Menu g_menu0x0081fc58;
// GLOBAL: CMR2 0x0081fe38
Menu g_menu0x0081fe38;
// GLOBAL: CMR2 0x00820018
Menu g_menu0x00820018;
// GLOBAL: CMR2 0x008201f8
Menu g_menu0x008201f8;
// GLOBAL: CMR2 0x008203d8
Menu g_menu0x008203d8;
// GLOBAL: CMR2 0x008205b8
Menu g_menu0x008205b8;
// GLOBAL: CMR2 0x00820798
Menu g_menu0x00820798;
// GLOBAL: CMR2 0x00820978
Menu g_menu0x00820978;
// GLOBAL: CMR2 0x00820b58
Menu g_menu0x00820b58;
// GLOBAL: CMR2 0x00820d38
Menu g_menu0x00820d38;
// GLOBAL: CMR2 0x008210f8
Menu g_menu0x008210f8;
// GLOBAL: CMR2 0x008212d8
Menu g_menu0x008212d8;
// GLOBAL: CMR2 0x008214b8
Menu g_menu0x008214b8;
// GLOBAL: CMR2 0x00821878
Menu g_menu0x00821878;
// GLOBAL: CMR2 0x00821c38
Menu g_menu0x00821c38;
// GLOBAL: CMR2 0x00821e18
Menu g_menu0x00821e18;
// GLOBAL: CMR2 0x008221d8
Menu g_menu0x008221d8;
// GLOBAL: CMR2 0x008223b8
Menu g_menu0x008223b8;
// GLOBAL: CMR2 0x00822598
Menu g_menu0x00822598;
// GLOBAL: CMR2 0x00822958
Menu g_menu0x00822958;
// GLOBAL: CMR2 0x00822d18
Menu g_menu0x00822d18;
// GLOBAL: CMR2 0x00822ef8
Menu g_menu0x00822ef8;
// GLOBAL: CMR2 0x008230d8
Menu g_menu0x008230d8;
// GLOBAL: CMR2 0x008232b8
Menu g_menu0x008232b8;
// GLOBAL: CMR2 0x00823498
Menu g_menu0x00823498;
// GLOBAL: CMR2 0x00823678
Menu g_menu0x00823678;
// GLOBAL: CMR2 0x00823858
Menu g_menu0x00823858;
// GLOBAL: CMR2 0x00823a38
Menu g_menu0x00823a38;
// GLOBAL: CMR2 0x00823c18
Menu g_menu0x00823c18;
// GLOBAL: CMR2 0x00823df8
Menu g_menu0x00823df8;
// GLOBAL: CMR2 0x00823fd8
Menu g_menu0x00823fd8;
// GLOBAL: CMR2 0x008241b8
Menu g_menu0x008241b8;
// GLOBAL: CMR2 0x00824498
Menu g_menu0x00824498;
// GLOBAL: CMR2 0x00824678
Menu g_menu0x00824678;
// GLOBAL: CMR2 0x00824858
Menu g_menu0x00824858;
// GLOBAL: CMR2 0x00824a38
Menu g_menu0x00824a38;
// GLOBAL: CMR2 0x00824c18
Menu g_menu0x00824c18;
// GLOBAL: CMR2 0x00824df8
Menu g_menu0x00824df8;
// GLOBAL: CMR2 0x00824fd8
Menu g_menu0x00824fd8;
// GLOBAL: CMR2 0x008251b8
Menu g_menu0x008251b8;
// GLOBAL: CMR2 0x00826140
Menu g_menu0x00826140;
// GLOBAL: CMR2 0x00826420
Menu g_menu0x00826420;
// GLOBAL: CMR2 0x00826600
Menu g_menu0x00826600;
// GLOBAL: CMR2 0x008267e0
Menu g_menu0x008267e0;
// GLOBAL: CMR2 0x00826ba0
Menu g_menu0x00826ba0;
// GLOBAL: CMR2 0x00826d80
Menu g_menu0x00826d80;
// GLOBAL: CMR2 0x00826f60
Menu g_menu0x00826f60;
// GLOBAL: CMR2 0x00827140
Menu g_menu0x00827140;
// GLOBAL: CMR2 0x00827320
Menu g_menu0x00827320;
// GLOBAL: CMR2 0x00827500
Menu g_menu0x00827500;
// GLOBAL: CMR2 0x008278c0
Menu g_menu0x008278c0;
// GLOBAL: CMR2 0x00827aa0
Menu g_menu0x00827aa0;
// GLOBAL: CMR2 0x00827c80
Menu g_menu0x00827c80;
// GLOBAL: CMR2 0x00827e60
Menu g_menu0x00827e60;
// GLOBAL: CMR2 0x00828040
Menu g_menu0x00828040;
// GLOBAL: CMR2 0x00828220
Menu g_menu0x00828220;
// GLOBAL: CMR2 0x00828500
Menu g_menu0x00828500;
// GLOBAL: CMR2 0x008286e0
Menu g_menu0x008286e0;
// GLOBAL: CMR2 0x008288c0
Menu g_menu0x008288c0;
// GLOBAL: CMR2 0x00828aa0
Menu g_menu0x00828aa0;
// GLOBAL: CMR2 0x00828c80
Menu g_menu0x00828c80;
// GLOBAL: CMR2 0x00828e60
Menu g_menu0x00828e60;
// GLOBAL: CMR2 0x00829140
Menu g_menu0x00829140;

// FUNCTION: CMR2 0x004f5520
void FUN_004f5520(void)
{
    Menu_Init(&g_menu0x008205b8, 0, 1, 0, &g_menu0x0081d6d8, NULL, 1, 0, 0);
    Menu_AddItemType1(&g_menu0x008205b8, 0, 0x1b, 0, 0);
    Menu_SetCallbacks(&g_menu0x008205b8, NULL, NULL, FUN_004e2b40, NULL);
    Menu_ValidateCursor(&g_menu0x008205b8, 0);
}

void FUN_004f3400(Menu *pMenu, int param);
void FUN_004f39e0(Menu *pMenu);
void FUN_004e2da0(Menu *pMenu);
void FUN_004ef270(Menu *pMenu, char back);
void FUN_004ef300(Menu *pMenu, int param);
void FUN_004ef420(Menu *pMenu);
void FUN_004d4ba0(Menu *pMenu);
void FUN_004f25d0(Menu *pMenu, int param);
void FUN_004ec930(Menu *pMenu, int param);
char FUN_004eaa30(void);

// Language menu: five languages, or three in region 1.
// FUNCTION: CMR2 0x004f5580
void FUN_004f5580(void)
{
    Menu_Init(&g_menu0x008221d8, 0, 1, 0, NULL, NULL, 1, 0, 0);
    if (CGameInfo::GetGameRegion() == 0) {
        Menu_AddItemType2(&g_menu0x008221d8, 0, 6, &g_menu0x0081d6d8, 0, 0);
        Menu_AddItemType2(&g_menu0x008221d8, 0, 7, &g_menu0x0081d6d8, 0, 1);
        Menu_AddItemType2(&g_menu0x008221d8, 0, 8, &g_menu0x0081d6d8, 0, 2);
        Menu_AddItemType2(&g_menu0x008221d8, 0, 9, &g_menu0x0081d6d8, 0, 3);
        Menu_AddItemType2(&g_menu0x008221d8, 0, 10, &g_menu0x0081d6d8, 0, 4);
    } else {
        Menu_AddItemType2(&g_menu0x008221d8, 0, 6, &g_menu0x0081d6d8, 0, 0);
        Menu_AddItemType2(&g_menu0x008221d8, 0, 7, &g_menu0x0081d6d8, 0, 1);
        Menu_AddItemType2(&g_menu0x008221d8, 0, 8, &g_menu0x0081d6d8, 0, 2);
    }
    Menu_SetCallbacks(&g_menu0x008221d8, (MenuCallback)FUN_004f3400, (MenuCallback)FUN_004f39e0,
                      (MenuCallback)FUN_004e2da0, (MenuCallback)FUN_004ef270);
    Menu_ValidateCursor(&g_menu0x008221d8, 0);
}

// Main menu: single rally, championship, time trial, multiplayer, options,
// (extras when unlocked) and quit.
// FUNCTION: CMR2 0x004f5670
void FUN_004f5670(void)
{
    Menu_Init(&g_menu0x0081d6d8, 0, 0, 0, &g_menu0x00823df8, NULL, 1, 1, 0);
    Menu_AddItemType2(&g_menu0x0081d6d8, 0, 0x4f, &g_menu0x008212d8, 0, 0);
    Menu_AddItemType2(&g_menu0x0081d6d8, 0, 0x50, &g_menu0x00823a38, 0, 1);
    Menu_AddItemType2(&g_menu0x0081d6d8, 0, 0x51, FUN_004fa2d0(), 0, 2);
    Menu_AddItemType2(&g_menu0x0081d6d8, 0, 0x52, &g_menu0x008241b8, (int)FUN_004ec930, 3);
    Menu_AddItemType2(&g_menu0x0081d6d8, 0, 0x53, &g_menu0x0081f118, 0, 4);
    if (FUN_004eaa30() != 0)
        Menu_AddItemType2(&g_menu0x0081d6d8, 0, 0x54, &g_menu0x00822958, 0, 5);
    Menu_AddItemType2(&g_menu0x0081d6d8, 0, 0x56, &g_menu0x00823df8, 0, 6);
    Menu_SetCallbacks(&g_menu0x0081d6d8, (MenuCallback)FUN_004ef300, (MenuCallback)FUN_004ef420,
                      (MenuCallback)FUN_004d4ba0, (MenuCallback)FUN_004f25d0);
    Menu_ValidateCursor(&g_menu0x0081d6d8, 0);
}

void FUN_004f0c40(Menu *pMenu, int param);
void FUN_004f2750(Menu *pMenu, int param);
void FUN_004f27d0(Menu *pMenu, int param);
void FUN_004f2620(Menu *pMenu, char back);
void FUN_004f3ae0(Menu *pMenu);
void FUN_004d9a40(Menu *pMenu);
void FUN_004f07e0(Menu *pMenu, char back);

// Profile menu of a rally: continue, new profile and up to 4 saved ones.
// FUNCTION: CMR2 0x004f5770
void FUN_004f5770(void)
{
    int i;

    Menu_Init(&g_menu0x008212d8, 0, 0xb, 0, &g_menu0x0081d6d8, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x008212d8, 0, 0x102, &g_menu0x008203d8, (int)FUN_004f0c40, -1);
    Menu_AddItemType4(&g_menu0x008212d8, 0, 0xe6, (int)FUN_004f2750, -1);
    i = 4;
    do {
        Menu_AddItemType4(&g_menu0x008212d8, 0, -1, (int)FUN_004f27d0, -1);
        i--;
    } while (i != 0);
    Menu_SetCallbacks(&g_menu0x008212d8, (MenuCallback)FUN_004f2620, (MenuCallback)FUN_004f3ae0,
                      (MenuCallback)FUN_004d9a40, (MenuCallback)FUN_004f07e0);
    Menu_ValidateCursor(&g_menu0x008212d8, 0);
}

void FUN_004f29b0(Menu *pMenu, int param);
void FUN_004e2590(Menu *pMenu);
void FUN_004f28c0(Menu *pMenu, int param);
void FUN_004ded80(Menu *pMenu);
void FUN_004f2970(Menu *pMenu, char back);
void FUN_004f3a50(Menu *pMenu, int param);

// Options menu: game, sound, graphics, controls, language (not in regions
// 2/3), cheats (id 1000) and back.
// FUNCTION: CMR2 0x004f5eb0
void FUN_004f5eb0(void)
{
    Menu_Init(&g_menu0x0081f118, 0, 0x13, 0, &g_menu0x0081d6d8, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x0081f118, 0, 0x14, &g_menu0x0081ef38, 0, 0);
    Menu_AddItemType2(&g_menu0x0081f118, 0, 0x15, &g_menu0x0081fa78, 0, 0);
    Menu_AddItemType2(&g_menu0x0081f118, 0, 0x16, &g_menu0x00823c18, 0, 0);
    Menu_AddItemType2(&g_menu0x0081f118, 0, 0x18, FUN_004fa4f0(), 0, 0);
    if (CGameInfo::GetGameRegion() != 3 && CGameInfo::GetGameRegion() != 2)
        Menu_AddItemType2(&g_menu0x0081f118, 0, 0x19, &g_menu0x008221d8, 0, 0);
    Menu_AddItemType2(&g_menu0x0081f118, 0, 0x148, &g_menu0x0081be78, 0, 1000);
    Menu_AddItemType1(&g_menu0x0081f118, 0, 0x67, 0, -1);
    Menu_SetCallbacks(&g_menu0x0081f118, (MenuCallback)FUN_004f29b0, (MenuCallback)FUN_004f3ae0,
                      (MenuCallback)FUN_004e2590, NULL);
    Menu_ValidateCursor(&g_menu0x0081f118, 0);
    g_menu0x0081f118.items[6].enabled = 1;
    g_menu0x0081f118.items[6].visible = 1;
}

// Cheats menu: the 8 cheats as on/off entries.
// FUNCTION: CMR2 0x004f5fc0
void FUN_004f5fc0(void)
{
    int i;

    Menu_Init(&g_menu0x0081be78, 0, 0x148, 0, &g_menu0x0081f118, NULL, 1, 0, 1);
    i = 0;
    do {
        Menu_AddItemType3(&g_menu0x0081be78, 0, i + 0x149, 2, 0, 0, 0, (int)FUN_004f3a50, -1);
        i++;
    } while (i < 8);
    Menu_SetCallbacks(&g_menu0x0081be78, (MenuCallback)FUN_004f28c0, NULL, (MenuCallback)FUN_004ded80,
                      (MenuCallback)FUN_004f2970);
    Menu_ValidateCursor(&g_menu0x0081be78, 0);
}

void FUN_004f21c0(Menu *pMenu, int param);
void FUN_004f2210(Menu *pMenu, int param);
void FUN_004e1920(Menu *pMenu);

// Display device menu: one entry per display device.
// FUNCTION: CMR2 0x004f63b0
void FUN_004f63b0(void)
{
    unsigned int i;

    Menu_Init(&g_menu0x0081c7d8, 0, 0x14, 0, &g_menu0x008210f8, NULL, 1, 0, 1);
    i = 0;
    if (CGraphics::FUN_004a8be0() != 0) {
        do {
            Menu_AddItemType4(&g_menu0x0081c7d8, 0, -1, (int)FUN_004f2210, 0);
            i++;
        } while (i < (unsigned int)CGraphics::FUN_004a8be0());
    }
    Menu_SetCallbacks(&g_menu0x0081c7d8, (MenuCallback)FUN_004f21c0, NULL, (MenuCallback)FUN_004e1920, NULL);
    Menu_ValidateCursor(&g_menu0x0081c7d8, 0);
}

void FUN_004f3a90(Menu *pMenu, int param);
void FUN_004e3340(Menu *pMenu);

// High score page (the table shown cycles on each press).
// FUNCTION: CMR2 0x004f66e0
void FUN_004f66e0(void)
{
    Menu_Init(&g_menu0x00820b58, 0, 0x162, 0, &g_menu0x00823c18, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x00820b58, 0, -1, (int)FUN_004f3a90, -1);
    Menu_SetCallbacks(&g_menu0x00820b58, NULL, NULL, (MenuCallback)FUN_004e3340, NULL);
    Menu_ValidateCursor(&g_menu0x00820b58, 0);
    g_menu0x00820b58.items[0].flag3 = 1;
}

void FUN_004e3a80(Menu *pMenu);

// Record page (the table shown cycles on each press).
// FUNCTION: CMR2 0x004f69f0
void FUN_004f69f0(void)
{
    Menu_Init(&g_menu0x00823498, 0, 0x162, 0, &g_menu0x00820d38, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x00823498, 0, -1, (int)FUN_004f3a90, -1);
    Menu_SetCallbacks(&g_menu0x00823498, NULL, NULL, (MenuCallback)FUN_004e3a80, NULL);
    Menu_ValidateCursor(&g_menu0x00823498, 0);
    g_menu0x00823498.items[0].flag3 = 1;
}

void FUN_004f2e70(Menu *pMenu, int param);
void FUN_004f3980(Menu *pMenu);
void FUN_004e5630(Menu *pMenu);

// Stage records page: one entry per rally.
// FUNCTION: CMR2 0x004f6b40
void FUN_004f6b40(void)
{
    Menu_Init(&g_menu0x00822ef8, 0, 0x164, 0, &g_menu0x00820d38, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x00822ef8, 0, 0x27, 0, -1);
    Menu_AddItemType4(&g_menu0x00822ef8, 0, 0x28, 0, -1);
    Menu_AddItemType4(&g_menu0x00822ef8, 0, 0x29, 0, -1);
    Menu_AddItemType4(&g_menu0x00822ef8, 0, 0x2a, 0, -1);
    Menu_AddItemType4(&g_menu0x00822ef8, 0, 0x2b, 0, -1);
    Menu_AddItemType4(&g_menu0x00822ef8, 0, 0x2c, 0, -1);
    Menu_AddItemType4(&g_menu0x00822ef8, 0, 0x2d, 0, -1);
    Menu_AddItemType4(&g_menu0x00822ef8, 0, 0x2e, 0, -1);
    Menu_SetCallbacks(&g_menu0x00822ef8, (MenuCallback)FUN_004f2e70, (MenuCallback)FUN_004f3980,
                      (MenuCallback)FUN_004e5630, NULL);
    Menu_ValidateCursor(&g_menu0x00822ef8, 0);
    g_menu0x00822ef8.items[0].flag3 = 1;
    g_menu0x00822ef8.items[1].flag3 = 1;
    g_menu0x00822ef8.items[2].flag3 = 1;
    g_menu0x00822ef8.items[3].flag3 = 1;
    g_menu0x00822ef8.items[4].flag3 = 1;
    g_menu0x00822ef8.items[5].flag3 = 1;
    g_menu0x00822ef8.items[6].flag3 = 1;
    g_menu0x00822ef8.items[7].flag3 = 1;
}

void FUN_004ef7c0(Menu *pMenu, int param);
void FUN_004f3610(Menu *pMenu, int param);
void FUN_004f3530(Menu *pMenu, int param);
void FUN_004f39f0(Menu *pMenu);
void FUN_004d5fb0(Menu *pMenu);

// Difficulty page (4 levels).
// FUNCTION: CMR2 0x004f6e50
void FUN_004f6e50(char difficulty)
{
    Menu_Init(&g_menu0x0081d318, 0, 0x1c, 0, &g_menu0x0081d4f8, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081d318, 0, 0xc5, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x0081d318, 0, 0xc6, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x0081d318, 0, 0xc7, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x0081d318, 0, 0xc8, (int)FUN_004ef7c0, -1);
    Menu_SetCallbacks(&g_menu0x0081d318, (MenuCallback)FUN_004f3610, (MenuCallback)FUN_004f39f0, (MenuCallback)FUN_004d5fb0, NULL);
    Menu_ValidateCursor(&g_menu0x0081d318, 0);
    g_menu0x0081d318.cursor = difficulty - 1;
}

// Difficulty page (4 levels).
// FUNCTION: CMR2 0x004f6f10
void FUN_004f6f10(char difficulty)
{
    Menu_Init(&g_menu0x00820978, 0, 0x1c, 0, &g_menu0x00824498, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x00820978, 0, 0xc5, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x00820978, 0, 0xc6, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x00820978, 0, 0xc7, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x00820978, 0, 0xc8, (int)FUN_004ef7c0, -1);
    Menu_SetCallbacks(&g_menu0x00820978, (MenuCallback)FUN_004f3610, (MenuCallback)FUN_004f39f0, (MenuCallback)FUN_004d5fb0, NULL);
    Menu_ValidateCursor(&g_menu0x00820978, 0);
    g_menu0x00820978.cursor = difficulty - 1;
}

// Difficulty page (4 levels).
// FUNCTION: CMR2 0x004f6fd0
void FUN_004f6fd0(char difficulty)
{
    Menu_Init(&g_menu0x0081f2f8, 0, 0x1c, 0, &g_menu0x00823fd8, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081f2f8, 0, 0xc5, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x0081f2f8, 0, 0xc6, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x0081f2f8, 0, 0xc7, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x0081f2f8, 0, 0xc8, (int)FUN_004ef7c0, -1);
    Menu_SetCallbacks(&g_menu0x0081f2f8, (MenuCallback)FUN_004f3610, (MenuCallback)FUN_004f39f0, (MenuCallback)FUN_004d5fb0, NULL);
    Menu_ValidateCursor(&g_menu0x0081f2f8, 0);
    g_menu0x0081f2f8.cursor = difficulty - 1;
}

// Difficulty page (4 levels).
// FUNCTION: CMR2 0x004f7090
void FUN_004f7090(char difficulty)
{
    Menu_Init(&g_menu0x00824678, 0, 0x1c, 0, &g_menu0x00823a38, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x00824678, 0, 0xc5, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x00824678, 0, 0xc6, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x00824678, 0, 0xc7, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x00824678, 0, 0xc8, (int)FUN_004ef7c0, -1);
    Menu_SetCallbacks(&g_menu0x00824678, (MenuCallback)FUN_004f3610, (MenuCallback)FUN_004f39f0, (MenuCallback)FUN_004d5fb0, NULL);
    Menu_ValidateCursor(&g_menu0x00824678, 0);
    g_menu0x00824678.cursor = difficulty - 1;
}

// Difficulty page (8 levels).
// FUNCTION: CMR2 0x004f7150
void FUN_004f7150(char difficulty)
{
    Menu_Init(&g_menu0x0081e5d8, 0, 0x40, 0, &g_menu0x00823a38, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081e5d8, 0, 0xc5, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x0081e5d8, 0, 0xc6, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x0081e5d8, 0, 0xc7, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x0081e5d8, 0, 0xc8, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x0081e5d8, 0, 0xc9, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x0081e5d8, 0, 0xca, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x0081e5d8, 0, 0xcb, (int)FUN_004ef7c0, -1);
    Menu_AddItemType4(&g_menu0x0081e5d8, 0, 0xcc, (int)FUN_004ef7c0, -1);
    Menu_SetCallbacks(&g_menu0x0081e5d8, (MenuCallback)FUN_004f3530, (MenuCallback)FUN_004f39f0, (MenuCallback)FUN_004d5fb0, NULL);
    Menu_ValidateCursor(&g_menu0x0081e5d8, 0);
    g_menu0x0081e5d8.cursor = difficulty - 1;
}

void FUN_004d6460(Menu *pMenu);
void FUN_004ef410(Menu *pMenu);
void FUN_004efb20(Menu *pMenu, int param);

// Two-choice page after a difficulty page.
// FUNCTION: CMR2 0x004f7510
void FUN_004f7510(char choice)
{
    Menu_Init(&g_menu0x008230d8, 0, 0x1e, 0, &g_menu0x0081d318, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x008230d8, 0, 0xd3, &g_menu0x008241b8, (int)FUN_004efb20, -1);
    Menu_AddItemType2(&g_menu0x008230d8, 0, 0xd4, &g_menu0x008241b8, (int)FUN_004efb20, -1);
    Menu_SetCallbacks(&g_menu0x008230d8, NULL, (MenuCallback)FUN_004ef410, (MenuCallback)FUN_004d6460, NULL);
    Menu_ValidateCursor(&g_menu0x008230d8, 0);
    g_menu0x008230d8.cursor = choice;
    g_menu0x008230d8.items[0].enabled = 1;
}

// Two-choice page after a difficulty page.
// FUNCTION: CMR2 0x004f75b0
void FUN_004f75b0(char choice)
{
    Menu_Init(&g_menu0x00824858, 0, 0x1e, 0, &g_menu0x00820978, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00824858, 0, 0xd3, &g_menu0x008241b8, (int)FUN_004efb20, -1);
    Menu_AddItemType2(&g_menu0x00824858, 0, 0xd4, &g_menu0x008241b8, (int)FUN_004efb20, -1);
    Menu_SetCallbacks(&g_menu0x00824858, NULL, (MenuCallback)FUN_004ef410, (MenuCallback)FUN_004d6460, NULL);
    Menu_ValidateCursor(&g_menu0x00824858, 0);
    g_menu0x00824858.cursor = choice;
    g_menu0x00824858.items[0].enabled = 1;
}

// Two-choice page after a difficulty page.
// FUNCTION: CMR2 0x004f7650
void FUN_004f7650(char choice)
{
    Menu_Init(&g_menu0x008251b8, 0, 0x1e, 0, &g_menu0x0081f2f8, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x008251b8, 0, 0xd3, &g_menu0x008241b8, (int)FUN_004efb20, -1);
    Menu_AddItemType2(&g_menu0x008251b8, 0, 0xd4, &g_menu0x008241b8, (int)FUN_004efb20, -1);
    Menu_SetCallbacks(&g_menu0x008251b8, NULL, (MenuCallback)FUN_004ef410, (MenuCallback)FUN_004d6460, NULL);
    Menu_ValidateCursor(&g_menu0x008251b8, 0);
    g_menu0x008251b8.cursor = choice;
    g_menu0x008251b8.items[0].enabled = 1;
}

void FUN_004f03f0(Menu *pMenu, char back);
void FUN_004f0960(Menu *pMenu, int param);
void FUN_004f0ac0(Menu *pMenu, int param);
void FUN_004f0c50(Menu *pMenu, int param);
void FUN_004d9880(Menu *pMenu);

// Player profile menu of the multiplayer modes: continue, new profile,
// no profile and up to 4 saved ones.
// FUNCTION: CMR2 0x004f76f0
void FUN_004f76f0(void)
{
    int i;

    Menu_Init(&g_menu0x008241b8, 0, 0x21, 0, NULL, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x008241b8, 0, 0x102, &g_menu0x008203d8, (int)FUN_004f0c40, -1);
    Menu_AddItemType4(&g_menu0x008241b8, 0, 0xe6, (int)FUN_004f0960, 0);
    Menu_AddItemType4(&g_menu0x008241b8, 0, 0xe5, (int)FUN_004f0ac0, 0);
    i = 4;
    do {
        Menu_AddItemType4(&g_menu0x008241b8, 0, -1, (int)FUN_004f0c50, -1);
        i--;
    } while (i != 0);
    Menu_SetCallbacks(&g_menu0x008241b8, (MenuCallback)FUN_004f03f0, NULL, (MenuCallback)FUN_004d9880,
                      (MenuCallback)FUN_004f07e0);
    Menu_ValidateCursor(&g_menu0x008241b8, 0);
}

void FUN_004ef970(Menu *pMenu, int param);
void FUN_004f01c0(Menu *pMenu, int param);
void FUN_004f0250(Menu *pMenu);
void FUN_004e1230(Menu *pMenu);

// Multiplayer race settings page: three settings and "start".
// FUNCTION: CMR2 0x004f8170
void FUN_004f8170(void)
{
    Menu_Init(&g_menu0x00823858, 0, 0x46, 0, &g_menu0x00822d18, NULL, 1, 3, 1);
    Menu_AddItemType3(&g_menu0x00823858, 0, -1, 4, 0, 0, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x00823858, 0, -1, 4, 0, 0, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x00823858, 0, -1, 5, 0, 0, 0, 0, 2);
    Menu_AddItemType4(&g_menu0x00823858, 0, -1, (int)FUN_004f01c0, 3);
    Menu_SetCallbacks(&g_menu0x00823858, (MenuCallback)FUN_004ef970, (MenuCallback)FUN_004f0250,
                      (MenuCallback)FUN_004e1230, NULL);
    Menu_ValidateCursor(&g_menu0x00823858, 0);
    g_menu0x00823858.items[0].flag3 = 1;
    g_menu0x00823858.items[1].flag3 = 1;
    g_menu0x00823858.items[2].flag3 = 1;
}

// FUNCTION: CMR2 0x004f5810
void FUN_004f5810(void)
{
    Menu_Init(&g_menu0x0081fe38, 0, 0x1a, 0, &g_menu0x008212d8, NULL, 1, 0, 1);
    Menu_SetCallbacks(&g_menu0x0081fe38, NULL, NULL, FUN_004d43e0, NULL);
    Menu_ValidateCursor(&g_menu0x0081fe38, 0);
}

// FUNCTION: CMR2 0x004f5850
void FUN_004f5850(void)
{
    Menu_Init(&g_menu0x0081c058, 0, 0x59, 0, &g_menu0x008212d8, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x0081c058, 0, 0x5a, &g_menu0x00823678, 0, -1);
    Menu_AddItemType2(&g_menu0x0081c058, 0, 0x82, &g_menu0x00820d38, 0, -1);
    Menu_AddItemType2(&g_menu0x0081c058, 0, 0x84, &g_menu0x00821878, 0, -1);
    Menu_SetCallbacks(&g_menu0x0081c058, (MenuCallback)FUN_004f3ac0, NULL, (MenuCallback)FUN_004d4c40, NULL);
    Menu_ValidateCursor(&g_menu0x0081c058, 0);
}

// FUNCTION: CMR2 0x004f58e0
void FUN_004f58e0(void)
{
    Menu_Init(&g_menu0x00823678, 0, 0x5a, 0, &g_menu0x0081c058, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x00823678, 0, -1, 0, -1);
    Menu_SetCallbacks(&g_menu0x00823678, NULL, NULL, FUN_004d4cf0, NULL);
    Menu_ValidateCursor(&g_menu0x00823678, 0);
}

// FUNCTION: CMR2 0x004f5940
void FUN_004f5940(void)
{
    Menu_Init(&g_menu0x00821878, 0, 0x84, 0, &g_menu0x0081c058, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x00821878, 0, 0x82, 0, 0);
    Menu_SetCallbacks(&g_menu0x00821878, NULL, NULL, FUN_004d50a0, NULL);
    Menu_ValidateCursor(&g_menu0x00821878, 0);
}

// FUNCTION: CMR2 0x004f59a0
void FUN_004f59a0(void)
{
    Menu_Init(&g_menu0x00823a38, 0, 0x58, 0, &g_menu0x0081d6d8, NULL, 1, 2, 1);
    Menu_AddItemType2(&g_menu0x00823a38, 0, 0xc, &g_menu0x0081dc78, 0, 0);
    Menu_AddItemType2(&g_menu0x00823a38, 0, 0xd, &g_menu0x00824498, 0, 0);
    Menu_AddItemType2(&g_menu0x00823a38, 0, 0xf, &g_menu0x00823fd8, 0, 0);
    Menu_AddItemType2(&g_menu0x00823a38, 0, 0x10, &g_menu0x00824678, 0, 0);
    Menu_AddItemType2(&g_menu0x00823a38, 0, 0x11, &g_menu0x0081e5d8, 0, 0);
    Menu_SetCallbacks(&g_menu0x00823a38, FUN_004ef5f0, (MenuCallback)FUN_004f3ae0, (MenuCallback)FUN_004e1fb0, FUN_004ef600);
    Menu_ValidateCursor(&g_menu0x00823a38, 0);
}

// FUNCTION: CMR2 0x004f5a60
void FUN_004f5a60(void)
{
    Menu_Init(&g_menu0x0081dc78, 0, 0xda, 0, &g_menu0x00823a38, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x0081dc78, 0, 0x95, &g_menu0x0081d4f8, 0, 0);
    Menu_AddItemType2(&g_menu0x0081dc78, 0, 0x8a, &g_menu0x00820018, 0, -1);
    Menu_SetCallbacks(&g_menu0x0081dc78, NULL, NULL, (MenuCallback)FUN_004e2040, NULL);
    Menu_ValidateCursor(&g_menu0x0081dc78, 0);
}

// FUNCTION: CMR2 0x004f5ae0
void FUN_004f5ae0(void)
{
    Menu_Init(&g_menu0x00820018, 0, 0xda, 0, &g_menu0x0081dc78, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x00820018, 0, -1, 1, 0, 0, 0, (int)FUN_004f3a00, -1);
    Menu_SetCallbacks(&g_menu0x00820018, FUN_004ef4c0, (MenuCallback)FUN_004ef4e0, FUN_004e20e0, NULL);
    Menu_ValidateCursor(&g_menu0x00820018, 0);
}

// FUNCTION: CMR2 0x004f5b50
void FUN_004f5b50(void)
{
    Menu_Init(&g_menu0x008214b8, 0, 0, 0, NULL, NULL, 1, 0, 1);
    Menu_SetCallbacks(&g_menu0x008214b8, NULL, FUN_004ef590, NULL, NULL);
    Menu_ValidateCursor(&g_menu0x008214b8, 0);
}

// FUNCTION: CMR2 0x004f5b90
void FUN_004f5b90(void)
{
    Menu_Init(&g_menu0x00822598, 0, 0x3c, 0, &g_menu0x008241b8, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x00822598, 0, -1, 0xa, 0, 1, 0, (int)FUN_004ecf80, 0);
    Menu_SetCallbacks(&g_menu0x00822598, FUN_004ec9a0, NULL, (MenuCallback)FUN_004dc7b0, (MenuCallback)FUN_004ecfa0);
    Menu_ValidateCursor(&g_menu0x00822598, 0);
}

// FUNCTION: CMR2 0x004f5c00
void FUN_004f5c00(void)
{
    Menu_Init(&g_menu0x00824c18, 0, 0x35, 0, &g_menu0x00822598, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x00824c18, 0, 0x1ec, (int)FUN_004ecd60, 0);
    Menu_AddItemType4(&g_menu0x00824c18, 0, 0x3d, (int)FUN_004ecea0, 0);
    Menu_AddItemType1(&g_menu0x00824c18, 0, 0x1b, 0, 0);
    Menu_SetCallbacks(&g_menu0x00824c18, (MenuCallback)FUN_004eca60, FUN_004ecaf0, FUN_004dc930, (MenuCallback)FUN_004ecfa0);
    Menu_ValidateCursor(&g_menu0x00824c18, 0);
}

// FUNCTION: CMR2 0x004f5c90
void FUN_004f5c90(void)
{
    Menu_Init(&g_menu0x00821c38, 0, 0x3d, 0, &g_menu0x00824c18, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x00821c38, 0, -1, 0, 0);
    Menu_AddItemType3(&g_menu0x00821c38, 0, -1, 5, 0, 1, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x00821c38, 0, -1, 0x7a, 0xb, 1, 0, 0, 2);
    Menu_AddItemType3(&g_menu0x00821c38, 0, -1, 8, 6, 1, 0, 0, 3);
    Menu_AddItemType4(&g_menu0x00821c38, 0, -1, 0, 4);
    Menu_AddItemType2(&g_menu0x00821c38, 0, -1, &g_menu0x0081bab8, 0, 5);
    Menu_AddItemType4(&g_menu0x00821c38, 0, 0x67, (int)FUN_004ed340, 6);
    Menu_AddItemType1(&g_menu0x00821c38, 0, 0x1b, 0, 7);
    Menu_SetCallbacks(&g_menu0x00821c38, FUN_004ecfd0, FUN_004ed100, FUN_004dce00, (MenuCallback)FUN_004ed500);
    Menu_ValidateCursor(&g_menu0x00821c38, 0);
}

// FUNCTION: CMR2 0x004f5d90
void FUN_004f5d90(void)
{
    Menu_Init(&g_menu0x0081e218, 0, 0x3e, 0, &g_menu0x00824c18, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x0081e218, 0, -1, 0, 0);
    Menu_AddItemType3(&g_menu0x0081e218, 0, -1, 0x16, 0, 1, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x0081e218, 0, -1, 2, 0, 1, 0, 0, 2);
    Menu_AddItemType3(&g_menu0x0081e218, 0, -1, 2, 0, 1, 0, (int)FUN_004ed610, 3);
    Menu_AddItemType1(&g_menu0x0081e218, 0, 0x1b, 0, 4);
    Menu_SetCallbacks(&g_menu0x0081e218, FUN_004ed530, FUN_004ed840, FUN_004dd4b0, (MenuCallback)FUN_004edb30);
    Menu_ValidateCursor(&g_menu0x0081e218, 0);
}

// FUNCTION: CMR2 0x004f5e50
void FUN_004f5e50(void)
{
    Menu_Init(&g_menu0x00824df8, 0, -1, 0, NULL, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00824df8, 0, -1, &g_menu0x0081e218, 0, -1);
    Menu_SetCallbacks(&g_menu0x00824df8, NULL, FUN_004edb60, FUN_004de1d0, NULL);
    Menu_ValidateCursor(&g_menu0x00824df8, 0);
}

// FUNCTION: CMR2 0x004f6040
void FUN_004f6040(void)
{
    Menu_Init(&g_menu0x0081ef38, 0, 0x14, 0, &g_menu0x0081f118, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081ef38, 0, 0x91, 2, 0, 0, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081ef38, 0, 0x93, 2, 0, 0, 0, 0, 2);
    Menu_AddItemType3(&g_menu0x0081ef38, 0, 0x96, 2, 0, 0, 0, 0, 3);
    Menu_AddItemType3(&g_menu0x0081ef38, 0, 0x151, 3, 0, 0, 0, 0, 4);
    Menu_AddItemType2(&g_menu0x0081ef38, 0, 0x8e, &g_menu0x008210f8, 0, -1);
    Menu_AddItemType2(&g_menu0x0081ef38, 0, 0x67, g_menu0x0081ef38.pParent, (int)FUN_004f2d20, -1);
    Menu_SetCallbacks(&g_menu0x0081ef38, (MenuCallback)FUN_004f2c40, NULL, FUN_004df410, NULL);
    Menu_ValidateCursor(&g_menu0x0081ef38, 0);
}

// FUNCTION: CMR2 0x004f6130
void FUN_004f6130(void)
{
    Menu_Init(&g_menu0x008210f8, 0, 0x8e, 0, &g_menu0x0081ef38, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x008210f8, 0, -1, &g_menu0x0081de58, 0, 0);
    Menu_AddItemType3(&g_menu0x008210f8, 0, -1, 2, 0, 0, 0, 0, 1);
    Menu_AddItemType2(&g_menu0x008210f8, 0, -1, &g_menu0x0081c7d8, 0, 2);
    Menu_AddItemType2(&g_menu0x008210f8, 0, -1, &g_menu0x0081cb98, 0, 3);
    Menu_AddItemType3(&g_menu0x008210f8, 0, -1, 3, 0, 0, 0, 0, 4);
    Menu_AddItemType3(&g_menu0x008210f8, 0, -1, 0xa, 0, 0, 0, 0, 5);
    Menu_AddItemType2(&g_menu0x008210f8, 0, 0x1ac, &g_menu0x0081cd78, 0, 6);
    Menu_AddItemType1(&g_menu0x008210f8, 0, 0x67, (int)FUN_004f1e40, 0xe);
    Menu_SetCallbacks(&g_menu0x008210f8, FUN_004f1c00, NULL, FUN_004dfe20, (MenuCallback)FUN_004f1f70);
    Menu_ValidateCursor(&g_menu0x008210f8, 0);
}

// FUNCTION: CMR2 0x004f6240
void FUN_004f6240(void)
{
    Menu_Init(&g_menu0x0081cd78, 0, 0x1b4, 0, &g_menu0x008210f8, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081cd78, 0, 0x1b8, 3, 0, 0, 0, 0, 8);
    Menu_AddItemType3(&g_menu0x0081cd78, 0, 0x1b7, 2, 0, 0, 0, 0, 9);
    Menu_AddItemType3(&g_menu0x0081cd78, 0, 0x1b6, 2, 0, 0, 0, 0, 0xa);
    Menu_AddItemType1(&g_menu0x0081cd78, 0, 0x67, (int)FUN_004f1db0, 0xe);
    Menu_SetCallbacks(&g_menu0x0081cd78, FUN_004f1d00, (MenuCallback)FUN_004f1d60, FUN_004e0770, NULL);
    Menu_ValidateCursor(&g_menu0x0081cd78, 0);
}

// FUNCTION: CMR2 0x004f6300
void FUN_004f6300(void)
{
    Menu_Init(&g_menu0x0081de58, 0, 0x1b1, 0, &g_menu0x008210f8, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x0081de58, 0, 0x133, (int)FUN_004f2050, 0);
    Menu_AddItemType4(&g_menu0x0081de58, 0, 0x1ae, (int)FUN_004f2050, 0);
    Menu_AddItemType4(&g_menu0x0081de58, 0, 0x1af, (int)FUN_004f2050, 0);
    Menu_AddItemType4(&g_menu0x0081de58, 0, 0x1b0, (int)FUN_004f2050, 0);
    Menu_SetCallbacks(&g_menu0x0081de58, FUN_004f1fa0, NULL, (MenuCallback)FUN_004e1890, NULL);
    Menu_ValidateCursor(&g_menu0x0081de58, 0);
}

// FUNCTION: CMR2 0x004f6420
void FUN_004f6420(void)
{
    Menu_Init(&g_menu0x0081cb98, 0, 0x14, 0, &g_menu0x008210f8, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x0081cb98, 0, -1, 2, 0, 0, 0, (int)FUN_004f2430, 0);
    Menu_SetCallbacks(&g_menu0x0081cb98, (MenuCallback)FUN_004f2360, (MenuCallback)FUN_004f23d0, FUN_004e1d70, NULL);
    Menu_ValidateCursor(&g_menu0x0081cb98, 0);
}

// FUNCTION: CMR2 0x004f6490
void FUN_004f6490(void)
{
    Menu_Init(&g_menu0x0081fa78, 0, 0x15, 0, &g_menu0x0081f118, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081fa78, 0, 0x5e, 0xb, 0xa, 0, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081fa78, 0, 0x5f, 0xb, 0xa, 0, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x0081fa78, 0, 0x60, 0xb, 0xa, 0, 0, 0, 2);
    Menu_AddItemType2(&g_menu0x0081fa78, 0, 0x67, g_menu0x0081fa78.pParent, 0, 4);
    Menu_SetCallbacks(&g_menu0x0081fa78, FUN_004f2840, (MenuCallback)FUN_004f2b00, FUN_004e2610, (MenuCallback)FUN_004f2b70);
    Menu_ValidateCursor(&g_menu0x0081fa78, 0);
}

// FUNCTION: CMR2 0x004f6540
void FUN_004f6540(void)
{
    Menu_Init(&g_menu0x00823c18, 0, 0x16, 0, &g_menu0x0081f118, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00823c18, 0, 0x162, &g_menu0x00820b58, 0, -1);
    Menu_AddItemType2(&g_menu0x00823c18, 0, 0x163, &g_menu0x0081c418, 0, -1);
    Menu_AddItemType2(&g_menu0x00823c18, 0, 0x164, &g_menu0x008223b8, 0, -1);
    Menu_AddItemType2(&g_menu0x00823c18, 0, 0x165, &g_menu0x008232b8, 0, -1);
    Menu_AddItemType2(&g_menu0x00823c18, 0, 0x166, &g_menu0x0081c5f8, 0, -1);
    Menu_SetCallbacks(&g_menu0x00823c18, FUN_004f3a70, NULL, (MenuCallback)FUN_004e3230, NULL);
    Menu_ValidateCursor(&g_menu0x00823c18, 0);
}

// FUNCTION: CMR2 0x004f6610
void FUN_004f6610(void)
{
    Menu_Init(&g_menu0x00820d38, 0, 0x16, 0, &g_menu0x0081c058, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00820d38, 0, 0x162, &g_menu0x00823498, 0, -1);
    Menu_AddItemType2(&g_menu0x00820d38, 0, 0x163, &g_menu0x008201f8, 0, -1);
    Menu_AddItemType2(&g_menu0x00820d38, 0, 0x164, &g_menu0x00822ef8, 0, -1);
    Menu_AddItemType2(&g_menu0x00820d38, 0, 0x165, &g_menu0x0081b518, 0, -1);
    Menu_AddItemType2(&g_menu0x00820d38, 0, 0x166, &g_menu0x0081f6b8, 0, -1);
    Menu_SetCallbacks(&g_menu0x00820d38, FUN_004f3a70, NULL, (MenuCallback)FUN_004e3230, NULL);
    Menu_ValidateCursor(&g_menu0x00820d38, 0);
}

// FUNCTION: CMR2 0x004f6740
void FUN_004f6740(void)
{
    Menu_Init(&g_menu0x0081c418, 0, 0x163, 0, &g_menu0x00823c18, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x27, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x28, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x29, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x2a, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x2b, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x2c, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x2d, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x2e, (int)FUN_004f3a90, -1);
    Menu_SetCallbacks(&g_menu0x0081c418, FUN_004f2d90, (MenuCallback)FUN_004f3970, FUN_004e4130, NULL);
    Menu_ValidateCursor(&g_menu0x0081c418, 0);
}

// FUNCTION: CMR2 0x004f6830
void FUN_004f6830(void)
{
    Menu_Init(&g_menu0x008223b8, 0, 0x164, 0, &g_menu0x00823c18, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x27, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x28, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x29, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x2a, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x2b, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x2c, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x2d, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x2e, 0, -1);
    Menu_SetCallbacks(&g_menu0x008223b8, FUN_004f2e70, (MenuCallback)FUN_004f3980, FUN_004e4fc0, NULL);
    Menu_ValidateCursor(&g_menu0x008223b8, 0);
}

// FUNCTION: CMR2 0x004f6910
void FUN_004f6910(void)
{
    Menu_Init(&g_menu0x008232b8, 0, 0x165, 0, &g_menu0x00823c18, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x008232b8, 0, 0xe9, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008232b8, 0, 0xe8, (int)FUN_004f3a90, -1);
    Menu_SetCallbacks(&g_menu0x008232b8, FUN_004f2f40, (MenuCallback)FUN_004f3990, FUN_004e5c90, NULL);
    Menu_ValidateCursor(&g_menu0x008232b8, 0);
}

// FUNCTION: CMR2 0x004f6990
void FUN_004f6990(void)
{
    Menu_Init(&g_menu0x0081c5f8, 0, 0x166, 0, &g_menu0x00823c18, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081c5f8, 0, -1, 0, -1);
    Menu_SetCallbacks(&g_menu0x0081c5f8, NULL, NULL, FUN_004e6a80, NULL);
    Menu_ValidateCursor(&g_menu0x0081c5f8, 0);
}

// FUNCTION: CMR2 0x004f6a50
void FUN_004f6a50(void)
{
    Menu_Init(&g_menu0x008201f8, 0, 0x163, 0, &g_menu0x00820d38, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x27, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x28, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x29, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x2a, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x2b, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x2c, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x2d, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x2e, (int)FUN_004f3a90, -1);
    Menu_SetCallbacks(&g_menu0x008201f8, FUN_004f2d90, (MenuCallback)FUN_004f3970, FUN_004e48b0, NULL);
    Menu_ValidateCursor(&g_menu0x008201f8, 0);
}

// FUNCTION: CMR2 0x004f6c90
void FUN_004f6c90(void)
{
    Menu_Init(&g_menu0x0081b518, 0, 0x165, 0, &g_menu0x00820d38, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081b518, 0, 0xe9, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081b518, 0, 0xe8, (int)FUN_004f3a90, -1);
    Menu_SetCallbacks(&g_menu0x0081b518, FUN_004f2f40, (MenuCallback)FUN_004f3990, FUN_004e63d0, NULL);
    Menu_ValidateCursor(&g_menu0x0081b518, 0);
}

// FUNCTION: CMR2 0x004f6d10
void FUN_004f6d10(void)
{
    Menu_Init(&g_menu0x0081f6b8, 0, 0x166, 0, &g_menu0x00820d38, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081f6b8, 0, -1, 0, -1);
    Menu_SetCallbacks(&g_menu0x0081f6b8, NULL, NULL, FUN_004e7120, NULL);
    Menu_ValidateCursor(&g_menu0x0081f6b8, 0);
}

// FUNCTION: CMR2 0x004f6d70
void FUN_004f6d70(void)
{
    Menu_Init(&g_menu0x00822958, 0, 0x34, 0, &g_menu0x0081d6d8, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x00822958, 0, -1, 0, 0);
    Menu_SetCallbacks(&g_menu0x00822958, FUN_004f1bd0, NULL, FUN_004dec30, NULL);
    Menu_ValidateCursor(&g_menu0x00822958, 0);
}

// FUNCTION: CMR2 0x004f6dd0
void FUN_004f6dd0(void)
{
    Menu_Init(&g_menu0x00823df8, 0, 3, 0, &g_menu0x0081d6d8, NULL, 1, 1, 1);
    Menu_AddItemType2(&g_menu0x00823df8, 0, 5, &g_menu0x00820798, 0, 0);
    Menu_AddItemType1(&g_menu0x00823df8, 0, 4, (int)FUN_004f3b20, 0);
    Menu_SetCallbacks(&g_menu0x00823df8, FUN_004f3b00, (MenuCallback)FUN_004f3ae0, (MenuCallback)FUN_004e2ab0, (MenuCallback)FUN_004f3b30);
    Menu_ValidateCursor(&g_menu0x00823df8, 0);
}

// FUNCTION: CMR2 0x004f7270
void FUN_004f7270(int unused)
{
    Menu_Init(&g_menu0x0081d4f8, 0, 0x1d, 0, &g_menu0x0081dc78, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x0081d4f8, 0, 0xd0, &g_menu0x0081d318, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x0081d4f8, 0, 0xd1, &g_menu0x0081d318, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x0081d4f8, 0, 0xd2, &g_menu0x0081d318, (int)FUN_004ef930, 0);
    Menu_SetCallbacks(&g_menu0x0081d4f8, FUN_004ef8f0, NULL, (MenuCallback)FUN_004d6290, NULL);
    Menu_ValidateCursor(&g_menu0x0081d4f8, 0);
}

// FUNCTION: CMR2 0x004f7310
void FUN_004f7310(int unused)
{
    Menu_Init(&g_menu0x00824498, 0, 0x1d, 0, &g_menu0x00823a38, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00824498, 0, 0xd0, &g_menu0x00820978, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x00824498, 0, 0xd1, &g_menu0x00820978, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x00824498, 0, 0xd2, &g_menu0x00820978, (int)FUN_004ef930, 0);
    Menu_SetCallbacks(&g_menu0x00824498, FUN_004ef8f0, NULL, (MenuCallback)FUN_004d6290, NULL);
    Menu_ValidateCursor(&g_menu0x00824498, 0);
}

// FUNCTION: CMR2 0x004f73b0
void FUN_004f73b0(int unused)
{
    Menu_Init(&g_menu0x00823fd8, 0, 0x1d, 0, &g_menu0x00823a38, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00823fd8, 0, 0xd0, &g_menu0x0081f2f8, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x00823fd8, 0, 0xd1, &g_menu0x0081f2f8, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x00823fd8, 0, 0xd2, &g_menu0x0081f2f8, (int)FUN_004ef930, 0);
    Menu_SetCallbacks(&g_menu0x00823fd8, FUN_004ef8f0, NULL, (MenuCallback)FUN_004d6290, NULL);
    Menu_ValidateCursor(&g_menu0x00823fd8, 0);
}

// FUNCTION: CMR2 0x004f7450
void FUN_004f7450(void)
{
    Menu_Init(&g_menu0x0081ed58, 0, 0x41, 0, &g_menu0x00823a38, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x0081ed58, 0, 0x42, &g_menu0x00823858, (int)FUN_004ef960, -1);
    Menu_AddItemType2(&g_menu0x0081ed58, 0, 0xd0, &g_menu0x00823858, (int)FUN_004ef960, -1);
    Menu_AddItemType2(&g_menu0x0081ed58, 0, 0xd1, &g_menu0x00823858, (int)FUN_004ef960, -1);
    Menu_AddItemType2(&g_menu0x0081ed58, 0, 0xd2, &g_menu0x00823858, (int)FUN_004ef960, -1);
    Menu_SetCallbacks(&g_menu0x0081ed58, FUN_004ef950, NULL, (MenuCallback)FUN_004d63e0, NULL);
    Menu_ValidateCursor(&g_menu0x0081ed58, 0);
}

// FUNCTION: CMR2 0x004f77a0
void FUN_004f77a0(void)
{
    Menu_Init(&g_menu0x008203d8, 0, 0x17a, 0, &g_menu0x008241b8, NULL, 1, 0, 1);
    Menu_AddItemType6(&g_menu0x008203d8, 0, -1, 1, 0, 1, 0, (int)FUN_004f0820, -1);
    Menu_AddItemType1(&g_menu0x008203d8, 0, 0x11a, 0, 0);
    Menu_SetCallbacks(&g_menu0x008203d8, FUN_004f0580, FUN_004f0620, FUN_004d6a60, FUN_004f0600);
    Menu_ValidateCursor(&g_menu0x008203d8, 0);
}

// FUNCTION: CMR2 0x004f7820
void FUN_004f7820(void)
{
    Menu_Init(&g_menu0x00824a38, 0, 0x22, 0, NULL, NULL, 1, 0, 1);
    Menu_AddItemType6(&g_menu0x00824a38, 0, -1, 0xa, 0, 1, 0, (int)FUN_004f1040, 0);
    Menu_AddItemType6(&g_menu0x00824a38, 0, -1, 0xa, 0, 1, 0, (int)FUN_004f1040, 1);
    Menu_AddItemType6(&g_menu0x00824a38, 0, -1, 0xa, 0, 1, 0, (int)FUN_004f1040, 2);
    Menu_SetCallbacks(&g_menu0x00824a38, FUN_004f0d30, FUN_004f0e80, FUN_004d6f10, FUN_004f0da0);
    Menu_ValidateCursor(&g_menu0x00824a38, 0);
}

// FUNCTION: CMR2 0x004f78c0
void FUN_004f78c0(void)
{
    Menu_Init(&g_menu0x0081d8b8, 0, 0x8c, 0, &g_menu0x00824a38, NULL, 1, 0, 1);
    Menu_AddItemType6(&g_menu0x0081d8b8, 0, -1, 0xa, 0, 1, 0, (int)FUN_004f13c0, 0);
    Menu_AddItemType6(&g_menu0x0081d8b8, 0, -1, 0xa, 0, 1, 0, (int)FUN_004f13c0, 1);
    Menu_AddItemType6(&g_menu0x0081d8b8, 0, -1, 0xa, 0, 1, 0, (int)FUN_004f13c0, 2);
    Menu_SetCallbacks(&g_menu0x0081d8b8, (MenuCallback)FUN_004f1160, (MenuCallback)FUN_004f11d0, FUN_004d7380, FUN_004f0e60);
    Menu_ValidateCursor(&g_menu0x0081d8b8, 0);
}

// FUNCTION: CMR2 0x004f7970
void FUN_004f7970(void)
{
    Menu_Init(&g_menu0x0081bc98, 0, 0x8d, 0, &g_menu0x0081d8b8, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081bc98, 0, -1, 0xff, 0x64, 1, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081bc98, 0, -1, 0xc, 0, 1, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x0081bc98, 0, -1, 0x1f, 0, 1, 0, 0, 2);
    Menu_AddItemType4(&g_menu0x0081bc98, 0, 0x67, (int)FUN_004f15d0, -1);
    Menu_SetCallbacks(&g_menu0x0081bc98, FUN_004f16f0, (MenuCallback)FUN_004f1640, FUN_004d7750, FUN_004f1b90);
    Menu_ValidateCursor(&g_menu0x0081bc98, 0);
}

// FUNCTION: CMR2 0x004f7a30
void FUN_004f7a30(void)
{
    Menu_Init(&g_menu0x00821e18, 0, 0x33, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x98, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x99, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x9a, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x9b, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x9c, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x9d, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x9e, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x9f, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa0, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa1, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa2, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa3, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa4, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa5, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa6, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa7, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa8, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa9, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xaa, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xab, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xac, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xad, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_SetCallbacks(&g_menu0x00821e18, FUN_004f17d0, (MenuCallback)FUN_004f39d0, FUN_004d7db0, FUN_004f1b90);
    Menu_ValidateCursor(&g_menu0x00821e18, 0);
}

// FUNCTION: CMR2 0x004f7d00
void FUN_004f7d00(void)
{
    Menu_Init(&g_menu0x0081cf58, 0, 0x97, 0, &g_menu0x00821e18, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081cf58, 0, 0x88, 2, 0, 0, 0, (int)FUN_004f1a40, 0);
    Menu_SetCallbacks(&g_menu0x0081cf58, FUN_004f1960, (MenuCallback)FUN_004f39d0, FUN_004d8480, NULL);
    Menu_ValidateCursor(&g_menu0x0081cf58, 0);
}

// FUNCTION: CMR2 0x004f7d70
void FUN_004f7d70(void)
{
    Menu_Init(&g_menu0x00822d18, 0, 0x33, 0, &g_menu0x008241b8, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x00822d18, 0, 0x88, 2, 0, 0, 0, (int)FUN_004f1b30, 0);
    Menu_SetCallbacks(&g_menu0x00822d18, FUN_004f19d0, NULL, FUN_004d9450, NULL);
    Menu_ValidateCursor(&g_menu0x00822d18, 0);
}

// FUNCTION: CMR2 0x004f7de0
void FUN_004f7de0(void)
{
    Menu_Init(&g_menu0x0081eb78, 0, 0x24, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x27, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x28, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x29, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x2a, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x2b, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x2c, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x2d, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x2e, (int)FUN_004efb70, -1);
    Menu_SetCallbacks(&g_menu0x0081eb78, FUN_004f36e0, (MenuCallback)FUN_004efb50, FUN_004d9ad0, NULL);
    Menu_ValidateCursor(&g_menu0x0081eb78, 0);
}

// FUNCTION: CMR2 0x004f7ed0
void FUN_004f7ed0(void)
{
    Menu_Init(&g_menu0x0081e3f8, 0, 0x25, 0, &g_menu0x0081eb78, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xc5, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xc6, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xc7, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xc8, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xc9, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xca, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xcb, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xcc, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xcd, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xce, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xcf, (int)FUN_004efde0, -1);
    Menu_SetCallbacks(&g_menu0x0081e3f8, FUN_004f3010, (MenuCallback)FUN_004efdc0, FUN_004d9c40, NULL);
    Menu_ValidateCursor(&g_menu0x0081e3f8, 0);
}

// FUNCTION: CMR2 0x004f8020
void FUN_004f8020(void)
{
    Menu_Init(&g_menu0x0081f4d8, 0, 0x155, 0, &g_menu0x0081e3f8, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xc5, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xc6, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xc7, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xc8, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xc9, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xca, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xcb, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xcc, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xcd, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xce, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xcf, (int)FUN_004f0050, -1);
    Menu_SetCallbacks(&g_menu0x0081f4d8, FUN_004f3120, (MenuCallback)FUN_004efdd0, (MenuCallback)FUN_004da630, NULL);
    Menu_ValidateCursor(&g_menu0x0081f4d8, 0);
}

// FUNCTION: CMR2 0x004f8250
void FUN_004f8250(void)
{
    Menu_Init(&g_menu0x00820798, 0, -1, 0, &g_menu0x0081d6d8, NULL, 1, 0, 0);
    Menu_SetCallbacks(&g_menu0x00820798, NULL, FUN_004ef5e0, NULL, NULL);
    Menu_ValidateCursor(&g_menu0x00820798, 0);
}

// FUNCTION: CMR2 0x004f8500
void FUN_004f8500(void)
{
    Menu_Init(&g_menu0x00824fd8, 0, 0x13, 0, &g_menu0x00821c38, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x00824fd8, 0, -1, 0xb, 0, 1, 0, (int)FUN_004edca0, 0);
    Menu_AddItemType3(&g_menu0x00824fd8, 0, -1, 8, 0, 1, 0, 0, 1);
    Menu_AddItemType4(&g_menu0x00824fd8, 0, 0x67, (int)FUN_004edd50, 2);
    Menu_AddItemType1(&g_menu0x00824fd8, 0, 0x1b, 0, 3);
    Menu_SetCallbacks(&g_menu0x00824fd8, FUN_004edb70, FUN_004ede10, FUN_004e77c0, NULL);
    Menu_ValidateCursor(&g_menu0x00824fd8, 0);
}

// FUNCTION: CMR2 0x004f85b0
void FUN_004f85b0(void)
{
    Menu_Init(&g_menu0x0081fc58, 0, 0x13, 0, &g_menu0x00821c38, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081fc58, 0, -1, 8, 0, 1, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081fc58, 0, -1, 0xb, 0, 1, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x0081fc58, 0, -1, 4, 0, 1, 0, 0, 2);
    Menu_AddItemType4(&g_menu0x0081fc58, 0, 0x67, (int)FUN_004ee090, 3);
    Menu_AddItemType1(&g_menu0x0081fc58, 0, 0x1b, 0, 4);
    Menu_SetCallbacks(&g_menu0x0081fc58, FUN_004edef0, FUN_004ee170, FUN_004e7ed0, NULL);
    Menu_ValidateCursor(&g_menu0x0081fc58, 0);
}

// FUNCTION: CMR2 0x004f8670
void FUN_004f8670(void)
{
    Menu_Init(&g_menu0x0081b338, 0, 0x13, 0, &g_menu0x00821c38, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081b338, 0, -1, 8, 0, 1, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081b338, 0, -1, 0xb, 0, 1, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x0081b338, 0, -1, 0x39, 0, 1, 0, 0, 2);
    Menu_AddItemType4(&g_menu0x0081b338, 0, 0x67, (int)FUN_004ee600, 3);
    Menu_AddItemType1(&g_menu0x0081b338, 0, 0x1b, 0, 4);
    Menu_SetCallbacks(&g_menu0x0081b338, FUN_004ee460, FUN_004ee6e0, FUN_004e8500, NULL);
    Menu_ValidateCursor(&g_menu0x0081b338, 0);
}

// FUNCTION: CMR2 0x004f8730
void FUN_004f8730(void)
{
    Menu_Init(&g_menu0x0081e998, 0, 0x13, 0, &g_menu0x00821c38, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081e998, 0, -1, 8, 0, 1, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081e998, 0, -1, 0xa, 0, 1, 0, 0, 1);
    Menu_AddItemType4(&g_menu0x0081e998, 0, 0x67, (int)FUN_004ee9b0, 2);
    Menu_AddItemType1(&g_menu0x0081e998, 0, 0x1b, 0, 3);
    Menu_SetCallbacks(&g_menu0x0081e998, FUN_004ee850, FUN_004eec30, FUN_004e8b60, NULL);
    Menu_ValidateCursor(&g_menu0x0081e998, 0);
}

// FUNCTION: CMR2 0x004f87d0
void FUN_004f87d0(void)
{
    Menu_Init(&g_menu0x0081d138, 0, 0x13, 0, &g_menu0x00821c38, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081d138, 0, -1, 8, 0, 1, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081d138, 0, -1, 0x3d, 0, 1, 0, 0, 1);
    Menu_AddItemType4(&g_menu0x0081d138, 0, 0x67, (int)FUN_004eec40, 2);
    Menu_AddItemType1(&g_menu0x0081d138, 0, 0x1b, 0, 3);
    Menu_SetCallbacks(&g_menu0x0081d138, (MenuCallback)FUN_004eeab0, FUN_004eec30, FUN_004e90f0, NULL);
    Menu_ValidateCursor(&g_menu0x0081d138, 0);
}

// FUNCTION: CMR2 0x004f8870
void FUN_004f8870(void)
{
    Menu_Init(&g_menu0x0081b158, 0, 0x13, 0, &g_menu0x00824c18, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x0081b158, 0, -1, (int)FUN_004eee60, -1);
    Menu_SetCallbacks(&g_menu0x0081b158, FUN_004eed50, (MenuCallback)FUN_004eed80, (MenuCallback)FUN_004e9820, NULL);
    Menu_ValidateCursor(&g_menu0x0081b158, 0);
}

// FUNCTION: CMR2 0x004f88e0
void FUN_004f88e0(void)
{
    Menu_Init(&g_menu0x0081bab8, 0, 0x13, 0, &g_menu0x00821c38, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081bab8, 0, -1, 1, 0, 1, 0, (int)FUN_004eefb0, 0);
    Menu_AddItemType4(&g_menu0x0081bab8, 0, 0x20, (int)FUN_004eefe0, 1);
    Menu_AddItemType4(&g_menu0x0081bab8, 0, 0x81, (int)FUN_004ef000, 2);
    Menu_AddItemType1(&g_menu0x0081bab8, 0, 0x1b, 0, -1);
    Menu_SetCallbacks(&g_menu0x0081bab8, FUN_004eef30, (MenuCallback)FUN_004ef030, FUN_004e9990, NULL);
    Menu_ValidateCursor(&g_menu0x0081bab8, 0);
}

// FUNCTION: CMR2 0x004f95d0
void FUN_004f95d0(int unused)
{
    Menu_Init(&g_menu0x00828220, 0, 0x1d, 0, &g_menu0x008267e0, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00828220, 0, 0xd0, &g_menu0x00827320, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x00828220, 0, 0xd1, &g_menu0x00827320, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x00828220, 0, 0xd2, &g_menu0x00827320, (int)FUN_004ef930, 0);
    Menu_SetCallbacks(&g_menu0x00828220, FUN_004faa00, NULL, (MenuCallback)FUN_004d6290, NULL);
    Menu_ValidateCursor(&g_menu0x00828220, 0);
}

// FUNCTION: CMR2 0x004f9670
void FUN_004f9670(void)
{
    Menu_Init(&g_menu0x00826f60, 0, 0x33, 0, &g_menu0x008286e0, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x98, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x99, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x9a, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x9b, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x9c, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x9d, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x9e, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x9f, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa0, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa1, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa2, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa3, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa4, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa5, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa6, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa7, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa8, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa9, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xaa, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xab, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xac, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xad, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_SetCallbacks(&g_menu0x00826f60, FUN_004faa50, (MenuCallback)FUN_004f39d0, FUN_004d8950, NULL);
    Menu_ValidateCursor(&g_menu0x00826f60, 0);
}

// FUNCTION: CMR2 0x004f9940
void FUN_004f9940(void)
{
    Menu_Init(&g_menu0x00826d80, 0, 0x97, 0, &g_menu0x00826f60, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x00826d80, 0, 0x88, 2, 0, 0, 0, (int)FUN_004fad40, 0x88);
    Menu_SetCallbacks(&g_menu0x00826d80, FUN_004fac70, (MenuCallback)FUN_004f39d0, FUN_004d8ed0, NULL);
    Menu_ValidateCursor(&g_menu0x00826d80, 0);
}

// FUNCTION: CMR2 0x004f99b0
void FUN_004f99b0(void)
{
    Menu_Init(&g_menu0x00828500, 0, 0x33, 0, &g_menu0x00827c80, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x98, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x99, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x9a, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x9b, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x9c, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x9d, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x9e, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x9f, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa0, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa1, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa2, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa3, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa4, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa5, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa6, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa7, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa8, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa9, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xaa, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xab, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xac, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xad, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_SetCallbacks(&g_menu0x00828500, FUN_004faa50, (MenuCallback)FUN_004f39d0, FUN_004d8950, NULL);
    Menu_ValidateCursor(&g_menu0x00828500, 0);
}

// FUNCTION: CMR2 0x004f9c80
void FUN_004f9c80(void)
{
    Menu_Init(&g_menu0x00827140, 0, 0x97, 0, &g_menu0x00828500, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x00827140, 0, 0x88, 2, 0, 0, 0, (int)FUN_004fadd0, 0x88);
    Menu_SetCallbacks(&g_menu0x00827140, FUN_004fac70, (MenuCallback)FUN_004f39d0, FUN_004d8ed0, NULL);
    Menu_ValidateCursor(&g_menu0x00827140, 0);
}

// FUNCTION: CMR2 0x004f9cf0
void FUN_004f9cf0(void)
{
    Menu_Init(&g_menu0x00827e60, 0, 0x33, 0, &g_menu0x008278c0, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x98, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x99, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x9a, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x9b, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x9c, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x9d, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x9e, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x9f, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa0, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa1, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa2, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa3, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa4, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa5, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa6, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa7, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa8, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa9, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xaa, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xab, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xac, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xad, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_SetCallbacks(&g_menu0x00827e60, FUN_004faa50, (MenuCallback)FUN_004f39d0, FUN_004d8950, NULL);
    Menu_ValidateCursor(&g_menu0x00827e60, 0);
}

// FUNCTION: CMR2 0x004f9fc0
void FUN_004f9fc0(void)
{
    Menu_Init(&g_menu0x00827500, 0, 0x97, 0, &g_menu0x00827e60, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x00827500, 0, 0x88, 2, 0, 0, 0, (int)FUN_004faea0, 0x88);
    Menu_SetCallbacks(&g_menu0x00827500, FUN_004fac70, (MenuCallback)FUN_004f39d0, FUN_004d8ed0, NULL);
    Menu_ValidateCursor(&g_menu0x00827500, 0);
}

// FUNCTION: CMR2 0x004fa030
void FUN_004fa030(void)
{
    Menu_Init(&g_menu0x00826ba0, 0, 0x26, 0, &g_menu0x00826d80, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x00826ba0, 0, 0xe9, (int)FUN_004fafe0, 1);
    Menu_AddItemType4(&g_menu0x00826ba0, 0, 0xe8, (int)FUN_004fafe0, 0);
    Menu_SetCallbacks(&g_menu0x00826ba0, FUN_004faef0, (MenuCallback)FUN_004fafd0, FUN_004db850, NULL);
    Menu_ValidateCursor(&g_menu0x00826ba0, 0);
}

// FUNCTION: CMR2 0x004fa0b0
void FUN_004fa0b0(void)
{
    Menu_Init(&g_menu0x00827320, 0, 0x25, 0, &g_menu0x00828220, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0xfa, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0xfb, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0xfc, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0xfd, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0xfe, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0xff, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0x100, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0x101, (int)FUN_004fb370, -1);
    Menu_SetCallbacks(&g_menu0x00827320, FUN_004fb010, (MenuCallback)FUN_004fb360, FUN_004dc710, NULL);
    Menu_ValidateCursor(&g_menu0x00827320, 0);
}

// FUNCTION: CMR2 0x004fa1c0
void FUN_004fa1c0(void)
{
    Menu_Init(&g_menu0x00826420, 0, 0x25, 0, &g_menu0x00827500, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0xfa, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0xfb, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0xfc, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0xfd, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0xfe, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0xff, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0x100, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0x101, (int)FUN_004fb370, -1);
    Menu_SetCallbacks(&g_menu0x00826420, FUN_004fb010, (MenuCallback)FUN_004fb360, FUN_004dc710, NULL);
    Menu_ValidateCursor(&g_menu0x00826420, 0);
}

// FUNCTION: CMR2 0x004f8290
Menu *FUN_004f8290(BYTE param1)
{
    if (param1 != 0 && CGameInfo::GetGameRegion() != 3 && CGameInfo::GetGameRegion() != 2)
        return &g_menu0x008221d8;
    return &g_menu0x0081d6d8;
}

// FUNCTION: CMR2 0x004f8330
Menu *FUN_004f8330(void)
{
    return &g_menu0x008214b8;
}

// FUNCTION: CMR2 0x004f8990
Menu *FUN_004f8990(void)
{
    return &g_menu0x00820798;
}

// FUNCTION: CMR2 0x004fa330
Menu *FUN_004fa330(void)
{
    return &g_menu0x00826ba0;
}

// FUNCTION: CMR2 0x004f8410
Menu *FUN_004f8410(void)
{
    return &g_menu0x0081d6d8;
}

// FUNCTION: CMR2 0x004f83a0
Menu *FUN_004f83a0(void)
{
    return &g_menu0x008241b8;
}

// FUNCTION: CMR2 0x004f8450
Menu *FUN_004f8450(void)
{
    return &g_menu0x00824c18;
}

// FUNCTION: CMR2 0x004f8470
Menu *FUN_004f8470(void)
{
    return &g_menu0x0081e218;
}

// FUNCTION: CMR2 0x004f8360
Menu *FUN_004f8360(void)
{
    return &g_menu0x0081f4d8;
}

// FUNCTION: CMR2 0x004f82c0
Menu *FUN_004f82c0(void)
{
    return &g_menu0x0081f118;
}

// FUNCTION: CMR2 0x004f82d0
Menu *FUN_004f82d0(void)
{
    return &g_menu0x008221d8;
}

// FUNCTION: CMR2 0x004f82e0
Menu *FUN_004f82e0(void)
{
    return &g_menu0x0081d318;
}

// FUNCTION: CMR2 0x004f82f0
Menu *FUN_004f82f0(void)
{
    return &g_menu0x00820978;
}

// FUNCTION: CMR2 0x004f8300
Menu *FUN_004f8300(void)
{
    return &g_menu0x0081f2f8;
}

// FUNCTION: CMR2 0x004f8310
Menu *FUN_004f8310(void)
{
    return &g_menu0x00824678;
}

// FUNCTION: CMR2 0x004f8320
Menu *FUN_004f8320(void)
{
    return &g_menu0x0081e5d8;
}

// FUNCTION: CMR2 0x004f8340
Menu *FUN_004f8340(void)
{
    return &g_menu0x0081eb78;
}

// FUNCTION: CMR2 0x004f8350
Menu *FUN_004f8350(void)
{
    return &g_menu0x0081e3f8;
}

// FUNCTION: CMR2 0x004f8370
Menu *FUN_004f8370(void)
{
    return &g_menu0x008230d8;
}

// FUNCTION: CMR2 0x004f8380
Menu *FUN_004f8380(void)
{
    return &g_menu0x00824858;
}

// FUNCTION: CMR2 0x004f8390
Menu *FUN_004f8390(void)
{
    return &g_menu0x008251b8;
}

// FUNCTION: CMR2 0x004f83b0
Menu *FUN_004f83b0(void)
{
    return &g_menu0x008203d8;
}

// FUNCTION: CMR2 0x004f83c0
Menu *FUN_004f83c0(void)
{
    return &g_menu0x00824a38;
}

// FUNCTION: CMR2 0x004f83d0
Menu *FUN_004f83d0(void)
{
    return &g_menu0x0081d8b8;
}

// FUNCTION: CMR2 0x004f83e0
Menu *FUN_004f83e0(void)
{
    return &g_menu0x0081bc98;
}

// FUNCTION: CMR2 0x004f83f0
Menu *FUN_004f83f0(void)
{
    return &g_menu0x00821e18;
}

// FUNCTION: CMR2 0x004f8400
Menu *FUN_004f8400(void)
{
    return &g_menu0x0081cf58;
}

// FUNCTION: CMR2 0x004f8420
Menu *FUN_004f8420(void)
{
    return &g_menu0x00823858;
}

// FUNCTION: CMR2 0x004f8430
Menu *FUN_004f8430(void)
{
    return &g_menu0x00822d18;
}

// FUNCTION: CMR2 0x004f8440
Menu *FUN_004f8440(void)
{
    return &g_menu0x00822598;
}

// FUNCTION: CMR2 0x004f8460
Menu *FUN_004f8460(void)
{
    return &g_menu0x00821c38;
}

// FUNCTION: CMR2 0x004f8480
Menu *FUN_004f8480(void)
{
    return &g_menu0x008212d8;
}

// FUNCTION: CMR2 0x004f8490
Menu *FUN_004f8490(void)
{
    return &g_menu0x0081c058;
}

// FUNCTION: CMR2 0x004f84a0
Menu *FUN_004f84a0(void)
{
    return &g_menu0x00823a38;
}

// FUNCTION: CMR2 0x004f84b0
Menu *FUN_004f84b0(void)
{
    return &g_menu0x00824fd8;
}

// FUNCTION: CMR2 0x004f84c0
Menu *FUN_004f84c0(void)
{
    return &g_menu0x0081fc58;
}

// FUNCTION: CMR2 0x004f84d0
Menu *FUN_004f84d0(void)
{
    return &g_menu0x0081b338;
}

// FUNCTION: CMR2 0x004f84e0
Menu *FUN_004f84e0(void)
{
    return &g_menu0x0081e998;
}

// FUNCTION: CMR2 0x004f84f0
Menu *FUN_004f84f0(void)
{
    return &g_menu0x0081d138;
}

// FUNCTION: CMR2 0x004f88d0
Menu *FUN_004f88d0(void)
{
    return &g_menu0x0081b158;
}

// FUNCTION: CMR2 0x004f89a0
Menu *FUN_004f89a0(void)
{
    return &g_menu0x00824df8;
}

// FUNCTION: CMR2 0x004f9360
Menu *FUN_004f9360(void)
{
    return &g_menu0x00826140;
}

// FUNCTION: CMR2 0x004fa2d0
Menu *FUN_004fa2d0(void)
{
    return &g_menu0x008267e0;
}

// FUNCTION: CMR2 0x004fa2e0
Menu *FUN_004fa2e0(void)
{
    return &g_menu0x00826600;
}

// FUNCTION: CMR2 0x004fa2f0
Menu *FUN_004fa2f0(void)
{
    return &g_menu0x00828220;
}

// FUNCTION: CMR2 0x004fa300
Menu *FUN_004fa300(void)
{
    return &g_menu0x00826f60;
}

// FUNCTION: CMR2 0x004fa310
Menu *FUN_004fa310(void)
{
    return &g_menu0x00828500;
}

// FUNCTION: CMR2 0x004fa320
Menu *FUN_004fa320(void)
{
    return &g_menu0x00827e60;
}

// FUNCTION: CMR2 0x004fa340
Menu *FUN_004fa340(void)
{
    return &g_menu0x00827320;
}

// FUNCTION: CMR2 0x004fa350
Menu *FUN_004fa350(void)
{
    return &g_menu0x00826420;
}

// FUNCTION: CMR2 0x004fa360
Menu *FUN_004fa360(void)
{
    return &g_menu0x00828040;
}

// FUNCTION: CMR2 0x004fa4c0
Menu *FUN_004fa4c0(void)
{
    return &g_menu0x00827aa0;
}

// FUNCTION: CMR2 0x004fa4f0
Menu *FUN_004fa4f0(void)
{
    return &g_menu0x00828c80;
}

// FUNCTION: CMR2 0x004fa500
Menu *FUN_004fa500(void)
{
    return &g_menu0x00828aa0;
}

// FUNCTION: CMR2 0x004fa510
Menu *FUN_004fa510(void)
{
    return &g_menu0x008288c0;
}

// FUNCTION: CMR2 0x004fa520
Menu *FUN_004fa520(void)
{
    return &g_menu0x00828e60;
}

// FUNCTION: CMR2 0x004fa530
Menu *FUN_004fa530(void)
{
    return &g_menu0x00829140;
}

void FUN_004fccb0(Menu *pMenu);

// Controls menu: two device entries and "back".
// FUNCTION: CMR2 0x004fa540
void FUN_004fa540(void)
{
    Menu_Init(&g_menu0x00828c80, 0, 0x18, 0, FUN_004f82c0(), NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00828c80, 0, -1, &g_menu0x00829140, (int)FUN_004fbea0, 0);
    Menu_AddItemType2(&g_menu0x00828c80, 0, -1, &g_menu0x00829140, (int)FUN_004fbea0, 0);
    Menu_AddItemType1(&g_menu0x00828c80, 0, 0x67, 0, 0);
    Menu_SetCallbacks(&g_menu0x00828c80, (MenuCallback)FUN_004fba90, NULL, (MenuCallback)FUN_004fccb0, NULL);
    Menu_ValidateCursor(&g_menu0x00828c80, 0);
}

void FUN_004fbec0(Menu *pMenu, int param);
void FUN_004fbf60(Menu *pMenu, char param);
void FUN_004fc070(Menu *pMenu);
void FUN_004fd080(Menu *pMenu);
void FUN_004fbff0(Menu *pMenu, char back);

// Device page of the controls menu: the 10 bindings and "back".
// FUNCTION: CMR2 0x004fa5d0
void FUN_004fa5d0(void)
{
    int i;

    Menu_Init(&g_menu0x008288c0, 0, 0x65, 0, &g_menu0x00828c80, NULL, 1, 0, 1);
    i = 0;
    do {
        Menu_AddItemType4(&g_menu0x008288c0, 0, i + 0x6b, (int)FUN_004fbec0, i);
        i++;
    } while (i < 10);
    Menu_AddItemType2(&g_menu0x008288c0, 0, 0x67, &g_menu0x00828c80, 0, 0);
    Menu_SetCallbacks(&g_menu0x008288c0, (MenuCallback)FUN_004fbf60, (MenuCallback)FUN_004fc070,
                      (MenuCallback)FUN_004fd080, (MenuCallback)FUN_004fbff0);
    Menu_ValidateCursor(&g_menu0x008288c0, 0);
}

void FUN_004fc850(Menu *pMenu, int param);
void FUN_004fc500(Menu *pMenu, int param);
void FUN_004fc620(Menu *pMenu);
void FUN_004fd480(Menu *pMenu);
void FUN_004fc880(Menu *pMenu, int param);
void FUN_004fdb10(Menu *pMenu);
void FUN_004fc8f0(Menu *pMenu, char back);
void FUN_004fc9b0(Menu *pMenu, int param);
void FUN_004fc9f0(Menu *pMenu);
void FUN_004fe240(Menu *pMenu);
void FUN_004fc970(Menu *pMenu, char back);

// Calibration page of the controls menu: one entry per axis and "back".
// FUNCTION: CMR2 0x004fa650
void FUN_004fa650(void)
{
    int i;

    Menu_Init(&g_menu0x00828aa0, 0, 0x75, 0, &g_menu0x008288c0, NULL, 1, 0, 1);
    i = 0;
    do {
        Menu_AddItemType4(&g_menu0x00828aa0, 0, -1, (int)FUN_004fc850, i);
        i++;
    } while (i < 8);
    Menu_AddItemType2(&g_menu0x00828aa0, 0, 0x67, &g_menu0x00828c80, 0, 0);
    Menu_SetCallbacks(&g_menu0x00828aa0, (MenuCallback)FUN_004fc500, (MenuCallback)FUN_004fc620,
                      (MenuCallback)FUN_004fd480, NULL);
    Menu_ValidateCursor(&g_menu0x00828aa0, 0);
}

// Pad page of the controls menu: two sensitivities, vibration and "back".
// FUNCTION: CMR2 0x004fa6d0
void FUN_004fa6d0(void)
{
    Menu_Init(&g_menu0x00828e60, 0, 0x75, 0, &g_menu0x008288c0, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x00828e60, 0, 0x7a, 0xb, 0, 0, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x00828e60, 0, 0x7b, 0xb, 0, 0, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x00828e60, 0, 0x1a8, 2, 0, 0, 0, 0, 2);
    Menu_AddItemType2(&g_menu0x00828e60, 0, 0x67, &g_menu0x00828c80, 0, 3);
    Menu_SetCallbacks(&g_menu0x00828e60, (MenuCallback)FUN_004fc880, NULL, (MenuCallback)FUN_004fdb10,
                      (MenuCallback)FUN_004fc8f0);
    Menu_ValidateCursor(&g_menu0x00828e60, 0);
}

// Device settings page of the controls menu: configuration of the slot
// (one of the connected devices) and its options.
// FUNCTION: CMR2 0x004fa780
void FUN_004fa780(void)
{
    Menu_Init(&g_menu0x00829140, 0, 0x65, 0, &g_menu0x00828c80, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x00829140, 0, -1, (BYTE)CInput::FUN_0049ef90(), 0, 0, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x00829140, 0, 0x68, 2, 0, 0, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x00829140, 0, 0x68, 2, 0, 0, 0, 0, 2);
    Menu_AddItemType3(&g_menu0x00829140, 0, 0x6a, 2, 0, 0, 0, 0, 3);
    Menu_AddItemType3(&g_menu0x00829140, 0, 0x20b, 2, 0, 0, 0, 0, 4);
    Menu_AddItemType3(&g_menu0x00829140, 0, 0x20c, 2, 0, 0, 0, 0, 5);
    Menu_AddItemType2(&g_menu0x00829140, 0, 0x67, &g_menu0x008288c0, 0, 6);
    Menu_SetCallbacks(&g_menu0x00829140, (MenuCallback)FUN_004fc9b0, (MenuCallback)FUN_004fc9f0,
                      (MenuCallback)FUN_004fe240, (MenuCallback)FUN_004fc970);
    Menu_ValidateCursor(&g_menu0x00829140, 0);
}

// Builds every page of the controls menu.
// FUNCTION: CMR2 0x004fa4d0
void FUN_004fa4d0(void)
{
    FUN_004fa540();
    FUN_004fa5d0();
    FUN_004fa650();
    CInput::FUN_0040c050();
    FUN_004fa780();
    FUN_004fa6d0();
}

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "Car.h"
#include "Font.h"
#include "Frontend.h"
#include "GameInfo.h"
#include "Graphics.h"
#include "InstallInfo.h"
#include "RallyData.h"
#include "Sprite.h"
#include "StageTiming.h"
#include "Texture.h"

// In-car dashboard of the HUD: the rev counter (an analogue dial with a
// needle, or a digital bar), the gear and the speed of each of the (up to
// two) players. The shown speed and revs are interpolated between the
// physics steps.

struct GenericFile;
extern char g_strPathConcat[];
int Car_GetWheelSpeed(Car *pCar, BYTE wheel, int unit);
BYTE *RallyData_FUN_00408a00(BYTE index);
BYTE FUN_0041b370(void);
extern float g_oneOverRandMax;
void FUN_00445db0(void);

struct DashGearNames {
    char c[12];
};

// GLOBAL: CMR2 0x0053ce10
int g_dashGearMarkerY[2];       // gear marker position drawn last frame
// GLOBAL: CMR2 0x0053ce18
int g_dashGearFlash[2];         // counts down after a gear change
// GLOBAL: CMR2 0x0053ce38
int g_dashSpeed[2];             // interpolated speed
// GLOBAL: CMR2 0x0053ce40
int g_dashAspect;
// GLOBAL: CMR2 0x0053ce44
int g_dashSpeedNext[2];
// GLOBAL: CMR2 0x0053ce4c
int g_dashSimple;               // plain rectangles instead of the textured bar
// GLOBAL: CMR2 0x0053ce50
unsigned short *g_dashRevTicks;
// GLOBAL: CMR2 0x0053ce54
int g_dashRev[2];               // interpolated revs (1.0 = red line)
// GLOBAL: CMR2 0x0053ce5c
int g_dashLastResX;
// GLOBAL: CMR2 0x0053ce60
int g_dashRevPrev[2];
// GLOBAL: CMR2 0x0053ce68
int g_dashSpeedPrev[2];
// GLOBAL: CMR2 0x0053ce70
int g_dashGear[2];              // index into g_dashGearNames
// GLOBAL: CMR2 0x0053ce78
int g_dashDigital[8];           // per car: digital bar rather than the dial
// GLOBAL: CMR2 0x0053ce98
int g_dashGearMarker[2];        // gear marker position, eased towards the gear
// GLOBAL: CMR2 0x0053cfa0
int g_dashRevNext[2];
// GLOBAL: CMR2 0x0053cfa8
int g_dashIdle[2];              // steps spent standing still
// GLOBAL: CMR2 0x0053cfb0
int g_dashLastGear[2];
// GLOBAL: CMR2 0x0053cfd0
Texture *g_dashMphTexture;
// GLOBAL: CMR2 0x0053cfd4
Texture *g_dashKphTexture;
// GLOBAL: CMR2 0x0053cfd8
Texture *g_dashDialTexture;
// GLOBAL: CMR2 0x0053cfdc
Texture *g_dashBarTexture;
// GLOBAL: CMR2 0x0053cfe0
SpriteRect g_dashBarOff;        // unlit bar in g_dashBarTexture
// GLOBAL: CMR2 0x0053cfe8
SpriteRect g_dashBarOn;         // lit bar

// GLOBAL: CMR2 0x00519d08
SpriteRect g_dashDialSrc = {0, 0, 0xbe, 0xbe};
// GLOBAL: CMR2 0x00519d10
SpriteRect g_dashUnk0x00519d10 = {0, 0, 0xbe, 0xbe};
// GLOBAL: CMR2 0x00519d18
BYTE g_dashFlashAlpha[4] = {0xff, 0, 0, 0};
// GLOBAL: CMR2 0x00519d1c
BYTE g_dashWhite[4] = {0xff, 0xff, 0xff, 0xff};
// GLOBAL: CMR2 0x00519d20
BYTE g_dashShadow[4] = {0, 0, 0, 0xff};
// GLOBAL: CMR2 0x00519d24
BYTE g_dashBarColour[4] = {0, 0xf8, 0xf8, 0xff};
// GLOBAL: CMR2 0x00519d28
BYTE g_dashBarRedColour[4] = {0xf8, 0x34, 0, 0xff};
// GLOBAL: CMR2 0x00519d2c
BYTE g_dashNeedleColour[4] = {0xf6, 0xf6, 0xf6, 0xff};
// GLOBAL: CMR2 0x00519d30
BYTE g_dashUnk0x00519d30[4] = {0xf6, 0xf6, 0xf6, 0x78};
// GLOBAL: CMR2 0x00519d34
BYTE g_dashGearColour[4] = {0x65, 0x92, 0xff, 0xc8};
// GLOBAL: CMR2 0x00519d38
BYTE g_dashReverseColour[4] = {0xff, 0, 0, 0xc8};
// GLOBAL: CMR2 0x00519d3c
BYTE g_dashNeutralColour[4] = {0, 0xff, 0, 0xc8};
// GLOBAL: CMR2 0x00519d40
BYTE g_dashDialColour[4] = {0xff, 0xff, 0xff, 0xff};
// Per car: the digital bar instead of the dial.
// GLOBAL: CMR2 0x00519d44
int g_dashCarDigital[14] = {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0};
// Lit width of the bar at each 1000 rpm step (640 and 1024 wide screens).
// GLOBAL: CMR2 0x00519d7c
unsigned short g_dashRevTicks640[28] = {5,  8,  13, 16, 20, 24, 30, 36,  42,  48,  54,  60,  66,  72,
                               78, 84, 91, 98, 105, 112, 119, 127, 135, 143, 151, 159, 167, 176};
// GLOBAL: CMR2 0x00519db4
unsigned short g_dashRevTicks1024[28] = {7,   14,  21,  28,  36,  44,  52,  60,  69,  77,  87,  96,  105, 114,
                                124, 135, 145, 157, 168, 180, 191, 203, 215, 228, 241, 254, 268, 282};
// GLOBAL: CMR2 0x00519dec
int g_dashLastResY = -1;
// GLOBAL: CMR2 0x00519df4
char g_strDashDialTga[] = "\\NEWIMAGE\\INTERIOR\\REV_640.TGA";
// GLOBAL: CMR2 0x00519e14
char g_strDashBarTga[] = "\\NEWIMAGE\\OSD\\DIGITAL\\REVDIGITAL.TGA";
// GLOBAL: CMR2 0x00519e3c
char g_strDashKphTga[] = "\\NEWIMAGE\\INTERIOR\\KPH.TGA";
// GLOBAL: CMR2 0x00519e58
char g_strDashMphTga[] = "\\NEWIMAGE\\INTERIOR\\MPH.TGA";
// GLOBAL: CMR2 0x00519e74
char g_strDashSpeedFormat[] = "%s %03d";
// GLOBAL: CMR2 0x00519e7c
char g_strDashKmh[] = "KMH";
// GLOBAL: CMR2 0x00519e80
char g_strDashMph[] = "MPH";
// GLOBAL: CMR2 0x00519e84
char g_strDashChar[] = "%c";
// GLOBAL: CMR2 0x00519e88
DashGearNames g_dashGearNames = {"RN123456"};
// GLOBAL: CMR2 0x00519e94
char g_strDashSpeedDigits[] = "%03d";

// Screen coordinates as a fraction of the resolution.
#define DASH_X(f) (short)(FixMul(g_pGraphics->resX << 16, (f)) >> 16)
#define DASH_Y(f) (short)(FixMul(g_pGraphics->resY << 16, (f)) >> 16)
#define DASH_HIRES()                                                                                   \
    (CGameInfo::GetScreenWidth() >= 0x400 && CFrontend::FUN_004b7560(0x400) && CFrontend::FUN_004b7590(0x400))
#define DASH_RAND() (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536)

// FUNCTION: CMR2 0x00445a60
void Dash_LoadTextures(void)
{
    char path[260];
    bool loaded;

    sprintf(path, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strDashMphTga);
    g_dashMphTexture = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile1(), path, &loaded, NULL, 0, 0);
    sprintf(path, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strDashKphTga);
    g_dashKphTexture = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile1(), path, &loaded, NULL, 0, 0);
    sprintf(path, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strDashBarTga);
    g_dashBarTexture = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile1(), path, &loaded, NULL, 0, 0);
    if (DASH_HIRES()) {
        g_dashBarOff.w = 0x125;
        g_dashBarOff.h = 0xc5;
        g_dashBarOff.x = 0;
        g_dashBarOff.y = 0;
        g_dashBarOn.w = 0x125;
        g_dashBarOn.h = 0xc5;
        g_dashBarOn.x = 0;
        g_dashBarOn.y = 0xc4;
    } else {
        g_dashBarOff.w = 0xb5;
        g_dashBarOff.h = 0x7b;
        g_dashBarOff.x = 0;
        g_dashBarOff.y = 0;
        g_dashBarOn.w = 0xb5;
        g_dashBarOn.h = 0x7b;
        g_dashBarOn.x = 0;
        g_dashBarOn.y = 0x7a;
    }
    sprintf(path, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strDashDialTga);
    g_dashDialTexture = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile1(), path, &loaded, NULL, 0, 0);
    if (DASH_HIRES()) {
        g_dashDialSrc.x = 0;
        g_dashDialSrc.y = 0;
        g_dashDialSrc.w = 0xf4;
        g_dashDialSrc.h = 0xf4;
        return;
    }
    g_dashDialSrc.x = 0;
    g_dashDialSrc.y = 0;
    g_dashDialSrc.w = 0x96;
    g_dashDialSrc.h = 0x96;
}

// Picks the dial or the digital bar for every car.
// FUNCTION: CMR2 0x00445d30
void Dash_InitStyle(void)
{
    int i;
    int *p;

    i = 0;
    if (CGameInfo::FUN_00405d70() > 0) {
        p = g_dashDigital;
        do {
            if (RallyData_FUN_00411880() != 0) {
                *p = 1;
                g_dashSimple = 1;
            } else {
                if (CGameInfo::FUN_00405ef0() == 0)
                    *p = g_dashCarDigital[(int)CFrontend::FUN_0040ee90(RallyData_FUN_004086b0(i))];
                else
                    *p = CGameInfo::FUN_00405ef0() == 1;
                g_dashSimple = 0;
            }
            i++;
            p++;
        } while (i < CGameInfo::FUN_00405d70());
    }
}

// FUNCTION: CMR2 0x00445c80
void Dash_Reset(void)
{
    int i;

    for (i = 0; i < 2; i++) {
        g_dashGearMarker[i] = -1;
        g_dashSpeedNext[i] = 0;
        g_dashRevNext[i] = 0;
        g_dashGear[i] = 0;
        g_dashIdle[i] = 0;
        g_dashRevPrev[i] = 0;
        g_dashSpeedPrev[i] = 0;
        g_dashGearMarkerY[i] = 0;
    }
    Dash_InitStyle();
    if (DASH_HIRES()) {
        g_dashRevTicks = g_dashRevTicks1024;
        return;
    }
    g_dashRevTicks = g_dashRevTicks640;
}

// Shown speed and revs between the last two physics steps (t = 0..1).
// FUNCTION: CMR2 0x00445df0
void Dash_Interpolate(int t)
{
    int inv;
    unsigned int i;

    inv = 0x10000 - t;
    i = 0;
    if ((BYTE)RallyDataState() > 0) {
        do {
            g_dashSpeed[i] = FixMul(g_dashSpeedNext[i], t) + FixMul(g_dashSpeedPrev[i], inv);
            g_dashRev[i] = FixMul(g_dashRevNext[i], t) + FixMul(g_dashRevPrev[i], inv);
            i++;
        } while (i < (RallyDataState() & 0xff));
    }
}

// Physics step of the dashboard: smooths the speed and revs of the car
// (with an idle flicker while standing), and eases the gear marker.
// FUNCTION: CMR2 0x00445ea0
void Dash_Update(int player)
{
    int y;
    int d;
    int lim;

    if (player >= 2)
        return;
    if (g_dashGearFlash[player] > 0)
        g_dashGearFlash[player] = g_dashGearFlash[player] - 1;
    g_dashSpeedPrev[player] = g_dashSpeedNext[player];
    g_dashRevPrev[player] = g_dashRevNext[player];
    g_dashGear[player] = Car_Get(player)->field_0xb1e;
    if ((BYTE)CGameInfo::FUN_00405eb0() != 0)
        g_dashSpeedNext[player] = FixMul(0x9999, Car_GetWheelSpeed(Car_Get(player), 0, 0)) +
                                  FixMul(0x6666, g_dashSpeedNext[player]);
    else
        g_dashSpeedNext[player] = FixMul(0x9999, Car_GetWheelSpeed(Car_Get(player), 0, 1)) +
                                  FixMul(0x6666, g_dashSpeedNext[player]);
    g_dashRevNext[player] =
        FixMul(0x9999, FixDiv(Car_Get(player)->field_0x7ac, Car_Get(player)->field_0x794)) +
        FixMul(0x6666, g_dashRevNext[player]);
    if (Car_Get(player)->field_0xb48 != 1) {
        if (g_dashSpeedNext[player] < 0x10000) {
            if (g_dashIdle[player] > 5) {
                if (g_dashRevNext[player] < 0x1eb8)
                    g_dashRevNext[player] = FixMul(DASH_RAND(), 0x7ae) + 0x170a;
            } else {
                g_dashIdle[player]++;
            }
        } else {
            g_dashIdle[player] = 0;
        }
    } else {
        if (g_dashSpeedNext[player] < 0x10000) {
            if (g_dashIdle[player] > 5) {
                if (g_dashRevNext[player] < 0x1eb8)
                    g_dashRevNext[player] = FixMul(DASH_RAND(), 0x7ae) + 0x170a;
            } else {
                g_dashIdle[player]++;
            }
        } else {
            g_dashIdle[player] = 0;
        }
    }
    if (g_dashRevNext[player] < 0x147a)
        g_dashRevNext[player] = FixMul(DASH_RAND(), 0x28f) + 0x11eb;
    g_dashSpeedNext[player] = FIX_ABS(g_dashSpeedNext[player]);
    if (g_dashGear[player] == 7)
        g_dashGear[player] = 0;
    else
        g_dashGear[player] = g_dashGear[player] + 1;
    y = FixMul(g_pGraphics->resY << 16, g_dashGear[player] * -0xae1 + 0xbe66) >> 16;
    if (g_dashGearMarker[player] == -1)
        g_dashGearMarker[player] = y;
    d = y - g_dashGearMarker[player];
    g_dashGearMarkerY[player] = g_dashGearMarker[player];
    lim = FixMul(g_pGraphics->resY << 16, 0x2d7) >> 16;
    if (d > lim)
        d = lim;
    if (d < -lim)
        d = -lim;
    g_dashGearMarker[player] += d;
    if (g_dashGear[player] != g_dashLastGear[player]) {
        if (g_dashGearFlash[player] == 0)
            g_dashGearFlash[player] = 6;
        g_dashLastGear[player] = g_dashGear[player];
    }
}

// Digital rev counter: the lit part of the bar texture (or plain
// rectangles), the gear letter and the speed.
// match 26%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00446270
void Dash_DrawBar(int player, int layer)
{
    DashGearNames gears;
    char text[16];
    short rect[4];
    SpriteRect src;
    SpriteRect dst;
    BYTE colour[4];
    unsigned int lit;
    short dx;
    short dy;
    int scale;
    int len;
    int split;
    int rest;
    int pos;
    int index;
    int frac;
    int extra;

    gears = g_dashGearNames;
    lit = 0;
    dx = 0;
    dy = 0;
    if (g_dashSimple == 0) {
        pos = FixMul(g_dashRev[player], 0x250000) - 0xa0000;
        index = pos >> 16;
        if (index >= 0) {
            frac = pos - (index << 16);
            lit = g_dashRevTicks[index];
            scale = (int)(__int64)((double)(int)(g_dashRevTicks[index + 1] - lit) * CGraphics::m_65536);
            lit = lit + (FixMul(frac, scale) >> 16);
            if ((int)lit >= (int)g_dashRevTicks[27])
                lit = g_dashRevTicks[27];
            if (DASH_HIRES())
                extra = 5;
            else
                extra = 3;
            if ((int)(extra + lit) >= g_dashBarOn.w)
                extra = g_dashBarOn.w - lit - 1;
            src = g_dashBarOn;
            src.w = (short)lit;
            dst = src;
            dst.x = DASH_X(0xfd70) - g_dashBarOn.w;
            dst.y = DASH_Y(0xfd70) - g_dashBarOn.h;
            Sprite_Queue(&src, &dst, g_dashBarTexture, 3, 0, NULL, NULL, g_dashWhite, 8);
            if (extra != 0) {
                *(DWORD *)colour = *(DWORD *)g_dashWhite;
                dst.x = dst.x + dst.w;
                src.x = src.x + src.w;
                colour[3] = 0x5a;
                dst.w = (short)extra;
                src.w = (short)extra;
                Sprite_Queue(&src, &dst, g_dashBarTexture, 3, 0, NULL, NULL, colour, 8);
            }
        }
        src = g_dashBarOn;
        src.w = (short)lit;
        dst = src;
        dst.x = DASH_X(0xfd70) - g_dashBarOn.w;
        dst.y = DASH_Y(0xfd70) - g_dashBarOn.h;
        Sprite_Queue(&src, &dst, g_dashBarTexture, 3, 0, NULL, NULL, g_dashWhite, 8);
        dst.x = DASH_X(0xfd70) - g_dashBarOn.w;
        dst.y = DASH_Y(0xfd70) - g_dashBarOn.h;
        dst.w = g_dashBarOn.w;
        dst.h = g_dashBarOn.h;
        src = g_dashBarOff;
        dst.x = DASH_X(0xfd70) - dst.w;
        Sprite_Queue(&src, &dst, g_dashBarTexture, 3, 0, NULL, NULL, g_dashWhite, 8);
    } else {
        scale = (int)(__int64)((double)((g_pGraphics->resX * 0x4b) / 640) * CGraphics::m_65536);
        len = FixMul(g_dashRevNext[player], scale) >> 16;
        if (CGameInfo::FUN_00405dc0() == 0) {
            if (player == 0)
                dx = -(short)(g_pGraphics->resX / 2);
        } else if (player == 0) {
            dy = -(short)(g_pGraphics->resY / 2);
        }
        split = (g_pGraphics->resY * 0x41) / 480;
        if (split < len) {
            rest = len - split;
        } else {
            rest = 0;
            split = len;
        }
        rect[0] = DASH_X(0xfd70) - (short)((g_pGraphics->resX * 0x82) / 640) + dx;
        rect[1] = DASH_Y(0xfd70) - (short)((g_pGraphics->resY * 0x28) / 480) + dy;
        rect[2] = (short)split;
        rect[3] = (short)((g_pGraphics->resY * 0x14) / 480);
        Sprite_FillRect((int)g_pGraphics + 0x150, rect, g_dashBarColour, 3);
        if (rest != 0) {
            rect[0] = rect[0] + rect[2];
            rect[2] = (short)rest;
            Sprite_FillRect((int)g_pGraphics + 0x150, rect, g_dashBarRedColour, 3);
        }
        rect[0] = DASH_X(0xfd70) - (short)((g_pGraphics->resX * 0x82) / 640) - 1 + dx;
        rect[2] = 1;
        rect[1] = DASH_Y(0xfd70) - (short)((g_pGraphics->resY * 0x28) / 480) - 1 + dy;
        rect[3] = (short)((g_pGraphics->resY * 0x14) / 480) + 2;
        Sprite_FillRect((int)g_pGraphics + 0x150, rect, g_dashWhite, 3);
        rect[3] = 1;
        rect[2] = (short)((g_pGraphics->resX * 0x4b) / 640) + 2;
        Sprite_FillRect((int)g_pGraphics + 0x150, rect, g_dashWhite, 3);
        rect[2] = 1;
        rect[0] = DASH_X(0xfd70) + 1 + dx +
                  ((short)((g_pGraphics->resX * 0x4b) / 640) - (short)((g_pGraphics->resX * 0x82) / 640));
        rect[3] = (short)((g_pGraphics->resY * 0x14) / 480) + 2;
        Sprite_FillRect((int)g_pGraphics + 0x150, rect, g_dashWhite, 3);
        rect[0] = DASH_X(0xfd70) - (short)((g_pGraphics->resX * 0x82) / 640) - 1 + dx;
        rect[1] = DASH_Y(0xfd70) + dy +
                  ((short)((g_pGraphics->resY * 0x14) / 480) - (short)((g_pGraphics->resY * 0x28) / 480));
        rect[3] = 1;
        rect[2] = (short)((g_pGraphics->resX * 0x4b) / 640) + 2;
        Sprite_FillRect((int)g_pGraphics + 0x150, rect, g_dashWhite, 3);
    }

    // Gear and speed.
    rect[0] = DASH_X(0xfd70) + 1 + dx;
    rect[1] = DASH_Y(0xfd70) + 3 + dy;
    sprintf(text, g_strDashChar, gears.c[g_dashGear[player]]);
    rect[1] = rect[1] + (short)((g_pGraphics->resY * -2) / 480);
    Font_DrawText(6, text, rect[0], rect[1], (int *)g_dashWhite, 0x24);
    rect[0] = rect[0] + (short)((g_pGraphics->resX * -0x67) / 640);
    rect[1] = rect[1] + (short)((g_pGraphics->resY * 3) / 480);
    sprintf(text, CGameInfo::FUN_00405eb0() == 0 ? g_strDashKmh : g_strDashMph);
    sprintf(CFrontend::m_stringDest, g_strDashSpeedFormat, text, g_dashSpeedNext[player] >> 16);
    rect[1] = rect[1] + (short)((g_pGraphics->resY * -4) / 480);
    Font_DrawText(5, CFrontend::m_stringDest, rect[0] + 1, rect[1] + 1, (int *)g_dashShadow, 0x21);
    Font_DrawText(5, CFrontend::m_stringDest, rect[0], rect[1], (int *)g_dashWhite, 0x21);
}

// Draws the dial face of the rev counter centred on pCentre (fractions of
// the screen).
// FUNCTION: CMR2 0x00447100
void Dash_DrawDialFace(int *pCentre, int unused, Texture *pTexture)
{
    SpriteRect dst;
    int x;
    int y;
    int w;
    int h;

    x = FixMul(g_pGraphics->resX << 16, pCentre[0]) >> 16;
    y = FixMul(g_pGraphics->resY << 16, pCentre[1]) >> 16;
    w = g_dashDialSrc.w;
    h = g_dashDialSrc.h;
    dst.x = x - w;
    dst.y = y - h / 2;
    dst.w = w;
    dst.h = h;
    Sprite_Queue(&g_dashDialSrc, &dst, pTexture, 2, 0, NULL, NULL, g_dashDialColour, 8);
}

// Draws the needle of the dial: a thin quad from the tail to the tip, with
// its point, rotated by angle about pCentre.
// match 58%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004471b0
void Dash_DrawNeedle(int *pCentre, int width, int tipWidth, int tail, int mid, int tip, unsigned int angle,
                     BYTE *pColour, int layer)
{
    BYTE edge[4];
    BYTE point[4];
    BYTE body[4];
    int a[2];
    int b[2];
    int c[2];
    int d[2];
    int e[2];
    int sx;
    int sy;
    int cx;
    int cy;
    int w1x;
    int w1y;
    int w2x;
    int w2y;
    int tailX;
    int tailY;
    int midX;
    int midY;

    point[0] = 0xf6;
    point[1] = 0xf6;
    point[2] = 0xf6;
    point[3] = pColour[3];
    body[0] = 0xfa;
    body[1] = 0xfa;
    body[2] = 0xfa;
    body[3] = pColour[3];
    edge[0] = 0xf6;
    edge[1] = 0xf6;
    edge[2] = 0xf6;
    edge[3] = pColour[3];
    if (DASH_HIRES()) {
        sx = 0x4000000;
        sy = 0x3000000;
    } else {
        sx = 0x2800000;
        sy = 0x1e00000;
    }
    cx = FixMul(pCentre[0], g_pGraphics->resX << 16) + (g_dashDialSrc.w / 2) * -0x10000;
    cy = FixMul(pCentre[1], g_pGraphics->resY << 16);
    w1x = FixMul(FixMul(width, g_sinTable[angle & 0xfff]), sx);
    w1y = FixMul(FixMul(FixMul(width, g_sinTable[(angle + 0x400) & 0xfff]), sy), g_dashAspect);
    w2x = FixMul(FixMul(tipWidth, g_sinTable[angle & 0xfff]), sx);
    w2y = FixMul(FixMul(FixMul(tipWidth, g_sinTable[(angle + 0x400) & 0xfff]), sy), g_dashAspect);
    e[0] = FixMul(-FixMul(tip, g_sinTable[(angle + 0x400) & 0xfff]), sx);
    e[1] = FixMul(FixMul(FixMul(tip, g_sinTable[angle & 0xfff]), sy), g_dashAspect);
    midX = FixMul(-FixMul(mid, g_sinTable[(angle + 0x400) & 0xfff]), sx);
    midY = FixMul(FixMul(FixMul(mid, g_sinTable[angle & 0xfff]), sy), g_dashAspect);
    tailX = FixMul(-FixMul(tail, g_sinTable[(angle + 0x400) & 0xfff]), sx);
    tailY = FixMul(FixMul(FixMul(tail, g_sinTable[angle & 0xfff]), sy), g_dashAspect);
    a[0] = tailX + w1x + cx;
    b[0] = tailX - w1x + cx;
    b[1] = tailY - w1y + cy;
    a[1] = w1y + tailY + cy;
    c[0] = midX + w2x + cx;
    d[0] = midX - w2x + cx;
    e[0] = e[0] + cx;
    d[1] = midY - w2y + cy;
    e[1] = e[1] + cy;
    c[1] = w2y + midY + cy;
    Tri2D_Queue(a, c, e, point, 2);
    Tri2D_Queue(a, e, b, body, 2);
    Tri2D_Queue(d, b, e, edge, 2);
}

// Analogue rev counter: the dial with its needle, the gear marker beside
// the gear letters, the speed and the MPH/KPH plate.
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00446bf0
void Dash_DrawDial(int player, int layer)
{
    DashGearNames gears;
    char speed[12];
    char unit[12];
    char letter[4];
    int dialCentre[2];
    int needleCentre[2];
    short rect[4];
    SpriteRect src;
    SpriteRect dst;
    BYTE colour[4];
    BYTE shadow[4];
    Texture *pTexture;
    DWORD c;
    int angle;
    int flash;
    int i;
    int k;

    gears = g_dashGearNames;
    sprintf(speed, g_strDashSpeedDigits, g_dashSpeedNext[player] >> 16);
    sprintf(unit, g_strDashMph);
    g_dashAspect = 0x15553;
    if (g_pGraphics->resX != g_dashLastResX || g_pGraphics->resY != g_dashLastResY) {
        FUN_00445db0();
        g_dashLastResX = g_pGraphics->resX;
        g_dashLastResY = g_pGraphics->resY;
    }
    k = FixMul(FixDiv(g_dashSpeedPrev[player] + g_dashSpeedNext[player], 0x640000), 0x9110000);
    dialCentre[0] = 0xf333;
    needleCentre[0] = 0xf333;
    dialCentre[1] = 0xd113;
    needleCentre[1] = 0xd113;
    angle = (short)(0x732 - (short)(FixMul(g_dashRev[player] + 0x1eb8, 0xb330000) >> 16));
    Dash_DrawNeedle(needleCentre, 0x100, 0xcc, -0x8cc, 0x1799, 0x18cc, angle, g_dashNeedleColour, layer);
    Dash_DrawDialFace(dialCentre, 0x3c01, g_dashDialTexture);
    Dash_DrawNeedle(needleCentre, 0x100, 0x100, -0x8cc, 0x1799, 0x18cc, angle, g_dashNeedleColour, layer);

    // Gear marker, flashing white after a gear change.
    rect[3] = DASH_Y(0x886);
    rect[2] = DASH_X(0x666);
    rect[0] = DASH_X(0xf45a) - 5;
    c = *(DWORD *)g_dashReverseColour;
    if (g_dashGear[player] != 0 && (c = *(DWORD *)g_dashNeutralColour, g_dashGear[player] != 1))
        c = *(DWORD *)g_dashGearColour;
    flash = g_dashGearFlash[player];
    *(DWORD *)colour = c;
    if (flash > 0) {
        if (flash < 4) {
            colour[0] += (BYTE)(((0xff - (c & 0xff)) * flash) / 3);
            colour[1] += (BYTE)(((0xff - (c >> 8 & 0xff)) * flash) / 3);
            colour[2] += (BYTE)(((0xff - (c >> 16 & 0xff)) * flash) / 3);
            colour[3] += (BYTE)((((*(DWORD *)g_dashFlashAlpha & 0xff) - colour[3]) * flash) / 3);
        } else {
            colour[0] = 0xff;
            colour[1] = 0xff;
            colour[2] = 0xff;
            colour[3] = g_dashFlashAlpha[0];
        }
    }
    rect[1] = (short)g_dashGearMarkerY[player];
    shadow[0] = colour[0];
    shadow[1] = colour[1];
    shadow[2] = colour[2];
    shadow[3] = colour[3] >> 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, shadow, 3);
    rect[1] = (short)g_dashGearMarker[player];
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, colour, 3);
    Font_DrawText(4, speed, FixMul(g_pGraphics->resX << 16, 0xf467) >> 16, FixMul(g_pGraphics->resY << 16, 0xd90f) >> 16,
                  (int *)g_dashWhite, 0x24);

    // Speed unit plate.
    src.x = 0;
    src.y = 0;
    pTexture = CGameInfo::FUN_00405eb0() == 0 ? g_dashKphTexture : g_dashMphTexture;
    src.w = pTexture->width;
    src.h = pTexture->height;
    dst.x = DASH_X(0xf78d);
    dst.y = (short)((unsigned int)(g_pGraphics->resY * 0xd113) >> 16);
    dst.w = src.w;
    dst.h = src.h;
    if (DASH_HIRES()) {
        dst.x = dst.x - src.w / 2;
        dst.y = dst.y - 0x12;
    } else {
        dst.x = dst.x - src.w / 2;
        dst.y = dst.y - 0xc;
    }
    Sprite_Queue(&src, &dst, CGameInfo::FUN_00405eb0() == 0 ? g_dashKphTexture : g_dashMphTexture, 2, 0, NULL, NULL,
                 g_dashWhite, 8);

    // Gear letters down the side of the dial.
    i = 0;
    k = 0xc553;
    do {
        sprintf(letter, g_strDashChar, gears.c[i]);
        Font_DrawText(0, letter, (FixMul(g_pGraphics->resX << 16, 0xf78d) >> 16) - 5,
                      FixMul(g_pGraphics->resY << 16, k) >> 16, (int *)g_dashWhite, 0x12);
        k -= 0xae1;
        i++;
    } while (k > 0x6e4b);
}

// Draws the dashboard of a player whose car shows one.
// FUNCTION: CMR2 0x00446210
void Dash_Draw(int player, int layer)
{
    if (player < 2 && (*RallyData_FUN_00408a00(FUN_0041b370() + (char)player) & 0x10) != 0) {
        if (g_dashDigital[FUN_0041b370() + player] != 0) {
            Dash_DrawBar(player, layer);
            return;
        }
        Dash_DrawDial(player, layer);
    }
}

// GLOBAL: CMR2 0x0053d090
short g_unk0x0053d090[4];
// GLOBAL: CMR2 0x0053d098
int g_unk0x0053d098[4];

// FUNCTION: CMR2 0x00447e00
short FUN_00447e00(BYTE index)
{
    return g_unk0x0053d090[index];
}

extern FixVector g_unk0x0053d048[4];

// Sets the player's gauge target and its speed-scaled copy (less at speed).
// FUNCTION: CMR2 0x00447e20
void FUN_00447e20(BYTE index, short value)
{

    g_unk0x0053d090[index] = value;
    g_unk0x0053d090[index + 2] =
        (short)FixMul(0x10000 - FixMul(FixDiv(g_unk0x0053d048[index].z - 0x50000, 0x50000), 0xcccc), value);
}

// FUNCTION: CMR2 0x00447ea0
int FUN_00447ea0(BYTE index)
{
    return g_unk0x0053d098[index];
}

// FUNCTION: CMR2 0x00447ec0
void FUN_00447ec0(BYTE index, int value)
{
    g_unk0x0053d098[index] = value;
}

// GLOBAL: CMR2 0x0053d048
FixVector g_unk0x0053d048[4];

// FUNCTION: CMR2 0x00447cf0
void FUN_00447cf0(FixVector *pOut, BYTE index)
{
    *pOut = g_unk0x0053d048[index];
}

// Camera mode of each player (4 = free camera using g_unk0x0053d000).
// GLOBAL: CMR2 0x0053cff8
BYTE g_unk0x0053cff8[8];
// GLOBAL: CMR2 0x0053d000
FixVector g_unk0x0053d000[6];
// Default view offsets of the fixed camera modes.
// GLOBAL: CMR2 0x00519ea0
FixVector g_unk0x00519ea0[3] = { { 0, 0x13333, -0x50000 }, { 0, 0x13333, -0x50000 }, { 0, 0x13333, -0x50000 } };

int FUN_0041f3a0(void);
int RallyData_FUN_00411880(void);

// View offset of a player's camera (lowered in the split-screen cockpit view).
// FUNCTION: CMR2 0x00447ee0
void FUN_00447ee0(FixVector *pOut, BYTE *pSel)
{
    BYTE mode;
    FixVector *p;

    mode = g_unk0x0053cff8[pSel[0]];
    if (mode == 4)
        p = &g_unk0x0053d000[pSel[1]];
    else
        p = &g_unk0x00519ea0[mode];
    *pOut = *p;
    if (FUN_0041f3a0() == 0 && RallyData_FUN_00411880() != 0 && CGameInfo::FUN_00405dc0()) {
        pOut->y -= 0x3333;
        pOut->z -= 0x9999;
    }
}

void FixMatrix_GetUp(FixVector *pOut, FixMatrix *pM);
void FixMatrix_GetRight(FixVector *pOut, FixMatrix *pM);

// Takes the camera up/right vectors of a view from the matrix, or copies them
// from the other view when both follow the same car in modes 4..6.
// FUNCTION: CMR2 0x00447be0
void FUN_00447be0(BYTE *pDst, BYTE *pSrc, FixMatrix *pM)
{
    int mode;

    if (pSrc[2] == pDst[2]) {
        mode = *(int *)(pSrc + 4);
        if (mode == 4 || mode == 5 || mode == 6) {
            g_unk0x0053d048[2 + pDst[0]] = g_unk0x0053d048[2 + pSrc[0]];
            g_unk0x0053d000[2 + pDst[0]] = g_unk0x0053d000[2 + pSrc[0]];
            return;
        }
    }
    FixMatrix_GetUp(&g_unk0x0053d048[2 + pDst[0]], pM);
    FixMatrix_GetRight(&g_unk0x0053d000[2 + pDst[0]], pM);
}

// Sets a player's camera offset (and its adjusted copy for the gauges).
// FUNCTION: CMR2 0x00447d20
void FUN_00447d20(BYTE index, FixVector *pOffset)
{
    int i = index;

    g_unk0x0053d048[i] = *pOffset;
    g_unk0x0053d000[i] = *pOffset;
    g_unk0x0053d000[i].y =
        0x20000 - FixMul(0x10000 - FixMul(FixDiv(g_unk0x0053d048[i].z - 0x50000, 0x50000), 0xcccc),
                         0x20000 - g_unk0x0053d000[i].y);
    g_unk0x0053d000[i].z = -g_unk0x0053d000[i].z;
    FUN_00447e20(index, FUN_00447e00(index));
}

// Resets a player's camera to the default offset, height and distance.
// FUNCTION: CMR2 0x00447ca0
void FUN_00447ca0(unsigned int index)
{
    FixVector offset;

    offset.x = 0;
    offset.y = 0x18000;
    offset.z = 0x68000;
    FUN_00447d20(index, &offset);
    FUN_00447e20(index, 0x2d);
    FUN_00447ec0(index, 0xe0000);
}

void RallyData_FUN_00408c20(int *pPos, short *pHeading, int *pValue, int index);
void FUN_00447a40(BYTE *pObj, FixMatrix *pRef);

// Stores the camera mode of the owner of a HUD slot and, when the slot's delay
// has elapsed, rebuilds its offset heading (through the shared position helper)
// and pushes the new height/offset into the dash state.
// match 29.79%: implementada; MSVC6 mantiene param_1 en ESI (y no en EDI) y el marco es de
// 12 bytes en vez de 16, asi que todos los desplazamientos de pila difieren; la logica y el
// orden de llamadas coinciden.
// FUNCTION: CMR2 0x00447530
void FUN_00447530(BYTE *param_1, BYTE *param_2, int param_3)
{
    FixVector offset;
    BYTE index;

    index = param_1[1];
    g_unk0x0053cff8[param_1[0]] = (BYTE)param_3;
    if (param_1[2] >= (BYTE)RallyDataState()) {
        RallyData_FUN_00408c20((int *)&offset, (short *)&param_2, &param_3, 0);
        FUN_00447d20(index, &offset);
        FUN_00447ec0(index, param_3);
    } else {
        RallyData_FUN_00408c20((int *)&offset, (short *)&param_2, &param_3,
                               (int)(BYTE)FUN_0041b370() + param_1[2]);
        FUN_00447d20(index, &offset);
        FUN_00447ec0(index, param_3);
    }
    FUN_00447e20(index, *(short *)&param_1);
    FUN_00447a40(param_1, (FixMatrix *)param_2);
}


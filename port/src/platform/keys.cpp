// SDL scancodes <-> DirectInput scan codes (DIK_*), the game's key codes:
// they are stored in the control bindings and the save files.

#include "platform/platform.h"

namespace {

struct KeyMapping {
    SDL_Scancode scancode;
    unsigned char dik;
};

const KeyMapping s_keys[] = {
    { SDL_SCANCODE_ESCAPE, 0x01 },     { SDL_SCANCODE_1, 0x02 },          { SDL_SCANCODE_2, 0x03 },
    { SDL_SCANCODE_3, 0x04 },          { SDL_SCANCODE_4, 0x05 },          { SDL_SCANCODE_5, 0x06 },
    { SDL_SCANCODE_6, 0x07 },          { SDL_SCANCODE_7, 0x08 },          { SDL_SCANCODE_8, 0x09 },
    { SDL_SCANCODE_9, 0x0A },          { SDL_SCANCODE_0, 0x0B },          { SDL_SCANCODE_MINUS, 0x0C },
    { SDL_SCANCODE_EQUALS, 0x0D },     { SDL_SCANCODE_BACKSPACE, 0x0E },  { SDL_SCANCODE_TAB, 0x0F },
    { SDL_SCANCODE_Q, 0x10 },          { SDL_SCANCODE_W, 0x11 },          { SDL_SCANCODE_E, 0x12 },
    { SDL_SCANCODE_R, 0x13 },          { SDL_SCANCODE_T, 0x14 },          { SDL_SCANCODE_Y, 0x15 },
    { SDL_SCANCODE_U, 0x16 },          { SDL_SCANCODE_I, 0x17 },          { SDL_SCANCODE_O, 0x18 },
    { SDL_SCANCODE_P, 0x19 },          { SDL_SCANCODE_LEFTBRACKET, 0x1A }, { SDL_SCANCODE_RIGHTBRACKET, 0x1B },
    { SDL_SCANCODE_RETURN, 0x1C },     { SDL_SCANCODE_LCTRL, 0x1D },      { SDL_SCANCODE_A, 0x1E },
    { SDL_SCANCODE_S, 0x1F },          { SDL_SCANCODE_D, 0x20 },          { SDL_SCANCODE_F, 0x21 },
    { SDL_SCANCODE_G, 0x22 },          { SDL_SCANCODE_H, 0x23 },          { SDL_SCANCODE_J, 0x24 },
    { SDL_SCANCODE_K, 0x25 },          { SDL_SCANCODE_L, 0x26 },          { SDL_SCANCODE_SEMICOLON, 0x27 },
    { SDL_SCANCODE_APOSTROPHE, 0x28 }, { SDL_SCANCODE_GRAVE, 0x29 },      { SDL_SCANCODE_LSHIFT, 0x2A },
    { SDL_SCANCODE_BACKSLASH, 0x2B },  { SDL_SCANCODE_Z, 0x2C },          { SDL_SCANCODE_X, 0x2D },
    { SDL_SCANCODE_C, 0x2E },          { SDL_SCANCODE_V, 0x2F },          { SDL_SCANCODE_B, 0x30 },
    { SDL_SCANCODE_N, 0x31 },          { SDL_SCANCODE_M, 0x32 },          { SDL_SCANCODE_COMMA, 0x33 },
    { SDL_SCANCODE_PERIOD, 0x34 },     { SDL_SCANCODE_SLASH, 0x35 },      { SDL_SCANCODE_RSHIFT, 0x36 },
    { SDL_SCANCODE_KP_MULTIPLY, 0x37 }, { SDL_SCANCODE_LALT, 0x38 },      { SDL_SCANCODE_SPACE, 0x39 },
    { SDL_SCANCODE_CAPSLOCK, 0x3A },   { SDL_SCANCODE_F1, 0x3B },         { SDL_SCANCODE_F2, 0x3C },
    { SDL_SCANCODE_F3, 0x3D },         { SDL_SCANCODE_F4, 0x3E },         { SDL_SCANCODE_F5, 0x3F },
    { SDL_SCANCODE_F6, 0x40 },         { SDL_SCANCODE_F7, 0x41 },         { SDL_SCANCODE_F8, 0x42 },
    { SDL_SCANCODE_F9, 0x43 },         { SDL_SCANCODE_F10, 0x44 },        { SDL_SCANCODE_NUMLOCKCLEAR, 0x45 },
    { SDL_SCANCODE_SCROLLLOCK, 0x46 }, { SDL_SCANCODE_KP_7, 0x47 },       { SDL_SCANCODE_KP_8, 0x48 },
    { SDL_SCANCODE_KP_9, 0x49 },       { SDL_SCANCODE_KP_MINUS, 0x4A },   { SDL_SCANCODE_KP_4, 0x4B },
    { SDL_SCANCODE_KP_5, 0x4C },       { SDL_SCANCODE_KP_6, 0x4D },       { SDL_SCANCODE_KP_PLUS, 0x4E },
    { SDL_SCANCODE_KP_1, 0x4F },       { SDL_SCANCODE_KP_2, 0x50 },       { SDL_SCANCODE_KP_3, 0x51 },
    { SDL_SCANCODE_KP_0, 0x52 },       { SDL_SCANCODE_KP_PERIOD, 0x53 },  { SDL_SCANCODE_NONUSBACKSLASH, 0x56 },
    { SDL_SCANCODE_F11, 0x57 },        { SDL_SCANCODE_F12, 0x58 },        { SDL_SCANCODE_KP_ENTER, 0x9C },
    { SDL_SCANCODE_RCTRL, 0x9D },      { SDL_SCANCODE_KP_DIVIDE, 0xB5 },  { SDL_SCANCODE_PRINTSCREEN, 0xB7 },
    { SDL_SCANCODE_RALT, 0xB8 },       { SDL_SCANCODE_PAUSE, 0xC5 },      { SDL_SCANCODE_HOME, 0xC7 },
    { SDL_SCANCODE_UP, 0xC8 },         { SDL_SCANCODE_PAGEUP, 0xC9 },     { SDL_SCANCODE_LEFT, 0xCB },
    { SDL_SCANCODE_RIGHT, 0xCD },      { SDL_SCANCODE_END, 0xCF },        { SDL_SCANCODE_DOWN, 0xD0 },
    { SDL_SCANCODE_PAGEDOWN, 0xD1 },   { SDL_SCANCODE_INSERT, 0xD2 },     { SDL_SCANCODE_DELETE, 0xD3 },
    { SDL_SCANCODE_LGUI, 0xDB },       { SDL_SCANCODE_RGUI, 0xDC },       { SDL_SCANCODE_APPLICATION, 0xDD },
};

}

int Keys_FromScancode(SDL_Scancode scancode)
{
    for (const KeyMapping &key : s_keys)
        if (key.scancode == scancode)
            return key.dik;
    return 0;
}

SDL_Scancode Keys_ToScancode(int dik)
{
    for (const KeyMapping &key : s_keys)
        if (key.dik == dik)
            return key.scancode;
    return SDL_SCANCODE_UNKNOWN;
}

const char *Keys_GetName(int dik)
{
    SDL_Scancode scancode = Keys_ToScancode(dik);
    if (scancode == SDL_SCANCODE_UNKNOWN)
        return "";
    // The key's name in the current keyboard layout ("Y" on a QWERTZ
    // keyboard for the scan code a US layout calls "Z").
    SDL_Keycode keycode = SDL_GetKeyFromScancode(scancode, SDL_KMOD_NONE, false);
    const char *name = SDL_GetKeyName(keycode);
    return name && *name ? name : SDL_GetScancodeName(scancode);
}

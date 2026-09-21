#ifndef _MENU_H
#define _MENU_H

// Frontend menu: a header plus up to 22 items of 0x14 bytes, followed by
// four callbacks. Built by the screen code through the Menu_Add* helpers.

struct Menu;

typedef void (*MenuCallback)(Menu *pMenu, int param);

struct MenuItem {
    int stringId;           // 0x0
    short id;               // 0x4  -1 = none
    BYTE enabled : 1;       // 0x6
    BYTE visible : 1;
    BYTE flag2 : 1;
    BYTE flag3 : 1;
    BYTE flag4 : 4;
    BYTE type;              // 0x7  1, 2, 3, 4 or 6
    short value;            // 0x8
    BYTE min;               // 0xa
    BYTE max;               // 0xb
    Menu *pSubMenu;         // 0xc  type 2 only
    int param;              // 0x10
};

struct Menu {
    int stringId;           // 0x0   title
    short field_0x4;
    char itemCount;         // 0x6
    char cursor;            // 0x7
    Menu *pParent;          // 0x8  -1 passed to Menu_Init means none
    char defaultCursor;     // 0xc
    BYTE field_0xd;
    BYTE flag0 : 1;         // 0xe
    BYTE flag1 : 1;
    BYTE flag2 : 1;
    BYTE flag3 : 1;
    BYTE flag4 : 1;
    BYTE flag5 : 1;
    BYTE flag6 : 1;         // userData == -1
    BYTE flag7 : 1;
    BYTE field_0xf;
    BYTE field_0x10;        // only bit 7 survives Menu_Init
    BYTE field_0x11[3];
    MenuItem items[22];     // 0x14
    MenuCallback pfnCallback0;      // 0x1cc
    MenuCallback pfnCallback1;      // 0x1d0
    MenuCallback pfnCallback2;      // 0x1d4
    MenuCallback pfnCallback3;      // 0x1d8
    int field_0x1dc;
};

void Menu_Init(Menu *pMenu, int stringId, short param3, int param4, Menu *pParent, int param6, BYTE flag4, BYTE defaultCursor, BYTE param9);
void Menu_ClearNextItem(Menu *pMenu);
void Menu_AddItemType3(Menu *pMenu, int stringId, short id, BYTE min, BYTE max, BYTE flag2, int unused, int param, short value);
void Menu_AddItemType6(Menu *pMenu, int stringId, short id, BYTE min, BYTE max, BYTE flag2, int unused, int param, short value);
void Menu_AddItemType4(Menu *pMenu, int stringId, short id, int param, short value);
void Menu_AddItemType1(Menu *pMenu, int stringId, short id, int param, short value);
void Menu_AddItemType2(Menu *pMenu, int stringId, short id, Menu *pSubMenu, int param, short value);
void Menu_SetCursor(Menu *pMenu, BYTE cursor);
void Menu_SetParent(Menu *pMenu, Menu *pParent);
int Menu_FindItem(Menu *pMenu, int id);
MenuItem *Menu_GetItem(Menu *pMenu, int id);
void Menu_SetItemSubMenu(Menu *pMenu, int id, Menu *pSubMenu);
void Menu_SelectItem(Menu *pMenu, int id);
void Menu_SetCallbacks(Menu *pMenu, MenuCallback pfn0, MenuCallback pfn1, MenuCallback pfn2, MenuCallback pfn3);
void Menu_SetFlags(Menu *pMenu, BYTE bit0, BYTE bit1, BYTE bit2, BYTE bit3);
void Menu_ValidateCursor(Menu *pMenu, int unused);
void Menu_CallCallback0(Menu *pMenu);
void Menu_CallCallback3(Menu *pMenu);
void Menu_CallCallback2(Menu *pMenu);
void Menu_SetNextAction(int action);
void Menu_PlaySound(int id);

#endif

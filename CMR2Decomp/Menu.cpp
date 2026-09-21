#include <windows.h>
#include "Menu.h"
#include "Input.h"
#include "Sound.h"

// GLOBAL: CMR2 0x0059f8fc
char g_unk0x0059f8fc;
// GLOBAL: CMR2 0x0059f908
int g_menuNextAction;
// GLOBAL: CMR2 0x0059fa14
char g_unk0x0059fa14;
// GLOBAL: CMR2 0x0059fa16
char g_unk0x0059fa16;
// GLOBAL: CMR2 0x0059fa17
char g_unk0x0059fa17;

// FUNCTION: CMR2 0x0049ffd0
void Menu_Init(Menu *pMenu, int stringId, short param3, int param4, Menu *pParent, int param6, BYTE flag4, BYTE defaultCursor, BYTE param9)
{
    pMenu->stringId = stringId;
    pMenu->field_0x4 = param3;
    pMenu->cursor = defaultCursor;
    pMenu->pParent = pParent == (Menu *)-1 ? NULL : pParent;
    pMenu->flag6 = pParent == (Menu *)-1;
    pMenu->itemCount = 0;
    pMenu->pfnCallback0 = NULL;
    pMenu->pfnCallback1 = NULL;
    pMenu->pfnCallback2 = NULL;
    pMenu->pfnCallback3 = NULL;
    pMenu->flag0 = 1;
    pMenu->flag1 = 1;
    pMenu->flag2 = 1;
    pMenu->flag3 = 1;
    pMenu->flag5 = 1;
    pMenu->flag4 = flag4;
    pMenu->field_0xd = param9;
    pMenu->defaultCursor = -1;
    pMenu->field_0x10 &= 0x80;
    pMenu->field_0x1dc = param6;
}

// FUNCTION: CMR2 0x004a0060
void Menu_ClearNextItem(Menu *pMenu)
{
    MenuItem *pItem;

    pItem = &pMenu->items[pMenu->itemCount];
    pItem->stringId = 0;
    pItem->type = 0;
    pItem->value = 0;
    pItem->param = 0;
    pItem->min = 0;
    pItem->max = 0;
    pItem->pSubMenu = NULL;
    pItem->id = -1;
    pItem->enabled = 1;
    pItem->visible = 1;
    pItem->flag2 = 0;
    pItem->flag3 = 0;
}

// FUNCTION: CMR2 0x004a00a0
void Menu_AddItemType3(Menu *pMenu, int stringId, short id, BYTE min, BYTE max, BYTE flag2, int unused, int param, short value)
{
    Menu_ClearNextItem(pMenu);
    pMenu->items[pMenu->itemCount].stringId = stringId;
    pMenu->items[pMenu->itemCount].id = id;
    pMenu->items[pMenu->itemCount].type = 3;
    pMenu->items[pMenu->itemCount].min = min;
    pMenu->items[pMenu->itemCount].max = max;
    pMenu->items[pMenu->itemCount].flag2 = flag2;
    pMenu->items[pMenu->itemCount].value = value;
    pMenu->items[pMenu->itemCount].param = param;
    pMenu->itemCount++;
}

// FUNCTION: CMR2 0x004a0150
void Menu_AddItemType6(Menu *pMenu, int stringId, short id, BYTE min, BYTE max, BYTE flag2, int unused, int param, short value)
{
    Menu_ClearNextItem(pMenu);
    pMenu->items[pMenu->itemCount].stringId = stringId;
    pMenu->items[pMenu->itemCount].id = id;
    pMenu->items[pMenu->itemCount].type = 6;
    pMenu->items[pMenu->itemCount].min = min;
    pMenu->items[pMenu->itemCount].max = max;
    pMenu->items[pMenu->itemCount].flag2 = flag2;
    pMenu->items[pMenu->itemCount].value = value;
    pMenu->items[pMenu->itemCount].param = param;
    pMenu->itemCount++;
}

// FUNCTION: CMR2 0x004a0200
void Menu_AddItemType4(Menu *pMenu, int stringId, short id, int param, short value)
{
    Menu_ClearNextItem(pMenu);
    pMenu->items[pMenu->itemCount].stringId = stringId;
    pMenu->items[pMenu->itemCount].id = id;
    pMenu->items[pMenu->itemCount].type = 4;
    pMenu->items[pMenu->itemCount].value = value;
    pMenu->items[pMenu->itemCount].param = param;
    pMenu->itemCount++;
}

// FUNCTION: CMR2 0x004a0270
void Menu_AddItemType1(Menu *pMenu, int stringId, short id, int param, short value)
{
    Menu_ClearNextItem(pMenu);
    pMenu->items[pMenu->itemCount].stringId = stringId;
    pMenu->items[pMenu->itemCount].id = id;
    pMenu->items[pMenu->itemCount].type = 1;
    pMenu->items[pMenu->itemCount].value = value;
    pMenu->items[pMenu->itemCount].param = param;
    pMenu->itemCount++;
}

// FUNCTION: CMR2 0x004a02e0
void Menu_AddItemType2(Menu *pMenu, int stringId, short id, Menu *pSubMenu, int param, short value)
{
    Menu_ClearNextItem(pMenu);
    pMenu->items[pMenu->itemCount].stringId = stringId;
    pMenu->items[pMenu->itemCount].id = id;
    pMenu->items[pMenu->itemCount].type = 2;
    pMenu->items[pMenu->itemCount].pSubMenu = pSubMenu;
    pMenu->items[pMenu->itemCount].value = value;
    pMenu->items[pMenu->itemCount].param = param;
    pMenu->itemCount++;
}

// FUNCTION: CMR2 0x004a0360
void Menu_SetCursor(Menu *pMenu, BYTE cursor)
{
    pMenu->cursor = cursor;
}

// FUNCTION: CMR2 0x004a0370
void Menu_SetParent(Menu *pMenu, Menu *pParent)
{
    pMenu->pParent = pParent;
}

// FUNCTION: CMR2 0x004a0380
int Menu_FindItem(Menu *pMenu, int id)
{
    int i;
    int count;

    count = pMenu->itemCount;
    for (i = 0; i < count; i++) {
        if (pMenu->items[i].id == id)
            return i;
    }
    return -1;
}

// FUNCTION: CMR2 0x004a03b0
MenuItem *Menu_GetItem(Menu *pMenu, int id)
{
    int i;

    i = Menu_FindItem(pMenu, id);
    if (i == -1)
        return NULL;
    return &pMenu->items[i];
}

// FUNCTION: CMR2 0x004a03e0
void Menu_SetItemSubMenu(Menu *pMenu, int id, Menu *pSubMenu)
{
    MenuItem *pItem;

    pItem = Menu_GetItem(pMenu, id);
    if (pItem != NULL && pItem->type == 2)
        pItem->pSubMenu = pSubMenu;
}

// FUNCTION: CMR2 0x004a0410
void Menu_SelectItem(Menu *pMenu, int id)
{
    pMenu->cursor = (char)Menu_FindItem(pMenu, id);
}

// FUNCTION: CMR2 0x004a0430
void Menu_SetCallbacks(Menu *pMenu, MenuCallback pfn0, MenuCallback pfn1, MenuCallback pfn2, MenuCallback pfn3)
{
    pMenu->pfnCallback0 = pfn0;
    pMenu->pfnCallback1 = pfn1;
    pMenu->pfnCallback2 = pfn2;
    pMenu->pfnCallback3 = pfn3;
}

// FUNCTION: CMR2 0x004a0460
void Menu_SetFlags(Menu *pMenu, BYTE bit0, BYTE bit1, BYTE bit2, BYTE bit3)
{
    pMenu->flag0 = bit0;
    pMenu->flag1 = bit1;
    pMenu->flag2 = bit2;
    pMenu->flag3 = bit3;
}

// Moves the cursor to the next item that is enabled (flags bit0|bit1),
// starting at defaultCursor when it is valid.
// FUNCTION: CMR2 0x004a04a0
void Menu_ValidateCursor(Menu *pMenu, int unused)
{
    char def;
    int count;
    int tries;
    int i;

    def = pMenu->defaultCursor;
    if (def >= 0 && def < pMenu->itemCount)
        pMenu->cursor = def;
    count = pMenu->itemCount;
    tries = 0;
    i = pMenu->cursor;
    for (tries = 0; tries < count; tries++, i++) {
        if (pMenu->items[i % count].enabled && pMenu->items[i % count].visible) {
            pMenu->cursor = (char)(i % count);
            return;
        }
    }
}

// FUNCTION: CMR2 0x004a04f0
void Menu_CallCallback0(Menu *pMenu)
{
    MenuCallback pfn;
    int param;

    pfn = pMenu->pfnCallback0;
    if (pfn != NULL) {
        if (g_unk0x0059fa16 == 0 && g_unk0x0059fa17 == 0)
            param = 0;
        else
            param = 1;
        pfn(pMenu, param);
    }
}

// FUNCTION: CMR2 0x004a0530
void Menu_CallCallback3(Menu *pMenu)
{
    if (pMenu->pfnCallback3 != NULL)
        pMenu->pfnCallback3(pMenu, 0);
}

// FUNCTION: CMR2 0x004a0550
void Menu_CallCallback2(Menu *pMenu)
{
    if (pMenu != NULL && pMenu->pfnCallback2 != NULL)
        ((void (*)(Menu *))pMenu->pfnCallback2)(pMenu);
}

// Queues the action of the selected item; stops the current joystick
// effect first unless a menu transition is already pending.
// FUNCTION: CMR2 0x0049ff50
void Menu_PlaySound(int id)
{
    if (id >= 0)
        FUN_004b7790(id, CInput::m_unk0x0059f900, 0x57e4, 0, 0, 0);
}

// FUNCTION: CMR2 0x004a0ad0
void Menu_SetNextAction(int action)
{
    if (g_unk0x0059f8fc == 0 && g_unk0x0059fa14 != 0)
        Menu_PlaySound(CInput::m_unk0x0059f910);
    g_unk0x0059fa16 = 0;
    g_menuNextAction = action;
}

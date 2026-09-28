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
// GLOBAL: CMR2 0x0059f904
char g_menuActionPending;
// GLOBAL: CMR2 0x0059fa15
char g_unk0x0059fa15;
// GLOBAL: CMR2 0x0059fa16
char g_unk0x0059fa16;
// GLOBAL: CMR2 0x0059fa18
unsigned int g_menuLastInput;
// GLOBAL: CMR2 0x0059fa17
char g_unk0x0059fa17;

// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049ffd0
void Menu_Init(Menu *pMenu, int stringId, short param3, int param4, Menu *pParent, MenuItemCallbacks *pItemCallbacks, BYTE flag4, BYTE defaultCursor, BYTE layout)
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
    pMenu->layout = layout;
    pMenu->defaultCursor = -1;
    pMenu->value &= 0x80;
    pMenu->pItemCallbacks = pItemCallbacks;
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
    i = pMenu->cursor;
    for (tries = 0; tries < count; tries++, i++) {
        int idx = i % count;
        if (pMenu->items[idx].enabled && pMenu->items[idx].visible) {
            pMenu->cursor = (char)idx;
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

#define ITEM_AT(pMenu, i) (&(pMenu)->items[(i)])
#define MENU_FLAGS(pMenu) (*((BYTE *)(pMenu) + 0xe))

// Applies one frame of input to the menu: moves the cursor, changes the
// value of type 3/6 items, fires the item callbacks and returns the action
// queued by the selected item (0 = none). Input bits: 0-3 directions (the
// pairs swap with layout), 4 select, 5 back.
// match 86%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a0570
int Menu_Update(Menu *pMenu, unsigned int input)
{
    BYTE bNext;
    BYTE bLeft;
    BYTE bRight;
    BYTE bBack;
    BYTE bSelect;
    BYTE bPrev;
    char count;
    char cursor;
    int i;
    int moved;
    int wrapped;
    MenuItem *pItem;
    int action;
    BYTE flags;
    BYTE v;

    if (g_unk0x0059fa15 == 0 && (input & 0xc) != 0 && (input & 3) != 0)
        input = 0;
    g_menuLastInput = input;
    g_unk0x0059f8fc = 0;
    if (pMenu == NULL)
        return 0;
    if (g_menuActionPending != 0) {
        Menu_CallCallback0(pMenu);
        g_menuActionPending = 0;
        g_unk0x0059fa17 = 0;
    }
    if (g_menuNextAction == 0) {
        flags = MENU_FLAGS(pMenu);
        if (flags & 4)
            bSelect = (input >> 4) & 1;
        else
            bSelect = 0;
        if (flags & 8)
            bBack = (input >> 5) & 1;
        else
            bBack = 0;
        if (flags & 2) {
            if (pMenu->layout == 1) {
                bLeft = input & 1;
                bRight = (input >> 1) & 1;
            } else {
                bLeft = (input >> 2) & 1;
                bRight = (input >> 3) & 1;
            }
            if (bLeft && bRight)
                bLeft = 0;
        } else {
            bLeft = 0;
            bRight = 0;
        }
        if ((flags & 1) && !bLeft && !bRight) {
            if (pMenu->layout == 1) {
                bPrev = (input >> 2) & 1;
                bNext = (input >> 3) & 1;
            } else {
                bPrev = input & 1;
                bNext = (input >> 1) & 1;
            }
            if (bPrev && bNext)
                bPrev = 0;
        } else {
            bPrev = 0;
            bNext = 0;
        }

        count = pMenu->itemCount;
        i = count;
        while (i > 0 && !ITEM_AT(pMenu, pMenu->cursor)->enabled) {
            cursor = pMenu->cursor + 1;
            i--;
            pMenu->cursor = cursor;
            if (cursor >= count)
                pMenu->cursor = 0;
        }

        if (bPrev) {
            cursor = pMenu->cursor;
            moved = 1;
            wrapped = 1;
            if (cursor > 0 || pMenu->flag4) {
                i = 0;
                do {
                    cursor--;
                    pMenu->cursor = cursor;
                    if (cursor < 0)
                        pMenu->cursor = cursor + count;
                    i++;
                    if (i >= count)
                        wrapped = 0;
                    cursor = pMenu->cursor;
                } while (!ITEM_AT(pMenu, cursor)->enabled || !ITEM_AT(pMenu, cursor)->visible);
                pMenu->moveFlags |= 1;
                if (wrapped && g_unk0x0059fa14 != 0)
                    Menu_PlaySound(CInput::m_unk0x0059f8f8);
            } else {
                moved = 0;
            }
            if (pMenu->pItemCallbacks != NULL && pMenu->pItemCallbacks->pfnMove != NULL)
                pMenu->pItemCallbacks->pfnMove(pMenu, ITEM_AT(pMenu, pMenu->cursor), moved);
        } else if (bNext) {
            cursor = pMenu->cursor;
            moved = 1;
            wrapped = 1;
            if (cursor < count - 1 || (pMenu->flag4 && count != 0)) {
                i = 0;
                do {
                    cursor++;
                    pMenu->cursor = cursor;
                    if (cursor >= count)
                        pMenu->cursor = 0;
                    i++;
                    if (i >= count)
                        wrapped = 0;
                    cursor = pMenu->cursor;
                } while (!ITEM_AT(pMenu, cursor)->enabled || !ITEM_AT(pMenu, cursor)->visible);
                pMenu->moveFlags &= 0xfe;
                if (wrapped && g_unk0x0059fa14 != 0)
                    Menu_PlaySound(CInput::m_unk0x0059f8f8);
            } else {
                moved = 0;
            }
            if (pMenu->pItemCallbacks != NULL && pMenu->pItemCallbacks->pfnMove != NULL)
                pMenu->pItemCallbacks->pfnMove(pMenu, ITEM_AT(pMenu, pMenu->cursor), moved);
        }

        pItem = ITEM_AT(pMenu, pMenu->cursor);
        if (bSelect && pItem->enabled)
            bSelect = 1;
        else
            bSelect = 0;
        if ((bPrev || bNext) && pItem->type == 6) {
            if ((pMenu->value & 0x7f) >= pItem->min)
                pMenu->value = ((pItem->min - 1) ^ pMenu->value) & 0x7f ^ pMenu->value;
            pItem->max = pMenu->value & 0x7f;
        }
        if (pItem->type == 3 || pItem->type == 6) {
            if (bLeft) {
                moved = 1;
                if (pItem->max > 0 || pItem->flag2) {
                    pItem->max--;
                    if (pItem->max == 0xff)
                        pItem->max = pItem->min - 1;
                    if (g_unk0x0059fa14 != 0)
                        Menu_PlaySound(CInput::m_unk0x0059f90c);
                    pMenu->moveFlags &= 0xfd;
                } else {
                    moved = 0;
                }
                if (pMenu->pItemCallbacks != NULL && pMenu->pItemCallbacks->pfnChange != NULL)
                    pMenu->pItemCallbacks->pfnChange(pMenu, ITEM_AT(pMenu, pMenu->cursor), moved);
            } else if (bRight || (bSelect && pItem->flag3)) {
                moved = 1;
                if (pItem->max < pItem->min - 1 || pItem->flag2 || (bSelect && pItem->flag3)) {
                    if (!bSelect || pItem->param == 0) {
                        pItem->max++;
                        if (pItem->max >= pItem->min)
                            pItem->max = 0;
                    }
                    if (g_unk0x0059fa14 != 0)
                        Menu_PlaySound(CInput::m_unk0x0059f90c);
                    pMenu->moveFlags |= 2;
                } else {
                    moved = 0;
                }
                if (pMenu->pItemCallbacks != NULL && pMenu->pItemCallbacks->pfnChange != NULL)
                    pMenu->pItemCallbacks->pfnChange(pMenu, ITEM_AT(pMenu, pMenu->cursor), moved);
            }
            pMenu->value = (pItem->max ^ pMenu->value) & 0x7f ^ pMenu->value;
        }

        if (bSelect == 0) {
            if (bBack) {
back:
                g_menuNextAction = (int)pMenu->pParent;
                g_unk0x0059f8fc = 1;
                if (pMenu->pItemCallbacks != NULL && pMenu->pItemCallbacks->pfnBack != NULL)
                    pMenu->pItemCallbacks->pfnBack(pMenu, ITEM_AT(pMenu, pMenu->cursor), 1);
                if (g_unk0x0059fa14 != 0)
                    Menu_PlaySound(CInput::m_unk0x0059f8f4);
            }
        } else {
            if (pItem->param != 0)
                ((void (*)(Menu *, MenuItem *))pItem->param)(pMenu, pItem);
            if (bBack || pItem->type == 1)
                goto back;
            if (pItem->type == 2) {
                g_menuNextAction = (int)pItem->pSubMenu;
                if (pMenu->pItemCallbacks != NULL && pMenu->pItemCallbacks->pfnSelect != NULL)
                    pMenu->pItemCallbacks->pfnSelect(pMenu, ITEM_AT(pMenu, pMenu->cursor), 1);
                if (g_unk0x0059fa14 != 0)
                    Menu_PlaySound(CInput::m_unk0x0059f910);
            }
        }
        if (pMenu->pfnCallback1 != NULL)
            ((void (*)(Menu *))pMenu->pfnCallback1)(pMenu);
        if (g_menuNextAction == 0)
            goto done;
        if (bBack) {
            v = 1;
            goto notify;
        }
    }
    if (ITEM_AT(pMenu, pMenu->cursor)->type == 1)
        v = 1;
    else
        v = 0;
notify:
    g_unk0x0059fa16 = v;
    if (pMenu->pfnCallback3 != NULL)
        pMenu->pfnCallback3(pMenu, v);
    if (g_menuNextAction != 0)
        g_menuActionPending = 1;
done:
    action = g_menuNextAction;
    g_menuNextAction = 0;
    return action;
}

// Goes back to the parent menu (with the back sound unless muted).
// FUNCTION: CMR2 0x004a0b30
void Menu_GoBack(Menu *pMenu)
{
    if (g_unk0x0059f8fc == 0 && g_unk0x0059fa14 != 0)
        Menu_PlaySound(CInput::m_unk0x0059f910);
    g_menuNextAction = (int)pMenu->pParent;
    g_unk0x0059fa17 = 1;
}

// FUNCTION: CMR2 0x004a0ba0
void FUN_004a0ba0(void)
{
    g_menuActionPending = 1;
    g_unk0x0059fa15 = 0;
    g_unk0x0059fa16 = 0;
    g_menuNextAction = 0;
}

// match 82%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a0bc0
void Menu_PlaySoundId(int id)
{
    switch (id) {
    case 0:
        id = CInput::m_unk0x0059f8f8;
        break;
    case 1:
        id = CInput::m_unk0x0059f910;
        break;
    case 2:
        id = CInput::m_unk0x0059f8f4;
        break;
    case 3:
        id = CInput::m_unk0x0059f8f0;
        break;
    case 4:
        id = CInput::m_unk0x0059f90c;
        break;
    }
    Menu_PlaySound(id);
}

// FUNCTION: CMR2 0x004a0c40
void FUN_004a0c40(char param1)
{
    g_unk0x0059fa14 = param1;
}

// FUNCTION: CMR2 0x004a0c50
void FUN_004a0c50(char param1)
{
    g_unk0x0059fa15 = param1;
}

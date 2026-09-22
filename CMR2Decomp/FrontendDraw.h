#ifndef _FRONTENDDRAW_H
#define _FRONTENDDRAW_H

#include "Menu.h"

// Drawing helpers shared by the frontend screens (0x4d2cd0-0x4d45b0)

extern BYTE g_colourWhite0x00524968[4];
extern BYTE g_colourText0x0052496c[4];

int FrontendDraw_BreadcrumbItem(int x, int y, BYTE *pColour, char last, char *text);
void FrontendDraw_Breadcrumb(int x, int y, char **ppText, int count);
void FrontendDraw_MenuTitle(Menu *pMenu);
int FrontendDraw_MenuPath(Menu *pMenu, int x, int y, char last, int depth, char **ppNames, int nameCount);
void FrontendDraw_PlayTime(void);

#endif

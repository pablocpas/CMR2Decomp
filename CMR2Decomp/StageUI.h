#ifndef _STAGE_UI_H
#define _STAGE_UI_H

#include <windows.h>

void StageUI_ApplySoundState(int channel, BYTE *pState);

void FormatGapToLeader(int iLeaderGap, unsigned int param_2, unsigned char param_3, int param_4, int param_5, void *param_6, unsigned int param_7, const char *pcNegPosSymbol, int param_9);

void PrepareFormatGapToLeader(int iLeaderGap, unsigned char param_2, unsigned int param_3, unsigned char param_4, int param_5, int param_6, void *param_7, unsigned int param_8, BOOL bIsAhead, int param_10);

void FUN_0041b300(void);
void FUN_0041b310(void);
int FUN_0041b320(void);
void FUN_0041b330(void);
void FUN_0041b340(char bFlag);
void FUN_0041b360(void);
BYTE FUN_0041b370(void);
int FUN_0041b380(void);
BYTE *FUN_0041b390(void);

#endif

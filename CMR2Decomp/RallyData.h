#ifndef _RALLY_DATA_H
#define _RALLY_DATA_H

unsigned int RallyDataCountryIndex(void);
unsigned char RallyDataStageIndex(void);
unsigned int RallyDataState(void);
unsigned int RallyData_FUN_00406940(void);
unsigned int RallyData_FUN_00406950(void);
unsigned int RallyData_FUN_00406990(void);
BYTE RallyData_FUN_004069a0(void);
void RallyData_UpdateFlags(void);
void RallyData_ResetSelection(void);
void RallyData_FUN_0040d600(BYTE param1);
void RallyData_FUN_0040d620(BYTE param1);
void RallyData_FUN_0040d640(BYTE param1);
void RallyData_FUN_0040d660(BYTE param1);
void RallyData_FUN_0040d680(BYTE param1);
void RallyData_FUN_0040d6a0(BYTE param1);

unsigned int RallyData_GetFlag24(void);
unsigned int RallyData_GetFlag25(void);
void RallyData_ValidateIndex(int index);
void *RallyData_GetRecord(BYTE index);
BYTE RallyData_FUN_004086b0(BYTE index);
unsigned int *RallyData_GetChampionshipState(void);
void RallyData_GetRoundDrivers(unsigned int *pFirst, unsigned int *pSecond);
char RallyData_FUN_00408500(unsigned int param1);
int RallyData_FUN_004070f0(void);
int RallyData_FUN_00411880(void);

#endif

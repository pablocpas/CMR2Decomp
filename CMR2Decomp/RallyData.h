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

#endif

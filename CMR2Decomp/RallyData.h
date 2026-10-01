#ifndef _RALLY_DATA_H
#define _RALLY_DATA_H

// Arcade knockout table: state bits 0-2 mode, 3-5 round (1 = first round ..
// 4 = final), 6-8, 12-15 current match, 16-19. Each match holds the two
// driver indices (bits 0-4 and 5-9), the winner (bit 11 second, bit 12 first)
// and the two times.
struct KnockoutMatch {
    union {
        unsigned int flags;
        struct {
            unsigned int first : 5;
            unsigned int second : 5;
            unsigned int played : 1;
            unsigned int winner : 2;
            unsigned int reserved : 19;
        } bits;
    };
    unsigned int time1;
    unsigned int time2;
};

struct KnockoutTable {
    union {
        unsigned int state;     // 0x52f2b4
        struct {
            unsigned int mode : 3;
            unsigned int round : 3;
            unsigned int unknown6 : 3;
            unsigned int active : 1;
            unsigned int progress : 6;
            unsigned int reserved : 16;
        } bits;
    };
    KnockoutMatch final;        // 0x52f2b8
    KnockoutMatch semis[2];     // 0x52f2c4
    KnockoutMatch quarters[4];  // 0x52f2dc
    KnockoutMatch round1[16];   // 0x52f30c
};

struct Car;
void FUN_004129d0(int car, short *pRect);
void FUN_004147f0(int car, short *position);
void RallyData_UpdateCarRoute(Car *pCar);

extern KnockoutTable g_knockout;
void RallyData_InitKnockoutBracket(void);

unsigned char RallyDataCountryIndex(void);
unsigned char RallyDataStageIndex(void);
unsigned int RallyData_FUN_00406940(void);
unsigned int RallyData_FUN_00406950(void);
unsigned int RallyData_FUN_00406990(void);
BYTE RallyData_FUN_004069a0(void);
unsigned char RallyDataState(void);
void RallyData_UpdateFlags(void);
void RallyData_ResetSelection(void);
void RallyData_FUN_0040d600(BYTE param1);
void RallyData_FUN_0040d620(BYTE param1);
void RallyData_FUN_0040d640(BYTE param1);
void RallyData_FUN_0040d660(BYTE param1);
void RallyData_FUN_0040d680(BYTE param1);
void RallyData_FUN_0040d6a0(BYTE param1);

int RallyData_FUN_004070f0(void);
unsigned int *RallyData_GetChampionshipState(void);
void RallyData_GetRoundDrivers(unsigned int *pFirst, unsigned int *pSecond);
unsigned int RallyData_GetFlag24(void);
unsigned int RallyData_GetFlag25(void);
void RallyData_ValidateIndex(int index);
void *RallyData_GetRecord(BYTE index);
char RallyData_FUN_00408500(BYTE param1);
BYTE RallyData_FUN_004086b0(BYTE index);
int RallyData_FUN_00411880(void);

#endif

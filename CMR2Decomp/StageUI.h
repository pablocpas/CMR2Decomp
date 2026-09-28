#ifndef _STAGE_UI_H
#define _STAGE_UI_H

#include <windows.h>

void StageUI_ApplySoundState(int channel, BYTE *pState);

void FormatGapToLeader(int iLeaderGap, unsigned int param_2, unsigned char param_3, int param_4, int param_5, void *param_6, unsigned int param_7, char *pcNegPosSymbol, int param_9);

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


// Race/stage-UI tables 0x537568..0x537dcc. The original uses them as rows of
// 0xb4 bytes (one per car) that overlap the named fields below, so they are
// kept as one contiguous block with the fields addressed inside it.
// GLOBAL: CMR2 0x00537568
extern BYTE g_raceBlock[0x864];

// Stage sounds of each car (rows of 0xb4 bytes starting at 0x5377c4).
struct CarSoundSet {
    int handle[10];         // 0x00 playing sound handle (-1 none)
    int id[10];             // 0x28 sound id
    int pitch[10];          // 0x50 random pitch
    BYTE surface[10];       // 0x78 surface when started
    BYTE field_0x82[0x32];
};

#define g_carSoundSets ((CarSoundSet *)(g_raceBlock + 0x25c))      // 0x5377c4
#define g_stageSoundCount (*(unsigned int *)(g_raceBlock + 0x68))  // 0x5375d0
#define g_stageSoundUsed (g_raceBlock + 0x6c)                      // 0x5375d4: [0x1f] sound groups in use
#define g_unk0x005375f4 (g_raceBlock + 0x8c)                       // 0x5375f4: surface per car
#define g_unk0x00537660 (*(int *)(g_raceBlock + 0xf8))             // 0x537660
#define g_unk0x00537664 (*(int *)(g_raceBlock + 0xfc))             // 0x537664

// Stage sound states; each can redirect to a shared pattern (0x1c bytes).
struct StageSoundPattern {
    char choices[4];
    char redirect;
    BYTE pad[3];
    int count;
    int base[4];
};
extern StageSoundPattern g_stageSoundPatterns[31];
short FUN_00478a10(short index);

// Per-car stage sound state (rows of 0xb4 bytes starting at 0x5377a8, inside
// g_raceBlock). The slot arrays are the same memory as CarSoundSet above.
struct RaceCarSoundState {
    short slotState[4];        // 0x00  last source state of each of the 4 slots
    int slotTime[4];           // 0x08  timestamp of the last change
    short state;               // 0x18  current stage sound state (-1 = none)
    short stateOld;            // 0x1a  state the slot sounds were started for
    int handle[10];            // 0x1c  playing sound handle (-1 = none)
    int id[10];                // 0x44  sound id
    int pitch[10];             // 0x6c  random pitch
    BYTE surface[10];          // 0x94  surface when the sound was started
    BYTE field_0x9e[0x2];
    int count;                 // 0xa0  number of sounds of the current pattern
    int countOld;              // 0xa4  count of the previous pattern
    int pattern;               // 0xa8  pattern index (0x19 = none)
    int time;                  // 0xac  timestamp of the last change
    BYTE field_0xb0[0x4];
};
#define g_carSoundStates ((RaceCarSoundState *)(g_raceBlock + 0x240))   // 0x5377a8

// Engine sample used while no stage sound group is running.
#define g_unk0x00537dc8 (*(int *)(g_raceBlock + 0x860))                 // 0x537dc8

#endif

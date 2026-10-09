#ifndef _STAGE_UI_H
#define _STAGE_UI_H


void StageUI_ApplySoundState(int channel, BYTE *pState);

void FormatGapToLeader(int iLeaderGap, unsigned int param_2, unsigned char param_3, int param_4, int param_5, void *param_6, unsigned int param_7, char *pcNegPosSymbol, int param_9);

void PrepareFormatGapToLeader(int iLeaderGap, unsigned char param_2, unsigned int param_3, unsigned char param_4, int param_5, int param_6, void *param_7, unsigned int param_8, BOOL bIsAhead, int param_10);

void StageUI_ClearRaceEndLatch(void);
void StageUI_SetRaceEndPending(void);
int StageUI_GetRaceEndState(void);
void StageUI_ClearRaceEndState(void);
void StageUI_RecordRaceEndEvent(char bFlag);
void StageUI_ResetRaceEndEventCount(void);
BYTE StageUI_GetRaceEndEventCount(void);
int StageUI_GetRaceResultValue(void);
BYTE *StageUI_GetRaceResultTable(void);


// Race/stage-UI tables 0x537568..0x537dcc (defined in Race.cpp): per-car
// surface sound tables, then the per-car stage sound rows (0xb4 bytes each).
// They are separate objects: MSVC6 only keeps a loop bound such as
// g_stageSoundCount in a register across stores into g_stageSoundUsed when it
// can tell the two apart, as the original does.
extern int g_surfacePrevD[8];        // 0x537568
extern int g_surfaceVolA[8];         // 0x537588
struct StageSoundBank {
    BYTE loaded;                     // 0x5375a8: surface samples loaded so far
    int first;                       // 0x5375ac: first surface sample slot
};
extern StageSoundBank g_stageSoundBank;
#define g_stageSoundLoaded (g_stageSoundBank.loaded)
#define g_stageSoundFirst (g_stageSoundBank.first)
extern int g_surfacePrevB[8];        // 0x5375b0
extern unsigned int g_stageSoundCount; // 0x5375d0
extern BYTE g_stageSoundUsed[0x1f];  // 0x5375d4: sound groups in use
extern BYTE g_unk0x005375f4[8];      // 0x5375f4: surface per car
extern int g_surfacePrevA[8];        // 0x5375fc
extern int g_surfaceSlotMax;         // 0x53761c
extern int g_surfaceVolB[8];         // 0x537620
extern int g_surfaceVolC[8];         // 0x537640
extern int g_unk0x00537660;          // 0x537660
extern int g_unk0x00537664;          // 0x537664
extern int g_surfaceVolD[8];         // 0x537668
extern int g_surfacePrevC[8];        // 0x537788

// Stage sounds of each car (rows of 0xb4 bytes starting at 0x5377c4).
struct CarSoundSet {
    int handle[10];         // 0x00 playing sound handle (-1 none)
    int id[10];             // 0x28 sound id
    int pitch[10];          // 0x50 random pitch
    BYTE surface[10];       // 0x78 surface when started
    BYTE field_0x82[0x32];
};


// Stage sound states; each can redirect to a shared pattern (0x1c bytes).
struct StageSoundPattern {
    char choices[4];
    char redirect;
    BYTE pad[3];
    int count;
    int base[4];
};
extern StageSoundPattern g_stageSoundPatterns[31];
short Surface_GetMappedIndex(short index);

// Per-car stage sound state (rows of 0xb4 bytes starting at 0x5377a8, inside
// g_carSoundStates). The slot arrays are the same memory as CarSoundSet above.
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
typedef char RaceCarSoundStateSizeCheck[sizeof(RaceCarSoundState) == 0xb4 ? 1 : -1];
extern RaceCarSoundState g_carSoundStates[8];                     // 0x5377a8
#define g_carSoundSets ((CarSoundSet *)((BYTE *)g_carSoundStates + 0x1c)) // 0x5377c4

// Engine sample used while no stage sound group is running.
extern int g_wheelSlipVolume[8][4];                              // 0x537d48
extern int g_unk0x00537dc8;                                       // 0x537dc8

#endif

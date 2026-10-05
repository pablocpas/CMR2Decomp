#ifndef _GAME_H
#define _GAME_H

#include <windows.h>
#include "../third_party/dx7sdk-7001/include/dplay.h"
#include "../third_party/dx7sdk-7001/include/dplobby.h"

typedef void (*FuncTableEntry)(struct Unk0049c2c0 *, BYTE);
typedef void (*OtherFuncTableEntry)(struct Unk0049c2c0 *, BYTE);

struct Unk00817d98
{
    union {
        int field0x1;
        struct {
            unsigned int state : 8;
            unsigned int value : 8;
            unsigned int rule : 8;
            unsigned int level : 2;
            unsigned int reserved : 6;
        } bits;
    };
    int field0x2;
};

struct FuncTableGroup
{
    FuncTableEntry func1;
    OtherFuncTableEntry func2;
};

// The callback cursor is read as a DWORD and updated through its low byte.
union CallbackIndex {
    BYTE index;
    DWORD packed;
};

struct Unk0049c2c0
{
    BYTE count;
    BYTE pad[3];                     // is this meant to be something?
    Unk00817d98 *unk;                // pointer to an 8 byte struct?
    FuncTableGroup *funcLookupTable; // pointer to the function table which points to functions like initializegame
    void *unk2;                      // pointer to something that is 64 bytes?
};

struct Unk0x005a1820 {
    char field_0x0[100];
    char field_0x64[100];
    unsigned int field_0xc8;
    unsigned int field_0xcc;
};

// GLOBAL: CMR2 0x00511a28
// CLSID_DirectPlay

// GLOBAL: CMR2 0x00511a18
// IID_IDirectPlay4A

// GLOBAL: CMR2 0x00511ae8
// CLSID_DirectPlayLobby

// GLOBAL: CMR2 0x00511ad8
// IID_IDirectPlayLobby3A

struct DPlayConnection {
    char name[256];
    void *pConnection;
    GUID guidSP;
};

extern BYTE g_unk0x005a0068[0x50];

// GLOBAL: CMR2 0x00511a38
// DPSPGUID_IPX

class CGame
{
public:
    static BOOL IsActive(void);
    static int GetCallbackCount(void);
    static void UnwindCallbacks(int count);
    static void SkipNextCallbackRenderPass(void);
    static int GetDrawnMeshTriangleCount(void);
    static int GetDrawnOverlayTriangleCount(void);
    static void SetObjectRenderMode(int param1);
    static int GetObjectRenderMode(void);
    static void QueuePrimaryDrawObject(void *param1);
    static void QueueSecondaryDrawObject(void *param1);
    static void SetSectorDrawState(int param1);
    static int GetSectorDrawState(void);

    // GLOBAL: CMR2 0x0059ce14
    static int m_unk0x0059ce14;
    // GLOBAL: CMR2 0x0059ce18
    static int m_unk0x0059ce18;
    static int m_unk0x0059ce1c;
    // GLOBAL: CMR2 0x0059ce20
    static int m_unk0x0059ce20;
    // GLOBAL: CMR2 0x0059ce28
    static int m_unk0x0059ce28;
    // GLOBAL: CMR2 0x0059ce2c
    static int m_unk0x0059ce2c;
    // The list ends at 0x593cb0 + 4096 * 4: the original's frame stamps
    // (0x597cb0/0x597cb4) are compiler statics, so they cannot be list slots.
    // GLOBAL: CMR2 0x00593cb0
    static void *m_unk0x00593cb0[4096];
    // GLOBAL: CMR2 0x00597d04
    static void *m_unk0x00597d04[4096];
    // GLOBAL: CMR2 0x005207f8
    static int m_unk0x005207f8;
    static void SetGameInputFocusState(int param1);
    static int GetGameInputFocusState(void);
    static bool CreateDirectPlay(void);
    static bool CreateDirectPlayLobby(void);
    static void ClearConnections(void);
    static void AddConnection(char *name, void *pConnection, unsigned int size, GUID *pGuidSP);
    static int __cdecl CompareConnections(const void *a, const void *b);
    static unsigned int GetConnectionCount(void);
    static DPlayConnection *GetConnection(BYTE index);
    static bool RejectUnsupportedNetworkOperation(BYTE param1, int param2, int param3);

    // GLOBAL: CMR2 0x00663dc4
    static int m_unk0x00663dc4;
    // GLOBAL: CMR2 0x00664650
    static DPlayConnection m_connections[10];
    // GLOBAL: CMR2 0x00665118
    static BYTE m_maxConnections;
    // GLOBAL: CMR2 0x00665119
    static BYTE m_connectionCount;
    static void SetShouldExit(void);
    static BOOL FUN_004d0780(void);
    static int GetFrontendResourceMode(void);
    static void RunStateUpdateCallbacks(Unk0049c2c0 *param1);
    static void RunStateRenderCallbacks(Unk0049c2c0 *param1);
    static void AdvanceCallbackStateTimers(Unk0049c2c0 *param1);
    static void InitializeCallbackStateRecord(Unk00817d98 *param1, int param2, int param3);
    static void InitializeCallbackStateMachine(Unk0049c2c0 *p1, BYTE count, Unk00817d98 *unk, FuncTableGroup *funcLookupTable, void *unk2);
    static void InitializeGame(Unk0049c2c0 *p1, BYTE p2);
    static BOOL UpdateSecondaryCallbackMachine();
    static BOOL FUN_0041b060();
    static void NoOpSecondaryStateCallback(struct Unk0049c2c0 *, BYTE);
    static int FUN_0049c1c0(Unk0049c2c0 *p, BYTE index, BYTE value, int level);
    static BYTE GetConfigurationStateByte(void);
    static void SetProfileSelectionState(BYTE param1);
    static void SetSecondaryOptionStateByte(BYTE param1);
    static bool ConsumeOptionRefreshRequest(void);
    static int RegisterCallback(void *param1, void *param2);
    static void UpdateActiveSoundSlots(void);
    static void SaveRaceCallbackDepth(void);
    static void RegisterNetworkResourceRelease(void);
    static void FreeNetworkReceiveBuffer(void);
    static void ReleaseNetworkReceiveBuffer(void);
    static void ResetSessionPlayerTable(bool param1);
    static BOOL DestroyLocalNetworkPlayer(void);
    static void FreeServiceProviderConnections(void);
    static bool Cleanup(void);
    static void DestroyDirectPlay(void);
    static void DestroyDirectPlayLobby(void);
    static IDirectPlay4A *GetDirectPlay(void);
    static bool LoadAndInitializeSplashScreens(bool param1);
    static bool InitializeNetworkSubsystem(void);
    static void FUN_004e2e50(void);
    static void SetStartupFlag(void);
    static void SetFrontendResourceMode(int param1);
    
    // GLOBAL: CMR2 0x00663db8
    static BOOL m_shouldExit;
    // GLOBAL: CMR2 0x00663db4
    static BOOL m_isActive;
    // GLOBAL: CMR2 0x00523c58
    static int m_unk0x00523c58;
    // GLOBAL: CMR2 0x00523c5c
    static int m_unk0x00523c5c;

// GLOBAL: CMR2 0x0052ea4c
    static int m_unk0x0052ea4c;
    // GLOBAL: CMR2 0x0052ea51
    static BYTE m_unk0x0052ea51;
    // GLOBAL: CMR2 0x00817eb0
    static bool m_unk0x00817eb0;
    // GLOBAL: CMR2 0x00817da0
    static Unk0049c2c0 m_unk0x00817da0;
    // GLOBAL: CMR2 0x00817d98
    static Unk00817d98 m_unk0x00817d98;
    // State transition rules of the grouped callback machine (FUN_0049c1c0):
    // byte 0 = current state, byte 1 = match (0xff = any), byte 2 = next
    // state, byte 3 = level. Terminated by 0xffffffff.
    // GLOBAL: CMR2 0x00523c18
    static unsigned int m_unk0x00523c18[16];
    // GLOBAL: CMR2 0x00593cac
    static BYTE m_unk0x00593cac;

    // GLOBAL: CMR2 0x00593ba4
    static Unk00817d98 *m_unk0x00593ba4;
    // GLOBAL: CMR2 0x00593ba8
    static CallbackIndex m_unk0x00593ba8;

    // GLOBAL: CMR2 0x00523bc8
    static FuncTableGroup m_initializeGameGroupedFuncTable[10];

    // GLOBAL: CMR2 0x00523d68
    static BYTE m_unk0x00523d68;
    // GLOBAL: CMR2 0x008180f9
    static BYTE m_unk0x008180f9;
    // GLOBAL: CMR2 0x008180fc
    static BYTE m_unk0x008180fc;
    // GLOBAL: CMR2 0x00516120
    static BYTE m_unk0x00516120;
    // GLOBAL: CMR2 0x00531768
    static BYTE m_unk0x00531768;
    // GLOBAL: CMR2 0x0052ea58
    static BYTE m_unk0x0052ea58;
    // GLOBAL: CMR2 0x0052ea59
    static BYTE m_unk0x0052ea59;

    static void *m_callbacks[64];
    static int m_unk0x00593ba0;

    // GLOBAL: CMR2 0x005a1818
    static BYTE m_unk0x005a1818;

    // GLOBAL: CMR2 0x005a1819
    static BYTE m_unk0x005a1819;

    // GLOBAL: CMR2 0x005a1820
    static Unk0x005a1820 m_unk0x005a1820[7];
    
    // GLOBAL: CMR2 0x005a1e34
    static int m_unk0x005a1e34;

    // GLOBAL: CMR2 0x005a1fc0
    static bool m_unk0x005a1fc0;

    // GLOBAL: CMR2 0x0066521c
    static IDirectPlay4A *m_pDirectPlay4A;

    // GLOBAL: CMR2 0x005a1ea0
    static DPID m_unk0x005a1ea0;

    // GLOBAL: CMR2 0x00665220
    static IDirectPlayLobby3A *m_pDirectPlayLobby3A;

    // GLOBAL: CMR2 0x005a1fb8 
    static void *m_unk0x005a1fb8;    

    // GLOBAL: CMR2 0x005a1fbc
    static BOOL m_unk0x005a1fbc;

    
    
};

#endif

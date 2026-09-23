#ifndef _INPUT_H
#define _INPUT_H

#include "../third_party/dx7sdk-7001/include/dinput.h"

struct JoystickBinding {
    DWORD range;
    DWORD deadzone;
    DWORD saturation;
    int field_0xc;
    BOOL field_0x10;
};

struct JoystickInfo {
    DWORD controlCount; // or button count?
    DWORD field_0x4;
    JoystickBinding bindings[7];
};

struct KeyboardInfo {
    BYTE field_0x468;
    BYTE field_0x469;
    BYTE field_0x46a;
    BYTE field_0x46b;
    BYTE field_0x46c;
    BYTE field_0x46d;
    BYTE field_0x46e;
    BYTE field_0x46f;
    BYTE field_0x470;
    BYTE field_0x471;
    BYTE field_0x472;
    BYTE field_0x473;
    BYTE field_0x474;
    BYTE field_0x475;
    BYTE field_0x476;
    BYTE field_0x477;
};

struct DeviceInfo {
    DWORD field_0x0;        // device type: 0/3 joystick, 1 keyboard, 2 mouse
    unsigned int field_0x4; // buttons held
    unsigned int field_0x8; // buttons pressed this frame
    unsigned int field_0xc; // direction buttons auto-repeated this frame
    int field_0x10;         // next auto-repeat time
    int field_0x14;
    int field_0x18;
    int field_0x1c;
    BYTE pad2[92];
    char deviceInstanceName[MAX_PATH];
    char deviceProductName[MAX_PATH];
    CHAR field_0x284[8][20];
    BYTE pad4[240];
    CHAR field_0x414[20];
    CHAR field_0x425[20];
    CHAR field_0x436[20];
    CHAR field_0x447[20];
    BOOL unk_isJoystick; // not 100% sure on this
    union {
        KeyboardInfo keyboard;
        JoystickInfo joystick;
    };
    BYTE pad6[16]; // bindings[7].range/deadzone/saturation/field_0xc overlap this (struct is 0x50c bytes)
};

struct Unk0x0059f8cc {
    BYTE field_0x0;
    BYTE field_0x1;
    BYTE field_0x2;
    BYTE field_0x3;
};

struct ForceFeedbackDevice {
    BOOL field_0x0;              // +0x0 (active flag)
    LPDIRECTINPUTDEVICE7A device; // +0x4
    BYTE field_0x8;              // +0x8 (flag cleared at end)
    BYTE padding1[3];            // +0x9-0xB
    LPDIRECTINPUTEFFECT effects[10];       // +0xC (10 effect pointers)
};

struct ControllerDataUnk0x210 {
    DWORD field_0x0;
    BYTE field_0x4[16];
};

struct ControllerData {
    unsigned int field_0x0;
    unsigned int field_0x4;
    char name[MAX_PATH];
    DWORD field_0x10c;
    DWORD field_0x110;
    DWORD field_0x114;
    DWORD field_0x118;
    DWORD field_0x11c;
    DWORD field_0x120;
    DWORD field_0x124;
    unsigned short field_0x128; // are you actually a struct?
    unsigned short field_0x12a;
    unsigned short field_0x12c;
    unsigned short field_0x12e;
    unsigned short field_0x130;
    unsigned short field_0x132;
    unsigned short field_0x134;
    unsigned short field_0x136;
    unsigned short field_0x138;
    unsigned short field_0x13a;
    unsigned short field_0x13c;
    BYTE field_0x13e[9];            // key (DIK_*) of each keyboard binding
    char keyNames[9][20];           // 0x147 text shown for each binding
    BYTE field_0x1fb[0x15];
    ControllerDataUnk0x210 field_0x210[10];
    unsigned short field_0x2d8[10];
    BYTE index;                     // 0x2ec
    BYTE field_0x2ed[3];
};

struct ControllerInfo {
    int field_0x0;
    int field_0x4;
    BYTE field_0x8;
    BYTE field_0x9;
    BYTE field_0xa_padding[308];
    BYTE field_0x13e;
    BYTE field_0x13f;
    BYTE field_0x140;
    BYTE field_0x141;
    BYTE field_0x142;
    BYTE field_0x143;
    BYTE field_0x144;
    BYTE field_0x145;
    BYTE field_0x146;
    BYTE pad[4185];
};

// GLOBAL: CMR2 0x00511908
// GUID_ConstantForce

// GLOBAL: CMR2 0x00511978
// GUID_Spring

// GLOBAL: CMR2 0x00511988
// GUID_Damper

// GLOBAL: CMR2 0x00511998
// GUID_Inertia

// GLOBAL: CMR2 0x005119a8
// GUID_Friction

// GLOBAL: CMR2 0x00511898
// GUID_SysMouse

// GLOBAL: CMR2 0x005118a8
// GUID_SysKeyboard

// GLOBAL: CMR2 0x005117c8
// IID_IDirectInputDevice7A

// GLOBAL: CMR2 0x00512e70
// c_dfDIMouse2

// GLOBAL: CMR2 0x00512e88
// c_dfDIKeyboard

// GLOBAL: CMR2 0x00512ea0
// c_dfDIJoystick2

class CInput {
public:
    static IDirectInput7A *m_lpDirectInput7;
    static Unk0x0059f8cc m_unk0x0059f8cc;
    static char m_strKeyboard[12];
    static DeviceInfo m_availableDevices[8];
    static PVOID m_keyboardDelay;
    static DWORD m_mouseGranularity;
    static PVOID m_keyboardSpeed;

    static LPDIRECTINPUTDEVICEA m_pDirectInputKeyboard;
    static LPDIRECTINPUTDEVICEA m_pDirectInputMouse;
    

    
    static USHORT m_unk0x00511400[8];
    static LPDIRECTINPUTDEVICEA m_unk0x0059f6b0[8];
    static CHAR m_strD[4];
    static CHAR m_strU[4];
    static CHAR m_strR[4];
    static CHAR m_strL[4];
    static ForceFeedbackDevice m_forceFeedbackDevices[8];
    static BOOL m_unk0x00666ee8;
    // GLOBAL: CMR2 0x00666ec8
    static DWORD m_unk0x00666ec8[8];
    // GLOBAL: CMR2 0x00667000
    static char m_formatBuffer[512];
    // GLOBAL: CMR2 0x0059f7c8
    static BYTE m_keyboardState[256];
    // GLOBAL: CMR2 0x00520808
    static DWORD m_buttonMasks[24];
    // GLOBAL: CMR2 0x0059f8f0
    static DWORD m_unk0x0059f8f0;
    // GLOBAL: CMR2 0x0059f8f4
    static DWORD m_unk0x0059f8f4;
    // GLOBAL: CMR2 0x0059f8f8
    static DWORD m_unk0x0059f8f8;
    // GLOBAL: CMR2 0x0059f900
    static DWORD m_unk0x0059f900;
    // GLOBAL: CMR2 0x0059f90c
    static DWORD m_unk0x0059f90c;
    // GLOBAL: CMR2 0x0059f910
    static DWORD m_unk0x0059f910;

    // GLOBAL: CMR2 0x00516908
    static char m_strControllerInfoDir[32];

    // GLOBAL: CMR2 0x005334f0
    static BOOL m_hasLoadedControllerInfo;

    // GLOBAL: CMR2 0x00532250
    static ControllerData m_controllerInfo[6];

    // GLOBAL: CMR2 0x005168f4
    static unsigned short m_unk0x005168f4[8];

    static BOOL DInputCreate(void);
    static LPDIRECTINPUTDEVICEA DInputCreateDevice(REFGUID param1, LPCDIDATAFORMAT pDataFormat);
    static BOOL DInputRelease(void);
    static int FUN_0049ef90();
    static BOOL FUN_0049f6b0(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef);
    static BOOL SetupKeyboard(void);
    static void SetupMouse(void);
    static void SetMouseCoopLevel(BOOL param1);
    static BOOL GetAttachedJoysticks(void);
    static BOOL SetupJoystick(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef);
    static void SetupJoystickDeviceInfo(DeviceInfo *deviceStruct);
    static void SetJoystickAxisRange(int param1, int param2, DWORD range);
    static void SetJoystickAxisDeadzone(int deviceID, int axisID, DWORD deadzone);
    static void SetJoystickAxisSaturation(int deviceID, int axisID, DWORD saturation);
    static BOOL FUN_004aae20(int deviceID, LPDIRECTINPUTDEVICE7 pDevice);
    static bool FUN_004ab5f0(HRESULT hr);
    static void SetForceFeedbackAutocenter(DWORD param1, int deviceID);
    static void ResetForceFeedbackEffects(void);
    static BOOL ResetForceFeedbackEffectsAlt(void);
    static void DInputReleaseDevices(void);
    static void LoadControllerInfo(void);
    static void FUN_0040be90(unsigned int param1);
    static void FUN_0040c440(unsigned int param1, ControllerData * param2);
    static void FUN_0049eb90(int param1, unsigned int param2, unsigned int param3);
    static DeviceInfo *FUN_0049ead0(int index);
    static BYTE FUN_0040c530(unsigned int param1);
    static void FUN_0049eb50(void);
    static int GetFirstPressedKey(void);
    static BOOL IsShiftPressed(void);
    static void FUN_0049efc0(void);
    static void FUN_004b7ca0(int param1);
    static void FUN_004b7d10(unsigned int param1);
    static void FUN_0040af20(void);
    static void FUN_0049eab0(void);
    static void FUN_0040bc90(int param1, DWORD param2);
    static void ReadKeyboardState(void);
    static int GetButtonIndexFromMask(unsigned int mask);
    static void FUN_0049ff80(DWORD p1, DWORD p2, DWORD p3, DWORD p4, DWORD p5);
    static void FUN_0049ffc0(DWORD param1);
    static DWORD FUN_0040be00(unsigned int param1);
    static DWORD FUN_0040be30(unsigned int param1);
    static DWORD FUN_0040be60(unsigned int param1);
    static unsigned int FUN_0040c210(unsigned int param1, int param2);
    static BOOL FUN_0040c270(int param1, ControllerData *param2);
    static void FUN_004aaf50(DWORD param1, int index);
    static void StartForceFeedbackEffect(int effectIndex, int deviceIndex);
    static char *FormatString(LPCSTR format, ...);
    static void ReadMouse(DeviceInfo *pDevice);
    static void ReadKeyboardDevice(DeviceInfo *pDevice);
    static void ReadJoystick(DeviceInfo *pDevice);
    static void FUN_0040c130(unsigned short *values, int index, unsigned short value);
    static void FUN_0040c550(BYTE *values, int index, BYTE value);
    static DeviceInfo *UpdateDevice(int index);
    static int CreateForceFeedbackEffect(int effectType, DWORD duration, LONG coefficient, LONG offset, int triggerButton, int deviceIndex);
    static int CreateSpringEffect(DWORD duration, LONG coefficient, LONG offset, int triggerButton, int deviceIndex);
    static int CreateDamperEffect(DWORD duration, LONG coefficient, LONG offset, int triggerButton, int deviceIndex);
    static short GetButtonMapping(unsigned short controller, int button);
    static HRESULT SetEffectGain(int effectIndex, DWORD gain, int deviceIndex);
    static HRESULT SetEffectGainAndDirection(int effectIndex, DWORD gain, LONG direction, int deviceIndex);
    static int CreateConstantForceEffect(DWORD duration, LONG direction, LONG magnitude, DWORD attackTime, DWORD attackLevel, DWORD fadeTime, DWORD fadeLevel, int triggerButton, int deviceIndex);
    static HRESULT SetConditionCoefficient(int effectIndex, LONG coefficient, int deviceIndex);
    static void SaveControllerInfo(void);
    static void FUN_0040c050(void);
    static void FUN_0040c610(DeviceInfo *pDevice, int index);

    // GLOBAL: CMR2 0x00516904
    static unsigned short m_controllerCount;

    // GLOBAL: CMR2 0x00520868
    static unsigned int m_directionButtonMask;
    // GLOBAL: CMR2 0x0052086c
    static BOOL m_unk0x0052086c;
    // GLOBAL: CMR2 0x00665328
    static DIEFFECT m_forceFeedbackEffects[80];
    // GLOBAL: CMR2 0x006664a8
    static DICONDITION m_forceFeedbackConditions[80];
};

#endif

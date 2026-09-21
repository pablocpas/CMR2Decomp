#include "Input.h"
#include "GameInfo.h"
#include "Graphics.h"
#include "main.h"
#include "InstallInfo.h"
#include "Frontend.h"
#include "FileBuffer.h"
#include "Game.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// GLOBAL: CMR2 0x00511758
// IID_IDirectInput7A

// GLOBAL: CMR2 0x0059f8c8
IDirectInput7A* CInput::m_lpDirectInput7;

// GLOBAL: CMR2 0x00520874
char CInput::m_strKeyboard[12] = "Keyboard";

// GLOBAL: CMR2 0x0059f8cc
Unk0x0059f8cc CInput::m_unk0x0059f8cc; // device count or something

// GLOBAL: CMR2 0x0059ce48
DeviceInfo CInput::m_availableDevices[8];

// GLOBAL: CMR2 0x0059f8d4
DWORD CInput::m_mouseGranularity;

// GLOBAL: CMR2 0x0059f8d8
PVOID CInput::m_keyboardDelay;

// GLOBAL: CMR2 0x0059f8dc
PVOID CInput::m_keyboardSpeed;

// GLOBAL: CMR2 0x0059f6a8
LPDIRECTINPUTDEVICEA CInput::m_pDirectInputKeyboard = NULL;
// GLOBAL: CMR2 0x0059f7c4
LPDIRECTINPUTDEVICEA CInput::m_pDirectInputMouse = NULL;

// GLOBAL: CMR2 0x00511400
USHORT CInput::m_unk0x00511400[8] = {0, 4, 8, 12, 16, 20, 24, 28};

// GLOBAL: CMR2 0x0059f6b0
LPDIRECTINPUTDEVICEA CInput::m_unk0x0059f6b0[8];

// GLOBAL: CMR2 0x00520880
CHAR CInput::m_strD[4] = " D";
// GLOBAL: CMR2 0x00520884
CHAR CInput::m_strU[4] = " U";
// GLOBAL: CMR2 0x00520888
CHAR CInput::m_strR[4] = " R";
// GLOBAL: CMR2 0x0052088C
CHAR CInput::m_strL[4] = " L";

// GLOBAL: CMR2 0x00666d28
ForceFeedbackDevice CInput::m_forceFeedbackDevices[8];

// GLOBAL: CMR2 0x00666ee8
BOOL CInput::m_unk0x00666ee8 = FALSE;
DWORD CInput::m_unk0x00666ec8[8];
char CInput::m_formatBuffer[512];
BYTE CInput::m_keyboardState[256];
DWORD CInput::m_buttonMasks[24] = {
    0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000,
    0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000, 0x1, 0x2, 0x4, 0x8
};
unsigned int CInput::m_directionButtonMask = 0xf;
BOOL CInput::m_unk0x0052086c = TRUE;
DIEFFECT CInput::m_forceFeedbackEffects[80];
DICONDITION CInput::m_forceFeedbackConditions[80];
DWORD CInput::m_unk0x0059f8f0;
DWORD CInput::m_unk0x0059f8f4;
DWORD CInput::m_unk0x0059f8f8;
DWORD CInput::m_unk0x0059f900;
DWORD CInput::m_unk0x0059f90c;
DWORD CInput::m_unk0x0059f910;

unsigned short CInput::m_controllerCount = 1;
char CInput::m_strControllerInfoDir[32] = "%s\\Configuration\\Controller.rcf";
BOOL CInput::m_hasLoadedControllerInfo;
ControllerData CInput::m_controllerInfo[6];
unsigned short CInput::m_unk0x005168f4[8] = {0, 1, 2, 3, 0, 0, 0, 0};

// FUNCTION: CMR2 0x0049fd30
BOOL CInput::DInputCreate(void) {
    DirectInputCreateEx(CMain::m_hInstance, 0x700, IID_IDirectInput7A, (LPVOID*)&CInput::m_lpDirectInput7, NULL);
    CGame::RegisterCallback(DInputRelease, NULL);
    return TRUE;
}

// FUNCTION: CMR2 0x0049fe30
LPDIRECTINPUTDEVICEA CInput::DInputCreateDevice(REFGUID guid, LPCDIDATAFORMAT pDataFormat) {
    LPDIRECTINPUTDEVICEA pDevice = NULL;
    LPDIRECTINPUTDEVICE7A pOtherDevice = NULL;
    HRESULT h1, h2, h3, h4;
    ULONG refCount;
    
    h1 = m_lpDirectInput7->CreateDevice(guid, &pDevice, NULL);
    if (SUCCEEDED(h1)) {
        h2 = pDevice->QueryInterface(IID_IDirectInputDevice7A, (LPVOID*)&pOtherDevice);
        if (pOtherDevice != NULL) {
            refCount = pOtherDevice->Release();
            if (refCount == 0)
                pOtherDevice = NULL;
        }
      
        if (SUCCEEDED(h2)) {
            h3 = pDevice->SetDataFormat(pDataFormat);
            if (FAILED(h3)) {
                if (pDevice != NULL) pDevice->Release();
                return NULL;
            }
            
            if (IsEqualGUID(guid, GUID_SysKeyboard)) {
                h4 = pDevice->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);  // 10
            } else if (IsEqualGUID(guid, GUID_SysMouse)) {
                if (g_pGraphics->isFullscreen) {
                    h4 = pDevice->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], DISCL_EXCLUSIVE | DISCL_FOREGROUND);  // 5
                } else {
                    h4 = pDevice->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);  // 6
                }
            } else {
                h4 = pDevice->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], 9);
            }
            
            // if we got a cooplevel success, return it
            if (FAILED(h4)) {
                if (pDevice != NULL) pDevice->Release();
                return NULL;
            }

            return pDevice;
        }
    }
    return NULL;
}

// FUNCTION: CMR2 0x0049fd60
BOOL CInput::DInputRelease(void) {
    DInputReleaseDevices();

    if (m_lpDirectInput7 != NULL) {
        ULONG result = m_lpDirectInput7->Release();
        if (result == 0)
            m_lpDirectInput7 = NULL;
    }

    return TRUE;
};

// FUNCTION: CMR2 0x0049fd90
void CInput::DInputReleaseDevices(void) {
    HRESULT hr;
    ULONG result;
    int iVar2 = 0;

    if (m_pDirectInputKeyboard != NULL) {
        m_pDirectInputKeyboard->Unacquire();
        if (m_pDirectInputKeyboard != NULL) {
            result = m_pDirectInputKeyboard->Release();
            if (result == 0) m_pDirectInputKeyboard = NULL;
        }
    }

    if (m_pDirectInputMouse != NULL) {
        m_pDirectInputMouse->Unacquire();
        if (m_pDirectInputMouse != NULL) {
            result = m_pDirectInputMouse->Release();
            if (result == 0) m_pDirectInputMouse = NULL;
        }
    }

    FUN_0049ef90();
    
    iVar2 = 0;
    LPDIRECTINPUTDEVICEA *pDevices = m_unk0x0059f6b0;

    do {
        if (pDevices[iVar2] != NULL) {
            hr = pDevices[iVar2]->Unacquire();
            if (SUCCEEDED(hr) && iVar2 < m_unk0x0059f8cc.field_0x3) {
                pDevices[iVar2] = pDevices[iVar2];
                if (pDevices[iVar2] != NULL) {
                    result = pDevices[iVar2]->Release();
                    if (result == 0)
                        pDevices[iVar2] = NULL;
                }
            }
        }

        iVar2++;
    } while (pDevices < pDevices + 4);
}

// FUNCTION: CMR2 0x0049ef90
int CInput::FUN_0049ef90(void) {
  m_unk0x0059f8cc.field_0x3 = 0;
  m_lpDirectInput7->EnumDevices(DIDEVTYPE_JOYSTICK, FUN_0049f6b0, NULL, DIEDFL_ATTACHEDONLY);
  return m_unk0x0059f8cc.field_0x1 + m_unk0x0059f8cc.field_0x3;
}

// FUNCTION: CMR2 0x0049f6b0
BOOL CInput::FUN_0049f6b0(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef) {
    m_unk0x0059f8cc.field_0x3++;
    return TRUE;
}

// FUNCTION: CMR2 0x0049f0e0
BOOL CInput::SetupKeyboard(void) {
    unsigned int uVar1;
    BOOL bVar2;
    int iVar3 = 0;

    do {
        uVar1 = m_unk0x0059f8cc.field_0x0;
        m_availableDevices[uVar1].field_0x0 = 1;
        m_availableDevices[uVar1].field_0x18 = FALSE;

        sprintf(m_availableDevices[uVar1].deviceInstanceName, m_strKeyboard);
        sprintf(m_availableDevices[m_unk0x0059f8cc.field_0x0].deviceProductName, m_strKeyboard);
        
        uVar1 = m_unk0x0059f8cc.field_0x0;
        m_availableDevices[uVar1].unk_isJoystick = FALSE;

        if (!iVar3) {
            m_availableDevices[uVar1].keyboard.field_0x468 = 0xcb;
            m_availableDevices[uVar1].keyboard.field_0x469 = 0xcd;
            m_availableDevices[uVar1].keyboard.field_0x46a = 0xc8;
            m_availableDevices[uVar1].keyboard.field_0x46b = 0xd0;
            m_availableDevices[uVar1].keyboard.field_0x46c = 0x39;
            m_availableDevices[uVar1].keyboard.field_0x46d = 0x1b;
            m_availableDevices[uVar1].keyboard.field_0x46e = 0x1a;
            m_availableDevices[uVar1].keyboard.field_0x46f = 0x2e;
            m_availableDevices[uVar1].keyboard.field_0x470 = 0x13;
        } else {
            m_availableDevices[uVar1].keyboard.field_0x468 = 0x4b;
            m_availableDevices[uVar1].keyboard.field_0x469 = 0x4d;
            m_availableDevices[uVar1].keyboard.field_0x46a = 0x48;
            m_availableDevices[uVar1].keyboard.field_0x46b = 0x50;
            m_availableDevices[uVar1].keyboard.field_0x46c = 0x51;
            m_availableDevices[uVar1].keyboard.field_0x46d = 0xc9;
            m_availableDevices[uVar1].keyboard.field_0x46e = 0xd1;
            m_availableDevices[uVar1].keyboard.field_0x46f = 0x49;
            m_availableDevices[uVar1].keyboard.field_0x470 = 0x47;            
        }

        m_availableDevices[uVar1].keyboard.field_0x471 = 0x0;
        m_availableDevices[uVar1].keyboard.field_0x474 = 0x3b;
        m_availableDevices[uVar1].keyboard.field_0x475 = 0x3c;
        m_availableDevices[uVar1].keyboard.field_0x476 = 0x3e;
        m_availableDevices[uVar1].keyboard.field_0x477 = 0x0;
        m_availableDevices[uVar1].keyboard.field_0x472 = 0x1;
        m_availableDevices[uVar1].keyboard.field_0x473 = 0x0;

        iVar3++;
        m_unk0x0059f8cc.field_0x0 = m_unk0x0059f8cc.field_0x0 + 1;
    } while (iVar3 < 2);

    bVar2 = SystemParametersInfoA(SPI_GETKEYBOARDDELAY, 0, &m_keyboardDelay, 0);
    if (!bVar2) m_keyboardDelay = (PVOID)0x3e8;
    else {
        switch ((int)m_keyboardDelay) {
            case 0:
                m_keyboardDelay = (PVOID)0x1f4;
            break;

            case 1:
                m_keyboardDelay = (PVOID)0x2ee;
            break;

            case 2:
                m_keyboardDelay = (PVOID)0x3e8;
            break;

            case 3:
                m_keyboardDelay = (PVOID)0x4e2;
            break;
        }
    }

    bVar2 = SystemParametersInfoA(SPI_GETKEYBOARDSPEED, 0, &m_keyboardSpeed, 0);
    if (!bVar2) m_keyboardSpeed = (PVOID)0x1f4;
    else m_keyboardSpeed = (PVOID)((int)m_keyboardSpeed * -0xd + 0x1f7);

    m_pDirectInputKeyboard = DInputCreateDevice(GUID_SysKeyboard, &c_dfDIKeyboard);
    if (m_pDirectInputKeyboard != NULL)
        if (SUCCEEDED(m_pDirectInputKeyboard->Acquire()))
            return TRUE;

    return FALSE;
}

// FUNCTION: CMR2 0x0049f060
void CInput::SetupMouse(void) {
    HRESULT hr;
    DIPROPDWORD diPropDword;
    m_pDirectInputMouse = DInputCreateDevice(GUID_SysMouse, &c_dfDIMouse2);

    diPropDword.diph.dwSize = 0x14;
    diPropDword.diph.dwHeaderSize = 0x10;
    diPropDword.diph.dwObj = 0;
    diPropDword.diph.dwHow = 0;
    diPropDword.dwData = 0x10;

    m_pDirectInputMouse->SetProperty(DIPROP_BUFFERSIZE, &diPropDword.diph);
    m_pDirectInputMouse->Acquire();
    hr = m_pDirectInputMouse->GetProperty(DIPROP_GRANULARITY, &diPropDword.diph);
    if (SUCCEEDED(hr)) {
        m_mouseGranularity = diPropDword.dwData;
    }

    SetMouseCoopLevel(1);
}

// FUNCTION: CMR2 0x0049f000
void CInput::SetMouseCoopLevel(BOOL param1) {
    if (m_pDirectInputMouse != NULL) {
        m_pDirectInputMouse->Unacquire();

        if (param1 && g_pGraphics->isFullscreen) {
            m_pDirectInputMouse->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx],DISCL_EXCLUSIVE | DISCL_FOREGROUND); // 5
        } else {
            m_pDirectInputMouse->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], DISCL_NONEXCLUSIVE | DISCL_FOREGROUND); // 6
        }

        m_pDirectInputMouse->Acquire();
    }
}

// FUNCTION: CMR2 0x0049f690
BOOL CInput::GetAttachedJoysticks(void) {
    HRESULT hr;
    hr = m_lpDirectInput7->EnumDevices(DIDEVTYPE_JOYSTICK, SetupJoystick, m_lpDirectInput7, DIEDFL_ATTACHEDONLY);
    return SUCCEEDED(hr);
}

// FUNCTION: CMR2 0x0049f6d0
BOOL CInput::SetupJoystick(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef) {
    HRESULT hr;
    unsigned int uVar2;
    DeviceInfo *pDeviceInfo;
    LPDIRECTINPUTDEVICEA pDevice;
    BYTE iVar6[4];
    int iVar10, axisID, iVar11;
    JoystickBinding * joystickBinding;
    DIPROPDWORD dipd;
    DIDEVCAPS devCaps;
    DIDEVICEOBJECTINSTANCEA didoi;

    uVar2 = m_unk0x0059f8cc.field_0x0;
    pDeviceInfo = &m_availableDevices[m_unk0x0059f8cc.field_0x0];
    if (GET_DIDEVICE_TYPE(lpddi->dwDevType) == DIDEVTYPE_JOYSTICK) {
        if (GET_DIDEVICE_SUBTYPE(lpddi->dwDevType) != DIDEVTYPE_MOUSE) pDeviceInfo->field_0x0 = 3;
    
        pDevice = DInputCreateDevice(lpddi->guidInstance, &c_dfDIJoystick2);
        m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2] = pDevice;

        if (pDevice != NULL) {
            strncpy(m_availableDevices[uVar2].deviceInstanceName, lpddi->tszInstanceName, sizeof(m_availableDevices[uVar2].deviceInstanceName));
            strncpy(m_availableDevices[uVar2].deviceProductName, lpddi->tszProductName, sizeof(m_availableDevices[uVar2].deviceProductName));
        
            pDeviceInfo->field_0x18 = m_unk0x0059f8cc.field_0x2;

            SetupJoystickDeviceInfo(pDeviceInfo);

            iVar10 = 0;
            axisID = 0;

            if (pDeviceInfo->joystick.controlCount > 0) {
                joystickBinding = pDeviceInfo->joystick.bindings;
                do {
                    if (joystickBinding[-1].field_0x10 != FALSE) {
                        SetJoystickAxisRange(m_unk0x0059f8cc.field_0x0, axisID, joystickBinding->range);
                        SetJoystickAxisDeadzone(m_unk0x0059f8cc.field_0x0, axisID, joystickBinding->deadzone);
                        SetJoystickAxisSaturation(m_unk0x0059f8cc.field_0x0, axisID, joystickBinding->saturation);
                        iVar10++;
                    }

                    axisID++;
                    joystickBinding ++;
                } while (axisID < pDeviceInfo->field_0x8);
            }

            devCaps.dwSize = 0x2c;
            hr = m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2]->GetCapabilities(&devCaps);
            
            if (SUCCEEDED(hr)) {
                if ((devCaps.dwFlags & DIDC_FORCEFEEDBACK)) {
                    m_availableDevices[m_unk0x0059f8cc.field_0x2].unk_isJoystick = TRUE;
                    
                    dipd.diph.dwSize = 0x14;
                    dipd.diph.dwHeaderSize = 0x10;
                    dipd.diph.dwHow = DIPH_DEVICE;
                    dipd.dwData = 0;

                    
                    m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2]->SetProperty(DIPROP_AUTOCENTER, &dipd.diph);
                    FUN_004aae20(m_unk0x0059f8cc.field_0x0, (LPDIRECTINPUTDEVICE7)m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2]);

                } else {
                    m_availableDevices[m_unk0x0059f8cc.field_0x2].unk_isJoystick = FALSE;
                }

                iVar10 = 0;
                didoi.dwSize = 0x13c;
                m_availableDevices[uVar2].field_0x14 = 0;

                if (devCaps.dwAxes > 0) {
                    
                    do {
                        hr = m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2]->GetObjectInfo(&didoi, iVar10 + 0x30, DIPH_BYOFFSET);
                        
                        if (SUCCEEDED(hr)) {
                            strncpy(m_availableDevices[uVar2].field_0x284[iVar10], didoi.tszName, 20);
                            m_availableDevices[uVar2].field_0x14++;
                        }
                        iVar10++;
                    } while (iVar10 < devCaps.dwAxes);
                }

                if (devCaps.dwAxes > 0) {
                    iVar10 = 0;
                    didoi.dwOfs = 0x20;

                    do {
                        if (0x20 < didoi.dwOfs) break;
                        
                        hr = m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2]->GetObjectInfo(&didoi, didoi.dwOfs, DIPH_BYOFFSET);
                        if (SUCCEEDED(hr)) {
                            strncpy(m_availableDevices[uVar2].field_0x414, didoi.tszName, 0x11);
                            strcat(m_availableDevices[uVar2].field_0x414, m_strL);
                            
                            strncpy(m_availableDevices[uVar2].field_0x425, didoi.tszName, 0x11);
                            strcat(m_availableDevices[uVar2].field_0x425, m_strR);
                            
                            strncpy(m_availableDevices[uVar2].field_0x436, didoi.tszName, 0x11);
                            strcat(m_availableDevices[uVar2].field_0x436, m_strU);
                            
                            strncpy(m_availableDevices[uVar2].field_0x447, didoi.tszName, 0x11);
                            strcat(m_availableDevices[uVar2].field_0x447, m_strD);

                            m_availableDevices[uVar2].field_0x14++;
                        }

                        iVar10++;
                        didoi.dwOfs++;
                    } while (iVar10 < devCaps.dwAxes);
                }

                iVar10 = 0;

                if (m_availableDevices[uVar2].field_0x14 > 0) {
                    int * pUnk0x1c = &m_availableDevices[uVar2].field_0x1c;
                    do {
                        *pUnk0x1c++ = 1u << iVar10;
                        iVar10++;
                    } while (iVar10 < m_availableDevices[uVar2].field_0x14);
                }

                m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2]->Acquire();
                m_unk0x0059f8cc.field_0x2++;
                m_unk0x0059f8cc.field_0x0++;
            }
        }
    }

    return DIENUM_CONTINUE;
}

// FUNCTION: CMR2 0x0049fad0
void CInput::SetupJoystickDeviceInfo(DeviceInfo *deviceInfo) {
    HRESULT hr;
    USHORT * unk0x00511400;
    JoystickBinding * bindings;

    DIPROPRANGE dipd;
    dipd.diph.dwSize = 0x18;
    dipd.diph.dwHeaderSize = sizeof(DIPROPRANGE);
    dipd.diph.dwHow = DIPH_BYOFFSET;

    deviceInfo->joystick.controlCount = 0;
    unk0x00511400 = m_unk0x00511400;
    bindings = deviceInfo->joystick.bindings;

    do {
        dipd.diph.dwObj = *unk0x00511400;
        hr = m_unk0x0059f6b0[deviceInfo->field_0x18]->GetProperty(DIPROP_RANGE, &dipd.diph);

        if (SUCCEEDED(hr)) {
            bindings[-1].field_0x10 = TRUE; // esentially just deviceInfo->unk_isJoystick but it doesnt match
            bindings->deadzone = 0xc8;
            bindings->range = 0x10000;
            bindings->saturation = 0x2710;

            deviceInfo->joystick.controlCount ++;
        }
        else {
            bindings[-1].field_0x10 = FALSE;
        }

        unk0x00511400++;
        bindings++;
    } while ((int)unk0x00511400 < (int)(m_unk0x00511400 + 8));
}

// FUNCTION: CMR2 0x0049ee10
void CInput::SetJoystickAxisRange(int deviceID, int axisID, DWORD range) {
    USHORT * unk0x00511400;
    DeviceInfo * pDeviceInfo = &m_availableDevices[deviceID];

    DIPROPRANGE dipd;
    dipd.diph.dwSize = 0x18;
    dipd.diph.dwHeaderSize = 0x10;
    dipd.diph.dwHow = DIPH_BYOFFSET;
    dipd.lMax = range;
    dipd.lMin = -range;
    dipd.diph.dwObj = m_unk0x00511400[axisID];

    m_unk0x0059f6b0[pDeviceInfo->field_0x18]->SetProperty(DIPROP_RANGE, &dipd.diph);
    pDeviceInfo->joystick.bindings[axisID].range = range;
}

// FUNCTION: CMR2 0x0049ee90
void CInput::SetJoystickAxisDeadzone(int deviceID, int axisID, DWORD deadzone) {
    USHORT * unk0x00511400;
    DeviceInfo * pDeviceInfo = &m_availableDevices[deviceID];

    DIPROPDWORD dipd;
    dipd.diph.dwSize = 0x14;
    dipd.diph.dwHeaderSize = 0x10;
    dipd.diph.dwHow = DIPH_BYOFFSET;
    dipd.dwData = deadzone;
    dipd.diph.dwObj = m_unk0x00511400[axisID];

    m_unk0x0059f6b0[pDeviceInfo->field_0x18]->SetProperty(DIPROP_DEADZONE, &dipd.diph);
    pDeviceInfo->joystick.bindings[axisID].deadzone = deadzone;
}

// FUNCTION: CMR2 0x0049ef10
void CInput::SetJoystickAxisSaturation(int deviceID, int axisID, DWORD saturation) {
    USHORT * unk0x00511400;
    DeviceInfo * pDeviceInfo = &m_availableDevices[deviceID];

    DIPROPDWORD dipd;
    dipd.diph.dwSize = 0x14;
    dipd.diph.dwHeaderSize = 0x10;
    dipd.diph.dwHow = DIPH_BYOFFSET;
    dipd.dwData = saturation;
    dipd.diph.dwObj = m_unk0x00511400[axisID];

    m_unk0x0059f6b0[pDeviceInfo->field_0x18]->SetProperty(DIPROP_SATURATION, &dipd.diph);
    pDeviceInfo->joystick.bindings[axisID].saturation = saturation;
}

// FUNCTION: CMR2 0x004aae20
BOOL CInput::FUN_004aae20(int deviceID, LPDIRECTINPUTDEVICE7 pDevice) {
    Graphics *pGraphics;
    HRESULT hr;

    pGraphics = g_pGraphics;
    if (m_forceFeedbackDevices[deviceID].field_0x0 == 0) {
        m_forceFeedbackDevices[deviceID].device = (LPDIRECTINPUTDEVICE7A)pDevice;
        if (pGraphics->isFullscreen) {
            hr = pDevice->SendForceFeedbackCommand(DISFFC_STOPALL);
            FUN_004ab5f0(hr);
        }

        SetForceFeedbackAutocenter(0, deviceID);
        m_forceFeedbackDevices[deviceID].field_0x0 = TRUE;
        ResetForceFeedbackEffects();
    }

    if (m_unk0x00666ee8 == FALSE) {
        m_unk0x00666ee8 = TRUE;
        CGame::RegisterCallback(ResetForceFeedbackEffectsAlt, NULL);
    }

    return TRUE;
}

// FUNCTION: CMR2 0x004ab5f0
bool CInput::FUN_004ab5f0(HRESULT hr) {
    return hr == 0;
}

// FUNCTION: CMR2 0x004aafd0
void CInput::SetForceFeedbackAutocenter(DWORD param1, int deviceID) {
    DIPROPDWORD dipdw;

    if (m_forceFeedbackDevices[deviceID].field_0x0 != FALSE) {
        dipdw.dwData = param1;
        dipdw.diph.dwSize = 0x14;
        dipdw.diph.dwHeaderSize = 0x10;
        dipdw.diph.dwObj = 0;
        dipdw.diph.dwHow = 0;

        m_forceFeedbackDevices[deviceID].device->SetProperty(DIPROP_AUTOCENTER, &dipdw.diph);
    }
}

// 96.55% match, only concern is this
// 0x4aaf35	-cmp edi, 0x666ed4
//          +cmp edi, CInput::m_dinputRefGuidKeyboard (DATA) (Input.cpp:560) <-- why  are you that
// FUNCTION: CMR2 0x004aaf00
void CInput::ResetForceFeedbackEffects(void) {
    LPDIRECTINPUTEFFECT* pEffects = m_forceFeedbackDevices[0].effects;
    
    do {
        ForceFeedbackDevice* pDevice = (ForceFeedbackDevice*)((BYTE*)pEffects - 0xC);
        
        if (pDevice->field_0x0 != FALSE) {
            LPDIRECTINPUTEFFECT* pEffect = pEffects;
            int count = 10;
            
            do {
                if (*pEffect != NULL) {
                    ULONG refcount = (*pEffect)->Release();
                    if (refcount == 0) {
                        *pEffect = NULL;
                    }
                }
                pEffect++;
                count--;
            } while (count != 0);
            
            pDevice->field_0x8 = 0;
        }
        
        pEffects = (LPDIRECTINPUTEFFECT*)((BYTE*)pEffects + 0x34);
    } while ((int)pEffects < (int)&m_forceFeedbackDevices[8]+16);
}

// 96.77% match, only concern is this
// 0x4aaeda	-cmp edi, 0x666ec8
// 	        +cmp edi, CInput::m_dinputRefGuidKeyboard (DATA) (Input.cpp:587)  <-- why  are you that
// FUNCTION: CMR2 0x004aaea0
BOOL CInput::ResetForceFeedbackEffectsAlt(void) {
    ForceFeedbackDevice* pDevice = m_forceFeedbackDevices;
    
    do {
        if (pDevice->field_0x0 != FALSE) {
            LPDIRECTINPUTEFFECT* pEffect = pDevice->effects;
            int count = 10;
            
            do {
                if (*pEffect != NULL) {
                    ULONG refcount = (*pEffect)->Release();
                    if (refcount == 0) {
                        *pEffect = NULL;
                    }
                }
                pEffect++;
                count--;
            } while (count != 0);
            
            pDevice->field_0x8 = 0;
            pDevice->field_0x0 = 0;
        }
        pDevice++;
    } while ((int)pDevice < (int)&m_forceFeedbackDevices[8]);
    m_unk0x00666ee8 = FALSE;
    return TRUE;
}

// FUNCTION: CMR2 0x0040bf70
void CInput::LoadControllerInfo(void) {
    ControllerData *fileBuffer;

    m_hasLoadedControllerInfo = 0;
    sprintf(CFrontend::m_stringDest, m_strControllerInfoDir, CInstallInfo::GetGameHDPath());

    fileBuffer = (ControllerData*)CFileBuffer::GetGenericFileBuffer(CFrontend::m_stringDest, TRUE);
    if (fileBuffer != NULL) {
        if (CGenericFileLoader::GetGenericFileSize() == 0x11a4) {
            memcpy(&m_controllerInfo, fileBuffer, sizeof(m_controllerInfo));

            // write the first two elements of the array
            memcpy(m_unk0x005168f4, &((unsigned int*)fileBuffer)[0x468], 4);

            CFileBuffer::FreeGenericFileBuffer(fileBuffer);
            m_hasLoadedControllerInfo = TRUE;

            FUN_0040be90(0);
            FUN_0040be90(1);
        }
    }
}

// FUNCTION: CMR2 0x0040be90
void CInput::FUN_0040be90(unsigned int param1) {
    int uVar1;
    unsigned short uVar2;
    ControllerData * pController;

    uVar2 = param1;
    uVar1 = m_unk0x005168f4[uVar2];
    pController = &m_controllerInfo[uVar1];

    if (uVar1 > 1) {
        FUN_0040c440(param1, pController);
    }

    FUN_0049eb90(uVar2, 1, pController->field_0x13e[0]);
    FUN_0049eb90(uVar2, 2, pController->field_0x13e[1]);
    FUN_0049eb90(uVar2, 4, pController->field_0x13e[2]);
    FUN_0049eb90(uVar2, 8, pController->field_0x13e[3]);
    FUN_0049eb90(uVar2, 0x10, pController->field_0x13e[4]);
    FUN_0049eb90(uVar2, 0x20, pController->field_0x13e[5]);
    FUN_0049eb90(uVar2, 0x40, pController->field_0x13e[6]);
    FUN_0049eb90(uVar2, 0x80, pController->field_0x13e[7]);
    FUN_0049eb90(uVar2, 0x100, pController->field_0x13e[8]);
}

// FUNCTION: CMR2 0x0040c440
void CInput::FUN_0040c440(unsigned int param1, ControllerData* param2) {
    unsigned short* puVar2;
    unsigned short uVar4;
    unsigned char bVar1;
    int iVar3;
    unsigned short combinedFlags;
    
    iVar3 = 0;
    puVar2 = &param2->field_0x128;
    
    do {
        if (*puVar2 == 0) {
            if (param2->field_0x13e[iVar3] != 0) {
                
                combinedFlags = param2->field_0x13a | param2->field_0x136 | 
                                param2->field_0x134 | param2->field_0x132 |
                                param2->field_0x130 | param2->field_0x12e | 
                                param2->field_0x12c | param2->field_0x12a |
                                param2->field_0x128 | 0x400;

                bVar1 = FUN_0040c530(combinedFlags & 0xffff);
                
                uVar4 = 1 << bVar1;
                
                FUN_0049eb90(param1 & 0xffff, uVar4 & 0xffff, param2->field_0x13e[iVar3]);
                FUN_0049eb90(param1 & 0xffff, (1 << iVar3) & 0xffff, 0);
                
                *puVar2 = uVar4;
            }
        }
        
        iVar3++;
        puVar2++;
    } while (iVar3 < 10);
}

// FUNCTION: CMR2 0x0049eb90
void CInput::FUN_0049eb90(int param1, unsigned int param2, unsigned int param3) {
    DeviceInfo* device = &m_availableDevices[param1];
    switch(param2) {
        case 1:
            device->keyboard.field_0x468 = param3;
            break;

        case 2:
            device->keyboard.field_0x469 = param3;
            break;

        case 4:
            device->keyboard.field_0x46a = param3;
            break;

        case 8:
            device->keyboard.field_0x46b = param3;
            break;

        case 0x10:
            device->keyboard.field_0x46c = param3;
            break;

        case 0x20:
            device->keyboard.field_0x46d = param3;
            break;

        case 0x40:
            device->keyboard.field_0x46e = param3;
            break;
        
        case 0x80:
            device->keyboard.field_0x46f = param3;
            break;

        case 0x100:
            device->keyboard.field_0x470 = param3;
            break;

        case 0x200:
            device->keyboard.field_0x471 = param3;
            break;

        case 0x400:
            device->keyboard.field_0x472 = param3;
            break;

        case 0x800:
            device->keyboard.field_0x473 = param3;
            break;
        
        case 0x1000:
            device->keyboard.field_0x474 = param3;
            break;

        case 0x2000:
            device->keyboard.field_0x475 = param3;
            break;
        
        case 0x4000:
            device->keyboard.field_0x476 = param3;
            break;

        case 0x8000:
            device->keyboard.field_0x477 = param3;
            break;
    }
}

// FUNCTION: CMR2 0x0040c530 
BYTE CInput::FUN_0040c530(unsigned int param1) {
    int iVar1 = 0;

    while (iVar1 < 32) {
        if (!(param1 & 1)) break;
        
        param1 >>= 1;
        iVar1++;
    }

    return iVar1;
}

// FUNCTION: CMR2 0x0049eb50
void CInput::FUN_0049eb50(void)
{
    int i;

    DInputReleaseDevices();
    m_unk0x0059f8cc.field_0x1 = 2;
    m_unk0x0059f8cc.field_0x0 = 0;
    m_unk0x0059f8cc.field_0x2 = 0;
    for (i = 0; i < 8; i++) {
        m_availableDevices[i].field_0x0 = -1;
        m_availableDevices[i].field_0x18 = -1;
    }
    SetupKeyboard();
    SetupMouse();
    GetAttachedJoysticks();
}

// FUNCTION: CMR2 0x0049edd0
int CInput::GetFirstPressedKey(void)
{
    int key;
    int result;

    result = -1;
    for (key = 0; key < 0xdd; key++) {
        if (m_keyboardState[key] & 0x80) {
            result = key;
            break;
        }
    }
    return result;
}

// FUNCTION: CMR2 0x0049edf0
BOOL CInput::IsShiftPressed(void)
{
    if ((m_keyboardState[DIK_LSHIFT] & 0x80) == 0 && (m_keyboardState[DIK_RSHIFT] & 0x80) == 0)
        return FALSE;
    return TRUE;
}

// FUNCTION: CMR2 0x0049efc0
void CInput::FUN_0049efc0(void)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (m_availableDevices[i].field_0x0 == 2) {
            m_availableDevices[i].joystick.bindings[0].range = 0;
            m_availableDevices[i].joystick.field_0x4 = 0;
            m_availableDevices[i].joystick.controlCount = 0;
            return;
        }
    }
}

// FUNCTION: CMR2 0x0049f300
void CInput::ReadKeyboardState(void)
{
    HRESULT hr;
    short retries;

    retries = 0;
    hr = m_pDirectInputKeyboard->GetDeviceState(sizeof(m_keyboardState), m_keyboardState);
    while (hr == DIERR_INPUTLOST && retries <= 20 && SUCCEEDED(m_pDirectInputKeyboard->Acquire())) {
        retries++;
        hr = m_pDirectInputKeyboard->GetDeviceState(sizeof(m_keyboardState), m_keyboardState);
    }
}

// FUNCTION: CMR2 0x0049fd00
int CInput::GetButtonIndexFromMask(unsigned int mask)
{
    int i;

    for (i = 0; i < 24; i++) {
        if (m_buttonMasks[i] & mask)
            break;
    }
    if (i == 24)
        i = -1;
    return i;
}

// FUNCTION: CMR2 0x0049ff80
void CInput::FUN_0049ff80(DWORD p1, DWORD p2, DWORD p3, DWORD p4, DWORD p5)
{
    m_unk0x0059f8f8 = p1;
    m_unk0x0059f910 = p2;
    m_unk0x0059f8f4 = p3;
    m_unk0x0059f8f0 = p4;
    m_unk0x0059f90c = p5;
}

// FUNCTION: CMR2 0x0049ffc0
void CInput::FUN_0049ffc0(DWORD param1)
{
    m_unk0x0059f900 = param1;
}

// FUNCTION: CMR2 0x0040be00
DWORD CInput::FUN_0040be00(unsigned int param1)
{
    return m_controllerInfo[m_unk0x005168f4[(unsigned short)param1]].field_0x114;
}

// FUNCTION: CMR2 0x0040be30
DWORD CInput::FUN_0040be30(unsigned int param1)
{
    return m_controllerInfo[m_unk0x005168f4[(unsigned short)param1]].field_0x120;
}

// FUNCTION: CMR2 0x0040be60
DWORD CInput::FUN_0040be60(unsigned int param1)
{
    return m_controllerInfo[m_unk0x005168f4[(unsigned short)param1]].field_0x124;
}

// FUNCTION: CMR2 0x0040c210
unsigned int CInput::FUN_0040c210(unsigned int param1, int param2)
{
    unsigned int controller;

    controller = m_unk0x005168f4[(unsigned short)param1];
    if (m_controllerInfo[controller].field_0x210[param2].field_0x0 != 0)
        return m_controllerInfo[controller].field_0x2d8[param2];
    return -1;
}

// FUNCTION: CMR2 0x0040c270
BOOL CInput::FUN_0040c270(int param1, ControllerData *param2)
{
    if ((&param2->field_0x128)[param1] != 0 && param2->field_0x13e[param1] != 0)
        return TRUE;
    return FALSE;
}

// FUNCTION: CMR2 0x004aaf50
void CInput::FUN_004aaf50(DWORD param1, int index)
{
    m_unk0x00666ec8[index] = param1;
}

// FUNCTION: CMR2 0x004aaf70
void CInput::StartForceFeedbackEffect(int effectIndex, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    LPDIRECTINPUTEFFECT pEffect;
    DWORD status;

    pDevice = &m_forceFeedbackDevices[deviceIndex];
    if (pDevice->field_0x0 != 0) {
        pEffect = pDevice->effects[effectIndex];
        if (pEffect != NULL) {
            FUN_004ab5f0(pEffect->GetEffectStatus(&status));
            if ((status & DIEGES_PLAYING) == 0)
                pDevice->effects[effectIndex]->Start(1, 0);
        }
    }
}

// FUNCTION: CMR2 0x004ab600
char *CInput::FormatString(LPCSTR format, ...)
{
    va_list args;

    va_start(args, format);
    wvsprintfA(m_formatBuffer, format, args);
    return m_formatBuffer;
}

// FUNCTION: CMR2 0x0049f360
void CInput::ReadMouse(DeviceInfo *pDevice)
{
    DIMOUSESTATE2 mouseState;
    HRESULT hr;
    int retries;
    int i;

    retries = 0;
    pDevice->field_0x4 = 0;
    ((LPDIRECTINPUTDEVICE7A)m_pDirectInputMouse)->Poll();
    hr = m_pDirectInputMouse->GetDeviceState(sizeof(mouseState), &mouseState);
    while (hr == DIERR_INPUTLOST) {
        if (retries > 5)
            return;
        m_pDirectInputMouse->Acquire();
        retries++;
        ((LPDIRECTINPUTDEVICE7A)m_pDirectInputMouse)->Poll();
        hr = m_pDirectInputMouse->GetDeviceState(sizeof(mouseState), &mouseState);
    }

    pDevice->joystick.bindings[0].field_0xc += mouseState.lX * 150;
    if (pDevice->joystick.bindings[0].field_0xc > 0x10000)
        pDevice->joystick.bindings[0].field_0xc = 0x10000;
    if (pDevice->joystick.bindings[0].field_0xc < -0x10000)
        pDevice->joystick.bindings[0].field_0xc = -0x10000;
    pDevice->joystick.bindings[1].field_0xc += mouseState.lY * 150;
    pDevice->joystick.bindings[2].field_0xc += mouseState.lZ * 150;

    for (i = 0; i < 8; i++) {
        if (i < 24 && (mouseState.rgbButtons[i] & 0x80))
            pDevice->field_0x4 |= m_buttonMasks[i];
    }
}

// FUNCTION: CMR2 0x0049f480
void CInput::ReadKeyboardDevice(DeviceInfo *pDevice)
{
    pDevice->field_0x4 = 0;
    if (CGameInfo::m_unk0x0059f8d0 != 0) {
        if (m_keyboardState[DIK_LEFT] & 0x80)
            pDevice->field_0x4 = 1;
        if (m_keyboardState[DIK_RIGHT] & 0x80)
            pDevice->field_0x4 |= 0x2;
        if (m_keyboardState[DIK_UP] & 0x80)
            pDevice->field_0x4 |= 0x4;
        if (m_keyboardState[DIK_DOWN] & 0x80)
            pDevice->field_0x4 |= 0x8;
        if (m_keyboardState[DIK_RETURN] & 0x80)
            pDevice->field_0x4 |= 0x10;
        if (m_keyboardState[DIK_ESCAPE] & 0x80)
            pDevice->field_0x4 |= 0x20;
        if (m_keyboardState[DIK_F1] & 0x80)
            pDevice->field_0x4 |= 0x1000;
        if (m_keyboardState[DIK_F2] & 0x80)
            pDevice->field_0x4 |= 0x2000;
    } else {
        if (m_keyboardState[pDevice->keyboard.field_0x468] & 0x80)
            pDevice->field_0x4 = 1;
        if (m_keyboardState[pDevice->keyboard.field_0x469] & 0x80)
            pDevice->field_0x4 |= 0x2;
        if (m_keyboardState[pDevice->keyboard.field_0x46a] & 0x80)
            pDevice->field_0x4 |= 0x4;
        if (m_keyboardState[pDevice->keyboard.field_0x46b] & 0x80)
            pDevice->field_0x4 |= 0x8;
        if (m_keyboardState[pDevice->keyboard.field_0x46c] & 0x80)
            pDevice->field_0x4 |= 0x10;
        if (m_keyboardState[pDevice->keyboard.field_0x46d] & 0x80)
            pDevice->field_0x4 |= 0x20;
        if (m_keyboardState[pDevice->keyboard.field_0x46e] & 0x80)
            pDevice->field_0x4 |= 0x40;
        if (m_keyboardState[pDevice->keyboard.field_0x46f] & 0x80)
            pDevice->field_0x4 |= 0x80;
        if (m_keyboardState[pDevice->keyboard.field_0x470] & 0x80)
            pDevice->field_0x4 |= 0x100;
        if (m_keyboardState[pDevice->keyboard.field_0x471] & 0x80)
            pDevice->field_0x4 |= 0x200;
        if (m_keyboardState[pDevice->keyboard.field_0x472] & 0x80)
            pDevice->field_0x4 |= 0x400;
        if (m_keyboardState[pDevice->keyboard.field_0x473] & 0x80)
            pDevice->field_0x4 |= 0x800;
        if (m_keyboardState[pDevice->keyboard.field_0x474] & 0x80)
            pDevice->field_0x4 |= 0x1000;
        if (m_keyboardState[pDevice->keyboard.field_0x475] & 0x80)
            pDevice->field_0x4 |= 0x2000;
        if (m_keyboardState[pDevice->keyboard.field_0x476] & 0x80)
            pDevice->field_0x4 |= 0x4000;
        if (m_keyboardState[pDevice->keyboard.field_0x477] & 0x80)
            pDevice->field_0x4 |= 0x8000;
    }
}

// FUNCTION: CMR2 0x0049fb70
void CInput::ReadJoystick(DeviceInfo *pDevice)
{
    DIJOYSTATE2 joyState;
    LPDIRECTINPUTDEVICE7A pJoystick;
    HRESULT hr;
    int retries;
    int i;

    retries = 0;
    pJoystick = (LPDIRECTINPUTDEVICE7A)m_unk0x0059f6b0[pDevice->field_0x18];
    pDevice->field_0x4 = 0;
    pJoystick->Poll();
    hr = pJoystick->GetDeviceState(sizeof(joyState), &joyState);
    while (hr == DIERR_INPUTLOST) {
        if (retries > 5)
            return;
        pJoystick->Acquire();
        retries++;
        pJoystick->Poll();
        hr = pJoystick->GetDeviceState(sizeof(joyState), &joyState);
    }

    pDevice->joystick.bindings[0].field_0xc = joyState.lX;
    pDevice->joystick.bindings[1].field_0xc = joyState.lY;
    pDevice->joystick.bindings[2].field_0xc = joyState.lZ;
    pDevice->joystick.bindings[3].field_0xc = joyState.lRx;
    pDevice->joystick.bindings[4].field_0xc = joyState.lRy;
    pDevice->joystick.bindings[5].field_0xc = joyState.lRz;
    pDevice->joystick.bindings[6].field_0xc = joyState.rglSlider[0];
    pDevice->joystick.bindings[7].field_0xc = joyState.rglSlider[1];

    if (joyState.lX < -39321)
        pDevice->field_0x4 |= 0x1;
    else if (joyState.lX > 39321)
        pDevice->field_0x4 |= 0x2;
    if (joyState.lY < -39321)
        pDevice->field_0x4 |= 0x4;
    else if (joyState.lY > 39321)
        pDevice->field_0x4 |= 0x8;

    if (LOWORD(joyState.rgdwPOV[0]) != 0xffff) {
        if (joyState.rgdwPOV[0] >= 4500 && joyState.rgdwPOV[0] <= 13500)
            pDevice->field_0x4 |= 0x2;
        if (joyState.rgdwPOV[0] >= 13500 && joyState.rgdwPOV[0] <= 22500)
            pDevice->field_0x4 |= 0x8;
        if (joyState.rgdwPOV[0] >= 22500 && joyState.rgdwPOV[0] <= 31500)
            pDevice->field_0x4 |= 0x1;
        if (joyState.rgdwPOV[0] >= 31500 || joyState.rgdwPOV[0] <= 4500)
            pDevice->field_0x4 |= 0x4;
    }

    for (i = 0; i < pDevice->field_0x14; i++) {
        if (i < 24 && (joyState.rgbButtons[i] & 0x80))
            pDevice->field_0x4 |= m_buttonMasks[i];
    }
}

// FUNCTION: CMR2 0x0040c130
void CInput::FUN_0040c130(unsigned short *values, int index, unsigned short value)
{
    switch (index) {
    case 0: values[0] = value; break;
    case 1: values[1] = value; break;
    case 2: values[2] = value; break;
    case 3: values[3] = value; break;
    case 4: values[4] = value; break;
    case 5: values[5] = value; break;
    case 6: values[6] = value; break;
    case 7: values[7] = value; break;
    case 8: values[8] = value; break;
    case 9: values[9] = value; break;
    }
}

// FUNCTION: CMR2 0x0040c550
void CInput::FUN_0040c550(BYTE *values, int index, BYTE value)
{
    switch (index) {
    case 0: values[0] = value; break;
    case 1: values[1] = value; break;
    case 2: values[2] = value; break;
    case 3: values[3] = value; break;
    case 4: values[4] = value; break;
    case 5: values[5] = value; break;
    case 6: values[6] = value; break;
    case 7: values[7] = value; break;
    case 8: values[8] = value; break;
    }
}

// FUNCTION: CMR2 0x0049e960
DeviceInfo *CInput::UpdateDevice(int index)
{
    DeviceInfo *pDevice;
    unsigned int oldButtons;
    unsigned int buttons;
    unsigned int dirButtons;
    int now;

    if (index < 0 || index >= 8)
        return NULL;

    pDevice = &m_availableDevices[index];
    oldButtons = pDevice->field_0x4;
    if (index > 1) {
        if (pDevice->field_0x18 < 0 || pDevice->field_0x18 > 3)
            return NULL;
    }

    switch (pDevice->field_0x0) {
    case 1:
        if (index == 0)
            ReadKeyboardState();
        ReadKeyboardDevice(pDevice);
        break;
    case 0:
    case 3:
        ReadJoystick(pDevice);
        break;
    case 2:
        ReadMouse(pDevice);
        break;
    }

    buttons = pDevice->field_0x4;
    pDevice->field_0x8 = (buttons ^ oldButtons) & buttons;
    if (buttons & 0xfffffff0)
        pDevice->field_0x4 = buttons | 0x10000;

    now = CMain::GetFrameTime();
    dirButtons = pDevice->field_0x4 & m_directionButtonMask;
    if (dirButtons != 0) {
        if (dirButtons != (m_directionButtonMask & oldButtons)) {
            pDevice->field_0xc = ~oldButtons & pDevice->field_0x4 & m_directionButtonMask;
            pDevice->field_0x10 = (int)m_keyboardDelay + now;
        } else if (pDevice->field_0x10 - now <= 0) {
            pDevice->field_0xc = dirButtons;
            pDevice->field_0x10 = (int)m_keyboardSpeed + now;
        } else {
            pDevice->field_0xc = 0;
        }
    } else {
        pDevice->field_0xc = 0;
        pDevice->field_0x10 = 0;
    }

    if (m_unk0x0052086c != 0)
        pDevice->field_0x8 |= pDevice->field_0xc;

    return pDevice;
}

// FUNCTION: CMR2 0x004ab380
int CInput::CreateForceFeedbackEffect(int effectType, DWORD duration, LONG coefficient, LONG offset, int triggerButton, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    GUID guid;
    DWORD dwAxes[2];
    LONG lDirection[2];
    int slot;
    int i;

    i = 0;
    pDevice = &m_forceFeedbackDevices[deviceIndex];
    if (pDevice->field_0x0 == 0)
        return -1;

    while (pDevice->effects[i] != NULL) {
        if (i == 10)
            return -1;
        i++;
    }

    switch (effectType) {
    case 0xb:
        guid = GUID_Spring;
        break;
    case 0xc:
        guid = GUID_Inertia;
        break;
    case 0xd:
        guid = GUID_Damper;
        break;
    case 0xe:
        guid = GUID_Friction;
        break;
    }

    slot = deviceIndex * 10 + i;
    dwAxes[0] = DIJOFS_X;
    lDirection[0] = 0;
    lDirection[1] = 0;

    m_forceFeedbackConditions[slot].lOffset = offset;
    m_forceFeedbackConditions[slot].lPositiveCoefficient = coefficient;
    m_forceFeedbackConditions[slot].lNegativeCoefficient = coefficient;
    m_forceFeedbackConditions[slot].dwPositiveSaturation = 10000;
    m_forceFeedbackConditions[slot].dwNegativeSaturation = 10000;
    m_forceFeedbackConditions[slot].lDeadBand = 0;

    m_forceFeedbackEffects[slot].dwSize = sizeof(DIEFFECT);
    m_forceFeedbackEffects[slot].dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
    m_forceFeedbackEffects[slot].dwDuration = duration;
    m_forceFeedbackEffects[slot].dwSamplePeriod = 10000;
    m_forceFeedbackEffects[slot].dwGain = 10000;
    if (triggerButton == -1)
        triggerButton = DIEB_NOTRIGGER;
    else
        triggerButton = DIJOFS_BUTTON(triggerButton);
    m_forceFeedbackEffects[slot].dwTriggerButton = triggerButton;
    m_forceFeedbackEffects[slot].dwTriggerRepeatInterval = 0;
    m_forceFeedbackEffects[slot].cAxes = 1;
    m_forceFeedbackEffects[slot].rgdwAxes = dwAxes;
    m_forceFeedbackEffects[slot].rglDirection = lDirection;
    m_forceFeedbackEffects[slot].lpEnvelope = NULL;
    m_forceFeedbackEffects[slot].cbTypeSpecificParams = sizeof(DICONDITION);
    m_forceFeedbackEffects[slot].lpvTypeSpecificParams = &m_forceFeedbackConditions[slot];

    pDevice->device->CreateEffect(guid, &m_forceFeedbackEffects[slot], &pDevice->effects[i], NULL);
    return i;
}

// FUNCTION: CMR2 0x004ab590
int CInput::CreateSpringEffect(DWORD duration, LONG coefficient, LONG offset, int triggerButton, int deviceIndex)
{
    return CreateForceFeedbackEffect(0xb, duration, coefficient, offset, triggerButton, deviceIndex);
}

// FUNCTION: CMR2 0x004ab5c0
int CInput::CreateDamperEffect(DWORD duration, LONG coefficient, LONG offset, int triggerButton, int deviceIndex)
{
    return CreateForceFeedbackEffect(0xd, duration, coefficient, offset, triggerButton, deviceIndex);
}

// FUNCTION: CMR2 0x0040c2a0
short CInput::GetButtonMapping(unsigned short controller, int button)
{
    unsigned short index;
    ControllerData *pController;
    short mapping;

    mapping = 0;
    index = m_unk0x005168f4[controller];
    pController = &m_controllerInfo[index];
    switch (button) {
    case 0: mapping = pController->field_0x128; break;
    case 1: mapping = pController->field_0x12a; break;
    case 2: mapping = pController->field_0x12c; break;
    case 3: mapping = pController->field_0x12e; break;
    case 4: mapping = pController->field_0x130; break;
    case 5: mapping = pController->field_0x132; break;
    case 6: mapping = pController->field_0x134; break;
    case 7: mapping = pController->field_0x136; break;
    case 8: mapping = pController->field_0x138; break;
    case 9: mapping = pController->field_0x13a; break;
    default: goto defaults;
    }

    if (mapping == 0) {
defaults:
        if (controller == 0 || controller == 1)
            goto fixed;
    }

    if (!FUN_0040c270(button, pController))
        return mapping;

fixed:
    switch (button) {
    case 0: mapping = 1; break;
    case 1: mapping = 2; break;
    case 2: mapping = 4; break;
    case 3: mapping = 8; break;
    case 4: mapping = 0x10; break;
    case 5: mapping = 0x20; break;
    case 6: mapping = 0x40; break;
    case 7: mapping = 0x80; break;
    case 8: mapping = 0x100; break;
    case 9: mapping = 0x200; break;
    }
    return mapping;
}

// FUNCTION: CMR2 0x004ab030
HRESULT CInput::SetEffectGain(int effectIndex, DWORD gain, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    LPDIRECTINPUTEFFECT pEffect;
    DWORD flags;

    pDevice = &m_forceFeedbackDevices[deviceIndex];
    flags = DIEP_GAIN;
    if (m_unk0x00666ec8[deviceIndex] == 0)
        flags = DIEP_GAIN | DIEP_NODOWNLOAD;

    if (pDevice->field_0x0 != 0 && (pEffect = pDevice->effects[effectIndex]) != NULL) {
        DIEFFECT effect = { sizeof(DIEFFECT) };
        effect.dwGain = gain;
        return pEffect->SetParameters(&effect, flags);
    }
    return E_INVALIDARG;
}

// FUNCTION: CMR2 0x004ab0b0
HRESULT CInput::SetEffectGainAndDirection(int effectIndex, DWORD gain, LONG direction, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    LPDIRECTINPUTEFFECT pEffect;
    DWORD flags;

    pDevice = &m_forceFeedbackDevices[deviceIndex];
    flags = DIEP_GAIN | DIEP_DIRECTION;
    if (m_unk0x00666ec8[deviceIndex] == 0)
        flags = DIEP_GAIN | DIEP_DIRECTION | DIEP_NODOWNLOAD;

    if (pDevice->field_0x0 != 0 && (pEffect = pDevice->effects[effectIndex]) != NULL) {
        DIEFFECT effect = { sizeof(DIEFFECT) };
        LONG lDirection[2];
        effect.dwFlags = DIEFF_POLAR | DIEFF_OBJECTOFFSETS;
        effect.cAxes = 2;
        effect.rgdwAxes = NULL;
        effect.dwGain = gain;
        effect.rglDirection = lDirection;
        lDirection[0] = direction;
        return pEffect->SetParameters(&effect, flags);
    }
    return E_INVALIDARG;
}

// FUNCTION: CMR2 0x004ab150
int CInput::CreateConstantForceEffect(DWORD duration, LONG direction, LONG magnitude, DWORD attackTime, DWORD attackLevel, DWORD fadeTime, DWORD fadeLevel, int triggerButton, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    int i;
    DIENVELOPE envelope;
    LONG lDirection[2];
    DWORD dwAxes[2];
    DICONSTANTFORCE constantForce;

    i = 0;
    pDevice = &m_forceFeedbackDevices[deviceIndex];
    if (pDevice->field_0x0 == 0)
        return -1;

    while (pDevice->effects[i] != NULL) {
        if (i == 10)
            return -1;
        i++;
    }

    constantForce.lMagnitude = magnitude;
    memset(&envelope, 0, sizeof(envelope));
    envelope.dwAttackTime = attackTime;
    envelope.dwFadeTime = fadeTime;
    envelope.dwAttackLevel = attackLevel;
    envelope.dwFadeLevel = fadeLevel;
    lDirection[0] = direction;
    DIEFFECT effect = { sizeof(DIEFFECT) };
    effect.dwSamplePeriod = 10000;
    effect.dwGain = 10000;
    envelope.dwSize = sizeof(DIENVELOPE);
    dwAxes[0] = DIJOFS_X;
    dwAxes[1] = DIJOFS_Y;
    lDirection[1] = 0;
    effect.dwFlags = DIEFF_POLAR | DIEFF_OBJECTOFFSETS;
    effect.dwDuration = duration;
    if (triggerButton == -1)
        effect.dwTriggerButton = triggerButton;
    else
        effect.dwTriggerButton = DIJOFS_BUTTON(triggerButton);
    effect.rgdwAxes = dwAxes;
    effect.rglDirection = lDirection;
    effect.lpEnvelope = &envelope;
    effect.lpvTypeSpecificParams = &constantForce;
    effect.dwTriggerRepeatInterval = 0;
    effect.cAxes = 2;
    effect.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);

    FUN_004ab5f0(pDevice->device->CreateEffect(GUID_ConstantForce, &effect, &pDevice->effects[i], NULL));
    return i;
}

// FUNCTION: CMR2 0x004ab2b0
HRESULT CInput::SetConditionCoefficient(int effectIndex, LONG coefficient, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    DWORD flags;
    int slot;

    pDevice = &m_forceFeedbackDevices[deviceIndex];
    flags = DIEP_TYPESPECIFICPARAMS;
    if (m_unk0x00666ec8[deviceIndex] == 0)
        flags = DIEP_TYPESPECIFICPARAMS | DIEP_NODOWNLOAD;

    if (pDevice->field_0x0 != 0 && pDevice->effects[effectIndex] != NULL) {
        slot = deviceIndex * 10 + effectIndex;
        m_forceFeedbackConditions[slot].lOffset = 0;
        m_forceFeedbackConditions[slot].lPositiveCoefficient = coefficient;
        m_forceFeedbackConditions[slot].lNegativeCoefficient = coefficient;
        m_forceFeedbackConditions[slot].dwPositiveSaturation = 10000;
        m_forceFeedbackConditions[slot].dwNegativeSaturation = 10000;
        m_forceFeedbackConditions[slot].lDeadBand = 0;
        m_forceFeedbackEffects[slot].cbTypeSpecificParams = sizeof(DICONDITION);
        m_forceFeedbackEffects[slot].lpvTypeSpecificParams = &m_forceFeedbackConditions[slot];
        return pDevice->effects[effectIndex]->SetParameters(&m_forceFeedbackEffects[slot], flags);
    }
    return E_INVALIDARG;
}

// FUNCTION: CMR2 0x0040bff0
void CInput::SaveControllerInfo(void)
{
    unsigned int *buffer;

    buffer = (unsigned int *)CFileBuffer::AllocateLockedBuffer(0x11a4);
    memcpy(buffer, m_controllerInfo, sizeof(m_controllerInfo));
    buffer[0x468] = *(unsigned int *)m_unk0x005168f4;
    sprintf(CFrontend::m_stringDest, m_strControllerInfoDir, CInstallInfo::GetGameHDPath());
    CInstallInfo::WriteFileToDisk(CFrontend::m_stringDest, 2, buffer, 0x11a4);
    CFileBuffer::FreeGenericFileBuffer(buffer);
}

// STUB: CMR2 0x0040c610
void CInput::FUN_0040c610(DeviceInfo *pDevice, int index)
{
}

// FUNCTION: CMR2 0x0040c050
void CInput::FUN_0040c050(void)
{
    DeviceInfo *pDevice;
    int i;

    m_controllerCount = FUN_0049ef90();
    for (i = 0; i < m_controllerCount; i++) {
        pDevice = UpdateDevice(i);
        if (m_hasLoadedControllerInfo == 0 || strcmp(m_controllerInfo[i].name, pDevice->deviceInstanceName) != 0)
            FUN_0040c610(pDevice, i);
    }
    if (m_controllerCount < 6)
        memset(&m_controllerInfo[m_controllerCount], 0, (6 - m_controllerCount) * sizeof(ControllerData));
    m_hasLoadedControllerInfo = TRUE;
}

// GLOBAL: CMR2 0x006ed3f4
int g_unk0x006ed3f4[30];
// GLOBAL: CMR2 0x006ed46c
int g_unk0x006ed46c[30];

// Queues one character for the input ring buffer.
// FUNCTION: CMR2 0x004b7ca0
void CInput::FUN_004b7ca0(int param1)
{
    int *p;
    int i;

    i = 0;
    p = g_unk0x006ed3f4;
    while (1) {
        if (*p == 0)
            break;
        p++;
        i++;
        if ((int)p >= (int)g_unk0x006ed46c)
            return;
    }
    g_unk0x006ed3f4[i] = param1;
}

// Queues one key press (only when the scan code carries a virtual key).
// FUNCTION: CMR2 0x004b7d10
void CInput::FUN_004b7d10(unsigned int param1)
{
    int *p;
    int i;

    if ((param1 & 0xff0000) != 0) {
        i = 0;
        p = g_unk0x006ed46c;
        while (1) {
            if (*p == 0)
                break;
            p++;
            i++;
            if ((int)p >= (int)(g_unk0x006ed46c + 30))
                return;
        }
        g_unk0x006ed46c[i] = param1;
    }
}

// GLOBAL: CMR2 0x005320a4
int g_unk0x005320a4;

// FUNCTION: CMR2 0x0040af20
void CInput::FUN_0040af20(void)
{
    g_unk0x005320a4 = CMain::GetFrameDelta();
}

// FUNCTION: CMR2 0x0049eab0
void CInput::FUN_0049eab0(void)
{
    int i;

    for (i = 0; i < 8; i++)
        UpdateDevice(i);
}

// GLOBAL: CMR2 0x005168f4
unsigned short g_unk0x005168f4[0x100];
// GLOBAL: CMR2 0x00532250
BYTE g_unk0x00532250[8 * 0x2f0];

// FUNCTION: CMR2 0x0040bc90
void CInput::FUN_0040bc90(int param1, DWORD param2)
{
    int index;

    index = g_unk0x005168f4[param1 & 0xffff];
    *(DWORD *)((char *)g_unk0x00532250 + index * 0x2f0 + 0x11c) = param2;
    FUN_004aaf50(param2, index);
}

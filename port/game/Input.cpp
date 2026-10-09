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

// PORT: the DirectInput 7 data formats (c_dfDIMouse2, c_dfDIKeyboard,
// c_dfDIJoystick2 and their descriptor arrays) are gone: the input layer
// returns the state structures directly.

// GLOBAL: CMR2 0x00511758
// IID_IDirectInput7A

// GLOBAL: CMR2 0x0059f8c8
void *CInput::m_lpDirectInput7;

// GLOBAL: CMR2 0x00520874
char CInput::m_strKeyboard[12] = "Keyboard";

// GLOBAL: CMR2 0x0059f8cc
Unk0x0059f8cc CInput::m_unk0x0059f8cc; // device count or something

// GLOBAL: CMR2 0x0059ce48
InputDeviceState CInput::m_deviceState;

// GLOBAL: CMR2 0x0059f8d4
DWORD CInput::m_mouseGranularity;

// GLOBAL: CMR2 0x0059f8d8
PVOID CInput::m_keyboardDelay;

// GLOBAL: CMR2 0x0059f8dc
PVOID CInput::m_keyboardSpeed;


// GLOBAL: CMR2 0x0059f7c4
InputDevice *CInput::m_pDirectInputMouse = NULL;

// GLOBAL: CMR2 0x00511400
USHORT CInput::m_unk0x00511400[8] = {0, 4, 8, 12, 16, 20, 24, 28};

// GLOBAL: CMR2 0x0059f6b0
InputDevice *CInput::m_unk0x0059f6b0[8];

// GLOBAL: CMR2 0x00520880
CHAR CInput::m_strD[4] = " D";
// GLOBAL: CMR2 0x00520884
CHAR CInput::m_strU[4] = " U";
// GLOBAL: CMR2 0x00520888
CHAR CInput::m_strR[4] = " R";
// GLOBAL: CMR2 0x0052088C
CHAR CInput::m_strL[4] = " L";

// GLOBAL: CMR2 0x00666d28
InputFeedbackState CInput::m_feedbackState;

// GLOBAL: CMR2 0x00666ee8
BOOL CInput::m_unk0x00666ee8 = FALSE;

char CInput::m_formatBuffer[512];
BYTE CInput::m_keyboardState[256];
DWORD CInput::m_buttonMasks[24] = {
    0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000,
    0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000, 0x1, 0x2, 0x4, 0x8
};
unsigned int CInput::m_directionButtonMask = 0xf;
BOOL CInput::m_unk0x0052086c = TRUE;
InputEffectParams CInput::m_forceFeedbackEffects[80];
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
// PORT: SDL is initialised by the platform layer.
BOOL CInput::DInputCreate(void) {
    CGame::RegisterCallback(DInputRelease, NULL);
    return TRUE;
}

// FUNCTION: CMR2 0x0049fe30
// PORT: type 0 keyboard, 1 mouse, 2 joystick (index); the cooperative
// levels become the mouse capture below.
InputDevice *CInput::DInputCreateDevice(int type, int index) {
    InputDevice *pDevice;

    if (type == 0)
        return Input_OpenKeyboard();
    if (type == 1) {
        pDevice = Input_OpenMouse();
        if (pDevice != NULL)
            Input_SetMouseExclusive(pDevice, g_pGraphics->isFullscreen);
        return pDevice;
    }
    return Input_OpenJoystick(index);
}

// FUNCTION: CMR2 0x0049fd60
BOOL CInput::DInputRelease(void) {
    DInputReleaseDevices();
    m_lpDirectInput7 = NULL;
    return TRUE;
};

// FUNCTION: CMR2 0x0049fd90
void CInput::DInputReleaseDevices(void) {
    int i;

    if (m_pDirectInputKeyboard != NULL) {
        Input_CloseDevice(m_pDirectInputKeyboard);
        m_pDirectInputKeyboard = NULL;
    }

    if (m_pDirectInputMouse != NULL) {
        Input_CloseDevice(m_pDirectInputMouse);
        m_pDirectInputMouse = NULL;
    }

    CountAttachedInputDevices();

    for (i = 0; i < 4; i++) {
        if (m_unk0x0059f6b0[i] != NULL && i < m_unk0x0059f8cc.field_0x3) {
            Input_CloseDevice(m_unk0x0059f6b0[i]);
            m_unk0x0059f6b0[i] = NULL;
        }
    }
}

// FUNCTION: CMR2 0x0049ef90
int CInput::CountAttachedInputDevices(void) {
  int i;
  int count;

  m_unk0x0059f8cc.field_0x3 = 0;
  count = Input_GetJoystickCount();
  for (i = 0; i < count; i++)
      CountJoystickEnumerationCallback();
  return m_unk0x0059f8cc.field_0x1 + m_unk0x0059f8cc.field_0x3;
}

// FUNCTION: CMR2 0x0049f6b0
BOOL CInput::CountJoystickEnumerationCallback(void) {
    m_unk0x0059f8cc.field_0x3++;
    return TRUE;
}

// match 89%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049f0e0
BOOL CInput::SetupKeyboard(void) {
    unsigned int uVar1;
    int iVar3 = 0;

    do {
        uVar1 = m_unk0x0059f8cc.field_0x0;
        m_availableDevices[uVar1].field_0x0 = 1;
        m_availableDevices[uVar1].field_0x18 = FALSE;

        ((int (__cdecl *)(char *, const char *))sprintf)(m_availableDevices[uVar1].deviceInstanceName, m_strKeyboard);
        ((int (__cdecl *)(char *, const char *))sprintf)(m_availableDevices[m_unk0x0059f8cc.field_0x0].deviceProductName, m_strKeyboard);
        
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

        m_unk0x0059f8cc.field_0x0 = m_unk0x0059f8cc.field_0x0 + 1;
        iVar3++;
    } while (iVar3 < 2);

    // PORT: the system repeat delay and interval (SystemParametersInfo).
    {
        DWORD delay, interval;

        Input_GetKeyRepeat(&delay, &interval);
        m_keyboardDelay = (PVOID)(uintptr_t)delay;
        m_keyboardSpeed = (PVOID)(uintptr_t)interval;
    }

    m_pDirectInputKeyboard = DInputCreateDevice(0, 0);
    if (m_pDirectInputKeyboard != NULL)
        return TRUE;

    return FALSE;
}

// FUNCTION: CMR2 0x0049f060
// PORT: no buffered data or granularity (it was 1 for every mouse).
void CInput::SetupMouse(void) {
    m_pDirectInputMouse = DInputCreateDevice(1, 0);
    m_mouseGranularity = 1;
    SetMouseCoopLevel(1);
}

// FUNCTION: CMR2 0x0049f000
void CInput::SetMouseCoopLevel(BOOL param1) {
    if (m_pDirectInputMouse != NULL)
        Input_SetMouseExclusive(m_pDirectInputMouse, param1 && g_pGraphics->isFullscreen);
}

// FUNCTION: CMR2 0x0049f690
BOOL CInput::GetAttachedJoysticks(void) {
    int count;
    int i;

    count = Input_GetJoystickCount();
    for (i = 0; i < count; i++)
        SetupJoystick(i);
    return TRUE;
}

// DirectInput enumeration callback for one joystick: creates the device, names
// its buttons (DIJOFS_BUTTON) and the four directions of its first POV hat
// (DIJOFS_POV(0)), and enables autocentre-off force feedback when present.
// match 86%: the axis loop keeps two counters in the original (i, axisID).
// FUNCTION: CMR2 0x0049f6d0
// PORT: called for each joystick SDL reports.
BOOL CInput::SetupJoystick(int joystickIndex) {
    unsigned int uVar2;
    DeviceInfo *pDeviceInfo;
    InputDevice *pDevice;
    BYTE iVar6[4];
    int iVar10, axisID, iVar11;
    int count;
    int offset;
    JoystickBinding * joystickBinding;
    InputCaps devCaps;
    char objectName[260];

    uVar2 = m_unk0x0059f8cc.field_0x0;
    pDeviceInfo = &m_availableDevices[m_unk0x0059f8cc.field_0x0];
    {
        pDeviceInfo->field_0x0 = 3;
        pDevice = DInputCreateDevice(2, joystickIndex);
        m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2] = pDevice;

        if (pDevice != NULL) {
            strncpy(pDeviceInfo->deviceInstanceName, Input_GetInstanceName(pDevice), sizeof(pDeviceInfo->deviceInstanceName));
            strncpy(pDeviceInfo->deviceProductName, Input_GetProductName(pDevice), sizeof(pDeviceInfo->deviceProductName));
        
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
                    }

                    iVar10++;
                    axisID++;
                    joystickBinding ++;
                } while (iVar10 < pDeviceInfo->joystick.controlCount);
            }

            Input_GetCaps(m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2], &devCaps);

            {
                if (devCaps.forceFeedback) {
                    pDeviceInfo->unk_isJoystick = TRUE;

                    Input_SetAutocenter(m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2], FALSE);
                    InitializeForceFeedbackDevice(m_unk0x0059f8cc.field_0x0, m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2]);
                } else {
                    pDeviceInfo->unk_isJoystick = FALSE;
                }

                // Button names (DIJOFS_BUTTON(i) = 0x30 + i).
                count = devCaps.buttons;
                pDeviceInfo->field_0x14 = 0;
                for (iVar10 = 0; iVar10 < count; iVar10++) {
                    if (Input_GetObjectName(m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2], iVar10 + 0x30, objectName, sizeof(objectName))) {
                        strncpy(pDeviceInfo->field_0x284[iVar10], objectName, 20);
                        pDeviceInfo->field_0x14++;
                    }
                }

                // The first POV hat (DIJOFS_POV(0) = 0x20) gives four direction names.
                count = devCaps.povs;
                iVar10 = 0;
                if (count > 0) {
                    offset = 0x20;
                    do {
                        if (offset > 0x20)
                            break;
                        if (Input_GetObjectName(m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2], offset, objectName, sizeof(objectName))) {
                            strncpy(pDeviceInfo->field_0x414, objectName, 0x11);
                            strcat(pDeviceInfo->field_0x414, m_strL);

                            strncpy(pDeviceInfo->field_0x425, objectName, 0x11);
                            strcat(pDeviceInfo->field_0x425, m_strR);

                            strncpy(pDeviceInfo->field_0x436, objectName, 0x11);
                            strcat(pDeviceInfo->field_0x436, m_strU);

                            strncpy(pDeviceInfo->field_0x447, objectName, 0x11);
                            strcat(pDeviceInfo->field_0x447, m_strD);
                        }
                        iVar10++;
                        offset += 4;
                    } while (iVar10 < count);
                }

                iVar10 = 0;

                if (pDeviceInfo->field_0x14 > 0) {
                    int * pUnk0x1c = &pDeviceInfo->field_0x1c;
                    do {
                        *pUnk0x1c++ = 1u << iVar10;
                        iVar10++;
                    } while (iVar10 < pDeviceInfo->field_0x14);
                }

                m_unk0x0059f8cc.field_0x2++;
                m_unk0x0059f8cc.field_0x0++;
            }
        }
    }

    return TRUE;
}

// FUNCTION: CMR2 0x0049fad0
void CInput::SetupJoystickDeviceInfo(DeviceInfo *deviceInfo) {
    USHORT * unk0x00511400;
    JoystickBinding * bindings;

    deviceInfo->joystick.controlCount = 0;
    unk0x00511400 = m_unk0x00511400;
    bindings = deviceInfo->joystick.bindings;

    do {
        // PORT: the axis exists when it has a range (DIPROP_RANGE succeeded).
        if (Input_HasAxis(m_unk0x0059f6b0[deviceInfo->field_0x18], *unk0x00511400)) {
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
    } while ((int)(intptr_t)unk0x00511400 < (int)(intptr_t)(m_unk0x00511400 + 8));
}

// FUNCTION: CMR2 0x0049ee10
void CInput::SetJoystickAxisRange(int deviceID, int axisID, DWORD range) {
    DeviceInfo * pDeviceInfo = &m_availableDevices[deviceID];

    Input_SetAxisRange(m_unk0x0059f6b0[pDeviceInfo->field_0x18], m_unk0x00511400[axisID], -(LONG)range, range);
    pDeviceInfo->joystick.bindings[axisID].range = range;
}

// FUNCTION: CMR2 0x0049ee90
void CInput::SetJoystickAxisDeadzone(int deviceID, int axisID, DWORD deadzone) {
    DeviceInfo * pDeviceInfo = &m_availableDevices[deviceID];

    Input_SetAxisDeadzone(m_unk0x0059f6b0[pDeviceInfo->field_0x18], m_unk0x00511400[axisID], deadzone);
    pDeviceInfo->joystick.bindings[axisID].deadzone = deadzone;
}

// FUNCTION: CMR2 0x0049ef10
void CInput::SetJoystickAxisSaturation(int deviceID, int axisID, DWORD saturation) {
    DeviceInfo * pDeviceInfo = &m_availableDevices[deviceID];

    Input_SetAxisSaturation(m_unk0x0059f6b0[pDeviceInfo->field_0x18], m_unk0x00511400[axisID], saturation);
    pDeviceInfo->joystick.bindings[axisID].saturation = saturation;
}

// FUNCTION: CMR2 0x004aae20
BOOL CInput::InitializeForceFeedbackDevice(int deviceID, InputDevice *pDevice) {
    if (m_forceFeedbackDevices[deviceID].field_0x0 == 0) {
        m_forceFeedbackDevices[deviceID].device = pDevice;
        if (g_pGraphics->isFullscreen)
            Input_StopAllEffects(pDevice);

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
bool CInput::IsForceFeedbackCallSuccessful(HRESULT hr) {
    return hr == 0;
}

// FUNCTION: CMR2 0x004aafd0
void CInput::SetForceFeedbackAutocenter(DWORD param1, int deviceID) {
    if (m_forceFeedbackDevices[deviceID].field_0x0 != FALSE)
        Input_SetAutocenter(m_forceFeedbackDevices[deviceID].device, param1 != 0);
}

// 96.55% match, only concern is this
// 0x4aaf35	-cmp edi, 0x666ed4
//          +cmp edi, CInput::m_dinputRefGuidKeyboard (DATA) (Input.cpp:560) <-- why  are you that
// FUNCTION: CMR2 0x004aaf00
void CInput::ResetForceFeedbackEffects(void) {
    int device;
    int i;

    for (device = 0; device < 8; device++) {
        ForceFeedbackDevice *pDevice = &m_forceFeedbackDevices[device];

        if (pDevice->field_0x0 != FALSE) {
            for (i = 0; i < 10; i++) {
                if (pDevice->effects[i] != NULL) {
                    Input_DestroyEffect(pDevice->effects[i]);
                    pDevice->effects[i] = NULL;
                }
            }
            pDevice->field_0x8 = 0;
        }
    }
}

// 96.77% match, only concern is this
// 0x4aaeda	-cmp edi, 0x666ec8
// 	        +cmp edi, CInput::m_dinputRefGuidKeyboard (DATA) (Input.cpp:587)  <-- why  are you that
// FUNCTION: CMR2 0x004aaea0
BOOL CInput::ResetForceFeedbackEffectsAlt(void) {
    int device;
    int i;

    for (device = 0; device < 8; device++) {
        ForceFeedbackDevice *pDevice = &m_forceFeedbackDevices[device];

        if (pDevice->field_0x0 != FALSE) {
            for (i = 0; i < 10; i++) {
                if (pDevice->effects[i] != NULL) {
                    Input_DestroyEffect(pDevice->effects[i]);
                    pDevice->effects[i] = NULL;
                }
            }
            pDevice->field_0x8 = 0;
            pDevice->field_0x0 = 0;
        }
    }
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

            ApplyControllerButtonBindings(0);
            ApplyControllerButtonBindings(1);
        }
    }
}

// FUNCTION: CMR2 0x0040be90
void CInput::ApplyControllerButtonBindings(unsigned short param1) {
    int uVar1;
    unsigned int raw;
    unsigned short uVar2;
    ControllerData * pController;

    raw = *(unsigned int *)&param1;
    uVar2 = param1;
    uVar1 = m_unk0x005168f4[uVar2];
    pController = &m_controllerInfo[uVar1];

    if (uVar1 > 1) {
        AssignUnmappedControllerActions(raw, pController);
    }

    SetDeviceKeyBinding(uVar2, 1, pController->field_0x13e[0]);
    SetDeviceKeyBinding(uVar2, 2, pController->field_0x13e[1]);
    SetDeviceKeyBinding(uVar2, 4, pController->field_0x13e[2]);
    SetDeviceKeyBinding(uVar2, 8, pController->field_0x13e[3]);
    SetDeviceKeyBinding(uVar2, 0x10, pController->field_0x13e[4]);
    SetDeviceKeyBinding(uVar2, 0x20, pController->field_0x13e[5]);
    SetDeviceKeyBinding(uVar2, 0x40, pController->field_0x13e[6]);
    SetDeviceKeyBinding(uVar2, 0x80, pController->field_0x13e[7]);
    SetDeviceKeyBinding(uVar2, 0x100, pController->field_0x13e[8]);
}

// FUNCTION: CMR2 0x0040c440
void CInput::AssignUnmappedControllerActions(unsigned int param1, ControllerData* param2) {
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

                bVar1 = FindFirstClearBindingBit(combinedFlags & 0xffff);
                
                uVar4 = 1 << bVar1;
                
                SetDeviceKeyBinding(param1 & 0xffff, uVar4 & 0xffff, param2->field_0x13e[iVar3]);
                SetDeviceKeyBinding(param1 & 0xffff, (1 << iVar3) & 0xffff, 0);
                
                *puVar2 = uVar4;
            }
        }
        
        iVar3++;
        puVar2++;
    } while (iVar3 < 10);
}

// FUNCTION: CMR2 0x0049eb90
void CInput::SetDeviceKeyBinding(int param1, unsigned int param2, unsigned int param3) {
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
BYTE CInput::FindFirstClearBindingBit(unsigned int param1) {
    int iVar1 = 0;

    while (iVar1 < 32) {
        if (!(param1 & 1)) break;
        
        param1 >>= 1;
        iVar1++;
    }

    return iVar1;
}

// FUNCTION: CMR2 0x0049eb50
void CInput::RebuildAvailableInputDevices(void)
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
    if ((m_keyboardState[INPUT_KEY_LSHIFT] & 0x80) == 0 && (m_keyboardState[INPUT_KEY_RSHIFT] & 0x80) == 0)
        return FALSE;
    return TRUE;
}

// FUNCTION: CMR2 0x0049efc0
void CInput::ClearFirstJoystickControlBindings(void)
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
    if (m_pDirectInputKeyboard != NULL)
        Input_ReadKeyboard(m_pDirectInputKeyboard, m_keyboardState);
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
void CInput::SetInputRepeatTimingParameters(DWORD p1, DWORD p2, DWORD p3, DWORD p4, DWORD p5)
{
    m_unk0x0059f8f8 = p1;
    m_unk0x0059f910 = p2;
    m_unk0x0059f8f4 = p3;
    m_unk0x0059f8f0 = p4;
    m_unk0x0059f90c = p5;
}

// FUNCTION: CMR2 0x0049ffc0
void CInput::SetInputRepeatTimingState(DWORD param1)
{
    m_unk0x0059f900 = param1;
}

// FUNCTION: CMR2 0x0040be00
DWORD CInput::GetControllerField114(unsigned int param1)
{
    return m_controllerInfo[m_unk0x005168f4[(unsigned short)param1]].field_0x114;
}

// FUNCTION: CMR2 0x0040be30
DWORD CInput::GetControllerField120(unsigned int param1)
{
    return m_controllerInfo[m_unk0x005168f4[(unsigned short)param1]].field_0x120;
}

// FUNCTION: CMR2 0x0040be60
DWORD CInput::GetControllerField124(unsigned short param1)
{
    return m_controllerInfo[m_unk0x005168f4[(unsigned short)param1]].field_0x124;
}

// FUNCTION: CMR2 0x0040c210
unsigned int CInput::GetEnabledControllerAxisBinding(unsigned int param1, int param2)
{
    unsigned int controller;

    controller = m_unk0x005168f4[(unsigned short)param1];
    if (m_controllerInfo[controller].field_0x210[param2].field_0x0 != 0)
        return m_controllerInfo[controller].field_0x2d8[param2];
    return -1;
}

// FUNCTION: CMR2 0x0040c270
BOOL CInput::IsControllerActionBound(int param1, ControllerData *param2)
{
    if ((&param2->field_0x128)[param1] != 0 && param2->field_0x13e[param1] != 0)
        return TRUE;
    return FALSE;
}

// FUNCTION: CMR2 0x004aaf50
void CInput::SetForceFeedbackDeviceValue(DWORD param1, int index)
{
    m_unk0x00666ec8[index] = param1;
}

// FUNCTION: CMR2 0x004aaf70
void CInput::StartForceFeedbackEffect(int effectIndex, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    InputEffect *pEffect;

    pDevice = &m_forceFeedbackDevices[deviceIndex];
    if (pDevice->field_0x0 != 0) {
        pEffect = pDevice->effects[effectIndex];
        if (pEffect != NULL && !Input_IsEffectPlaying(pEffect))
            Input_StartEffect(pEffect);
    }
}

// FUNCTION: CMR2 0x004ab600
char *CInput::FormatString(LPCSTR format, ...)
{
    va_list args;

    va_start(args, format);
    vsprintf(m_formatBuffer, format, args);
    return m_formatBuffer;
}

// FUNCTION: CMR2 0x0049f360
void CInput::ReadMouse(DeviceInfo *pDevice)
{
    InputMouseState mouseState;
    int i;

    pDevice->field_0x4 = 0;
    if (m_pDirectInputMouse == NULL || !Input_ReadMouse(m_pDirectInputMouse, &mouseState))
        return;

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
        if (m_keyboardState[INPUT_KEY_LEFT] & 0x80)
            pDevice->field_0x4 = 1;
        if (m_keyboardState[INPUT_KEY_RIGHT] & 0x80)
            pDevice->field_0x4 |= 0x2;
        if (m_keyboardState[INPUT_KEY_UP] & 0x80)
            pDevice->field_0x4 |= 0x4;
        if (m_keyboardState[INPUT_KEY_DOWN] & 0x80)
            pDevice->field_0x4 |= 0x8;
        if (m_keyboardState[INPUT_KEY_RETURN] & 0x80)
            pDevice->field_0x4 |= 0x10;
        if (m_keyboardState[INPUT_KEY_ESCAPE] & 0x80)
            pDevice->field_0x4 |= 0x20;
        if (m_keyboardState[INPUT_KEY_F1] & 0x80)
            pDevice->field_0x4 |= 0x1000;
        if (m_keyboardState[INPUT_KEY_F2] & 0x80)
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
    InputJoystickState joyState;
    InputDevice *pJoystick;
    int i;

    pJoystick = m_unk0x0059f6b0[pDevice->field_0x18];
    pDevice->field_0x4 = 0;
    if (pJoystick == NULL || !Input_ReadJoystick(pJoystick, &joyState))
        return;

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

    if ((joyState.rgdwPOV[0] & 0xffff) != 0xffff) {
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
void CInput::SetIndexedWordBinding(unsigned short *values, int index, unsigned short value)
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
void CInput::SetIndexedByteBinding(BYTE *values, int index, BYTE value)
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
// PORT: the condition parameters go into the effect's InputEffectParams.
int CInput::CreateForceFeedbackEffect(int effectType, DWORD duration, LONG coefficient, LONG offset, int triggerButton, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    InputEffectParams *pParams;
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

    slot = deviceIndex * 10 + i;
    pParams = &m_forceFeedbackEffects[slot];
    memset(pParams, 0, sizeof(*pParams));
    switch (effectType) {
    case 0xb:
        pParams->type = INPUT_EFFECT_SPRING;
        break;
    case 0xc:
        pParams->type = INPUT_EFFECT_INERTIA;
        break;
    case 0xd:
        pParams->type = INPUT_EFFECT_DAMPER;
        break;
    case 0xe:
        pParams->type = INPUT_EFFECT_FRICTION;
        break;
    }
    pParams->offset = offset;
    pParams->coefficient = coefficient;
    pParams->saturation = 10000;
    pParams->deadBand = 0;
    pParams->duration = duration;
    pParams->gain = 10000;
    pParams->triggerButton = triggerButton == -1 ? INPUT_EFFECT_NO_TRIGGER : INPUT_OFFSET_BUTTON(triggerButton);

    pDevice->effects[i] = Input_CreateEffect(pDevice->device, pParams);
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
    int index;
    ControllerData *pController;
    short mapping;

    mapping = 0;
    index = m_unk0x005168f4[controller] * sizeof(ControllerData);
    switch (button) {
    case 0: mapping = *(short *)((BYTE *)&m_controllerInfo[0].field_0x128 + index); break;
    case 1: mapping = *(short *)((BYTE *)&m_controllerInfo[0].field_0x12a + index); break;
    case 2: mapping = *(short *)((BYTE *)&m_controllerInfo[0].field_0x12c + index); break;
    case 3: mapping = *(short *)((BYTE *)&m_controllerInfo[0].field_0x12e + index); break;
    case 4: mapping = *(short *)((BYTE *)&m_controllerInfo[0].field_0x130 + index); break;
    case 5: mapping = *(short *)((BYTE *)&m_controllerInfo[0].field_0x132 + index); break;
    case 6: mapping = *(short *)((BYTE *)&m_controllerInfo[0].field_0x134 + index); break;
    case 7: mapping = *(short *)((BYTE *)&m_controllerInfo[0].field_0x136 + index); break;
    case 8: mapping = *(short *)((BYTE *)&m_controllerInfo[0].field_0x138 + index); break;
    case 9: mapping = *(short *)((BYTE *)&m_controllerInfo[0].field_0x13a + index); break;
    default: goto defaults;
    }

    if (mapping == 0) {
defaults:
        if (controller == 0 || controller == 1)
            goto fixed;
    }

    pController = (ControllerData *)((BYTE *)m_controllerInfo + index);
    if (!IsControllerActionBound(button, pController))
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
    InputEffectParams *pParams;

    pDevice = &m_forceFeedbackDevices[deviceIndex];
    if (pDevice->field_0x0 != 0 && pDevice->effects[effectIndex] != NULL) {
        pParams = &m_forceFeedbackEffects[deviceIndex * 10 + effectIndex];
        pParams->gain = gain;
        Input_UpdateEffect(pDevice->effects[effectIndex], pParams);
        return 0;
    }
    return -1;
}

// FUNCTION: CMR2 0x004ab0b0
HRESULT CInput::SetEffectGainAndDirection(int effectIndex, DWORD gain, LONG direction, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    InputEffectParams *pParams;

    pDevice = &m_forceFeedbackDevices[deviceIndex];
    if (pDevice->field_0x0 != 0 && pDevice->effects[effectIndex] != NULL) {
        pParams = &m_forceFeedbackEffects[deviceIndex * 10 + effectIndex];
        pParams->gain = gain;
        pParams->direction = direction;
        Input_UpdateEffect(pDevice->effects[effectIndex], pParams);
        return 0;
    }
    return -1;
}

// FUNCTION: CMR2 0x004ab150
int CInput::CreateConstantForceEffect(DWORD duration, LONG direction, LONG magnitude, DWORD attackTime, DWORD attackLevel, DWORD fadeTime, DWORD fadeLevel, int triggerButton, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    InputEffectParams *pParams;
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

    // PORT: the original built its DIEFFECT on the stack; the port keeps the
    // parameters in the effect's slot so later gain/direction changes have
    // them.
    pParams = &m_forceFeedbackEffects[deviceIndex * 10 + i];
    memset(pParams, 0, sizeof(*pParams));
    pParams->type = INPUT_EFFECT_CONSTANT;
    pParams->magnitude = magnitude;
    pParams->attackTime = attackTime;
    pParams->attackLevel = attackLevel;
    pParams->fadeTime = fadeTime;
    pParams->fadeLevel = fadeLevel;
    pParams->direction = direction;
    pParams->gain = 10000;
    pParams->duration = duration;
    pParams->triggerButton = triggerButton == -1 ? INPUT_EFFECT_NO_TRIGGER : INPUT_OFFSET_BUTTON(triggerButton);

    pDevice->effects[i] = Input_CreateEffect(pDevice->device, pParams);
    return i;
}

// FUNCTION: CMR2 0x004ab2b0
HRESULT CInput::SetConditionCoefficient(int effectIndex, LONG coefficient, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    InputEffectParams *pParams;

    pDevice = &m_forceFeedbackDevices[deviceIndex];
    if (pDevice->field_0x0 != 0 && pDevice->effects[effectIndex] != NULL) {
        pParams = &m_forceFeedbackEffects[deviceIndex * 10 + effectIndex];
        pParams->offset = 0;
        pParams->coefficient = coefficient;
        pParams->saturation = 10000;
        pParams->deadBand = 0;
        Input_UpdateEffect(pDevice->effects[effectIndex], pParams);
        return 0;
    }
    return -1;
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

// Key labels of the keyboard controls: the first keyboard uses text for the
// cursor keys and [ ] C R, the second one the numeric keypad keys.
// GLOBAL: CMR2 0x00516928
char g_keypad7[] = "7";
// GLOBAL: CMR2 0x0051692c
char g_keypad9[] = "9";
// GLOBAL: CMR2 0x00516930
char g_keypad3[] = "3";
// GLOBAL: CMR2 0x00516934
char g_keypad2[] = "2";
// GLOBAL: CMR2 0x00516938
char g_keypad8[] = "8";
// GLOBAL: CMR2 0x0051693c
char g_keypad6[] = "6";
// GLOBAL: CMR2 0x00516940
char g_keypadFormat[] = "%s %s";
// GLOBAL: CMR2 0x00516948
char g_keypad4[] = "4";
// GLOBAL: CMR2 0x0051694c
char g_keyR[] = "R";
// GLOBAL: CMR2 0x00516950
char g_keyC[] = "C";
// GLOBAL: CMR2 0x00516954
char g_keyOpenBracket[] = "[";
// GLOBAL: CMR2 0x00516958
char g_keyCloseBracket[] = "]";

// The five-dword joystick axis bindings of a device start at 0x46c.
#define DEVICE_BINDING(pDevice, i) ((ControllerDataUnk0x210 *)((BYTE *)(pDevice) + 0x46c + (i) * 0x14))

// Fills controller slot index from a detected device: its name, type flags,
// the default bindings and the texts shown for them in the controls menu.
// FUNCTION: CMR2 0x0040c610
void CInput::InitDetectedControllerSlot(DeviceInfo *pDevice, int index)
{
    ControllerData *pController;

    pController = &m_controllerInfo[index];
    pController->index = (BYTE)index;
    ((int (__cdecl *)(char *, const char *))sprintf)(pController->name, pDevice->deviceInstanceName);
    pController->field_0x10c = (pDevice->field_0x0 == 3 || pDevice->field_0x0 == 2) ? 1 : 0;
    pController->field_0x110 = (pDevice->field_0x0 == 3 || pDevice->field_0x0 == 2) ? 1 : 0;
    pController->field_0x114 = (pDevice->field_0x0 == 3 || pDevice->field_0x0 == 2) ? 1 : 0;
    pController->field_0x118 = pDevice->unk_isJoystick;
    m_controllerInfo[index].field_0x4 = 0x8000;
    m_controllerInfo[index].field_0x0 = 0x8000;
    if (pDevice->field_0x0 == 1) {
        if (index == 0) {
            m_controllerInfo[0].field_0x13e[0] = pDevice->keyboard.field_0x468;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[0], CFrontend::GetTextString(0x1f9));
            m_controllerInfo[0].field_0x13e[1] = pDevice->keyboard.field_0x469;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[1], CFrontend::GetTextString(0x1fa));
            m_controllerInfo[0].field_0x13e[2] = pDevice->keyboard.field_0x46a;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[2], CFrontend::GetTextString(0x1fb));
            m_controllerInfo[0].field_0x13e[3] = pDevice->keyboard.field_0x46b;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[3], CFrontend::GetTextString(0x1fc));
            m_controllerInfo[0].field_0x13e[4] = pDevice->keyboard.field_0x46c;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[4], CFrontend::GetTextString(0x1fd));
            m_controllerInfo[0].field_0x13e[5] = pDevice->keyboard.field_0x46d;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[5], g_keyCloseBracket);
            m_controllerInfo[0].field_0x13e[6] = pDevice->keyboard.field_0x46e;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[6], g_keyOpenBracket);
            m_controllerInfo[0].field_0x13e[7] = pDevice->keyboard.field_0x46f;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[7], g_keyC);
            m_controllerInfo[0].field_0x13e[8] = pDevice->keyboard.field_0x470;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[8], g_keyR);
            return;
        }
        pController->field_0x13e[0] = pDevice->keyboard.field_0x468;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[0], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad4));
        pController->field_0x13e[1] = pDevice->keyboard.field_0x469;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[1], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad6));
        pController->field_0x13e[2] = pDevice->keyboard.field_0x46a;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[2], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad8));
        pController->field_0x13e[3] = pDevice->keyboard.field_0x46b;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[3], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad2));
        pController->field_0x13e[4] = pDevice->keyboard.field_0x46c;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[4], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad3));
        pController->field_0x13e[5] = pDevice->keyboard.field_0x46d;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[5], CFrontend::GetTextString(0x204));
        pController->field_0x13e[6] = pDevice->keyboard.field_0x46e;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[6], CFrontend::GetTextString(0x205));
        pController->field_0x13e[7] = pDevice->keyboard.field_0x46f;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[7], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad9));
        pController->field_0x13e[8] = pDevice->keyboard.field_0x470;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[8], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad7));
        return;
    }
    if (pDevice->field_0x0 == 2) {
        *(DWORD *)DEVICE_BINDING(pDevice, 0) = 1;
        pController->field_0x210[0] = *DEVICE_BINDING(pDevice, 0);
        pController->field_0x2d8[0] = 0;
        pController->field_0x210[1] = *DEVICE_BINDING(pDevice, 0);
        pController->field_0x2d8[1] = 0;
        pController->field_0x128 = 0;
        pController->field_0x12a = 0;
        pController->field_0x12c = 0x10;
        pController->field_0x12e = 0x20;
        pController->field_0x130 = 0;
        pController->field_0x132 = 0;
        pController->field_0x134 = 0;
        pController->field_0x136 = 0;
        pController->field_0x138 = 0;
        pController->field_0x13a = 0;
        pController->field_0x13e[4] = (BYTE)*(DWORD *)DEVICE_BINDING(pDevice, 0);
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[4], CFrontend::GetTextString(0x1fe));
        pController->field_0x13e[5] = pDevice->keyboard.field_0x46d;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[5], CFrontend::GetTextString(0x1fe));
        pController->field_0x13e[6] = pDevice->keyboard.field_0x46e;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[6], CFrontend::GetTextString(0x1fe));
        pController->field_0x13e[7] = pDevice->keyboard.field_0x46f;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[7], CFrontend::GetTextString(0x1fe));
        pController->field_0x13e[8] = pDevice->keyboard.field_0x470;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[8], CFrontend::GetTextString(0x1fe));
        return;
    }
    if (pController->field_0x10c != 0) {
        pController->field_0x128 = 1;
        pController->field_0x12a = 2;
        if (*(DWORD *)DEVICE_BINDING(pDevice, 0) != 0) {
            pController->field_0x210[0] = *DEVICE_BINDING(pDevice, 0);
            pController->field_0x2d8[0] = 0;
            if (*(DWORD *)DEVICE_BINDING(pDevice, 0) != 0) {
                pController->field_0x210[1] = *DEVICE_BINDING(pDevice, 0);
                pController->field_0x2d8[1] = 0;
            }
        }
        pController->field_0x12c = 4;
        if (*(DWORD *)DEVICE_BINDING(pDevice, 1) != 0) {
            pController->field_0x210[2] = *DEVICE_BINDING(pDevice, 1);
            pController->field_0x2d8[2] = 1;
        }
        pController->field_0x12e = 8;
        if (*(DWORD *)DEVICE_BINDING(pDevice, 1) != 0) {
            pController->field_0x210[3] = *DEVICE_BINDING(pDevice, 1);
            pController->field_0x2d8[3] = 1;
        }
        pController->field_0x130 = 0x10;
        pController->field_0x132 = 0x20;
        pController->field_0x134 = 0x40;
        pController->field_0x136 = 0x80;
        pController->field_0x138 = 0x100;
        pController->field_0x13a = 0x200;
        return;
    }
    memset(pController->field_0x210, 0, 0x32 * 4);
    pController->field_0x128 = 1;
    pController->field_0x12a = 2;
    pController->field_0x12c = 0x10;
    pController->field_0x12e = 0x20;
    pController->field_0x130 = 0x40;
    pController->field_0x132 = 0x80;
    pController->field_0x134 = 0x100;
    pController->field_0x136 = 0x200;
    pController->field_0x138 = 0x800;
    pController->field_0x13a = 0x400;
}

// FUNCTION: CMR2 0x0040c050
void CInput::RefreshControllerConfigurations(void)
{
    DeviceInfo *pDevice;
    int i;

    m_controllerCount = CountAttachedInputDevices();
    for (i = 0; i < m_controllerCount; i++) {
        pDevice = UpdateDevice(i);
        if (m_hasLoadedControllerInfo == 0 || strcmp(m_controllerInfo[i].name, pDevice->deviceInstanceName) != 0)
            InitDetectedControllerSlot(pDevice, i);
    }
    if (m_controllerCount < 6)
        memset(&m_controllerInfo[m_controllerCount], 0, (6 - m_controllerCount) * sizeof(ControllerData));
    m_hasLoadedControllerInfo = TRUE;
}

struct InputQueues {
    int characters[30];
    int keys[30];
};

// GLOBAL: CMR2 0x006ed3f4
InputQueues g_inputQueues;
#define g_unk0x006ed3f4 (g_inputQueues.characters)
#define g_unk0x006ed46c (g_inputQueues.keys)

// FUNCTION: CMR2 0x004b7c80
void Input_ClearCharacterQueue(void)
{
    int i;

    for (i = 0; i < 30; i++)
        g_unk0x006ed3f4[i] = 0;
}

// Queues one character for the input ring buffer.
// FUNCTION: CMR2 0x004b7ca0
void CInput::QueueInputCharacter(int param1)
{
    int *p;
    int i;

    i = 0;
    p = g_unk0x006ed3f4;
    while ((int)p < (int)(g_unk0x006ed3f4 + 30)) {
        if (*p == 0) {
            g_unk0x006ed3f4[i] = param1;
            return;
        }
        p++;
        i++;
    }
}

// Pops the oldest character of the input ring buffer.
// FUNCTION: CMR2 0x004b7cd0
bool Input_PopQueuedCharacter(int *pOut)
{
    int i;

    if (g_unk0x006ed3f4[0] != 0) {
        *pOut = g_unk0x006ed3f4[0];
        i = 0;
        do {
            g_unk0x006ed3f4[i] = g_unk0x006ed3f4[i + 1];
            i++;
        } while (i < 29);
        g_unk0x006ed3f4[29] = 0;
        return true;
    }
    return false;
}

// Queues one key press (only when the scan code carries a virtual key).
// FUNCTION: CMR2 0x004b7d10
void CInput::QueueVirtualKeyPress(unsigned int param1)
{
    int *p;
    int i;

    // PORT: param1 is the key code (the original got the WM_KEYDOWN lParam
    // and required its scan code byte to be set).
    if (param1 != 0) {
        i = 0;
        p = g_unk0x006ed46c;
        while (p < g_unk0x006ed46c + 30) {
            if (*p == 0) {
                g_unk0x006ed46c[i] = param1;
                return;
            }
            p++;
            i++;
        }
    }
}

// GLOBAL: CMR2 0x005320a4
int g_unk0x005320a4;

// FUNCTION: CMR2 0x0040af20
void CInput::UpdateInputFrameDelta(void)
{
    g_unk0x005320a4 = CMain::GetFrameDelta();
}

// FUNCTION: CMR2 0x0049eab0
void CInput::UpdateAllAvailableDevices(void)
{
    int i;

    for (i = 0; i < 8; i++)
        UpdateDevice(i);
}

// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040bc90
void CInput::SetControllerForceFeedbackValue(unsigned short param1, DWORD param2)
{
    unsigned short index;

    index = CInput::m_unk0x005168f4[param1];
    SetForceFeedbackDeviceValue(m_controllerInfo[index].field_0x11c = param2, index);
}


// FUNCTION: CMR2 0x0049ead0
DeviceInfo *CInput::GetAvailableDeviceRecord(int index)
{
    return &m_availableDevices[index];
}

// FUNCTION: CMR2 0x004b7d40
void Input_ClearKeyPressQueue(void)
{
    int i;

    for (i = 0; i < 30; i++)
        g_unk0x006ed46c[i] = 0;
}

// Pops the oldest key of the key press queue.
// FUNCTION: CMR2 0x004b7d60
int Input_PopQueuedKeyPress(int *pOut)
{
    int i;

    if (g_unk0x006ed46c[0] != 0) {
        *pOut = g_unk0x006ed46c[0];
        i = 0;
        do {
            g_unk0x006ed46c[i] = g_unk0x006ed46c[i + 1];
            i++;
        } while (i < 29);
        g_unk0x006ed46c[29] = 0;
        return 1;
    }
    return 0;
}

// Latched state of the two throttle axes of each device (pressed once until released).
// GLOBAL: CMR2 0x005334f4
int g_unk0x005334f4[8][2];

// Turns the joystick's accelerate/brake axes into one-shot "left"/"right"
// menu presses (bits 2 and 3) when they are pushed past a quarter.
// FUNCTION: CMR2 0x0040bad0
void Input_TranslatePedalsToMenuKeys(void)
{
    unsigned int i;
    DeviceInfo *pDev;

    for (i = 0; i < 8; i++) {
        pDev = CInput::GetAvailableDeviceRecord(i);
        if (pDev->field_0x0 == 3 && CInput::GetEnabledControllerAxisBinding(0, 2) != CInput::GetEnabledControllerAxisBinding(0, 3)) {
            pDev->field_0x8 &= 0xfffffff3;
            if (((int *)pDev)[CInput::GetEnabledControllerAxisBinding(0, 2) * 5 + 0x11f] < 0x3333) {
                if (g_unk0x005334f4[i][0] == 0) {
                    pDev->field_0x8 |= 4;
                    g_unk0x005334f4[i][0] = 1;
                }
            } else {
                g_unk0x005334f4[i][0] = 0;
            }
            if (((int *)pDev)[CInput::GetEnabledControllerAxisBinding(0, 3) * 5 + 0x11f] < 0x3333) {
                if (g_unk0x005334f4[i][1] == 0) {
                    pDev->field_0x8 |= 8;
                    g_unk0x005334f4[i][1] = 1;
                }
            } else {
                g_unk0x005334f4[i][1] = 0;
            }
        }
    }
}

// Controller slots are mapped to entries of m_controllerInfo through m_unk0x005168f4.
// FUNCTION: CMR2 0x0040bba0
unsigned short Input_GetAssignedController(void)
{
    return CInput::m_controllerCount;
}

// FUNCTION: CMR2 0x0040bbb0
ControllerData *Input_GetControllerTable(void)
{
    return CInput::m_controllerInfo;
}

// FUNCTION: CMR2 0x0040bbc0
unsigned short Input_GetControllerSlotMapping(unsigned short slot)
{
    return CInput::m_unk0x005168f4[slot];
}

// FUNCTION: CMR2 0x0040bbe0
void Input_SetControllerSlotMapping(unsigned short slot, unsigned short index)
{
    CInput::m_unk0x005168f4[slot] = index;
}

// FUNCTION: CMR2 0x0040bc00
unsigned int Input_GetControllerPrimaryFlags(unsigned short slot)
{
    return CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x0;
}

// FUNCTION: CMR2 0x0040bc30
unsigned int Input_GetControllerSecondaryFlags(unsigned short slot)
{
    return CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x4;
}

// FUNCTION: CMR2 0x0040bc60
void Input_SetControllerPrimaryFlags(unsigned short slot, unsigned int value)
{
    CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x0 = value;
}

// FUNCTION: CMR2 0x0040bcd0
DWORD Input_GetControllerField11C(unsigned short slot)
{
    return CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x11c;
}

// FUNCTION: CMR2 0x0040bd00
void Input_SetControllerSecondaryFlags(unsigned short slot, unsigned int value)
{
    CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x4 = value;
}

// FUNCTION: CMR2 0x0040bd30
DWORD Input_GetControllerField118(unsigned short slot)
{
    return CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x118;
}

// Merges the buttons of the joystick assigned to the slot into pOut.
// FUNCTION: CMR2 0x0040bd60
void Input_MergeAssignedJoystickButtons(int slot, DeviceInfo *pOut)
{
    DeviceInfo *pDev;

    if ((short)Input_GetControllerSlotMapping(slot) != 0 && (short)Input_GetControllerSlotMapping(slot) != 1) {
        pDev = CInput::GetAvailableDeviceRecord(Input_GetControllerSlotMapping(slot));
        if (pDev->field_0x0 == 0 || pDev->field_0x0 == 3) {
            pOut->field_0x8 |= pDev->field_0x8;
            pOut->field_0x4 |= pDev->field_0x4;
            pOut->field_0xc |= pDev->field_0xc;
        }
    }
}

// FUNCTION: CMR2 0x0040bdd0
DWORD Input_GetControllerField110(unsigned short slot)
{
    return CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x110;
}


int Input_PopQueuedKeyPress(int *pOut);

// Name of the last key pressed (waits until the key queue is empty).
// FUNCTION: CMR2 0x0049ed80
void Input_GetLastKeyName(LPSTR pName, unsigned int size)
{
    LONG key;

    key = 0;
    while (Input_PopQueuedKeyPress((int *)&key) != 0)
        ;
    // PORT: the queue holds key codes (DIK_*), named in the current layout.
    strncpy(pName, key != 0 ? Input_GetKeyName(key) : "", size & 0xff);
    if ((size & 0xff) != 0)
        pName[(size & 0xff) - 1] = 0;
}

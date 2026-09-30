#include "StageObjectCount.h"
#include <windows.h>
#include <stdlib.h>
#include "StageBlock.h"
#include <string.h>
#include "RallyData.h"
#include "SceneNode.h"
#include "Frontend.h"
#include "InstallInfo.h"
#include "Texture.h"
#include "StageTiming.h"
#include "AIHelper.h"
#include "RegKey.h"
#include <stdio.h>
#include <math.h>
#include "FixedPoint.h"
#include "Car.h"
#include "WheelTrail.h"
#include "GameInfo.h"
#include "Input.h"
#include "main.h"
#include "Game.h"
#include "GenericFileLoader.h"
#include "FileBuffer.h"
#include "Mesh.h"
#include "Graphics.h"
#include "Font.h"
#include "Sprite.h"
#include "Collision2D.h"
#include "Menu.h"
// --- module prototypes (address order; see tools/fastcmp/tuproto.py) ---
void FUN_00460330(int param_1, int view);
void FUN_00460390(int index, int view);
void FUN_00460a30(FixVector *pOut);
void FUN_00460b60(int *p, int unused);
BYTE FUN_00460bf0(int index);
int FUN_00460c10(int index);
void StageObject_SetScaledValue(int value, int index);
int FUN_00460c80(BYTE *pCar);
int FUN_00460ca0(int amount, Car *pCar);
void StageObject_SetLighting(const BYTE *pPrimary, const BYTE *pSecondary);
int FUN_004616c0(BYTE a, BYTE b, int t);
void FUN_00461710(BYTE *out, BYTE *from, BYTE *to, int t);
BYTE *FUN_00461830(unsigned short timeOfDay, int slot, BYTE **records);
void FUN_00461a30(int timePrimary, int timeSecondary, BYTE **records, BYTE **pPrimary, BYTE **pSecondary);
void FUN_00461a70(BYTE *pA, BYTE *pB);
void FUN_00461b30(BYTE *pObject, int type);
void FUN_00461bb0(int t);
void FUN_00461c30(int index);
void FUN_00462aa0(unsigned int param_1, int param_2);
void FUN_00462cb0(BYTE *pColour);
void FUN_00462d10(short *pRect);
void FUN_00462d80(int param_1, int param_2);
void StageLights_UpdateDirection(void);
int *FUN_00463270(int i, int j);
void StageLights_SetTransform(FixVector *pAxes);
void StageLights_LoadTextures(void);
void StageLights_Create(void);
void StageLights_Update(void);
void FUN_00463ce0(BYTE value);
void StageLights_Off(void);
void CarLights_LoadTextures(void);
void FUN_00463fe0(int param_1);
void FUN_004643f0(int param_1);
int FUN_004648f0(void);
void FUN_00464960(unsigned int param_1);
BYTE *FUN_00464af0(int index);
BYTE *FUN_00464b10(int view);
void FUN_00464b60(void);
void FUN_00464c60(int car);
void StageObject_UpdateSkidTrails(int carIndex);
void FUN_00465530(void);
void FUN_00465600(void);
void FUN_00465780(int param_1);
void FUN_004657d0(int car);
void FUN_004658e0(int index);
int StageObject_GetWheelSlip(int carIndex, int wheelIndex);
int FUN_00465e40(int car, int wheel);
int FUN_00465ea0(int value);
void FUN_00465f60(int frames, int samples);
void FUN_00465f80(void);
void FUN_00465f90(char *path);
void FUN_00465fc0(Car *pCar);
void FUN_00466030(int a, int b);
void FUN_00466080(void);
int FUN_00466090(void);
void FUN_004660a0(int **pValue, int slot, char flag);
void FUN_004660e0(BYTE value);
int FUN_004660f0(void);
void FUN_00466100(int param_1);
void FUN_00466360(void);
void FUN_00466490(void);
void FUN_004664c0(short *pOrder, short count);
void FUN_00466520(void);
void FUN_00466570(short *param_1, short param_2, int param_3, int param_4);
void FUN_00466630(int value);
void FUN_004675c0(Car *pCar, Car *pOther);
void FUN_004688b0(BYTE *p);
void FUN_00468a80(Car *pCar, int amount);
void FUN_00468c10(Car *pCar);
int FUN_00469100(Car *pCar, BYTE *pRecord);
void FUN_004694a0(int param_1, int param_2, char param_3);
void FUN_00469690(int param_1);
void FUN_00469bf0(Car *pCar, int index);
int FUN_00469c30(FixVector *pOut, int *pParts, int index);
int StageObject_IsEligibleType(short type, int mode, int category);
void FUN_00469e40(int param_1, short *param_2, short param_3);
void FUN_0046a500(int param_1);
void FUN_0046acb0(int param_1, int param_2, int param_3);
void FUN_0046afe0(int param_1, int param_2, int param_3);
void FUN_0046b400(int value, int index);
void FUN_0046b420(void);
void FUN_0046b440(Mesh **ppMeshes, int mesh, int vertex, int *pOut);
int FUN_0046b4c0(BYTE *pCar);
void FUN_0046b4e0(BYTE *pCar);
void FUN_0046b670(BYTE *pCar);
void FUN_0046b6b0(SceneNode *pNode, BYTE threshold);
void FUN_0046b6e0(SceneNode *pNode, BYTE threshold);
void FUN_0046b710(void);
void FUN_0046b740(int i, int value, int j);
void FUN_0046b760(int index, int reset);
void FUN_0046b790(int type, int car, int index);
void FUN_0046b8f0(Car *pCar);
void FUN_0046bb40(void);
int FUN_0046bd20(int i, int j);
BYTE FUN_0046bd40(int index);
int StageObject_UsesExtendedMode(void);
void FUN_0046bdc0(BYTE *pIn, BYTE *pOut, int active, int handbrake, int lightA, int lightB);
int FUN_0046bec0(int *pState, BYTE *pIn, BYTE *pOut, BYTE *pCounter, Car *pCar);
void FUN_0046c240(BYTE *pDst, BYTE car);
void FUN_0046c2a0(int param_1, BYTE param_2);
void FUN_0046c320(int *pState, BYTE car);
void FUN_0046c390(int *pState, BYTE car);
void FUN_0046c410(int param_1, BYTE param_2);
void FUN_0046c450(BYTE *pOut, BYTE car);
int FUN_0046c4b0(int *pState, BYTE *pIn, BYTE car, BYTE *pCounter);
void Replay_InitSlots(void);
BYTE *FUN_0046c5a0(short frames, short samples, int type);
int Replay_FreeBuffers(void);
void FUN_0046c750(int param_1, int param_2, int param_3);
void FUN_0046c8e0(void);
int FUN_0046cbe0(BYTE *packet, BYTE car);
int Replay_StopRecording(BYTE *pBuffer);
void FUN_0046cce0(int param_1, int param_2, int param_3, BYTE param_4);
void FUN_0046cfa0(int *pState);
void FUN_0046d270(void);
int FUN_0046d2a0(int *p);
BYTE *FUN_0046d2d0(char *path);
int Replay_Save(BYTE *pBuffer, char *pName);
void Replay_SetupPointers(BYTE *pBuffer, int unused);
int FUN_0046d500(void);
void FUN_0046d510(void);
void FUN_0046d5e0(void);
void FUN_0046d610(BYTE *p);
void FUN_0046d8d0(int param_1, int *param_2);
void FUN_0046de20(unsigned int *param_1, unsigned int *param_2, unsigned int *param_3, int *param_4, FixMatrix *param_5, int *param_6, unsigned int *param_7, int *param_8);
void FUN_0046e340(BYTE *pMatrices, BYTE *pInfo);
void FUN_0046e440(void);
void Events_Reset(void);
void Events_Init(int unused, int slot, char animate);
void Events_ComputeSteps(void);
void FUN_0046e780(void);
void FUN_0046ea10(int param_1);
void FUN_0046ea80(int param_1, int param_2);
void FUN_0046ec40(int index, int x, int y);
BYTE Events_Tick(int index);
void FUN_0046ed80(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6, int param_7, int param_8);
BYTE FUN_0046eeb0(int index, int wheel);
void Events_Flush(void);
void FUN_0046ef50(void);
BYTE FUN_0046f030(void);
void FUN_0046f060(void);
void FUN_0046f330(int param_1);
void FUN_0046f4a0(FixVector *pOut);
void FUN_0046f4c0(int *pOut);
void FUN_0046f4d0(int *pOut);
void FUN_0046f4e0(int *pOut1, int *pOut2);
int FUN_0046f500(void);
void FUN_0046f550(void);
void FUN_0046f7e0(void);
void StageObject_InitMovingObject(int *pState, int carIndex);
void FUN_0046fe70(int *param_1, int param_2, int param_3);
void FUN_00471950(int t);
void FUN_00471a60(int param_1);
void FUN_00471af0(void);
unsigned int FUN_00471bd0(BYTE **pOut);
void FUN_00472830(void);
void FUN_00472870(void);
void FUN_004728b0(void);
void FUN_004728c0(void);
int FUN_004728d0(void);
int FUN_004728e0(void);
int FUN_00472990(KnockoutMatch *pMatch);
BYTE FUN_004729f0(void);
void FUN_00472a30(void);
void FUN_00472ca0(void);
void FUN_00472cb0(void);
void FUN_00472e00(BYTE *param_1, unsigned int param_2);
int FUN_00473290(void);
int FUN_00473310(void);
unsigned int FUN_004735a0(unsigned int *pHigh);
int FUN_00473680(unsigned int *p);
char *FUN_004736b0(KnockoutMatch *pMatch, int side);
int FUN_00473790(KnockoutMatch *pMatch, int side);
int FUN_004737d0(KnockoutMatch *pMatch, int param2);
char *FUN_00473810(KnockoutMatch *pMatch, int side);
void FUN_004738f0(Menu *pMenu);
void FUN_00473d60(Menu *pMenu);
void FUN_00474420(char *text, int fraction, unsigned int font, int x, unsigned int y, int *pColour, unsigned int flags);
void FUN_004744f0(void);
void FUN_00474fe0(int param1, int param2, int param3, int param4, int param5, KnockoutMatch *param6);
char *FUN_004752f0(int *p, int index, int mode);
void FUN_00475430(int param1, KnockoutMatch *param2, short *param3, BYTE *param4, BYTE *param5, int param6, int param7, int *param8, int param9);
void FUN_00475740(short *pRect, BYTE *pColour, BYTE *pEdgeColour, int drawTexture);
int FUN_00475970(int scale, int unused, short *pRect, BYTE *pColour, int layer);
void FUN_004759d0(int unused1, int unused2);
void FUN_00475f00(void);
BYTE *FUN_00475f70(void);
void FUN_004760a0(int record, BYTE car);
void FUN_00476410(BYTE *p, int *src, int unused, BYTE value);
void FUN_004764e0(BYTE *p);
int FUN_00476520(BYTE index);
void FUN_00476540(int index);
void FUN_004765e0(BYTE *pObj, int a, int b);
void FUN_00476640(int car);
int FUN_00476850(int param_1, int param_2);
void FUN_00476a40(int index);
void FUN_00476c70(int index);
void FUN_00476e00(BYTE *param_1, int *param_2, int unused);
void FUN_00477340(int player);
void FUN_00477460(int index);
void FUN_004775f0(Texture *pTexture, int state, int cacheBase, int index);
void FUN_00477850(int object, int *src);
void FUN_004778b0(BYTE *object, int unused);
void FUN_00477a90(void);
void FUN_00477ac0(int index);
void FUN_00477b60(int car, int unused1, int unused2, BYTE flag);
void FUN_00477c20(int index, char set0, char set1, BYTE mask);
void FUN_00477c80(int lane, int *pA, int *pB, int slot);
void FUN_00477ce0(int car);
void FUN_00477f30(void);
void FUN_00478130(int index);
void FUN_00478150(int index);
void FUN_00478170(int index);
void FUN_00478190(int index, int seconds);
int FUN_004781c0(int index);
void FUN_004789b0(BYTE *pCar);
BYTE FUN_00478b80(void);
void FUN_0047aa60(int value);
void FUN_0047aa70(void);
void FUN_0047ad20(void);
void FUN_0047b000(int slot);
void FUN_0047b0e0(int player, int device);
void FUN_0047b620(int slot);
void FUN_0047b640(int slot);
void FUN_0047b7b0(int slot);
void FUN_0047b870(int index);
void FUN_0047b970(unsigned int param_1);
void FUN_0047bad0(unsigned int param_1, unsigned int param_2);
void FUN_0047bca0(int slot);
void FUN_0047bda0(void);
void FUN_0047bdc0(char restart);
void FUN_0047bdd0(Car *pCar, int car, int preview);
void FUN_0047c1b0(void);
void FUN_0047c1e0(char replay, char restart);
BYTE *FUN_0047c2f0(void);
int FUN_0047c5b0(int index);
void FUN_0047c5c0(void);
unsigned int FUN_0047c5e0(int param_1);
void FUN_0047c9a0(int param_1, int param_2, int *param_3);
void FUN_0047ca30(int param_1);
void FUN_0047cbc0(int unused, int table, int entry, int *pOut1, int *pOut2);
void FUN_0047cc30(void);
void StageObject_UpdateApproachingCar(int carIndex, int time);
int FUN_0047cd00(int index);
int FUN_0047cd10(int param_1, int *param_2, int param_3, int *param_4);
unsigned int FUN_0047d0e0(int index, BYTE *pOut, int *pCount, BYTE *pFlag);
int FUN_0047d330(int car, int preview);
void FUN_0047d510(void);
void FUN_0047d5a0(BYTE car);
void FUN_0047d850(Car *pCar, int *param_2);
void FUN_0047dd70(void);
void FUN_0047e1e0(int t);
void FUN_0047e490(BYTE *pColour);
void FUN_0047e4d0(BYTE count);
BYTE FUN_0047ea20(void);
void FUN_0047eab0(void);
void FUN_0047f510(int param_1, BYTE *pOut, BYTE *pFrom, BYTE *pTo);
void FUN_0047f740(void);
void StageObject_SpawnDebris(const FixVector *pPosition, const FixVector *pVelocity, unsigned int variant);
void FUN_00480220(void);
void FUN_00480380(void);
void FUN_004805f0(int value);
void StageObject_UpdateDebris(int scale);
void FUN_00480a50(void);
void FUN_00480a60(void);
void FUN_00480ac0(BYTE *pCar, int slot, int reset);
void FUN_00480af0(BYTE *pCar, BYTE *pObject, BYTE flag);
void FUN_00480b40(BYTE *pCar);
void FUN_00484310(void);
unsigned int FUN_00484d10(int i, int j);
BYTE *FUN_00484de0(BYTE *pCar, int slot);
void FUN_00484e00(int param_1, short count);
void FUN_004853c0(FixVector *pImpulse);
void FUN_004854a0(int index);
void FUN_00485690(short *pOrder, short count, int view);
void FUN_00485860(unsigned int index, int *pTarget, int flag);
int StageObject_DistanceFade(FixVector *delta);
void FUN_00486500(int scale);
void FUN_00486630(int list, int index, int value);
void FUN_004866a0(void);
void FUN_00486700(void);
void FUN_00486740(BYTE *pObj, int *pSrc, BYTE index, BYTE value);
void FUN_00486b20(BYTE *pCar, BYTE *pInfo);
void FUN_00486b90(BYTE *pCar, BYTE *pInfo);
void FUN_00486be0(BYTE *p, int unused);
void FUN_00486c00(BYTE *p, BYTE *q);
void FUN_00486c30(int *pObj, int *param2, int *param3, FixVector *pVerts);
void FUN_00486fc0(int *pMatrix, int *pOffset);
int FUN_00487130(void);
void FUN_00487140(int *param_1, int *param_2, int *param_3, int *param_4);
void FUN_004873f0(int *param_1, int param_2, int param_3);
void FUN_004877a0(BYTE *pCars, short *pOrder, short count);
int FUN_004878a0(Car *pCar);
int FUN_00487b80(int r1, int r2, int *pA, int *pB);
void FUN_00487c40(int *pMatrix, int param_2, int *pOffset);
void FUN_00487e00(FixVector *pPos, int *pInfo);
void FUN_00487e50(int *pBox, Car *pCar);
int FUN_00487f60(Car *pCar, int *pEntry, int *pBox, int *pObject);
int FUN_00488640(int *pBoxA, int *pBoxB, FixVector *pOffset, int scale);
int FUN_00488de0(FixVector *pVertsA, FixVector *pVertsB, FixVector *pDir, int *pDistance);
void FUN_0048c870(BYTE index, BYTE other, int *pDelta, int flag);
void FUN_0048c900(BYTE index);
BYTE *FUN_0048ca40(int index);
void FUN_0048ca70(void);
int FUN_0048ca90(void);
int FUN_0048caa0(int *pList);
void FUN_0048cae0(BYTE *pRecord, FixMatrix *pRef, int spot);
void FUN_0048cc30(BYTE *pRecord, FixMatrix *pRef);
void FUN_0048ce80(BYTE *pRecord, FixMatrix *pRef);
void FUN_0048d0f0(BYTE *pRecord, FixMatrix *pRef);
void FUN_0048d7b0(BYTE *pCar, BYTE *pInfo);
void FUN_0048d800(BYTE *pInfo, BYTE *pCar);
void FUN_0048d850(BYTE *pCar, BYTE *pInfo);
unsigned int FUN_0048d8b0(FixVector *pPos);
int FUN_0048d930(BYTE *p);
void FUN_0048d950(BYTE *pSurface, FixMatrix *pMatrix);
void FUN_0048db00(BYTE *p, int step);
void FUN_0048dc30(BYTE *pCar, int step);
void FUN_0048dca0(BYTE *pCar, int amount);
void FUN_0048dce0(FixVector *pOut, BYTE *pSurface, Car *pCar, FixMatrix *pMatrix);
int FUN_0048df10(BYTE *pCar);
void FUN_0048df50(Car *param_1);
void FUN_0048e0a0(Car *pCar, int param);
int FUN_0048e580(char type);
int FUN_0048e730(int *param_1, int *param_2, int param_3, char param_4);
int FUN_0048f400(void);
void FUN_004920d0(DWORD *pColour, DWORD *pReference);
void FUN_00492220(DWORD *pColour, DWORD *pReference);
void FUN_004926f0(int angle, int unused, int sunAngle);
void FUN_00492890(FixVector *pOut);
void FUN_004928c0(int *pOut1, int *pOut2, int *pOut3);
void FUN_004928f0(int *pOut);
void FUN_00492900(int value);
int FUN_00492910(void);
void FUN_00492b50(void);
void FUN_00492bb0(int *pOut);
void FUN_00492bd0(int view);
void FUN_00492e30(FixVector *pLight);
void FUN_00492fd0(int value);
int Track_GetGroundHeight5(FixVector *pPoint, FixVector *pNormal, short *pTri, short *pSurfaceClass, int defaultY);
void FUN_004930e0(int param_1, int param_2);
BYTE *FUN_00498570(int index);
int StageObject_Atan2Degrees(int y, int x);
int FUN_00498db0(int angle);
unsigned int FUN_0049e940(void);
int FUN_004b5320(void *pNode, int value);
// --- end module prototypes ---

struct GlowLight;
GlowLight *Glow_Add(int type, FixVector *pos, FixVector *dir, int unused1, int sizeX, int sizeY, int billboardTexture,
                    int layerTexture, int intensity, int node, BYTE projected, int unused2, int field_0x40);
void FUN_004ae3d0(BYTE *p, BYTE value);
void FUN_004ae3f0(BYTE *p, int value);
int FUN_00457e10(BYTE *pCar, int offset);
struct KnockoutMatch;
int FUN_00472990(KnockoutMatch *pMatch);
int FUN_0042cae0(Car *pCar, int variant);
int StageObject_GetWheelSlip(int carIndex, int wheelIndex);
BYTE FUN_00460bf0(int index);
int FUN_00460c10(int index);
int *FUN_00463270(int carIndex, int wheelIndex);
unsigned int FUN_00471bd0(BYTE **pOut);
void RallyData_FUN_00471cc0(int *pDest, void **pEntry);
int Track_GetGroundHeight(FixVector *pPoint, FixVector *pNormal, short *pTri,
                          unsigned short *pSurface, int defaultY);
void Sector_RemoveNode(SceneNode *pNode);
void FUN_004b8b10(SceneNode *pNode);
int FUN_004b7790(unsigned short id, int volume, int frequency, int loopStart, int loops, int is3D);
void Stage_InitLightMeshes(void);
int *FUN_00407520(int index);
void FUN_004925c0(int oldHeight, int newHeight, int mode);
void FUN_00492900(int value);
void FUN_00492fd0(int value);
void FUN_00477850(int object, int *src);
void FUN_0048df50(Car *param_1);
short *Car_GetOrder(void);
short Car_GetOrderCount(void);
BOOL Sound_LoadSample(char *name, BYTE flags, GenericFile *pFile);
void FUN_0048c900(BYTE index);
char *FUN_004752f0(int *p, int index, int mode);
void RallyData_FUN_00407500(BYTE param1);
unsigned short FUN_0040bbc0(unsigned short slot);
void FUN_0042b720(int index, char value);
int FUN_00469bc0(void *pCar, int index);
void FUN_00476c70(int index);
int FUN_00405600(void);
int FUN_00476850(int car, int pCar);

// Global fixed-point lighting parameters for both stage conditions.
// GLOBAL: CMR2 0x00547950
int g_stageLighting[0x178 / 4];

// Default intensity by stage weather index.
// GLOBAL: CMR2 0x0051b0b8
int g_stageWeatherIntensity[9] = {
    0x10000, 0x10000, 0x1999, 0x1999, 0x1999, 0x1999, 0x1999, 0x1999, 0x1999
};

struct StageSurfaceInfo {
    int flags;
    BYTE red, green, blue, alpha;
};
// GLOBAL: CMR2 0x0051bc68
StageSurfaceInfo g_stageSurfaceInfo[9] = {
    {0, 0, 0, 0, 0}, {1, 2, 2, 2, 0x18},
    {9, 0x48, 0x32, 0x27, 0x12}, {9, 0x14, 0x0d, 0, 0x12},
    {9, 0x0e, 0x0c, 0x06, 0x12}, {0x16, 0xad, 0xbd, 0xc6, 0x12},
    {0x16, 0xf4, 0xf4, 0xf4, 0x12}, {9, 0x48, 0x32, 0x27, 0x12},
    {4, 0x64, 0x64, 0x64, 0xff}
};
// GLOBAL: CMR2 0x0051bcb0
BYTE g_stageSurfaceMap[48] = {
    0, 7, 0, 0, 4, 4, 7, 7, 7, 0, 7, 7, 2, 2, 0, 0,
    0, 6, 5, 0, 0, 0, 6, 6, 0, 1, 2, 7, 7, 0, 6, 0,
    0, 0, 0, 3, 3, 3, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0
};
// GLOBAL: CMR2 0x00588620
BYTE g_trailColor[8][4];
// GLOBAL: CMR2 0x00588750
int g_trailFrame;
extern BYTE g_trailPointUsed[8][4][200];
extern BYTE g_trailPoints[8][4][200][0x28];
extern int g_unk0x00549b20[8][4];
extern FixVector g_unk0x00549c20[8][4];

// Accessors of the stage object tables (0x460bf0-0x4789b0)

extern void *g_unk0x00547ac8;
extern BYTE g_unk0x00543e98;
extern FixVector g_unk0x00547930;
extern int g_unk0x00547940;
extern void *g_unk0x0058c928;
extern void *g_unk0x0058c92c;
extern void *g_unk0x0058c930;
extern void *g_unk0x00543ecc;
extern int g_unk0x0058cf7c;
extern BYTE *g_unk0x0058c94c;
extern unsigned int g_unk0x0058ca6c;

int FUN_0046d2a0(int *p);
int RallyData_FUN_00421370(BYTE *p);
int RallyData_FUN_00421420(void);
unsigned int RallyData_FUN_00407e90(void);
unsigned int RallyData_FUN_00407ea0(void);
float FUN_00456ae0(void);

// Chooses the stage object path for the current game mode and rally state.
// FUNCTION: CMR2 0x0046bd50
int StageObject_UsesExtendedMode(void)
{
    if (CGameInfo::FUN_00405d80() == 8 ||
        CGameInfo::FUN_00405d80() == 9 ||
        CGameInfo::FUN_00405d80() == 10)
        return false;
    if (CGameInfo::FUN_00405d80() == 11 ||
        CGameInfo::FUN_00405d80() == 12)
        return true;
    if (CGameInfo::FUN_00405d80() != 0 &&
        CGameInfo::FUN_00405d80() != 1 &&
        CGameInfo::FUN_00405d80() != 2 &&
        CGameInfo::FUN_00405d80() != 3)
        return true;
    return (BYTE)RallyData_FUN_00407e90() != 0;
}

void FUN_0049c440(Mesh *pMesh, int mask, int value);
void FUN_0049c4b0(Mesh *pMesh, int mask, int value);

// Enables/disables the sub-meshes of a stage object's model according to the
// object type and the current game mode.
// FUNCTION: CMR2 0x004694a0
void FUN_004694a0(int param_1, int param_2, char param_3)
{
    int *pParts;
    BYTE *pType;
    int i;
    int uVar4;
    int uVar3;

    pParts = FUN_00469680((int)*(char *)(param_1 + 0xb1a));
    pType = FUN_00456be0((int)*(char *)(param_1 + 0xb1a));
    if (*(char *)(pType + 0x20) == 'C' ||
        (pType = FUN_00456be0((int)*(char *)(param_1 + 0xb1a)), *(char *)(pType + 0x20) == 'A')) {
        if (param_2 == 0) {
            uVar4 = 0;
            uVar3 = 1;
        } else if (param_2 == 1 || param_3 == 4 || param_3 == 5) {
            uVar4 = 3;
            uVar3 = 4;
        } else {
            uVar4 = 5;
            uVar3 = 7;
        }
        switch (param_3) {
        case 0:
            i = FUN_004692b0(7, (BYTE *)pParts);
            if (i >= 0) {
                FUN_0049c440((Mesh *)pParts[i], 0x100, uVar4);
                FUN_0049c4b0((Mesh *)pParts[i], 0x100, uVar3);
                return;
            }
            break;
        case 1:
            i = FUN_004692b0(0xc, (BYTE *)pParts);
            if (i >= 0) {
                FUN_0049c440((Mesh *)pParts[i], 0x20, uVar4);
                FUN_0049c4b0((Mesh *)pParts[i], 0x20, uVar3);
            }
            i = FUN_004692b0(7, (BYTE *)pParts);
            if (i >= 0) {
                FUN_0049c440((Mesh *)pParts[i], 0x20, uVar4);
                FUN_0049c4b0((Mesh *)pParts[i], 0x20, uVar3);
                return;
            }
            break;
        case 2:
            i = FUN_004692b0(7, (BYTE *)pParts);
            if (i >= 0) {
                FUN_0049c440((Mesh *)pParts[i], 0x40, uVar4);
                FUN_0049c4b0((Mesh *)pParts[i], 0x40, uVar3);
                return;
            }
            break;
        case 3:
            i = FUN_004692b0(7, (BYTE *)pParts);
            if (i >= 0) {
                FUN_0049c440((Mesh *)pParts[i], 0x80, uVar4);
                FUN_0049c4b0((Mesh *)pParts[i], 0x80, uVar3);
                return;
            }
            break;
        case 4:
            i = FUN_004692b0(0xe, (BYTE *)pParts);
            if (i >= 0) {
                FUN_0049c440((Mesh *)pParts[i], 4, uVar4);
                return;
            }
            break;
        case 5:
            i = FUN_004692b0(0xe, (BYTE *)pParts);
            if (i >= 0)
                FUN_0049c440((Mesh *)pParts[i], 8, uVar4);
            break;
        }
    }
}

// FUNCTION: CMR2 0x00469de0
int StageObject_IsEligibleType(short type, int mode, int category)
{
    int result = 0;
    if (mode == 0 || category == 6) {
        switch (type) {
        case 1:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 16:
        case 24:
        case 25:
        case 27:
        case 28:
            result = 1;
        }
    }
    return result;
}

// Full strength within ten fixed-point units, fading to zero at fifty.
// FUNCTION: CMR2 0x004863d0
int StageObject_DistanceFade(FixVector *delta)
{
    int component;
    int length;
    int fade;

    component = delta->x;
    if (component < 0) component = -component;
    if (component <= 0x320000) {
        component = delta->y;
        if (component < 0) component = -component;
        if (component <= 0x320000) {
            component = delta->z;
            if (component < 0) component = -component;
            if (component <= 0x320000) {
                length = FixVecLength(delta);
                if (length <= 0x320000) {
                    length -= 0xa0000;
                    if (length <= 0) return 0x10000;
                    fade = FixMul(length, 0x666);
                    if (fade > 0x10000) fade = 0x10000;
                    return 0x10000 - fade;
                }
            }
        }
    }
    return 0;
}

// GLOBAL: CMR2 0x00543f28
int g_unk0x00543f28[8 * 4];
// GLOBAL: CMR2 0x00547b80
BYTE g_unk0x00547b80;
// GLOBAL: CMR2 0x00548110
BYTE g_unk0x00548110[8][8];
// GLOBAL: CMR2 0x00588758
int *g_unk0x00588758;
// GLOBAL: CMR2 0x00588760
char g_unk0x00588760;
// GLOBAL: CMR2 0x00588761
signed char g_unk0x00588761;
// GLOBAL: CMR2 0x00588864
int g_unk0x00588864;
// GLOBAL: CMR2 0x00588868
int g_unk0x00588868;
// GLOBAL: CMR2 0x00588970
int g_unk0x00588970[8];
// GLOBAL: CMR2 0x00588ba4
BYTE g_unk0x00588ba4[16];
// GLOBAL: CMR2 0x00588bb4
int g_unk0x00588bb4[16];
// GLOBAL: CMR2 0x00588cd4
ReplayLevelState g_replayLevelState;
// GLOBAL: CMR2 0x00588d38
int g_unk0x00588d38;
// GLOBAL: CMR2 0x00589438
int g_unk0x00589438;
// GLOBAL: CMR2 0x0058943c
int g_unk0x0058943c;
// GLOBAL: CMR2 0x00589440
int g_unk0x00589440;
// GLOBAL: CMR2 0x00589444
int g_unk0x00589444;
// GLOBAL: CMR2 0x00589448
GenericFile g_unk0x00589448;
// GLOBAL: CMR2 0x0058cf68
int g_unk0x0058cf68;
// GLOBAL: CMR2 0x0058cf6c
int g_unk0x0058cf6c;
// GLOBAL: CMR2 0x0058cf80
BYTE g_unk0x0058cf80[0x100];
// Scratch rotation matrix for the stage-object nodes.
// GLOBAL: CMR2 0x0058d260
FixMatrix g_unk0x0058d260;
BYTE g_stageBlock[0x430];
// GLOBAL: CMR2 0x0058da30
int g_unk0x0058da30[8];
// GLOBAL: CMR2 0x0058da10
int g_unk0x0058da10[8];
// GLOBAL: CMR2 0x0058dda8
int g_unk0x0058dda8;
// GLOBAL: CMR2 0x0058e230
int g_unk0x0058e230[15];
// Random seed of the stage (kept for replays).
// GLOBAL: CMR2 0x0058e26c
int g_unk0x0058e26c;
// GLOBAL: CMR2 0x0058e270
char g_unk0x0058e270[16];
// GLOBAL: CMR2 0x0058e0b0
BYTE g_unk0x0058e0b0[8];
struct StageObjectValue { int value; BYTE rest[0x2c]; };
// GLOBAL: CMR2 0x0058e0b8
StageObjectValue g_unk0x0058e0b8[4];
// Driving state of the CPU car being updated (0xb8 bytes; FUN_0047bdd0 and the
// helpers it calls read and write its fields by offset).
// GLOBAL: CMR2 0x0058e178
int g_unk0x0058e178[0x2e];

// GLOBAL: CMR2 0x005113f8
double g_radiansToDegrees = 57.295827908797776;

// The larger of the longitudinal and lateral wheel slip, after each dead zone.
// FUNCTION: CMR2 0x00465d70
int StageObject_GetWheelSlip(int carIndex, int wheelIndex)
{
    if (carIndex < 8) {
        int slip = Car_Get(carIndex)->field_0x880[wheelIndex];
        if (slip < 0)
            slip = -Car_Get(carIndex)->field_0x880[wheelIndex];
        else
            slip = Car_Get(carIndex)->field_0x880[wheelIndex];
        slip -= 0xccc;
        if (slip < 0) slip = 0;

        int lateral = Car_Get(carIndex)->field_0x870[wheelIndex];
        if (lateral < 0)
            lateral = -Car_Get(carIndex)->field_0x870[wheelIndex];
        else
            lateral = Car_Get(carIndex)->field_0x870[wheelIndex];
        lateral -= 0x2666;
        if (lateral < 0) lateral = 0;
        if (slip < lateral) slip = lateral;
        if (slip < 0) slip = -slip;
        if (slip > 0) {
            slip = FixDiv(slip, 0x10000);
            if (slip < 0x10000) return slip;
            return 0x10000;
        }
    }
    return 0;
}

// Returns a 16.16 angle in degrees from the two vector components.
// FUNCTION: CMR2 0x00498d80
int StageObject_Atan2Degrees(int y, int x)
{
    double degrees = atan2((double)y, (double)x) * g_radiansToDegrees;
    return (int)(__int64)(degrees * CGraphics::m_65536);
}

int FUN_0041d290(void);
char FUN_00420190(void);

// FUNCTION: CMR2 0x00478130
void FUN_00478130(int index)
{
    g_unk0x0058da10[index] = FUN_0041d290();
}

// FUNCTION: CMR2 0x00478150
void FUN_00478150(int index)
{
    g_unk0x0058da30[index] = FUN_0041d290() - g_unk0x0058da10[index];
}

// FUNCTION: CMR2 0x0047aa60
void FUN_0047aa60(int value)
{
    g_unk0x0058dda8 = value;
}

// FUNCTION: CMR2 0x0047c5b0
int FUN_0047c5b0(int index)
{
    return g_unk0x0058e230[index];
}

// FUNCTION: CMR2 0x0047c5c0
void FUN_0047c5c0(void)
{
    StageObjectValue *p = g_unk0x0058e0b8;
    do {
        p->value = 0x10000;
        p++;
    } while ((int)p < (int)&g_unk0x0058e178);
}

// Per-type animation tables (0x58e394..0x58e4a4; the loader also fills the tail bytes).
// GLOBAL: CMR2 0x0058e394
BYTE *g_unk0x0058e394[68];
// GLOBAL: CMR2 0x0058e4a4
BYTE *g_unk0x0058e4a4;

// Looks up the pair of values that table `table` gives for the key of entry `entry`.
// FUNCTION: CMR2 0x0047cbc0
void FUN_0047cbc0(int unused, int table, int entry, int *pOut1, int *pOut2)
{
    int key = *(int *)(entry * 0x10 + 4 + g_unk0x0058e4a4);
    BYTE *p = g_unk0x0058e394[table];
    int i;

    for (i = 0; i < *(int *)(p + 0x84); i++) {
        if (key == (char)p[4 + i * 8]) {
            *pOut1 = (char)p[7 + i * 8];
            *pOut2 = (char)g_unk0x0058e394[table][6 + i * 8];
            return;
        }
    }
}

// FUNCTION: CMR2 0x0047cc30
void FUN_0047cc30(void)
{
    g_unk0x0058e0b0[0] = FUN_00420190();
    g_unk0x0058e0b0[1] = 1;
}

// FUNCTION: CMR2 0x0047cd00
int FUN_0047cd00(int index)
{
    return g_unk0x0058e270[index];
}

struct Block0x309 { int data[0x309]; };
struct Block0x134 { int data[0x134]; };
struct Block6 { int data[6]; };

struct StageTableEntry { int flag; short a; short b; };
// GLOBAL: CMR2 0x0051b9f0
StageTableEntry g_unk0x0051b9f0[5] = {
    {0, 2, 2}, {0, 2, 1}, {0x10000, 2, 1}, {0, 1, 2}, {1, 1, 2}
};

// FUNCTION: CMR2 0x00464b00
StageTableEntry *FUN_00464b00(int index)
{
    return &g_unk0x0051b9f0[index];
}

// FUNCTION: CMR2 0x0046b6b0
void FUN_0046b6b0(SceneNode *pNode, BYTE threshold)
{
    if (pNode->type == 0 && pNode->pObject != NULL &&
        *(BYTE *)(*(int *)((BYTE *)pNode->pObject + 0x24) + 0x37) <= threshold)
        pNode->field_0x17c = 0;
}

// FUNCTION: CMR2 0x0046b6e0
void FUN_0046b6e0(SceneNode *pNode, BYTE threshold)
{
    for (; pNode != NULL; pNode = pNode->pNext) {
        FUN_0046b6b0(pNode, threshold);
        if (pNode->pFirstChild != NULL)
            FUN_0046b6e0(pNode->pFirstChild, threshold);
    }
}

struct StageObjectEntry0x128 { int field_0x0; int *pObject; BYTE rest[0x120]; };
// GLOBAL: CMR2 0x005894e0
StageObjectEntry0x128 g_unk0x005894e0[40];
// GLOBAL: CMR2 0x0058c320
BYTE g_unk0x0058c320;
// GLOBAL: CMR2 0x0058c324
int g_unk0x0058c324;
// GLOBAL: CMR2 0x0058c924
BYTE g_unk0x0058c924;

// FUNCTION: CMR2 0x0046f7e0
void FUN_0046f7e0(void)
{
    int **pp;

    g_unk0x0058c924 = 0;
    pp = &g_unk0x005894e0[0].pObject;
    do {
        if (*pp != NULL)
            (*pp)[0xcc / 4] += 0xd8f00000U;
        pp = (int **)((BYTE *)pp + sizeof(StageObjectEntry0x128));
    } while ((int)pp < (int)&g_unk0x005894e0[40].pObject);
}

// Initializes a moving stage object from its route entry and the car's motion.
// match 51%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046f810
void StageObject_InitMovingObject(int *pState, int carIndex)
{
    Car *pCar = Car_Get(carIndex);
    BYTE *pEntries = NULL;
    int count = FUN_00471bd0(&pEntries);
    SceneNode *pNode = (SceneNode *)pState[1];
    int entryIndex = -1;
    int i;
    for (i = 0; i < count; ++i) {
        if ((int)(pEntries + i * 8) == pState[0]) {
            entryIndex = i;
            break;
        }
    }
    if (entryIndex < 0) {
        pNode->type = SCENE_NODE_EMPTY;
        return;
    }

    BYTE mappedIndex = ((BYTE *)g_unk0x0058c928)[entryIndex];
    pState[0x34] = ((int *)g_unk0x0058c930)[mappedIndex];
    pNode->type = SCENE_NODE_MESH;
    pNode->pObject = (void *)((int *)g_unk0x0058c92c)[mappedIndex];
    pNode->field_0x17c = (BYTE)(1 << (carIndex & 31));
    pState[0x45] = -0x10000;

    FixVector position;
    RallyData_FUN_00471cc0((int *)&position, (void **)pState);
    FixMatrix *pNodeMatrix = &pNode->current;
    FixMatrix_SetPosition(&position, pNodeMatrix);

    BYTE *pObject = *(BYTE **)pState[0];
    FixMatrix *pObjectMatrix = (FixMatrix *)(pObject + 0x18);
    FixVector basis;
    FixMatrix_GetRight(&basis, pObjectMatrix);
    FixMatrix_SetRight(&basis, pNodeMatrix);
    FixMatrix_GetUp(&basis, pObjectMatrix);
    FixMatrix_SetUp(&basis, pNodeMatrix);
    FixMatrix_GetForward(&basis, pObjectMatrix);
    FixMatrix_SetForward(&basis, pNodeMatrix);

    pState[0x32] = (int)(pState + 2);
    memcpy(pState + 2, pNodeMatrix, 0x40);
    memcpy(pState + 0x22, pNodeMatrix, 0x40);
    memcpy(pState + 0x12, pNodeMatrix, 0x40);

    int objectType = pState[0x34];
    int groundPath = objectType == 1 || (objectType == 0 && pCar->speed <= 0xc000);
    int airbornePath = objectType == 0 && pCar->speed > 0xc000;
    pState[0x44] = 0;
    *(short *)(pState + 0x47) = -1;

    if (groundPath || airbornePath) {
        FixVector velocity = pCar->velocity;
        FixVector localVelocity;
        FixVector axis;
        BYTE *pParams = *(BYTE **)(*(BYTE **)(pObject + 0xc) + 0x10c);
        if (groundPath) {
            int triangle = -1;
            int surface = 0;
            Track_GetGroundHeight(&position, (FixVector *)(pState + 0x35),
                                  (short *)&triangle, (unsigned short *)&surface, 0);
            if (pCar->speed > 0x8000) {
                FixVecScaleRecip(&velocity, &velocity, pCar->speed);
                FixVecScale(&velocity, &velocity, 0x8000);
            }
            FixMatrix_InverseRotateVector(&localVelocity, &velocity, (FixMatrix *)pState[0x32]);
            axis.x = FixMul(localVelocity.z, 0x10000);
            axis.y = 0;
            axis.z = -FixMul(localVelocity.x, 0x10000);
            if (pState[0x34] == 0)
                axis.z = 0;
            pState[0x41] = FixMul(*(int *)(pParams + 0x44), 0x8000);
            pState[0x42] = FixMul(*(int *)(pParams + 0x4c), 0x8000);
            pState[0x43] = FixMul(*(int *)(pParams + 0x48), 0x8000);
            pState[0x3e] = 0;
            pState[0x3f] = pState[0x42];
            pState[0x40] = 0;
            FixVecScale((FixVector *)(pState + 0x38), &axis, g_physicsTimeStep);
            pState[0x33] = 1;
        } else {
            FixMatrix_InverseRotateVector(&localVelocity, &velocity, (FixMatrix *)(pState + 2));
            axis.x = -FixMul(localVelocity.z, 0x6666);
            axis.y = 0;
            axis.z = FixMul(localVelocity.x, 0x6666);
            FixVecScale((FixVector *)(pState + 0x38), &axis, g_physicsTimeStep);
            FixVecScale((FixVector *)(pState + 0x3b), &velocity, 0xcccc);
            pState[0x3c] = FixMul(FixVecLength(&velocity), 0x4ccc);
            pState[0x41] = FixMul(*(int *)(pParams + 0x44), 0x8000);
            pState[0x42] = FixMul(*(int *)(pParams + 0x4c), 0x8000);
            pState[0x43] = FixMul(*(int *)(pParams + 0x48), 0x8000);
            pState[0x3e] = 0;
            pState[0x3f] = pState[0x42];
            pState[0x40] = 0;
            pState[0x48] = 0;
            pState[0x33] = 2;
            pState[0x49] = 1;
        }
    }

    memcpy(&pNode->world, pNodeMatrix, sizeof(FixMatrix));
    if (pNode->sector != -1)
        Sector_RemoveNode(pNode);
    if (pNode->sector == -1)
        FUN_004b8b10(pNode);
}

// Derives the stage's object scale from the loaded records: the average of
// their +0x54 fields (the single record in created-flag mode 1), floored at
// 0x4ccc, and scales the global stage vector by it, quadrupled for a type 2.
// match 84%: MSVC picks EDX for the record count and ESI for the loop counter
// (the original has them swapped); the code itself is identical.
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00460a30
void FUN_00460a30(FixVector *pOut)
{
    DWORD flags;
    int value;
    int typeTwo;
    int count;
    int i;
    int *p;

    // The original loads the whole dword at the flag byte (the variable lived
    // in that translation unit; see CONOCIMIENTO 4.y) and only uses its low byte.
    flags = *(DWORD *)&g_unk0x00543e98;
    value = 0;
    typeTwo = 0;
    if ((char)flags == 1) {
        value = *((int *)g_unk0x00547ac8 + 0x15);
        if (*((int *)g_unk0x00547ac8) == 2)
            typeTwo = 1;
    } else {
        count = (BYTE)flags;
        p = (int *)g_unk0x00547ac8;
        for (i = count; i > 0; i--) {
            value += p[0x15];
            if (*p == 2)
                typeTwo = 1;
            p += 0x5e;
        }
        if ((char)flags != 0)
            value = FixDiv(value, count << 16);
        else
            value = 0x4ccc;
    }
    if (value < 0x4ccc)
        value = 0x4ccc;
    value = FixMul(value, g_unk0x00547940);
    if (typeTwo)
        value = FixMul(0x20000, value);
    FixVecScale(pOut, &g_unk0x00547930, value);
}

// FUNCTION: CMR2 0x00460bf0
BYTE FUN_00460bf0(int index)
{
    return *((BYTE *)g_unk0x00547ac8 + index * 0x178);
}

// FUNCTION: CMR2 0x00460c10
int FUN_00460c10(int index)
{
    return *(int *)((BYTE *)g_unk0x00547ac8 + 0x54 + index * 0x178);
}

// GLOBAL: CMR2 0x00543d50
int g_unk0x00543d50;

// Sets an object's scalar and derives its fixed-point scaled component.
// FUNCTION: CMR2 0x00460c30
void StageObject_SetScaledValue(int value, int index)
{
    BYTE *entry = (BYTE *)g_unk0x00547ac8 + index * 0x178;
    index = value;
    *(int *)(entry + 0x54) = value;
    value = FixMul(g_unk0x00543d50, index);
    index = (int)*(short *)(entry + 0x74) << 16;
    *(int *)(entry + 0x5c) = FixMul(value, index);
}

// FUNCTION: CMR2 0x00460c80
int FUN_00460c80(BYTE *pCar)
{
    return *(int *)((BYTE *)g_unk0x00543ecc + 8 + (signed char)pCar[0xb1a] * 0xc);
}

// FUNCTION: CMR2 0x00463270
int *FUN_00463270(int i, int j)
{
    return &g_unk0x00543f28[j + i * 4];
}

// FUNCTION: CMR2 0x00463ce0
void FUN_00463ce0(BYTE value)
{
    g_unk0x00547b80 = value;
    if (value > 7)
        g_unk0x00547b80 = 6;
}

// FUNCTION: CMR2 0x00464af0
BYTE *FUN_00464af0(int index)
{
    return g_unk0x00548110[index];
}


extern Texture *g_unk0x00588740;
extern Texture *g_unk0x00588744;
extern Texture *g_unk0x00588748;
extern int g_unk0x00549ba0[8][4];
int FUN_00465ea0(int value);
#define TRAIL_POINT(i) (&g_trailPoints[0][0][0][0] + (i) * 0x28)
#define TRAIL_FADE(p, b) ((p)[b] - (p)[b] * *(int *)(p) * 0x7c / 0x4d8)

// Draws the tyre marks of one car: for every wheel it walks its ring of 200
// trail points backwards in steps of g_stageSurfaceInfo[8].flags, joins each
// point to the previous consecutive one with a textured quad (two triangles)
// whose alpha fades with the points' age, and picks the mark texture from the
// surface the point was laid on.
// FUNCTION: CMR2 0x004658e0
void FUN_004658e0(int index)
{
    BYTE curColour[4] = { 0, 0, 0, 25 };
    BYTE prevColour[4] = { 0, 0, 0, 25 };
    Quad2DInputVertex a;
    Quad2DInputVertex b;
    Quad2DInputVertex c;
    Quad2DInputVertex d;
    int *pHead;
    int base;
    int wheel;
    int j;
    int step;
    int cur;
    int k;
    int found;
    BYTE *pCur;
    BYTE *pPrev;
    int alpha0;
    int alpha1;
    int alpha2;
    int alpha3;
    Texture *pTexture;

    if (index >= 8 || CGameInfo::FUN_00405cd0() == 2)
        return;
    pHead = g_unk0x00549ba0[index];
    base = index * 800;
    a.u = 0;
    a.v = 0;
    b.u = 0x10000;
    b.v = 0;
    d.u = 0x10000;
    d.v = 0x10000;
    c.u = 0;
    c.v = 0x10000;
    for (wheel = 4; wheel != 0; wheel--, pHead++, base += 200) {
        j = 0;
        do {
            found = 0;
            step = g_stageSurfaceInfo[8].flags;
            cur = (*pHead - j + 200) % 200;
            if (step > 1) {
                k = cur - step + 200;
                do {
                    if (found)
                        break;
                    step--;
                    k++;
                    if ((&g_trailPointUsed[0][0][0])[base + cur] == 0 ||
                        (&g_trailPointUsed[0][0][0])[base + k % 200] == 0)
                        continue;
                    pPrev = TRAIL_POINT(k % 200 + base);
                    pCur = TRAIL_POINT(base + cur);
                    found = *(int *)(pCur + 4) == *(int *)(pPrev + 4) + step;
                    if (found != 1)
                        continue;
                    if (pPrev[0x20] <= 200 && *(int *)pPrev <= 0x4d8) {
                        alpha3 = pPrev[0x25];
                        alpha2 = pPrev[0x24];
                        alpha0 = pCur[0x24];
                        alpha1 = pCur[0x25];
                    } else {
                        alpha2 = TRAIL_FADE(pPrev, 0x24);
                        alpha3 = TRAIL_FADE(pPrev, 0x25);
                        alpha0 = TRAIL_FADE(pCur, 0x24);
                        alpha1 = TRAIL_FADE(pCur, 0x25);
                    }
                    alpha0 = FUN_00465ea0(alpha0);
                    alpha1 = FUN_00465ea0(alpha1);
                    alpha2 = FUN_00465ea0(alpha2);
                    alpha3 = FUN_00465ea0(alpha3);
                    if (alpha0 <= 1 && alpha1 <= 1 && alpha2 <= 1 && alpha3 <= 1)
                        continue;
                    curColour[0] = pCur[0x20];
                    curColour[1] = pCur[0x21];
                    curColour[2] = pCur[0x22];
                    prevColour[0] = pPrev[0x20];
                    prevColour[1] = pPrev[0x21];
                    prevColour[2] = pPrev[0x22];
                    a.x = *(int *)(pCur + 8);
                    a.y = *(int *)(pCur + 0xc);
                    a.z = *(int *)(pCur + 0x10);
                    *(DWORD *)a.colour = *(DWORD *)curColour;
                    a.colour[3] = (BYTE)alpha0;
                    b.x = *(int *)(pCur + 0x14);
                    b.y = *(int *)(pCur + 0x18);
                    b.z = *(int *)(pCur + 0x1c);
                    *(DWORD *)b.colour = *(DWORD *)curColour;
                    b.colour[3] = (BYTE)alpha1;
                    c.x = *(int *)(pPrev + 8);
                    c.y = *(int *)(pPrev + 0xc);
                    c.z = *(int *)(pPrev + 0x10);
                    *(DWORD *)c.colour = *(DWORD *)prevColour;
                    c.colour[3] = (BYTE)alpha2;
                    d.x = *(int *)(pPrev + 0x14);
                    d.y = *(int *)(pPrev + 0x18);
                    d.z = *(int *)(pPrev + 0x1c);
                    *(DWORD *)d.colour = *(DWORD *)prevColour;
                    d.colour[3] = (BYTE)alpha3;
                    if (*(int *)(pPrev + 8) == *(int *)(pPrev + 0x14) && *(int *)(pPrev + 0xc) == *(int *)(pPrev + 0x18))
                        continue;
                    switch ((g_stageSurfaceInfo[pCur[0x26] & 0xf].flags >> 2) & 3) {
                    case 1:
                        pTexture = g_unk0x00588744;
                        break;
                    case 2:
                        a.colour[0] = 0;
                        a.colour[1] = 0;
                        a.colour[2] = 0;
                        a.colour[3] = (BYTE)(alpha0 / 3);
                        *(DWORD *)c.colour = *(DWORD *)a.colour;
                        b.colour[0] = 0;
                        b.colour[1] = 0;
                        b.colour[2] = 0;
                        b.colour[3] = (BYTE)(alpha1 / 3);
                        *(DWORD *)d.colour = *(DWORD *)b.colour;
                        pTexture = g_unk0x00588748;
                        break;
                    default:
                        pTexture = g_unk0x00588740;
                        break;
                    }
                    Quad2D_QueueFixedTriangle(0, &b, &a, &c, pTexture, (Quad2D *)0x26);
                    Quad2D_QueueFixedTriangle(0, &d, &b, &c, pTexture, (Quad2D *)0x26);
                } while (step > 1);
            }
            j += step;
        } while (j < 200);
    }
}
#undef TRAIL_POINT
#undef TRAIL_FADE

// FUNCTION: CMR2 0x00465ea0
int FUN_00465ea0(int value)
{
    if (value < 0)
        return 0;
    if (value > 0xff)
        value = 0xff;
    return value;
}

// FUNCTION: CMR2 0x00465f80
void FUN_00465f80(void)
{
    g_unk0x00588760 = 0xff;
    g_unk0x00588864 = -1;
}

// GLOBAL: CMR2 0x0058875c
Car *g_unk0x0058875c;

void FUN_00465ec0(SceneNode *pNode, int alpha, BYTE checkFlag);
void FUN_00465f20(SceneNode *pNode, int alpha, BYTE checkFlag);

// Makes a car the ghost car: flags it and fades its body nodes in.
// FUNCTION: CMR2 0x00465fc0
void FUN_00465fc0(Car *pCar)
{
    g_unk0x0058875c = pCar;
    pCar->field_0xc0c = 1;
    g_unk0x00588761 = -1;
    g_unk0x00588864 = -1;
    FUN_00465ec0(pCar->pNode0x71c, 100, 1);
    FUN_00465ec0(pCar->pNode0x720, 100, 1);
    FUN_00465f20(pCar->pNode0x71c->pFirstChild, 100, 1);
    FUN_00465f20(pCar->pNode0x720->pFirstChild, 100, 1);
}

// FUNCTION: CMR2 0x00466080
void FUN_00466080(void)
{
    FUN_0046d2a0(g_unk0x00588758);
}

// FUNCTION: CMR2 0x00466090
int FUN_00466090(void)
{
    if (g_unk0x00588758 != NULL)
        return g_unk0x00588758[1];
    return 0;
}

// FUNCTION: CMR2 0x004660e0
void FUN_004660e0(BYTE value)
{
    g_unk0x00588761 = value;
}

// FUNCTION: CMR2 0x004660f0
int FUN_004660f0(void)
{
    return g_unk0x00588760;
}

// FUNCTION: CMR2 0x0046b400
void FUN_0046b400(int value, int index)
{
    g_unk0x00588970[index] = value;
}

// FUNCTION: CMR2 0x0046b420
void FUN_0046b420(void)
{
    memset(g_unk0x00588970, 0, 8 * sizeof(int));
}

// Reads vertex `vertex` of mesh `mesh` (float source data) as a 16.16 vector.
// FUNCTION: CMR2 0x0046b440
void FUN_0046b440(Mesh **ppMeshes, int mesh, int vertex, int *pOut)
{
    pOut[0] = (int)(__int64)(*(float *)((BYTE *)ppMeshes[mesh]->pVertexData + vertex * 0x30) * CGraphics::m_65536);
    pOut[1] = (int)(__int64)(*(float *)((BYTE *)ppMeshes[mesh]->pVertexData + vertex * 0x30 + 4) * CGraphics::m_65536);
    pOut[2] = (int)(__int64)(*(float *)((BYTE *)ppMeshes[mesh]->pVertexData + vertex * 0x30 + 8) * CGraphics::m_65536);
}


extern float g_oneOverRandMax;

// Flicker timer of a car's broken light: once the damage (+0x29c) passes
// half, the light alternates on and off (+0x4cc) for random spans, the off
// spans getting shorter and the on spans longer the heavier the damage.
// FUNCTION: CMR2 0x0046b4e0
void FUN_0046b4e0(BYTE *pCar)
{
    int *p;
    int k;

    p = FUN_00469680((signed char)pCar[0xb1a]);
    if (p[0xa7] > 0x8000) {
        k = FixMul(p[0xa7] - 0x8000, 0x20000);
        if (k < 0)
            k = 0;
        else if (k > 0x10000)
            k = 0x10000;
        k = FixMul(k, 0xcccc);
        if (p[0x103] <= 0) {
            if (p[0x133] != 0) {
                p[0x133] = 0;
                p[0x103] = FixMul(0x10000 - k, FixMul((int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536), 0xfa0000));
                p[0x103] += FixMul(0x10000 - k, 0x320000);
            } else {
                p[0x133] = 1;
                p[0x103] = FixMul(k, FixMul((int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536), 0xf0000));
                p[0x103] += FixMul(k, 0);
            }
        } else {
            p[0x103] -= 0x10000;
        }
    }
}

// FUNCTION: CMR2 0x0046b4c0
int FUN_0046b4c0(BYTE *pCar)
{
    return g_unk0x00588970[(signed char)pCar[0xb1a]];
}

// FUNCTION: CMR2 0x0046b710
void FUN_0046b710(void)
{
    int *p;
    p = &g_unk0x00588cd4[1];
    memset(g_unk0x00588bb4, 0, 8 * sizeof(int));
    do {
        p[-1] = 0;
        *p = 0;
        p += 2;
    } while ((int)p < (int)g_unk0x00588d18);
}

// FUNCTION: CMR2 0x0046b740
void FUN_0046b740(int i, int value, int j)
{
    g_unk0x00588cd4[j + i * 2] = value;
}

// FUNCTION: CMR2 0x0046b760
void FUN_0046b760(int index, int reset)
{
    if (reset != 0) {
        g_unk0x00588bb4[index] = 0;
        return;
    }
    g_unk0x00588bb4[index] = 1;
}

// FUNCTION: CMR2 0x0046bd20
int FUN_0046bd20(int i, int j)
{
    return g_unk0x00588cd4[j + i * 2];
}

// FUNCTION: CMR2 0x0046bd40
BYTE FUN_0046bd40(int index)
{
    return g_unk0x00588ba4[index];
}

// Decodes one 4-byte replay input packet into the control record pOut and the
// car's handbrake/light switches; returns 1 when the packet's repeat count is
// used up.
// FUNCTION: CMR2 0x0046bec0
int FUN_0046bec0(int *pState, BYTE *pIn, BYTE *pOut, BYTE *pCounter, Car *pCar)
{
    BYTE steer = pIn[3] & 0x3f;
    BYTE *p = (BYTE *)pCar;

    if ((pIn[3] & 0x40) != 0) {
        pOut[0] = steer;
        pOut[1] = 0;
    } else {
        pOut[0] = 0;
        pOut[1] = steer;
    }
    pOut[2] = pIn[0] >> 2;
    pOut[3] = pIn[1] >> 2;
    *(unsigned int *)(pOut + 8) = pIn[2] >> 7;
    pOut[4] = (pIn[1] & 3) - 1;
    *(unsigned int *)(pOut + 0xc) = (pIn[2] & 0x40) >> 6;
    if ((pIn[0] & 1) != 0)
        *(int *)(p + 0xb8c) = 1;
    else
        *(int *)(p + 0xb8c) = 0;
    if ((pIn[0] & 2) != 0)
        *(int *)(p + 0xb90) = 1;
    else
        *(int *)(p + 0xb90) = 0;
    if ((pIn[3] & 0x80) != 0) {
        if (*pState != 0)
            *(int *)(p + 0xb88) = 1;
        else
            *(int *)(p + 0xb88) = 2;
    } else
        *(int *)(p + 0xb88) = 0;
    if ((BYTE)++*pCounter < (pIn[2] & 0x3f))
        return 0;
    *pCounter = 0;
    return 1;
}

// FUNCTION: CMR2 0x0046bfb0
void FUN_0046bfb0(Block0x309 *pSrc, Block0x309 *pDst)
{
    *pDst = *pSrc;
}

// Restores a car from a saved state record, keeping the destination's scene
// node bindings, its wheel emitter vectors and its timing index.
// FUNCTION: CMR2 0x0046bfd0
void FUN_0046bfd0(Block0x309 *pSrc, Car *pDst)
{
    SceneNode *pNode0x71c = pDst->pNode0x71c;
    SceneNode *pNode0x720 = pDst->pNode0x720;
    SceneNode *pNode0x724 = pDst->pNode0x724;
    SceneNode *pExtraNodes[4];
    SceneNode *pWheelNodes[4];
    SceneNode *pViewNodeNear = pDst->pViewNodeNear;
    SceneNode *pViewNodeFar = pDst->pViewNodeFar;
    FixMatrix *pWorld = pDst->pWorld;
    FixMatrix *pBodyMatrix = pDst->pBodyMatrix;
    FixVector wheelEmitter[4];
    char carIndex = pDst->field_0xb1a;
    int i;

    for (i = 0; i < 4; i++) {
        pExtraNodes[i] = pDst->pExtraNodes[i];
        pWheelNodes[i] = pDst->pWheelNodes[i];
    }
    memcpy(wheelEmitter, pDst->wheelEmitter, sizeof(wheelEmitter));

    *(Block0x309 *)pDst = *pSrc;

    pDst->pNode0x71c = pNode0x71c;
    pDst->pNode0x720 = pNode0x720;
    pDst->pNode0x724 = pNode0x724;
    pDst->pViewNodeNear = pViewNodeNear;
    pDst->pViewNodeFar = pViewNodeFar;
    pDst->pWorld = pWorld;
    pDst->pBodyMatrix = pBodyMatrix;
    for (i = 0; i < 4; i++) {
        pDst->pExtraNodes[i] = pExtraNodes[i];
        pDst->pWheelNodes[i] = pWheelNodes[i];
        pDst->wheelEmitter[i] = wheelEmitter[i];
    }
    pDst->field_0xb1a = carIndex;

    if ((BYTE)RallyData_FUN_00407ea0() != 0)
        *(float *)(pDst->field_0xa90 + 8) = 25.0f;
    if (pDst->field_0xb1a > 0 && (BYTE)RallyData_FUN_00407e90() != 0 &&
        (BYTE)CGameInfo::FUN_00405e00() == 0 && (BYTE)RallyDataState() == 1)
        *(float *)(pDst->field_0xa90 + 8) = FUN_00456ae0();
}

// FUNCTION: CMR2 0x0046c180
void FUN_0046c180(Block0x134 *pSrc, Block0x134 *pDst)
{
    *pDst = *pSrc;
}

// Copies a 0x134-int car state record, keeping the destination's first three
// 15-int blocks.
// match 77%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046c1a0
void FUN_0046c1a0(Block0x134 *pSrc, Block0x134 *pDst)
{
    int keepC[15];
    int keepB[15];
    int keepA[15];
    int i;

    memcpy(keepC, pDst->data + 30, sizeof(keepC));
    memcpy(keepB, pDst->data + 15, sizeof(keepB));
    memcpy(keepA, pDst->data, sizeof(keepA));
    *pDst = *pSrc;
    for (i = 0; i < 15; i++) {
        pDst->data[i] = keepA[i];
        pDst->data[15 + i] = keepB[i];
        pDst->data[30 + i] = keepC[i];
    }
}

// FUNCTION: CMR2 0x0046c220
void FUN_0046c220(Block6 *pSrc, Block6 *pDst)
{
    *pDst = *pSrc;
}

int *FUN_00469680(int index);
struct RaceRecord;
RaceRecord *RallyData_FUN_00421510(int index);
void RallyData_FUN_004207a0(int index);

// Snapshots a car's state into the 0x1100-byte record at pDst.
// FUNCTION: CMR2 0x0046c240
void FUN_0046c240(BYTE *pDst, BYTE car)
{
    Car *pCar = Car_Get(car);

    FUN_0046c180((Block0x134 *)FUN_00469680(car), (Block0x134 *)pDst);
    FUN_0046bfb0((Block0x309 *)pCar, (Block0x309 *)(pDst + 0x4d0));
    FUN_0046c220((Block6 *)RallyData_FUN_00421510(car), (Block6 *)(pDst + 0x10f4));
    RallyData_FUN_004207a0(car);
}

// FUNCTION: CMR2 0x0046d2a0
int FUN_0046d2a0(int *p)
{
    if (p != NULL && p[1] != 0) {
        p[1] = 0;
        p[2] = 0;
        p[5] = 0;
        return 1;
    }
    return 0;
}

// FUNCTION: CMR2 0x0046d500
int FUN_0046d500(void)
{
    return g_unk0x00588d38;
}

// FUNCTION: CMR2 0x0046f4c0
void FUN_0046f4c0(int *pOut)
{
    *pOut = g_unk0x00589438;
}

// FUNCTION: CMR2 0x0046f4d0
void FUN_0046f4d0(int *pOut)
{
    *pOut = g_unk0x0058943c;
}

// GLOBAL: CMR2 0x0058c928
void *g_unk0x0058c928;
// GLOBAL: CMR2 0x0058c92c
void *g_unk0x0058c92c;
// GLOBAL: CMR2 0x0058c930
void *g_unk0x0058c930;

// Releases the three files loaded by 0x46f550 (registered callback).
// FUNCTION: CMR2 0x0046f500
int FUN_0046f500(void)
{
    if (g_unk0x0058c928 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x0058c928);
        g_unk0x0058c928 = NULL;
    }
    if (g_unk0x0058c930 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x0058c930);
        g_unk0x0058c930 = NULL;
    }
    if (g_unk0x0058c92c != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x0058c92c);
        g_unk0x0058c92c = NULL;
    }
    g_unk0x0058c320 = 0;
    return 1;
}

// FUNCTION: CMR2 0x0046f4e0
void FUN_0046f4e0(int *pOut1, int *pOut2)
{
    *pOut1 = g_unk0x00589440;
    *pOut2 = g_unk0x00589444;
}

// GLOBAL: CMR2 0x0051c6d0
char g_strTempGro[] = "TEMP.GRO";
// GLOBAL: CMR2 0x0051c6dc
char g_strTopC3d[] = "top.c3d";
// GLOBAL: CMR2 0x0051c6e4
char g_strC3dExt[] = ".c3d";
// GLOBAL: CMR2 0x0051c6ec
char g_strBflExt[] = ".bfl";
// GLOBAL: CMR2 0x0051c6f4
char g_strTempSky[] = "TEMP.SKY";

void FUN_004b2e40(BYTE *p, int value);
void FUN_0046ef50(void);
int FUN_004b9380(unsigned int, unsigned int, unsigned int);
GenericFile *FUN_0041f500(void);
int RallyData_FUN_00411060(void);
BYTE FUN_0046f030(void);

// Loads the stage's sky and ground objects: the TEMP.SKY archive (also opened
// as .bfl, .c3d and top.c3d) and TEMP.GRO, releasing each one's meshes first.
// FUNCTION: CMR2 0x0046f060
void FUN_0046f060(void)
{
    char buffer[MAX_PATH];
    GenericFile *pFile;
    GenericFile *pC3d;
    int node;
    BYTE *pMesh;

    pFile = (GenericFile *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), g_strTempSky, NULL, NULL, 0);
    if (pFile != NULL) {
        g_unk0x00589438 = (int)FUN_004b9380((unsigned int)pFile, (unsigned int)RallyData_FUN_00411060(),
                                           (unsigned int)FUN_0041f500());
        if (g_unk0x00589438 != 0) {
            FUN_004b2e40(*(BYTE **)(g_unk0x00589438 + 0xc), 0);
            *(int *)(g_unk0x00589438 + 0x180) = 0;
            node = *(int *)(g_unk0x00589438 + 4);
            if (node != 0) {
                pMesh = *(BYTE **)(node + 0xc);
                *(int *)(node + 0x180) = 0;
                FUN_004b2e40(pMesh, 0);
                node = *(int *)(*(int *)(g_unk0x00589438 + 4));
                if (node != 0) {
                    pMesh = *(BYTE **)(node + 0xc);
                    *(int *)(node + 0x180) = 0;
                    FUN_004b2e40(pMesh, 0);
                    node = *(int *)(*(int *)(*(int *)(g_unk0x00589438 + 4)));
                    if (node != 0) {
                        pMesh = *(BYTE **)(node + 0xc);
                        *(int *)(node + 0x180) = 0;
                        FUN_004b2e40(pMesh, 0);
                    }
                }
            }
        }
    }
    FUN_0046ef50();
    strcpy(buffer, CFrontend::m_stringDest);
    strcpy(CFrontend::m_stringDest, buffer);
    strcat(CFrontend::m_stringDest, g_strBflExt);
    CGenericFileLoader::FUN_004a9d70(&g_unk0x00589448, CFrontend::m_stringDest);
    strcpy(CFrontend::m_stringDest, buffer);
    strcat(CFrontend::m_stringDest, g_strC3dExt);
    pC3d = (GenericFile *)CGenericFileLoader::FindFile(&g_unk0x00589448, CFrontend::m_stringDest, NULL, NULL, 0);
    strcpy(CFrontend::m_stringDest, buffer);
    strcat(CFrontend::m_stringDest, g_strTopC3d);
    pFile = (GenericFile *)CGenericFileLoader::FindFile(&g_unk0x00589448, CFrontend::m_stringDest, NULL, NULL, 0);
    if (pFile != NULL) {
        g_unk0x00589444 = (int)FUN_004b9380((unsigned int)pFile, (unsigned int)RallyData_FUN_00411060(),
                                           (unsigned int)&g_unk0x00589448);
        if (g_unk0x00589444 != 0) {
            pMesh = *(BYTE **)(g_unk0x00589444 + 0xc);
            *(int *)(g_unk0x00589444 + 0x180) = 0;
            FUN_004b2e40(pMesh, 0);
            *(BYTE *)(g_unk0x00589444 + 0x17c) = 0;
        }
    }
    if (pC3d != NULL) {
        g_unk0x00589440 = (int)FUN_004b9380((unsigned int)pC3d, (unsigned int)RallyData_FUN_00411060(),
                                           (unsigned int)&g_unk0x00589448);
        if (g_unk0x00589440 != 0) {
            pMesh = *(BYTE **)(g_unk0x00589440 + 0xc);
            *(int *)(g_unk0x00589440 + 0x180) = 0;
            FUN_004b2e40(pMesh, 0);
        }
    }
    pFile = (GenericFile *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), g_strTempGro, NULL, NULL, 0);
    if (pFile != NULL) {
        g_unk0x0058943c = (int)FUN_004b9380((unsigned int)pFile, (unsigned int)RallyData_FUN_00411060(),
                                           (unsigned int)FUN_0041f500());
        if (g_unk0x0058943c != 0) {
            pMesh = *(BYTE **)(g_unk0x0058943c + 0xc);
            *(int *)(g_unk0x0058943c + 0x180) = 0;
            FUN_004b2e40(pMesh, 0);
        }
    }
    CGame::RegisterCallback((void *)FUN_0046f030, NULL);
}

int RallyData_FUN_00411060(void);
void Mesh_ResetCloneCount(void);
Mesh *Mesh_CloneInto(Mesh *pSrc, BYTE *pSource);
extern double g_minus65536;

// Loads the stage object list, creates a scene node and clones the mesh of
// every object, then classifies each bounding box as ground or wall.
// match 49%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// The three bounding-box accumulators land in ESI/EDX/ECX instead of the
// original's EDI/ESI/EDX and MSVC merges the "if (v < 0)" phi without the
// original's extra jmp; the code itself is identical.
// FUNCTION: CMR2 0x0046f550
void FUN_0046f550(void)
{
    BYTE *pEntries;
    int i;
    int count;
    BYTE *pEntry;
    int maxX;
    int maxY;
    int maxZ;
    int mesh;
    float *pFloats;

    Mesh_ResetCloneCount();
    pEntry = (BYTE *)g_unk0x005894e0 + 0x114;
    do {
        SceneNode *pNode = SceneNode_Create((SceneNode *)RallyData_FUN_00411060());
        *(int *)(pEntry - 0x110) = (int)pNode;
        *(int *)((BYTE *)pNode + 0x178) = 3;
        *(int *)pEntry = -0x10000;
        pEntry += 0x128;
    } while ((int)pEntry < (int)((BYTE *)g_unk0x005894e0 + 0x114 + 40 * 0x128));

    count = FUN_00471bd0(&pEntries);
    g_unk0x0058c92c = CFileBuffer::AllocateLockedBuffer(count * 4);
    i = 0;
    if (count > 0) {
        do {
            i++;
            ((int *)g_unk0x0058c92c)[i - 1] = 0;
        } while (i < count);
    }
    g_unk0x0058c928 = 0;
    if (count > 0) {
        g_unk0x0058c928 = CFileBuffer::AllocateLockedBuffer(count);
        i = 0;
        g_unk0x0058c320 = 0;
        if (count > 0) {
            do {
                mesh = *(int *)(*(int *)(pEntries + i * 8) + 0xc);
                if ((int)g_unk0x0058c320 < count) {
                    ((BYTE *)g_unk0x0058c928)[i] = (BYTE)g_unk0x0058c320;
                    mesh = (int)Mesh_CloneInto((Mesh *)mesh, (BYTE *)*(int *)(pEntries + i * 8));
                    ((int *)g_unk0x0058c92c)[(BYTE)g_unk0x0058c320] = mesh;
                    if (((int *)g_unk0x0058c92c)[(BYTE)g_unk0x0058c320] == 0)
                        ((BYTE *)g_unk0x0058c928)[i] = 0;
                    else
                        g_unk0x0058c320++;
                } else {
                    ((BYTE *)g_unk0x0058c928)[i] = 0xff;
                }
                i++;
            } while (i < count);
        }
    }
    g_unk0x0058c930 = 0;
    if (g_unk0x0058c320 > 0)
        g_unk0x0058c930 = CFileBuffer::AllocateLockedBuffer((g_unk0x0058c320 & 0xff) << 2);
    i = 0;
    if (g_unk0x0058c320 > 0) {
        do {
            int pObject = ((int *)g_unk0x0058c92c)[i];
            int n;
            int x;
            int y;
            int z;
            maxZ = 0;
            maxY = 0;
            maxX = 0;
            n = *(int *)((BYTE *)pObject + 0x10);
            if (n > 0) {
                pFloats = *(float **)((BYTE *)pObject + 0xc);
                do {
                    x = (int)(__int64)(pFloats[0] * CGraphics::m_65536);
                    if (x < 0)
                        x = (int)(__int64)(pFloats[0] * g_minus65536);
                    y = (int)(__int64)(pFloats[1] * CGraphics::m_65536);
                    if (y < 0)
                        y = (int)(__int64)(pFloats[1] * g_minus65536);
                    z = (int)(__int64)(pFloats[2] * CGraphics::m_65536);
                    if (z < 0)
                        z = (int)(__int64)(pFloats[2] * g_minus65536);
                    if (x > maxX)
                        maxX = x;
                    if (y > maxY)
                        maxY = y;
                    if (z > maxZ)
                        maxZ = z;
                    pFloats += 0xc;
                    n--;
                } while (n != 0);
            }
            if (FixDiv(maxZ, maxX) < 0x4ccc)
                ((int *)g_unk0x0058c930)[i] = 0;
            else if (FixDiv(maxX, maxY) < 0x4ccc)
                ((int *)g_unk0x0058c930)[i] = 1;
            else
                ((int *)g_unk0x0058c930)[i] = 0;
            i++;
        } while (i < (int)(g_unk0x0058c320 & 0xff));
    }
    CGame::RegisterCallback((void *)FUN_0046f500, NULL);
}

// FUNCTION: CMR2 0x00471bd0
unsigned int FUN_00471bd0(BYTE **pOut)
{
    *pOut = g_unk0x0058c94c;
    return g_unk0x0058ca6c;
}

// FUNCTION: CMR2 0x004728b0
void FUN_004728b0(void)
{
    g_unk0x0058cf68 = 1;
}

// FUNCTION: CMR2 0x004728c0
void FUN_004728c0(void)
{
    g_unk0x0058cf68 = 0;
}

// FUNCTION: CMR2 0x004728d0
int FUN_004728d0(void)
{
    return g_unk0x0058cf68;
}

// Returns 1 when a match of the current knockout round is undecided or one of
// its drivers is not in the table.
// FUNCTION: CMR2 0x004728e0
int FUN_004728e0(void)
{
    unsigned int *pState;
    KnockoutMatch *pMatch = NULL;
    int count = 0;
    int i;

    pState = RallyData_GetChampionshipState();
    switch ((*pState >> 3) & 7) {
    case 1:
        count = 8;
        pMatch = (KnockoutMatch *)(pState + 0x16);
        break;
    case 2:
        count = 4;
        pMatch = (KnockoutMatch *)(pState + 10);
        break;
    case 3:
        count = 2;
        pMatch = (KnockoutMatch *)(pState + 4);
        break;
    case 4:
        count = 1;
        pMatch = (KnockoutMatch *)(pState + 1);
    }
    for (i = 0; i < count; i++, pMatch++) {
        if ((RallyData_FUN_00408500((BYTE)(pMatch->flags & 0x1f)) == -1 && (pMatch->flags & 0x400) == 0) ||
            (RallyData_FUN_00408500((BYTE)((pMatch->flags >> 5) & 0x1f)) == -1 && (pMatch->flags & 0x400) == 0))
            return 1;
        if (FUN_00472990(pMatch))
            return 1;
    }
    return 0;
}

// Whether the human player lost the knockout match (the winner is not a human
// driver): bit 11 means the second driver won, bit 12 the first one.
// FUNCTION: CMR2 0x00472990
int FUN_00472990(KnockoutMatch *pMatch)
{
    if (RallyData_FUN_00408500(pMatch->flags & 0x1f) == -1 && (pMatch->flags & 0x1800) == 0x800)
        return 1;
    if (RallyData_FUN_00408500((pMatch->flags >> 5) & 0x1f) == -1 && (pMatch->flags & 0x1800) == 0x1000)
        return 1;
    return 0;
}

// FUNCTION: CMR2 0x00472ca0
void FUN_00472ca0(void)
{
    g_unk0x0058cf7c = 0;
}

// Sets the ground material of the two cars from the current championship
// state (the knockout bracket entry selected by the state flags).
// FUNCTION: CMR2 0x00472cb0
void FUN_00472cb0(void)
{
    unsigned int *pState = RallyData_GetChampionshipState();
    BYTE value;
    int i;

    switch ((*pState >> 3) & 7) {
    case 1:
        if (RallyData_FUN_00408500((BYTE)pState[((*pState >> 0xc) & 0xf) * 3 + 0x16] & 0x1f) == -1 &&
            RallyData_FUN_00408500((BYTE)(pState[((*pState >> 0xc) & 0xf) * 3 + 0x16] >> 5) & 0x1f) == -1)
            goto fail;
        value = 1;
        break;
    case 2:
        if (RallyData_FUN_00408500((BYTE)pState[((*pState >> 0xc) & 0xf) * 3 + 10] & 0x1f) == -1 &&
            RallyData_FUN_00408500((BYTE)(pState[((*pState >> 0xc) & 0xf) * 3 + 10] >> 5) & 0x1f) == -1)
            goto fail;
        value = 1;
        break;
    case 3:
        if (RallyData_FUN_00408500((BYTE)pState[((*pState >> 0xc) & 0xf) * 3 + 4] & 0x1f) == -1 &&
            RallyData_FUN_00408500((BYTE)(pState[((*pState >> 0xc) & 0xf) * 3 + 4] >> 5) & 0x1f) == -1)
            goto fail;
        value = 1;
        break;
    case 4:
        if (RallyData_FUN_00408500((BYTE)pState[1] & 0x1f) == -1 &&
            RallyData_FUN_00408500((BYTE)(pState[1] >> 5) & 0x1f) == -1)
            goto fail;
        value = 1;
        break;
    default:
    fail:
        value = 2;
        break;
    }
    RallyData_FUN_00407500(value);
    for (i = 0; i < 2; i++) {
        FUN_0042b720(i, (BYTE)FUN_0040bbc0((unsigned short)i));
        if (i >= (int)(RallyDataState() & 0xff))
            FUN_0042b720(i, -1);
    }
}

// Stage fade timers and race-state flags of the in-race state machine
// (0x472e00). Each "*" object is registered with the timer manager and its
// first byte is the timer slot.
// Magic value handed to FUN_004283e0 as opaque data, not an address.
// GLOBAL: CMR2 0x0051c9a8
int g_unk0x0051c9a8 = 0xacb49c;
// GLOBAL: CMR2 0x0051c9ac
char g_strVs0x0051c9ac[] = "vs.";
// GLOBAL: CMR2 0x0058ca80
BYTE g_unk0x0058ca80;
// GLOBAL: CMR2 0x0058ca84
int g_unk0x0058ca84;
// The five fade-timer objects below live inside the 0x58ca90 block declared in
// StageUI.cpp, so they are addressed as offsets of it (a separate declaration
// would overlap it).
extern BYTE g_unk0x0058ca90[];
#define g_unk0x0058cc70 (g_unk0x0058ca90[0x1e0])
#define g_unk0x0058cc74 (*(int *)(g_unk0x0058ca90 + 0x1e4))
#define g_unk0x0058ce58 (*(int *)(g_unk0x0058ca90 + 0x3c8))
#define g_unk0x0058ce5c (*(unsigned int *)(g_unk0x0058ca90 + 0x3cc))
#define g_unk0x0058cf60 (g_unk0x0058ca90[0x4d0])

extern BYTE *g_unk0x0058ca88;
extern BYTE g_unk0x0058ca8c[4];
extern int g_unk0x0058cf64;
extern int g_unk0x0058cf7c;

typedef void (*FadeCallback)(BYTE index);

extern unsigned int g_unk0x0058cf74;
extern unsigned int g_unk0x0058cf78;
extern int g_unk0x0058cf70;

void FUN_00478c40(void);
void FUN_00418ee0(void);
void FUN_00418780(void);
void FUN_004284d0(unsigned int player, int check);
void FUN_004285b0(unsigned int player, int t, int check);
void FUN_00473470(void);
BYTE FUN_004bc0c0(BYTE *p);
int Timer_GetValue(BYTE index);
void FUN_004bc290(BYTE *p, int, int, int, int, int, BYTE);
void FUN_004bc440(void);
void FUN_004bc470(BYTE *p);
unsigned int *RallyData_GetRoundEntry(void);
int FUN_00428740(BYTE index);
int FUN_00473680(unsigned int *p);
int FUN_00473310(void);
int FUN_00473290(void);
void FUN_0041b310(void);
int FUN_0041b320(void);
void FUN_00455470(char);
void FUN_00472a30(void);
BOOL FUN_0046c500(void);
void FUN_0041c260(void);
void FUN_004283e0(BYTE index, FadeCallback pfnDone, int param3, int param4, int param5, char force);
void FUN_0041f2a0(void);
void RallyData_FUN_004070c0(void);
void FUN_00473360(void);
struct Menu;
void FUN_004734f0(Menu *pMenu);
void FUN_00473540(BYTE index);

// Per-frame step of the in-race state machine: waits for the stage objects of
// the round, runs the state transitions and drives the fade in/out of the
// scene.
// FUNCTION: CMR2 0x00472e00
void FUN_00472e00(BYTE *param_1, unsigned int param_2)
{
    unsigned int *pState;
    unsigned int *pEntry;
    unsigned int state;
    int i;
    pState = RallyData_GetChampionshipState();
    FUN_00478c40();
    FUN_00418ee0();
    FUN_00418780();
    g_unk0x0058ca88 = param_1;
    g_unk0x0058ca84 = param_2 & 0xff;
    for (i = 0; i < 2; i++) {
        FUN_004284d0(i, 0);
        FUN_004285b0(i, 0x10000, 0);
    }
    if (FUN_00428740(0) != 0)
        return;
    if (g_unk0x0058cf6c != 0)
        FUN_00473470();
    if (FUN_004bc0c0(&g_unk0x0058ca80))
        g_unk0x0058cc74 = Timer_GetValue(g_unk0x0058ca80);
    if (FUN_004bc0c0(&g_unk0x0058cf60))
        g_unk0x0058ce58 = Timer_GetValue(g_unk0x0058cf60);
    else
        g_unk0x0058ce58 = 0;

    switch (g_unk0x0058cf7c) {
    case 0:
        FUN_004bc440();
        g_unk0x0058cc74 = 0;
        if ((*pState & 0x38) == 8) {
            FUN_004bc290(&g_unk0x0058ca80, 2, 0xd, 0, 0, 0x10000, 0);
            FUN_004bc290(&g_unk0x0058cf60, 2, 7, 0, 0, 0x10000, 0);
        }
        *pState = *pState & 0xff1fffff;
        FUN_00473360();
        g_unk0x0058cf7c = 1;
        for (i = 0; i < 2; i++) {
            if (i >= (int)(RallyDataState() & 0xff))
                FUN_0042b720(i, -1);
        }
        g_unk0x0058cf64 = 0;
        return;
    case 1:
        if (g_unk0x0058cf64 != 0) {
            FUN_00455470(1);
            g_unk0x0058cf7c = 2;
            g_unk0x0058cf64 = 0;
            return;
        }
        break;
    case 2:
        pEntry = RallyData_GetRoundEntry();
        if (FUN_00473680(pEntry) != 0) {
            g_unk0x0058cf7c = 5;
            FUN_00472a30();
            g_unk0x0058cf78 = 0;
            g_unk0x0058cf74 = 0;
        }
        if (g_unk0x0058cf64 != 0) {
            FUN_00472cb0();
            FUN_0046c500();
            FUN_0041c260();
            if (FUN_00473310() != 0) {
                g_unk0x0058cf7c = 3;
                FUN_00472a30();
                g_unk0x0058cf78 = 0;
                g_unk0x0058cf74 = 0;
                g_unk0x0058cf64 = 0;
                return;
            } else {
                g_unk0x0058ce5c = (unsigned int)RallyData_GetRoundEntry();
                g_unk0x0058cf64 = 0;
                g_unk0x0058cf7c = 4;
                return;
            }
        }
        break;
    case 3:
        if (g_unk0x0058cf64 != 0) {
            g_unk0x0058cf7c = 5;
            FUN_004bc290(&g_unk0x0058cf60, 2, 7, 0, 0, 0x10000, 0);
            g_unk0x0058cf64 = 0;
            return;
        }
        break;
    case 4:
        if (FUN_0041b320() == 0) {
            FUN_0041b310();
            g_unk0x0058cf64 = 0;
            return;
        }
        FUN_004283e0(0, (FadeCallback)FUN_00473540, 1, 0, g_unk0x0051c9a8, 0);
        if ((char)RallyDataState() == 2)
            FUN_004283e0(1, NULL, 1, 0, g_unk0x0051c9a8, 0);
        break;
    case 5:
        if ((*pState & 0x400000) == 0) {
            g_unk0x0058cf64 = 0;
            g_unk0x0058cf7c = 2;
            return;
        }
        if ((*pState & 0x38) == 0x20) {
            if (g_unk0x0058cf64 != 0) {
                g_unk0x0058cf64 = 0;
                g_unk0x0058cf7c = 7;
                return;
            }
        } else if (g_unk0x0058cf64 != 0) {
            g_unk0x0058cc74 = 0x10000;
            FUN_004bc290(&g_unk0x0058cc70, 2, 0xd, 0, 0, 0x10000, 0);
            g_unk0x0058cf64 = 0;
            g_unk0x0058cf7c = 6;
            return;
        }
        break;
    case 6:
        if (FUN_004bc0c0(&g_unk0x0058cc70)) {
            g_unk0x0058cc74 = Timer_GetValue(g_unk0x0058cc70);
            g_unk0x0058cf64 = 0;
            return;
        }
        if (!FUN_004bc0c0(g_unk0x0058ca8c)) {
            FUN_004bc470(&g_unk0x0058cc70);
            g_unk0x0058cc74 = 0;
            FUN_004bc290(g_unk0x0058ca8c, 2, 0xd, 0, 0, 0x10000, 1);
            g_unk0x0058cf64 = 0;
            return;
        }
        g_unk0x0058cc74 = Timer_GetValue(g_unk0x0058ca8c[0]);
        if (!FUN_004bc0c0(g_unk0x0058ca8c))
            g_unk0x0058cc74 = 0;
        if (g_unk0x0058cf64 != 0 || !FUN_004bc0c0(g_unk0x0058ca8c)) {
            FUN_004bc440();
            g_unk0x0058cc74 = 0;
            g_unk0x0058ce58 = 0;
            g_unk0x0058cf70 = FUN_00473290();
            state = *pState;
            *pState = (((state & 0xfffffff8) + 8 ^ state) & 0x38 ^ state) & 0xffbf0fff | 0x200000;
            RallyData_FUN_004070c0();
            FUN_0041f2a0();
            i = 0;
            if (*g_unk0x0058ca88 > 0) {
                do {
                    CGame::FUN_0049c1c0((Unk0049c2c0 *)g_unk0x0058ca88, i, 1, 2);
                    i++;
                } while (i < (int)*g_unk0x0058ca88);
            }
            g_unk0x0058cf7c = 0;
            g_unk0x0058cf64 = 0;
            return;
        }
        break;
    case 7:
        FUN_004283e0(0, (FadeCallback)FUN_004734f0, 1, 0, g_unk0x0051c9a8, 0);
        if ((char)RallyDataState() == 2)
            FUN_004283e0(1, NULL, 1, 0, g_unk0x0051c9a8, 0);
        break;
    }
    g_unk0x0058cf64 = 0;
}

// FUNCTION: CMR2 0x00473680
int FUN_00473680(unsigned int *p)
{
    if ((*p & 0x1f) != 0x1f && (*p & 0x3e0) != 0x3e0)
        return 0;
    return 1;
}

int *RallyData_FUN_00407f20(int index);

// Name of the driver on the given side (0 first, 1 second) of a knockout
// match: the player's name, the AI name, or "" for an empty slot.
// match 55%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004736b0
char *FUN_004736b0(KnockoutMatch *pMatch, int side)
{
    unsigned int driver;

    if (side == 0) {
        if ((pMatch->flags & 0x1f) == 0x1f)
            return CMain::m_logFileBlankLine;
        if (RallyData_FUN_00408500(pMatch->flags & 0x1f) != -1) {
            sprintf(CFrontend::m_stringDest, CAIHelper::GetNameForID(RallyData_FUN_00408500(pMatch->flags & 0x1f)));
            return CFrontend::m_stringDest;
        }
        driver = pMatch->flags;
    } else {
        if (side != 1)
            return CFrontend::m_stringDest;
        if ((pMatch->flags & 0x3e0) == 0x3e0)
            return CMain::m_logFileBlankLine;
        if (RallyData_FUN_00408500((pMatch->flags >> 5) & 0x1f) != -1) {
            sprintf(CFrontend::m_stringDest,
                    CAIHelper::GetNameForID(RallyData_FUN_00408500((pMatch->flags >> 5) & 0x1f)));
            return CFrontend::m_stringDest;
        }
        driver = pMatch->flags >> 5;
    }
    sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, (char *)RallyData_GetRecord(driver & 0x1f));
    return CFrontend::m_stringDest;
}

// Whether the given side of the match is the human player.
// FUNCTION: CMR2 0x00473790
int FUN_00473790(KnockoutMatch *pMatch, int side)
{
    unsigned int driver;

    if (side == 0)
        driver = pMatch->flags;
    else
        driver = pMatch->flags >> 5;
    driver &= 0x1f;
    if (driver == 0x1f)
        return 0;
    return RallyData_FUN_00408500(driver) == -1;
}

// Side of the match to show: the human player's side when it is param2,
// otherwise the other side.
// FUNCTION: CMR2 0x004737d0
int FUN_004737d0(KnockoutMatch *pMatch, int param2)
{
    int slot = pMatch->flags & 0x1f;

    if (slot != 0x1f && RallyData_FUN_00408500(slot) == -1)
        return param2 != 0;
    return param2 == 0;
}

// Same as FUN_004736b0 with the car names of the AI drivers.
// match 55%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00473810
char *FUN_00473810(KnockoutMatch *pMatch, int side)
{
    unsigned int driver;

    if (side == 0) {
        if ((pMatch->flags & 0x1f) == 0x1f)
            return CMain::m_logFileBlankLine;
        if (RallyData_FUN_00408500(pMatch->flags & 0x1f) != -1) {
            sprintf(CFrontend::m_stringDest, (char *)RallyData_FUN_00407f20(RallyData_FUN_00408500(pMatch->flags & 0x1f)));
            return CFrontend::m_stringDest;
        }
        driver = pMatch->flags;
    } else {
        if (side != 1)
            return CFrontend::m_stringDest;
        if ((pMatch->flags & 0x3e0) == 0x3e0)
            return CMain::m_logFileBlankLine;
        if (RallyData_FUN_00408500((pMatch->flags >> 5) & 0x1f) != -1) {
            sprintf(CFrontend::m_stringDest,
                    (char *)RallyData_FUN_00407f20(RallyData_FUN_00408500((pMatch->flags >> 5) & 0x1f)));
            return CFrontend::m_stringDest;
        }
        driver = pMatch->flags >> 5;
    }
    sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, (char *)RallyData_GetRecord(driver & 0x1f));
    return CFrontend::m_stringDest;
}

extern int g_unk0x0058cf64;
// GLOBAL: CMR2 0x0058cf74
unsigned int g_unk0x0058cf74;
// GLOBAL: CMR2 0x0058cf78
unsigned int g_unk0x0058cf78;

extern char g_minSecMSECFormatString[];

// Formats one of the two lap time fields as "%02d:%02d.%02d", printing the
// identical placeholder while the value eases towards its target.
// match 68%: same logic; MSVC keeps the two eased values in different
// registers and spills one extra
// match 68%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004752f0
char *FUN_004752f0(int *p, int index, int mode)
{
    unsigned int current1;
    unsigned int current2;
    int time1;
    int time2;

    time1 = p[1];
    time2 = p[2];
    if (mode != 0) {
        current1 = g_unk0x0058cf74;
        if (current1 <= (unsigned)(time1 - 0x27))
            current1 += 0x27;
        else
            current1 = time1;
        g_unk0x0058cf74 = current1;
        current2 = g_unk0x0058cf78;
        if (current2 <= (unsigned)(time2 - 0x27))
            current2 += 0x27;
        else
            current2 = time2;
        g_unk0x0058cf78 = current2;
        if (current1 == (unsigned)time1 && current2 == (unsigned)time2)
            g_unk0x0058cf64 = 1;
    }
    if (index == 0) {
        if ((p[0] & 0x1f) == 0x1f)
            return CMain::m_logFileBlankLine;
        RallyData_FUN_00408500((BYTE)(p[0] & 0x1f));
        sprintf(CFrontend::m_stringDest, g_minSecMSECFormatString, time1 / 6000, (time1 % 6000) / 100,
                time1 % 100);
    } else if (index == 1) {
        if ((p[0] & 0x3e0) == 0x3e0)
            return CMain::m_logFileBlankLine;
        RallyData_FUN_00408500((BYTE)((p[0] >> 5) & 0x1f));
        sprintf(CFrontend::m_stringDest, g_minSecMSECFormatString, time2 / 6000, (time2 % 6000) / 100,
                time2 % 100);
    }
    return CFrontend::m_stringDest;
}

// Builds the in-race menu of two items (returned by FUN_00475f70); both
// actions are forwarded to CGame (0x49c070 / 0x49c080).
void FUN_0049c070(Menu *pMenu, int param);
void FUN_0049c080(Menu *pMenu, int param);
void FUN_0049bcb0(Menu *pMenu);
// FUNCTION: CMR2 0x00475f00
void FUN_00475f00(void)
{
    Menu_Init((Menu *)g_unk0x0058cf80, 0, -1, 0, NULL, NULL, 1, 0, 1);
    Menu_AddItemType4((Menu *)g_unk0x0058cf80, 0, 0xf4, (int)FUN_0049c070, -1);
    Menu_AddItemType4((Menu *)g_unk0x0058cf80, 0, 0xf5, (int)FUN_0049c080, -1);
    Menu_SetCallbacks((Menu *)g_unk0x0058cf80, NULL, NULL, (MenuCallback)FUN_0049bcb0, NULL);
    Menu_ValidateCursor((Menu *)g_unk0x0058cf80, 0);
}

// FUNCTION: CMR2 0x00475f70
BYTE *FUN_00475f70(void)
{
    return g_unk0x0058cf80;
}

// Updates one stage object's state byte and re-syncs its scene node with the
// given source matrix; when the node ends up in another sector it is detached
// and released from the scene again.
// FUNCTION: CMR2 0x00476410
void FUN_00476410(BYTE *p, int *src, int unused, BYTE value)
{
    int index;
    FixVector pos;

    g_unk0x0058d4d0[*p] = value;
    FUN_00477850((int)p, src);
    if (g_unk0x0058d49c[p[2]] != NULL) {
        SceneNode_Unused((SceneNode *)g_unk0x0058d49c[p[2]]);
        pos.x = ((SceneNode *)g_unk0x0058d49c[p[2]])->world.position.x;
        pos.y = ((SceneNode *)g_unk0x0058d49c[p[2]])->world.position.y;
        pos.z = ((SceneNode *)g_unk0x0058d49c[p[2]])->world.position.z;
        index = (short)Sector_FromPosition(&pos);
        if (index != ((SceneNode *)g_unk0x0058d49c[p[2]])->sector) {
            FUN_004b8b10((SceneNode *)g_unk0x0058d49c[p[2]]);
            SceneNode_Unused((SceneNode *)g_unk0x0058d49c[p[2]]);
        }
    }
}

// FUNCTION: CMR2 0x004764e0
void FUN_004764e0(BYTE *p)
{
    g_unk0x0058d3b0[p[2]] = 1;
}

// FUNCTION: CMR2 0x00476520
int FUN_00476520(BYTE index)
{
    return g_unk0x0058d6a8[index];
}

// Caches, for a car, whether its class is special and pointers into its
// timing record.
// FUNCTION: CMR2 0x00476540
void FUN_00476540(int index)
{
    Car *pCar = Car_Get(index);
    char type = pCar->field_0xb1b[0];
    int p;

    if (type == 8 || type == 7 || type == 9 || type == 13)
        g_unk0x0058d2f0[index] = 1;
    else
        g_unk0x0058d2f0[index] = 0;
    p = FUN_00457e10((BYTE *)pCar, 5);
    ((int *)g_unk0x0058d4f0)[index * 7] = p;
    p += 8;
    ((int *)g_unk0x0058d4f0)[index * 7 + 1] = p;
    p += 4;
    ((int *)g_unk0x0058d4f0)[index * 7 + 2] = p;
    p += 0xc;
    ((int *)g_unk0x0058d4f0)[index * 7 + 3] = p;
    p += 4;
    ((int *)g_unk0x0058d4f0)[index * 7 + 4] = p;
    p += 8;
    ((int *)g_unk0x0058d4f0)[index * 7 + 5] = p;
    p += 8;
    ((int *)g_unk0x0058d4f0)[index * 7 + 6] = p;
}

// Draws a stage box: a filled rectangle, its one pixel outline and, optionally,
// the championship round box texture scaled to the rectangle.
// FUNCTION: CMR2 0x00475740
void FUN_00475740(short *pRect, BYTE *pColour, BYTE *pEdgeColour, int drawTexture)
{
    short edge[4];
    SpriteRect dest;

    if (pColour != NULL)
        Sprite_FillRect((int)g_pGraphics + 0x150, pRect, pColour, 2);
    if (pEdgeColour != NULL) {
        edge[0] = pRect[0];
        edge[1] = pRect[1];
        edge[2] = pRect[2];
        edge[3] = 1;
        Sprite_FillRect((int)g_pGraphics + 0x150, edge, pEdgeColour, 2);
        edge[0] = (short)(pRect[0] + pRect[2]);
        edge[1] = pRect[1];
        edge[2] = 1;
        edge[3] = (short)(pRect[3] + 1);
        Sprite_FillRect((int)g_pGraphics + 0x150, edge, pEdgeColour, 2);
        edge[0] = pRect[0];
        edge[1] = (short)(pRect[1] + pRect[3]);
        edge[2] = pRect[2];
        edge[3] = 1;
        Sprite_FillRect((int)g_pGraphics + 0x150, edge, pEdgeColour, 2);
        edge[0] = pRect[0];
        edge[1] = pRect[1];
        edge[2] = 1;
        edge[3] = pRect[3];
        Sprite_FillRect((int)g_pGraphics + 0x150, edge, pEdgeColour, 2);
    }
    if (drawTexture != 0) {
        dest.x = pRect[0];
        dest.y = pRect[1];
        // The texture quad is scaled to the rectangle and to the reference
        // resolution (640x480) it was authored for.
        dest.w = (short)(((int)((Texture *)FUN_00405600())->width * (int)pRect[2] * (int)g_pGraphics->resX / 0x280) /
                          ((int)g_pGraphics->resX * 0x55 / 0x280));
        dest.h = (short)(((int)((Texture *)FUN_00405600())->height * (int)pRect[3] * (int)g_pGraphics->resY / 0x1e0) /
                          ((int)g_pGraphics->resY * 0x26 / 0x1e0));
        Sprite_Queue((SpriteRect *)(FUN_00405600() + 0x11c), &dest, (Texture *)FUN_00405600(), 2, 0, 0, 0,
                     pEdgeColour, 8);
    }
}

// Blends a car's stage object transform: rotates its mount nodes by the car
// heading and interpolates the blended node's matrix between the reference
// node and the rotated mount using the body fade factor.
// FUNCTION: CMR2 0x00476640
void FUN_00476640(int car)
{
    // Per car fade curve (16.16), sampled with index = 12 * fade.
    int fadeCurve[13] = {0, 0x51e, 0xccc, 0x1999, 0x3333, 0x6666, 0x9999, 0xcccc,
                         0xe666, 0xf333, 0xfae1, 0x10000, 0x10000};
    Car *pCar = Car_Get(car);
    SceneNode *pRef = *(SceneNode **)(g_stageBlock + 0x288 + car * 0x1c);
    SceneNode *pBlend = *(SceneNode **)(g_stageBlock + 0x28c + car * 0x1c);
    SceneNode *pMid = *(SceneNode **)(g_stageBlock + 0x290 + car * 0x1c);
    SceneNode *pRot = *(SceneNode **)(g_stageBlock + 0x298 + car * 0x1c);
    short angle = -pCar->heading;
    FixVector axis;
    FixMatrix rot;
    FixMatrix combined;
    FixMatrix original;
    int posX;
    int posY;
    int posZ;
    int fade;

    SceneNode_SetRotation(pRot, *(FixAngles **)(g_stageBlock + 0x260 + car * 0x1c));
    axis = pRot->current.right;
    FixMatrix_FromAxisAngle(&rot, &axis, angle);

    // Rotate the node without touching its translation.
    posX = pRot->current.position.x;
    posY = pRot->current.position.y;
    posZ = pRot->current.position.z;
    pRot->current.position.x = 0;
    pRot->current.position.y = 0;
    pRot->current.position.z = 0;
    FixMatrix_Multiply(&pRot->current, &pRot->current, &rot);
    pRot->current.position.x = posX;
    pRot->current.position.y = posY;
    pRot->current.position.z = posZ;

    SceneNode_SetRotation(pMid, *(FixAngles **)(g_stageBlock + 0x260 + car * 0x1c));
    FixMatrix_Multiply(&combined, &pMid->current, &rot);
    combined.position.x += posX;
    combined.position.y += posY;
    combined.position.z += posZ;

    original = pRef->current;

    fade = FUN_00476850(car, (int)pCar);
    FixMatrix_Interpolate(&pBlend->current, &combined, &original, 0, 0,
                          fadeCurve[FixMulShift32(0xC0000, fade)], 1);
}

// FUNCTION: CMR2 0x00477a90
void FUN_00477a90(void)
{
    memset(g_unk0x0058d6b0, 0xff, 7 * 4);
    memset(g_unk0x0058d2a0, 0xff, 12 * 4);
}

// FUNCTION: CMR2 0x00478170
void FUN_00478170(int index)
{
    g_unk0x0058da30[index] = 0;
}

// FUNCTION: CMR2 0x00478190
void FUN_00478190(int index, int seconds)
{
    g_unk0x0058da30[index] += seconds * 100;
}

// FUNCTION: CMR2 0x004781c0
int FUN_004781c0(int index)
{
    return g_unk0x0058da30[index];
}

// FUNCTION: CMR2 0x004789b0
void FUN_004789b0(BYTE *pCar)
{
    *(int *)(pCar + 0xa74) = *(int *)(pCar + 0xa78);
}

// Second group (0x4805f0-0x49e940)


extern void *g_unk0x00592734;
void FUN_0046f4c0(int *pOut);
void FUN_004ae410(BYTE a, BYTE b, int c, int d);

// GLOBAL: CMR2 0x005909bc
int g_unk0x005909bc;
// GLOBAL: CMR2 0x005909c8
FixVector g_unk0x005909c8[4];
// GLOBAL: CMR2 0x00590b7c
BYTE *g_unk0x00590b7c[4][8];
// GLOBAL: CMR2 0x00590bfc
BYTE g_unk0x00590bfc;
// GLOBAL: CMR2 0x00590bfd
BYTE g_unk0x00590bfd;
// GLOBAL: CMR2 0x00590c24
BYTE g_unk0x00590c24[4][8];
// GLOBAL: CMR2 0x00590c44
int g_unk0x00590c44;
// GLOBAL: CMR2 0x00590c48
int g_unk0x00590c48;
// GLOBAL: CMR2 0x00590c4c
int g_unk0x00590c4c;
// GLOBAL: CMR2 0x00590c50
int g_unk0x00590c50;
// GLOBAL: CMR2 0x00590c60
BYTE g_unk0x00590c60[4];
// GLOBAL: CMR2 0x00590d70
int g_unk0x00590d70;
// GLOBAL: CMR2 0x00590db0
int g_unk0x00590db0[64];
// GLOBAL: CMR2 0x00590ed0
BYTE g_unk0x00590ed0[8][0x98];
// GLOBAL: CMR2 0x00591390
int g_unk0x00591390;
// GLOBAL: CMR2 0x005913d8
int g_unk0x005913d8;
// GLOBAL: CMR2 0x005913dc
BYTE g_unk0x005913dc[4];
// GLOBAL: CMR2 0x005913f8
BYTE g_unk0x005913f8[8][8];
// GLOBAL: CMR2 0x0059146c
int g_unk0x0059146c[8];
// GLOBAL: CMR2 0x005914c8
FixVector g_unk0x005914c8;
// GLOBAL: CMR2 0x005916a0
FixVector g_unk0x005916a0[4];
// GLOBAL: CMR2 0x005916f0
int g_unk0x005916f0[4];
// GLOBAL: CMR2 0x00591868
FixVector g_unk0x00591868[4];
// GLOBAL: CMR2 0x00591898
FixVector g_unk0x00591898[4];
// GLOBAL: CMR2 0x00591740
int g_unk0x00591740[4];
// GLOBAL: CMR2 0x00591750
BYTE *g_unk0x00591750;
// GLOBAL: CMR2 0x005918c8
int g_unk0x005918c8;
// GLOBAL: CMR2 0x005920f0
BYTE *g_unk0x005920f0;
// GLOBAL: CMR2 0x00592114
FixVector g_unk0x00592114;
// GLOBAL: CMR2 0x00592128
int g_unk0x00592128;
// GLOBAL: CMR2 0x0059212c
int g_unk0x0059212c;
// GLOBAL: CMR2 0x00592130
int g_unk0x00592130;
// GLOBAL: CMR2 0x00592134
int g_unk0x00592134;

// GLOBAL: CMR2 0x00590af8
void *g_unk0x00590af8;
// GLOBAL: CMR2 0x00590afc
BYTE g_unk0x00590afc;
// GLOBAL: CMR2 0x00590b00
int g_unk0x00590b00;
// GLOBAL: CMR2 0x00590b04
void *g_unk0x00590b04;
// GLOBAL: CMR2 0x00590b08
void *g_unk0x00590b08;
// GLOBAL: CMR2 0x00590b0c
void **g_unk0x00590b0c;

// GLOBAL: CMR2 0x0051ea20
char g_strMenuSoundNames[5][7] = {"move", "select", "back", "error", "toggle"};

extern char g_strMenuSoundFormat[24];
extern DWORD g_unk0x0058dc58;

// Loads the five menu sounds from the common frontend archive, for the stage
// menus. Returns 0 if any sample failed to load.
// FUNCTION: CMR2 0x00478b80
BYTE FUN_00478b80(void)
{
    char *name;
    BYTE result;

    result = 1;
    g_unk0x0058dc58 = 0;
    name = &g_strMenuSoundNames[0][0];
    do {
        sprintf(CFrontend::m_stringDest, g_strMenuSoundFormat, CInstallInfo::GetGameCDPath(), name);
        if (Sound_LoadSample(CFrontend::m_stringDest, 0, (GenericFile *)StageTiming_GetStageFile0()) == 0)
            result = 0;
        name += 7;
    } while ((int)name < (int)&g_strMenuSoundNames[5][0]);
    return result;
}

// Releases the vehicle files loaded by 0x47e4d0 (registered callback).
// FUNCTION: CMR2 0x0047ea20
BYTE FUN_0047ea20(void)
{
    int i;

    if (g_unk0x00590af8 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00590af8);
        g_unk0x00590af8 = NULL;
    }
    if (g_unk0x00590b04 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00590b04);
        g_unk0x00590b04 = NULL;
    }
    if (g_unk0x00590b08 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00590b08);
        g_unk0x00590b08 = NULL;
    }
    if (g_unk0x00590b0c != NULL) {
        for (i = 0; i < 3; i++) {
            if (g_unk0x00590b0c[i] != NULL) {
                CFileBuffer::FreeGenericFileBuffer(g_unk0x00590b0c[i]);
                g_unk0x00590b0c[i] = NULL;
            }
        }
        if (g_unk0x00590b0c != NULL) {
            CFileBuffer::FreeGenericFileBuffer(g_unk0x00590b0c);
            g_unk0x00590b0c = NULL;
        }
    }
    g_unk0x00590afc = 0;
    return 1;
}

extern float g_oneOverRandMax;

// Colours picked for the vehicle debris fragments.
// GLOBAL: CMR2 0x0051f4ec
DWORD g_stageDebrisPalette[6] = {
    0xffb34aff, 0xffff5c30, 0xffffff3d,
    0xff4ec4ff, 0xff2368ff, 0xff27ff9e
};

// Starts a vehicle debris effect in the first free slot, including its fragments and sound.
// match 36%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047fcb0
void StageObject_SpawnDebris(const FixVector *pPosition, const FixVector *pVelocity, unsigned int variant)
{
    BYTE *pPool = (BYTE *)g_unk0x00590af8;
    int slotIndex = -1;
    int count = g_unk0x00590afc;
    int i;
    for (i = 0; i < count; ++i) {
        if (*(int *)(pPool + i * 0x938 + 0x694) == 0) {
            slotIndex = i;
            break;
        }
    }
    if (slotIndex < 0)
        return;

    BYTE *pSlot = pPool + slotIndex * 0x938;
    *(int *)(pSlot + 0x694) = 1;
    memcpy(pSlot, pPosition, sizeof(FixVector));
    memcpy(pSlot + 0x1e0, pPosition, sizeof(FixVector));
    memcpy(pSlot + 0xc, pVelocity, sizeof(FixVector));
    pSlot[0x690] = 0;
    pSlot[0x691] = 0;
    for (i = 0; i < 20; ++i) {
        memcpy(pSlot + 0x18 + i * 12, pPosition, sizeof(FixVector));
        memcpy(pSlot + 0x1f8 + i * 12, pPosition, sizeof(FixVector));
        *(int *)(pSlot + 0x69c + i * 4) = 0;
    }
    *(int *)(pSlot + 0x934) = 0;

    int randomFixed;
    if (variant == 0) {
        *(int *)(pSlot + 0x664) = 0x190000;
    } else {
        randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
        *(int *)(pSlot + 0x664) = FixMul(randomFixed, 0xa0000) + 0x50000;
    }
    *(int *)(pSlot + 0x66c) = 0xa3d;
    *(int *)(pSlot + 0x670) = 0x624;

    int colourIndex = rand() % 6;
    pSlot[0x692] = (BYTE)colourIndex;
    *(DWORD *)(pSlot + 0x678) = g_stageDebrisPalette[(BYTE)colourIndex];
    pSlot[0x67b] = 0xff;
    *(DWORD *)(pSlot + 0x67c) = *(DWORD *)(pSlot + 0x678);
    *(DWORD *)(pSlot + 0x680) = *(DWORD *)(pSlot + 0x678);
    colourIndex = rand() % 6;
    *(DWORD *)(pSlot + 0x684) = g_stageDebrisPalette[colourIndex];
    pSlot[0x687] = 0xff;
    colourIndex = rand() % 6;
    *(DWORD *)(pSlot + 0x68c) = g_stageDebrisPalette[colourIndex];
    pSlot[0x68f] = 0xff;
    colourIndex = rand() % 6;
    *(DWORD *)(pSlot + 0x688) = g_stageDebrisPalette[colourIndex];
    pSlot[0x68b] = 0xff;

    randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
    if (variant == 0)
        *(int *)(pSlot + 0x698) = randomFixed < 0x8000 ? 1 : 2;
    else
        *(int *)(pSlot + 0x698) = randomFixed < 0x10001 ? 1 : 0;

    if (*(int *)(pSlot + 0x698) == 1) {
        int chance = FixMul((rand() % 9 + 1) * 0x10000, 0x1999);
        randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
        if (randomFixed < 0xfd71) {
            *(int *)(pSlot + 0x930) = 0;
        } else {
            *(int *)(pSlot + 0x930) = 1;
            chance = FixMul(chance, 0x20000);
        }
        int enabled = 0;
        int row, column, fragment;
        for (row = 0; row < 3; ++row) {
            for (column = 0; column < 6; ++column) {
                for (fragment = 0; fragment < 4; ++fragment) {
                    int offset = (row * 6 + column) * 4 + fragment;
                    randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
                    int on = randomFixed <= chance;
                    *(int *)(pSlot + 0x6ec + offset * 4) = on;
                    enabled += on;
                    randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
                    *(int *)(pSlot + 0x80c + offset * 4) = randomFixed >= 0x8001;
                }
            }
        }
        int density = FixDiv(enabled << 16, 0x480000);
        *(int *)(pSlot + 0x668) = FixMul(density, 0xf0000) + 0xa0000;
        *(int *)(pSlot + 0x92c) = 1;
        *(int *)(pSlot + 0x660) = FixMul(0x10000 - density, 0xccc) + 0x11eb;
        if (*(int *)(pSlot + 0x930) != 0) {
            *(int *)(pSlot + 0x674) = *(int *)(pSlot + 0x668);
            *(int *)(pSlot + 0x668) = FixMul(*(int *)(pSlot + 0x668), 0x20000);
        }
    } else {
        memset(pSlot + 0x6ec, 0, 0x48 * 4);
        *(int *)(pSlot + 0x668) = 0x50000;
        *(int *)(pSlot + 0x660) = 0x11eb;
        *(int *)(pSlot + 0x92c) = 0;
    }

    FUN_004b7790((unsigned short)(g_unk0x005909bc + 10), 0xccc, 0x5622, 0, 0, 0);
    randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
    if (randomFixed > 0x1999) {
        randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
        int volume = FixMul(randomFixed, 0x4000) + 0x4000;
        pSlot[0x693] = (BYTE)FUN_004b7790((unsigned short)(g_unk0x005909bc + rand() % 2),
                                            volume, 0x5622, 0, 0, 0);
    } else {
        pSlot[0x693] = 0xff;
    }
}

extern double g_unk0x00511300;

// Spawns one debris burst at a random entry of the four-way spawn table with a
// random, upward-biased velocity of length 1.5..2.5.
// FUNCTION: CMR2 0x00480380
void FUN_00480380(void)
{
    FixVector velocity;
    FixVector *pPosition;
    unsigned short angle;
    int randomFixed;
    int randomSpread;

    randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
    angle = (unsigned short)(__int64)((double)FixMul(randomFixed, 0x1680000) * g_unk0x00511300);
    pPosition = &g_unk0x005909c8[rand() % 4];

    velocity.x = g_sinTable[angle & 0xfff];
    velocity.y = 0;
    velocity.z = g_sinTable[(angle + 0x400) & 0xfff];

    FixVecScale(&velocity, &velocity, (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536));
    velocity.y = 0x40000;

    FIX_NORMALIZE_INTO(velocity, velocity)

    randomSpread = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
    FixVecScale(&velocity, &velocity, FixMul(0x10000, randomSpread) + 0x18000);

    StageObject_SpawnDebris(pPosition, &velocity, 0);
}

// FUNCTION: CMR2 0x004805f0
void FUN_004805f0(int value)
{
    g_unk0x005909bc = value;
}

// Interpolates every active debris slot's derived position arrays one step
// toward their targets by `scale` (16.16 fixed point).
// FUNCTION: CMR2 0x00480600
void StageObject_UpdateDebris(int scale)
{
    BYTE *pSlot;
    FixVector delta;
    int *p;
    int *q;
    int *d;
    int index;
    int offset;
    int count;
    int row;
    int column;

    index = 0;
    if (g_unk0x00590afc == 0)
        return;
    offset = 0;
    do {
        pSlot = (BYTE *)g_unk0x00590af8 + offset;
        if (*(int *)(pSlot + 0x694) != 0) {
            delta.x = *(int *)pSlot - *(int *)(pSlot + 0x1e0);
            delta.y = *(int *)(pSlot + 4) - *(int *)(pSlot + 0x1e4);
            delta.z = *(int *)(pSlot + 8) - *(int *)(pSlot + 0x1e8);
            FixVecScale(&delta, &delta, scale);
            *(int *)(pSlot + 0x1ec) = *(int *)(pSlot + 0x1e0) + delta.x;
            *(int *)(pSlot + 0x1f0) = *(int *)(pSlot + 0x1e4) + delta.y;
            *(int *)(pSlot + 0x1f4) = *(int *)(pSlot + 0x1e8) + delta.z;

            p = (int *)(pSlot + 0x18);
            q = (int *)(pSlot + 0x1f8);
            d = (int *)(pSlot + 0x2e8);
            count = 20;
            do {
                delta.x = p[0] - q[0];
                delta.y = p[1] - q[1];
                delta.z = p[2] - q[2];
                FixVecScale(&delta, &delta, scale);
                d[0] = q[0] + delta.x;
                d[1] = q[1] + delta.y;
                d[2] = q[2] + delta.z;
                p += 3;
                q += 3;
                d += 3;
            } while (--count);

            p = (int *)(pSlot + 0x108);
            q = (int *)(pSlot + 0x3d8);
            d = (int *)(pSlot + 0x4b0);
            row = 3;
            do {
                column = 6;
                do {
                    delta.x = p[0] - q[0];
                    delta.y = p[1] - q[1];
                    delta.z = p[2] - q[2];
                    FixVecScale(&delta, &delta, scale);
                    d[0] = q[0] + delta.x;
                    d[1] = q[1] + delta.y;
                    d[2] = q[2] + delta.z;
                    p += 3;
                    q += 3;
                    d += 3;
                } while (--column);
            } while (--row);
        }
        index++;
        offset += 0x938;
    } while (index < (int)(g_unk0x00590afc & 0xff));
}
// FUNCTION: CMR2 0x00480a50
void FUN_00480a50(void)
{
    g_unk0x00590d70 = 0;
}

// FUNCTION: CMR2 0x00480ac0
void FUN_00480ac0(BYTE *pCar, int slot, int reset)
{
    if (g_unk0x00590b7c[slot][(signed char)pCar[0xb1a]] != NULL && reset != 0)
        g_unk0x00590b7c[slot][(signed char)pCar[0xb1a]][0x17c] = 0;
}

// FUNCTION: CMR2 0x00484d10
unsigned int FUN_00484d10(int i, int j)
{
    return g_unk0x00590c24[i][j];
}

// FUNCTION: CMR2 0x00484de0
BYTE *FUN_00484de0(BYTE *pCar, int slot)
{
    return g_unk0x00590b7c[slot][(signed char)pCar[0xb1a]];
}

struct Unk0x00590d74;
extern Unk0x00590d74 *g_unk0x00590d74;
extern int g_unk0x00590b30[8];
extern void **g_unk0x00590c6c;
extern int g_unk0x00590c00[8];
extern FixVector g_unk0x00590b50;

// Steps one 0x3c-byte record (`index`) of the current car's stage-object list:
// the record's +0x0 vector is copied into +0xc, the +0x30/+0x34 velocities lose
// the part of g_unk0x00590b50 that lies along the record's axis vector, and then
// a damped spring (k = 0x20000, c = 0x3333, step = g_physicsTimeStep) integrates
// +0x0/+0x2. Records with the +0x38 flag set are skipped.
// FUNCTION: CMR2 0x004854a0
void FUN_004854a0(int index)
{
    int car = *(char *)((BYTE *)g_unk0x00590d74 + 0xb1a);
    int *pRecord = (int *)((BYTE *)g_unk0x00590c6c[car] + (index & 0xff) * 0x3c);
    FixVector *pAxis = (FixVector *)(g_unk0x00590c00[car] + (index & 0xff) * 0x20 + 0xc);
    FixVector velocity;
    FixVector projected;
    int dot;

    if (pRecord[0xe] != 0)
        return;

    pRecord[3] = pRecord[0];
    pRecord[4] = pRecord[1];
    pRecord[5] = pRecord[2];

    velocity = g_unk0x00590b50;
    dot = FixVecDot(&velocity, pAxis);
    FixVecScale(&projected, pAxis, dot);
    velocity.x -= projected.x;
    velocity.y -= projected.y;
    velocity.z -= projected.z;

    pRecord[0xc] -= velocity.x;
    pRecord[0xd] -= velocity.z;

    pRecord[0xc] += FixMul(g_physicsTimeStep,
                           -(FixMul(0x20000, pRecord[0]) + FixMul(0x3333, pRecord[0xc])));
    pRecord[0xd] += FixMul(g_physicsTimeStep,
                           -(FixMul(0x20000, pRecord[2]) + FixMul(0x3333, pRecord[0xd])));
    pRecord[0] += FixMul(g_physicsTimeStep, pRecord[0xc]);
    pRecord[2] += FixMul(g_physicsTimeStep, pRecord[0xd]);
    pRecord[1] = 0x10000;
}

// Steps every 0x3c-byte record of the ordered cars' lists: the record's +0x18
// vector becomes its +0xc vector plus the (+0x0 - +0xc) difference scaled by
// `scale`.
// match 65%: same logic; register allocation and the inner loop scheduling differ
// match 65%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00486500
void FUN_00486500(int scale)
{
    int count;
    int n;
    int offset;
    short *pIndex;
    int *p;
    FixVector v;

    pIndex = Car_GetOrder();
    count = Car_GetOrderCount();
    if (count - 1 >= 0) {
        pIndex += count - 1;
        do {
            g_unk0x00590d74 = (Unk0x00590d74 *)Car_Get(*pIndex);
            if (*(int *)((BYTE *)g_unk0x00590d74 + 0xc0c) == 0) {
                n = *(int *)g_unk0x00590b30[*(char *)((BYTE *)g_unk0x00590d74 + 0xb1a)] - 1;
                if (n >= 0) {
                    offset = n * 0x3c;
                    n++;
                    do {
                        p = (int *)((BYTE *)g_unk0x00590c6c[*(char *)((BYTE *)g_unk0x00590d74 + 0xb1a)] +
                                    offset);
                        v.x = p[0] - p[3];
                        v.y = p[1] - p[4];
                        v.z = p[2] - p[5];
                        FixVecScale(&v, &v, scale);
                        p[6] = p[3] + v.x;
                        p[7] = p[4] + v.y;
                        p[8] = p[5] + v.z;
                        offset -= 0x3c;
                    } while (--n);
                }
            }
            pIndex--;
        } while (--count);
    }
}

// FUNCTION: CMR2 0x00486be0
void FUN_00486be0(BYTE *p, int unused)
{
    g_unk0x00590db0[*p] = 0x10000;
}

// FUNCTION: CMR2 0x00486c00
void FUN_00486c00(BYTE *p, BYTE *q)
{
    FUN_004ae410(q[2], q[1], 0, 0);
    g_unk0x00590db0[*p] = 0x10000;
}

// FUNCTION: CMR2 0x00487130
int FUN_00487130(void)
{
    return g_unk0x00591390;
}

// Applies a per-frame delta to one stage object (plus an optional second car
// index) and recurses into its children; `flag` enables the 0x5913d8 path.
// FUNCTION: CMR2 0x0048c870
void FUN_0048c870(BYTE index, BYTE other, int *pDelta, int flag)
{
    int i;

    g_unk0x005914c8.x = pDelta[0];
    g_unk0x005914c8.y = pDelta[1];
    g_unk0x005914c8.z = pDelta[2];
    g_unk0x005913d8 = flag;
    memset(g_unk0x0059146c, 0, sizeof(g_unk0x0059146c));
    g_unk0x0059146c[index] = 1;
    if (other != 0xff)
        g_unk0x0059146c[(char)other] = 1;
    i = 0;
    if (g_unk0x005913dc[index] != 0) {
        do {
            FUN_0048c900(g_unk0x005913f8[index][i]);
            i++;
        } while (i < g_unk0x005913dc[index]);
    }
}

// Moves one stage object and its eight box corners by the current frame delta
// and recurses over its children (each object is only moved once).
// match 70%: same logic; the corner pointer walk uses a different base bias
// match 70%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048c900
void FUN_0048c900(BYTE index)
{
    Car *pCar;
    BYTE i;
    int j;
    int *p;

    if (g_unk0x0059146c[index] != 0)
        return;
    g_unk0x0059146c[index] = 1;
    pCar = Car_Get(index);
    pCar->position.x += g_unk0x005914c8.x;
    pCar->position.y += g_unk0x005914c8.y;
    pCar->position.z += g_unk0x005914c8.z;
    p = (int *)&pCar->corners[0].y;
    j = 8;
    do {
        p[-1] += g_unk0x005914c8.x;
        p[0] += g_unk0x005914c8.y;
        p[1] += g_unk0x005914c8.z;
        p += 3;
    } while (--j);
    if (g_unk0x005913d8 != 0) {
        if (*(int *)(g_unk0x00590ed0[index] + 0x28) != 0) {
            p = (int *)(g_unk0x00590ed0[index] + 0x34);
            j = 4;
            do {
                p[-1] += g_unk0x005914c8.x;
                p[0] += g_unk0x005914c8.y;
                p[1] += g_unk0x005914c8.z;
                p += 3;
            } while (--j);
        }
    }
    i = 0;
    if (g_unk0x005913dc[index] != 0) {
        do {
            FUN_0048c900(g_unk0x005913f8[index][i]);
            i++;
        } while (i < g_unk0x005913dc[index]);
    }
}

// FUNCTION: CMR2 0x0048ca40
BYTE *FUN_0048ca40(int index)
{
    return g_unk0x00590ed0[index];
}

// FUNCTION: CMR2 0x0048ca90
int FUN_0048ca90(void)
{
    return g_unk0x005918c8;
}

// Index of the 0x6c-byte record whose position (+0x34) is nearest to pPos.
// FUNCTION: CMR2 0x0048d8b0
unsigned int FUN_0048d8b0(FixVector *pPos)
{
    unsigned int i;
    unsigned int best = 0;
    int bestDistance = 0x270f0000;
    int distance;
    FixVector d;

    for (i = 0; i < (unsigned int)g_unk0x005918c8; i++) {
        d.x = *(int *)(g_unk0x00591750 + i * 0x6c + 0x34) - pPos->x;
        d.y = *(int *)(g_unk0x00591750 + i * 0x6c + 0x38) - pPos->y;
        d.z = *(int *)(g_unk0x00591750 + i * 0x6c + 0x3c) - pPos->z;
        distance = FixVec_Length(&d);
        if (distance < bestDistance) {
            best = i;
            bestDistance = distance;
        }
    }
    return best;
}

// FUNCTION: CMR2 0x0048d930
int FUN_0048d930(BYTE *p)
{
    return g_unk0x00591740[*p];
}

extern int g_unk0x00591710[4];
void FUN_0048dce0(FixVector *pOut, BYTE *pSurface, Car *pCar, FixMatrix *pMatrix);

// Sliding impact of a car on a surface: decays the accumulated displacement by
// 0.9, adds the new weighted impact vector (0.1 * FUN_0048dce0), moves the
// object position by it, then refreshes the interpolated radius target.
// FUNCTION: CMR2 0x0048d950
void FUN_0048d950(BYTE *pSurface, FixMatrix *pMatrix)
{
    unsigned int index = *pSurface;
    FixVector *pPos = &g_unk0x005916a0[index];
    BYTE *pRecord = g_unk0x00591750 + g_unk0x00591740[index] * 0x6c;
    FixVector *pImpact = &g_unk0x00591868[index];
    FixVector impact;
    FixVector delta;
    int t;

    FUN_0048dce0(&impact, pSurface, Car_Get(pSurface[2]), pMatrix);
    FixVecScale(pImpact, pImpact, 0xe666);
    FixVecScale(&impact, &impact, 0x1999);
    pImpact->x += impact.x;
    pImpact->y += impact.y;
    pImpact->z += impact.z;
    FixMatrix_GetPosition(pPos, pMatrix);
    pPos->x += pImpact->x;
    pPos->y += pImpact->y;
    pPos->z += pImpact->z;
    delta.x = pPos->x - *(int *)(pRecord + 0x28);
    delta.y = pPos->y - *(int *)(pRecord + 0x2c);
    delta.z = pPos->z - *(int *)(pRecord + 0x30);
    t = FixDiv(FixVec_Length(&delta), *(int *)(pRecord + 0x40));
    g_unk0x00591710[index] = FixMul(t, *(int *)(pRecord + 0x44)) +
                             FixMul(0x10000 - t, *(int *)(pRecord + 0x54));
}

// FUNCTION: CMR2 0x00492890
void FUN_00492890(FixVector *pOut)
{
    int obj;

    FUN_0046f4c0(&obj);
    FixMatrix_RotateVector(pOut, &g_unk0x00592114, (FixMatrix *)(obj + 0x98));
}

// FUNCTION: CMR2 0x004928c0
void FUN_004928c0(int *pOut1, int *pOut2, int *pOut3)
{
    *pOut3 = g_unk0x00592128;
    *pOut1 = g_unk0x0059212c;
    *pOut2 = g_unk0x00592130;
}

// FUNCTION: CMR2 0x004928f0
void FUN_004928f0(int *pOut)
{
    *pOut = g_unk0x00592134;
}

// FUNCTION: CMR2 0x00492900
void FUN_00492900(int value)
{
    g_unk0x00592134 = value;
}

extern Mesh *g_stageMesh2Copy;
extern short g_stageMesh2Count;
// GLOBAL: CMR2 0x005920fc
int g_unk0x005920fc;
// GLOBAL: CMR2 0x00592100
int g_unk0x00592100;
// GLOBAL: CMR2 0x00592104
int g_unk0x00592104;

// Finds the vertex of stage mesh 2 closest to g_unk0x00592114 within a radius
// that shrinks to the best distance found so far, caches its stage-space
// position in 0x5920fc/0x592100/0x592104 and returns its index (-1 if none).
// match 53%: same logic; MSVC laid the locals out in different stack slots and
// kept the delta in different registers (the pointer pair to the delta local
// comes from the original's FixMul argument materialisation).
// FUNCTION: CMR2 0x00492910
int FUN_00492910(void)
{
    int obj[2];
    FixVector delta;
    FixVector vert;
    int dx;
    int dy;
    int dz;
    int dist;
    int limit;
    int best;
    int i;
    BYTE *pVertices;

    limit = 0x640000;
    best = -1;
    if (g_stageMesh2Count <= 0)
        return -1;
    FUN_0046f4e0(&obj[0], &obj[1]);
    FixMatrix_InverseRotateVector(&delta, &g_unk0x00592114, (FixMatrix *)(obj[1] + 0x98));
    for (i = 0; i < g_stageMesh2Count; i++) {
        pVertices = (BYTE *)g_stageMesh2Copy->pVertexData;
        vert.x = (int)(__int64)(*(float *)(pVertices + i * 0x30) * CGraphics::m_65536);
        vert.y = (int)(__int64)(*(float *)(pVertices + i * 0x30 + 4) * CGraphics::m_65536);
        vert.z = (int)(__int64)(*(float *)(pVertices + i * 0x30 + 8) * CGraphics::m_65536);
        dx = delta.x - vert.x;
        dy = delta.y - vert.y;
        dz = delta.z - vert.z;
        if (FIX_ABS(dx) <= limit && FIX_ABS(dy) <= limit && FIX_ABS(dz) <= limit) {
            dist = FixMul(dx, dx) + FixMul(dy, dy) + FixMul(dz, dz);
            if (dist <= 0x27100000) {
                limit = FixSqrt(dist);
                best = i;
            }
        }
    }
    if (best == -1)
        return -1;
    pVertices = (BYTE *)g_stageMesh2Copy->pVertexData;
    g_unk0x005920fc = (int)(__int64)(*(float *)(pVertices + best * 0x30) * CGraphics::m_65536);
    g_unk0x00592100 = (int)(__int64)(*(float *)(pVertices + best * 0x30 + 4) * CGraphics::m_65536);
    g_unk0x00592104 = (int)(__int64)(*(float *)(pVertices + best * 0x30 + 8) * CGraphics::m_65536);
    return best;
}

// FUNCTION: CMR2 0x00492bb0
void FUN_00492bb0(int *pOut)
{
    *pOut = *(int *)(*(BYTE **)(g_unk0x005920f0 + 0x24) + 0x34);
}

// FUNCTION: CMR2 0x00498570
BYTE *FUN_00498570(int index)
{
    return (BYTE *)g_unk0x00592734 + index * 0x2a4;
}

// Wraps a 16.16 angle in degrees into [-180, 180).
// FUNCTION: CMR2 0x00498db0
int FUN_00498db0(int angle)
{
    if (angle >= 0xb40000)
        angle -= 0x1680000;
    if (angle < -0xb40000)
        angle += 0x1680000;
    return angle;
}

// Marks a nearby car as travelling roughly towards the player car.
// match 81%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047cc50
void StageObject_UpdateApproachingCar(int carIndex, int time)
{
    g_unk0x0058e270[carIndex] = 1;
    Car *pPlayer = Car_Get(0);
    Car *pOther = Car_Get(carIndex);
    int otherZ = pOther->position.z;
    int otherX = pOther->position.x;
    int heading = StageObject_Atan2Degrees(pOther->right.z, pOther->right.x);
    int duration = RallyData_FUN_00421420();
    int difference = RallyData_FUN_00421370((BYTE *)pPlayer) - time;
    if (difference < -100) difference += duration;
    if (difference >= 0 && difference <= 7) {
        int direction = StageObject_Atan2Degrees(pPlayer->position.z - otherZ,
                                                 pPlayer->position.x - otherX);
        direction = FUN_00498db0(heading - direction);
        if (direction <= 0xa0000 && direction >= -0xa0000) return;
    }
    g_unk0x0058e270[carIndex] = 0;
}

// Buttons held on any connected device.
// FUNCTION: CMR2 0x0049e940
unsigned int FUN_0049e940(void)
{
    unsigned int buttons = 0;
    int i;

    for (i = 0; i < 8; i++) {
        if (CInput::m_availableDevices[i].field_0x0 != -1)
            buttons |= CInput::m_availableDevices[i].field_0x4;
    }
    return buttons;
}

// Keeps the subset of the six cars that are still inside the race-time,
// squared-distance and heading windows of car `index`, copies the per-car keep
// flags to pOut, accumulates their number into pCount and reports the overtake
// side in pFlag (1 / 0xff). Returns the first byte of the 0x10-byte race-record
// table row of `index`.
// FUNCTION: CMR2 0x0047d0e0
unsigned int FUN_0047d0e0(int index, BYTE *pOut, int *pCount, BYTE *pFlag)
{
    Car *cars[6];
    int active[6];
    int scratch[6];
    Car *pCar;
    int i;
    int duration;
    int z;
    int x;
    int referenceTime;
    int heading;

    for (i = 0; i < 6; i++)
        active[i] = 1;
    for (i = 0; i < 6; i++) {
        *pFlag = 0;
        cars[i] = Car_Get(i);
    }
    duration = RallyData_FUN_00421420();
    pCar = cars[index];
    active[index] = 0;
    z = pCar->position.z;
    x = pCar->position.x;
    *pCount = 0;
    referenceTime = RallyData_FUN_00421370((BYTE *)pCar);

    for (i = 0; i < (signed char)g_unk0x0058e0b0[1]; i++) {
        if (active[i] != 0) {
            int delta = RallyData_FUN_00421370((BYTE *)cars[i]);
            scratch[i] = delta;
            delta -= referenceTime;
            if (delta < 0 || delta > 5) {
                if (delta < -100)
                    delta += duration;
                if (delta <= 0 || delta > 5)
                    active[i] = 0;
            }
        }
    }

    for (i = 0; i < (signed char)g_unk0x0058e0b0[1]; i++) {
        if (active[i] != 0) {
            int dz = cars[i]->position.z - z;
            int dx = cars[i]->position.x - x;
            int dist = FixMul(dz, dz) + FixMul(dx, dx);
            scratch[i] = dist;
            if (dist > 0x90000)
                active[i] = 0;
        }
    }

    heading = StageObject_Atan2Degrees(pCar->right.z, pCar->right.x);
    for (i = 0; i < (signed char)g_unk0x0058e0b0[1]; i++) {
        if (active[i] != 0) {
            int d = StageObject_Atan2Degrees(cars[i]->right.z, cars[i]->right.x);
            d = FUN_00498db0(heading - d);
            if (d > 0x140000 || d < -0x140000)
                active[i] = 0;
        }
    }

    for (i = 0; i < (signed char)g_unk0x0058e0b0[1]; i++) {
        if (active[i] != 0) {
            int d = FUN_00498db0(StageObject_Atan2Degrees(cars[i]->position.z - z,
                                                          cars[i]->position.x - x) - heading);
            active[i] = 0;
            if (d > 0x500000 && d < 0x780000)
                *pFlag = 1;
            if (d < -0x500000 && d > -0x780000)
                *pFlag = 0xff;
        }
    }

    for (i = 0; i < (signed char)g_unk0x0058e0b0[1]; i++) {
        pOut[i] = (BYTE)active[i];
        *pCount += active[i];
    }
    return g_unk0x0058e4a4[referenceTime * 0x10];
}

void FUN_004bcad0(int value);
void Scene_SetLight(FixVector *pLight, int boost);
int Track_GetGroundHeightSurface(FixVector *pPoint, FixVector *pNormal, short *pTri, short *pSurfaceClass,
                                 unsigned short *pSurface, int defaultY);
DWORD FUN_004b74e0(void);
DWORD FUN_004b74f0(void);
DWORD FUN_004b7500(void);
int *FUN_00469680(int index);
void FUN_00480ac0(BYTE *pCar, int slot, int reset);
void FUN_0042b720(int index, char value);
int FUN_00457e10(BYTE *pCar, int offset);
struct KnockoutMatch;
int FUN_00472990(KnockoutMatch *pMatch);
short Car_GetOrderCount(void);

// FUNCTION: CMR2 0x00492fd0
void FUN_00492fd0(int value)
{
    FUN_004bcad0(value);
}

// Position of the object held in 0x589438 (its matrix at +0x98).
// FUNCTION: CMR2 0x0046f4a0
void FUN_0046f4a0(FixVector *pOut)
{
    if (g_unk0x00589438 != 0)
        FixMatrix_GetPosition(pOut, (FixMatrix *)(g_unk0x00589438 + 0x98));
}

// Ground height at a point; the surface id is written over the defaultY slot.
// FUNCTION: CMR2 0x004930b0
int Track_GetGroundHeight5(FixVector *pPoint, FixVector *pNormal, short *pTri, short *pSurfaceClass, int defaultY)
{
    return Track_GetGroundHeightSurface(pPoint, pNormal, pTri, pSurfaceClass, (unsigned short *)&defaultY, defaultY);
}

// Sets the stage light, without the boost for country 3.
// FUNCTION: CMR2 0x00492e30
void FUN_00492e30(FixVector *pLight)
{
    if ((char)RallyDataCountryIndex() == 3) {
        Scene_SetLight(pLight, 0);
        return;
    }
    Scene_SetLight(pLight, 1);
}

// Clears the value of every car slot not in use (or all of them when
// CGameInfo::FUN_00406320 is set).
// FUNCTION: CMR2 0x0047c1b0
void FUN_0047c1b0(void)
{
    int i;

    for (i = 0; i < 6; i++) {
        if (i >= (int)(RallyDataState() & 0xff) || CGameInfo::FUN_00406320() != 0)
            FUN_0042b720(i, 0xff);
    }
}

// FUNCTION: CMR2 0x00466490
void FUN_00466490(void)
{
    if (FUN_004b74e0() == 0 && FUN_004b74f0() == 0 && FUN_004b7500() == 0) {
        g_unk0x00588868 = 0;
        return;
    }
    g_unk0x00588868 = 1;
}

// GLOBAL: CMR2 0x0058896c
int g_unk0x0058896c;

unsigned int RallyData_FUN_00407e70(void);
BYTE FUN_00422fb0(BYTE index);
int FUN_0041f3a0(void);
void FUN_00494db0(Car *pCar, int view);
void FUN_00460330(int a, int b);
void FUN_00485690(short *pOrder, short count, int view);
void FUN_0047f740(void);

// Updates the "damaged / off-road" state of every car in the given order.
// match 86%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00466570
void FUN_00466570(short *param_1, short param_2, int param_3, int param_4)
{
    short *p;
    Car *pCar;
    int i;

    i = 0;
    if ((int)param_2 > 0) {
        p = param_1;
        do {
            pCar = Car_Get(*p);
            if (*(int *)((BYTE *)pCar + 0xc0c) == 0 &&
                *(int *)((BYTE *)pCar + 0xb68 + param_4 * 4) == 0) {
                if (FUN_0041f3a0() != 0) {
                    if ((char)RallyData_FUN_00407e90() || (char)RallyData_FUN_00407e70() ||
                        (unsigned int)i == (FUN_00422fb0(1) & 0xff))
                        FUN_00494db0(pCar, 1);
                } else if (FUN_0046bd20(*(char *)((BYTE *)pCar + 0xb1a), param_4) != 7) {
                    FUN_00494db0(pCar, param_4);
                }
            }
            i = i + 1;
            p = p + 1;
        } while (i < (int)param_2);
    }
    FUN_00485690(param_1, param_2, param_4);
    FUN_00460330(param_3, param_4);
    if (g_unk0x0058896c != 0)
        FUN_0047f740();
}

// Fixed-point to float conversion factors and the fade thresholds of the
// stage object lighting.
// GLOBAL: CMR2 0x00511378
extern const float g_unk0x00511378 = 5.0f;
// GLOBAL: CMR2 0x005113d0
extern const float g_unk0x005113d0 = 1.0f / 45.0f;
// GLOBAL: CMR2 0x005113dc
extern const float g_unk0x005113dc = 480.0f;
// GLOBAL: CMR2 0x005113e0
extern const float g_unk0x005113e0 = 640.0f;
// GLOBAL: CMR2 0x005113c8
extern const double g_zero0x005113c8 = 0.0;

extern const float g_netZero;
extern const float g_netByteScale;
extern const float g_netOne;

int FUN_00422f50(BYTE index);

// Fades a stage object in and out from the screen distance between two
// projected points of the player's car.
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00466100
void FUN_00466100(int param_1)
{
    Car *pCar;
    BYTE *pView;
    FixVector pos;
    FixVector dir;
    int screen[2];
    float f2;
    float f4;
    float f1;
    int alpha;

    pCar = Car_Get(1);
    pView = (BYTE *)g_viewNodes[param_1];
    FixMatrix_GetPosition(&pos, (FixMatrix *)((BYTE *)pCar->pNode0x71c + 0x98));
    pos.y = pos.y + 0x10000;
    FUN_004bad40(screen, &pos, pView);
    if (screen[0] != -0x640000 || screen[1] != -0x640000) {
        f2 = (float)(screen[0] * CGraphics::m_oneOver65536);
        f4 = (float)(screen[1] * CGraphics::m_oneOver65536);
        FixMatrix_GetUp(&dir, (FixMatrix *)(pView + 0x98));
        dir.x = dir.x + pos.x;
        dir.y = dir.y + pos.y;
        dir.z = dir.z + pos.z;
        FUN_004bad40(screen, &dir, pView);
        if (screen[0] != -0x640000 || screen[1] != -0x640000) {
            f1 = ((float)(screen[0] * CGraphics::m_oneOver65536) - f2) * g_unk0x005113e0 /
                 (float)*(int *)g_pGraphics;
            f2 = ((float)(screen[1] * CGraphics::m_oneOver65536) - f4) * g_unk0x005113dc /
                 (float)*(int *)((BYTE *)g_pGraphics + 4);
            f1 = (float)sqrt(f1 * f1 + f2 * f2);
            if (f1 <= g_unk0x00511378)
                return;
            f1 = g_netOne - (f1 - g_unk0x00511378) * g_unk0x005113d0;
            if (f1 < g_netOne) {
                if (g_netZero < f1)
                    alpha = (int)(__int64)(f1 * g_netByteScale);
                else
                    alpha = 0;
            } else {
                alpha = 0xff;
            }
            if (FUN_00422f50(param_1) == 7)
                alpha = 0x80;
            if (alpha == g_unk0x00588864)
                return;
            FUN_00465ec0(pCar->pNode0x71c, alpha, 0);
            FUN_00465f20(pCar->pNode0x71c->pFirstChild, alpha, 0);
            FUN_00465ec0(pCar->pNode0x720, alpha, 0);
            FUN_00465f20(pCar->pNode0x720->pFirstChild, alpha, 0);
            g_unk0x00588864 = alpha;
            return;
        }
    }
    if (g_unk0x00588864 == 0)
        return;
    FUN_00465ec0(pCar->pNode0x71c, 0, 0);
    FUN_00465f20(pCar->pNode0x71c->pFirstChild, 0, 0);
    FUN_00465ec0(pCar->pNode0x720, 0, 0);
    FUN_00465f20(pCar->pNode0x720->pFirstChild, 0, 0);
    g_unk0x00588864 = 0;
}

// Swaps *pValue with the value stored for `slot` when that slot is pending.
// FUNCTION: CMR2 0x004660a0
void FUN_004660a0(int **pValue, int slot, char flag)
{
    int *old;

    if (g_unk0x00588761 == slot) {
        old = g_unk0x00588758;
        g_unk0x00588758 = *pValue;
        *pValue = old;
        g_unk0x00588761 = -1;
        g_unk0x00588760 = flag;
    }
}

// FUNCTION: CMR2 0x0046b670
void FUN_0046b670(BYTE *pCar)
{
    int *p;
    int i;

    p = FUN_00469680((char)pCar[0xb1a]);
    i = 0;
    while (i < 4) {
        FUN_00480ac0(pCar, i, *(int *)((BYTE *)p + 0x4b0 + i * 4));
        i++;
    }
}

// Per car in race order: its split value (see FUN_00457e10).
// GLOBAL: CMR2 0x00590d90
int g_carSplitValues[8];

// FUNCTION: CMR2 0x00486700
void FUN_00486700(void)
{
    int *p;
    int i;

    i = 0;
    if (Car_GetOrderCount() > 0) {
        p = g_carSplitValues;
        do {
            *p = FUN_00457e10((BYTE *)Car_Get(i), 4);
            i++;
            p++;
        } while (i < Car_GetOrderCount());
    }
}

// GLOBAL: CMR2 0x0058cf70
int g_unk0x0058cf70;

// Clears the championship "pending" flag (bit 23) when set, or when 0x58cf70 is clear.
// FUNCTION: CMR2 0x004729f0
BYTE FUN_004729f0(void)
{
    unsigned int *pState;

    pState = RallyData_GetChampionshipState();
    if ((*pState & 0x800000) == 0 && g_unk0x0058cf70 != 0)
        return 0;
    *pState &= 0xff7fffff;
    g_unk0x0058cf7c = 0;
    CGame::FUN_004057e0(0);
    return 1;
}

// Eight records of 0x48 bytes: ten shorts at +0x1c (reset to -1) and two
// flag bytes at +0x44/+0x45.
// GLOBAL: CMR2 0x0058d6d0
BYTE g_unk0x0058d6d0[8][0x48];

// Fills the 12 outline values of a stage box (11 boundary levels plus the
// corner colour at +0x16) and repaints its two textures once the cached copy
// differs from the new values.
int FUN_00445dd0(int index);
void FUN_004775f0(Texture *pTexture, int state, int cacheBase, int index);
// FUNCTION: CMR2 0x00477460
void FUN_00477460(int index)
{
    unsigned short *pNew = g_unk0x0058d310 + index * 0xc;
    unsigned short *pOld = (unsigned short *)(g_stageBlock + index * 0x18);
    int changed = 0;
    int limit;
    int slot;
    int i;

    limit = FixMulShift32(FUN_00445dd0(index), 0xb0000);
    for (i = 0; i <= 0xa; i++)
        pNew[i] = ((i >= limit) - 1) & 0xff;
    if (g_unk0x0058d4c4[index * 2] != 0)
        FUN_004775f0((Texture *)g_unk0x0058d4c4[index * 2], (int)Car_Get(index)->field_0xb1e, 2, index);
    if (g_unk0x0058d4c0[index * 2] != 0) {
        slot = index + 8;
        pNew[0xb] = 0x6c;
        // the original leaves the scan by setting the counter to 0xc
        for (i = 0; i < 0xc; i++) {
            if (pNew[i] != pOld[i]) {
                changed = 1;
                i = 0xc;
            }
        }
        if (changed != 0) {
            CGraphics::BltTexture((Texture *)g_unk0x0058d4c0[index * 2], slot);
            CGraphics::RemapTextureAlpha((Texture *)g_unk0x0058d4c0[index * 2], 0xe0, 0xff, 0xd0, pNew[1], 0xc0,
                                         pNew[2], slot);
            CGraphics::RemapTextureAlpha((Texture *)g_unk0x0058d4c0[index * 2], 0xb0, pNew[3], 0xa0, pNew[4], 0x90,
                                         pNew[5], slot);
            CGraphics::RemapTextureAlpha((Texture *)g_unk0x0058d4c0[index * 2], 0x80, pNew[6], 0x70, pNew[7], 0x60,
                                         pNew[8], slot);
            CGraphics::RemapTextureAlpha((Texture *)g_unk0x0058d4c0[index * 2], 0x50, pNew[9], 0x40, pNew[10], 0x30,
                                         pNew[0xb], slot);
            for (i = 0; i < 0xc; i++)
                pOld[i] = pNew[i];
        }
    }
}

// Body colours of the two stage objects for the object's current body state:
// seven alpha values per object, compared against the ones already applied to
// the texture so the remap only runs when they change.
// FUNCTION: CMR2 0x004775f0
void FUN_004775f0(Texture *pTexture, int state, int cacheBase, int index)
{
    WORD *pColours = g_unk0x0058d2d4 + index * 7;
    WORD *pApplied = (WORD *)g_unk0x0058d6b0 + index * 7;
    int changed = 0;
    int cacheSlot = index + cacheBase * 8;
    int i;

    switch (state) {
    case 0:
        pColours[0] = 0;
        pColours[1] = 0xff;
        pColours[2] = 0xff;
        pColours[3] = 0xff;
        pColours[4] = 0xff;
        pColours[5] = 0xff;
        pColours[6] = 0xff;
        break;
    case 1:
        pColours[0] = 0;
        pColours[1] = 0;
        pColours[2] = 0xff;
        pColours[3] = 0xff;
        pColours[4] = 0;
        pColours[5] = 0;
        pColours[6] = 0;
        break;
    case 2:
        pColours[0] = 0xff;
        pColours[1] = 0xff;
        pColours[2] = 0xff;
        pColours[3] = 0;
        pColours[4] = 0xff;
        pColours[5] = 0xff;
        pColours[6] = 0;
        break;
    case 3:
        pColours[0] = 0xff;
        pColours[1] = 0xff;
        pColours[2] = 0xff;
        pColours[3] = 0xff;
        pColours[4] = 0xff;
        pColours[5] = 0;
        pColours[6] = 0;
        break;
    case 4:
        pColours[0] = 0xff;
        pColours[1] = 0;
        pColours[2] = 0xff;
        pColours[3] = 0xff;
        pColours[4] = 0;
        pColours[5] = 0;
        pColours[6] = 0xff;
        break;
    case 5:
        pColours[0] = 0xff;
        pColours[1] = 0xff;
        pColours[2] = 0;
        pColours[3] = 0xff;
        pColours[4] = 0xff;
        pColours[5] = 0;
        pColours[6] = 0xff;
        break;
    case 6:
        pColours[0] = 0xff;
        pColours[1] = 0xff;
        pColours[2] = 0;
        pColours[3] = 0xff;
        pColours[4] = 0xff;
        pColours[5] = 0xff;
        pColours[6] = 0xff;
        break;
    case 7:
        pColours[0] = 0xff;
        pColours[1] = 0;
        pColours[2] = 0;
        pColours[3] = 0;
        pColours[4] = 0;
        pColours[5] = 0xff;
        pColours[6] = 0;
        break;
    case 8:
        pColours[0] = 0xff;
        pColours[1] = 0xff;
        pColours[2] = 0xff;
        pColours[3] = 0xff;
        pColours[4] = 0xff;
        pColours[5] = 0xff;
        pColours[6] = 0xff;
        break;
    case 9:
        pColours[0] = 0xff;
        pColours[1] = 0xff;
        pColours[2] = 0xff;
        pColours[3] = 0xff;
        pColours[4] = 0;
        pColours[5] = 0;
        pColours[6] = 0xff;
        break;
    default:
        pColours[0] = 0xff;
        pColours[1] = 0;
        pColours[2] = 0;
        pColours[3] = 0;
        pColours[4] = 0;
        pColours[5] = 0;
        pColours[6] = 0;
        break;
    }
    // the original leaves the scan by setting the counter to 7 (a `break` generates different code)
    for (i = 0; i < 7; i++) {
        if (pColours[i] != pApplied[i]) {
            changed = 1;
            i = 7;
        }
    }
    if (changed != 0) {
        CGraphics::BltTexture(pTexture, cacheSlot);
        CGraphics::RemapTextureAlpha(pTexture, 0x80, pColours[0], 0xe0, pColours[1], 0xd0, pColours[2], cacheSlot);
        CGraphics::RemapTextureAlpha(pTexture, 0xc0, pColours[3], 0xb0, pColours[4], 0xa0, pColours[5], cacheSlot);
        CGraphics::RemapTextureAlpha(pTexture, 0x90, pColours[6], 0x90, pColours[6], 0x90, pColours[6], cacheSlot);
        for (i = 0; i < 7; i++)
            pApplied[i] = pColours[i];
    }
}

// Fades the two 5-slot colour ramps of a car's stage object towards its flag
// bytes, repaints both cached textures when anything changed and stores the frame.
extern int g_unk0x0051bd3c;
// FUNCTION: CMR2 0x00477ce0
void FUN_00477ce0(int car)
{
    BYTE *pRecord = g_unk0x0058d6d0[car];
    short values[5];
    int changed = 0;
    int value;
    int i;

    for (i = 0; i < 5; i++) {
        if ((pRecord[0x44] & (1 << i)) == 0) {
            value = *(short *)(pRecord + 0x30 + i * 2) - FixMul(0x50, g_unk0x0051bd3c);
            if (value < 0)
                value = 0;
        } else {
            value = *(short *)(pRecord + 0x30 + i * 2) + FixMul(0x80, g_unk0x0051bd3c);
            if (value > 0xff)
                value = 0xff;
        }
        *(short *)(pRecord + 0x30 + i * 2) = value;
        *(short *)(pRecord + 0x8 + i * 2) = value;
        if ((pRecord[0x45] & (1 << i)) == 0) {
            value = *(short *)(pRecord + 0x3a + i * 2) - FixMul(0x50, g_unk0x0051bd3c);
            if (value < 0)
                value = 0;
        } else {
            value = *(short *)(pRecord + 0x3a + i * 2) + FixMul(0x80, g_unk0x0051bd3c);
            if (value > 0xff)
                value = 0xff;
        }
        *(short *)(pRecord + 0x3a + i * 2) = (BYTE)value;
        *(short *)(pRecord + 0x12 + i * 2) = (BYTE)value;
    }
    if (pRecord[0x46] == 0) {
        *(short *)(pRecord + 0x8) = (short)((*(short *)(pRecord + 0x8) / 3) * 2);
        *(short *)(pRecord + 0x12) = (short)((*(short *)(pRecord + 0x12) / 3) * 2);
    } else {
        value = *(short *)(pRecord + 0xa) + (*(short *)(pRecord + 0x8) / 3) * 2;
        if (value > 0xff)
            value = 0xff;
        *(short *)(pRecord + 0xa) = (BYTE)value;
        value = *(short *)(pRecord + 0x14) + (*(short *)(pRecord + 0x12) / 3) * 2;
        if (value > 0xff)
            value = 0xff;
        *(short *)(pRecord + 0x14) = (BYTE)value;
    }
    for (i = 0; i < 5; i++) {
        if (*(short *)(pRecord + 0x12 + i * 2) != *(short *)(pRecord + 0x26 + i * 2) ||
            *(short *)(pRecord + 0x8 + i * 2) != *(short *)(pRecord + 0x1c + i * 2))
            changed = 1;
        if (*(short *)(pRecord + 0x8 + i * 2) > *(short *)(pRecord + 0x12 + i * 2))
            values[i] = *(short *)(pRecord + 0x8 + i * 2);
        else
            values[i] = *(short *)(pRecord + 0x12 + i * 2);
    }
    if (changed != 0) {
        Texture **ppTextures = (Texture **)pRecord;
        Texture *pTexture;

        for (i = 0; i < 2; i++) {
            pTexture = ppTextures[i];
            if (pTexture != NULL) {
                CGraphics::BltTexture(pTexture, car);
                CGraphics::RemapTextureAlpha(pTexture, 0xbf, values[1], 0x40, values[3], 0x80, values[2], car);
                CGraphics::RemapTextureAlpha(pTexture, 0x60, values[0], 0xe0, values[4], 0xe0, values[4], car);
            }
        }
        for (i = 0; i < 5; i++) {
            *(short *)(pRecord + 0x1c + i * 2) = *(short *)(pRecord + 0x8 + i * 2);
            *(short *)(pRecord + 0x26 + i * 2) = *(short *)(pRecord + 0x12 + i * 2);
        }
    }
}

// FUNCTION: CMR2 0x00477f30
void FUN_00477f30(void)
{
    int i;

    for (i = 0; i < 8; i++) {
        ((short *)g_unk0x0058d6d0[i])[0xe] = -1;
        ((short *)g_unk0x0058d6d0[i])[0xf] = -1;
        ((short *)g_unk0x0058d6d0[i])[0x10] = -1;
        ((short *)g_unk0x0058d6d0[i])[0x11] = -1;
        ((short *)g_unk0x0058d6d0[i])[0x12] = -1;
        ((short *)g_unk0x0058d6d0[i])[0x13] = -1;
        ((short *)g_unk0x0058d6d0[i])[0x14] = -1;
        ((short *)g_unk0x0058d6d0[i])[0x15] = -1;
        ((short *)g_unk0x0058d6d0[i])[0x16] = -1;
        ((short *)g_unk0x0058d6d0[i])[0x17] = -1;
    }
}

int FUN_00460c80(BYTE *pCar);
char RallyData_FUN_00408500(BYTE param1);

// GLOBAL: CMR2 0x005909b8
int g_unk0x005909b8;
// GLOBAL: CMR2 0x005909c0
BYTE g_unk0x005909c0[4];
// GLOBAL: CMR2 0x005909c4
BYTE g_unk0x005909c4[4];

// Per car: ticks until the next headlight glow may be spawned.
// GLOBAL: CMR2 0x0058e4a8
int g_unk0x0058e4a8[8];
// Glow record of the stage objects: pRec offsets are relative to this base,
// i.e. 0x18 bytes below the glow fields that 0x47d510 walks.
// GLOBAL: CMR2 0x0058e4c8
BYTE g_unk0x0058e4c8[100][0x5c];
// Same records as 0x47d5a0 walks, seen from their position field (+0xc): the
// pointer arithmetic of that view lives in 0x47e1e0.
// GLOBAL: CMR2 0x0058e4d4
BYTE g_unk0x0058e4d4[100][0x5c];

// Spawns the headlight glow of one stage object: finds the first free record,
// places it at the top corner of the car's bounding box, aims it along the
// body's right axis and drops it onto the ground below.
// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// The instruction sequence is the original's; the residual difference is the
// register numbering of the vector temporaries.
// FUNCTION: CMR2 0x0047d5a0
void FUN_0047d5a0(BYTE car)
{
    FixVector dir;
    FixVector half;
    Car *pCar;
    short surface;
    int *pRec;
    int i;

    if (g_unk0x0058e4a8[car] <= 0) {
        pRec = (int *)g_unk0x0058e4c8;
        i = 0;
        do {
            if (pRec[0x15] == 0) {
                pCar = Car_Get(car);
                FixVecScale(&dir, &pCar->up, *(int *)&pCar->field_0x770[4]);
                half.x = pCar->corners[1].x - pCar->corners[0].x;
                half.y = pCar->corners[1].y - pCar->corners[0].y;
                half.z = pCar->corners[1].z - pCar->corners[0].z;
                FixVecScale(&half, &half, 0x8000);
                pRec[3] = half.x + pCar->corners[0].x + dir.x;
                pRec[4] = half.y + pCar->corners[0].y + dir.y;
                pRec[5] = half.z + pCar->corners[0].z + dir.z;
                FixVecScale((FixVector *)pRec, &pCar->right, 0xcccc);
                pRec[0] += pCar->velocity.x;
                pRec[1] += pCar->velocity.y;
                pRec[2] += pCar->velocity.z;
                FixVecScale((FixVector *)pRec, &pCar->right, FixVecDot((FixVector *)pRec, &pCar->right));
                pRec[0x12] = 0x320000;
                pRec[9] = 0x10000;
                pRec[0x15] = 1;
                *(BYTE *)(pRec + 0x16) = car;
                pRec[0x11] = 0;
                *(short *)(pRec + 0x13) = -1;
                pRec[0x11] = Track_GetGroundHeightSurface((FixVector *)(pRec + 3), (FixVector *)(pRec + 6),
                                                          (short *)(pRec + 0x13), &surface,
                                                          (unsigned short *)&surface, 0);
                pRec[4] = pRec[0x11] + 0x8000;
                *(FixVector *)(pRec + 0xa) = *(FixVector *)(pRec + 3);
                *(FixVector *)(pRec + 0xd) = *(FixVector *)(pRec + 6);
                pRec[0x10] = pRec[9];
                i = 100;
                g_unk0x0058e4a8[car] = 0x100000;
            }
            pRec += 0x17;
            i++;
        } while (i < 100);
    }
}

void Glow_SetPosition(GlowLight *pLight, FixVector *pPos, FixVector *pDir);
void Glow_SetLayerPlane(GlowLight *pLight, FixVector *pPoint, FixVector *pNormal, int layerIntensity);

// Interpolates every headlight glow between its spawn record (the copy at
// +0x28/+0x34) and the current car state by the fraction t, normalises the
// direction and moves the light with it.
// match 59%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// The addresses are the original's (checked against the emitting disassembly);
// MSVC materialises the record pointer 4 bytes higher and compensates with -4
// displacements, so every memory operand reads differently.
// FUNCTION: CMR2 0x0047e1e0
void FUN_0047e1e0(int t)
{
    FixVector pos;
    FixVector normal;
    FixVector delta;
    FixVector ground;
    int *pRec;
    int length;
    int size;
    int i;

    pRec = (int *)g_unk0x0058e4d4;
    i = 100;
    do {
        if (pRec[0x12] == 0) {
            FUN_004ae3d0((BYTE *)pRec[0x11], 0);
        } else {
            delta.x = pRec[0] - pRec[7];
            delta.y = pRec[1] - pRec[8];
            delta.z = pRec[2] - pRec[9];
            FixVecScale(&delta, &delta, t);
            pos.x = delta.x + pRec[7];
            pos.y = delta.y + pRec[8];
            pos.z = delta.z + pRec[9];
            delta.x = pRec[3] - pRec[0xa];
            delta.y = pRec[4] - pRec[0xb];
            delta.z = pRec[5] - pRec[0xc];
            FixVecScale(&delta, &delta, t);
            normal.x = delta.x + pRec[0xa];
            normal.y = delta.y + pRec[0xb];
            normal.z = delta.z + pRec[0xc];
            length = FixVecLength(&normal);
            if (length == 0) {
                normal.x = 0;
                normal.y = 0;
                normal.z = 0;
            } else {
                FixVecScaleRecip(&normal, &normal, length);
            }
            size = FixMul(pRec[6] - pRec[0xd], t) + pRec[0xd];
            if (size <= 0) {
                FUN_004ae3d0((BYTE *)pRec[0x11], 0);
            } else {
                FUN_004ae3d0((BYTE *)pRec[0x11], 1);
                FUN_004ae3f0((BYTE *)pRec[0x11], size);
                Glow_SetPosition((GlowLight *)pRec[0x11], &pos, &pos);
                ground = pos;
                ground.y -= 0x8000;
                Glow_SetLayerPlane((GlowLight *)pRec[0x11], &ground, &normal, 0);
            }
        }
        pRec += 0x17;
    } while (--i);
}

// FUNCTION: CMR2 0x0047e490
void FUN_0047e490(BYTE *pColour)
{
    g_unk0x005909c0[0] = pColour[0];
    g_unk0x005909c0[1] = pColour[1];
    g_unk0x005909c0[2] = pColour[2];
    g_unk0x005909c4[0] = 0xff;
    g_unk0x005909c4[1] = 0xff;
    g_unk0x005909c4[2] = 0xff;
    g_unk0x005909b8 = 0;
}

// Fades the stage ambient colour towards the base colour while the 0x5909b8
// timer runs out, then applies it with one step of boost.
// match 72%: same logic; MSVC puts the length/scale locals in the other stack
// slots and swaps the addend order
// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00480220
void FUN_00480220(void)
{
    BYTE colour[4];
    int length;
    int scale;

    length = FixSqrt(g_unk0x005909b8);
    scale = (g_unk0x005909c4[0] & 0xff) << 16;
    scale = FixMulShift32(scale, length) + (g_unk0x005909c0[0] & 0xff);
    if (scale > 0xff)
        scale = 0xff;
    colour[0] = scale;
    scale = (g_unk0x005909c4[1] & 0xff) << 16;
    scale = FixMulShift32(scale, length) + (g_unk0x005909c0[1] & 0xff);
    if (scale > 0xff)
        scale = 0xff;
    colour[1] = scale;
    scale = (g_unk0x005909c4[2] & 0xff) << 16;
    scale = FixMulShift32(scale, length) + (g_unk0x005909c0[2] & 0xff);
    if (scale > 0xff)
        scale = 0xff;
    colour[2] = scale;
    g_unk0x005909b8 -= 0x8000;
    if (g_unk0x005909b8 < 0)
        g_unk0x005909b8 = 0;
    Scene_SetAmbient(colour, 1);
}

// Selects a list of 0x6c-byte records (count first); returns whether it is non-empty.
// FUNCTION: CMR2 0x0048caa0
int FUN_0048caa0(int *pList)
{
    unsigned int count = 0;

    if (pList != NULL) {
        g_unk0x005918c8 = *pList;
        count = g_unk0x005918c8;
        g_unk0x00591750 = (BYTE *)(pList + 1);
    } else {
        g_unk0x005918c8 = count;
        g_unk0x00591750 = NULL;
    }
    return count > 0;
}

// Whether a car's wheel sits on a surface of kind 0, 3, 12, 13 or 26 while FUN_00460c80 > 0.
// FUNCTION: CMR2 0x0046eeb0
BYTE FUN_0046eeb0(int index, int wheel)
{
    BYTE result;
    BYTE *pCar;

    result = 0;
    pCar = (BYTE *)Car_Get(index);
    switch (*(short *)(pCar + 0xaae + wheel * 2)) {
    case 0:
    case 3:
    case 0xc:
    case 0xd:
    case 0x1a:
        if (FUN_00460c80(pCar) > 0)
            result = 1;
    }
    return result;
}

// GLOBAL: CMR2 0x00591730
int g_unk0x00591730[4];

// Adds to a car's level (clamped to 1.0).
// FUNCTION: CMR2 0x0048dca0
void FUN_0048dca0(BYTE *pCar, int amount)
{
    BYTE car;

    if (amount > 0) {
        car = *pCar;
        if ((g_unk0x00591730[car] += amount) > 0x10000)
            g_unk0x00591730[car] = 0x10000;
    }
}

// Builds the impact displacement of a car on a surface record: the surface's
// right vector minus the car's (normalised) velocity direction, scaled by the
// record's factor and the car speed.
// FUNCTION: CMR2 0x0048dce0
void FUN_0048dce0(FixVector *pOut, BYTE *pSurface, Car *pCar, FixMatrix *pMatrix)
{
    FixVector direction;
    FixVector right;
    int length;
    int scale;

    direction = pCar->velocity;
    FixMatrix_GetRight(&right, pMatrix);
    if (pCar->speed < 0x28f) {
        direction.x = 0;
        direction.y = 0;
        direction.z = 0;
    } else {
        length = FixVecLength(&direction);
        if (length == 0) {
            direction.x = 0;
            direction.y = 0;
            direction.z = 0;
        } else {
            FixVecScaleRecip(&direction, &direction, length);
        }
        if (FixVecDot(&right, &direction) < 0) {
            direction.x = -direction.x;
            direction.y = -direction.y;
            direction.z = -direction.z;
        }
    }
    pOut->x = right.x - direction.x;
    pOut->y = right.y - direction.y;
    pOut->z = right.z - direction.z;
    scale = FixMul(*(int *)(g_unk0x00591750 + g_unk0x00591740[*pSurface] * 0x6c + 0x48), pCar->speed);
    FixVecScale(pOut, pOut, scale);
}

// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048df10
int FUN_0048df10(BYTE *pCar)
{
    short v;

    if (*(int *)(pCar + 4) == 7) {
        v = *(short *)(g_unk0x00591750 + g_unk0x00591740[*pCar] * 0x6c + 2);
        if (v < -0x3f4 && v > -0x40b)
            return 1;
    }
    return 0;
}

// FUNCTION: CMR2 0x00486b90
void FUN_00486b90(BYTE *pCar, BYTE *pInfo)
{
    int kind;
    int value;

    kind = *(int *)(pInfo + 4);
    if (kind != 2 && kind != 1 && kind != 10)
        value = 0;
    else
        value = 0x10000;
    g_unk0x00590db0[*pCar] = value;
}

int FUN_00472990(KnockoutMatch *pMatch);

// Returns 1 when a match of the current knockout round is still undecided.
// FUNCTION: CMR2 0x00473290
int FUN_00473290(void)
{
    unsigned int *pState;
    KnockoutMatch *pMatch = NULL;
    int count = 0;
    int i;

    pState = RallyData_GetChampionshipState();
    switch ((*pState >> 3) & 7) {
    case 1:
        count = 8;
        pMatch = (KnockoutMatch *)(pState + 0x16);
        break;
    case 2:
        count = 4;
        pMatch = (KnockoutMatch *)(pState + 10);
        break;
    case 3:
        count = 2;
        pMatch = (KnockoutMatch *)(pState + 4);
        break;
    case 4:
        count = 1;
        pMatch = (KnockoutMatch *)(pState + 1);
    }
    for (i = 0; i < count; i++, pMatch++) {
        if (FUN_00472990(pMatch))
            return 1;
    }
    return 0;
}

// Whether both drivers of the current round are known.
// FUNCTION: CMR2 0x00473310
int FUN_00473310(void)
{
    unsigned int first;
    unsigned int second;

    RallyData_GetChampionshipState();
    RallyData_GetRoundDrivers(&first, &second);
    if (RallyData_FUN_00408500((BYTE)first) != -1 && RallyData_FUN_00408500((BYTE)second) != -1)
        return 1;
    return 0;
}

// GLOBAL: CMR2 0x00591490
int g_unk0x00591490;
// GLOBAL: CMR2 0x00591494
int g_unk0x00591494;
// GLOBAL: CMR2 0x00591498
FixVector g_unk0x00591498;

// True when two spheres (radii r1, r2) overlap.
// match 51%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// True when two spheres (radii r1, r2) overlap. The 32-bit EAX result is
// tested by the callers (0x48a1f0), so the helper returns int, not bool.
// match 84%: identical instruction stream and stack layout except that the
// original kept the radius sum in a register with no home slot (its FixMul
// operands spill into the dead r1/r2 argument slots) while MSVC6 here homes
// it at [ebp-4], shifting the frame by four bytes.
// FUNCTION: CMR2 0x00487b80
int FUN_00487b80(int r1, int r2, int *pA, int *pB)
{
    int r = r1 + r2;
    FixVector delta;

    delta.x = pA[0] - pB[0];
    delta.y = pA[1] - pB[1];
    delta.z = pA[2] - pB[2];

    if ((delta.x < 0 ? -delta.x : delta.x) <= r && (delta.y < 0 ? -delta.y : delta.y) <= r &&
        (delta.z < 0 ? -delta.z : delta.z) <= r)
        return FixVecDot(&delta, &delta) < FixMul(r, r);
    return 0;
}

// FUNCTION: CMR2 0x00487e00
void FUN_00487e00(FixVector *pPos, int *pInfo)
{
    g_unk0x00591498 = *pPos;
    g_unk0x00591490 = *(int *)pInfo[1];
    g_unk0x00591494 = *(int *)(*(int *)(*(int *)(pInfo[0] + 0xc) + 0x10c) + 0x4c);
}

// Updates a car's stage shadow/light when its position, projected on the two
// box axes, is inside the light box (with the global tolerance).
// The original hoists both tolerance'd limits before testing the absolute
// values, which is what the two named locals reproduce.
// FUNCTION: CMR2 0x00487e50
void FUN_00487e50(int *pBox, Car *pCar)
{
    FixVector delta;
    int u;
    int v;

    delta.x = g_unk0x00591498.x - ((int *)pBox[0x25])[0];
    delta.y = g_unk0x00591498.y - ((int *)pBox[0x25])[1];
    delta.z = g_unk0x00591498.z - ((int *)pBox[0x25])[2];
    delta.y = 0;
    u = FixVecDot(&delta, (FixVector *)(pBox + 4));
    v = FixVecDot(&delta, (FixVector *)(pBox + 7));
    if (FIX_ABS(u) > pBox[0] && FIX_ABS(v) > pBox[1])
        return;
    {
        int limit0;
        int limit1;
        limit0 = pBox[0] + g_unk0x00591490;
        limit1 = pBox[1] + g_unk0x00591490;
        if (FIX_ABS(u) > limit0 || FIX_ABS(v) > limit1)
            return;
    }
    FUN_0048df50(pCar);
}

// GLOBAL: CMR2 0x0051fadc
BYTE g_unk0x0051fadc[4] = { 0, 1, 3, 2 };
// GLOBAL: CMR2 0x00590ec8
BYTE g_unk0x00590ec8[4];
// GLOBAL: CMR2 0x00590ecc
char g_unk0x00590ecc[4];
// GLOBAL: CMR2 0x005914a4
BYTE g_unk0x005914a4[4];
// GLOBAL: CMR2 0x005914c4
char g_unk0x005914c4[4];
// GLOBAL: CMR2 0x005914d4
char g_unk0x005914d4;
// GLOBAL: CMR2 0x005915f4
char g_unk0x005915f4;
// Box of the stage object being collided with (0x98 bytes: half sizes, axes,
// corners from +0x30, the car offset at +0x94).
// GLOBAL: CMR2 0x005915f8
int g_unk0x005915f8[0x26];

// Finds the closest overlap between two 4-corner boxes along the horizontal
// direction `pDir`, testing every pair of edges of their corner quads. Appends
// the winning contact (edge orientation 0=horizontal/2=vertical plus the corner
// index) to the A or B side list and returns 1; returns 0 if nothing overlaps.
// FUNCTION: CMR2 0x00488de0
int FUN_00488de0(FixVector *pVertsA, FixVector *pVertsB, FixVector *pDir, int *pDistance)
{
    BYTE *pNext;
    BYTE *pCur;
    BYTE corner;
    BYTE cornerVert;
    BYTE edge;
    int rayEdge;
    int best;
    int found;
    int dist;
    int diff;
    int i;
    int j;

    edge = 0;
    corner = 0;
    found = 0;
    best = -0x640000;
    for (i = 0; i < 4; i++) {
        pNext = &g_unk0x0051fadc[(i + 1) % 4];
        for (j = 1; j <= 4; j++) {
            pCur = &g_unk0x0051fadc[j % 4];
            g_collisionQuad[0] = pVertsA[g_unk0x0051fadc[i] + 4];
            g_collisionQuad[1] = pVertsA[*pNext + 4];
            g_collisionQuad[2] = pVertsB[g_unk0x0051fadc[j - 1] + 4];
            g_collisionQuad[3] = pVertsB[*pCur + 4];
            dist = Collision_RayQuad(pDir, &rayEdge, &corner);
            if (dist != 0x7d000000 && dist > best) {
                best = dist;
                switch (corner & 0xff) {
                case 0:
                    cornerVert = g_unk0x0051fadc[i];
                    found = 1;
                    diff = (int)g_unk0x0051fadc[j - 1] - (int)*pCur;
                    if (diff < 0)
                        diff = -diff;
                    edge = (diff == 1) ? 0 : 2;
                    break;
                case 1:
                    cornerVert = *pNext;
                    found = 1;
                    diff = (int)g_unk0x0051fadc[j - 1] - (int)*pCur;
                    if (diff < 0)
                        diff = -diff;
                    edge = (diff == 1) ? 0 : 2;
                    break;
                case 2:
                    cornerVert = g_unk0x0051fadc[j - 1];
                    found = 0;
                    diff = (int)g_unk0x0051fadc[i] - (int)*pNext;
                    if (diff < 0)
                        diff = -diff;
                    edge = (diff == 1) ? 0 : 2;
                    break;
                case 3:
                    cornerVert = *pCur;
                    found = 0;
                    diff = (int)g_unk0x0051fadc[i] - (int)*pNext;
                    if (diff < 0)
                        diff = -diff;
                    edge = (diff == 1) ? 0 : 2;
                    break;
                }
            }
        }
    }
    if (best < 0)
        return 0;
    if (found != 0) {
        int index = g_unk0x005915f4;
        g_unk0x00590ec8[index] = edge;
        g_unk0x005914c4[index] = cornerVert;
        g_unk0x005915f4++;
        *pDistance = best;
        return 1;
    }
    {
        int index = g_unk0x005914d4;
        g_unk0x005914a4[index] = edge;
        g_unk0x00590ecc[index] = cornerVert;
        g_unk0x005914d4++;
        *pDistance = best;
        return 1;
    }
}

extern int g_unk0x0051bd40;
extern int g_unk0x0051bd3c;

// Sets the scale 0x51bd40 (value / 25, at least 0.6) and its reciprocal 0x51bd3c.
// FUNCTION: CMR2 0x00466630
void FUN_00466630(int value)
{
    g_unk0x0051bd40 = FixMul(0xa3d, value);
    if (g_unk0x0051bd40 < 0x9999)
        g_unk0x0051bd40 = 0x9999;
    g_unk0x0051bd3c = FixDiv(0x10000, g_unk0x0051bd40);
}

extern FixVector g_stageDeformHull[12];
extern FixVector g_stageDeformOffset;
extern FixVector g_stageDeformNormal;
extern FixVector g_stageDeformImpact;
extern int g_stageDeformSpeed;
extern int g_stageDeformStrength;
extern int g_stageDeformMode;

// Rebuilds the deformation hull against the other car's velocity: the four
// source vertices come from its velocity/velocityNext, the four middle ones
// from the car's own bounds, and the last four are the input points clamped
// by the car scales.
// match 48%: MSVC biased the hull pointer walk differently (same logic)
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004675c0
void FUN_004675c0(Car *pCar, Car *pOther)
{
    FixVector *pOut;
    FixVector *pIn;
    int v;

    g_stageDeformHull[0].x = pOther->velocity.z;
    g_stageDeformHull[0].y = -pCar->halfExtents.y;
    g_stageDeformHull[0].z = pOther->velocityNext.y;
    g_stageDeformHull[1].x = pOther->velocity.z;
    g_stageDeformHull[1].y = -pCar->halfExtents.y;
    g_stageDeformHull[1].z = pOther->velocityNext.z;
    g_stageDeformHull[2].x = pOther->velocityNext.x;
    g_stageDeformHull[2].y = -pCar->halfExtents.y;
    g_stageDeformHull[2].z = pOther->velocityNext.y;
    g_stageDeformHull[3].x = pOther->velocityNext.x;
    g_stageDeformHull[3].y = -pCar->halfExtents.y;
    g_stageDeformHull[3].z = pOther->velocityNext.z;
    pOut = &g_stageDeformHull[8];
    pIn = (FixVector *)pCar->field_0x240;
    do {
        pOut[-4] = pOut[-8];
        if ((int)pOut < (int)&g_stageDeformHull[10])
            v = *(int *)pCar->field_0x770;
        else
            v = *(int *)(pCar->field_0x770 + 4);
        pOut[-4].y += v;
        *pOut = *pIn;
        if (pOut->x > 0)
            pOut->x -= pCar->scale0x764;
        else
            pOut->x += pCar->scale0x768;
        if (pOut->z > 0)
            pOut->z -= pCar->scale0x76c;
        else
            pOut->z += pCar->scale0x76c;
        pOut++;
        pIn++;
    } while ((int)pOut < (int)&g_stageDeformHull[12]);
}

// Builds the deformation offset, normal and impact vectors plus the strength
// and mode flags from a per-car byte record (angles, break flags, scale).
// match 89%: the original folds the 0x10000<<16 division into a plain IDIV in
// the two later scale blocks, and stores the mode as a byte (declared int in
// StageTiming.cpp)
// FUNCTION: CMR2 0x004688b0
void FUN_004688b0(BYTE *p)
{
    g_stageDeformOffset.x = (char)p[9] << 16;
    g_stageDeformOffset.y = (char)p[10] << 16;
    g_stageDeformOffset.z = (char)p[11] << 16;
    FixVecScale(&g_stageDeformOffset, &g_stageDeformOffset,
                FixMul(0xa0000, FixDiv(0x10000, 0x7f0000)));
    g_stageDeformNormal.x = (char)p[3] << 16;
    g_stageDeformNormal.y = (char)p[4] << 16;
    g_stageDeformNormal.z = (char)p[5] << 16;
    FixVecScaleRecip(&g_stageDeformNormal, &g_stageDeformNormal, 0x7f0000);
    g_stageDeformImpact.x = (char)p[6] << 16;
    g_stageDeformImpact.y = (char)p[7] << 16;
    g_stageDeformImpact.z = (char)p[8] << 16;
    FixVecScaleRecip(&g_stageDeformImpact, &g_stageDeformImpact, 0x7f0000);
    g_stageDeformStrength = (BYTE)p[0] << 16;
    g_stageDeformStrength = FixDiv(g_stageDeformStrength, 0xff0000);
    g_stageDeformMode = p[1];
    if (p[1] == 1) {
        g_stageDeformSpeed = (BYTE)p[2] << 16;
        g_stageDeformSpeed = FixMul(g_stageDeformSpeed, FixMul(0xa0000, FixDiv(0x10000, 0xff0000)));
    }
}

extern BYTE *g_unk0x00588b94;

int FUN_00469100(Car *pCar, BYTE *pRecord);

// Rebuilds the per-wheel/gear block (0x240..0x2c4) of the car's 0x4d0-byte
// record from its source block (0x21c..), scales it, then recomputes the
// derived torques and scales (0x3d8..0x408).
// match 88%: same code; MSVC kept the cached record fields in EDX/EAX in the
// original and in EDI here (register numbering).
// FUNCTION: CMR2 0x00468c10
void FUN_00468c10(Car *pCar)
{
    BYTE *pRecord;
    int *p;
    int i;
    int a;
    int b;
    int value;

    pRecord = g_unk0x00588b94 + pCar->field_0xb1a * 0x4d0;
    if (*(int *)pCar->field_0xb50 == 0)
        return;
    p = (int *)(pRecord + 0x240);
    p[0] = *(int *)(pRecord + 0x234);
    *(int *)(pRecord + 0x244) = *(int *)(pRecord + 0x21c);
    *(int *)(pRecord + 0x248) = *(int *)(pRecord + 0x23c);
    *(int *)(pRecord + 0x24c) = *(int *)(pRecord + 0x224);
    *(int *)(pRecord + 0x254) = *(int *)(pRecord + 0x21c);
    *(int *)(pRecord + 0x250) = *(int *)(pRecord + 0x234);
    *(int *)(pRecord + 0x258) = *(int *)(pRecord + 0x234);
    *(int *)(pRecord + 0x25c) = *(int *)(pRecord + 0x21c);
    *(int *)(pRecord + 0x260) = *(int *)(pRecord + 0x23c);
    *(int *)(pRecord + 0x264) = *(int *)(pRecord + 0x224);
    *(int *)(pRecord + 0x268) = *(int *)(pRecord + 0x234);
    *(int *)(pRecord + 0x26c) = *(int *)(pRecord + 0x21c);
    *(int *)(pRecord + 0x270) = *(int *)(pRecord + 0x23c);
    *(int *)(pRecord + 0x274) = *(int *)(pRecord + 0x224);
    *(int *)(pRecord + 0x278) = *(int *)(pRecord + 0x228);
    *(int *)(pRecord + 0x27c) = *(int *)(pRecord + 0x228);
    *(int *)(pRecord + 0x280) = FixMul(*(int *)(pRecord + 0x22c) + *(int *)(pRecord + 0x228) +
                                       *(int *)(pRecord + 0x230), 0x5553);
    *(int *)(pRecord + 0x28c) = *(int *)(pRecord + 0x228);
    *(int *)(pRecord + 0x288) = *(int *)(pRecord + 0x230);
    *(int *)(pRecord + 0x290) = *(int *)(pRecord + 0x230);
    *(int *)(pRecord + 0x294) = *(int *)(pRecord + 0x230);
    *(int *)(pRecord + 0x2a0) = *(int *)(pRecord + 0x230);
    *(int *)(pRecord + 0x298) = *(int *)(pRecord + 0x228);
    *(int *)(pRecord + 0x29c) = *(int *)(pRecord + 0x228);
    *(int *)(pRecord + 0x284) = 0;
    *(int *)(pRecord + 0x2a4) = FixMul(*(int *)(pRecord + 0x230) + *(int *)(pRecord + 0x228) +
                                       *(int *)(pRecord + 0x22c), 0x5553);
    *(int *)(pRecord + 0x2a8) = *(int *)(pRecord + 0x228);
    *(int *)(pRecord + 0x2ac) = *(int *)(pRecord + 0x230);
    *(int *)(pRecord + 0x2b0) = *(int *)(pRecord + 0x238);
    *(int *)(pRecord + 0x2b4) = *(int *)(pRecord + 0x220);
    *(int *)(pRecord + 0x2b8) = *(int *)(pRecord + 0x23c);
    *(int *)(pRecord + 0x2bc) = *(int *)(pRecord + 0x224);
    *(int *)(pRecord + 0x2c0) = *(int *)(pRecord + 0x234);
    *(int *)(pRecord + 0x2c4) = *(int *)(pRecord + 0x21c);
    if (*(int *)pCar->field_0xb7c == 0)
        *(int *)(pRecord + 0x27c) = 0;
    if (*(int *)(pCar->field_0xb7c + 4) == 0)
        *(int *)(pRecord + 0x280) = 0;
    for (i = 0; i < 0x22; i++) {
        p[i] = FixMul(p[i], p[i + 0x22]);
        p[i] = p[i] + p[i + 0x44];
        if (p[i] > 0x10000)
            p[i] = 0x10000;
    }
    *(int *)(pRecord + 0x404) = 0x10000 - FixMul(*(int *)(pRecord + 0x2a0), 0x666) -
                                FixMul(*(int *)(pRecord + 0x27c), 0x1333) -
                                FixMul(*(int *)(pRecord + 0x2a4), 0x2666);
    *(int *)(pRecord + 0x3dc) = FixMul(*(int *)(pRecord + 0x258), FixMul(0x3333, 0xffff0000));
    *(int *)(pRecord + 0x3e0) = FixMul(*(int *)(pRecord + 0x25c), FixMul(0x3333, 0xffff0000));
    *(int *)(pRecord + 0x3e4) = FixMul(*(int *)(pRecord + 0x260), FixMul(0x3333, 0x8000));
    *(int *)(pRecord + 0x3e8) = FixMul(*(int *)(pRecord + 0x264), FixMul(0x3333, 0xffff8000));
    *(int *)(pRecord + 0x3d8) = FixMul(*(int *)(pRecord + 0x250) * 2, 0x8000);
    *(int *)(pRecord + 0x3d8) = FixMul(*(int *)(pRecord + 0x3d8), 0xa0000);
    *(int *)(pRecord + 0x3fc) = 0x10000 - FixMul(FixMul(*(int *)(pRecord + 0x268) +
                                                         *(int *)(pRecord + 0x26c), 0x8000), 0x3333);
    *(int *)(pRecord + 0x400) = 0x10000 - FixMul(FixMul(*(int *)(pRecord + 0x270) +
                                                         *(int *)(pRecord + 0x274), 0x8000), 0x3333);
    p = (int *)(pRecord + 0x3ec);
    for (i = 0; i < 4; i++) {
        p[i] = FixMul(p[i - 0x6b], 0xccc);
    }
    if (*(int *)pCar->field_0x7b8 != 0x10000 && *(int *)pCar->field_0x7b8 != 0) {
        value = FixMul(*(int *)(pRecord + 0x280), 0x8000) + *(int *)pCar->field_0x7b8;
        pCar->field_0x7b4 = value;
        if (value > 0x10000)
            pCar->field_0x7b4 = 0x10000;
    }
    *(char *)(pRecord + 0x468) = (char)FixMulShift32(*(int *)(pRecord + 0x278), 0xf0000);
    value = FUN_00469100(pCar, pRecord);
    *(int *)(pRecord + 0x408) = FixMul(value, 0x4000);
    if (FUN_00469bc0(pCar, 3) != 0)
        *(int *)(pRecord + 0x408) = *(int *)(pRecord + 0x408) + -0x3333;
    *(int *)(pRecord + 0x284) = value;
    if (value > 0x10000)
        *(int *)(pRecord + 0x284) = 0x10000;
}

// Adds `amount` to the 3x3 grid at +0x21c of the car's 0x4d0-byte record,
// weighted by how close each grid point is to the car's contact offsets
// (0x5dc/0x5e4).
// match 50%: same logic; MSVC6 kept the car pointer in EDI and the FixMul
// temporaries in the parameter slots instead of the slots we get.
// FUNCTION: CMR2 0x00468a80
void FUN_00468a80(Car *pCar, int amount)
{
    int *pGrid;
    int *pElem;
    int row;
    int i;
    int xOff;
    int yOff;
    int halfAmount;
    int gridStep;
    int xStep;
    int yStep;
    int xLimit;
    int yLimit0;
    int yLimit1;
    int colLimit;

    pGrid = (int *)(g_unk0x00588b94 + pCar->field_0xb1a * 0x4d0 + 0x21c);
    halfAmount = FixMul(amount, 0x8000);
    gridStep = FixMul(amount, 0x3333);
    xStep = FixMul(pCar->halfExtents.x, 0xaac0);
    yStep = FixMul(pCar->halfExtents.z, 0xc000);
    xLimit = FixMul(pCar->halfExtents.x, 0x553f) + halfAmount;
    yLimit0 = FixMul(pCar->halfExtents.z, 0x4000) + halfAmount;
    yLimit1 = FixMul(pCar->halfExtents.z, 0x8000) + halfAmount;
    row = 0;
    yOff = -yStep;
    for (row = 0; row < 3; row++) {
        colLimit = row == 1 ? yLimit1 : yLimit0;
        xOff = xStep;
        pElem = pGrid;
        for (i = 0; i < 3; i++) {
            if (FIX_ABS(*(int *)((BYTE *)pCar + 0x5dc) - xOff) <= xLimit &&
                FIX_ABS(*(int *)((BYTE *)pCar + 0x5e4) - yOff) <= colLimit) {
                *pElem += gridStep;
                if (*pElem > 0x640000)
                    *pElem = 0x640000;
            }
            xOff -= xStep;
            pElem++;
        }
        pGrid += 3;
        yOff += yStep;
    }
}
// Averages (16.16) the per-object distance between every stage object's float
// vertex data and its fixed-point copy, skipping parts 1 and 3 when the record
// flag is set (their object count still feeds the divisor).
// FUNCTION: CMR2 0x00469100
int FUN_00469100(Car *pCar, BYTE *pRecord)
{
    int partA;
    int partB;
    int total;
    int sum;
    int entry;
    int *pCounts;
    int i;
    int offsetDst;
    int offsetSrc;

    if (g_unk0x00588970[pCar->field_0xb1a] == 0)
        return 0;
    partA = FUN_00484d10(1, pCar->field_0xb1a);
    partB = FUN_00484d10(3, pCar->field_0xb1a);
    total = 0;
    sum = 0;
    entry = 0;
    pCounts = (int *)(pRecord + 0x420);
    if (*(int *)(pRecord + 0x45c) > 0) {
        do {
            if (entry == partA) {
                if (FUN_00469bc0(pCar, 1) != 0) {
                    total += *pCounts;
                    goto next;
                }
            } else if (entry == partB) {
                if (FUN_00469bc0(pCar, 3) != 0) {
                    total += *pCounts;
                    goto next;
                }
            }
            i = 0;
            if (*pCounts > 0) {
                offsetDst = 0;
                offsetSrc = 0;
                do {
                    int *pDst = (int *)(*(int *)(pRecord + 0x78 + entry * 4) + offsetDst);
                    float *pSrc = (float *)(*(int *)(*(int *)(pRecord + entry * 4) + 0xc) + offsetSrc);
                    int dx = (int)(__int64)((double)pSrc[0] * CGraphics::m_65536) - pDst[0];
                    int dy = (int)(__int64)((double)pSrc[1] * CGraphics::m_65536) - pDst[1];
                    int dz = (int)(__int64)((double)pSrc[2] * CGraphics::m_65536) - pDst[2];
                    int v;

                    if (dx < 0)
                        dx = -dx;
                    if (dy < 0)
                        dy = -dy;
                    if (dz < 0)
                        dz = -dz;
                    v = FixMul(dz + dy + dx, 0x50000);
                    if (v > 0x10000)
                        v = 0x10000;
                    offsetSrc += 0x30;
                    sum += v;
                    total++;
                    i++;
                    offsetDst += 0x20;
                } while (i < *pCounts);
            }
next:
            entry++;
            pCounts++;
        } while (entry < *(int *)(pRecord + 0x45c));
    }
    return FixDiv(sum, total << 16);
}

// Sets the fixed-point lighting values for both stage weather conditions.
// match 24%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00460da0
void StageObject_SetLighting(const BYTE *pPrimary, const BYTE *pSecondary)
{
    Stage_InitLightMeshes();
    *(WORD *)&g_stageLighting[0x5c] = 0xffff;
    int *pWeatherPair = FUN_00407520(RallyDataStageIndex());
    g_stageLighting[0x2c] = g_stageWeatherIntensity[pWeatherPair[0]];
    g_stageLighting[0x59] = g_stageWeatherIntensity[pWeatherPair[1]];
    if (pPrimary != NULL && pSecondary != NULL) {
        g_stageLighting[0x1b] = *(int *)pPrimary;
        g_stageLighting[0x1c] = *(int *)(pPrimary + 0x4);
        g_stageLighting[0x1d] = *(int *)(pPrimary + 0x8);
        g_stageLighting[0x48] = *(int *)pSecondary;
        g_stageLighting[0x49] = *(int *)(pSecondary + 0x4);
        g_stageLighting[0x4a] = *(int *)(pSecondary + 0x8);
        FUN_004925c0(g_stageLighting[0x1b], g_stageLighting[0x1c], g_stageLighting[0x1d]);
        FUN_00492900(0x10000);
        g_stageLighting[0x0] = (unsigned int)pPrimary[0x20] * 0x10000;
        g_stageLighting[0x1] = (unsigned int)pPrimary[0x21] * 0x10000;
        g_stageLighting[0x2] = (unsigned int)pPrimary[0x22] * 0x10000;
        g_stageLighting[0x3] = (unsigned int)pPrimary[0x1c] * 0x10000 + (unsigned int)pPrimary[0x20] * -0x10000;
        g_stageLighting[0x4] = (unsigned int)pPrimary[0x1d] * 0x10000 + (unsigned int)pPrimary[0x21] * -0x10000;
        g_stageLighting[0x5] = (unsigned int)pPrimary[0x1e] * 0x10000 + (unsigned int)pPrimary[0x22] * -0x10000;
        g_stageLighting[0x2d] = (unsigned int)pSecondary[0x20] * 0x10000;
        g_stageLighting[0x2e] = (unsigned int)pSecondary[0x21] * 0x10000;
        g_stageLighting[0x2f] = (unsigned int)pSecondary[0x22] * 0x10000;
        g_stageLighting[0x30] = (unsigned int)pSecondary[0x1c] * 0x10000 + (unsigned int)pSecondary[0x20] * -0x10000;
        g_stageLighting[0x31] = (unsigned int)pSecondary[0x1d] * 0x10000 + (unsigned int)pSecondary[0x21] * -0x10000;
        g_stageLighting[0x32] = (unsigned int)pSecondary[0x1e] * 0x10000 + (unsigned int)pSecondary[0x22] * -0x10000;
        g_stageLighting[0x6] = (unsigned int)pPrimary[0x24] << 0x10;
        g_stageLighting[0x7] = (unsigned int)pPrimary[0x25] << 0x10;
        g_stageLighting[0x8] = (unsigned int)pPrimary[0x26] << 0x10;
        g_stageLighting[0x33] = (unsigned int)pSecondary[0x24] << 0x10;
        g_stageLighting[0x34] = (unsigned int)pSecondary[0x25] << 0x10;
        g_stageLighting[0x35] = (unsigned int)pSecondary[0x26] << 0x10;
        g_stageLighting[0x27] = FixDiv((int)pPrimary[0x27] << 16, 0xff0000);
        g_stageLighting[0x54] = FixDiv((int)pSecondary[0x27] << 16, 0xff0000);
        g_stageLighting[0x9] = (unsigned int)pPrimary[0x3c] << 0x10;
        g_stageLighting[0xa] = (unsigned int)pPrimary[0x3d] << 0x10;
        g_stageLighting[0xb] = (unsigned int)pPrimary[0x3e] << 0x10;
        g_stageLighting[0x36] = (unsigned int)pSecondary[0x3c] << 0x10;
        g_stageLighting[0x37] = (unsigned int)pSecondary[0x3d] << 0x10;
        g_stageLighting[0x38] = (unsigned int)pSecondary[0x3e] << 0x10;
        g_stageLighting[0x2b] = *(int *)(pPrimary + 0x10);
        g_stageLighting[0x58] = *(int *)(pSecondary + 0x10);
        g_stageLighting[0x18] = (unsigned int)pPrimary[0x34] << 0x10;
        g_stageLighting[0x19] = (unsigned int)pPrimary[0x35] << 0x10;
        g_stageLighting[0x1a] = (unsigned int)pPrimary[0x36] << 0x10;
        g_stageLighting[0x45] = (unsigned int)pSecondary[0x34] << 0x10;
        g_stageLighting[0x46] = (unsigned int)pSecondary[0x35] << 0x10;
        g_stageLighting[0x47] = (unsigned int)pSecondary[0x36] << 0x10;
        g_stageLighting[0x2a] = (unsigned int)pPrimary[0x37] << 0x10;
        g_stageLighting[0x57] = (unsigned int)pSecondary[0x37] << 0x10;
        g_stageLighting[0x15] = (unsigned int)pPrimary[0x38] << 0x10;
        g_stageLighting[0x16] = (unsigned int)pPrimary[0x39] << 0x10;
        g_stageLighting[0x17] = (unsigned int)pPrimary[0x3a] << 0x10;
        g_stageLighting[0x42] = (unsigned int)pSecondary[0x38] << 0x10;
        g_stageLighting[0x43] = (unsigned int)pSecondary[0x39] << 0x10;
        g_stageLighting[0x44] = (unsigned int)pSecondary[0x3a] << 0x10;
        g_stageLighting[0xc] = (unsigned int)pPrimary[0x28] << 0x10;
        g_stageLighting[0xd] = (unsigned int)pPrimary[0x29] << 0x10;
        g_stageLighting[0xe] = (unsigned int)pPrimary[0x2a] << 0x10;
        g_stageLighting[0x39] = (unsigned int)pSecondary[0x28] << 0x10;
        g_stageLighting[0x3a] = (unsigned int)pSecondary[0x29] << 0x10;
        g_stageLighting[0x3b] = (unsigned int)pSecondary[0x2a] << 0x10;
        g_stageLighting[0x28] = FixDiv((int)pPrimary[0x2b] << 16, 0xff0000);
        g_stageLighting[0x55] = FixDiv((int)pSecondary[0x2b] << 16, 0xff0000);
        g_stageLighting[0xf] = (unsigned int)pPrimary[0x2c] << 0x10;
        g_stageLighting[0x10] = (unsigned int)pPrimary[0x2d] << 0x10;
        g_stageLighting[0x11] = (unsigned int)pPrimary[0x2e] << 0x10;
        g_stageLighting[0x3c] = (unsigned int)pSecondary[0x2c] << 0x10;
        g_stageLighting[0x3d] = (unsigned int)pSecondary[0x2d] << 0x10;
        g_stageLighting[0x3e] = (unsigned int)pSecondary[0x2e] << 0x10;
        g_stageLighting[0x29] = (int)((unsigned int)pPrimary[0x2f] << 0x10);
        g_stageLighting[0x56] = (int)((unsigned int)pSecondary[0x2f] << 0x10);
        g_stageLighting[0x12] = (unsigned int)pPrimary[0x30] << 0x10;
        g_stageLighting[0x13] = (unsigned int)pPrimary[0x31] << 0x10;
        g_stageLighting[0x14] = (unsigned int)pPrimary[0x32] << 0x10;
        g_stageLighting[0x3f] = (unsigned int)pSecondary[0x30] << 0x10;
        g_stageLighting[0x40] = (unsigned int)pSecondary[0x31] << 0x10;
        g_stageLighting[0x41] = (unsigned int)pSecondary[0x32] << 0x10;
        g_stageLighting[0x1e] = (unsigned int)pPrimary[0x44] << 0x10;
        g_stageLighting[0x1f] = (unsigned int)pPrimary[0x45] << 0x10;
        g_stageLighting[0x20] = (unsigned int)pPrimary[0x46] << 0x10;
        g_stageLighting[0x25] = *(int *)(pPrimary + 0x14);
        g_stageLighting[0x26] = *(int *)(pPrimary + 0x18);
        g_stageLighting[0x4b] = (unsigned int)pSecondary[0x44] << 0x10;
        g_stageLighting[0x4c] = (unsigned int)pSecondary[0x45] << 0x10;
        g_stageLighting[0x4d] = (unsigned int)pSecondary[0x46] << 0x10;
        g_stageLighting[0x52] = *(int *)(pSecondary + 0x14);
        g_stageLighting[0x53] = *(int *)(pSecondary + 0x18);
        g_stageLighting[0x21] = (unsigned int)pPrimary[0x40] << 0x10;
        g_stageLighting[0x22] = (unsigned int)pPrimary[0x41] << 0x10;
        g_stageLighting[0x23] = (unsigned int)pPrimary[0x42] << 0x10;
        g_stageLighting[0x24] = (unsigned int)pPrimary[0x43] << 0x10;
        g_stageLighting[0x4e] = (unsigned int)pSecondary[0x40] << 0x10;
        g_stageLighting[0x4f] = (unsigned int)pSecondary[0x41] << 0x10;
        g_stageLighting[0x50] = (unsigned int)pSecondary[0x42] << 0x10;
        g_stageLighting[0x51] = (unsigned int)pSecondary[0x43] << 0x10;
    } else {
        g_stageLighting[0x1b] = 0xfffb0000;
        g_stageLighting[0x1c] = 0xffda0000;
        g_stageLighting[0x1d] = 0xfff30000;
        g_stageLighting[0x48] = 0xfffb0000;
        g_stageLighting[0x49] = 0xffda0000;
        g_stageLighting[0x4a] = 0xfff30000;
        FUN_004925c0(0xfffb0000, 0xffda0000, 0xfff30000);
        FUN_00492900(0x10000);
        g_stageLighting[0x3] = -0x5d0000;
        g_stageLighting[0x6] = 0xff0000;
        g_stageLighting[0x30] = -0x5d0000;
        g_stageLighting[0x4] = -0x620000;
        g_stageLighting[0x5] = 0;
        g_stageLighting[0x7] = 0xff0000;
        g_stageLighting[0x8] = 0xff0000;
        g_stageLighting[0x1e] = 0;
        g_stageLighting[0x1f] = 0;
        g_stageLighting[0x20] = 0;
        g_stageLighting[0x25] = 0;
        g_stageLighting[0x26] = 0;
        g_stageLighting[0x21] = 0;
        g_stageLighting[0x22] = 0;
        g_stageLighting[0x31] = -0x620000;
        g_stageLighting[0x32] = 0;
        g_stageLighting[0x9] = 0xff0000;
        g_stageLighting[0x33] = 0xff0000;
        g_stageLighting[0xa] = 0xff0000;
        g_stageLighting[0xb] = 0xff0000;
        g_stageLighting[0x34] = 0xff0000;
        g_stageLighting[0x35] = 0xff0000;
        g_stageLighting[0x2] = 0xff0000;
        g_stageLighting[0x36] = 0xff0000;
        g_stageLighting[0x0] = 0xbc0000;
        g_stageLighting[0x1] = 0xca0000;
        g_stageLighting[0x29] = 0x640000;
        g_stageLighting[0x2b] = 0xfa0000;
        g_stageLighting[0x27] = 0x8000;
        g_stageLighting[0x18] = 0xff0000;
        g_stageLighting[0x19] = 0xff0000;
        g_stageLighting[0x1a] = 0xff0000;
        g_stageLighting[0x2a] = 0xff0000;
        g_stageLighting[0x15] = 0xff0000;
        g_stageLighting[0x16] = 0xff0000;
        g_stageLighting[0x17] = 0xff0000;
        g_stageLighting[0xc] = 0x9b0000;
        g_stageLighting[0xd] = 0x9b0000;
        g_stageLighting[0xe] = 0x9b0000;
        g_stageLighting[0x28] = 0x8000;
        g_stageLighting[0xf] = 0xff0000;
        g_stageLighting[0x10] = 0xff0000;
        g_stageLighting[0x11] = 0xff0000;
        g_stageLighting[0x12] = 0xc80000;
        g_stageLighting[0x13] = 0xc80000;
        g_stageLighting[0x14] = 0xc80000;
        g_stageLighting[0x23] = 0;
        g_stageLighting[0x24] = 0;
        g_stageLighting[0x37] = 0xff0000;
        g_stageLighting[0x38] = 0xff0000;
        g_stageLighting[0x58] = 0xfa0000;
        g_stageLighting[0x2d] = 0xbc0000;
        g_stageLighting[0x2e] = 0xca0000;
        g_stageLighting[0x45] = 0xff0000;
        g_stageLighting[0x46] = 0xff0000;
        g_stageLighting[0x47] = 0xff0000;
        g_stageLighting[0x42] = 0xff0000;
        g_stageLighting[0x43] = 0xff0000;
        g_stageLighting[0x44] = 0xff0000;
        g_stageLighting[0x39] = 0x9b0000;
        g_stageLighting[0x2f] = 0xff0000;
        g_stageLighting[0x3a] = 0x9b0000;
        g_stageLighting[0x3b] = 0x9b0000;
        g_stageLighting[0x54] = 0x8000;
        g_stageLighting[0x55] = 0x8000;
        g_stageLighting[0x3c] = 0xff0000;
        g_stageLighting[0x3e] = 0xff0000;
        g_stageLighting[0x3d] = 0xff0000;
        g_stageLighting[0x3f] = 0xc80000;
        g_stageLighting[0x41] = 0xc80000;
        g_stageLighting[0x40] = 0xc80000;
        g_stageLighting[0x4b] = 0;
        g_stageLighting[0x4d] = 0;
        g_stageLighting[0x4c] = 0;
        g_stageLighting[0x4e] = 0;
        g_stageLighting[0x57] = 0xff0000;
        g_stageLighting[0x56] = 0x640000;
        g_stageLighting[0x52] = 0;
        g_stageLighting[0x53] = 0;
        g_stageLighting[0x4f] = 0;
        g_stageLighting[0x50] = 0;
        g_stageLighting[0x51] = 0;
    }
    g_stageLighting[0x5a] = 0xffff0000;
    if (g_stageLighting[0x25] == 0 && g_stageLighting[0x26] == 0 &&
        g_stageLighting[0x52] == 0 && g_stageLighting[0x53] == 0) {
        FUN_00492fd0(0);
        g_stageLighting[0x5d] = 0;
        return;
    }
    FUN_00492fd0(1);
    g_stageLighting[0x5d] = 1;
}

// Blends two byte values: b + (a - b) * t, clamped to 255.
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004616c0
int FUN_004616c0(BYTE a, BYTE b, int t)
{
    int v;

    v = (((int)b << 16) + FixMul(((int)a << 16) - ((int)b << 16), t)) >> 16;
    if (v > 0xff)
        v = 0xff;
    return v;
}

// GLOBAL: CMR2 0x005885a0
int g_unk0x005885a0[8][4];
// GLOBAL: CMR2 0x00549ba0
int g_unk0x00549ba0[8][4];

// Advances (mod 200) the counters of a car's four flagged slots and clears the flags.
// FUNCTION: CMR2 0x00464c60
void FUN_00464c60(int car)
{
    int i;

    if (car < 8) {
        for (i = 0; i < 4; i++) {
            if (g_unk0x005885a0[car][i] != 0) {
                g_unk0x005885a0[car][i] = 0;
                g_unk0x00549ba0[car][i] = (g_unk0x00549ba0[car][i] + 1) % 200;
            }
        }
    }
}

// Updates the four wheel skid trails and fades their colours by surface and slip.
// match 46%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00464cb0
void StageObject_UpdateSkidTrails(int carIndex)
{
    if (carIndex >= 8 || CGameInfo::FUN_00405cd0() == 2)
        return;

    Car *pCar = Car_Get(carIndex);
    if (carIndex == 0)
        ++g_trailFrame;

    int shortLifetime = ((FUN_00460bf0(carIndex) == 1 || FUN_00460bf0(carIndex) == 2) &&
                         FUN_00460c10(carIndex) > 0x3333);
    int baseColor = 0x00ffff00;
    int wheel;
    int pointIndex;
    for (wheel = 0; wheel < 4; ++wheel) {
        int *pBaseColor = FUN_00463270(carIndex, wheel);
        baseColor = pBaseColor != NULL ? *pBaseColor : 0x00ffff00;
        for (pointIndex = 0; pointIndex < 200; ++pointIndex) {
            BYTE *pPoint = g_trailPoints[carIndex][wheel][pointIndex];
            if (g_trailPointUsed[carIndex][wheel][pointIndex] != 0) {
                int lifetime = shortLifetime ? g_stageSurfaceInfo[8].flags << 2 :
                                               g_stageSurfaceInfo[8].flags * 0x50;
                if (lifetime < *(int *)pPoint) {
                    g_trailPointUsed[carIndex][wheel][pointIndex] = 0;
                    pPoint[0x24] = 0;
                    pPoint[0x25] = 0;
                }
            }
        }
    }

    for (wheel = 0; wheel < 4; ++wheel) {
        char currentSurface = (char)pCar->wheelSurface[wheel];
        char previousSurface = (char)g_trailSurface[carIndex][wheel];
        BYTE currentMaterial = g_stageSurfaceMap[currentSurface];
        BYTE previousMaterial = g_stageSurfaceMap[previousSurface];
        StageSurfaceInfo *pCurrent = &g_stageSurfaceInfo[currentMaterial];
        StageSurfaceInfo *pPrevious = &g_stageSurfaceInfo[previousMaterial];
        int activeSurface = g_trailTimer[carIndex][wheel] > 2 && g_trailCount[carIndex] > 1 &&
                            pCar->field_0xb74 == 1 && (pCurrent->flags & 1) != 0 &&
                            (pPrevious->flags & 1) != 0;
        int dualSurface = (pCurrent->flags & 2) != 0 && (pPrevious->flags & 2) != 0;

        if (pCar->field_0xbac[wheel] == 1 && (activeSurface || dualSurface)) {
            int slip = StageObject_GetWheelSlip(carIndex, wheel);
            if ((pCurrent->flags & 2) == 0 || pCar->field_0xbac[wheel ^ 2] == 0) {
                if (slip <= 0)
                    continue;
            } else {
                if (currentSurface == 0x19)
                    continue;
                slip = 0x8000;
            }

            FixVector *pDelta = &g_trailDelta[carIndex][wheel];
            if ((pDelta->x != 0 || pDelta->z != 0) && g_trailReset[carIndex][wheel] == 0) {
                g_unk0x005885a0[carIndex][wheel] = 1;
                int trailIndex = g_unk0x00549ba0[carIndex][wheel];
                BYTE *pPoint = g_trailPoints[carIndex][wheel][trailIndex];
                // The original keeps the colour selected by the first loop's last wheel.

                FixVector up = { 0, 0x10000, 0 };
                FixVector side;
                FixVecCross(&side, pDelta, &up);
                int length = FixVecLength(&side);
                if (length == 0) {
                    side.x = side.y = side.z = 0;
                } else {
                    FixVecScaleRecip(&side, &side, length);
                }
                FixVecScale(&side, &side, 0x1eb8);

                if (CGameInfo::FUN_004063f0(6) == 0) {
                    if (FUN_0042cae0(pCar, 1) != 0)
                        FixVecScale(&side, &side, 0x9999);
                } else {
                    FixVecScale(&side, &side, 0x28000);
                }

                FixVector *pPosition = &g_unk0x00549c20[carIndex][wheel];
                int yOffset = -0x20c - wheel * 0x83;
                *(int *)(pPoint + 8) = pPosition->x + side.x;
                *(int *)(pPoint + 0xc) = pPosition->y + side.y + yOffset;
                *(int *)(pPoint + 0x10) = pPosition->z + side.z;
                *(int *)(pPoint + 0x14) = pPosition->x - side.x;
                *(int *)(pPoint + 0x18) = pPosition->y - side.y + yOffset;
                *(int *)(pPoint + 0x1c) = pPosition->z - side.z;

                BYTE opacity = (BYTE)(FixMul(slip, 0xff0000) >> 16);
                pPoint[0x24] = opacity;
                pPoint[0x25] = opacity;
                BYTE oldMaterial = pPoint[0x26];
                *(int *)pPoint = 0;
                pPoint[0x26] = (oldMaterial & 0xf0) | (currentMaterial & 0x0f);
                g_trailPointUsed[carIndex][wheel][trailIndex] = 1;
                *(int *)(pPoint + 4) = g_trailFrame;

                BYTE *pColor = g_trailColor[carIndex];
                if ((pCurrent->flags & 2) == 0) {
                    pColor[0] = (BYTE)((int)pColor[0] + ((int)pPrevious->red - (int)pColor[0]) / 2);
                    pColor[1] = (BYTE)((int)pColor[1] + ((int)pPrevious->green - (int)pColor[1]) / 2);
                    pColor[2] = (BYTE)((int)pColor[2] + ((int)pPrevious->blue - (int)pColor[2]) / 2);
                    pPoint[0x20] = (BYTE)((int)pColor[0] * (baseColor & 0xff) / 0xff);
                    pPoint[0x21] = (BYTE)((int)pColor[1] * ((baseColor >> 8) & 0xff) / 0xff);
                    pPoint[0x22] = (BYTE)((int)pColor[2] * ((baseColor >> 16) & 0xff) / 0xff);
                    g_unk0x00543708[carIndex][wheel] = 1;
                    g_unk0x00549b20[carIndex][wheel] = 0;
                } else {
                    pColor[0] = (BYTE)((int)pPrevious->red * (baseColor & 0xff) / 0xff);
                    pColor[1] = (BYTE)((int)pPrevious->green * ((baseColor >> 8) & 0xff) / 0xff);
                    pColor[2] = (BYTE)((int)pPrevious->blue * ((baseColor >> 16) & 0xff) / 0xff);
                    *(int *)(pPoint + 0x20) = *(int *)pColor;
                    g_unk0x00543708[carIndex][wheel] = 0;
                    g_unk0x00549b20[carIndex][wheel] = 1;
                }

                BYTE *pNextPoint = g_trailPoints[carIndex][wheel][(trailIndex + 1) % 200];
                pNextPoint[0x24] = 0;
                pNextPoint[0x25] = 0;
            }
        } else if (g_unk0x00543708[carIndex][wheel] != 0 ||
                   g_unk0x00549b20[carIndex][wheel] != 0) {
            g_unk0x005885a0[carIndex][wheel] = 1;
        }
    }
}

// Object classes that fill the four per-car slots of 0x590b7c.
// GLOBAL: CMR2 0x0051f888
char g_carSlotClasses[4] = { 9, 10, 12, 13 };

// Stores an object in every car slot whose class matches it.
// FUNCTION: CMR2 0x00480af0
void FUN_00480af0(BYTE *pCar, BYTE *pObject, BYTE flag)
{
    int i;

    for (i = 3; i >= 0; i--) {
        if (g_carSlotClasses[i] == (char)pObject[0x30]) {
            g_unk0x00590b7c[i][(char)pCar[0xb1a]] = pObject;
            g_unk0x00590c24[i][(char)pCar[0xb1a]] = flag;
        }
    }
}

// Clears record `index` of the 0x48-byte table at 0x58d6d0.
// FUNCTION: CMR2 0x00477ac0
void FUN_00477ac0(int index)
{
    BYTE *p = g_unk0x0058d6d0[index];

    *(short *)(p + 0x8) = 0;
    *(short *)(p + 0xa) = 0;
    *(short *)(p + 0xc) = 0;
    *(short *)(p + 0xe) = 0;
    *(short *)(p + 0x10) = 0;
    *(short *)(p + 0x12) = 0;
    *(short *)(p + 0x14) = 0;
    *(short *)(p + 0x16) = 0;
    *(short *)(p + 0x18) = 0;
    *(short *)(p + 0x1a) = 0;
    *(short *)(p + 0x30) = 0;
    *(short *)(p + 0x32) = 0;
    *(short *)(p + 0x34) = 0;
    *(short *)(p + 0x36) = 0;
    *(short *)(p + 0x38) = 0;
    *(short *)(p + 0x3a) = 0;
    *(short *)(p + 0x3c) = 0;
    *(short *)(p + 0x3e) = 0;
    *(short *)(p + 0x40) = 0;
    *(short *)(p + 0x42) = 0;
    *(short *)(p + 0x1c) = -1;
    *(short *)(p + 0x1e) = -1;
    *(short *)(p + 0x20) = -1;
    *(short *)(p + 0x22) = -1;
    *(short *)(p + 0x24) = -1;
    *(short *)(p + 0x26) = -1;
    *(short *)(p + 0x28) = -1;
    *(short *)(p + 0x2a) = -1;
    *(short *)(p + 0x2c) = -1;
    *(short *)(p + 0x2e) = -1;
    p[0x44] = 0;
    p[0x45] = 0;
}

SceneNode *SceneNode_FindByType(SceneNode *pNode, unsigned int type);
void FUN_004b2e40(BYTE *p, int value);
struct Unk0x004a3e20;
void FUN_004a3e20(Unk0x004a3e20 *pObject, int value);

// Sets up a car's damage record: clears it and keeps the textures of its two
// body parts (scene nodes of type 0xe).
// FUNCTION: CMR2 0x00477b60
void FUN_00477b60(int car, int unused1, int unused2, BYTE flag)
{
    BYTE *pRecord = g_unk0x0058d6d0[car];
    BYTE *pMesh;
    Texture *pTexture;

    pRecord[0x46] = flag;
    *(Texture **)(pRecord + 0) = NULL;
    *(Texture **)(pRecord + 4) = NULL;
    FUN_00477ac0(car);
    RallyData_ValidateIndex(car);
    pMesh = *(BYTE **)((BYTE *)SceneNode_FindByType(Car_Get(car)->pNode0x720, 0xe) + 0xc);
    FUN_004b2e40(pMesh, 0);
    pTexture = CGraphics::m_pTextureManager->textureBuffer[*(int *)(*(BYTE **)(pMesh + 0x24) + 4)];
    *(Texture **)(pRecord + 0) = pTexture;
    FUN_004a3e20((Unk0x004a3e20 *)pTexture, 2);
    if (Car_Get(car)->pNode0x724 != NULL) {
        pMesh = *(BYTE **)((BYTE *)SceneNode_FindByType(Car_Get(car)->pNode0x724, 0xe) + 0xc);
        FUN_004b2e40(pMesh, 0);
        pTexture = CGraphics::m_pTextureManager->textureBuffer[*(int *)(*(BYTE **)(pMesh + 0x24) + 4)];
        *(Texture **)(pRecord + 4) = pTexture;
        FUN_004a3e20((Unk0x004a3e20 *)pTexture, 2);
    }
}

// Sets or clears bits in the two flag bytes of record `index` of 0x58d6d0.
// FUNCTION: CMR2 0x00477c20
void FUN_00477c20(int index, char set0, char set1, BYTE mask)
{
    BYTE *p;

    p = g_unk0x0058d6d0[index];
    if (set0 != 0)
        p[0x44] |= mask;
    else
        p[0x44] &= ~mask;
    if (set1 != 0) {
        p[0x45] |= mask;
        return;
    }
    p[0x45] &= ~mask;
}

extern unsigned short *g_stageRandomTextures[3];
extern Mesh *g_stageMesh4Copy;

// Gives every triangle of the stage mesh one of the three random textures.
// FUNCTION: CMR2 0x00492b50
void FUN_00492b50(void)
{
    int r;
    int n;
    int offset;

    r = rand() % 3;
    if (g_stageRandomTextures[r] != NULL) {
        for (n = g_stageMesh4Copy->triangleCount - 1; n >= 0; n--) {
            offset = n * 0x4c - 0x4c;
            // The biased offset addresses the texture word (+4) of triangle n.
            *(unsigned int *)((BYTE *)g_stageMesh4Copy->pTriangles + (0x50 + offset)) = *g_stageRandomTextures[r];
        }
    }
}

extern int g_unk0x00590c64;

int RallyData_FUN_00411060(void);

// Destroys, in the four node tables, the nodes of every car that belong to
// the current stage kind.
// match 52%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004866a0
void FUN_004866a0(void)
{
    int car;
    int i;
    SceneNode *pNode;

    for (car = 0; car < g_unk0x00590c64; car++) {
        for (i = 0; i < 4; i++) {
            if (*(SceneNode **)((BYTE *)g_unk0x00590d7c[i] + car * 0x1a0) != NULL) {
                pNode = *(SceneNode **)((BYTE *)g_unk0x00590d7c[i] + car * 0x1a0);
                if ((int)pNode->pParent == RallyData_FUN_00411060())
                    SceneNode_Destroy(pNode);
            }
        }
    }
}

// FUNCTION: CMR2 0x0048d850
void FUN_0048d850(BYTE *pCar, BYTE *pInfo)
{
    short v;

    v = *(short *)(g_unk0x00591750 + 2 + g_unk0x00591740[*pCar] * 0x6c);
    if (v > 0x3f4 && v < 0x40b) {
        FUN_00486c00(pCar, pInfo);
        return;
    }
    if (v < -0x3f4 && v > -0x40b)
        FUN_00486c00(pCar, pInfo);
}

// Two per-lane shorts (+8 and +0x12, `slot` 0..4) scaled by 256.
// FUNCTION: CMR2 0x00477c80
void FUN_00477c80(int lane, int *pA, int *pB, int slot)
{
    BYTE *p = g_unk0x0058d6d0[lane];

    if (pA != NULL)
        *pA = (*(short *)(p + 8 + slot * 2) * 0x10000) / 256;
    if (pB != NULL)
        *pB = (*(short *)(p + 0x12 + slot * 2) * 0x10000) / 256;
}

// Resets the car slot tuning values and reseeds the random generator.
// FUNCTION: CMR2 0x00480a60
void FUN_00480a60(void)
{
    srand(400);
    g_unk0x00590c44 = 0x8000;
    g_unk0x00590c48 = 0x8000;
    g_unk0x00590c4c = 0x8000;
    g_unk0x00590c50 = 0x3333;
    g_unk0x00590c60[0] = 0x16;
    g_unk0x00590c60[1] = 0x14;
    g_unk0x00590c60[2] = 0x15;
    g_unk0x00590c60[3] = 0x12;
    g_unk0x00590bfc = 1;
    g_unk0x00590bfd = 2;
}

int Sprite_FillRect(int unused, short *pRect, BYTE *pColour, int layer);

extern BYTE g_barTextColour[4];
// Panel, text and selection colours of the stage-data screen.
// GLOBAL: CMR2 0x0051c984
BYTE g_unk0x0051c984[4] = { 210, 202, 210, 128 };
// GLOBAL: CMR2 0x0051c994
BYTE g_unk0x0051c994[4] = { 143, 135, 143, 255 };

int FUN_004055e0(void);
int FUN_004055f0(void);

// Draws the stage-data panel of the pause screen: its background, the row
// separators and, for every item, the label and the highlight sprite.
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// Identical instruction sequence; the original keeps the loop counter in ESI and
// pushes EBX/EDI inside the loop, our build spills one more register in the prologue,
// which shifts every stack slot by 4 (register-slot renumbering).
// FUNCTION: CMR2 0x004738f0
void FUN_004738f0(Menu *pMenu)
{
    MenuItem *pItem;
    short rect[4];
    short rect2[4];
    int i;
    int texture;
    BYTE *pColour;

    rect2[0] = (short)((int)(g_pGraphics->resX * 0x64) / 0x280);
    rect2[1] = (short)((int)(g_pGraphics->resY * 0xd1) / 0x1e0);
    texture = FUN_004055e0();
    rect2[2] = *(short *)(texture + 0x120);
    texture = FUN_004055e0();
    rect2[3] = *(short *)(texture + 0x122);
    rect[0] = (short)((int)(g_pGraphics->resX * 0x63) / 0x280);
    rect[1] = (short)((int)(g_pGraphics->resY * 0xa0) / 0x1e0);
    rect[2] = (short)((int)(g_pGraphics->resX * 0x11a) / 0x280);
    rect[3] = (short)((int)(g_pGraphics->resY * 0x26) / 0x1e0);
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, g_unk0x0051c984, 2);
    pItem = pMenu->items;
    for (i = 0; i < pMenu->itemCount; i++, pItem++) {
        rect2[1] = (short)((int)(g_pGraphics->resY * i * 0x24) / 0x1e0
                           + (int)(g_pGraphics->resY * 0xd1) / 0x1e0);
        if (pMenu->cursor == i) {
            pColour = g_barTextColour;
            texture = FUN_004055e0();
        } else {
            pColour = g_unk0x0051c994;
            texture = FUN_004055f0();
        }
        Font_DrawText(0, CFrontend::GetTextString(pItem->id),
                      (int)(g_pGraphics->resX * 0x78) / 0x280,
                      (int)(g_pGraphics->resY * i * 0x24) / 0x1e0
                          + (int)(g_pGraphics->resY * 0xdd) / 0x1e0,
                      (int *)pColour, 0x11);
        Sprite_Queue((SpriteRect *)(texture + 0x11c), (SpriteRect *)rect2, (Texture *)texture,
                     2, 0, NULL, NULL, pColour, 8);
        if (i == 0 || pMenu->cursor == i) {
            rect[1] = (short)((int)(g_pGraphics->resY * i * 0x24) / 0x1e0
                              + (int)(g_pGraphics->resY * 0xc6) / 0x1e0);
            rect[3] = 1;
            Sprite_FillRect((int)g_pGraphics + 0x150, rect, pColour, 1);
        }
        rect[1] = (short)((int)(g_pGraphics->resY * i * 0x24) / 0x1e0
                          + (int)(g_pGraphics->resY * 0xea) / 0x1e0);
        rect[3] = 1;
        Sprite_FillRect((int)g_pGraphics + 0x150, rect, pColour, 1);
    }
}


extern BYTE g_barTextColour[4];
extern BYTE g_unk0x0051c9a4[4];
extern char g_strVs0x0051c9ac[];
void FUN_00474420(char *text, int fraction, unsigned int font, int x, unsigned int y, int *pColour, unsigned int flags);
void FUN_00474fe0(int param1, int param2, int param3, int param4, int param5, KnockoutMatch *param6);
void FUN_00475430(int param1, KnockoutMatch *param2, short *param3, BYTE *param4, BYTE *param5,
                  int param6, int param7, int *param8, int param9);
#define KO_X(n) ((int)(g_pGraphics->resX * (n)) / 0x280)
#define KO_Y(n) ((int)(g_pGraphics->resY * (n)) / 0x1e0)

// Draws the arcade knockout bracket of the current round: the title, then
// for every match its number, both driver panels (dimmed loser, highlighted
// player) and "vs." on the match being raced; while the bracket view is up
// (0x58ca8c) the player's panel instead zooms towards the next round's slot.
// FUNCTION: CMR2 0x004744f0
void FUN_004744f0(void)
{
    BYTE *pBox2;
    BYTE *pName1;
    BYTE *pBox1;
    BYTE *pName2;
    BYTE *pText;
    short rect[4];
    int vs;
    KnockoutTable *pTable;
    int current;
    unsigned int round;
    KnockoutMatch *pMatch;
    int fading;
    KnockoutMatch *pMatches;
    int y0;
    int count;
    int highlight1;
    int bracket;
    int highlight2;
    int x0;
    int i;
    int x2;
    int y2;
    int y;
    int x;

    sprintf(CFrontend::m_stringDest, CFrontend::FUN_0040ede0(RallyData_FUN_004086b0(0)));
    CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
    Font_DrawText(0, CFrontend::m_stringDest, KO_X(0x1e), Font_GetLineHeight(0) + KO_Y(0x39) + KO_Y(5),
                  (int *)g_barTextColour, 0x11);
    pTable = (KnockoutTable *)RallyData_GetChampionshipState();
    round = (pTable->state >> 3) & 7;
    bracket = FUN_004bc0c0(g_unk0x0058ca8c);
    if (FUN_004bc0c0(&g_unk0x0058cc70) || (!FUN_004bc0c0(g_unk0x0058ca8c) && g_unk0x0058cf7c == 6))
        fading = 1;
    else
        fading = 0;
    switch (round) {
    case 1:
        count = 8;
        x0 = KO_X(0x56);
        y0 = KO_Y(0x8e);
        pMatches = pTable->round1;
        break;
    case 2:
        count = 4;
        x0 = KO_X(0x93);
        y0 = KO_Y(0x8e);
        pMatches = pTable->quarters;
        break;
    case 3:
        count = 2;
        x0 = KO_X(0x93);
        y0 = KO_Y(0xcf);
        pMatches = pTable->semis;
        break;
    case 4:
        count = 1;
        x0 = KO_X(0x115);
        y0 = KO_Y(0xcf);
        pMatches = &pTable->final;
        break;
    }
    for (i = 0; i < count; i++) {
        current = 0;
        highlight2 = 0;
        highlight1 = 0;
        vs = 0;
        y = 0;
        x = 0;
        if (bracket) {
            switch (round) {
            case 1:
                x2 = KO_X(0x93);
                y2 = KO_Y(0x8e);
                break;
            case 2:
                x2 = KO_X(0x93);
                y2 = KO_Y(0xcf);
                break;
            case 3:
                x2 = KO_X(0x115);
                y2 = KO_Y(0xcf);
                break;
            }
        }
        switch (round) {
        case 1:
            if (i == 4 || i == 5 || i == 6 || i == 7)
                y = KO_Y(0x82);
            if (i == 2 || i == 3 || i == 6 || i == 7)
                x = KO_X(0x108);
            if (bracket) {
                if (i / 2 == 2 || i / 2 == 3)
                    y2 += KO_Y(0x82);
                if (i / 2 == 1 || i / 2 == 3)
                    x2 += KO_X(0x108);
            }
            break;
        case 2:
            if (i == 2 || i == 3)
                y = KO_Y(0x82);
            if (i == 1 || i == 3)
                x = KO_X(0x108);
            if (bracket && i / 2 == 1)
                x2 += KO_X(0x108);
            break;
        case 3:
            if (i == 1)
                x = KO_X(0x108);
            break;
        }
        y += y0;
        x += (round == 1 && i % 2) ? KO_X(0x78) + x0 : x0;
        if (g_unk0x0058cf7c == 3 && i == (int)((pTable->state >> 12) & 0xf) - 1)
            current = 1;
        pMatch = &pMatches[i];
        if ((pMatch->flags & 0x3e0) == 0x3e0)
            highlight2 = 1;
        if ((pMatch->flags & 0x1f) == 0x1f)
            highlight1 = 1;
        if (((g_unk0x0058cf7c == 2 || g_unk0x0058cf7c == 4 || g_unk0x0058cf7c == 5) &&
             i == (int)((pTable->state >> 12) & 0xf)) ||
            (g_unk0x0058cf7c == 3 && i == (int)((pTable->state >> 12) & 0xf) - 1)) {
            pName1 = NULL;
            vs = 1;
            pName2 = NULL;
            pBox1 = pBox2 = pText = g_barTextColour;
        } else if (pMatch->flags & 0x400) {
            if ((pMatch->flags & 0x1800) == 0x800) {
                highlight2 = 1;
                pName1 = g_unk0x0051c9a4;
                pName2 = NULL;
                pBox1 = pBox2 = pText = g_unk0x0051c994;
            } else {
                highlight1 = 1;
                pName1 = NULL;
                pName2 = g_unk0x0051c9a4;
                pBox1 = pBox2 = pText = g_unk0x0051c994;
            }
        } else {
            pName1 = NULL;
            pBox1 = pBox2 = pText = g_unk0x0051c994;
            pName2 = NULL;
        }
        if (!bracket) {
            if (!FUN_004bc0c0(&g_unk0x0058ca80) && !fading) {
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x4f), i + 1);
                Font_DrawText(0, CFrontend::m_stringDest, (int)g_pGraphics->resX / 0x280 + x, y - KO_Y(5), (int *)pText, 0x11);
            } else {
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x4f), i + 1);
                FUN_00474420(CFrontend::m_stringDest, g_unk0x0058cc74, 0, (int)g_pGraphics->resX / 0x280 + x, y - KO_Y(5),
                             (int *)pText, 0x11);
            }
            if (fading && highlight1) {
                if (pName1 != NULL)
                    pName1[3] = (BYTE)(FixMul(0xff0000, g_unk0x0058cc74) >> 16);
                if (pBox1 != NULL)
                    pBox1[3] = (BYTE)(FixMul(0xff0000, g_unk0x0058cc74) >> 16);
                if (pText != NULL)
                    pText[3] = (BYTE)(FixMul(0xff0000, g_unk0x0058cc74) >> 16);
            } else {
                if (pName1 != NULL)
                    pName1[3] = 0xff;
                if (pBox1 != NULL)
                    pBox1[3] = 0xff;
                if (pText != NULL)
                    pText[3] = 0xff;
            }
            rect[0] = (short)x;
            rect[1] = (short)y;
            rect[2] = (short)KO_X(0x56);
            rect[3] = (short)KO_Y(0x26);
            FUN_00475430(0, pMatch, rect, pName1, pBox1, highlight1, current, (int *)pText, 0);
        } else if (highlight2) {
            if (i % 2 == 1)
                y2 += KO_Y(0x2d);
            FUN_00474fe0(x, y, x2, y2, 0, pMatch);
        }
        if (vs)
            Font_DrawText(0, g_strVs0x0051c9ac, x - KO_X(2), KO_Y(0x2f) + y, (int *)g_barTextColour, 0x14);
        if (!bracket) {
            if (fading && highlight2) {
                if (pName2 != NULL)
                    pName2[3] = (BYTE)(FixMul(0xff0000, g_unk0x0058cc74) >> 16);
                if (pBox2 != NULL)
                    pBox2[3] = (BYTE)(FixMul(0xff0000, g_unk0x0058cc74) >> 16);
                if (pText != NULL)
                    pText[3] = (BYTE)(FixMul(0xff0000, g_unk0x0058cc74) >> 16);
            } else {
                if (pName2 != NULL)
                    pName2[3] = 0xff;
                if (pBox2 != NULL)
                    pBox2[3] = 0xff;
                if (pText != NULL)
                    pText[3] = 0xff;
            }
            rect[0] = (short)x;
            rect[1] = (short)(KO_Y(0x2d) + y);
            rect[2] = (short)KO_X(0x56);
            rect[3] = (short)KO_Y(0x26);
            FUN_00475430(1, pMatch, rect, pName2, pBox2, highlight2, current, (int *)pText, 0);
        } else if (highlight1) {
            if (i % 2 == 1)
                y2 += KO_Y(0x2d);
            FUN_00474fe0(x, KO_Y(0x2d) + y, x2, y2, 1, pMatch);
        }
        g_barTextColour[3] = 0xff;
    }
}
#undef KO_X
#undef KO_Y

// Draws the first `fraction` of a text (typing effect; spaces don't count)
// and the next character on its own.
// FUNCTION: CMR2 0x00474420
void FUN_00474420(char *text, int fraction, unsigned int font, int x, unsigned int y, int *pColour, unsigned int flags)
{
    int length = (int)strlen(text);
    int count = FixMul(length << 16, fraction) >> 16;
    int i;
    int width;
    int colour;

    for (i = 0; i < count; i++) {
        if (text[i] == ' ' && count < length)
            count++;
        CFrontend::m_stringDest[i] = text[i];
    }
    CFrontend::m_stringDest[count] = 0;
    Font_DrawText(font, CFrontend::m_stringDest, x, y, pColour, flags);
    if (count < length) {
        width = Font_GetTextWidth(font, (BYTE *)CFrontend::m_stringDest);
        CFrontend::m_stringDest[0] = text[count];
        CFrontend::m_stringDest[1] = 0;
        colour = *pColour;
        Font_DrawText(font, CFrontend::m_stringDest, width + x, y, &colour, flags);
    }
}

// Fills a rectangle whose width is scaled by `scale` (16.16).
// FUNCTION: CMR2 0x00475970
int FUN_00475970(int scale, int unused, short *pRect, BYTE *pColour, int layer)
{
    short rect[4];

    rect[0] = pRect[0];
    rect[1] = pRect[1];
    rect[2] = (short)(FixMul((int)pRect[2] << 16, scale) >> 16);
    rect[3] = pRect[3];
    return Sprite_FillRect(unused, rect, pColour, layer);
}

// Wheel slip (field 0x870) above 0.15, as 0..1.
// match 89%: MSVC reuses the add's flags for the sign test (js) instead of
// re-issuing test/jl/jle, and folds the +(-0x2666) into a sub
// FUNCTION: CMR2 0x00465e40
int FUN_00465e40(int car, int wheel)
{
    int v, w;

    if (*(int *)((BYTE *)Car_Get(car) + 0x870 + wheel * 4) < 0)
        v = -*(int *)((BYTE *)Car_Get(car) + 0x870 + wheel * 4);
    else
        v = *(int *)((BYTE *)Car_Get(car) + 0x870 + wheel * 4);
    w = v - 0x2666;
    if (w < 0 || w <= 0)
        return 0;
    if (w >= 0x10000)
        return 0x10000;
    return w;
}

void Scene_GetLightColourBytes(DWORD *pColour);

// Brightness of the scene light colour: (r + g + b - 70) / 550, clamped to 0..1.
// FUNCTION: CMR2 0x004648f0
int FUN_004648f0(void)
{
    BYTE c[4];
    int v;

    Scene_GetLightColourBytes((DWORD *)c);
    v = FixDiv((c[0] + c[1] + c[2] - 0x46) << 16, 0x2260000);
    if (v < 0)
        return 0;
    if (v > 0x10000)
        v = 0x10000;
    return v;
}

// GLOBAL: CMR2 0x00547fa0
FixVector g_unk0x00547fa0 = { 0, 0, 0 };
// GLOBAL: CMR2 0x00547fe0
FixVector g_unk0x00547fe0 = { 0, 0, 0 };
// GLOBAL: CMR2 0x00547fec
SceneNode *g_unk0x00547fec = NULL;
// GLOBAL: CMR2 0x00547ff0
SceneNode *g_unk0x00547ff0 = NULL;
// GLOBAL: CMR2 0x00547ff4
int g_unk0x00547ff4 = 0;

void FUN_004b6ef0(int value);

// Places the two rear view nodes of a car from its brightness level.
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00464960
void FUN_00464960(unsigned int param_1)
{
    unsigned int car;
    Car *pCar;
    int value;
    int level;
    FixVector pos;
    FixVector offset;

    car = param_1;
    pCar = Car_Get(car);
    level = FUN_004648f0();
    if (level == 0x10000)
        *(int *)((BYTE *)pCar + 0xb58) = 0;
    else
        *(int *)((BYTE *)pCar + 0xb58) = 1;
    value = FixMul(level, 0x4c0000);
    FUN_00477c80(car, (int *)&param_1, (int *)&param_1, 4);
    if (param_1 == 0)
        value = 0x3e80000;
    FUN_004b6ef0(param_1 != 0);
    param_1 = 0x10000 - param_1;
    if ((int)param_1 < 0x10001) {
        if ((int)param_1 < 0)
            param_1 = 0;
    } else {
        param_1 = 0x10000;
    }
    param_1 = FixMul((int)param_1, 0x190000);
    level = value + param_1;
    if (level != g_unk0x00547ff4) {
        Scene_SetLightAttenuation(g_unk0x00547fec, level);
        Scene_SetLightAttenuation(g_unk0x00547ff0, level);
        g_unk0x00547ff4 = level;
    }
    FixMatrix_GetPosition(&pos, (FixMatrix *)((BYTE *)pCar->pNode0x71c + 0x98));
    FixMatrix_RotateVector(&offset, &g_unk0x00547fe0, (FixMatrix *)((BYTE *)pCar->pNode0x71c + 0x98));
    offset.x += pos.x;
    offset.y += pos.y;
    offset.z += pos.z;
    SceneNode_SetPosition(g_unk0x00547fec, &offset);
    FixMatrix_RotateVector(&offset, &g_unk0x00547fa0, (FixMatrix *)((BYTE *)pCar->pNode0x71c + 0x98));
    offset.x += pos.x;
    offset.y += pos.y;
    offset.z += pos.z;
    SceneNode_SetPosition(g_unk0x00547ff0, &offset);
}

struct Unk0x00590d74;
extern Unk0x00590d74 *g_unk0x00590d74;

// Puts the car's (up to four) attached nodes back to their creation transform
// and forgets them.
// FUNCTION: CMR2 0x00480b40
void FUN_00480b40(BYTE *pCar)
{
    SceneNode *pNode;
    int offset;
    int i;

    srand(400);
    g_unk0x00590d74 = (Unk0x00590d74 *)pCar;
    offset = (char)pCar[0xb1a] * 0x1a0;
    for (i = 3; i >= 0; i--) {
        pNode = *(SceneNode **)((BYTE *)g_unk0x00590d7c[i] + offset);
        if (pNode != NULL) {
            pNode->current = pNode->local;
            *(SceneNode **)((BYTE *)g_unk0x00590d7c[i] + offset) = NULL;
        }
    }
}

// Per-car values eased towards their targets (0x591710) by a step.
// GLOBAL: CMR2 0x00591690
int g_unk0x00591690[4];
// GLOBAL: CMR2 0x00591710
int g_unk0x00591710[4];

// Steps one stage object's current vector toward its target by `step`,
// marking it arrived (and snapping to the target) once it is closer than that.
// FUNCTION: CMR2 0x0048db00
void FUN_0048db00(BYTE *p, int step)
{
    BYTE index;
    FixVector delta;

    index = *p;
    if (g_unk0x005916f0[index] == 0) {
        delta.x = g_unk0x005916a0[index].x - g_unk0x00591898[index].x;
        delta.y = g_unk0x005916a0[index].y - g_unk0x00591898[index].y;
        delta.z = g_unk0x005916a0[index].z - g_unk0x00591898[index].z;
        if ((int)FixVec_Length(&delta) < step) {
            g_unk0x005916f0[index] = 1;
        } else {
            FixVec_Normalize(&delta, &delta);
            FixVecScale(&delta, &delta, step);
            g_unk0x00591898[index].x += delta.x;
            g_unk0x00591898[index].y += delta.y;
            g_unk0x00591898[index].z += delta.z;
        }
        if (g_unk0x005916f0[index] == 0)
            return;
    }
    g_unk0x00591898[index] = g_unk0x005916a0[index];
}

// match 35%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048dc30
void FUN_0048dc30(BYTE *pCar, int step)
{
    BYTE car;
    int cur;
    int d;

    car = *pCar;
    cur = g_unk0x00591690[car];
    d = g_unk0x00591710[car] - cur;
    if (FIX_ABS(d) < step) {
        g_unk0x00591690[car] = g_unk0x00591710[car];
        return;
    }
    if (d > 0) {
        g_unk0x00591690[car] = cur + step;
        return;
    }
    g_unk0x00591690[car] = cur - step;
}

// Five-bit field of the current round entry (bits 0..4, or 5..9 with pHigh).
// match 40%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004735a0
unsigned int FUN_004735a0(unsigned int *pHigh)
{
    unsigned int *pState;
    unsigned int *pEntry;
    unsigned int state;

    pState = RallyData_GetChampionshipState();
    state = *pState;
    pEntry = pHigh;
    switch ((state >> 3) & 7) {
    case 1:
        pEntry = pState + ((state >> 12) & 0xf) * 3 + 0x16;
        break;
    case 2:
        pEntry = pState + ((state >> 12) & 0xf) * 3 + 10;
        break;
    case 3:
        pEntry = pState + ((state >> 12) & 0xf) * 3 + 4;
        break;
    case 4:
        pEntry = pState + 1;
        break;
    }
    state = *pEntry;
    if (pHigh != NULL)
        state >>= 5;
    return state & 0x1f;
}

extern Car *g_collisionCar;
extern FixVector g_collisionTarget;
extern FixVector g_collisionLineStart;

// 1 when no corner of the collision car lies strictly between the heights of
// the line start and the target.
// FUNCTION: CMR2 0x0048f400
int FUN_0048f400(void)
{
    int start = g_collisionLineStart.y;
    int target = g_collisionTarget.y;
    int result = 1;
    int i;
    if (target > start) {
        for (i = 0; i < 8; i++) {
            int y = g_collisionCar->corners[i].y;
            if (y < target && y > start) {
                result = 0;
                break;
            }
        }
    } else {
        for (i = 0; i < 8; i++) {
            int y = g_collisionCar->corners[i].y;
            if (y > target && y < start) {
                result = 0;
                break;
            }
        }
    }
    return result;
}

void FUN_004ae410(BYTE a, BYTE b, int c, int d);

// FUNCTION: CMR2 0x00486b20
void FUN_00486b20(BYTE *pCar, BYTE *pInfo)
{
    int kind;

    if (*(int *)(pCar + 4) == 1 && *(int *)(pInfo + 4) == 2)
        FUN_00486be0(pInfo, (int)pCar);
    else
        FUN_004ae410(pCar[2], pCar[1], 1, 1);
    kind = *(int *)(pInfo + 4);
    int value;
    if (kind != 2 && kind != 1 && kind != 10)
        value = 0;
    else
        value = 0x10000;
    g_unk0x00590db0[*pCar] = value;
}

extern void **g_unk0x00590c6c;

// Resets record `index` (0x3c bytes) of list `list`: its three vectors to the
// origin and its final int to `value`.
// FUNCTION: CMR2 0x00486630
void FUN_00486630(int list, int index, int value)
{
    FixVector *p;

    p = (FixVector *)((BYTE *)g_unk0x00590c6c[list] + index * 0x3c);
    p[0].x = 0;
    p[0].y = 0;
    p[0].z = 0;
    p[3].x = 0;
    p[3].y = 0;
    p[3].z = 0;
    p[1] = p[0];
    p[2] = p[0];
    p[4].x = 0;
    p[4].y = 0;
    p[4].z = value;
}

// Sun visibility (0..100) from the lens flare sample.
// GLOBAL: CMR2 0x00543ea0
short g_sunVisibility;

BYTE Flare_SampleVisibility(short *pRect, BYTE *pColour, BYTE tolerance);
void FUN_00492bb0(int *pOut);

void Scene_GetAmbientColour(DWORD *pColour);
void Scene_SetAmbient(BYTE *pColour, int boost);
unsigned char RallyDataCountryIndex(void);
int FUN_00407270(void);
void FUN_0047e490(BYTE *pColour);

extern void *g_unk0x00543eb8;
// GLOBAL: CMR2 0x00547acc
StageObjectCount g_stageObjectCount;

int *RallyData_FUN_004075b0(int index);
// GLOBAL: CMR2 0x0051b114
int g_unk0x0051b114[9] = { 0, 0, 0, 0x6666, 0x8000, 0x9999, 0x6666, 0x8000, 0x9999 };

// Speed limits of a stage object: fixed in some stages, else scaled by the
// difficulty and the object's type factor.
// FUNCTION: CMR2 0x00461b30
void FUN_00461b30(BYTE *pObject, int type)
{
    unsigned int speed;

    *(int *)(pObject + 0x14) = 0;
    *(int *)(pObject + 0x18) = 0;
    speed = (CGameInfo::FUN_00405ca0() + 2) * 0x320000;
    if (*RallyData_FUN_004075b0(RallyDataStageIndex()) != 0) {
        *(int *)(pObject + 0x14) = 0xf0000;
        *(int *)(pObject + 0x18) = 0x500000;
        return;
    }
    if (g_unk0x0051b114[type] != 0)
        *(int *)(pObject + 0x18) = FixDiv(speed, g_unk0x0051b114[type]);
}

int *FUN_00407520(int index);
void FUN_00461b30(BYTE *pObject, int type);

// Time of day steps (mm*100+ss, 500 = 05:00) that bound the twelve stage object
// record columns.
// GLOBAL: CMR2 0x0051b034
WORD g_stageObjectTimeSteps[12] = {
    500, 600, 800, 1000, 1200, 1500, 1700, 1800, 1900, 2000, 2200, 2400
};

// Per stage setting, the two rows of the loaded .hor stage object records to
// blend: 0 = plain, 1 = CLO, 2 = STO, 3 = BLI.
// GLOBAL: CMR2 0x0051b0dc
BYTE g_stageObjectWeatherRows[9][2] = {
    { 0, 0 }, { 0, 0 }, { 0, 1 }, { 1, 2 }, { 1, 2 },
    { 1, 2 }, { 1, 3 }, { 1, 3 }, { 1, 3 }
};

// Per stage setting, the 16.16 factor that blends those two rows.
// GLOBAL: CMR2 0x0051b0f0
int g_stageObjectWeatherFactors[9] = {
    0, 0, 0x10000, 0, 0x8000, 0x10000, 0, 0x8000, 0x10000
};

// Stage object record of the primary stage setting (0x4e bytes).
// GLOBAL: CMR2 0x00543d00
BYTE g_stageObjectPrimary[0x4e];
// Stage object record of the secondary stage setting.
// GLOBAL: CMR2 0x00543e38
BYTE g_stageObjectSecondary[0x4e];

void FUN_00461710(BYTE *out, BYTE *from, BYTE *to, int t);

// Blends the stage object record for one time of day into the primary (slot 0)
// or secondary (slot 1) global record: the time picks the two adjacent columns
// of the loaded .hor table, the stage setting pair picks the record rows and
// their fixed factor, and the two column results are interpolated by how far
// the time lies into its band. NULL when a record is missing or invalid.
// FUNCTION: CMR2 0x00461830
BYTE *FUN_00461830(unsigned short timeOfDay, int slot, BYTE **records)
{
    BYTE buffer0[0x50];
    BYTE buffer1[0x50];
    int *pWeatherPair;
    int index;
    int lower;
    int i;
    int prev;
    int prevSeconds;
    int length;
    int position;
    int factor;
    int rowFrom;
    int rowTo;
    int rowFactor;
    BYTE *pFrom0;
    BYTE *pFrom1;
    BYTE *pTo0;
    BYTE *pTo1;
    BYTE *dest;

    pWeatherPair = FUN_00407520(RallyDataStageIndex());
    if (slot == 0)
        index = pWeatherPair[0];
    else
        index = pWeatherPair[1];

    lower = -1;
    for (i = 0; i < 12; i++) {
        if (g_stageObjectTimeSteps[i] >= timeOfDay) {
            lower = i;
            break;
        }
    }
    if (lower == -1)
        return NULL;

    prev = lower - 1;
    if (prev < 0) {
        prev += 12;
        prevSeconds = 0;
    } else {
        prevSeconds = (g_stageObjectTimeSteps[prev] / 100 * 60 + g_stageObjectTimeSteps[prev] % 100) << 16;
    }
    length = ((g_stageObjectTimeSteps[lower] / 100 * 60 + g_stageObjectTimeSteps[lower] % 100) << 16) - prevSeconds;

    if (timeOfDay % 100 >= 60)
        return NULL;
    position = ((timeOfDay / 100 * 60 + timeOfDay % 100) << 16) - prevSeconds;

    factor = FixDiv(position, length);
    if (factor > 0x10000)
        factor = 0x10000;
    else if (factor < 0)
        factor = 0;

    rowFrom = g_stageObjectWeatherRows[index][0];
    rowTo = g_stageObjectWeatherRows[index][1];
    rowFactor = g_stageObjectWeatherFactors[index];
    pFrom0 = records[rowFrom * 12 + prev];
    pFrom1 = records[rowFrom * 12 + lower];
    pTo0 = records[rowTo * 12 + prev];
    pTo1 = records[rowTo * 12 + lower];
    if (pFrom0 == NULL || pTo0 == NULL || pFrom1 == NULL || pTo1 == NULL)
        return NULL;

    dest = g_stageObjectPrimary;
    if (slot != 0)
        dest = g_stageObjectSecondary;

    FUN_00461710(buffer0, pFrom0, pTo0, rowFactor);
    FUN_00461710(buffer1, pFrom1, pTo1, rowFactor);
    FUN_00461710(dest, buffer0, buffer1, factor);
    return dest;
}

// Blends both stage object records of the stage's setting pair into the two out
// pointers: the primary from the first setting's time of day, the secondary
// from the second one.
// FUNCTION: CMR2 0x00461a30
void FUN_00461a30(int timePrimary, int timeSecondary, BYTE **records, BYTE **pPrimary, BYTE **pSecondary)
{
    BYTE *pObjectPrimary;
    BYTE *pObjectSecondary;

    pObjectPrimary = FUN_00461830(timePrimary, 0, records);
    pObjectSecondary = FUN_00461830(timeSecondary, 1, records);
    *pPrimary = pObjectPrimary;
    *pSecondary = pObjectSecondary;
}

// Sets the speed limits of the two stage objects of a pair from the stage's
// setting pair; a stopped one takes over the other's limit.
// FUNCTION: CMR2 0x00461a70
void FUN_00461a70(BYTE *pA, BYTE *pB)
{
    int *pPair;
    int a;
    int b;
    int limit;

    if (pA != NULL && pB != NULL) {
        pPair = FUN_00407520(RallyDataStageIndex());
        a = pPair[0];
        b = pPair[1];
        if (a == 1) {
            if (b == 0 || b == 1)
                pA[0x2f] = 200;
            else
                pA[0x2f] = 0x32;
        }
        if (b == 1) {
            if (a == 0 || a == 1)
                pB[0x2f] = 200;
            else
                pB[0x2f] = 0x32;
        }
        FUN_00461b30(pA, a);
        FUN_00461b30(pB, b);
        if (*(int *)(pA + 0x14) == 0 && *(int *)(pA + 0x18) == 0 &&
            (*(int *)(pB + 0x14) != 0 || *(int *)(pB + 0x18) != 0)) {
            limit = *(int *)(pB + 0x18);
            *(int *)(pA + 0x18) = limit;
            *(int *)(pA + 0x14) = limit;
            return;
        }
        if (*(int *)(pB + 0x14) == 0 && *(int *)(pB + 0x18) == 0 &&
            (*(int *)(pA + 0x14) != 0 || *(int *)(pA + 0x18) != 0)) {
            limit = *(int *)(pA + 0x18);
            *(int *)(pB + 0x18) = limit;
            *(int *)(pB + 0x14) = limit;
        }
    }
}

void FUN_00491790(short index, short *pA, short *pB, short *pC, short *pD, unsigned short *pFlags);
void FUN_004917f0(int *pOut, unsigned short *pIndices, int unused);
int Graphics_GetTriangleHeight(unsigned short *pHeightIndices, FixVector *pVertices, FixVector *pPosition);
void Scene_GetLightColour(DWORD *pColour, int level);
void Scene_GetAmbientColour(DWORD *pColour);
void FUN_00492e60(int *pRGB);
void FUN_004984b0(int car, int level);

// Averages the ground lighting over a car's four wheel contact points and
// stores the resulting colour and light level.
// match 61%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00462aa0
void FUN_00462aa0(unsigned int param_1, int param_2)
{
    Car *pCar;
    short *pIndex;
    int vertex;
    int i;
    int sum;
    int count;
    unsigned short idx[3];
    unsigned short d;
    unsigned short flags;
    int lighting[9];
    int h;
    BYTE light[4];
    DWORD ambient[1];
    int r, g, b;
    int avg;
    int total;
    int v;

    pCar = Car_Get(param_1);
    i = 0;
    sum = 0;
    count = 0;
    pIndex = (short *)((BYTE *)pCar + 0xa9e);
    vertex = (int)pCar + 0x270;
    do {
        if (*pIndex >= 0) {
            FUN_00491790(*pIndex, (short *)&idx[0], (short *)&idx[1], (short *)&idx[2],
                         (short *)&d, &flags);
            FUN_004917f0(lighting, idx, (int)&d);
            h = Graphics_GetTriangleHeight(idx, (FixVector *)lighting, (FixVector *)vertex);
            if (h != -0x3e70000) {
                sum = sum + h;
                count = count + 1;
                Scene_GetLightColour((DWORD *)light, h);
                ((BYTE *)g_unk0x00543f28)[(i + pCar->field_0xb1a * 4) * 4] = light[0];
                ((BYTE *)g_unk0x00543f28)[(i + pCar->field_0xb1a * 4) * 4 + 1] = light[1];
                ((BYTE *)g_unk0x00543f28)[(i + pCar->field_0xb1a * 4) * 4 + 2] = light[2];
            }
        }
        i = i + 1;
        pIndex = pIndex + 1;
        vertex = vertex + 0xc;
    } while (i < 4);
    if (count > 0) {
        avg = FixDiv(sum, count << 16);
        if (avg > 0x10000)
            avg = 0x10000;
        else if (avg < 0)
            avg = 0;
        Scene_GetLightColour((DWORD *)light, avg);
        r = (*(int *)(light + 0) & 0xff) << 16;
        g = (*(int *)(light + 1) & 0xff) << 16;
        b = (*(int *)(light + 2) & 0xff) << 16;
        Scene_GetAmbientColour(ambient);
        r = r - (*(int *)((BYTE *)ambient + 0) & 0xff) * 0x10000;
        g = g - (*(int *)((BYTE *)ambient + 1) & 0xff) * 0x10000;
        b = b - (*(int *)((BYTE *)ambient + 2) & 0xff) * 0x10000;
        if ((FUN_00422fb0(param_2) & 0xff) == param_1)
            FUN_00492e60(&r);
        *(int *)((BYTE *)pCar + 0xa70) = avg;
        total = (r < 0 ? -r : r) + (g < 0 ? -g : g) + (b < 0 ? -b : b);
        v = FixMul(total, 0x55);
        if (v > 0x10000)
            v = 0x10000;
        FUN_004984b0(param_1, v);
    }
}

// Interpolates the two animated values of every 0x2c-byte record by t (16.16).
// FUNCTION: CMR2 0x00461bb0
void FUN_00461bb0(int t)
{
    int i;
    BYTE *p;

    for (i = 0; i < g_unk0x00547acc; i++) {
        p = (BYTE *)g_unk0x00543eb8 + i * 0x2c;
        *(int *)(p + 0x1c) = FixMul(t, *(int *)(p + 0x10) - *(int *)(p + 0x18)) + *(int *)(p + 0x18);
        *(int *)(p + 0x24) = *(int *)(p + 0x20) + FixMul(t, *(int *)(p + 0x14) - *(int *)(p + 0x20));
    }
}
void FUN_004920d0(DWORD *pColour, DWORD *pReference);
void FUN_00492220(DWORD *pColour, DWORD *pReference);
void FUN_004923d0(DWORD *pColour);
void FUN_00492470(DWORD *pColour);
void FUN_00492520(DWORD *pColour);
void FUN_004926f0(int angle, int unused, int sunAngle);
void FUN_00492bd0(int view);
void FUN_00492f10(void);
void FUN_00492fe0(DWORD *pColour, int start, int end);
void FUN_00462cb0(BYTE *pColour);
void Stage_SetHeightColours(BYTE *pLow, BYTE *pHigh, BYTE *pReference, int referenceBlend);
void StageLights_UpdateDirection(void);
extern int g_unk0x00543d88;
extern int g_unk0x00543d8c;

// Blended ramp value and the two step sizes derived from it.
// GLOBAL: CMR2 0x00543d58
int g_unk0x00543d58;
// GLOBAL: CMR2 0x00543d5c
int g_unk0x00543d5c;
// Stage object colour pushed straight to the stage mesh.
// GLOBAL: CMR2 0x00543eb4
BYTE g_unk0x00543eb4[4];
// Set while the object ramps still have to be re-applied.
// GLOBAL: CMR2 0x00543ef8
int g_unk0x00543ef8;
// GLOBAL: CMR2 0x00543f00
BillboardDef g_unk0x00543f00;

// Rebuilds every lighting colour of the stage from the current weather blend
// factor and pushes them to the stage meshes: height ramp (low/high/reference),
// ground, sky, light and ambient colours plus the sun light vector. Called once
// per view index by the stage renderer.
// The table g_stageLighting holds two complete parameter sets - primary in
// [0x00..0x2c] and secondary (the other weather) in [0x2d..0x59], every
// secondary entry exactly 0x2d dwords above its primary counterpart - followed
// by [0x5a] the blend factor, [0x5b] the blended intensity, [0x5c] the
// {current, target} weather words and [0x5d] the weather-changed flag.
// match 30%: the logic follows the asm, but MSVC6 gives the twelve colour
// buffers (and three of the scratch vectors) different stack slots than the
// original - the slot assignment depends on the whole local set and could not
// be reproduced - so every memory operand is displaced.
// match 30%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00461c30
void FUN_00461c30(int index)
{
    BYTE lowColour[4] = {0, 0, 0, 0xff};
    BYTE highColour[4] = {0, 0, 0, 0xff};
    BYTE rampColour[4] = {0, 0, 0, 0xff};
    BYTE referenceColour[4] = {0, 0, 0, 0xff};
    BYTE groundColour[4] = {0, 0, 0, 0xff};
    BYTE groundRefColour[4] = {0, 0, 0, 0xff};
    BYTE objectColour[4] = {0, 0, 0, 0xff};
    BYTE ambientColour[4] = {0, 0, 0, 0xff};
    BYTE objectRefColour[4] = {0xff, 0xff, 0xff, 0xff};
    BYTE lightColour[4] = {0xff, 0xff, 0xff, 0xff};
    BYTE skyColour[4] = {0xff, 0xff, 0xff, 0xff};
    BYTE heightColour[4] = {0xff, 0xff, 0xff, 0xff};
    FixVector v[3];
    FixVector colour;
    FixVector delta;
    FixVector ambientMix;
    int blend;
    int value;
    int flag;
    int *pObject;
    int *pView;

    pObject = (int *)((BYTE *)g_unk0x00543eb8 + index * 0x2c);
    pView = (int *)((BYTE *)g_unk0x00547ac8 + index * 0x178);
    if (pObject[7] != g_stageLighting[0x5a] || g_unk0x00543d88 != g_unk0x00543d8c ||
        *((WORD *)&g_stageLighting[0x5c] + 1) != *(WORD *)&g_stageLighting[0x5c] || pObject[10] != 0) {
        g_stageLighting[0x5a] = pObject[7];
        pObject[10] = 0;
        if (g_stageLighting[0x5a] > 0x10000)
            g_stageLighting[0x5a] = 0x10000;
        flag = 1;
        if (*(WORD *)&g_stageLighting[0x5c] == 0xffff || pView[0x15] <= 0xcccc || pView[0] != 1)
            flag = 0;
        g_stageLighting[0x5b] = FixMul(g_stageLighting[0x59] - g_stageLighting[0x2c], g_stageLighting[0x5a]) +
                                g_stageLighting[0x2c];

        v[2].x = g_stageLighting[0x2d] - g_stageLighting[0x00];
        v[2].y = g_stageLighting[0x2e] - g_stageLighting[0x01];
        v[2].z = g_stageLighting[0x2f] - g_stageLighting[0x02];
        FixVecScale(&v[2], &v[2], g_stageLighting[0x5a]);
        v[2].x += g_stageLighting[0x00];
        v[2].y += g_stageLighting[0x01];
        v[2].z += g_stageLighting[0x02];
        lowColour[0] = (BYTE)(v[2].x >> 16);
        lowColour[1] = (BYTE)(v[2].y >> 16);
        lowColour[2] = (BYTE)(v[2].z >> 16);

        v[0].x = g_stageLighting[0x30] - g_stageLighting[0x03];
        v[0].y = g_stageLighting[0x31] - g_stageLighting[0x04];
        v[0].z = g_stageLighting[0x32] - g_stageLighting[0x05];
        FixVecScale(&v[0], &v[0], g_stageLighting[0x5a]);
        v[0].x += g_stageLighting[0x03];
        v[0].y += g_stageLighting[0x04];
        v[0].z += g_stageLighting[0x05];
        colour.x = v[0].x + v[2].x;
        colour.y = v[0].y + v[2].y;
        colour.z = v[0].z + v[2].z;
        highColour[0] = (BYTE)(colour.x >> 16);
        highColour[1] = (BYTE)(colour.y >> 16);
        highColour[2] = (BYTE)(colour.z >> 16);

        v[1].x = g_stageLighting[0x33] - g_stageLighting[0x06];
        v[1].y = g_stageLighting[0x34] - g_stageLighting[0x07];
        v[1].z = g_stageLighting[0x35] - g_stageLighting[0x08];
        FixVecScale(&v[1], &v[1], g_stageLighting[0x5a]);
        v[1].x += g_stageLighting[0x06];
        v[1].y += g_stageLighting[0x07];
        v[1].z += g_stageLighting[0x08];
        referenceColour[0] = (BYTE)(v[1].x >> 16);
        referenceColour[1] = (BYTE)(v[1].y >> 16);
        referenceColour[2] = (BYTE)(v[1].z >> 16);

        colour.x = g_stageLighting[0x36] - g_stageLighting[0x09];
        colour.y = g_stageLighting[0x37] - g_stageLighting[0x0a];
        colour.z = g_stageLighting[0x38] - g_stageLighting[0x0b];
        FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
        colour.x += g_stageLighting[0x09];
        colour.y += g_stageLighting[0x0a];
        colour.z += g_stageLighting[0x0b];
        g_unk0x00543eb4[3] = 0xff;
        g_unk0x00543eb4[0] = (BYTE)(colour.x >> 16);
        g_unk0x00543eb4[1] = (BYTE)(colour.y >> 16);
        g_unk0x00543eb4[2] = (BYTE)(colour.z >> 16);

        value = FixMul(g_stageLighting[0x58] - g_stageLighting[0x2b], g_stageLighting[0x5a]) +
                g_stageLighting[0x2b];
        g_unk0x00543d58 = FixMul(value, 0x66);
        g_unk0x00543d5c = FixMul(value, 0x88);
        blend = FixMul(g_stageLighting[0x54] - g_stageLighting[0x27], g_stageLighting[0x5a]) +
                g_stageLighting[0x27];

        colour.x = g_stageLighting[0x3c] - g_stageLighting[0x0f];
        colour.y = g_stageLighting[0x3d] - g_stageLighting[0x10];
        colour.z = g_stageLighting[0x3e] - g_stageLighting[0x11];
        FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
        colour.x += g_stageLighting[0x0f];
        colour.y += g_stageLighting[0x10];
        colour.z += g_stageLighting[0x11];
        objectColour[0] = (BYTE)(colour.x >> 16);
        objectColour[1] = (BYTE)(colour.y >> 16);
        objectColour[2] = (BYTE)(colour.z >> 16);
        objectColour[3] = (BYTE)((g_stageLighting[0x29] +
                                  FixMul(g_stageLighting[0x56] - g_stageLighting[0x29], g_stageLighting[0x5a])) >> 16);

        colour.x = g_stageLighting[0x3f] - g_stageLighting[0x12];
        colour.y = g_stageLighting[0x40] - g_stageLighting[0x13];
        colour.z = g_stageLighting[0x41] - g_stageLighting[0x14];
        FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
        colour.x += g_stageLighting[0x12];
        colour.y += g_stageLighting[0x13];
        colour.z += g_stageLighting[0x14];
        FixVecScale(&colour, &colour, FixMul(g_stageLighting[0x5b], 0x3333) + 0xcccc);
        ambientMix.x = colour.x;
        ambientMix.y = colour.y;
        ambientMix.z = colour.z;
        ambientColour[0] = (BYTE)(ambientMix.x >> 16);
        ambientColour[1] = (BYTE)(ambientMix.y >> 16);
        ambientColour[2] = (BYTE)(ambientMix.z >> 16);

        delta.x = g_stageLighting[0x39] - g_stageLighting[0x0c];
        delta.y = g_stageLighting[0x3a] - g_stageLighting[0x0d];
        delta.z = g_stageLighting[0x3b] - g_stageLighting[0x0e];
        FixVecScale(&delta, &delta, g_stageLighting[0x5a]);
        delta.x += g_stageLighting[0x0c];
        delta.y += g_stageLighting[0x0d];
        delta.z += g_stageLighting[0x0e];
        delta.x -= ambientMix.x;
        delta.y -= ambientMix.y;
        delta.z -= ambientMix.z;
        FixVecScale(&delta, &delta, g_stageLighting[0x5b]);
        delta.x += ambientMix.x;
        delta.y += ambientMix.y;
        delta.z += ambientMix.z;
        groundColour[0] = (BYTE)(delta.x >> 16);
        groundColour[1] = (BYTE)(delta.y >> 16);
        groundColour[2] = (BYTE)(delta.z >> 16);
        value = FixMul(g_stageLighting[0x55] - g_stageLighting[0x28], g_stageLighting[0x5a]) +
                g_stageLighting[0x28];

        colour.x = v[1].x - delta.x;
        colour.y = v[1].y - delta.y;
        colour.z = v[1].z - delta.z;
        FixVecScale(&colour, &colour, FixMul(value, blend));
        colour.x += delta.x;
        colour.y += delta.y;
        colour.z += delta.z;
        colour.x -= ambientMix.x;
        colour.y -= ambientMix.y;
        colour.z -= ambientMix.z;
        FixVecScale(&colour, &colour, g_stageLighting[0x5b]);
        colour.x += ambientMix.x;
        colour.y += ambientMix.y;
        colour.z += ambientMix.z;
        groundRefColour[0] = (BYTE)(colour.x >> 16);
        groundRefColour[1] = (BYTE)(colour.y >> 16);
        groundRefColour[2] = (BYTE)(colour.z >> 16);

        colour.x = g_stageLighting[0x4e] - g_stageLighting[0x21];
        colour.y = g_stageLighting[0x4f] - g_stageLighting[0x22];
        colour.z = g_stageLighting[0x50] - g_stageLighting[0x23];
        FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
        colour.x += g_stageLighting[0x21];
        colour.y += g_stageLighting[0x22];
        colour.z += g_stageLighting[0x23];
        skyColour[0] = (BYTE)(colour.x >> 16);
        skyColour[1] = (BYTE)(colour.y >> 16);
        skyColour[2] = (BYTE)(colour.z >> 16);
        skyColour[3] = (BYTE)((g_stageLighting[0x24] +
                               FixMul(g_stageLighting[0x51] - g_stageLighting[0x24], g_stageLighting[0x5a])) >> 16);

        if (g_stageLighting[0x5d] != 0) {
            colour.x = g_stageLighting[0x4b] - g_stageLighting[0x1e];
            colour.y = g_stageLighting[0x4c] - g_stageLighting[0x1f];
            colour.z = g_stageLighting[0x4d] - g_stageLighting[0x20];
            FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
            colour.x += g_stageLighting[0x1e];
            colour.y += g_stageLighting[0x1f];
            colour.z += g_stageLighting[0x20];
            heightColour[0] = (BYTE)(colour.x >> 16);
            heightColour[1] = (BYTE)(colour.y >> 16);
            heightColour[2] = (BYTE)(colour.z >> 16);
            heightColour[3] = 0xff;
            FUN_00492fe0((DWORD *)heightColour,
                         g_stageLighting[0x25] +
                             FixMul(g_stageLighting[0x52] - g_stageLighting[0x25], g_stageLighting[0x5a]),
                         g_stageLighting[0x26] +
                             FixMul(g_stageLighting[0x53] - g_stageLighting[0x26], g_stageLighting[0x5a]));
        }
        if (flag) {
            if (groundColour[0] <= 0xeb)
                groundColour[0] = (BYTE)(groundColour[0] + 0x14);
            else
                groundColour[0] = 0xff;
            if (groundColour[1] <= 0xeb)
                groundColour[1] = (BYTE)(groundColour[1] + 0x14);
            else
                groundColour[1] = 0xff;
            if (groundColour[2] <= 0xeb)
                groundColour[2] = (BYTE)(groundColour[2] + 0x14);
            else
                groundColour[2] = 0xff;
        }

        colour.x = g_stageLighting[0x45] - g_stageLighting[0x18];
        colour.y = g_stageLighting[0x46] - g_stageLighting[0x19];
        colour.z = g_stageLighting[0x47] - g_stageLighting[0x1a];
        FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
        colour.x += g_stageLighting[0x18];
        colour.y += g_stageLighting[0x19];
        colour.z += g_stageLighting[0x1a];
        rampColour[0] = (BYTE)(colour.x >> 16);
        rampColour[1] = (BYTE)(colour.y >> 16);
        rampColour[2] = (BYTE)(colour.z >> 16);
        rampColour[3] = (BYTE)((g_stageLighting[0x2a] +
                                FixMul(g_stageLighting[0x57] - g_stageLighting[0x2a], g_stageLighting[0x5a])) >> 16);

        colour.x = g_stageLighting[0x2b] - g_stageLighting[0x15];
        colour.y = g_stageLighting[0x2c] - g_stageLighting[0x16];
        colour.z = g_stageLighting[0x2d] - g_stageLighting[0x17];
        FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
        colour.x += g_stageLighting[0x15];
        colour.y += g_stageLighting[0x16];
        colour.z += g_stageLighting[0x17];
        colour.x -= ambientMix.x;
        colour.y -= ambientMix.y;
        colour.z -= ambientMix.z;
        FixVecScale(&colour, &colour, g_stageLighting[0x5b]);
        colour.x += ambientMix.x;
        colour.y += ambientMix.y;
        colour.z += ambientMix.z;

        if (flag) {
            objectRefColour[3] = (objectColour[3] <= 0xc8) ? (BYTE)(objectColour[3] + 0x32) : 0xfa;
            groundColour[0] = 0xff;
            groundColour[1] = 0xff;
            groundColour[2] = 0xff;
            groundColour[3] = 0xff;
            groundRefColour[0] = 0xff;
            groundRefColour[1] = 0xff;
            groundRefColour[2] = 0xff;
            groundRefColour[3] = 0xff;
            colour.x = 0xff0000;
            colour.y = 0xff0000;
            colour.z = 0xff0000;
            blend = 0x4ccc;
        } else {
            *(int *)objectRefColour = *(int *)objectColour;
        }

        FUN_00492220((DWORD *)objectColour, (DWORD *)objectRefColour);
        Stage_SetHeightColours(lowColour, highColour, referenceColour, blend);
        FUN_004920d0((DWORD *)groundColour, (DWORD *)groundRefColour);
        FUN_00492470((DWORD *)rampColour);
        FUN_00492520((DWORD *)skyColour);
        lightColour[3] = (BYTE)-(int)flag;
        FUN_004923d0((DWORD *)lightColour);
        FUN_00462cb0(ambientColour);
        FUN_00492e30(&colour);
        FUN_004925c0(g_stageLighting[0x1b] + FixMul(g_stageLighting[0x48] - g_stageLighting[0x1b], g_stageLighting[0x5a]),
                     g_stageLighting[0x1c], g_stageLighting[0x1d]);
        StageLights_UpdateDirection();
    }
    FUN_004926f0(0, 0, pObject[9]);
    if (g_unk0x00543ef8 != 0)
        g_unk0x00543ef8 = 0;
    FUN_00492bd0(index);
    FUN_00492f10();
}

// Sets the scene's ambient colour when it changes.
// FUNCTION: CMR2 0x00462cb0
void FUN_00462cb0(BYTE *pColour)
{
    BYTE ambient[4];

    Scene_GetAmbientColour((DWORD *)ambient);
    if (ambient[0] != pColour[0] || ambient[1] != pColour[1] || ambient[2] != pColour[2]) {
        if ((BYTE)RallyDataCountryIndex() == 3)
            Scene_SetAmbient(pColour, 0);
        else
            Scene_SetAmbient(pColour, 1);
    }
    if ((BYTE)FUN_00407270())
        FUN_0047e490(pColour);
}

// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00462d10
void FUN_00462d10(short *pRect)
{
    BYTE colour[4];
    int c;

    FUN_00492bb0(&c);
    colour[0] = (BYTE)c;
    colour[1] = (BYTE)(c >> 8);
    colour[2] = (BYTE)(c >> 16);
    g_sunVisibility = 100 - Flare_SampleVisibility(pRect, colour, 0x28);
    if (g_sunVisibility < 0) {
        g_sunVisibility = 0;
        return;
    }
    if (g_sunVisibility > 100)
        g_sunVisibility = 100;
}

// ---------------------------------------------------------------------------
// Stage lights: up to eight glowing lamps (e.g. start lights) whose on/off
// pattern per state comes from g_stageLightStates, plus the car lamp textures.

void FUN_004ae3d0(BYTE *p, BYTE value);
void FUN_004ae3f0(BYTE *p, int value);
void FUN_004ae260(void);
struct Unk0x004a3e20;
void FUN_004a3e20(Unk0x004a3e20 *pObject, int value);

struct StageLight {
    BYTE *pGlow;        // glow source
    BYTE *pGlow2;       // second glow when the kind doubles them
    int level;          // 0..1, eased towards the pattern
};

// Position (x, y in units of the transform axes) of each light per kind.
// GLOBAL: CMR2 0x0051b1a8
int g_stageLightOffsets[4][8][2] = {
    -112787, 246022, -75038, 246022, -38273, 246022, -196, 246022, 37093, 246022, 74579, 246022, 111935, 246022, 0, 0,
    -238616, 170590, -238616, 143130, -238616, 114950, -215220, 175964, -215220, 160104, -215220, 144244, -215220, 128385, -215220, 112525,
    -624558, 203882, -604962, 203882, -585891, 203882, -567934, 203882, -550633, 203882, -531693, 203882, -513736, 203882, 0, 0,
    -567672, 203882, -548143, 203882, -529072, 203882, -511049, 203882, -493748, 203882, -474873, 203882, -456851, 203882,
};
// Depth of the lights per kind.
// GLOBAL: CMR2 0x0051b2a8
int g_stageLightDepth[4] = {
    -58982, -38469, -31653, -31653,
};
// Colour (0 red, 1 green) of each light per kind.
// GLOBAL: CMR2 0x0051b2b8
int g_stageLightColour[4][8] = {
    0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 2, 2, 2,
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
};
// On/off pattern of each light, per kind (4), state (7) and light (8).
// GLOBAL: CMR2 0x0051b338
int g_stageLightStates[4][7][8] = {
    0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0,
    1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
    1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0,
    1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0,
    1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0,
    1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
    1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
    1,
};
// Offset (along the light row's right axis) of each light's second glow;
// 0 when the kind has only one glow per light.
// GLOBAL: CMR2 0x0051b6b8
int g_stageLightDouble[4] = {
    0, 459341, 1048444, 944439,
};
// Glow size per kind and colour.
// GLOBAL: CMR2 0x0051b6c8
int g_stageLightSize[4][3] = {
    19660, 19660, 0, 13107, 13107, 6553, 9830, 9830,
    0, 9830, 9830,
};
// Countries (bit per stage index) that use the eight-light layout.
// GLOBAL: CMR2 0x0051b6f8
unsigned short g_stageLightEightMask[10] = {
    0x0000, 0x03ff, 0x03ff, 0x0000, 0x0002, 0x0200, 0x03ff, 0x0250,
};
// GLOBAL: CMR2 0x00547ad8
int g_stageLightKind;
// GLOBAL: CMR2 0x00547adc
int g_stageLightHasMatrix;
// GLOBAL: CMR2 0x00547ae0
StageLight g_stageLights[8];
// GLOBAL: CMR2 0x00547b40
FixMatrix g_stageLightMatrix;
// GLOBAL: CMR2 0x00547cc8
Texture *g_stageLightRedTexture;
// GLOBAL: CMR2 0x00547ccc
Texture *g_stageLightGreenTexture;
// GLOBAL: CMR2 0x00547cd0
Texture *g_stageLightTexture;
// GLOBAL: CMR2 0x00547cd4
int g_stageLightsActive;
// GLOBAL: CMR2 0x00547cd8
int g_stageLightCount;

// Lamp textures of a car (0x4c bytes; two cars).
struct CarLights {
    BYTE field_0x0[0x2c];
    Texture *pBrake;        // 0x2c
    Texture *pReverse;      // 0x30
    Texture *pHazard;       // 0x34
    Texture *pHazard2;      // 0x38
    Texture *pHead;         // 0x3c
    BYTE field_0x40[0xc];
};

// GLOBAL: CMR2 0x00547f80
CarLights g_carLights[2];

// GLOBAL: CMR2 0x0051b724
char g_strLightRedTga[] = "\\NEWIMAGE\\lgt_red.tga";
// GLOBAL: CMR2 0x0051b70c
char g_strLightGreenTga[] = "\\NEWIMAGE\\lgt_gre.tga";
// GLOBAL: CMR2 0x0051b9d4
char g_strHazardLiteTga[] = "\\NEWIMAGE\\hazdlite.tga";
// GLOBAL: CMR2 0x0051b9bc
char g_strRevLiteTga[] = "\\NEWIMAGE\\revlite.tga";
// GLOBAL: CMR2 0x0051b9a4
char g_strHeadLiteTga[] = "\\NEWIMAGE\\headlite.tga";
// GLOBAL: CMR2 0x0051b98c
char g_strBrakeLiteTga[] = "\\NEWIMAGE\\brkelite.tga";

StageFile *StageTiming_GetStageFile0(void);

extern char g_strPathConcat[];

#define LOAD_STAGE_TEXTURE(dst, name)                                                          \
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), name);    \
    dst = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(), CFrontend::m_stringDest, \
                                    &loaded, NULL, 0, 0)

// Places the lights: rows of the given 4x3 vectors are the right, up,
// forward axes and the position (NULL: no transform).
// match 47%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00463290
void StageLights_SetTransform(FixVector *pAxes)
{
    FixVector v;

    if (pAxes == NULL) {
        g_stageLightHasMatrix = (int)pAxes;
        return;
    }
    g_stageLightHasMatrix = 1;
    v = pAxes[0];
    FixMatrix_SetRight(&v, &g_stageLightMatrix);
    v = pAxes[1];
    FixMatrix_SetUp(&v, &g_stageLightMatrix);
    v = pAxes[2];
    FixMatrix_SetForward(&v, &g_stageLightMatrix);
    v = pAxes[3];
    FixMatrix_SetPosition(&v, &g_stageLightMatrix);
}

// FUNCTION: CMR2 0x00463360
void StageLights_LoadTextures(void)
{
    bool loaded;

    LOAD_STAGE_TEXTURE(g_stageLightRedTexture, g_strLightRedTga);
    LOAD_STAGE_TEXTURE(g_stageLightGreenTexture, g_strLightGreenTga);
    FUN_004a3e20((Unk0x004a3e20 *)g_stageLightRedTexture, 1);
    FUN_004a3e20((Unk0x004a3e20 *)g_stageLightGreenTexture, 1);
    g_stageLightTexture = g_stageLightRedTexture;
}

// Eases every light towards its pattern for the current state and updates
// its glow(s).
// FUNCTION: CMR2 0x00463bd0
void StageLights_Update(void)
{
    StageLight *p;
    int target;
    int d;
    int i;

    if (g_stageLightsActive == 0)
        return;
    for (i = 0, p = g_stageLights; i < g_stageLightCount; i++, p++) {
        target = g_stageLightStates[g_stageLightKind][g_unk0x00547b80][i] != 0 ? 0x10000 : 0;
        d = target - p->level;
        if (FIX_ABS(d) < 0x3333)
            p->level = target;
        else if (d > 0)
            p->level += 0x3333;
        else
            p->level -= 0x3333;
        if (p->level > 0) {
            FUN_004ae3d0(p->pGlow, 1);
            if (g_stageLightDouble[g_stageLightKind] != 0)
                FUN_004ae3d0(p->pGlow2, 1);
        } else {
            FUN_004ae3d0(p->pGlow, 0);
            if (g_stageLightDouble[g_stageLightKind] != 0)
                FUN_004ae3d0(p->pGlow2, 0);
        }
        FUN_004ae3f0(p->pGlow, p->level);
        if (g_stageLightDouble[g_stageLightKind] != 0)
            FUN_004ae3f0(p->pGlow2, p->level);
    }
}

// Turns every light off (state 6).
// FUNCTION: CMR2 0x00463d00
void StageLights_Off(void)
{
    int i;

    g_unk0x00547b80 = 6;
    if (g_stageLightsActive == 0)
        return;
    for (i = 0; i < g_stageLightCount; i++) {
        g_stageLights[i].level = 0;
        FUN_004ae3d0(g_stageLights[i].pGlow, 0);
        if (g_stageLightDouble[g_stageLightKind] != 0)
            FUN_004ae3d0(g_stageLights[i].pGlow2, 0);
    }
}

// Resets the glow table and loads the lamp textures of both cars.
// FUNCTION: CMR2 0x00463d60
void CarLights_LoadTextures(void)
{
    bool loaded;

    FUN_004ae260();
    CGame::RegisterCallback(FUN_004ae260, NULL);
    LOAD_STAGE_TEXTURE(g_carLights[0].pHazard, g_strHazardLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[0].pReverse, g_strRevLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[0].pHead, g_strHeadLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[0].pBrake, g_strBrakeLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[0].pHazard2, g_strHazardLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[1].pHazard, g_strHazardLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[1].pReverse, g_strRevLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[1].pHead, g_strHeadLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[1].pBrake, g_strBrakeLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[1].pHazard2, g_strHazardLiteTga);
}

void FUN_00492890(FixVector *pOut);
void FUN_004928c0(int *pOut1, int *pOut2, int *pOut3);
void FUN_00498370(FixVector *v);

// Direction of the stage light (sun direction raised by the sky offset),
// normalised; passed on to FUN_00498370.
// GLOBAL: CMR2 0x005477f8
FixVector g_stageLightDirection;

// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00463070
void StageLights_UpdateDirection(void)
{
    FixVector d;
    FixVector s;
    int lift;
    int unused1;
    int unused2;
    int m;

    FUN_00492890(&d);
    FUN_004928c0(&lift, &unused1, &unused2);
    d.y += lift;
    if (FIX_ABS(d.x) > FIX_ABS(d.y) && FIX_ABS(d.x) > FIX_ABS(d.z))
        m = FIX_ABS(d.x);
    else if (FIX_ABS(d.y) > FIX_ABS(d.x) && FIX_ABS(d.y) > FIX_ABS(d.z))
        m = FIX_ABS(d.y);
    else
        m = FIX_ABS(d.z);
    FixVecScaleRecip(&s, &d, m);
    FIX_NORMALIZE_INTO(g_stageLightDirection, s);
    FUN_00498370(&g_stageLightDirection);
}

// Light row axes with the scale removed.
// GLOBAL: CMR2 0x00547b88
FixMatrix g_stageLightBasis;

unsigned char RallyDataStageIndex(void);
struct GlowLight;
GlowLight *Glow_Add(int type, FixVector *pos, FixVector *dir, int unused1, int sizeX, int sizeY, int billboardTexture,
                    int layerTexture, int intensity, int node, BYTE projected, int unused2, int field_0x40);

#define LIGHT_SIZE(i) FixMul(lenX, g_stageLightSize[g_stageLightKind][g_stageLightColour[g_stageLightKind][i]])

// Creates the glows of the stage lights (start gantry) from the placed
// transform: layout by country and stage, one or two glows per light, all off.
// match 53%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00463410
void StageLights_Create(void)
{
    FixVector pos[8];
    FixVector v;
    FixVector local;
    FixVector zero;
    FixVector dir;
    FixVector origin;
    FixVector pair;
    int lenX, lenY, lenZ;
    BYTE stage;
    int i;

    local.x = 0;
    local.y = 0;
    local.z = -0x10000;
    zero.x = 0;
    zero.y = 0;
    zero.z = 0;
    g_stageLightsActive = 0;
    if (g_stageLightHasMatrix == 0)
        return;
    FixMatrix_GetPosition(&origin, &g_stageLightMatrix);
    if (origin.x == 0 && origin.y == 0 && origin.z == 0)
        return;
    stage = RallyDataStageIndex();
    if ((char)RallyDataStageIndex() == 10) {
        g_stageLightCount = 7;
        if ((char)RallyDataCountryIndex() == 3)
            g_stageLightKind = 2;
        else
            g_stageLightKind = 3;
    } else if ((g_stageLightEightMask[RallyDataCountryIndex() & 0xff] & (unsigned short)(1 << stage)) == 0) {
        g_stageLightCount = 7;
        g_stageLightKind = 0;
    } else {
        g_stageLightCount = 8;
        g_stageLightKind = 1;
    }
    FixMatrix_GetRight(&v, &g_stageLightMatrix);
    lenX = FixVecLength(&v);
    FixVecScaleRecip(&v, &v, lenX);
    FixMatrix_SetRight(&v, &g_stageLightBasis);
    FixMatrix_GetUp(&v, &g_stageLightMatrix);
    lenY = FixVecLength(&v);
    FixVecScaleRecip(&v, &v, lenY);
    FixMatrix_SetUp(&v, &g_stageLightBasis);
    FixMatrix_GetForward(&v, &g_stageLightMatrix);
    lenZ = FixVecLength(&v);
    FixVecScaleRecip(&v, &v, lenZ);
    FixMatrix_SetForward(&v, &g_stageLightBasis);
    FixMatrix_RotateVector(&dir, &local, &g_stageLightBasis);
    FIX_NORMALIZE_INTO(dir, dir);
    FixMatrix_GetRight(&pair, &g_stageLightMatrix);
    FixVecScale(&pair, &pair, g_stageLightDouble[g_stageLightKind]);
    for (i = 0; i < g_stageLightCount; i++) {
        v.x = FixMul(g_stageLightOffsets[g_stageLightKind][i][0], lenX);
        v.y = FixMul(g_stageLightOffsets[g_stageLightKind][i][1], lenY);
        v.z = FixMul(g_stageLightDepth[g_stageLightKind], lenZ);
        FixMatrix_RotateVector(&pos[i], &v, &g_stageLightBasis);
        pos[i].x += origin.x;
        pos[i].y += origin.y;
        pos[i].z += origin.z;
        g_stageLights[i].pGlow = (BYTE *)Glow_Add(
            2, &pos[i], &dir, (int)&zero, LIGHT_SIZE(i), LIGHT_SIZE(i),
            (int)(&g_stageLightRedTexture)[g_stageLightColour[g_stageLightKind][i]],
            (int)(&g_stageLightRedTexture)[g_stageLightColour[g_stageLightKind][i]], 0, 0, 0, (int)&dir, 0);
        if (g_stageLightDouble[g_stageLightKind] != 0) {
            pos[i].x += pair.x;
            pos[i].y += pair.y;
            pos[i].z += pair.z;
            g_stageLights[i].pGlow2 = (BYTE *)Glow_Add(
                2, &pos[i], &dir, (int)&zero, LIGHT_SIZE(i), LIGHT_SIZE(i),
                (int)(&g_stageLightRedTexture)[g_stageLightColour[g_stageLightKind][i]],
                (int)(&g_stageLightRedTexture)[g_stageLightColour[g_stageLightKind][i]], 0, 0, 0, (int)&dir, 0);
        }
        g_stageLights[i].level = 0;
        FUN_004ae3d0(g_stageLights[i].pGlow, 0);
        if (g_stageLightDouble[g_stageLightKind] != 0)
            FUN_004ae3d0(g_stageLights[i].pGlow2, 0);
    }
    g_unk0x00547b80 = 6;
    g_stageLightsActive = 1;
}

// ---------------------------------------------------------------------------
// Replay buffers and stage event records.

extern int g_unk0x00588d3c;
extern void *g_unk0x00588e80[8];

// Both sets share a sixteen-entry slot table, traversed as a single array.
// GLOBAL: CMR2 0x00588d40
void **g_unk0x00588d40[16];
#define g_unk0x00588d60 (g_unk0x00588d40 + 8)
// GLOBAL: CMR2 0x00588ea0
void *g_unk0x00588ea0[8];

// FUNCTION: CMR2 0x0046c540
void Replay_InitSlots(void)
{
    int i;

    g_unk0x00588d3c = 0;
    memset(g_unk0x00588e80, 0, sizeof(g_unk0x00588e80));
    for (i = 0; i < 8; i++)
        g_unk0x00588d40[i] = &g_unk0x00588e80[i];
    g_unk0x00588d14 = 0;
    memset(g_unk0x00588ea0, 0, sizeof(g_unk0x00588ea0));
    for (i = 0; i < 8; i++)
        g_unk0x00588d60[i] = &g_unk0x00588ea0[i];
}


// GLOBAL: CMR2 0x00588ec8
BYTE g_unk0x00588ec8;

// Frees the replay buffers (the second set only when not needed any more).
// match 65%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046c6d0
int Replay_FreeBuffers(void)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (g_unk0x00588e80[i] != NULL) {
            CFileBuffer::FreeGenericFileBuffer(g_unk0x00588e80[i]);
            g_unk0x00588e80[i] = NULL;
        }
        if (CGameInfo::FUN_00406320() || CGameInfo::FUN_00405d80() == 3 || CGameInfo::FUN_00405d80() == 7) {
            if (g_unk0x00588ea0[i] != NULL && g_unk0x00588d18[i] == 0) {
                CFileBuffer::FreeGenericFileBuffer(g_unk0x00588ea0[i]);
                g_unk0x00588ea0[i] = NULL;
                g_unk0x00588d18[i] = 0;
            }
        }
    }
    g_unk0x00588d3c = 0;
    g_unk0x00588d14 = 0;
    g_unk0x00588ec8 = 0;
    return 1;
}

// Stops recording into a replay buffer, closing the current segment.
// match 36%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046cc60
int Replay_StopRecording(BYTE *pBuffer)
{
    short *pCount;

    if (pBuffer == NULL || *(int *)(pBuffer + 0xc) == 0)
        return 0;
    *(int *)(pBuffer + 0xc) = 0;
    if (*(int *)(pBuffer + 0x10) == 0) {
        if (*(int *)(pBuffer + 0x1c) != 2)
            goto done;
    } else if (*(int *)(pBuffer + 0x1c) != 2) {
        pCount = (short *)(*(int *)(pBuffer + 0x104) + *(short *)(pBuffer + 0x100) * 2);
        (*pCount)++;
        (*(short *)(pBuffer + 0x100))++;
        *(int *)(pBuffer + 0x10) = 0;
        *(int *)(pBuffer + 0x18) = 0;
        return 1;
    }
    *(short *)(pBuffer + 0x100) = 1;
done:
    *(int *)(pBuffer + 0x10) = 0;
    *(int *)(pBuffer + 0x18) = 0;
    return 1;
}

// GLOBAL: CMR2 0x0051bfec
char g_strReplaySaved[] = "\n";

// Writes a replay buffer (header plus its used arrays) to a file.
// FUNCTION: CMR2 0x0046d3f0
int Replay_Save(BYTE *pBuffer, char *pName)
{
    int frames;
    int extra;
    int records;

    frames = *(short *)(pBuffer + 0xfc);
    if (*(int *)(pBuffer + 0x1c) == 2)
        extra = *(short *)(pBuffer + 0xfe) * frames * 0x10;
    else
        extra = *(short *)(pBuffer + 0xfe) * frames * 4;
    if (*(int *)(pBuffer + 0x1c) == 0)
        records = frames * 0x114c;
    else
        records = frames * 0x5c;
    CInstallInfo::WriteFileToDisk(pName, 0, pBuffer, records + 0x110 + frames * 2 + extra);
    puts(pName);
    puts(g_strReplaySaved);
    return 1;
}

// Points the arrays of a replay buffer into its data block (after the 0x110
// header), according to its type.
// match 45%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046d470
void Replay_SetupPointers(BYTE *pBuffer, int unused)
{
    int recordSize;
    int frames;
    int extraSize;

    if (*(int *)(pBuffer + 0x1c) == 0) {
        *(BYTE **)(pBuffer + 0x24) = pBuffer + 0x110;
        *(BYTE **)(pBuffer + 0x30) = NULL;
        recordSize = 0x114c;
    } else {
        *(BYTE **)(pBuffer + 0x30) = pBuffer + 0x110;
        *(BYTE **)(pBuffer + 0x24) = NULL;
        recordSize = 0x5c;
    }
    frames = *(short *)(pBuffer + 0xfc);
    if (*(int *)(pBuffer + 0x1c) == 2) {
        *(BYTE **)(pBuffer + 0x3c) = NULL;
        extraSize = 0x10;
        *(BYTE **)(pBuffer + 0x40) = pBuffer + frames * recordSize + 0x110;
    } else {
        *(BYTE **)(pBuffer + 0x40) = NULL;
        extraSize = 4;
        *(BYTE **)(pBuffer + 0x3c) = pBuffer + frames * recordSize + 0x110;
    }
    *(BYTE **)(pBuffer + 0x104) = pBuffer + (*(short *)(pBuffer + 0xfe) * extraSize + recordSize) * frames + 0x110;
}

// Velocity estimate between two matrices (current at +0, previous at +0x40):
// (difference - 3 * offset) / 6, stored at +0x80.
// match 79%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046e340
void FUN_0046e340(BYTE *pMatrices, BYTE *pInfo)
{
    FixVector a;
    FixVector b;
    FixVector d;
    FixVector o;

    FixMatrix_GetPosition(&a, (FixMatrix *)pMatrices);
    FixMatrix_GetPosition(&b, (FixMatrix *)(pMatrices + 0x40));
    d.x = b.x - a.x;
    d.y = b.y - a.y;
    d.z = b.z - a.z;
    FixVecScale(&o, (FixVector *)(pInfo + 0x408), 0x30000);
    d.x -= o.x;
    d.y -= o.y;
    d.z -= o.z;
    FixVecScale((FixVector *)(pMatrices + 0x80), &d, 0x2aaa);
}

// Timed stage events (0x1c bytes each).
struct EventRec {
    int pos;            // 0x0  packed x/y of the event's area in the texture
    short a;            // 0x4  width
    short b;            // 0x6  height
    BYTE field_0x8[8];
    unsigned short step;    // 0x10
    short range;        // 0x12
    unsigned short counter; // 0x14
    short field_0x16;   // 0x16
    BYTE paused;        // 0x18
    BYTE active;        // 0x19 has an area
    BYTE pad_0x1a[2];
};

// GLOBAL: CMR2 0x00588ed8
EventRec g_eventRecords[11];
// GLOBAL: CMR2 0x00589210
int g_eventScales[2];
#define g_eventScale (g_eventScales[0])
// GLOBAL: CMR2 0x00589318
int g_unk0x00589318;
// GLOBAL: CMR2 0x0058931c
int g_eventCount;
// GLOBAL: CMR2 0x00589320
int g_unk0x00589320[4];
// GLOBAL: CMR2 0x00589330
BYTE g_eventsDirty;
// GLOBAL: CMR2 0x00588ed0
Texture *g_eventTextures[2];
#define g_eventTexture (g_eventTextures[0])

// Scales the event steps by their share of the largest a*b product.
// FUNCTION: CMR2 0x0046e6a0
void Events_ComputeSteps(void)
{
    EventRec *p;
    int maxProduct;
    int product;
    int share;
    int i;

    maxProduct = 0;
    for (i = g_eventCount, p = g_eventRecords; i > 0; i--, p++) {
        product = p->b * p->a;
        if (product > maxProduct)
            maxProduct = product;
    }
    for (i = 0, p = g_eventRecords; i < g_eventCount; i++, p++) {
        share = FixDiv((p->b * p->a) << 16, (maxProduct / 2) << 16);
        p->step = (unsigned short)FixMul(share, g_eventScale);
        p->range = (short)FixMul(share, 30000);
        p->counter = 0;
        p->field_0x16 = 0;
        p->paused = 0;
    }
    if (g_eventCount > 2) {
        i = g_eventCount - 2;
        p = &g_eventRecords[2];
        do {
            p->step >>= 1;
            if (p->step == 0)
                p->step = 1;
            p++;
        } while (--i);
    }
}

// GLOBAL: CMR2 0x00589331
BYTE g_unk0x00589331;

// GLOBAL: CMR2 0x00589334
int g_unk0x00589334;

// Stamp patterns of the event sprites: a 4x4 grid per rotation and a 16x16
// grid per rotation.
// GLOBAL: CMR2 0x0051c240
BYTE g_unk0x0051c240[0x40] = {
    0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
};
// GLOBAL: CMR2 0x0051c280
BYTE g_unk0x0051c280[0x400] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00,
    0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void Events_Reset(void);

// Resets the stage events and finds the event texture (name ending in BODF).
// FUNCTION: CMR2 0x0046e580
void Events_Init(int unused, int slot, char animate)
{
    int i;
    Texture *pTexture;

    Events_Reset();
    g_eventCount = 0;
    g_unk0x00589331 = animate == 0;
    for (i = 0; i < 2048; i++) {
        pTexture = CGraphics::m_pTextureManager->textureBuffer[i];
        if (pTexture != NULL &&
            strncmp(pTexture->name + strlen(pTexture->name) - 8, CGraphics::m_strSuffixBODF, 4) == 0) {
            (&g_eventTexture)[slot] = CGraphics::m_pTextureManager->textureBuffer[i];
            break;
        }
    }
    if ((&g_eventTexture)[slot] != NULL)
        (&g_eventScale)[slot] = (&g_eventTexture)[slot]->width;
}

// Adds a stage event (up to 11) for the texture area pArea (packed position,
// width, height) and recomputes the event steps.
// match 20%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046e620
void Events_Add(EventRec *pArea, int unused)
{
    EventRec *p;

    if (g_eventCount < 11) {
        p = &g_eventRecords[g_eventCount];
        p->active = 0;
        p->paused = 0;
        p->step = 0;
        p->counter = 0;
        if (pArea != NULL && pArea->a > 0 && pArea->b > 0) {
            p->pos = pArea->pos;
            *(int *)&p->a = *(int *)&pArea->a;
            p->active = 1;
        }
        g_eventCount++;
    }
    Events_ComputeSteps();
}

// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046e530
void Events_Reset(void)
{
    EventRec *p;
    int i;

    g_unk0x00589318 = 0;
    if (g_eventCount > 0) {
        p = g_eventRecords;
        for (i = g_eventCount; i != 0; i--, p++) {
            p->counter = 0;
            p->field_0x16 = 0;
            p->paused = 0;
        }
    }
    g_eventsDirty = 0;
    for (i = 0; i < 4; i++)
        g_unk0x00589320[i] = 0;
}

// Advances an event's counter; returns 1 when it wraps past 255.
// FUNCTION: CMR2 0x0046ed40
BYTE Events_Tick(int index)
{
    BYTE wrapped;

    wrapped = 0;
    if (g_eventRecords[index].paused == 0) {
        g_eventRecords[index].counter += g_eventRecords[index].step;
        if (g_eventRecords[index].counter >= 0x100) {
            g_eventRecords[index].counter = 0;
            wrapped = 1;
        }
    }
    return wrapped;
}

void Graphics_ReloadTexture(Texture *pTexture);

// FUNCTION: CMR2 0x0046ef20
void Events_Flush(void)
{
    if (g_eventCount > 0 && g_eventsDirty != 0) {
        Graphics_ReloadTexture(g_eventTexture);
        Events_Reset();
    }
}

// Cloud cover (0 light .. 2 heavy) of each weather setting.
// GLOBAL: CMR2 0x0051c688
BYTE g_cloudLevels[9] = { 1, 1, 2, 2, 2, 2, 2, 2, 2 };
// GLOBAL: CMR2 0x0051c694
char g_strCloudLight[] = "Cloud_Light";
// GLOBAL: CMR2 0x0051c6a0
char g_strCloudMed[] = "Cloud_Med";
// GLOBAL: CMR2 0x0051c6ac
char g_strCloudHeavy[] = "Cloud_Heavy";
// GLOBAL: CMR2 0x0051c6b8
char g_strPcLow[] = "PcLow\\";
// GLOBAL: CMR2 0x0051c6c0
char g_strPc[] = "Pc\\";
// GLOBAL: CMR2 0x0051c6c4
char g_strCloudsDir[] = "%s\\Clouds\\";

int *FUN_00407520(int index);

// Builds the path of the stage's cloud texture in CFrontend::m_stringDest
// from the heavier of the two weather settings.
// match 79%: the two level loads stay in byte registers (cl) instead of being
// zero-extended into 32-bit ones (bl/edx) as in the original
// FUNCTION: CMR2 0x0046ef50
void FUN_0046ef50(void)
{
    int *pPair;
    int level;
    char *suffix;

    sprintf(CFrontend::m_stringDest, g_strCloudsDir, CInstallInfo::GetTracksDir());
    if (CGameInfo::FUN_00405d00() == 0)
        suffix = g_strPc;
    else
        suffix = g_strPcLow;
    strcat(CFrontend::m_stringDest, suffix);
    pPair = FUN_00407520(RallyDataStageIndex());
    level = 0;
    if (g_cloudLevels[pPair[0]] > 0)
        level = g_cloudLevels[pPair[0]];
    if (g_cloudLevels[pPair[1]] > level)
        level = g_cloudLevels[pPair[1]];
    switch (level) {
    case 2:
        strcat(CFrontend::m_stringDest, g_strCloudHeavy);
        break;
    case 1:
        strcat(CFrontend::m_stringDest, g_strCloudMed);
        break;
    case 0:
        strcat(CFrontend::m_stringDest, g_strCloudLight);
        break;
    }
}

// Releases the file loaded by 0x46f060 (registered callback).
// FUNCTION: CMR2 0x0046f030
BYTE FUN_0046f030(void)
{
    if (g_unk0x00589448.buffer) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00589448.buffer);
        g_unk0x00589448.buffer = NULL;
    }
    g_unk0x00589448.didFileLoad = FALSE;
    g_unk0x00589448.fileSize = 0;
    return 1;
}

int FUN_00422f50(BYTE index);

// Places the four view nodes of a split screen from the current view position.
// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046f330
void FUN_0046f330(int param_1)
{
    int car;
    int node;
    FixVector pos;
    FixVector vecB;
    FixVector vecC;
    FixVector offset;

    car = param_1;
    node = FUN_00422f50(car);
    if (node == 3)
        node = (int)Car_Get(car)->pNode0x720;
    else
        node = (int)g_viewNodes[car];
    FixMatrix_GetPosition(&pos, (FixMatrix *)(node + 0x98));
    vecC.x = pos.x;
    vecB.x = pos.x;
    vecC.z = pos.z;
    vecB.z = pos.z;
    pos.y = pos.y - 0xf0000;
    vecC.y = pos.y;
    vecB.y = pos.y;
    FUN_004928c0(&offset.x, &offset.y, &offset.z);
    pos.y = pos.y + offset.x;
    vecC.y = vecC.y + offset.y;
    vecB.y = vecB.y + offset.z;
    if ((char)RallyDataCountryIndex() == 3 && (char)RallyDataStageIndex() == 7)
        vecB.y = vecB.y - 0x40000;
    if (g_unk0x00589438 != 0)
        SceneNode_SetPosition((SceneNode *)g_unk0x00589438, &pos);
    if (g_unk0x0058943c != 0)
        SceneNode_SetPosition((SceneNode *)g_unk0x0058943c, &vecC);
    if (g_unk0x00589440 != 0)
        SceneNode_SetPosition((SceneNode *)g_unk0x00589440, &vecB);
    if (g_unk0x00589444 != 0) {
        vecB.y = vecB.y - 0x140000;
        SceneNode_SetPosition((SceneNode *)g_unk0x00589444, &vecB);
    }
    FUN_004928f0(&param_1);
    offset.x = 0;
    offset.y = param_1;
    offset.z = 0;
    FixMatrix_SetUp(&offset, (FixMatrix *)((BYTE *)g_unk0x00589438 + 0x98));
}

void FUN_00486b20(BYTE *pCar, BYTE *pInfo);
void FUN_00486b90(BYTE *pCar, BYTE *pInfo);

#define RECORD_NEAR_90(v) (((v) > 0x3f4 && (v) < 0x40b) || ((v) < -0x3f4 && (v) > -0x40b))

// FUNCTION: CMR2 0x0048d7b0
void FUN_0048d7b0(BYTE *pCar, BYTE *pInfo)
{
    short v;

    v = *(short *)(g_unk0x00591750 + 2 + g_unk0x00591740[*pCar] * 0x6c);
    if (RECORD_NEAR_90(v))
        FUN_00486b20(pCar, pInfo);
}

// FUNCTION: CMR2 0x0048d800
void FUN_0048d800(BYTE *pInfo, BYTE *pCar)
{
    short v;

    v = *(short *)(g_unk0x00591750 + 2 + g_unk0x00591740[*pCar] * 0x6c);
    if (RECORD_NEAR_90(v))
        FUN_00486b90(pCar, pInfo);
}

// Headlight glows of the stage objects (100 records of 0x5c bytes).
// GLOBAL: CMR2 0x0058e4e0
BYTE g_unk0x0058e4e0[100][0x5c];

// Creates the glow of every record and resets the records.
// match 89%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047d510
void FUN_0047d510(void)
{
    FixVector unused;
    BYTE *p;

    for (p = g_unk0x0058e4e0[0]; (int)p < (int)g_unk0x0058e4e0[100]; p += 0x5c) {
        *(int *)(p + 0x3c) = 0;
        *(GlowLight **)(p + 0x38) =
            Glow_Add(1, &unused, &unused, (int)&unused, 0x3333, 0x3333, (int)g_carLights[0].pHazard,
                     (int)g_carLights[1].pHazard, 0x10000, 0, 0xb4, (int)&unused, 0x20000);
        FUN_004ae3d0(*(BYTE **)(p + 0x38), 0);
        *(int *)(p + 0x2c) = 0;
        *(short *)(p + 0x34) = -1;
        *(int *)(p + 0x0) = 0;
        *(int *)(p + 0x4) = 0x10000;
        *(int *)(p + 0x8) = 0;
    }
    memset(g_unk0x0058e4a8, 0, sizeof(g_unk0x0058e4a8));
}

#include "WheelTrail.h"

// Skid mark points of every car wheel: a used flag and a 0x28-byte record
// for each of the 200 points.
// GLOBAL: CMR2 0x00548220
BYTE g_trailPointUsed[8][4][200];
// GLOBAL: CMR2 0x00549b20
int g_unk0x00549b20[8][4];
// GLOBAL: CMR2 0x00549da0
BYTE g_trailPoints[8][4][200][0x28];

// Clears every wheel's skid marks and trail state.
// FUNCTION: CMR2 0x00465530
void FUN_00465530(void)
{
    int car;
    int point;
    int wheel;

    for (car = 0; car < 8; car++) {
        for (point = 0; point < 200; point++) {
            for (wheel = 0; wheel < 4; wheel++) {
                g_trailPointUsed[car][wheel][point] = 0;
                *(int *)g_trailPoints[car][wheel][point] = 0;
            }
        }
    }
    memset(g_unk0x00549ba0, 0, sizeof(g_unk0x00549ba0));
    memset(g_trailState, 0, sizeof(g_trailState));
    memset(g_trailTimer, 0, sizeof(g_trailTimer));
    memset(g_trailPrevState, 0, sizeof(g_trailPrevState));
    memset(g_unk0x00549b20, 0, sizeof(g_unk0x00549b20));
    memset(g_unk0x00543708, 0, sizeof(g_unk0x00543708));
    for (wheel = 0; wheel < 8 * 4; wheel++)
        ((int *)g_trailReset)[wheel] = 1;
    for (car = 0; car < 8; car++) {
        FixVector *pDelta = g_trailDelta[car];
        for (wheel = 0; wheel < 4; wheel++) {
            pDelta->x = 0;
            pDelta->y = 0;
            pDelta->z = 0;
            pDelta++;
        }
    }
}

// GLOBAL: CMR2 0x0051bce0
char g_strNewImageSkidBlankTga[] = "\\NEWIMAGE\\skid\\skids_blank.tga";
// GLOBAL: CMR2 0x0051bd00
char g_strNewImageSkidMarkTga[] = "\\NEWIMAGE\\skid\\skids_mark.tga";
// GLOBAL: CMR2 0x0051bd20
char g_strNewImageSkid1Tga[] = "\\NEWIMAGE\\skid\\skids_1.tga";

// GLOBAL: CMR2 0x00588740
Texture *g_unk0x00588740;
// GLOBAL: CMR2 0x00588744
Texture *g_unk0x00588744;
// GLOBAL: CMR2 0x00588748
Texture *g_unk0x00588748;

// Loads the three skid mark textures and picks the trail lifetime and the
// skid colour for the current weather and country.
// FUNCTION: CMR2 0x00465600
void FUN_00465600(void)
{
    char name[260];
    bool loaded;
    BYTE country;

    FUN_00465530();
    sprintf(name, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strNewImageSkid1Tga);
    g_unk0x00588740 = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(), name, &loaded, 0, 0, 0);
    sprintf(name, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strNewImageSkidMarkTga);
    g_unk0x00588744 = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(), name, &loaded, 0, 0, 0);
    sprintf(name, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strNewImageSkidBlankTga);
    g_unk0x00588748 = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(), name, &loaded, 0, 0, 0);
    if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 ||
        CGameInfo::FUN_00405d80() == 7)
        g_stageSurfaceInfo[8].flags = 4;
    else
        g_stageSurfaceInfo[8].flags = 2;
    country = (BYTE)RallyDataCountryIndex();
    if (country != 0) {
        if (country > 3 && country <= 5) {
            g_stageSurfaceInfo[2].red = 0xb7;
            g_stageSurfaceInfo[2].green = 0x89;
            g_stageSurfaceInfo[2].blue = 0x43;
            *(int *)&g_stageSurfaceInfo[7].red = *(int *)&g_stageSurfaceInfo[2].red;
        } else {
            g_stageSurfaceInfo[2].red = 0xb7;
            g_stageSurfaceInfo[2].green = 0x89;
            g_stageSurfaceInfo[2].blue = 0x43;
            *(int *)&g_stageSurfaceInfo[7].red = *(int *)&g_stageSurfaceInfo[2].red;
        }
    } else {
        g_stageSurfaceInfo[2].red = 0x89;
        g_stageSurfaceInfo[2].green = 0x7a;
        g_stageSurfaceInfo[2].blue = 0x6f;
        *(int *)&g_stageSurfaceInfo[7].red = *(int *)&g_stageSurfaceInfo[2].red;
    }
}

// GLOBAL: CMR2 0x00590b50
FixVector g_unk0x00590b50;

int FixMatrix_InverseRotateVector(FixVector *pOut, FixVector *pV, FixMatrix *pM);
extern float g_oneOverRandMax;

// Applies an impulse (scaled by a random 0.7..1.0) against the vehicle's
// velocity, in its body frame.
// match 34%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004853c0
void FUN_004853c0(FixVector *pImpulse)
{
    BYTE *pVehicle = (BYTE *)g_unk0x00590d74;
    FixVector scaled;
    FixVector local;
    int random;
    int scale;

    FixMatrix_InverseRotateVector(&g_unk0x00590b50, (FixVector *)(pVehicle + 0x408), *(FixMatrix **)(pVehicle + 0x750));
    random = (int)(__int64)(rand() * g_oneOverRandMax * CGraphics::m_65536);
    scale = FixMul(random, 0x4ccc) + 0xb333;
    scaled.x = FixMul(pImpulse->x, scale);
    scaled.y = FixMul(pImpulse->y, scale);
    scaled.z = FixMul(pImpulse->z, scale);
    FixMatrix_InverseRotateVector(&local, &scaled, *(FixMatrix **)(pVehicle + 0x750));
    g_unk0x00590b50.y -= local.y;
    g_unk0x00590b50.x -= local.x;
    g_unk0x00590b50.z -= local.z;
}

void SceneNode_SetViewMaskTree(SceneNode *pNode, BYTE mask);
int FUN_0046bec0(int *pState, BYTE *pIn, BYTE *pOut, BYTE *pCounter, Car *pCar);

// Hides every node of the cars whose replay buffer is a finished ghost run.
// FUNCTION: CMR2 0x0046e440
void FUN_0046e440(void)
{
    void ***pp;
    BYTE *pBuffer;
    Car *pCar;

    for (pp = g_unk0x00588d40; (int)pp < (int)(g_unk0x00588d40 + 16); pp++) {
        pBuffer = (BYTE *)**pp;
        if (pBuffer != NULL && *(int *)(pBuffer + 4) != 0 && *(int *)(pBuffer + 0x1c) == 2 &&
            *(int *)(pBuffer + 0xe8) == 0) {
            pCar = Car_Get(pBuffer[0x20]);
            SceneNode_SetViewMaskTree(pCar->pNode0x720, 0);
            SceneNode_SetViewMaskTree(pCar->pWheelNodes[0], 0);
            SceneNode_SetViewMaskTree(pCar->pWheelNodes[1], 0);
            SceneNode_SetViewMaskTree(pCar->pWheelNodes[2], 0);
            SceneNode_SetViewMaskTree(pCar->pWheelNodes[3], 0);
            if (pCar->pNode0x724 != NULL)
                SceneNode_SetViewMaskTree(pCar->pNode0x724, 0);
            if (pCar->pExtraNodes[0] != NULL) {
                SceneNode_SetViewMaskTree(pCar->pExtraNodes[0], 0);
                SceneNode_SetViewMaskTree(pCar->pExtraNodes[1], 0);
                SceneNode_SetViewMaskTree(pCar->pExtraNodes[2], 0);
                SceneNode_SetViewMaskTree(pCar->pExtraNodes[3], 0);
            }
        }
    }
}

// Decodes a replay input packet into a car's control record.
// FUNCTION: CMR2 0x0046c4b0
int FUN_0046c4b0(int *pState, BYTE *pIn, BYTE car, BYTE *pCounter)
{
    Car *pCar = Car_Get(car);

    return FUN_0046bec0(pState, pIn, (BYTE *)pCar + 0x1d0, pCounter, pCar);
}

// GLOBAL: CMR2 0x0058e0a0
Car *g_unk0x0058e0a0;
// Which of the steering/throttle/brake controls are analogue.
// GLOBAL: CMR2 0x0058e0a8
BYTE g_unk0x0058e0a8[3];
// Button bit of each car control (read from the controller mapping).
// GLOBAL: CMR2 0x0051f4b0
unsigned short g_carButtonMasks[9] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 0x100 };

short *Car_GetOrder(void);

// Resets a car's controls for the start of the stage (automatic box on,
// velocity damped).
// match 23%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047b870
void FUN_0047b870(int index)
{
    BYTE *p;

    g_unk0x0058e0a0 = Car_Get(Car_GetOrder()[index]);
    p = (BYTE *)g_unk0x0058e0a0;
    *(int *)(p + 0x1dc) = 0;
    g_unk0x0058e0a0->field_0x1d8 = 0;
    p[0x1d4] = 0;
    g_unk0x0058e0a0->flag0x1d0[3] = 0;
    g_unk0x0058e0a0->flag0x1d0[2] = 0;
    g_unk0x0058e0a0->flag0x1d0[1] = 0;
    g_unk0x0058e0a0->flag0x1d0[0] = 0;
    if (g_unk0x0058e0a0->field_0xb94 != 0)
        g_unk0x0058e0a0->flag0x1d0[3] = 0;
    else
        g_unk0x0058e0a0->flag0x1d0[3] = 1;
    g_unk0x0058e0a0->flag0x1d0[2] = 0;
    g_unk0x0058e0a0->field_0x1d8 = 1;
    g_unk0x0058e0a0->field_0xb9c = 0;
    *(int *)(p + 0x1e4) = 0;
    p = (BYTE *)g_unk0x0058e0a0;
    g_unk0x0058e0a0->velocity.x = FixMul(g_unk0x0058e0a0->velocity.x, 0xf851);
    ((Car *)p)->velocity.y = FixMul(((Car *)p)->velocity.y, 0xf851);
    ((Car *)p)->velocity.z = FixMul(((Car *)p)->velocity.z, 0xf851);
}

DWORD FUN_0040bdd0(unsigned short slot);

// Reads the controller mapping of a slot into the car control masks and the
// analogue flags.
// FUNCTION: CMR2 0x0047bca0
void FUN_0047bca0(int slot)
{
    g_carButtonMasks[0] = CInput::GetButtonMapping(slot, 0);
    g_carButtonMasks[1] = CInput::GetButtonMapping(slot, 1);
    g_carButtonMasks[2] = CInput::GetButtonMapping(slot, 2);
    g_carButtonMasks[3] = CInput::GetButtonMapping(slot, 3);
    g_carButtonMasks[4] = CInput::GetButtonMapping(slot, 4);
    g_carButtonMasks[5] = CInput::GetButtonMapping(slot, 5);
    g_carButtonMasks[6] = CInput::GetButtonMapping(slot, 6);
    g_carButtonMasks[7] = CInput::GetButtonMapping(slot, 7);
    g_carButtonMasks[8] = CInput::GetButtonMapping(slot, 8);
    if (FUN_0040bdd0(slot) == 0 || (int)CInput::FUN_0040c210(slot, 0) == -1)
        g_unk0x0058e0a8[0] = 0;
    else
        g_unk0x0058e0a8[0] = 1;
    if (CInput::FUN_0040be00(slot) == 0 || (int)CInput::FUN_0040c210(slot, 2) == -1)
        g_unk0x0058e0a8[1] = 0;
    else
        g_unk0x0058e0a8[1] = 1;
    if (CInput::FUN_0040be00(slot) != 0 && (int)CInput::FUN_0040c210(slot, 3) != -1) {
        g_unk0x0058e0a8[2] = 1;
        return;
    }
    g_unk0x0058e0a8[2] = 0;
}

// Encodes a car's control record into a 4-byte replay packet.
// match 31%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046bdc0
void FUN_0046bdc0(BYTE *pIn, BYTE *pOut, int active, int handbrake, int lightA, int lightB)
{
    BYTE hb = (handbrake != 0 && active != 0) ? 1 : 0;
    BYTE a = (lightA != 0 && active != 0) ? 1 : 0;
    BYTE b = (lightB != 0 && active != 0) ? 1 : 0;
    BYTE byte3 = pOut[3];
    BYTE v;
    BYTE b0;

    v = (byte3 & 0x7f) | (hb << 7);
    pOut[3] = v;
    b0 = ((pOut[0] ^ a) & 1) ^ pOut[0];
    pOut[0] = b0;
    pOut[0] = (b << 1) | (b0 & 0xfd);
    if (pIn[0] == 0) {
        v = (byte3 & 0x3f) | (hb << 7);
        pOut[3] = v;
        byte3 = pIn[1];
    } else {
        v |= 0x40;
        pOut[3] = v;
        byte3 = pIn[0];
    }
    pOut[3] = ((byte3 ^ v) & 0x3f) ^ v;
    pOut[0] = (pIn[2] << 2) | (b << 1) | (b0 & 1);
    pOut[1] = (pIn[3] << 2) | (pOut[1] & 3);
    pOut[2] = (pIn[8] << 7) | (pOut[2] & 0x7f);
    pOut[1] = (((pIn[4] + 1) ^ pOut[1]) & 3) ^ pOut[1];
    v = ((pIn[0xc] & 1) << 6) | (pOut[2] & 0xbf);
    pOut[2] = (((v + 1) ^ v) & 0x3f) ^ v;
}

// Queued stage event draws (area, x, y, rows).
struct EventDraw {
    EventRec *pEvent;
    char x;
    char y;
    char rows;
    BYTE pad;
};
// GLOBAL: CMR2 0x00589010
EventDraw g_eventDraws[64];

// Stamps the pending event draws into one quadrant of the event texture and
// clears the pending list.
// match 37%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046e780
void FUN_0046e780(void)
{
    RECT rect;
    short *pPos;
    int i;
    int j;
    int k;
    int x, y;
    int n;
    int idx;
    BYTE c;
    BYTE colour[4];

    if (g_eventCount < 1)
        return;
    if (g_unk0x00589318 < 1)
        return;
    colour[0] = 0x2f;
    colour[1] = 0x27;
    colour[2] = 0x14;
    colour[3] = 0xc0;
    g_unk0x00589334 = g_unk0x00589334 + 1;
    if (g_unk0x00589334 > 3)
        g_unk0x00589334 = 0;
    switch (g_unk0x00589334) {
    case 0:
        rect.left = 0;
        rect.top = 0;
        rect.right = *(short *)((BYTE *)g_eventTexture + 0x120) / 2 - 1;
        rect.bottom = *(short *)((BYTE *)g_eventTexture + 0x122) / 2 - 1;
        break;
    case 1:
        rect.left = *(short *)((BYTE *)g_eventTexture + 0x120) / 2 - 1;
        rect.top = 0;
        rect.right = *(short *)((BYTE *)g_eventTexture + 0x120) - 1;
        rect.bottom = *(short *)((BYTE *)g_eventTexture + 0x122) / 2 - 1;
        break;
    case 2:
        rect.left = 0;
        rect.top = *(short *)((BYTE *)g_eventTexture + 0x122) / 2 - 1;
        rect.right = *(short *)((BYTE *)g_eventTexture + 0x120) / 2 - 1;
        rect.bottom = *(short *)((BYTE *)g_eventTexture + 0x122) - 1;
        break;
    default:
        rect.left = *(short *)((BYTE *)g_eventTexture + 0x120) / 2 - 1;
        rect.top = *(short *)((BYTE *)g_eventTexture + 0x122) / 2 - 1;
        rect.right = *(short *)((BYTE *)g_eventTexture + 0x120) - 1;
        rect.bottom = *(short *)((BYTE *)g_eventTexture + 0x122) - 1;
        break;
    }
    CGraphics::LockTexture(g_eventTexture, &rect);
    i = 0;
    if (g_unk0x00589318 > 0) {
        do {
            j = 0;
            pPos = (short *)g_eventDraws[i].pEvent;
            x = (BYTE)g_eventDraws[i].x + pPos[0];
            y = (BYTE)g_eventDraws[i].y + pPos[1];
            if (CGameInfo::FUN_00405d10() == 0) {
                x = x * 4;
                y = y * 4;
            }
            idx = rand() % 4;
            n = CGameInfo::FUN_00405d10() ? 4 : 0x10;
            if (n > 0) {
                do {
                    k = 0;
                    do {
                        if (CGameInfo::FUN_00405d10() == 0)
                            c = g_unk0x0051c280[(idx * 0x10 + j) * 0x10 + k];
                        else
                            c = g_unk0x0051c240[(j + idx * 4) * 4 + k];
                        if (c != 0) {
                            if (rect.left < x + j && x + j < rect.right && rect.top < y + k &&
                                y + k < rect.bottom)
                                CGraphics::BlendPixel(g_eventTexture, x + j - rect.left,
                                                      y + k - rect.top, colour);
                        }
                        k++;
                    } while (k < n);
                    j++;
                } while (j < n);
            }
            i = i + 1;
            g_eventsDirty = 1;
        } while ((short)i < g_unk0x00589318);
    }
    CGraphics::UnlockTexture(g_eventTexture);
    g_unk0x00589318 = 0;
}

BYTE Events_Tick(int index);

// Queues a draw of event `index` at (x, y) when it ticks and is on the area;
// pauses it once it has been drawn `range` times.
// match 32%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046ec40
void FUN_0046ec40(int index, int x, int y)
{
    EventRec *p = &g_eventRecords[index];
    int over;

    if (index < g_eventCount && p->active != 0 && Events_Tick(index) && x >= 0 && y >= 0 &&
        x < p->a * 4 - 4 && y < p->b && g_unk0x00589318 < 0x40 && p != NULL) {
        g_eventDraws[g_unk0x00589318].pEvent = p;
        g_eventDraws[g_unk0x00589318].x = (char)x;
        over = y - p->b + 4;
        g_eventDraws[g_unk0x00589318].y = (char)y;
        if (over < 1)
            g_eventDraws[g_unk0x00589318].rows = 4;
        else
            g_eventDraws[g_unk0x00589318].rows = 4 - (char)over;
        if ((unsigned short)p->range <= (unsigned short)p->field_0x16) {
            p->paused = 1;
            g_unk0x00589318++;
            return;
        }
        g_unk0x00589318++;
        p->field_0x16++;
    }
}

// Sets the screen rectangles of the views from the screen size (full, top,
// bottom, left and right halves).
// FUNCTION: CMR2 0x00464b60
void FUN_00464b60(void)
{
    short *pFull = (short *)g_unk0x00548110[0];
    short *p = (short *)g_unk0x0051b9f0;
    int *pSize = (int *)g_pGraphics;
    int half;

    pFull[0] = 0;
    pFull[1] = 0;
    pFull[2] = (short)pSize[0];
    pFull[3] = (short)pSize[1];
    p[0] = 0;
    p[1] = 0;
    p[2] = (short)pSize[0];
    p[3] = (short)pSize[1];
    p[4] = 0;
    p[5] = 0;
    p[6] = (short)pSize[0];
    half = pSize[1] / 2;
    p[8] = 0;
    p[7] = (short)half;
    p[9] = (short)(pSize[1] / 2);
    p[10] = (short)pSize[0];
    half = pSize[1] / 2;
    p[12] = 0;
    p[11] = (short)half;
    p[13] = 0;
    p[14] = (short)(pSize[0] / 2);
    p[15] = (short)pSize[1];
    half = pSize[0] / 2;
    p[17] = 0;
    p[16] = (short)half;
    p[18] = (short)(pSize[0] / 2);
    p[19] = (short)pSize[1];
}

#define FIXVEC_EQ(a, b) ((a).x == (b).x && (a).y == (b).y && (a).z == (b).z)

// Interpolates every stage object's matrix between its two keys and flags
// the ones that moved.
// match 68%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00471950
void FUN_00471950(int t)
{
    int i;
    BYTE *p;
    FixMatrix old;
    FixMatrix *pCurrent;

    for (i = 0; i < g_unk0x0058c924; i++) {
        p = (BYTE *)&g_unk0x005894e0[i];
        pCurrent = (FixMatrix *)(p + 0x88);
        old = *pCurrent;
        FixMatrix_Interpolate(pCurrent, (FixMatrix *)(p + 0x48), (FixMatrix *)(p + 8), t, t, t, 1);
        if (FIXVEC_EQ(old.right, pCurrent->right) && FIXVEC_EQ(old.up, pCurrent->up) &&
            FIXVEC_EQ(old.forward, pCurrent->forward) && FIXVEC_EQ(old.position, pCurrent->position))
            *(int *)(p + 0x124) = 0;
        else
            *(int *)(p + 0x124) = 1;
    }
}

int FUN_004b5320(void *pNode, int value);

// Copies each stage object's interpolated matrix onto its scene node and
// calls the node refresh when the key changed.
// FUNCTION: CMR2 0x00471a60
void FUN_00471a60(int param_1)
{
    int i;
    BYTE *p;

    i = 0;
    if (g_unk0x0058c924 > 0) {
        p = (BYTE *)g_unk0x005894e0;
        do {
            *(FixMatrix *)(*(int *)(p + 4) + 0x98) = *(FixMatrix *)(p + 0x88);
            if (*(int *)(p + 0x124) != 0) {
                *(FixMatrix *)(*(int *)(p + 4) + 0xd8) = *(FixMatrix *)(p + 0x88);
                *(int *)(p + 0x114) = FUN_004b5320((void *)*(int *)(p + 4), *(int *)(p + 0x114));
                *(int *)(p + 0x124) = 0;
            }
            i = i + 1;
            p = p + 0x128;
        } while (i < (int)(g_unk0x0058c924 & 0xff));
    }
}

char FUN_00420190(void);
void FUN_0043f570(Car *pCar);

// Seeds the stage's random numbers (unless replaying) and gives the computer
// cars their start revs by difficulty.
// FUNCTION: CMR2 0x0047c1e0
void FUN_0047c1e0(char replay, char restart)
{
    int i;
    int r;
    int v;
    int start;
    Car *pCar;

    if (replay == 0 || restart != 0)
        g_unk0x0058e26c = CMain::GetFrameTime();
    if (CGameInfo::FUN_00406320())
        g_unk0x0058e26c = 0;
    srand(g_unk0x0058e26c);
    rand();
    for (i = 0; i < (BYTE)FUN_00420190(); i++) {
        r = rand();
        switch (CGameInfo::FUN_00405d90()) {
        case 0:
            v = r * 2;
            break;
        case 1:
            v = r + 0x8000;
            break;
        case 2:
            if (i < 3)
                v = 0x10000;
            else
                v = r + 0x8000;
            break;
        default:
            v = 0;
        }
        if (r > 0x4ccc)
            start = 0;
        else
            start = 0x10000 - v % 10;
        if (i >= (int)(RallyDataState() & 0xff)) {
            pCar = Car_Get(i);
            pCar->field_0x7a4 = FixMul(start, pCar->field_0x794);
            if (replay != 0 && restart == 0)
                FUN_0043f570(pCar);
        }
    }
}

void FUN_004658e0(int index);

// Refreshes the skid trails of every car in the race order, skipping the
// replay-style modes where CGameInfo::FUN_00405cd0() returns 2.
// FUNCTION: CMR2 0x00465780
void FUN_00465780(int param_1)
{
    short s;
    int i;

    if (CGameInfo::FUN_00405cd0() != 2) {
        i = 0;
        s = Car_GetOrderCount();
        if (0 < s) {
            do {
                if (i < 8 && FUN_0046bd20(i, param_1) != 7)
                    FUN_004658e0(i);
                i = i + 1;
                s = Car_GetOrderCount();
            } while (i < s);
        }
    }
}

BYTE *FUN_00498570(int index);

// Exhaust points of each car (4 per car).
// GLOBAL: CMR2 0x00549c20
FixVector g_unk0x00549c20[8][4];

// Rebuilds a car's four exhaust points halfway between its body path points.
// match 67%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004657d0
void FUN_004657d0(int car)
{
    int off;
    int *pOut;
    int *pA;
    int *pB;
    FixVector d;

    if (car < 8 && FUN_00498570(car) != NULL) {
        pOut = &g_unk0x00549c20[car][0].y;
        for (off = 0x1c8; off < 0x1f8; off += 0xc) {
            pA = (int *)(FUN_00498570(car) - 0x30 + off);
            pB = (int *)(FUN_00498570(car) + off);
            d.x = pA[0] - pB[0];
            d.y = pA[1] - pB[1];
            d.z = pA[2] - pB[2];
            FixVecScale(&d, &d, 0x8000);
            d.x += pB[0];
            d.y += pB[1];
            d.z += pB[2];
            *(FixVector *)(pOut - 1) = d;
            pOut += 3;
            pOut[-3] += 0xccc;
        }
    }
}

BYTE FUN_0042b710(int index);
void FUN_0046bdc0(BYTE *pIn, BYTE *pOut, int active, int handbrake, int lightA, int lightB);

// Encodes a car's controls into a replay packet.
// FUNCTION: CMR2 0x0046c450
void FUN_0046c450(BYTE *pOut, BYTE car)
{
    Car *pCar = Car_Get(car);
    DeviceInfo *pDev = CInput::FUN_0049ead0((char)FUN_0042b710(pCar->field_0xb1a));

    FUN_0046bdc0((BYTE *)pCar + 0x1d0, pOut, pDev->field_0x0 == 3, *(int *)((BYTE *)pCar + 0xb88),
                 *(int *)((BYTE *)pCar + 0xb8c), *(int *)((BYTE *)pCar + 0xb90));
}

int FUN_0041f3a0(void);

// Screen rectangle of a view: full screen with one player, else the half
// for the split direction.
// FUNCTION: CMR2 0x00464b10
BYTE *FUN_00464b10(int view)
{
    FUN_00464b60();
    if ((BYTE)RallyDataState() != 1 && FUN_0041f3a0() == 0) {
        if (CGameInfo::FUN_00405dc0())
            return (BYTE *)&g_unk0x0051b9f0[1 + view];
        return (BYTE *)&g_unk0x0051b9f0[3 + view];
    }
    return (BYTE *)g_unk0x0051b9f0;
}

// Starts a fresh stage object session.
// FUNCTION: CMR2 0x0047bda0
void FUN_0047bda0(void)
{
    FUN_0047c5c0();
    FUN_0047c1b0();
    FUN_0047c1e0(0, 0);
    FUN_0047cc30();
}

// Starts replay mode with the selected restart flag.
// FUNCTION: CMR2 0x0047bdc0
void FUN_0047bdc0(char restart)
{
    FUN_0047c1e0(1, restart);
}

// Checks whether a replay packet agrees with current controls.
// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046cbe0
int FUN_0046cbe0(BYTE *packet, BYTE car)
{
    BYTE current[4];

    if ((packet[2] & 0x3f) == 0)
        return 1;
    if ((packet[2] & 0x3f) >= 0x3f)
        return 0;
    FUN_0046c450(current, car);
    return ((packet[0] ^ current[0]) & 0xfc) == 0 &&
           ((packet[1] ^ current[1]) & 0xfc) == 0 &&
           ((packet[3] ^ current[3]) & 0x7f) == 0 &&
           ((packet[1] ^ current[1]) & 3) == 0 &&
           ((packet[2] ^ current[2]) & 0xc0) == 0 &&
           ((packet[0] ^ current[0]) & 3) == 0 &&
           ((packet[3] ^ current[3]) & 0x80) == 0;
}

// Interpolates a stage object record between two frames.
// match 63%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00461710
void FUN_00461710(BYTE *out, BYTE *from, BYTE *to, int t)
{
    int i;

    for (i = 0x1c; i < 0x48; i += 4) {
        out[i] = (BYTE)FUN_004616c0(to[i], from[i], t);
        out[i + 1] = (BYTE)FUN_004616c0(to[i + 1], from[i + 1], t);
        out[i + 2] = (BYTE)FUN_004616c0(to[i + 2], from[i + 2], t);
        out[i + 3] = (BYTE)FUN_004616c0(to[i + 3], from[i + 3], t);
    }
    for (i = 0; i < 7; i++)
        ((int *)out)[i] = ((int *)from)[i] + FixMul(((int *)to)[i] - ((int *)from)[i], t);
    for (i = 0x48; i < 0x4e; i++)
        out[i] = (BYTE)FUN_004616c0(to[i], from[i], t);
}

#include <stdlib.h>

struct Unk0x004a3e20;
void FUN_004a3e20(Unk0x004a3e20 *pObject, int value);

// Finds the rev counter and digit textures for a player.
// match 73%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00477340
void FUN_00477340(int player)
{
    char base[260];
    unsigned int i;
    Texture *texture;

    for (i = 0; i < CGraphics::m_textureCount; i++) {
        texture = CGraphics::m_pTextureManager->textureBuffer[i];
        _splitpath(texture->name, CFrontend::m_stringDest, CFrontend::m_stringDest,
                   base, CFrontend::m_stringDest);
        sprintf(CFrontend::m_stringDest, (char *)CGenericFileLoader::StrUpperPolish((BYTE *)base));
        if (strcmp(CFrontend::m_stringDest, CGraphics::m_strSuffixREVCT) == 0) {
            *(Texture **)(g_stageBlock + 0x220 + player * 8) = texture;
            FUN_004a3e20((Unk0x004a3e20 *)texture, 2);
        }
        if (strcmp(CFrontend::m_stringDest, CGraphics::m_strSuffixDIGIT) == 0)
            *(Texture **)(g_stageBlock + 0x224 + player * 8) = texture;
    }
}

BYTE FUN_004918c0(void);

// Starts stage objects and registers their frame callback.
// FUNCTION: CMR2 0x0048ca70
void FUN_0048ca70(void)
{
    FUN_0047bda0();
    CGame::RegisterCallback(FUN_004918c0, NULL);
}

// Allocates and registers a replay buffer.
// match 54%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046c5a0
BYTE *FUN_0046c5a0(short frames, short samples, int type)
{
    BYTE *buffer;
    int recordSize;
    int extraSize;
    BYTE slot;

    if (*(BYTE *)&g_unk0x00588ec8 == 0) {
        CGame::RegisterCallback(Replay_FreeBuffers, NULL);
        *(BYTE *)&g_unk0x00588ec8 = 1;
    }
    if (g_unk0x00588d3c == 8)
        return NULL;
    recordSize = type == 0 ? 0x114c : 0x5c;
    extraSize = type == 2 ? 0x10 : 4;
    buffer = (BYTE *)CFileBuffer::AllocateLockedBuffer(0x110 + frames * 2 + frames * recordSize + samples * frames * extraSize);
    if (buffer == NULL)
        return NULL;
    *(int *)(buffer + 0x1c) = type;
    *(short *)(buffer + 0xfc) = frames;
    *(short *)(buffer + 0xfe) = samples;
    Replay_SetupPointers(buffer, 0);
    *(int *)(buffer + 4) = 0;
    *(int *)(buffer + 0xc) = 0;
    *(int *)(buffer + 0x14) = 0;
    *(int *)(buffer + 0x18) = 0;
    *(short *)(buffer + 0x100) = 0;
    for (slot = 0; slot < 8; slot++) {
        if (g_unk0x00588e80[slot] == NULL) {
            g_unk0x00588e80[slot] = buffer;
            break;
        }
    }
    g_unk0x00588d3c++;
    return buffer;
}

// Loads a replay buffer and validates its recorded dimensions.
// match 38%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046d2d0
BYTE *FUN_0046d2d0(char *path)
{
    DWORD size;
    BYTE *buffer;
    int frames;
    int samples;

    if (*(BYTE *)&g_unk0x00588ec8 == 0) {
        CGame::RegisterCallback(Replay_FreeBuffers, NULL);
        *(BYTE *)&g_unk0x00588ec8 = 1;
    }
    buffer = (BYTE *)CFileBuffer::GetGenericFileBuffer(path, 1);
    if (buffer == NULL) {
        buffer = (BYTE *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                                      path, NULL, &size, 0);
        g_unk0x00588d18[g_unk0x00588d14] = 1;
    } else {
        size = CGenericFileLoader::GetGenericFileSize();
        g_unk0x00588d18[g_unk0x00588d14] = 0;
    }
    if (buffer == NULL)
        return NULL;
    frames = *(short *)(buffer + 0xfc);
    samples = *(short *)(buffer + 0xfe);
    if (size != 0x110 + frames * (0x114e + samples * 4) &&
        size != 0x110 + frames * (0x5e + samples * 4) &&
        size != 0x110 + frames * (0x114e + samples * 16) &&
        size != 0x110 + frames * (0x5e + samples * 16)) {
        CFileBuffer::FreeGenericFileBuffer(buffer);
        return NULL;
    }
    Replay_SetupPointers(buffer, 1);
    g_unk0x00588ea0[g_unk0x00588d14] = buffer;
    g_unk0x00588d14++;
    return buffer;
}

void FixMatrix_RotateAboutRight(FixMatrix *pOut, unsigned short angle);

// GLOBAL: CMR2 0x0051c9b0
short g_unk0x0051c9b0 = 0x71;

// Builds a stage object's world matrix from its car and mount point.
// match 41%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004778b0
void FUN_004778b0(BYTE *object, int unused)
{
    int car = object[2];
    FixVector position = *(FixVector *)(g_stageBlock + 0xe0 + car * 36);
    FixMatrix orient;
    FixMatrix mount;
    FixMatrix combined;
    BYTE *node;
    BYTE *pCar;

    FixMatrix_Identity(&mount);
    FixMatrix_SetPosition(&position, &mount);
    FixMatrix_Identity(&orient);
    orient.right.x = 0;
    orient.right.y = 0;
    orient.right.z = -0x10000;
    orient.up.x = 0;
    orient.up.y = 0x10000;
    orient.up.z = 0;
    orient.forward.x = 0x10000;
    orient.forward.y = 0;
    orient.forward.z = 0;
    FixMatrix_RotateAboutRight(&orient, (unsigned short)g_unk0x0051c9b0);
    FixMatrix_SetPosition((FixVector *)(g_stageBlock + 0xe0 + car * 36), &orient);
    node = *(BYTE **)(g_stageBlock + 0x294 + car * 0x1c);
    FixMatrix_Multiply(&combined, &orient, (FixMatrix *)(node + 0x98));
    pCar = (BYTE *)Car_Get(car);
    FixMatrix_Multiply((FixMatrix *)(object + 8), &combined, *(FixMatrix **)(pCar + 0x754));
    *(int *)(object + 0x48) = 0x10000;
    *(int *)(object + 0x4c) = 0x1999;
    *(int *)(object + 0x50) = 0;
    *(int *)(object + 0x54) = 0xa000;
    *(int *)(object + 0x58) = 0;
    *(int *)(object + 0x5c) = 0x10000;
}

GenericFile *FUN_0041f500(void);
BYTE *FUN_00475a40(void);
void StageUI_DrawChampionshipBar(void);
// El prototipo real: devuelve int y el 5o parametro es BYTE (los llamadores de Game.cpp/RallyData.cpp
// y la definicion en Game.cpp:2575 coinciden en esa firma). Un prototipo viejo aqui generaba otro
// nombre manglado y rompia el enlace.
int FUN_0049d3f0(int, int, void *, int, BYTE);
int FUN_004b9380(unsigned int, unsigned int, unsigned int);
int RallyData_FUN_0040eeb0(void);
int *FUN_0040f050(int view);
void FUN_00428680(unsigned int player, short *pRect, int check);
struct Menu;
void Menu_CallCallback2(Menu *pMenu);

// GLOBAL: CMR2 0x0051c950
char g_strTempObj[] = "TEMP.OBJ";
// GLOBAL: CMR2 0x0051c95c
char g_strTempSht[] = "TEMP.SHT";

// Replaces the active replay buffer with a freshly allocated one.
// FUNCTION: CMR2 0x00465f60
void FUN_00465f60(int frames, int samples)
{
    g_unk0x00588758 = (int *)FUN_0046c5a0(frames, samples, 0);
    FUN_00465f80();
}

// Loads a replay buffer from a path, falling back to a 10-second default one.
// FUNCTION: CMR2 0x00465f90
void FUN_00465f90(char *path)
{
    g_unk0x00588758 = (int *)FUN_0046d2d0(path);
    if (g_unk0x00588758 == NULL)
        FUN_00465f60(1, 0x1d4c);
    FUN_00465f80();
}

// Loads the stage's TEMP.OBJ model into memory.
// FUNCTION: CMR2 0x00472830
void FUN_00472830(void)
{
    void *pObj;
    GenericFile *pFile;

    pObj = CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), g_strTempObj, NULL, NULL, 0);
    if (pObj != NULL) {
        pFile = FUN_0041f500();
        FUN_004b9380((unsigned int)pObj, RallyData_FUN_00411060(), (unsigned int)pFile);
    }
}

// Loads the stage's TEMP.SHT model into memory.
// FUNCTION: CMR2 0x00472870
void FUN_00472870(void)
{
    void *pObj;
    GenericFile *pFile;

    pObj = CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), g_strTempSht, NULL, NULL, 0);
    if (pObj != NULL) {
        pFile = FUN_0041f500();
        FUN_004b9380((unsigned int)pObj, RallyData_FUN_00411060(), (unsigned int)pFile);
    }
}

// Clears the screen and redraws the split bars and championship positions.
// FUNCTION: CMR2 0x004759d0
void FUN_004759d0(int unused1, int unused2)
{
    int i;

    CGraphics::ClearTarget();
    CGraphics::ClearZBuffer();
    StageUI_DrawChampionshipBar();
    Menu_CallCallback2((Menu *)FUN_00475a40());
    i = 0;
    if ((BYTE)RallyDataState() > 0) {
        do {
            FUN_00428680(i, (short *)FUN_00464b10(i), 0);
            i++;
        } while (i < (int)(RallyDataState() & 0xff));
    }
    FUN_0049d3f0(RallyData_FUN_00411060(), RallyData_FUN_0040eeb0(), (void *)FUN_0040f050(0), 0, 0);
    FUN_0049de40();
}

struct ObjectMatrix16 { int v[16]; };

// Builds a stage object's orientation matrix from a car and a node, flipping
// the right-hand column and rotating about the object's right axis.
// FUNCTION: CMR2 0x00477850
void FUN_00477850(int object, int *src)
{
    int *dst = (int *)(object + 8);

    *(ObjectMatrix16 *)dst = *(ObjectMatrix16 *)src;
    dst[0] = -src[8];
    *(int *)(object + 0xc) = -src[9];
    *(int *)(object + 0x10) = -src[10];
    *(int *)(object + 0x28) = src[0];
    *(int *)(object + 0x2c) = src[1];
    *(int *)(object + 0x30) = src[2];
    FixMatrix_RotateAboutRight((FixMatrix *)dst, (unsigned short)g_unk0x0051c9b0);
    FUN_004778b0((BYTE *)object, (int)src);
}

// Builds a wheel/damper orientation matrix from two scale factors.
// match 76%: FixVector temp slot order differs from the original (same logic)
// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00486fc0
void FUN_00486fc0(int *pMatrix, int *pOffset)
{
    FixVector u, t;
    int i;

    FixVecScale(&t, (FixVector *)&pMatrix[4], pMatrix[0]);
    FixVecScale(&u, (FixVector *)&pMatrix[7], pMatrix[1]);
    pMatrix[0xc] = t.x - u.x;
    pMatrix[0xd] = t.y - u.y;
    pMatrix[0xe] = t.z - u.z;
    pMatrix[0xf] = u.x + t.x;
    pMatrix[0x10] = u.y + t.y;
    pMatrix[0x11] = u.z + t.z;
    FixVecScale(&t, &t, -0x10000);
    pMatrix[0x12] = t.x - u.x;
    pMatrix[0x13] = t.y - u.y;
    pMatrix[0x14] = t.z - u.z;
    pMatrix[0x15] = u.x + t.x;
    pMatrix[0x16] = u.y + t.y;
    pMatrix[0x17] = u.z + t.z;
    for (i = 0; i < 4; i++) {
        pMatrix[0xc + i * 3] += pOffset[0];
        pMatrix[0xd + i * 3] += pOffset[1];
        pMatrix[0xe + i * 3] += pOffset[2];
        pMatrix[0xd + i * 3] = pMatrix[2];
    }
}

// Builds a sprite orientation matrix from a pair of 16-bit extents.
// FUNCTION: CMR2 0x00487c40
void FUN_00487c40(int *pMatrix, int param_2, int *pOffset)
{
    short *p = *(short **)(*(int *)(param_2 + 4) + 4);
    FixVector t, u;
    int i;

    if (pMatrix[10] == 0) {
        pMatrix[0] = (int)p[4] << 9;
        pMatrix[1] = (int)p[5] << 9;
        pMatrix[4] = (int)p[0] << 9;
        pMatrix[5] = 0;
        pMatrix[6] = (int)p[1] << 9;
        pMatrix[7] = (int)p[1] << 9;
        pMatrix[8] = 0;
        pMatrix[9] = p[0] * -0x200;
        pMatrix[2] = p[2] * 0x200 + pOffset[1];
        pMatrix[3] = p[3] * 0x200 + pOffset[1];
        pMatrix[0x25] = (int)pOffset;
        pMatrix[0x24] = 0;
        FixVecScale(&t, (FixVector *)&pMatrix[4], pMatrix[0]);
        FixVecScale(&u, (FixVector *)&pMatrix[7], pMatrix[1]);
        pMatrix[0xc] = t.x - u.x;
        pMatrix[0xe] = t.z - u.z;
        pMatrix[0xf] = u.x + t.x;
        pMatrix[0x11] = u.z + t.z;
        FixVecScale(&t, &t, -0x10000);
        pMatrix[0x12] = t.x - u.x;
        pMatrix[0x14] = t.z - u.z;
        pMatrix[0x15] = u.x + t.x;
        pMatrix[0x17] = u.z + t.z;
        for (i = 0; i < 4; i++) {
            pMatrix[0xc + i * 3] += pOffset[0];
            pMatrix[0xd + i * 3] = pMatrix[3];
            pMatrix[0xe + i * 3] += pOffset[2];
        }
        pMatrix[10] = 1;
    }
}

// Blends two colours according to a fade timer and writes the result.
// match 75%: colour blend block differs in scheduling/register use (same logic)
// match 74%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047f510
void FUN_0047f510(int param_1, BYTE *pOut, BYTE *pFrom, BYTE *pTo)
{
    int t = *(int *)(param_1 + 0x668);
    FixVector c;
    int v, r, g, b;

    if (t > 0xa0000) {
        if (*(int *)(param_1 + 0x930) != 0) {
            v = t - *(int *)(param_1 + 0x674);
            if (v > 0x50000) {
                *(DWORD *)pOut = *(DWORD *)pFrom;
                return;
            }
            if (v < -0x50000) {
                *(DWORD *)pOut = *(DWORD *)pTo;
                return;
            }
            v = FixMul(v, 0x1999) + 0x8000;
            c.x = (pFrom[0] << 16) - (pTo[0] << 16);
            c.y = (pFrom[1] << 16) - (pTo[1] << 16);
            c.z = (pFrom[2] << 16) - (pTo[2] << 16);
            FixVecScale(&c, &c, v);
            r = c.x + (pTo[0] << 16);
            g = c.y + (pTo[1] << 16);
            b = c.z + (pTo[2] << 16);
            if (r > 0xff0000)
                r = 0xff0000;
            if (g > 0xff0000)
                g = 0xff0000;
            if (b > 0xff0000)
                b = 0xff0000;
            pOut[0] = (BYTE)(r >> 16);
            pOut[1] = (BYTE)(g >> 16);
            pOut[2] = (BYTE)(b >> 16);
            pOut[3] = 0xff;
            return;
        }
        *(DWORD *)pOut = *(DWORD *)pFrom;
        return;
    }
    v = FixMul(t, 0x1999);
    if (v > 0x10000)
        v = 0x10000;
    else if (v < 0)
        v = 0;
    if (*(int *)(param_1 + 0x930) != 0) {
        c.x = pTo[0] << 16;
        c.y = pTo[1] << 16;
        c.z = pTo[2] << 16;
    } else {
        c.x = pFrom[0] << 16;
        c.y = pFrom[1] << 16;
        c.z = pFrom[2] << 16;
    }
    pOut[0] = (BYTE)(FixMul(c.x, v) >> 16);
    pOut[1] = (BYTE)(FixMul(c.y, v) >> 16);
    pOut[2] = (BYTE)(FixMul(c.z, v) >> 16);
    pOut[3] = 0xff;
}

// Sets per-car visibility bits used by the stage object renderer.
// match 72%: pairs of flag bytes are not scheduled in parallel like the original (same logic)
// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046b790
void FUN_0046b790(int type, int car, int index)
{
    BYTE bit = 1 << car;

    switch (type) {
    case 0:
        g_unk0x00588ba4[8] |= bit;
        g_unk0x00588ba4[9] |= bit;
        g_unk0x00588ba4[10] |= bit;
        g_unk0x00588ba4[11] |= bit;
    case 3:
        g_unk0x00588ba4[12] |= bit;
        g_unk0x00588ba4[13] |= bit;
        g_unk0x00588ba4[14] |= bit;
        break;
    case 1:
        g_unk0x00588ba4[8] |= bit;
        g_unk0x00588ba4[9] |= bit;
        g_unk0x00588ba4[10] |= bit;
        g_unk0x00588ba4[13] |= bit;
        g_unk0x00588ba4[14] |= bit;
        break;
    case 2:
        g_unk0x00588ba4[8] |= bit;
        g_unk0x00588ba4[9] |= bit;
        g_unk0x00588ba4[10] |= bit;
        return;
    case 4:
        g_unk0x00588ba4[12] |= bit;
        return;
    case 5:
        g_unk0x00588ba4[8] |= bit;
        break;
    case 8:
        g_unk0x00588ba4[8] |= bit;
        g_unk0x00588ba4[11] |= bit;
        return;
    default:
        return;
    }
    g_unk0x00588ba4[index] |= bit;
}

int FUN_0040b010(int index);
unsigned int FUN_00409cb0(int index);
unsigned int FUN_0040b1e0(int index);

// Resets the per-view flags of a car's body, wheel and extra nodes.
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046b8f0
void FUN_0046b8f0(Car *pCar)
{
    int node;

    *(int *)(g_unk0x00588ba4 + 8) = 0;
    *(short *)(g_unk0x00588ba4 + 12) = 0;
    g_unk0x00588ba4[14] = 0;
    g_unk0x00588ba4[pCar->field_0xb1a] = 0;
    FUN_0046b790(g_unk0x00588cd4[pCar->field_0xb1a * 2], 0, pCar->field_0xb1a);
    FUN_0046b790(g_unk0x00588cd4[pCar->field_0xb1a * 2 + 1], 1, pCar->field_0xb1a);
    pCar->pNode0x71c->field_0x17c = 0xff;
    SceneNode_SetViewMaskTree(pCar->pWheelNodes[0], g_unk0x00588ba4[9]);
    SceneNode_SetViewMaskTree(pCar->pWheelNodes[1], g_unk0x00588ba4[9]);
    SceneNode_SetViewMaskTree(pCar->pWheelNodes[2], g_unk0x00588ba4[9]);
    SceneNode_SetViewMaskTree(pCar->pWheelNodes[3], g_unk0x00588ba4[9]);
    SceneNode_SetViewMaskTree(pCar->pNode0x720, g_unk0x00588ba4[10]);
    node = (int)SceneNode_FindByType(pCar->pNode0x720, 6);
    if (node != 0)
        *(BYTE *)(node + 0x17c) = g_unk0x00588ba4[13];
    node = (int)SceneNode_FindByType(pCar->pNode0x720, 0xe);
    if (node != 0)
        *(BYTE *)(node + 0x17c) = g_unk0x00588ba4[14];
    if (pCar->pNode0x724 != NULL) {
        SceneNode_SetViewMaskTree(pCar->pNode0x724, g_unk0x00588ba4[12]);
        node = (int)SceneNode_FindByType(pCar->pNode0x724, 6);
        if (node != 0)
            *(BYTE *)(node + 0x17c) = g_unk0x00588ba4[13];
        node = (int)SceneNode_FindByType(pCar->pNode0x724, 0xe);
        if (node != 0)
            *(BYTE *)(node + 0x17c) = g_unk0x00588ba4[14];
    }
    if (pCar->pExtraNodes[0] != NULL)
        SceneNode_SetViewMaskTree(pCar->pExtraNodes[0], g_unk0x00588ba4[12]);
    if (pCar->pExtraNodes[1] != NULL)
        SceneNode_SetViewMaskTree(pCar->pExtraNodes[1], g_unk0x00588ba4[12]);
    if (pCar->pExtraNodes[2] != NULL)
        SceneNode_SetViewMaskTree(pCar->pExtraNodes[2], g_unk0x00588ba4[12]);
    if (pCar->pExtraNodes[3] != NULL)
        SceneNode_SetViewMaskTree(pCar->pExtraNodes[3], g_unk0x00588ba4[12]);
    if (*(int *)(g_stageBlock + 0x41c + pCar->field_0xb1a * 4) != 0)
        SceneNode_SetViewMaskTree(*(SceneNode **)(g_stageBlock + 0x41c + pCar->field_0xb1a * 4),
                                  g_unk0x00588ba4[11]);
    if (*(int *)(g_stageBlock + 0x2c0 + pCar->field_0xb1a * 4) != 0)
        *(BYTE *)(*(int *)(g_stageBlock + 0x2c0 + pCar->field_0xb1a * 4) + 0x17c) = g_unk0x00588ba4[8];
    if (*(int *)(g_stageBlock + 0x3fc + pCar->field_0xb1a * 4) != 0)
        *(BYTE *)(*(int *)(g_stageBlock + 0x3fc + pCar->field_0xb1a * 4) + 0x17c) = g_unk0x00588ba4[8];
    FUN_0046b6b0(pCar->pNode0x71c, 10);
    FUN_0046b6e0(pCar->pNode0x71c->pFirstChild, 10);
    FUN_0046b6b0(pCar->pNode0x720, 10);
    FUN_0046b6e0(pCar->pNode0x720->pFirstChild, 10);
}

// Rebuilds the per-wheel visibility values of the cars in race order.
// match 52%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046bb40
void FUN_0046bb40(void)
{
    int flags[2];
    short *pOrder;
    Car *pCar;
    int i;
    int v;
    int count;
    unsigned int swap;

    i = 0;
    do {
        switch (FUN_00422f50(i)) {
        case 1:
            flags[i] = 6;
            break;
        case 2:
            flags[i] = 5;
            break;
        case 3:
            flags[i] = 8;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            flags[i] = 1;
            break;
        default:
            flags[i] = 0;
        }
        i++;
    } while (i < 2);
    count = Car_GetOrderCount() - 1;
    if (-1 < (short)count) {
        pOrder = Car_GetOrder() + (short)count;
        count = (short)count + 1;
        do {
            pCar = Car_Get(*pOrder);
            swap = (*(unsigned int *)(*(int *)(*(int *)((BYTE *)pCar + 0x720) + 0xc) + 0x30) >>
                    0x12) & 1;
            i = 0;
            do {
                if ((FUN_00422fb0(i) & 0xff) == (unsigned int)pCar->field_0xb1a) {
                    v = flags[i];
                    if (v == 1) {
                        if (g_unk0x00588bb4[pCar->field_0xb1a] == 0) {
                            if (swap != 0)
                                v = 2;
                        } else if (swap == 0) {
                            v = 3;
                        } else {
                            v = 4;
                        }
                    }
                } else {
                    if ((BYTE)RallyDataState() <= 1 || StageObject_UsesExtendedMode() != 0) {
                        if (g_unk0x00588bb4[pCar->field_0xb1a] == 0) {
                            if (swap == 0)
                                v = 1;
                            else
                                v = 2;
                        } else if (swap == 0) {
                            v = 3;
                        } else {
                            v = 4;
                        }
                    } else {
                        v = 7;
                    }
                }
                FUN_0046b740(pCar->field_0xb1a, v, i);
                i++;
            } while (i < 2);
            pOrder--;
            count--;
        } while (count != 0);
    }
    if ((char)CGameInfo::FUN_00405e00() != 0) {
        for (i = 0; i < 7; i++) {
            if ((char)FUN_00409cb0(i) == 0 && (char)FUN_0040b1e0(i) != 0) {
                FUN_0046b740(FUN_0040b010(i), 7, 0);
                FUN_0046b740(FUN_0040b010(i), 7, 1);
            }
        }
    }
}

extern Car *g_collisionCar;

// Applies the fade-driven roll (about the node's current up axis) to the two
// scene nodes of car `index`'s stage object and advances its fade state machine.
// FUNCTION: CMR2 0x00476a40
void FUN_00476a40(int index)
{
    int off = index * 0x1c;
    SceneNode *pNode;
    FixVector axis;
    FixVector position;

    pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0xc);
    if (pNode != NULL) {
        SceneNode_SetRotation(pNode, *(FixAngles **)(g_unk0x0058d4f0 + off));
        pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0xc);
        axis = pNode->current.up;
        short angle = g_unk0x0058d4e0[index * 6];
        if (g_unk0x0058d2f0[index] != 0)
            angle = -angle;
        FixMatrix_FromAxisAngle(&g_unk0x0058d260, &axis, angle);
        pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0xc);
        position = pNode->current.position;
        pNode->current.position.x = 0;
        pNode->current.position.y = 0;
        pNode->current.position.z = 0;
        FixMatrix_Multiply(&pNode->current, &pNode->current, &g_unk0x0058d260);
        pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0xc);
        pNode->current.position = position;
    }
    pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0x10);
    if (pNode != NULL) {
        SceneNode_SetRotation(pNode, *(FixAngles **)(g_unk0x0058d4f0 + off + 0x14));
        pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0x10);
        axis = pNode->current.up;
        short *pNum = *(short **)(g_unk0x0058d4f0 + off + 0x18);
        short *pDen = *(short **)(g_unk0x0058d4f0 + off + 4);
        int angle;
        if (g_unk0x0058d2f0[index] != 0)
            angle = -(*pNum * g_unk0x0058d4e0[index * 6] / *pDen);
        else
            angle = *pNum * g_unk0x0058d4e0[index * 6] / *pDen;
        FixMatrix_FromAxisAngle(&g_unk0x0058d260, &axis, angle);
        pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0x10);
        position = pNode->current.position;
        pNode->current.position.x = 0;
        pNode->current.position.y = 0;
        pNode->current.position.z = 0;
        FixMatrix_Multiply(&pNode->current, &pNode->current, &g_unk0x0058d260);
        pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0x10);
        pNode->current.position = position;
    }
    FUN_00476c70(index);
}

// Steps the fade state machine of one stage object: 0 -> 2 -> 3 ramps the
// offset up to the record's limit and 1 ramps it back to zero, retriggering
// from the object's +0x54 field.
// FUNCTION: CMR2 0x00476c70
void FUN_00476c70(int index)
{
    short *pMax;

    if (FUN_00460bf0(index) == 1) {
        if (FUN_00460c10(index) > 0x3333 && g_unk0x0058d4d8[index * 3] == 0) {
            g_unk0x0058d4d8[index * 3] = 2;
            g_unk0x0058d4e0[index * 6 + 1] = rand() % 0xb + 0x2d;
        }
        if (FUN_00460c10(index) > 0x8000 && g_unk0x0058d4d8[index * 3] == 2) {
            g_unk0x0058d4d8[index * 3] = 3;
            g_unk0x0058d4e0[index * 6 + 1] = rand() % 0xb + 0x5b;
        }
        if (FUN_00460c10(index) < 0x1999 && g_unk0x0058d4d8[index * 3] == 2)
            g_unk0x0058d4d8[index * 3] = 1;
        if (FUN_00460c10(index) < 0x6666 && g_unk0x0058d4d8[index * 3] == 3) {
            g_unk0x0058d4d8[index * 3] = 2;
            g_unk0x0058d4e0[index * 6 + 1] = rand() % 0xb + 0x2d;
        }
    } else {
        g_unk0x0058d4d8[index * 3] = 0;
    }
    if (g_unk0x0058d4d8[index * 3] != 0) {
        if (g_unk0x0058d4d8[index * 3 + 1] != 0) {
            g_unk0x0058d4e0[index * 6] += g_unk0x0058d4e0[index * 6 + 1];
            pMax = *(short **)(g_unk0x0058d4f0 + index * 0x1c + 4);
            if (g_unk0x0058d4e0[index * 6] > *pMax) {
                g_unk0x0058d4d8[index * 3 + 1] = 0;
                g_unk0x0058d4e0[index * 6] = *pMax;
                return;
            }
        } else {
            g_unk0x0058d4e0[index * 6] -= g_unk0x0058d4e0[index * 6 + 1];
            if (g_unk0x0058d4e0[index * 6] < 0) {
                g_unk0x0058d4d8[index * 3 + 1] = 1;
                if (g_unk0x0058d4d8[index * 3] == 1)
                    g_unk0x0058d4d8[index * 3] = 0;
                g_unk0x0058d4e0[index * 6] = 0;
            }
        }
    }
}

// Updates per-wheel slip tables and damps the car's velocity.
// match 82%: below the 90% bar; kept as FUNCTION so reccmp measures it.
// FUNCTION: CMR2 0x0048df50
void FUN_0048df50(Car *param_1)
{
    int i;
    int off;
    int v;

    g_collisionCar = param_1;
    i = 0;
    off = 0xbbc;
    do {
        int a, b, d, u;

        v = *(int *)((BYTE *)g_collisionCar + 0x778);
        if (v > 0x10000)
            v = 0x10000;
        v = FixMul(v, 0x6666);
        a = *(int *)((BYTE *)g_collisionCar + i + 0x270);
        b = *(int *)((BYTE *)g_collisionCar + i + 0x278);
        if ((a < 0 ? -a : a) - (b < 0 ? -b : b) < 0)
            d = (b < 0 ? -b : b) - (a < 0 ? -a : a);
        else
            d = (a < 0 ? -a : a) - (b < 0 ? -b : b);
        u = FixMul((d % 0x401) << 6, v);
        if (*(int *)((BYTE *)g_collisionCar + off) == 0 ||
            *(int *)((BYTE *)g_collisionCar + off - 0x2c0) <= u) {
            *(int *)((BYTE *)g_collisionCar + off - 0x2c0) = u;
            *(int *)((BYTE *)g_collisionCar + off) = 1;
            *(int *)((BYTE *)g_collisionCar + i + 0x564) = 0;
            *(int *)((BYTE *)g_collisionCar + i + 0x568) = 0x10000;
            *(int *)((BYTE *)g_collisionCar + i + 0x56c) = 0;
        }
        off += 4;
        i += 0xc;
    } while (off < 0xbcc);
    FixVecScale((FixVector *)((BYTE *)g_collisionCar + 0x408), (FixVector *)((BYTE *)g_collisionCar + 0x408),
                0xf851);
}

// Builds a 3x4 matrix from three basis vectors plus a translation.
// match 43%: matrix combination ordering differs from the original
// match 43%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00487140
void FUN_00487140(int *param_1, int *param_2, int *param_3, int *param_4)
{
    int x = param_4[0];
    int y = param_4[1];
    int z = param_4[2];
    int a, b, c;
    int v7, v2, v8, v6;

    a = FixMul(param_3[0], x);
    b = FixMul(param_3[4], y);
    c = FixMul(param_3[8], z);
    param_1[0xc] = c + b + a;
    param_1[0xf] = (b - c) + a;
    param_1[0x15] = (b - c) - a;
    param_1[0x12] = (c - a) + b;
    a = FixMul(param_3[1], x);
    b = FixMul(param_3[5], y);
    c = FixMul(param_3[9], z);
    param_1[0xd] = c + b + a;
    param_1[0x10] = (b - c) + a;
    param_1[0x16] = (b - c) - a;
    param_1[0x13] = (c - a) + b;
    a = FixMul(param_3[2], x);
    b = FixMul(param_3[6], y);
    c = FixMul(param_3[10], z);
    v7 = c + b + a;
    param_1[0xe] = v7;
    v2 = (b - c) + a;
    v8 = (b - c) - a;
    v6 = (c - a) + b;
    param_1[0x17] = v8;
    param_1[0x14] = v6;
    param_1[3] = -param_1[0x12];
    param_1[4] = -param_1[0x13];
    param_1[5] = -v6;
    param_1[6] = -param_1[0xf];
    param_1[7] = -param_1[0x10];
    param_1[8] = -v2;
    param_1[9] = -param_1[0xc];
    param_1[10] = -param_1[0xd];
    param_1[0x11] = v2;
    param_1[0xb] = -v7;
    v8 = -v8;
    param_1[0] = -param_1[0x15];
    param_1[1] = -param_1[0x16];
    param_1[2] = v8;
    param_1[0] = -param_1[0x15] + param_2[0];
    param_1[1] = -param_1[0x16] + param_2[1];
    param_1[2] = v8 + param_2[2];
    param_1[3] = param_1[3] + param_2[0];
    param_1[4] = param_1[4] + param_2[1];
    param_1[5] = param_1[5] + param_2[2];
    param_1[6] = param_1[6] + param_2[0];
    param_1[7] = param_1[7] + param_2[1];
    param_1[8] = param_1[8] + param_2[2];
    param_1[9] = param_1[9] + param_2[0];
    param_1[10] = param_1[10] + param_2[1];
    param_1[0xb] = param_1[0xb] + param_2[2];
    param_1[0xc] = param_1[0xc] + param_2[0];
    param_1[0xd] = param_1[0xd] + param_2[1];
    param_1[0xe] = param_1[0xe] + param_2[2];
    param_1[0xf] = param_1[0xf] + param_2[0];
    param_1[0x10] = param_1[0x10] + param_2[1];
    param_1[0x11] = param_1[0x11] + param_2[2];
    param_1[0x12] = param_1[0x12] + param_2[0];
    param_1[0x13] = param_1[0x13] + param_2[1];
    param_1[0x14] = param_1[0x14] + param_2[2];
    param_1[0x15] = param_1[0x15] + param_2[0];
    param_1[0x16] = param_1[0x16] + param_2[1];
    param_1[0x17] = param_1[0x17] + param_2[2];
}

int Car_GetWheelSpeed(Car *pCar, BYTE wheel, int unit);
void FUN_00498ca0(char *pDesc, int *pValues, int *pOut);
void FUN_0046ed80(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6,
                  int param_7, int param_8);

// Spawns a dust/smoke puff at a randomised position relative to a wheel.
// match 48%: randomised offset evaluation order differs from the original
// match 47%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046ed80
void FUN_0046ed80(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6,
                  int param_7, int param_8)
{
    short s;
    int x1, x2, a, b, c;

    s = (short)(rand() % (param_8 / 2));
    rand();
    param_4 = (s * param_4) / (param_8 / 2);
    if (s <= 0x400) {
        unsigned int idx = (int)s & 0xfff;
        int conv = (int)(__int64)((double)param_5 * CGraphics::m_65536);
        int v = FixMul(conv, g_sinTable[idx]);
        if (v < 0)
            v = -v;
        param_5 = v >> 0x10;
    }
    a = rand() % (param_5 + 1);
    x2 = a * param_7 + param_3;
    b = (rand() % 0x11 - 8) / (rand() % 3 + 1);
    x1 = (param_4 - 0x10) * param_6 + param_2 + b;
    c = (rand() % 0x11 - 8) / (rand() % 3 + 1);
    FUN_0046ec40(param_1, x1, x2 + c);
}

// Emits skid/dust effects for the wheels that are slipping.
// match 49%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046ea80
void FUN_0046ea80(int param_1, int param_2)
{
    int i;

    i = 2;
    if (param_1 == 2 || param_1 == 3) {
        EventRec *p;
        if (g_eventCount > 2) {
            p = &g_eventRecords[2];
            do {
                int r1 = rand();
                short s = p->a;
                int r2 = rand();
                FUN_0046ec40(i, (r1 * s) / 0x7fff, (r2 * p->b) / 0x7fff);
                i++;
                p++;
            } while (i < g_eventCount);
        }
    } else {
        int e = (param_1 != 0) ? 1 : 0;
        int va = g_eventRecords[e].a;
        int vb = g_eventRecords[e].b;
        int vd = va / 5;
        int vf = va - vd;
        int vg = vb - 8;
        int x, y;
        if (param_2 < 0x8000)
            param_2 = 0;
        else if (param_2 > 0x18000)
            param_2 = 0x18000;
        x = (param_2 * (vf - vd)) / 0xb333;
        y = (param_2 * vg) / 0xb333;
        if (x > 0x20) {
            if (param_1 == 0) {
                if (g_unk0x00589331 != 0)
                    FUN_0046ed80(0, vd, vb - 1, x, y, 1, -1, 0x4fa);
                else
                    FUN_0046ed80(0, vf - 1, 1, x, y, -1, 1, 0x4fa);
            } else if (g_unk0x00589331 != 0) {
                FUN_0046ed80(1, vd, 1, x, y, 1, 1, 0x4fa);
            } else {
                FUN_0046ed80(1, vf - 1, vb - 1, x, y, -1, -1, 0x4fa);
            }
        }
    }
    g_unk0x00589320[param_1] = g_unk0x00589320[param_1] + 1;
}

// Spawns skid effects for all four wheels of a car.
// FUNCTION: CMR2 0x0046ea10
void FUN_0046ea10(int param_1)
{
    int i;

    if (g_eventCount > 0) {
        Car_Get(param_1);
        i = 0;
        do {
            if (FUN_0046eeb0(param_1, i) != 0) {
                int v = Car_GetWheelSpeed(Car_Get(param_1), (BYTE)i, 0);
                if (v > 0x1e0000) {
                    int n = 2;
                    do {
                        FUN_0046ea80(i, v / 0x3c);
                        n--;
                    } while (n != 0);
                }
            }
            i++;
        } while (i < 4);
    }
}

// GLOBAL: CMR2 0x0058e088
int g_unk0x0058e088[6];
// Views into the stage object pointer table: 0x58e3ac and 0x58e44c are its
// 7th and 47th entries, 0x58e4a0 is the last one.
#define g_unk0x0058e3ac ((char **)(g_unk0x0058e394 + 6))
#define g_unk0x0058e44c ((char **)(g_unk0x0058e394 + 46))
#define g_unk0x0058e4a0 ((int *)g_unk0x0058e394[67])

// Projects a stage object's rotation table into its output rows.
// match 23%: nested projection loops do not match the original layout
// match 23%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047ca30
void FUN_0047ca30(int param_1)
{
    char *pRow;
    int **ppData;
    int *pDst;
    int row;

    if (*(int *)(param_1 + 8) > 1) {
        pRow = (char *)(param_1 + 0xc);
        ppData = (int **)(param_1 + 0x14);
        pDst = (int *)g_unk0x0058e0b8;
        row = 0xc;
        do {
            int j = 1;
            if (pRow[1] > 0) {
                do {
                    int *pData;
                    int *pTable;
                    int sum;
                    int cols;
                    sum = 0;
                    pTable = pDst;
                    pData = *ppData;
                    cols = *pRow + 1;
                    if (cols > 0) {
                        do {
                            if (*pData != 0)
                                sum += FixMul(*pTable, *pData);
                            pTable++;
                            pData++;
                            cols--;
                        } while (cols != 0);
                    }
                    if (*(int *)(param_1 + 8) - 1 < (int)(pRow + (-0xb - param_1))) {
                        ((int *)g_unk0x0058e0b8)[row + j] = sum;
                    } else {
                        int idx = FixDiv(sum, 0x10000000);
                        if (idx < 0) {
                            if (-idx < 0x40)
                                ((int *)g_unk0x0058e0b8)[row + j] = -g_unk0x0058e4a0[-idx];
                            else
                                ((int *)g_unk0x0058e0b8)[row + j] = 0xffff0000;
                        } else if (idx < 0x40) {
                            ((int *)g_unk0x0058e0b8)[row + j] = g_unk0x0058e4a0[idx];
                        } else {
                            ((int *)g_unk0x0058e0b8)[row + j] = 0x10000;
                        }
                    }
                    j++;
                } while (j <= pRow[1]);
            }
            pDst += 0xc;
            row += 0xc;
            ppData++;
            pRow++;
        } while ((int)(pRow + (-0xb - param_1)) < *(int *)(param_1 + 8));
    }
}

// Builds a stage object's per-row output from its type tables.
// FUNCTION: CMR2 0x0047c9a0
void FUN_0047c9a0(int param_1, int param_2, int *param_3)
{
    unsigned int mask = 0;
    int i = 0;

    do {
        BYTE b = *(BYTE *)(g_unk0x0058e3ac[param_1] + 1 + i);
        int v = (int)(signed char)b;
        if (v != 0) {
            unsigned int bit = 1 << v;
            if ((mask & bit) == 0) {
                mask |= bit;
                FUN_00498ca0(g_unk0x0058e44c[v], param_3, (int *)g_unk0x0058e0b8);
                FUN_0047ca30((int)g_unk0x0058e44c[v]);
            }
            *(int *)(i + param_2) =
                *(int *)(&g_unk0x0058e088[(int)*(char *)(g_unk0x0058e3ac[param_1] + i) +
                                          *(int *)(g_unk0x0058e44c[v] + 8) * 0xc]);
        }
        i += 4;
    } while (i < 0x14);
}

int Track_GetGroundHeightSurface(FixVector *pPoint, FixVector *pNormal, short *pTri,
                                 short *pSurfaceClass, unsigned short *pSurface, int defaultY);

// Updates each wheel's suspension height against the ground.
// match 57%: short loop counter and clamp block differ from the original
// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004930e0
void FUN_004930e0(int param_1, int param_2)
{
    int *p;
    short i;
    short local_8;

    p = FUN_00469680((int)*(char *)(param_1 + 0xb1a));
    i = 0;
    local_8 = 0;
    if (param_2 > 0) {
        do {
            int v = Track_GetGroundHeightSurface(
                (FixVector *)(param_1 + (i * 3 + 0x9c) * 4),
                (FixVector *)(param_1 + (i * 3 + 0x129) * 4),
                (short *)(param_1 + 0xa9e + i * 2),
                (short *)(param_1 + 0xaae + i * 2),
                (unsigned short *)(param_1 + 0xac6 + i * 2),
                *(int *)(param_1 + 0x8dc + i * 4));
            *(int *)(param_1 + 0x8dc + i * 4) = v;
            if (*(short *)(param_1 + 0xa9e + i * 2) == -1)
                local_8 = local_8 + 1;
            if (i < 4) {
                int t = *(int *)(param_1 + 0x978 + i * 4) + *(int *)((int)p + 600 + i * 4);
                if (t > 0x10000)
                    t = 0x10000;
                *(int *)(param_1 + 0x8dc + i * 4) += FixMul(t, *(int *)(param_1 + 0x938 + i * 4));
            }
            if (*(int *)(param_1 + 0xbbc + i * 4) != 0) {
                int *src;
                *(int *)(param_1 + 0x8dc + i * 4) += *(int *)(param_1 + 0x8fc + i * 4);
                src = (int *)(param_1 + (i * 3 + 0x159) * 4);
                ((int *)(param_1 + (i * 3 + 0x129) * 4))[0] = src[0];
                ((int *)(param_1 + (i * 3 + 0x129) * 4))[1] = src[1];
                ((int *)(param_1 + (i * 3 + 0x129) * 4))[2] = src[2];
                *(short *)(param_1 + 0xaae + i * 2) = 0x2f;
            }
            if (*(short *)(param_1 + 0xaae + i * 2) == 0xf &&
                *(int *)(param_1 + 0xa7c) == 0 && *(int *)(param_1 + 0xbf8) == 0) {
                *(int *)(param_1 + 0xa7c) = 0x190000;
            }
            if (i < 4)
                *(int *)(param_1 + 0x8dc + i * 4) -= *(int *)(param_1 + 0x700 + i * 8);
            i++;
        } while ((int)i < param_2);
    }
    if ((short)param_2 < 8) {
        int n = 8 - (short)param_2;
        int *pDst = (int *)(param_1 + 0x8dc + (short)param_2 * 4);
        do {
            n--;
            *pDst = pDst[-4] - 0x50000;
            pDst++;
        } while (n != 0);
    }
    if (local_8 != param_2)
        *(int *)(param_1 + 0xa80) = 0;
    else
        *(int *)(param_1 + 0xa80) = *(int *)(param_1 + 0xa80) + 0x10000;
}

// Views into g_stageBlock for the object fade tables at 0x58d2d0/0x58d360/0x58d478.
#define g_unk0x0058d2d0 ((BYTE *)(g_stageBlock + 0x30))
#define g_unk0x0058d360 ((int *)(g_stageBlock + 0xc0))
#define g_unk0x0058d478 ((BYTE *)(g_stageBlock + 0x1d8))

// Fades a car's stage object in and out as its body state changes.
// match 83%: fade state machine branches differ from the original
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00476850
int FUN_00476850(int param_1, int param_2)
{
    int result = 0;

    if (g_unk0x0058d360[param_1] == 1 &&
        ((unsigned int)g_unk0x0058d2d0[param_1] != (int)*(char *)(param_2 + 0xb20) ||
         *(int *)(param_2 + 0x1d8) != 0)) {
        g_unk0x0058d2d0[param_1] = *(char *)(param_2 + 0xb20);
        g_unk0x0058d360[param_1] = 2;
        g_unk0x0058d478[param_1] = 0;
    }
    if (g_unk0x0058d360[param_1] == 2) {
        int local_c = (int)(__int64)((double)(BYTE)g_unk0x0058d478[param_1] * CGraphics::m_65536);
        BYTE c;
        result = FixDiv(local_c, 0x70000);
        g_unk0x0058d2d0[param_1] = *(char *)(param_2 + 0xb20);
        c = g_unk0x0058d478[param_1];
        g_unk0x0058d478[param_1] = c + 1;
        if ((BYTE)(c + 1) > 7) {
            g_unk0x0058d360[param_1] = 3;
            g_unk0x0058d478[param_1] = 0;
        }
    }
    if (g_unk0x0058d360[param_1] == 3) {
        result = 0x10000;
        g_unk0x0058d478[param_1] = g_unk0x0058d478[param_1] + 1;
        if ((unsigned int)g_unk0x0058d2d0[param_1] != (int)*(char *)(param_2 + 0xb20) ||
            *(int *)(param_2 + 0x1d8) != 0) {
            g_unk0x0058d478[param_1] = 0;
            g_unk0x0058d2d0[param_1] = *(char *)(param_2 + 0xb20);
        }
        if ((BYTE)g_unk0x0058d478[param_1] > 3) {
            g_unk0x0058d360[param_1] = 4;
            g_unk0x0058d478[param_1] = 0;
        }
    }
    if (g_unk0x0058d360[param_1] == 4) {
        int local_c = (int)(__int64)((double)(BYTE)g_unk0x0058d478[param_1] * CGraphics::m_65536);
        result = 0x10000 - FixDiv(local_c, 0x70000);
        if ((unsigned int)g_unk0x0058d2d0[param_1] == (int)*(char *)(param_2 + 0xb20) &&
            *(int *)(param_2 + 0x1d8) == 0) {
            BYTE c = g_unk0x0058d478[param_1];
            g_unk0x0058d478[param_1] = c + 1;
            if ((BYTE)(c + 1) > 7) {
                g_unk0x0058d360[param_1] = 1;
                g_unk0x0058d478[param_1] = 0;
                return 0;
            }
        } else {
            BYTE c = g_unk0x0058d478[param_1];
            g_unk0x0058d2d0[param_1] = *(char *)(param_2 + 0xb20);
            g_unk0x0058d360[param_1] = 2;
            g_unk0x0058d478[param_1] = 7 - c;
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// TEMPORARY link scaffolding: callees that are not decompiled yet. Their
// signatures come from the original's `ret N`; the bodies are empty so the
// calls sites compile and reccmp can measure the callers.
int FUN_004b50b0(SceneNode *pNode, int param_2);
void FUN_004a3240(int unused);

// Runs FUN_004b50b0 on a node, then FUN_004a3240; returns the first result.
// FUNCTION: CMR2 0x004b5320
int FUN_004b5320(void *pNode, int value)
{
    int result;

    result = FUN_004b50b0((SceneNode *)pNode, value);
    FUN_004a3240((int)pNode);
    return result;
}


extern Mesh *g_stageMesh1Copy;
extern Mesh *g_stageMesh3Copy;
extern short g_stageMesh1Count;
extern short g_stageMesh3Count;
extern FixVector g_stageRangeOrigin;
extern BYTE g_unk0x00592146;
extern BYTE g_stageColourAlpha;
extern int g_stageColourMode;
extern int g_stageColourStep;
// Byte colour r, g, b, a -> 0xAARRGGBB
struct StageRGBA {
    BYTE r, g, b, a;
};
#define STAGE_ARGB(c) (((((DWORD)(c).a << 8 | (c).r) << 8 | (c).g) << 8) | (c).b)

// Colours the ground mesh: the vertex at the reference point gets the
// reference colour, every other one the plain colour.
// FUNCTION: CMR2 0x004920d0
void FUN_004920d0(DWORD *pColour, DWORD *pReference)
{
    StageRGBA colour;
    StageRGBA reference;
    int i;
    float *vertex;

    colour = *(StageRGBA *)pColour;
    reference = *(StageRGBA *)pReference;
    for (i = g_stageMesh1Count - 1; i >= 0; i--) {
        vertex = (float *)((BYTE *)g_stageMesh1Copy->pVertexData + i * 0x30);
        if (g_stageRangeOrigin.x == (int)(__int64)(vertex[0] * CGraphics::m_65536) &&
            g_stageRangeOrigin.y == (int)(__int64)(vertex[1] * CGraphics::m_65536) &&
            g_stageRangeOrigin.z == (int)(__int64)(vertex[2] * CGraphics::m_65536))
            *(DWORD *)((BYTE *)vertex + 0x18) = STAGE_ARGB(reference);
        else
            *(DWORD *)((BYTE *)vertex + 0x18) = STAGE_ARGB(colour);
        *(DWORD *)((BYTE *)g_stageMesh1Copy->pVertexData + i * 0x30 + 0x1c) = (DWORD)g_unk0x00592146 << 24;
    }
    g_stageColourMode = 1;
}

// Colours the object meshes the same way (reference point at 0x5920fc), and
// the whole of the third mesh in the plain colour.
// FUNCTION: CMR2 0x00492220
void FUN_00492220(DWORD *pColour, DWORD *pReference)
{
    StageRGBA colour;
    StageRGBA reference;
    int i;
    float *vertex;

    colour = *(StageRGBA *)pColour;
    reference = *(StageRGBA *)pReference;
    for (i = g_stageMesh2Count - 1; i >= 0; i--) {
        vertex = (float *)((BYTE *)g_stageMesh2Copy->pVertexData + i * 0x30);
        if (g_unk0x005920fc == (int)(__int64)(vertex[0] * CGraphics::m_65536) &&
            g_unk0x00592100 == (int)(__int64)(vertex[1] * CGraphics::m_65536) &&
            g_unk0x00592104 == (int)(__int64)(vertex[2] * CGraphics::m_65536))
            *(DWORD *)((BYTE *)vertex + 0x18) = STAGE_ARGB(reference);
        else
            *(DWORD *)((BYTE *)vertex + 0x18) = STAGE_ARGB(colour);
        *(DWORD *)((BYTE *)g_stageMesh2Copy->pVertexData + i * 0x30 + 0x1c) = (DWORD)g_stageColourAlpha << 24;
    }
    if (g_stageMesh3Copy != NULL) {
        for (i = g_stageMesh3Count - 1; i >= 0; i--) {
            *(DWORD *)((BYTE *)g_stageMesh3Copy->pVertexData + i * 0x30 + 0x18) = STAGE_ARGB(colour);
            *(DWORD *)((BYTE *)g_stageMesh3Copy->pVertexData + i * 0x30 + 0x1c) = (DWORD)g_stageColourAlpha << 24;
        }
    }
    g_stageColourStep = 1;
}

extern const double g_oneOver180;  // defined in StageTiming.cpp
extern const double g_pi;

// Yaws the stage's main object to `angle` (16.16 degrees; -999 keeps it
// square) and the sun objects to `sunAngle`, by rewriting the right and
// forward axes of their matrices.
// match 33%: same operations and multiply order as the original; MSVC schedules the
// FixMatrix_SetRight pushes before the fsin there, which throws the diff alignment off.
// FUNCTION: CMR2 0x004926f0
void FUN_004926f0(int angle, int unused, int sunAngle)
{
    int pObject;
    int pSun;
    int pSun2;
    FixVector right;
    FixVector forward;
    double radians;

    FUN_0046f4c0(&pObject);
    if (angle != -999 << 16) {
        right.y = 0;
        forward.y = 0;
        radians = angle * g_oneOver180 * g_pi * CGraphics::m_oneOver65536;
        forward.z = right.x = (int)(__int64)(cos(radians) * CGraphics::m_65536);
        right.z = (int)(__int64)(sin(radians) * CGraphics::m_65536);
        forward.x = -right.z;
        FixMatrix_SetRight(&right, (FixMatrix *)(pObject + 0x98));
    } else {
        right.x = 0x10000;
        forward.z = 0x10000;
        right.y = 0;
        right.z = 0;
        forward.x = 0;
        forward.y = 0;
        FixMatrix_SetRight(&right, (FixMatrix *)(pObject + 0x98));
    }
    FixMatrix_SetForward(&forward, (FixMatrix *)(pObject + 0x98));
    FUN_0046f4e0(&pSun, &pSun2);
    right.y = 0;
    forward.y = 0;
    radians = sunAngle * g_oneOver180 * g_pi * CGraphics::m_oneOver65536;
    forward.z = right.x = (int)(__int64)(cos(radians) * CGraphics::m_65536);
    right.z = (int)(__int64)(sin(radians) * CGraphics::m_65536);
    forward.x = -right.z;
    FixMatrix_SetRight(&right, (FixMatrix *)(pSun + 0x98));
    FixMatrix_SetForward(&forward, (FixMatrix *)(pSun + 0x98));
    if (pSun2 != 0) {
        FixMatrix_SetRight(&right, (FixMatrix *)(pSun2 + 0x98));
        FixMatrix_SetForward(&forward, (FixMatrix *)(pSun2 + 0x98));
    }
}

extern SceneNode *g_stageLightNode;
extern SceneNode *g_stageLightRoot;

// Turns the two stage light nodes to face the view: their right, forward and
// up axes become the view's right, up and -forward, scaled to 0xbc6.
// FUNCTION: CMR2 0x00492bd0
void FUN_00492bd0(int view)
{
    FixMatrix *pView;
    FixVector axis;

    if (g_stageLightNode != NULL) {
        pView = &g_viewNodes[view]->current;
        FixMatrix_GetRight(&axis, pView);
        FixVecScale(&axis, &axis, 0xbc6);
        FixMatrix_SetRight(&axis, &g_stageLightNode->current);
        FixMatrix_GetUp(&axis, pView);
        FixVecScale(&axis, &axis, 0xbc6);
        FixMatrix_SetForward(&axis, &g_stageLightNode->current);
        FixMatrix_GetForward(&axis, pView);
        FixVecScale(&axis, &axis, -0xbc6);
        FixMatrix_SetUp(&axis, &g_stageLightNode->current);
    }
    if (g_stageLightRoot != NULL) {
        pView = &g_viewNodes[view]->current;
        FixMatrix_GetRight(&axis, pView);
        FixVecScale(&axis, &axis, 0xbc6);
        FixMatrix_SetRight(&axis, &g_stageLightRoot->current);
        FixMatrix_GetUp(&axis, pView);
        FixVecScale(&axis, &axis, 0xbc6);
        FixMatrix_SetForward(&axis, &g_stageLightRoot->current);
        FixMatrix_GetForward(&axis, pView);
        FixVecScale(&axis, &axis, -0xbc6);
        FixMatrix_SetUp(&axis, &g_stageLightRoot->current);
    }
}

void FUN_0042b800(int, int, int);
void FUN_0045e610(void);
void FUN_004702a0(void);
extern int g_unk0x0067f228;

short Car_GetOrderCount(void);
SceneNode *SceneNode_FindByType(SceneNode *pNode, unsigned int type);
void FUN_00486910(BYTE *pObj, int *pSrc);

extern BYTE g_unk0x00590ec0[16];

// Picks the scene node of a car's object payload by the payload type letter
// ('C' or 'A' select the 9-mode node, anything else the 5-mode one) and stores
// it in both per-object tables, then copies the payload into the object.
// FUNCTION: CMR2 0x00486740
void FUN_00486740(BYTE *pObj, int *pSrc, BYTE index, BYTE value)
{
    g_unk0x00590d8c[*pObj] = value;
    g_unk0x00590ec0[*pObj] = index;
    if ((short)index < Car_GetOrderCount() && Car_Get(index)->pNode0x720 != NULL) {
        if (FUN_00456be0(index)[0x20] == 'C' || FUN_00456be0(index)[0x20] == 'A')
            g_stageBlock_58d340[index] =
                (int)SceneNode_FindByType(Car_Get(index)->pNode0x720, 9);
        else
            g_stageBlock_58d340[index] =
                (int)SceneNode_FindByType(Car_Get(index)->pNode0x720, 5);
        g_stageBlock_58d47c[index] = (int)SceneNode_FindByType(Car_Get(index)->pNode0x720, 5);
    }
    FUN_00486910(pObj, pSrc);
}

// Sets the hit flag of one entry of a car's timing record and refreshes the
// derived record block.
// FUNCTION: CMR2 0x00469bf0
void FUN_00469bf0(Car *pCar, int index)
{
    *(int *)(g_unk0x00588b94 + 0x4b0 + (index + pCar->field_0xb1a * 0x134) * 4) = 1;
    FUN_00468c10(pCar);
}

// Saves the car's torque state into pState and copies the current race record
// block into the following slot of pState.
// FUNCTION: CMR2 0x0046c320
void FUN_0046c320(int *pState, BYTE car)
{
    Car *pCar = Car_Get(car);

    *pState = pCar->field_0x7a4;
    FUN_0042b800(car, 1, 1);
    pCar->field_0x7a4 = *pState;
    if (pCar->field_0xb48 != 1)
        FUN_0043f570(pCar);
    pState = pState + 1;
    pCar->field_0xb9c = 1;
    FUN_0046c220((Block6 *)RallyData_FUN_00421510(car), (Block6 *)pState);
    RallyData_FUN_004207a0(car);
}

// Restores the car's torque state from pState, copies pState's race record
// block back into the car record and revalidates the stage state.
// FUNCTION: CMR2 0x0046c390
void FUN_0046c390(int *pState, BYTE car)
{
    Car *pCar = Car_Get(car);

    FUN_0042b800(car, 1, 1);
    pCar->field_0x7a4 = *pState;
    if (pCar->field_0xb48 != 1)
        FUN_0043f570(pCar);
    pCar->field_0xb9c = 1;
    FUN_0046c220((Block6 *)(pState + 1), (Block6 *)RallyData_FUN_00421510(car));
    RallyData_FUN_004207a0(car);
    FUN_0045e610();
    FUN_004702a0();
    RallyData_ValidateIndex(car);
}

// GLOBAL: CMR2 0x0058ca74
int g_unk0x0058ca74;
// GLOBAL: CMR2 0x0058ca68
int g_unk0x0058ca68;
// GLOBAL: CMR2 0x0058c934
int g_unk0x0058c934;
// GLOBAL: CMR2 0x0058c950
int g_unk0x0058c950;

// Saves the scene-node and render-object counts around loading the two stage
// model variants (TEMP.OBJ and TEMP.SHT).
// FUNCTION: CMR2 0x00471af0
void FUN_00471af0(void)
{
    g_unk0x0058ca74 = g_sceneNodeCount;
    g_unk0x0058ca68 = g_unk0x0067f228;
    FUN_00472830();
    g_unk0x0058c934 = g_sceneNodeCount;
    g_unk0x0058c950 = g_unk0x0067f228;
    FUN_00472870();
    FUN_0046f060();
}

BYTE *FUN_0041b390(void);
BYTE FUN_0041b370(void);
int FUN_0041b380(void);
int FUN_004232a0(BYTE index, int mode);
int RallyData_FUN_00408800(BYTE index);
void FUN_00421720(unsigned char, int, int, unsigned char, int);

// GLOBAL: CMR2 0x0051f4c0
unsigned int g_unk0x0051f4c0 = 0x100;
// GLOBAL: CMR2 0x0058df98
unsigned int g_unk0x0058df98;
// GLOBAL: CMR2 0x0058df9c
unsigned int g_unk0x0058df9c;
// GLOBAL: CMR2 0x0058e0a4
unsigned int g_unk0x0058e0a4;

// Driver-camera cycle: while the cycle key is held the active driver is
// advanced (or, on the championship round screen, picked from the round
// drivers) and the requested view mode is applied to the car.
// FUNCTION: CMR2 0x0047bad0
void FUN_0047bad0(unsigned int param_1, unsigned int param_2)
{
    int mode;

    if (*(char *)(*(int *)(FUN_0041b390() + 4) + param_2 * 8) == 7 ||
        *(char *)(*(int *)(FUN_0041b390() + 4) + param_2 * 8) == 8) {
        if ((param_1 & (g_unk0x0051f4c0 & 0xffff)) != 0) {
            if (FUN_00422f50(param_2) != 10)
                FUN_00421720(g_unk0x0058e0a0->field_0xb1a, 10, 0xffff,
                             FUN_00422fb0(g_unk0x0058e0a0->field_0xb1a), 0);
        }
        if ((param_1 & (g_unk0x0051f4c0 & 0xffff)) == 0) {
            if (FUN_00422f50(g_unk0x0058e0a0->field_0xb1a) == 10) {
                switch (FUN_0041b380()) {
                case 4:
                    g_unk0x0058e0a4 = (FUN_0041b370() & 0xff) + param_2;
                    break;
                case 0:
                case 1:
                    g_unk0x0058e0a4 = param_2;
                    break;
                case 2:
                    RallyData_GetRoundDrivers(&g_unk0x0058df98, &g_unk0x0058df9c);
                    if (RallyData_FUN_00408500(g_unk0x0058df98 & 0xff) == -1)
                        g_unk0x0058e0a4 = g_unk0x0058df98;
                    else
                        g_unk0x0058e0a4 = g_unk0x0058df9c;
                    break;
                case 3:
                    if (param_2 == 0)
                        RallyData_GetRoundDrivers(&g_unk0x0058e0a4, &g_unk0x0058df9c);
                    else
                        RallyData_GetRoundDrivers(&g_unk0x0058df98, &g_unk0x0058e0a4);
                    break;
                }
                CGameInfo::FUN_00405d70();
                mode = RallyData_FUN_00408800(g_unk0x0058e0a4 & 0xff);
                if (FUN_004232a0(g_unk0x0058e0a0->field_0xb1a, mode) != 0) {
                    FUN_00421720(g_unk0x0058e0a0->field_0xb1a,
                                 RallyData_FUN_00408800(g_unk0x0058e0a4 & 0xff), 0xffff,
                                 FUN_00422fb0(g_unk0x0058e0a0->field_0xb1a), 0);
                } else {
                    FUN_00421720(g_unk0x0058e0a0->field_0xb1a, 4, 0xffff,
                                 FUN_00422fb0(g_unk0x0058e0a0->field_0xb1a), 0);
                }
            }
        }
    }
}

extern int g_unk0x00547ad0;

// 0x547abc is g_stageLighting[0x5b] (the lighting block runs to 0x547ac8).
#define g_unk0x00547abc (g_stageLighting[0x5b])

// Positions a lens flare of the given view node on screen: its brightness
// follows the sun visibility and the camera's pitch, its colour is scaled by
// the same factor and the sprite is queued on layer 2.
// match 61%: same structure, calls and constants; MSVC places the brightness
// temporaries in the unused parameter homes instead of the original's slots.
// Rebuilds the box axes of a stage object from the car/ground vectors and
// accumulates the extent of the eight corner points of the object.
// match 82%: MSVC6 keeps the constant zero in ESI in the original and in EDX here (register
// allocation only; logic, constants and block layout match)
// FUNCTION: CMR2 0x00486c30
void FUN_00486c30(int *pObj, int *param2, int *param3, FixVector *pVerts)
{
    FixVector *p;
    FixVector d;
    int len;
    int dot1;
    int dot2;
    int m;
    int i;

    if (pObj[10] != 0)
        return;

    pObj[3] = 0;
    pObj[2] = 0;
    pObj[1] = 0;
    pObj[0] = 0;
    m = param2[1];
    if (m < 0)
        m = -m;
    if (m > 0xfd70) {
        ((FixVector *)(pObj + 4))->x = param2[3];
        ((FixVector *)(pObj + 4))->y = param2[4];
        ((FixVector *)(pObj + 4))->z = param2[5];
        pObj[5] = 0;
        len = FixVecLength((FixVector *)(pObj + 4));
        if (len == 0) {
            pObj[4] = 0;
            pObj[5] = 0;
            pObj[6] = 0;
        } else {
            FixVecScaleRecip((FixVector *)(pObj + 4), (FixVector *)(pObj + 4), len);
        }
    } else {
        ((FixVector *)(pObj + 4))->x = param2[0];
        ((FixVector *)(pObj + 4))->y = param2[1];
        ((FixVector *)(pObj + 4))->z = param2[2];
        pObj[5] = 0;
        len = FixVecLength((FixVector *)(pObj + 4));
        if (len == 0) {
            pObj[4] = 0;
            pObj[5] = 0;
            pObj[6] = 0;
        } else {
            FixVecScaleRecip((FixVector *)(pObj + 4), (FixVector *)(pObj + 4), len);
        }
    }
    pObj[8] = 0;
    pObj[7] = pObj[6];
    pObj[9] = -pObj[4];

    p = pVerts;
    i = 8;
    do {
        d.x = p->x - param3[0];
        d.y = p->y - param3[1];
        d.z = p->z - param3[2];
        dot1 = FixVecDot(&d, (FixVector *)(pObj + 4));
        dot2 = FixVecDot(&d, (FixVector *)(pObj + 7));
        if (dot1 > 0 && dot1 > pObj[0])
            pObj[0] = dot1;
        if (dot2 > 0 && dot2 > pObj[1])
            pObj[1] = dot2;
        if (d.y > pObj[2])
            pObj[2] = d.y;
        if (d.y < pObj[3])
            pObj[3] = d.y;
        p++;
    } while (--i);

    pObj[2] += param3[1];
    pObj[3] += param3[1];
    pObj[0x24] = (int)pVerts;
    pObj[0x25] = (int)param3;
    pObj[10] = 1;
    FUN_00486fc0(pObj, param3);
}

// Text colour and layer used by the knockout screen header.
// GLOBAL: CMR2 0x0051c980
BYTE g_unk0x0051c980 = 3;
// GLOBAL: CMR2 0x0051c9a4
BYTE g_unk0x0051c9a4[4] = { 0x61, 0x61, 0x7d, 0xff };
extern BYTE g_barTextColour[4];

// Draws the two header lines of a knockout match: interpolates the panel
// rectangle, then prints both driver names with the shared bar colours.
// FUNCTION: CMR2 0x00474fe0
void FUN_00474fe0(int param1, int param2, int param3, int param4, int param5, KnockoutMatch *param6)
{
    short rect[4];
    int x;
    int y;

    x = (param3 - param1) * g_unk0x0058cc74 / 0x10000 + param1;
    rect[0] = (short)x;
    y = (param4 - param2) * g_unk0x0058cc74 / 0x10000 + param2;
    rect[1] = (short)y;
    rect[2] = (short)((int)g_pGraphics->resX * 0x56 / 0x280);
    rect[3] = (short)((int)g_pGraphics->resY * 0x26 / 0x1e0);
    FUN_00475740(rect, g_unk0x0051c9a4, g_barTextColour, 0);
    if (CGameInfo::GetScreenWidth() >= 0x400 && CFrontend::FUN_004b7560(0x400) != 0 &&
        CFrontend::FUN_004b7590(0x400) != 0) {
        Font_DrawText(0, (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004736b0(param6, param5)),
                      (int)g_pGraphics->resX * 0x54 / 0x280 + x,
                      (int)g_pGraphics->resY * 0x10 / 0x1e0 - (int)g_pGraphics->resY * 2 / 0x1e0 + y,
                      (int *)g_barTextColour, 0x14);
        Font_DrawText(g_unk0x0051c980,
                      (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004752f0((int *)param6, param5, 0)),
                      (int)g_pGraphics->resX * 0x54 / 0x280 + x,
                      (int)g_pGraphics->resY * 0x1c / 0x1e0 + y + (int)g_pGraphics->resY * 7 / 0x1e0,
                      (int *)g_barTextColour, 0x14);
        return;
    }
    Font_DrawText(0, (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004736b0(param6, param5)),
                  (int)g_pGraphics->resX * 0x54 / 0x280 + x,
                  (int)g_pGraphics->resY * 0x10 / 0x1e0 + y,
                  (int *)g_barTextColour, 0x14);
    Font_DrawText(g_unk0x0051c980,
                  (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004752f0((int *)param6, param5, 0)),
                  (int)g_pGraphics->resX * 0x54 / 0x280 + x,
                  (int)g_pGraphics->resY * 0x1c / 0x1e0 + y,
                  (int *)g_barTextColour, 0x14);
}

BYTE FUN_004bc0c0(BYTE *p);

// Draws one pair of text lines at a stage-object rectangle, scaling the
// rectangle vertically when the panel is animating in.
// match 81%: the original's Font_DrawText takes 16-bit x/y (it adds them as words and
// pushes the register unextended); our Font.h declares them 32-bit, so every coordinate
// costs one movsx. Logic and constants are exact.
// FUNCTION: CMR2 0x00475430
void FUN_00475430(int param1, KnockoutMatch *param2, short *param3, BYTE *param4, BYTE *param5,
                  int param6, int param7, int *param8, int param9)
{
    short rect[4];

    if (FUN_004bc0c0(&g_unk0x0058ca80) != 0 || param9 != 0) {
        rect[0] = param3[0];
        rect[1] = param3[1];
        rect[2] = (short)(FixMul(param3[2] << 16, g_unk0x0058cc74) >> 16);
        rect[3] = (short)(FixMul(param3[3] << 16, g_unk0x0058cc74) >> 16);
        FUN_00475740(rect, param4, param5, param6);
        return;
    }
    rect[0] = param3[0];
    rect[1] = param3[1];
    rect[2] = param3[2];
    rect[3] = param3[3];
    FUN_00475740(rect, param4, param5, param6);
    if (CGameInfo::GetScreenWidth() >= 0x400 && CFrontend::FUN_004b7560(0x400) != 0 &&
        CFrontend::FUN_004b7590(0x400) != 0) {
        Font_DrawText(0, (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004736b0(param2, param1)),
                      (short)((short)((int)g_pGraphics->resX * 0x54 / 0x280) + param3[0]),
                      (short)((short)((int)g_pGraphics->resY * 0x10 / 0x1e0 - (int)g_pGraphics->resY * 2 / 0x1e0) + param3[1]),
                      param8, 0x14);
        Font_DrawText(g_unk0x0051c980,
                      (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004752f0((int *)param2, param1, param7)),
                      (short)((short)((int)g_pGraphics->resX * 0x54 / 0x280) + param3[0]),
                      (short)((short)((int)g_pGraphics->resY * 7 / 0x1e0 + (int)g_pGraphics->resY * 0x1c / 0x1e0) + param3[1]),
                      param8, 0x14);
        return;
    }
    Font_DrawText(0, (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004736b0(param2, param1)),
                  (short)((short)((int)g_pGraphics->resX * 0x54 / 0x280) + param3[0]),
                  (short)((short)((int)g_pGraphics->resY * 0x10 / 0x1e0) + param3[1]),
                  param8, 0x14);
    Font_DrawText(g_unk0x0051c980,
                  (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004752f0((int *)param2, param1, param7)),
                  (short)((short)((int)g_pGraphics->resX * 0x54 / 0x280) + param3[0]),
                  (short)((short)((int)g_pGraphics->resY * 0x1c / 0x1e0) + param3[1]),
                  param8, 0x14);
}

BYTE *FUN_0041b390(void);

// Steps every live replay object: advances the frame counter of the current
// record and appends the next one, rebuilding the lookup row when the current
// frame is exhausted.
// match 38%: the original keeps the constant zero in EBX and a separate `flag`/`valid`
// pair that MSVC folds here, which moves the loop's register allocation; the replay
// stepping logic and constants are otherwise transcribed from the dump
// FUNCTION: CMR2 0x0046c8e0
void FUN_0046c8e0(void)
{
    void ***pp;
    void **pObj;
    int *pRec;
    short *pIndex;
    int state;
    int flag;
    int valid;
    int slot;
    int offset;
    BYTE *pEnt;
    int base;
    char c;
    BYTE b;

    for (pp = g_unk0x00588d40; (int)pp < (int)(g_unk0x00588d40 + 16); pp++) {
        pObj = *pp;
        if (pObj == NULL)
            continue;
        pRec = (int *)*pObj;
        if (pRec == NULL)
            continue;
        if (pRec[3] == 0)
            continue;
        if (pRec[7] == 2)
            continue;
        if (*(BYTE *)((BYTE *)Car_Get(*(BYTE *)((BYTE *)pRec + 0x20)) + 0xb43) <= 0)
            continue;

        state = pRec[4];
        pIndex = (short *)(pRec[0x41] + *(short *)((BYTE *)pRec + 0x100) * 2);
        flag = 0;
        valid = 0;
        if (state != 0)
            valid = 1;
        else
            flag = 1;
        if (valid == 0)
            goto done;
        if (state == 0)
            goto done;

        slot = *(short *)((BYTE *)pRec + 0xfe) * *(short *)((BYTE *)pRec + 0x100) + *pIndex;
        offset = pRec[0xf] + slot * 4;
        if (FUN_0046cbe0((BYTE *)offset, *(BYTE *)((BYTE *)pRec + 0x20)) == 0) {
            *pIndex += 1;
            if (*pIndex == *(short *)((BYTE *)pRec + 0xfe)) {
                pRec[4] = 0;
                *pIndex += 1;
                *(short *)((BYTE *)pRec + 0x100) += 1;
                if (*(short *)((BYTE *)pRec + 0x100) == *(short *)((BYTE *)pRec + 0xfc))
                    Replay_StopRecording((BYTE *)pRec);
                goto done;
            }
            slot = *(short *)((BYTE *)pRec + 0xfe) * *(short *)((BYTE *)pRec + 0x100) + *pIndex;
            offset = pRec[0xf] + slot * 4;
            *(BYTE *)(offset + 2) = *(BYTE *)(pRec[0xf] + 2 + slot * 4) & 0xc0;
        }
        FUN_0046c450((BYTE *)offset, *(BYTE *)((BYTE *)pRec + 0x20));

        base = (int)FUN_0041b390();
        c = CGameInfo::FUN_00405e00();
        if (c == '\0')
            c = *(char *)(*(int *)(base + 4) + (DWORD)*(BYTE *)((BYTE *)pRec + 0x20) * 8);
        else
            c = **(char **)(base + 4);

        if (pRec[7] == 0) {
            base = pRec[9] + *(short *)((BYTE *)pRec + 0x100) * 0x114c;
            pEnt = (BYTE *)(base + 0x110c + (DWORD)*(BYTE *)(base + 0x1148) * 6);
            if (c != *(char *)(pEnt - 2)) {
                *(char *)(pEnt + 4) = c;
                *(short *)pEnt = *pIndex;
                *(unsigned short *)(pEnt + 2) = *(BYTE *)(offset + 2) & 0x3f;
                *(BYTE *)(base + 0x1148) += 1;
                if (*(short *)(pEnt + 2) == 0) {
                    b = *(BYTE *)(pRec[0xf] - 2 + slot * 4);
                    *(short *)pEnt -= 1;
                    *(unsigned short *)(pEnt + 2) = b & 0x3f;
                } else {
                    *(short *)(pEnt + 2) -= 1;
                }
            }
        } else {
            base = *(short *)((BYTE *)pRec + 0x100) * 0x5c + pRec[0xc];
            pEnt = (BYTE *)(base + (DWORD)*(BYTE *)(base + 0x58) * 6);
            if (c != *(char *)(pEnt + 0x1a)) {
                *(char *)(pEnt + 0x20) = c;
                *(short *)(pEnt + 0x1c) = *pIndex;
                *(unsigned short *)(pEnt + 0x1e) = *(BYTE *)(offset + 2) & 0x3f;
                *(BYTE *)(base + 0x58) += 1;
                if (*(short *)(pEnt + 0x1e) == 0) {
                    b = *(BYTE *)(pRec[0xf] - 2 + slot * 4);
                    *(short *)(pEnt + 0x1c) -= 1;
                    *(unsigned short *)(pEnt + 0x1e) = b & 0x3f;
                } else {
                    *(short *)(pEnt + 0x1e) -= 1;
                }
            }
        }

done:
        if (flag != 0) {
            offset = pRec[0xf] + *(short *)((BYTE *)pRec + 0xfe) * *(short *)((BYTE *)pRec + 0x100) * 4;
            *(short *)(pRec[0x41] + *(short *)((BYTE *)pRec + 0x100) * 2) = 0;
            *(BYTE *)(offset + 2) &= 0xc0;
            pRec[6] = 1;
            if (pRec[7] == 0)
                pRec[0xb] = pRec[9] + *(short *)((BYTE *)pRec + 0x100) * 0x114c;
            else
                pRec[0xe] = *(short *)((BYTE *)pRec + 0x100) * 0x5c + pRec[0xc];
            pRec[4] = 1;
        }
    }
}

// Draws the stage icon of one view node: projects its world position to the
// screen, blends the object ramp into the stage colour and queues the sprite.
// FUNCTION: CMR2 0x00462d80
void FUN_00462d80(int param_1, int param_2)
{
    SpriteRect uv;
    SpriteRect dst;
    SpriteRect icon;
    FixVector vR;
    FixVector vP;
    FixVector vOut;
    BYTE colour[4];
    int view;
    int ramp;
    int value;

    int diff;
    int scale;
    int base;

    icon.w = 0x10;
    icon.h = 0x10;
    icon.x = 0;
    icon.y = 0;
    if (g_unk0x00547ad0 == 0)
        return;
    uv.x = *(short *)(g_unk0x00547ad0 + 0x11c);
    uv.y = *(short *)(g_unk0x00547ad0 + 0x11e);
    uv.w = *(short *)(g_unk0x00547ad0 + 0x120);
    uv.h = *(short *)(g_unk0x00547ad0 + 0x122);
    view = (int)g_viewNodes[param_2];
    FUN_00492890(&vR);
    FUN_0046f4a0(&vP);
    vR.x += vP.x;
    vR.y += vP.y;
    vR.z += vP.z;
    FUN_004bad40((int *)&vOut, &vR, (BYTE *)view);
    vOut.x >>= 16;
    vOut.y >>= 16;
    icon.x = (short)(vOut.x - icon.w / 2);
    icon.y = (short)(vOut.y - icon.h / 2);
    FUN_00462d10((short *)&icon);
    ramp = FixMul(g_unk0x00547abc, -0x20000) + 0x20000;
    if (ramp < 0)
        ramp = 0;
    else if (ramp > 0x10000)
        ramp = 0x10000;
    value = FixMul((int)g_sunVisibility << 16, 0x28f);
    base = param_2 * 0x178;
    diff = value - *(int *)((BYTE *)g_unk0x00547ac8 + base + 0x68);
    if ((diff < 0 ? -diff : diff) > FixMul(0x4ccc, g_unk0x0051bd3c)) {
        if (diff > 0)
            *(int *)((BYTE *)g_unk0x00547ac8 + base + 0x68) += FixMul(0x4ccc, g_unk0x0051bd3c);
        else
            *(int *)((BYTE *)g_unk0x00547ac8 + base + 0x68) -= FixMul(0x4ccc, g_unk0x0051bd3c);
    } else {
        *(int *)((BYTE *)g_unk0x00547ac8 + base + 0x68) = value;
    }
    ramp += *(int *)((BYTE *)g_unk0x00547ac8 + base + 0x68);
    if (ramp > 0x10000)
        ramp = 0x10000;
    scale = FixMul(0x10000 - ramp, 0x30000);
    if (scale > 0x10000)
        scale = 0x10000;
    colour[0] = (BYTE)FixMulShift32(g_unk0x00543eb4[0] << 16, scale);
    colour[1] = (BYTE)FixMulShift32(g_unk0x00543eb4[1] << 16, scale);
    colour[2] = (BYTE)FixMulShift32(g_unk0x00543eb4[2] << 16, scale);
    colour[3] = 0xff;
    dst.w = (short)FixMulShift32(g_unk0x00543d58, *(int *)g_pGraphics << 16);
    dst.h = (short)FixMulShift32(g_unk0x00543d5c, *((int *)g_pGraphics + 1) << 16);
    dst.x = (short)(vOut.x - dst.w / 2);
    dst.y = (short)(vOut.y - dst.h / 2);
    Sprite_Queue(&uv, &dst, (Texture *)g_unk0x00547ad0, 2, 0, 0, 0, colour, 8);
}

// GLOBAL: CMR2 0x005894b8
BYTE g_unk0x005894b8[40];
// GLOBAL: CMR2 0x0058c424
int g_unk0x0058c424[320];

int FUN_00471d40(BYTE **pEntry, int bit);
void FUN_00471d80(BYTE **pp, int bit, int set);
void FUN_00470240(BYTE **pElement, int car);

// Queues a moving stage object: appends it to the free table, or (when the
// table is full) evicts the entry whose objects are farthest from the cars.
// FUNCTION: CMR2 0x0046fe70
void FUN_0046fe70(int *param_1, int param_2, int param_3)
{
    StageObjectEntry0x128 *entry;
    FixVector pos;
    Car *pCar;
    int dx;
    int dy;
    int dz;
    int d;
    int inv;
    int v;
    int best;
    int bestIx;
    int j;
    int c;

    if (FUN_00471d40((BYTE **)&param_1, param_3) != 0) {
        FUN_00471d80((BYTE **)&param_1, param_3, 0);
        pCar = Car_Get(param_3);
        if (pCar->field_0xc0c == 0 && (char)RallyDataState() == 1) {
            *(int *)(*param_1 + 4) += -0x3e80000;
            *(BYTE *)(*param_1 + 0x14) = 0;
        }
    }
    pCar = Car_Get(param_3);
    if (pCar->field_0xc0c != 0)
        return;
    if (g_unk0x0058c924 < 0x28) {
        entry = &g_unk0x005894e0[g_unk0x0058c924];
        entry->field_0x0 = (int)param_1;
        *(int *)((BYTE *)entry + 0x118) = param_2;
        StageObject_InitMovingObject((int *)entry, param_3);
        g_unk0x0058c924++;
        return;
    }
    c = 0;
    if (g_unk0x0058c924 != 0) {
        entry = &g_unk0x005894e0[0];
        do {
            RallyData_FUN_00471cc0((int *)&pos, (void **)entry);
            for (j = 0; j < Car_GetOrderCount(); j++) {
                pCar = Car_Get(j);
                dx = pCar->position.x - pos.x;
                pCar = Car_Get(j);
                dy = pCar->position.y - pos.y;
                pCar = Car_Get(j);
                dz = pCar->position.z - pos.z;
                d = (abs(dz) < abs(dx)) ? abs(dx) : abs(dz);
                if (d < 0x290) {
                    g_unk0x0058c424[j + c * 8] = 0;
                } else {
                    inv = (int)(0x100000000i64 / d);
                    dx = (int)(((__int64)dx * inv) >> 16);
                    dz = (int)(((__int64)dz * inv) >> 16);
                    dy = 0;
                    v = FixMul(dx, dx) + FixMul(dz, dz);
                    if (v != 0)
                        v = FixSqrt(v);
                    g_unk0x0058c424[j + c * 8] = v;
                    g_unk0x0058c424[j + c * 8] = FixMul(v, d);
                }
            }
            g_unk0x005894b8[c] = 0;
            c++;
            entry++;
        } while (c < g_unk0x0058c924);
    }
    for (j = 0; j < Car_GetOrderCount(); j++) {
        best = g_unk0x0058c424[j];
        bestIx = 0;
        for (c = 1; c < g_unk0x0058c924; c++) {
            if (best < g_unk0x0058c424[j + c * 8]) {
                bestIx = c;
                best = g_unk0x0058c424[j + c * 8];
            }
        }
        g_unk0x005894b8[bestIx]++;
    }
    best = g_unk0x005894b8[0];
    bestIx = 0;
    for (c = 1; c < g_unk0x0058c924; c++) {
        if (best < g_unk0x005894b8[c]) {
            bestIx = c;
            best = g_unk0x005894b8[c];
        }
    }
    entry = &g_unk0x005894e0[bestIx];
    for (j = 0; j < 8; j++)
        FUN_00470240((BYTE **)entry->field_0x0, j);
    entry->field_0x0 = (int)param_1;
    *(int *)((BYTE *)entry + 0x118) = param_2;
    StageObject_InitMovingObject((int *)entry, param_3);
}

extern int *g_unk0x00588b9c;
extern int *g_unk0x00588ba0;

// Builds the vertex buffer of one stage object for one car: converts the
// float source vertices to 16.16 fixed point and packs the normal bytes.
// FUNCTION: CMR2 0x0046afe0
void FUN_0046afe0(int param_1, int param_2, int param_3)
{
    int i;
    int j;
    int off;
    int total;
    int *pRec;
    int n;
    int c;
    int b;
    FixVector v;

    g_unk0x00588b9c[param_1] = (int)CFileBuffer::AllocateLockedBuffer(*(int *)(param_3 + 0x45c) << 2);
    total = *(int *)(param_3 + 0x45c) * 4;
    g_unk0x00588ba0[param_1] = (int)CFileBuffer::AllocateLockedBuffer(*(int *)(param_3 + 0x45c));
    i = 0;
    if (*(int *)(param_3 + 0x45c) > 0) {
        pRec = (int *)(param_3 + 0x420);
        do {
            int *pVertices;

            off = i * 4;
            n = *pRec;
            pVertices = (int *)((BYTE *)g_unk0x00588b9c[param_1] + off);
            *pVertices = (int)CFileBuffer::AllocateLockedBuffer(n << 5);
            total += n * 0x20;
            ((BYTE *)g_unk0x00588ba0[param_1])[i] = *(BYTE *)(*(int *)(pRec - 0xf9) + 0x30);
            j = 0;
            if (n > 0) {
                c = 0;
                b = 0;
                do {
                    float *pF = (float *)(*(int *)(pRec - 0x108) + 0xc + c);
                    int *pDst = (int *)(*(int *)((BYTE *)g_unk0x00588b9c[param_1] + off) + b);
                    DWORD col;
                    int t;

                    pDst[0] = (int)(__int64)(pF[0] * CGraphics::m_65536);
                    pDst[1] = (int)(__int64)(pF[1] * CGraphics::m_65536);
                    pDst[2] = (int)(__int64)(pF[2] * CGraphics::m_65536);
                    pDst[3] = (int)(__int64)(pF[3] * CGraphics::m_65536);
                    pDst[4] = (int)(__int64)(pF[4] * CGraphics::m_65536);
                    pDst[5] = (int)(__int64)(pF[5] * CGraphics::m_65536);
                    ((BYTE *)pDst)[0x1b] = (pDst[0] < 0) ? 0x7f : 0x81;
                    ((BYTE *)pDst)[0x1c] = (pDst[1] < 0) ? 0x7f : 0x81;
                    ((BYTE *)pDst)[0x1d] = (pDst[2] < 0) ? 0x7f : 0x81;
                    col = *(DWORD *)(*(int *)(pRec - 0x108) + 0xc + 0x18 + c);
                    t = (int)((col >> 16) & 0xff) - 0x80;
                    if (t < -0x7f)
                        t = -0x7f;
                    else if (t > 0x7f)
                        t = 0x7f;
                    ((BYTE *)pDst)[0x1b] = (BYTE)t;
                    t = (int)((col >> 8) & 0xff) - 0x80;
                    if (t < -0x7f)
                        t = -0x7f;
                    else if (t > 0x7f)
                        t = 0x7f;
                    ((BYTE *)pDst)[0x1c] = (BYTE)t;
                    t = (int)(col & 0xff) - 0x80;
                    if (t < -0x7f)
                        t = -0x7f;
                    else if (t > 0x7f)
                        t = 0x7f;
                    ((BYTE *)pDst)[0x1d] = (BYTE)t;
                    v.x = (int)(signed char)((BYTE *)pDst)[0x1b] * -0x200;
                    v.y = (int)(signed char)((BYTE *)pDst)[0x1c] * -0x200;
                    v.z = (int)(signed char)((BYTE *)pDst)[0x1d] * -0x200;
                    {
                        int len = FixVecLength(&v);

                        if (len == 0) {
                            ((BYTE *)pDst)[0x18] = 0;
                            ((BYTE *)pDst)[0x19] = 0;
                            ((BYTE *)pDst)[0x1a] = 0;
                        } else {
                            int inv = (int)(0x100000000i64 / len);

                            v.x = FixMul(v.x, inv);
                            v.y = FixMul(v.y, inv);
                            v.z = FixMul(v.z, inv);
                            t = v.x >> 9;
                            if (t > 0x7f)
                                t = 0x7f;
                            else if (t < -0x7f)
                                t = -0x7f;
                            ((BYTE *)pDst)[0x18] = (BYTE)t;
                            t = v.y >> 9;
                            if (t > 0x7f)
                                t = 0x7f;
                            else if (t < -0x7f)
                                t = -0x7f;
                            ((BYTE *)pDst)[0x19] = (BYTE)t;
                            t = v.z >> 9;
                            if (t > 0x7f)
                                t = 0x7f;
                            else if (t < -0x7f)
                                t = -0x7f;
                            ((BYTE *)pDst)[0x1a] = (BYTE)t;
                        }
                    }
                    j++;
                    b += 0x20;
                    c += 0x30;
                } while (j < n);
            }
            i++;
            pRec++;
        } while (i < *(int *)(param_3 + 0x45c));
    }
}

int FUN_004218d0(unsigned int view);
void RallyData_FUN_00408760(BYTE index, int value);

// Applies the driver-camera cycle: while the cycle key is held it picks the
// target driver (either from the current knockout round or by advancing the
// active one) and hands the resulting view mode to the rally data layer.
// match 53%: implementada, MSVC6 cachea param_1 en EDI y coloca distinta la tabla del switch
// FUNCTION: CMR2 0x0047b970
void FUN_0047b970(unsigned int param_1)
{
    unsigned int uVar4;
    unsigned int uVar5;
    BYTE *p;

    p = FUN_0041b390();
    if (**(char **)(p + 4) == 10) {
        if (FUN_0041f3a0() == 0) {
            uVar4 = FUN_004218d0(0);
        } else {
            if (CGameInfo::FUN_00405d80() == 2)
                uVar4 = param_1;
            else
                uVar4 = FUN_004218d0(1);
        }
    } else {
        uVar4 = FUN_004218d0(*(BYTE *)((BYTE *)g_unk0x0058e0a0 + 0xb1a));
    }

    p = FUN_0041b390();
    if ((**(char **)(p + 4) == 8) || (p = FUN_0041b390(), **(char **)(p + 4) == 7)) {
        uVar5 = FUN_0041b380();
        switch (uVar5) {
        case 0:
        case 1:
            g_unk0x0058e0a4 = param_1;
            break;
        case 2:
            RallyData_GetRoundDrivers(&g_unk0x0058df98, &g_unk0x0058df9c);
            if (RallyData_FUN_00408500(g_unk0x0058df98 & 0xff) == -1)
                g_unk0x0058e0a4 = g_unk0x0058df98;
            else
                g_unk0x0058e0a4 = g_unk0x0058df9c;
            break;
        case 3:
            if (param_1 == 0)
                RallyData_GetRoundDrivers(&g_unk0x0058e0a4, &g_unk0x0058df9c);
            else
                RallyData_GetRoundDrivers(&g_unk0x0058df98, &g_unk0x0058e0a4);
            break;
        case 4:
            g_unk0x0058e0a4 = (FUN_0041b370() & 0xff) + param_1;
            break;
        }
        {
            BYTE bVar2 = CGameInfo::FUN_00405d70();
            if ((int)g_unk0x0058e0a4 < (int)(unsigned int)bVar2 &&
                (uVar4 == 1 || uVar4 == 2 || uVar4 == 3 || uVar4 == 5 || uVar4 == 4)) {
                RallyData_FUN_00408760((BYTE)g_unk0x0058e0a4, uVar4);
            }
        }
    }
}

// Builds the bounding box of an object's collision points (8 triples): the
// object's 2D direction is normalised from its accumulated translation and
// every point is projected onto it, tracking the box bounds.
// match 59%: implementada, difiere el marco de pila y la fusion de bloques de normalizacion
// FUNCTION: CMR2 0x004873f0
void FUN_004873f0(int *param_1, int param_2, int param_3)
{
    short *pOut = *(short **)(param_3 + 4);
    FixVector v;
    int absY = param_1[1];
    int dx = 0;
    int dy = 0;
    int maxRight = 0;
    int maxLeft = 0;
    int len;
    int *p;
    int n;

    pOut[3] = 0;
    pOut[2] = 0;

    if (absY < 0)
        absY = -absY;

    if (absY > 0xfd70) {
        v.x = param_1[3];
        v.y = 0;
        v.z = param_1[5];
        len = FixVecLength(&v);
        if (len == 0) {
            dx = 0;
            dy = 0;
        } else {
            FixVecScaleRecip(&v, &v, len);
            dx = v.x;
            dy = v.z;
        }
    } else {
        v.x = param_1[0];
        v.y = 0;
        v.z = param_1[2];
        len = FixVecLength(&v);
        if (len == 0) {
            dx = 0;
            dy = 0;
        } else {
            FixVecScaleRecip(&v, &v, len);
            dx = v.x;
            dy = v.z;
        }
    }

    p = (int *)(param_2 + 8);
    n = 8;
    do {
        int c = p[-1];
        int right = FixMul(dx, p[-2]) + FixMul(dy, p[0]);
        int left = FixMul(-dx, p[0]) + FixMul(dy, p[-2]);
        short q = (short)(c >> 9);

        if (right > 0 && maxRight < right)
            maxRight = right;
        if (left > 0 && maxLeft < left)
            maxLeft = left;
        if ((short)pOut[2] * 0x200 < c)
            pOut[2] = q;
        if (c < (short)pOut[3] * 0x200)
            pOut[3] = q;

        p += 3;
    } while (--n != 0);

    pOut[0] = (short)(dx >> 9);
    pOut[4] = (short)(maxRight >> 9);
    pOut[1] = (short)(dy >> 9);
    pOut[5] = (short)(maxLeft >> 9);
}

extern double g_minus65536;

// Chooses the wall-collision response of a car part from its lateral/forward
// offsets and the wall distances stored in the reference record; returns the
// facing side (1..4) or a slanted-response code (5/6), 0 when clear.
// match 54%: implementada, MSVC6 reparte distinto los locales y el modo del muro
// FUNCTION: CMR2 0x0047c5e0
unsigned int FUN_0047c5e0(int param_1)
{
    int mode;
    int off;
    int limit;
    int blocked;
    int local10;
    int isStackC;
    int iVar4;
    int iVar7;
    int iVar8;
    int iVar5;
    unsigned int u;

    mode = 0;
    if (*(int *)(param_1 + 0x5c) < -0xa0000)
        mode = 1;
    if (0xa0000 < *(int *)(param_1 + 0x5c))
        mode = 2;

    iVar7 = *(int *)(param_1 + 0x34);
    if (iVar7 < 0) {
        iVar4 = *(int *)(param_1 + 0x54) * 0x10;
        local10 = (int)(__int64)((double)(int)*(signed char *)(g_unk0x0058e4a4 + iVar4 + 0xd) * g_minus65536);
        iVar8 = iVar7 - local10;
        {
            char cVar1 = *(signed char *)(g_unk0x0058e4a4 + iVar4 + 0xe);
            local10 = (int)(__int64)((double)(int)cVar1 * g_minus65536);
            iVar5 = 0;
            if ((iVar7 - local10 < 0x40000) && (cVar1 < 0x11))
                iVar5 = iVar7 - local10;
        }
        if ((0 < iVar8) && (iVar8 < 0x40000))
            iVar5 = iVar8;
        if (0 < iVar5) {
            if (0x5a0000 < *(int *)(param_1 + 0x38))
                return 1;
            if (*(int *)(param_1 + 0x38) - FixMul(-iVar5, 0xf0000) + 0x3c0000 < 0)
                return 2;
        }
        if (iVar8 < 0) {
            iVar7 = *(int *)(param_1 + 0x38);
            if (iVar7 < 1) {
                if (iVar7 < -0x6e0000)
                    return 2;
                return ((iVar7 < -0x45ffff) - 1 & 0xfffffffe) + 6;
            }
            if (0x6e0000 < iVar7)
                return 1;
            return ((0x45ffff < iVar7) - 1 & 0xfffffffe) + 5;
        }
        local10 = (int)(__int64)((double)(int)*(signed char *)(g_unk0x0058e4a4 + iVar4 + 0xc) * g_minus65536);
        local10 = *(int *)(param_1 + 0x34) - local10;
        if ((local10 < 0) && (mode != 2)) {
            if (0x3c0000 < *(int *)(param_1 + 0x38))
                return 1;
            if (*(int *)(param_1 + 0x38) < -0x2d0000)
                return 2;
            if (*(int *)(param_1 + 0x38) - FixMul(-local10, 0x50000) < 0)
                return 3;
        }
    } else {
        iVar5 = *(int *)(param_1 + 0x54) * 0x10;
        local10 = (int)(__int64)((double)(int)*(signed char *)(g_unk0x0058e4a4 + iVar5 + 9) * g_minus65536);
        iVar4 = -iVar7 - local10;
        {
            char cVar1 = *(signed char *)(g_unk0x0058e4a4 + iVar5 + 10);
            local10 = (int)(__int64)((double)(int)cVar1 * g_minus65536);
            local10 = -iVar7 - local10;
            isStackC = 0;
            if ((local10 < 0x40000) && (cVar1 < 0x11))
                isStackC = local10;
        }
        if ((0 < iVar4) && (iVar4 < 0x40000))
            isStackC = iVar4;
        if (0 < isStackC) {
            if (*(int *)(param_1 + 0x38) < -0x5a0000)
                return 3;
            iVar7 = *(int *)(param_1 + 0x38) - FixMul(isStackC, 0xf0000);
            if (iVar7 != 0x3c0000 && -1 < iVar7 + -0x3c0000)
                return 4;
        }
        if (iVar4 < 0) {
            iVar7 = *(int *)(param_1 + 0x38);
            if (iVar7 < 1) {
                if (iVar7 < -0x6e0000)
                    return 3;
                return ((iVar7 < -0x45ffff) - 1 & 0xfffffffc) + 5;
            }
            if (0x6e0000 < iVar7)
                return 4;
            return ((0x45ffff < iVar7) - 1 & 0xfffffffc) + 6;
        }
        local10 = (int)(__int64)((double)(int)*(signed char *)(g_unk0x0058e4a4 + iVar5 + 8) * g_minus65536);
        iVar7 = -local10 - *(int *)(param_1 + 0x34);
        if ((iVar7 < 0) && (mode != 1)) {
            if (0x2d0000 < *(int *)(param_1 + 0x38))
                return 4;
            if (*(int *)(param_1 + 0x38) < -0x3c0000)
                return 3;
            u = FixMul(iVar7, 0x50000);
            if (*(unsigned int *)(param_1 + 0x38) != u &&
                -1 < (int)(*(unsigned int *)(param_1 + 0x38) - u))
                return 1;
        }
    }
    if (0x780000 < *(int *)(param_1 + 0x38))
        return 4;
    return (-0x780001 < *(int *)(param_1 + 0x38)) - 1 & 2;
}

int RallyData_FUN_00421420(void);
int RallyData_FUN_00421370(BYTE *p);
void RallyData_FUN_00421530(int index, int *pOut);
extern double g_unk0x00511300;

// Picks the closest car ahead of the reference angle among the active cars,
// rejecting those out of range or outside the angular window, and writes the
// chosen one's relative state code to *param_2.
// match 54%: implementada, difiere el reparto de locales/registros y la fusion de bloques
// FUNCTION: CMR2 0x0047cd10
int FUN_0047cd10(int param_1, int *param_2, int param_3, int *param_4)
{
    Car *cars[6];
    int flags[6];
    int maxAng[6];
    int angles[6];
    int i, n;
    int wrap, base, refX, refZ;
    int outA[3], outB[3];
    int outIdx;
    int iVar7, iVar8, iVar9, iVar10;
    unsigned int uVar2, length;

    *param_2 = 0;
    for (i = 0; i < 6; i++) {
        flags[i] = 0;
        cars[i] = Car_Get(i);
    }
    wrap = RallyData_FUN_00421420();
    refX = *(int *)((BYTE *)cars[param_1] + 0x2d8);
    refZ = *(int *)((BYTE *)cars[param_1] + 0x2d0);
    base = param_4[4];
    n = (int)(signed char)g_unk0x0058e0b0[0];

    if (0 < n) {
        for (i = 0; i < n; i++) {
            if ((i != param_1) && (*(int *)((BYTE *)cars[i] + 0x778) < 0xc800))
                flags[i] = 1;
        }
    }
    if (0 < n) {
        for (i = 0; i < n; i++) {
            if (flags[i] != 0) {
                iVar8 = StageObject_Atan2Degrees(*(int *)((BYTE *)cars[i] + 0x368),
                                                 *(int *)((BYTE *)cars[i] + 0x360));
                iVar8 = FUN_00498db0(base - iVar8);
                if ((iVar8 < -0x5a0000) || (0x5a0000 < iVar8))
                    maxAng[i] = 10;
                else
                    maxAng[i] = 5;
            }
        }
    }
    if (0 < n) {
        for (i = 0; i < n; i++) {
            if (flags[i] != 0) {
                iVar8 = RallyData_FUN_00421370((BYTE *)cars[i]);
                angles[i] = iVar8;
                iVar8 = iVar8 - param_3;
                if (iVar8 < -100)
                    iVar8 = iVar8 + wrap;
                if ((iVar8 < 0) || (maxAng[i] < iVar8))
                    flags[i] = 0;
            }
        }
    }
    outIdx = 0;
    if (0 < (signed char)g_unk0x0058e0b0[0]) {
        do {
            if (flags[outIdx] != 0) {
                int dx = *(int *)((BYTE *)cars[outIdx] + 0x2d8) - refX;
                int dz = *(int *)((BYTE *)cars[outIdx] + 0x2d0) - refZ;

                uVar2 = (unsigned int)FixSqrt(FixMul(dx, dx) + FixMul(dz, dz));
                length = uVar2;
                iVar7 = StageObject_Atan2Degrees(dx, dz);
                iVar8 = FUN_00498db0(base - iVar7);
                if ((iVar8 < 0x2d0001) && (-0x2d0001 < iVar8)) {
                    iVar10 = FixDiv(uVar2 - 0x70000, 0x70000) + 0x20000;
                    if (iVar10 < 0x50001) {
                        if (iVar10 < 0)
                            iVar10 = 0;
                    } else {
                        iVar10 = 0x50000;
                    }
                    length = (unsigned int)FixMul((int)length,
                        g_sinTable[(int)(__int64)((double)iVar8 * g_unk0x00511300) & 0xfff]);
                    iVar9 = (param_3 + 5) - wrap;
                    iVar7 = param_3 + 5;
                    if (-1 < iVar9)
                        iVar7 = iVar9;
                    RallyData_FUN_00421530(iVar7, outB);
                    iVar7 = StageObject_Atan2Degrees(outB[2] - refX, outB[0] - refZ);
                    FUN_00498db0(base - iVar7);
                    if (((int)length <= iVar10) && (-iVar10 <= (int)length)) {
                        iVar8 = FUN_00498db0(iVar8 - iVar7);
                        if (iVar8 < 0) {
                            if (0x320000 < *param_4) {
                                flags[outIdx] = 4;
                                *param_2 = flags[outIdx];
                                return outIdx;
                            }
                            flags[outIdx] = 2;
                            *param_2 = flags[outIdx];
                            return outIdx;
                        }
                        if (0x320000 < *param_4) {
                            flags[outIdx] = 3;
                            *param_2 = flags[outIdx];
                            return outIdx;
                        }
                        flags[outIdx] = 1;
                        *param_2 = flags[outIdx];
                        return outIdx;
                    }
                    flags[outIdx] = 0;
                } else {
                    flags[outIdx] = 0;
                }
            }
            outIdx++;
        } while (outIdx < (signed char)g_unk0x0058e0b0[0]);
    }
    return -1;
}

// Initialises the per-car stage-object record for one lane: validates it, stores
// the car index, zeroes the timers and dispatches to the type-specific reset
// (object list, light list or mesh list) resetting the light buffer too.
// match 57%: implementada; MSVC6 no emite el `mov eax,1` final (firma void por compatibilidad con las llamadas de Race.cpp)
// FUNCTION: CMR2 0x0046c750
void FUN_0046c750(int param_1, int param_2, int param_3)
{
    BYTE *p;
    char cVar1;

    if (param_1 == 0 || *(int *)(param_1 + 0xc) != 0 || *(int *)(param_1 + 4) != 0)
        return;
    *(int *)param_1 = 0;
    if ((BYTE)param_3 < 2)
        *(int *)param_1 = (int)CInput::FUN_0040be60(param_3 & 0xff);
    *(BYTE *)(param_1 + 0x20) = (BYTE)param_3;
    *(int *)(param_1 + 0xc) = 1;
    *(WORD *)(param_1 + 0x100) = 0;
    *(int *)(param_1 + 0x10) = 1;
    *(WORD *)(*(int *)(param_1 + 0x104)) = 0;
    if (*(int *)(param_1 + 0x1c) != 2)
        *(BYTE *)(*(int *)(param_1 + 0x3c) + 2) &= 0xc0;
    if (*(int *)(param_1 + 0x1c) == 0) {
        p = FUN_0041b390();
        *(BYTE *)(*(int *)(param_1 + 0x24) + 0x1110) =
            (BYTE)((unsigned int)*(int *)(*(int *)(p + 4) + (BYTE)*(BYTE *)(param_1 + 0x20) * 8) >> 0x10);
        *(WORD *)(*(int *)(param_1 + 0x24) + 0x110c) = 0;
        *(WORD *)(*(int *)(param_1 + 0x24) + 0x110e) = 0;
        *(BYTE *)(*(int *)(param_1 + 0x24) + 0x1148) = 1;
    } else {
        p = FUN_0041b390();
        cVar1 = (char)CGameInfo::FUN_00405e00();
        if (cVar1 == '\0')
            *(BYTE *)(*(int *)(param_1 + 0x30) + 0x20) =
                (BYTE)((unsigned int)*(int *)(*(int *)(p + 4) + (BYTE)*(BYTE *)(param_1 + 0x20) * 8) >> 0x10);
        else
            *(BYTE *)(*(int *)(param_1 + 0x30) + 0x20) =
                (BYTE)((unsigned int)*(int *)(*(int *)(p + 4)) >> 0x10);
        *(WORD *)(*(int *)(param_1 + 0x30) + 0x1c) = 0;
        *(WORD *)(*(int *)(param_1 + 0x30) + 0x1e) = 0;
        *(BYTE *)(*(int *)(param_1 + 0x30) + 0x58) = 1;
    }
    if (*(int *)(param_1 + 0x1c) == 0) {
        FUN_0046c240((BYTE *)*(int *)(param_1 + 0x24), (BYTE)param_3);
        *(int *)(param_1 + 0x18) = 0;
        *(BYTE *)(param_1 + 0xf8) = 0;
        return;
    }
    if (*(int *)(param_1 + 0x1c) == 1) {
        FUN_0046c320((int *)*(int *)(param_1 + 0x30), (BYTE)param_3);
        *(int *)(param_1 + 0x18) = 0;
        *(BYTE *)(param_1 + 0xf8) = 0;
        return;
    }
    cVar1 = (char)CGameInfo::FUN_00405e00();
    if (cVar1 == '\0' || *(BYTE *)(param_1 + 0x20) != 0) {
        cVar1 = (char)CGameInfo::FUN_00405e00();
        if (cVar1 != '\0')
            goto done;
    }
    {
        Car *pCar = Car_Get((BYTE)*(BYTE *)(param_1 + 0x20));
        *(int *)((BYTE *)pCar + 0xc18) = 0;
    }
done:
    *(int *)(param_1 + 0x18) = 0;
    *(BYTE *)(param_1 + 0xf8) = 0;
}

// Views into the stage object block declared in StageBlock.h, used by the
// stage object pose code.
// GLOBAL: CMR2 0x0058d2f8  (per object: 3 int, animated pose offset)
#define g_unk0x0058d2f8 (g_stageBlock + 0x58)
// GLOBAL: CMR2 0x0058d368  (per object: 0x24-byte timing record, angle at +0)
#define g_unk0x0058d368 (g_stageBlock + 0xc8)
// GLOBAL: CMR2 0x0058d374  (per object: int pose offset, +4/+8 are y/z)
#define g_unk0x0058d374 (g_stageBlock + 0xd4)
// GLOBAL: CMR2 0x0058d560  (scratch 4x4 16.16 matrix, 16 int = 0x40 bytes)
#define g_unk0x0058d560 ((int *)(g_stageBlock + 0x2c0))

// Integrates one stage object's pose: rebuilds the object matrix from the car
// basis, low-pass filters the object's angles against the car's body axes and
// moves the object's scene node.
// match 16%: implementada (logica transliterada del decompilado); la diferencia es de
// reparto de registros/bloques en los productos escalares y el filtrado de angulos, no de logica
// FUNCTION: CMR2 0x00476e00
void FUN_00476e00(BYTE *param_1, int *param_2, int unused)
{
    Car *pCar;
    int index;
    int *pMatrix;
    FixVector acc;
    int dRight, dForward;
    short angRight, angForward, tableAng;
    int i, recOff;
    int offX, offY, offZ;
    FixVector pos;

    index = param_1[2];
    pCar = Car_Get(index);

    // Object matrix: rows 0 and 2 swapped (the old row 2 negated), translation
    // cleared, then rotated about the object's right axis.
    pMatrix = g_unk0x0058d560;
    *(ObjectMatrix16 *)pMatrix = *(ObjectMatrix16 *)param_2;
    pMatrix[0] = -param_2[8];
    pMatrix[1] = -param_2[9];
    pMatrix[2] = -param_2[10];
    pMatrix[8] = param_2[0];
    pMatrix[9] = param_2[1];
    pMatrix[10] = param_2[2];
    pMatrix[12] = 0;
    pMatrix[13] = 0;
    pMatrix[14] = 0;
    FixMatrix_RotateAboutRight((FixMatrix *)pMatrix,
                               ((unsigned int)param_2[0] & 0xffff0000) |
                                   (unsigned int)(unsigned short)g_unk0x0051c9b0);

    // Steps this object's 12-bit angle by 3.
    i = (int)*(short *)(g_unk0x0058d368 + index * 0x24) + 3;
    i &= 0x80000fff;
    if (i < 0)
        i = (i - 1 | 0xfffff000) + 1;
    *(short *)(g_unk0x0058d368 + index * 0x24) = (short)i;

    // Angle of the car body axes against the car's last acceleration.
    acc.x = pCar->velocity.x - pCar->velocityNext.x;
    acc.y = pCar->velocity.y - pCar->velocityNext.y;
    acc.z = pCar->velocity.z - pCar->velocityNext.z;
    dRight = FixVecDot(&acc, &pCar->right);
    angRight = (short)FixDiv(dRight, 0x1e0000);
    acc.x = pCar->velocity.x - pCar->velocityNext.x;
    acc.y = pCar->velocity.y - pCar->velocityNext.y;
    acc.z = pCar->velocity.z - pCar->velocityNext.z;
    dForward = FixVecDot(&acc, &pCar->forward);
    angForward = (short)FixDiv(dForward, 0x1e0000);

    // Short from the object's timing record: the target yaw.
    tableAng = **(short **)(g_unk0x0058d4f0 + index * 0x1c + 0xc);

    // Eases each angle a fifth of the way towards its target.
    recOff = index * 0x24;
    i = (int)angForward - (int)*(short *)(g_unk0x0058d368 + recOff + 2);
    *(short *)(g_unk0x0058d368 + recOff + 2) += (short)(i / 5);
    i = (int)tableAng - (int)*(short *)(g_unk0x0058d368 + recOff + 4);
    *(short *)(g_unk0x0058d368 + recOff + 4) += (short)(i / 5);
    i = (int)angRight - (int)*(short *)(g_unk0x0058d368 + recOff + 6);
    *(short *)(g_unk0x0058d368 + recOff + 6) += (short)(i / 5);

    // The two dwords written are (x, y) at +2 and (z, pad) at +6.
    if (g_unk0x0058d3b0[index] != 0) {
        *(unsigned int *)(g_unk0x0058d368 + recOff + 2) =
            ((unsigned int)(unsigned short)tableAng << 16) | (unsigned short)angForward;
        *(unsigned int *)(g_unk0x0058d368 + recOff + 6) = (unsigned short)angRight;
    }

    SceneNode_SetRotation(*(SceneNode **)(g_unk0x0058d530 + index * 0x1c + 4),
                          (FixAngles *)(g_unk0x0058d368 + recOff + 2));

    // Pose offset from the body axes, clamped and eased towards its target.
    offX = FixDiv(-dRight, 0x4ccc);
    if (offX < -0x28f)
        offX = -0x28f;
    else if (offX > 0x1999)
        offX = 0x1999;
    offZ = FixDiv(-dForward, 0x4ccc);
    if (offZ < -0xf5c)
        offZ = -0xf5c;
    else if (offZ > 0xf5c)
        offZ = 0xf5c;

    offY = 0;
    if (g_unk0x0058d3b0[index] == 0) {
        offX = FixMul(offX - *(int *)(g_unk0x0058d374 + recOff), 0xa3d);
        offY = FixMul(-*(int *)(g_unk0x0058d374 + recOff + 4), 0xa3d);
        offZ = FixMul(offZ - *(int *)(g_unk0x0058d374 + recOff + 8), 0xa3d);
        *(int *)(g_unk0x0058d374 + recOff) += offX;
        *(int *)(g_unk0x0058d374 + recOff + 4) += offY;
        *(int *)(g_unk0x0058d374 + recOff + 8) += offZ;
    } else {
        *(int *)(g_unk0x0058d374 + recOff) = offX;
        *(int *)(g_unk0x0058d374 + recOff + 4) = 0;
        *(int *)(g_unk0x0058d374 + recOff + 8) = offZ;
        g_unk0x0058d3b0[index] = 0;
    }

    pos.x = *(int *)(g_unk0x0058d2f8 + index * 0xc) + *(int *)(g_unk0x0058d374 + recOff);
    pos.y = *(int *)(g_unk0x0058d2f8 + index * 0xc + 4) + *(int *)(g_unk0x0058d374 + recOff + 4);
    pos.z = *(int *)(g_unk0x0058d2f8 + index * 0xc + 8) + *(int *)(g_unk0x0058d374 + recOff + 8);
    if (CGameInfo::FUN_004063f0(6) != 0) {
        pCar = Car_Get(index);
        pos.y += FixMul(pCar->field_0xa8c, 0x8000);
    }
    SceneNode_SetPosition(*(SceneNode **)(g_unk0x0058d530 + index * 0x1c + 4), &pos);
}


// ---------------------------------------------------------------------------
// W151.Impl46a500_46acb0: callees that are not declared in any header used by
// this translation unit yet.
// ---------------------------------------------------------------------------
void Car_SpawnDebris(int size, FixVector *pPos, Car *pCar, FixVector *pAxes, int count, int glassChance);
void ForceFeedback_UpdateSlot(BYTE *pCar, FixVector *pIn, int nonzero);
void FUN_00418c30(unsigned int view, int volume, char heavy, int listener);
// g_deformImpactTicks (definida en StageTiming.cpp) y g_unk0x00511310 (SceneNode.cpp):
// su anotacion // GLOBAL: vive en su fichero de definicion; aqui solo el extern.
extern int g_deformImpactTicks[2];
extern const double g_unk0x00511310;

// Car impact update: checks the four axle travel limits, samples the current
// suspension extremes per object type and, when one of this car's objects is
// hit above 0x4ccc units/sec, builds the contact frame (forward / lateral /
// up) around the hit point, spawns the debris, adds the impact damage to the
// object record and refreshes the impact sound / force-feedback state.
// match 60%: implementada (contacto de impacto: marco forward/lateral/up, debris,
// dano y sonido); difiere el reparto de registros del bloque de normalizacion
// FUNCTION: CMR2 0x0046a500
void FUN_0046a500(int param_1)
{
    int *pParts;
    int i;
    int iBig;
    int iFlagC;
    int idx;
    int dot;
    int speed;
    int len;
    BOOL bVar5;
    BOOL bVar14;
    unsigned int uVar15;
    unsigned int uVar16;
    FixVector vDir;
    FixVector vSide;
    FixVector vUp;
    FixVector vHit;
    FixVector *pvAxes;

    iFlagC = 0;
    pParts = FUN_00469680((int)*(char *)(param_1 + 0xb1a));
    bVar5 = FALSE;
    bVar14 = TRUE;
    i = 0;
    do {
        int limit;

        limit = *(int *)(param_1 + 0x988 + i * 4);
        if (limit < 0x51e)
            limit = 0x51e;
        if (-limit <= *(int *)(param_1 + 0x9d8 + i * 4)) {
            bVar14 = FALSE;
            i = 4;
        }
        i++;
    } while (i < 4);
    iBig = 0;
    if (bVar14) {
        int *p;
        int n;

        bVar5 = TRUE;
        iBig = 0;
        p = (int *)(param_1 + 0x9c8);
        n = 4;
        do {
            int v;

            v = *p;
            if (v < 0)
                v = -v;
            if (0xd91 < v)
                iBig = 1;
            p++;
            n--;
        } while (n != 0);
    }
    if (*(int *)(param_1 + 0xb74) == 0)
        return;
    if (iBig == 0) {
        if (bVar5)
            ForceFeedback_UpdateSlot((BYTE *)param_1, (FixVector *)(param_1 + 0x408), 0);
        return;
    }
    bVar5 = FALSE;
    idx = 0;
    do {
        if (StageObject_IsEligibleType((short)*(unsigned short *)(param_1 + 0xaae + idx * 2), 0,
                                       (int)*(unsigned char *)(param_1 + 0xb29)) != 0) {
            bVar5 = TRUE;
            idx = 4;
        }
        idx++;
    } while (idx < 4);
    if (bVar5 && 0x4ccc < *(int *)(param_1 + 0x778)) {
        iFlagC = 1;
        pvAxes = (FixVector *)(param_1 + 0x48c);
        vDir.x = FixMul(*(int *)(param_1 + 0x408), -0x10000);
        vDir.y = FixMul(*(int *)(param_1 + 0x40c), -0x10000);
        vDir.z = FixMul(*(int *)(param_1 + 0x410), -0x10000);
        dot = FixVecDot(pvAxes, &vDir);
        vSide.x = FixMul(pvAxes->x, dot);
        vSide.y = FixMul(pvAxes->y, dot);
        vSide.z = FixMul(pvAxes->z, dot);
        vDir.x -= vSide.x;
        vDir.y -= vSide.y;
        vDir.z -= vSide.z;
        len = FixVecLength(&vDir);
        if (len == 0) {
            vDir.x = 0;
            vDir.y = 0;
            vDir.z = 0;
        } else {
            FixVecScaleRecip(&vDir, &vDir, len);
        }
        FixVecCross(&vSide, &vDir, pvAxes);
        len = FixVecLength(&vSide);
        if (len == 0) {
            vSide.x = 0;
            vSide.y = 0;
            vSide.z = 0;
        } else {
            FixVecScaleRecip(&vSide, &vSide, len);
        }
        FixVecCross(&vUp, &vSide, &vDir);
        len = FixVecLength(&vUp);
        if (len == 0) {
            vUp.x = 0;
            vUp.y = 0;
            vUp.z = 0;
        } else {
            FixVecScaleRecip(&vUp, &vUp, len);
        }
        FixVecScale(&vUp, &vUp, 0x8000);
        vHit.x = FixMul(*(int *)(param_1 + 0x270) - *(int *)(param_1 + 0x294), 0x8000) +
                 *(int *)(param_1 + 0x294) - *(int *)(param_1 + 0x2d0);
        vHit.y = FixMul(*(int *)(param_1 + 0x274) - *(int *)(param_1 + 0x298), 0x8000) +
                 *(int *)(param_1 + 0x298) - *(int *)(param_1 + 0x2d4);
        vHit.z = FixMul(*(int *)(param_1 + 0x278) - *(int *)(param_1 + 0x29c), 0x8000) +
                 *(int *)(param_1 + 0x29c) - *(int *)(param_1 + 0x2d8);
        Car_SpawnDebris(*(int *)(param_1 + 0x778), &vHit, (Car *)param_1, &vDir, 0x1e0000, 0);
        speed = *(int *)(param_1 + 0x778);
        if (speed > 0x10000)
            speed = 0x10000;
        *(int *)((BYTE *)pParts + 0x22c) += FixMul(speed, 0x28f);
        FUN_00468c10((Car *)param_1);
    }
    uVar15 = RallyDataState();
    if ((int)*(char *)(param_1 + 0xb1a) < (int)(uVar15 & 0xff)) {
        i = (int)*(char *)(param_1 + 0xb1a);
        uVar16 = CMain::GetFrameDelta();
        if (0x19 < (unsigned int)(uVar16 - g_deformImpactTicks[i]) && *(int *)(param_1 + 0x778) > 0) {
            if (*(int *)(param_1 + 0x778) <= 0x10000)
                uVar15 = (unsigned int)FixMul(*(int *)(param_1 + 0x778), 0x5c28);
            else
                uVar15 = 0x5c28;
            i = (int)*(char *)(param_1 + 0xb1a);
            FUN_00418c30((unsigned int)i, (int)uVar15, (char)iFlagC, i);
            uVar16 = CMain::GetFrameDelta();
            g_deformImpactTicks[(int)*(char *)(param_1 + 0xb1a)] = (int)uVar16;
        }
    }
    ForceFeedback_UpdateSlot((BYTE *)param_1, (FixVector *)(param_1 + 0x408), 0);
    *(int *)(param_1 + 0x408) = FixMul(*(int *)(param_1 + 0x408), 0xf851);
    *(int *)(param_1 + 0x40c) = FixMul(*(int *)(param_1 + 0x40c), 0xf851);
    *(int *)(param_1 + 0x410) = FixMul(*(int *)(param_1 + 0x410), 0xf851);
}

// Rebuilds the per-part bounding box of a stage object record for one car:
// converts the packed integer vertices of every part to floats, tracks the
// per-part min/max in 16.16 units, expands the record's global x/z bounds and
// stores each part centre at +0xb4 and its half extents at +0xf0.
// match 11%: implementada (caja envolvente por pieza: vertices a float, min/max
// 16.16 por pieza, centro en +0xb4 y semiejes en +0xf0); el codegen de la
// conversion float y del bucle de vertices diverge mucho del original
// FUNCTION: CMR2 0x0046acb0
void FUN_0046acb0(int param_1, int param_2, int param_3)
{
    int i;
    int count;
    int matchIdx;
    int idx;
    int key;
    int n;
    int rOff;
    int fOff;
    int maxX;
    int maxY;
    int maxZ;
    int minX;
    int minY;
    int minZ;
    int fx;
    int fy;
    int fz;
    int *pRec;
    int *pDst;
    int *pVertRecs;
    float *pFloats;
    FixVector extents;

    if (g_unk0x00588970[param_1] == 0)
        return;
    if (g_unk0x00588b9c[param_1] == 0)
        FUN_0046afe0(param_1, param_2, param_3);
    i = 0;
    count = *(int *)(param_3 + 0x45c);
    if (count > 0) {
        pDst = (int *)(param_3 + 0xb4);
        pRec = (int *)(param_3 + 0x78);
        do {
            key = *(int *)(*(int *)(param_3 + 0x3c + i * 4) + 0x30) & 0xff;
            matchIdx = -1;
            idx = 0;
            if (count >= 0) {
                do {
                    int next;

                    next = idx;
                    if ((int)((BYTE *)g_unk0x00588ba0[param_1])[idx] == key) {
                        next = count;
                        matchIdx = idx;
                    }
                    idx = next + 1;
                } while (idx <= count);
                if (matchIdx >= 0) {
                    pRec[0] = ((int *)g_unk0x00588b9c[param_1])[matchIdx];
                    maxX = -0x640000;
                    maxY = -0x640000;
                    maxZ = -0x640000;
                    minX = 0x640000;
                    minY = 0x640000;
                    minZ = 0x640000;
                    if (pRec[0xea] > 0) {
                        rOff = 0;
                        fOff = 0;
                        n = 0;
                        do {
                            pVertRecs = (int *)(rOff + pRec[0]);
                            pFloats = (float *)(*(int *)(*(int *)(param_3 + i * 4) + 0xc) + fOff);
                            pFloats[0] = (float)(pVertRecs[0] * g_unk0x00511310);
                            pFloats[1] = (float)(pVertRecs[1] * g_unk0x00511310);
                            pFloats[2] = (float)(pVertRecs[2] * g_unk0x00511310);
                            pVertRecs = (int *)(rOff + 0xc + pRec[0]);
                            pFloats[3] = (float)(pVertRecs[0] * g_unk0x00511310);
                            pFloats[4] = (float)(pVertRecs[1] * g_unk0x00511310);
                            pFloats[5] = (float)(pVertRecs[2] * g_unk0x00511310);
                            fx = (int)(__int64)(pFloats[0] * CGraphics::m_65536);
                            fy = (int)(__int64)(pFloats[1] * CGraphics::m_65536);
                            fz = (int)(__int64)(pFloats[2] * CGraphics::m_65536);
                            if (maxX < fx)
                                maxX = fx;
                            if (fx < minX)
                                minX = fx;
                            if (maxY < fy)
                                maxY = fy;
                            if (fy < minY)
                                minY = fy;
                            if (maxZ < fz)
                                maxZ = fz;
                            if (fz < minZ)
                                minZ = fz;
                            if (fx < 0) {
                                if (fx < *(int *)(param_3 + 0x414))
                                    *(int *)(param_3 + 0x414) = fx;
                            } else if (*(int *)(param_3 + 0x410) < fx) {
                                *(int *)(param_3 + 0x410) = fx;
                            }
                            if (fz < 0) {
                                if (fz < *(int *)(param_3 + 0x41c))
                                    *(int *)(param_3 + 0x41c) = fz;
                            } else if (*(int *)(param_3 + 0x418) < fz) {
                                *(int *)(param_3 + 0x418) = fz;
                            }
                            n++;
                            rOff += 0x20;
                            fOff += 0x30;
                        } while (n < pRec[0xea]);
                    }
                    extents.x = minX - maxX;
                    extents.y = minY - maxY;
                    extents.z = minZ - maxZ;
                    FixVecScale(&extents, &extents, 0x8000);
                    pDst[0] = extents.x + maxX;
                    pDst[1] = extents.y + maxY;
                    pDst[2] = extents.z + maxZ;
                    pDst[0x2d] = maxX - pDst[0];
                    pDst[0x2e] = maxY - pDst[1];
                    pDst[0x2f] = maxZ - pDst[2];
                }
            }
            i++;
            count = *(int *)(param_3 + 0x45c);
            pRec++;
            pDst += 3;
        } while (i < count);
    }
}

// Constantes de cuantizacion de los registros de luz (compartidas con
// NetRace.cpp). Las de NetRace/Graphics/Car/GameInfo ya tienen su anotacion
// // GLOBAL: en su fichero de definicion; aqui solo van los extern.
extern const float g_netZero;
extern const float g_netElevationScale;
extern const float g_netByteScale;
extern const float g_netHeadingScale;
extern const float g_netSignedShortScale;
extern const float g_netMinusOne;
extern const float g_netOne;
extern float g_65536f;
extern double g_unk0x00511300;
extern const float g_unk0x00511358;   // defined in NetRace.cpp (single definition)
extern const float g_unk0x0051135c;   // defined in NetRace.cpp (single definition)
extern const float g_unk0x00511368;   // defined in NetRace.cpp (single definition)
extern const float g_unk0x0051137c;   // defined in NetRace.cpp (single definition)

// Rebuilds the per-frame light/colour record of a car from its render matrix:
// the car position relative to the sector it stands in is packed as a signed
// 16.16 angle pair into param_2[1], the sector index into param_2[2], the two
// basis vectors (right/forward) are converted with the atan2/acos tables into
// the two colour bytes of param_2[2] / param_2[3], and the light intensity and
// entity/flag bits are gathered into param_2[3].
// match 45%: implementada; difiere el codegen de la conversion a angulo empaquetado
// (FixAtan2/FixAcos por tablas) y el reparto de registros del bloque de color
// FUNCTION: CMR2 0x0046d8d0
void FUN_0046d8d0(int param_1, int *param_2)
{
    FixMatrix *pMatrix = *(FixMatrix **)(param_1 + 0x750);
    FixVector pos;
    FixVector vec[2];
    Sector *pSector;
    float fX;
    float fZ;
    int i;
    int r;

    FixMatrix_GetPosition(&pos, pMatrix);
    *param_2 = pos.y;
    param_2[2] = (param_2[2] & 0xffff0000) | (*(unsigned short *)(param_1 + 0xb00) & 0xffff);

    pSector = g_sectors[*(short *)(param_1 + 0xb00)];
    pos.x -= pSector->x;
    pos.y -= pSector->y;
    pos.z -= pSector->z;

    // 0.0078125f is the original constant at 0x00511338, declared as the private
    // CGraphics member m_oneOver128 (1.0f / 128.0f).
    fX = (float)((double)pos.x * CGraphics::m_oneOver65536 * 0.0078125f);
    fZ = (float)((double)pos.z * CGraphics::m_oneOver65536 * 0.0078125f);

    if (fX < g_netOne) {
        if (fX <= g_netMinusOne) {
            param_2[1] = (param_2[1] & 0xffff8003) | 0x8003;
        } else {
            r = (int)(__int64)(fX * g_netSignedShortScale);
            param_2[1] = (param_2[1] & 0xffff0000) | (r & 0xffff);
        }
    } else {
        param_2[1] = (param_2[1] & 0xffff7ffd) | 0x7ffd;
    }

    if (fZ < g_netOne) {
        if (g_netMinusOne < fZ) {
            r = (int)(__int64)(fZ * g_netSignedShortScale);
            param_2[1] = (r << 16) | (param_2[1] & 0xffff);
        } else {
            param_2[1] = (param_2[1] & 0xffff) | 0x80030000;
        }
    } else {
        param_2[1] = (param_2[1] & 0xffff) | 0x7ffd0000;
    }

    FixMatrix_GetRight(&vec[0], pMatrix);
    FixMatrix_GetForward(&vec[1], pMatrix);

    for (i = 0; i < 2; i++) {
        int vx = vec[i].x;
        int vy = vec[i].y;
        int vz = vec[i].z;
        int ax = vx < 0 ? -vx : vx;
        int ay = vy < 0 ? -vy : vy;
        int az = vz < 0 ? -vz : vz;
        int ang1;
        int ang2;
        int v2;
        float a1;
        double a2;

        ang1 = (ax == 0) ? 0 : (int)FixAtan2(az, ax) * 0x1680;

        if (vx < 0) {
            if (vz < 0)
                ang1 += 0xb40000;
            else
                ang1 = 0xb40000 - ang1;
        } else if (vz > 0) {
            if (vx <= 0)
                ang1 = 0xb40000 - ang1;
        } else {
            ang1 = 0x1680000 - ang1;
        }

        ang2 = FixAcos(ay);
        v2 = (0x400 - ang2) * 0x1680;
        if (vy <= 0)
            v2 = 0xb40000 - v2;

        a1 = (float)ang1 * (float)CGraphics::m_oneOver65536 * g_netHeadingScale * g_netByteScale;
        a2 = (double)v2 * CGraphics::m_oneOver65536 * g_netElevationScale * g_netByteScale;

        if (a1 < g_netZero)
            a1 = 0.0f;
        else if (a1 > g_netByteScale)
            a1 = g_netByteScale;

        if (a2 < g_netZero)
            a2 = 0.0f;
        else if (a2 > g_netByteScale)
            a2 = g_netByteScale;

        if (i == 0) {
            r = ((int)(__int64)a1 & 0xff) | ((int)(__int64)a2 << 8);
            param_2[2] = (r << 16) | (param_2[2] & 0xffff);
        } else {
            param_2[3] = (param_2[3] & 0xffff0000) |
                         ((((int)(__int64)a2 & 0xff) << 8) | ((int)(__int64)a1 & 0xff));
        }
    }

    {
        int num = *(short *)(param_1 + 0xb10) * 0x1680;
        int den = *(short *)(param_1 + 0xb16) * 0x1680;
        int level = FixDiv(num, den) + 0x10000;
        int flag;

        level = FixMul(level, 0xf8000);
        if (level < 0)
            level = 0;
        else if (level > 0x1f0000)
            level = 0x1f0000;
        param_2[3] = (param_2[3] & 0xffe0ffff) | ((level >> 16 & 0x1f) << 16);

        if (*(int *)(param_1 + 0x79c) != 0)
            param_2[3] |= 0x2000000;
        else
            param_2[3] &= 0xfdffffff;
        param_2[3] = (param_2[3] & 0xfeffffff) | ((*(unsigned int *)(param_1 + 0xb54) & 1) << 0x18);
        param_2[3] = (param_2[3] & 0xf7ffffff) | ((*(unsigned int *)(param_1 + 0xc14) & 1) << 0x1b);
        *(int *)(param_1 + 0xc14) = 0;
        param_2[3] = (param_2[3] & 0xff1fffff) | ((*(unsigned char *)(param_1 + 0xb46) & 7) << 0x15);
        *(unsigned char *)(param_1 + 0xb46) = 0;

        flag = 0;
        if (*(int *)(param_1 + 0x724) != 0 && *(char *)(*(int *)(param_1 + 0x724) + 0x17c) != 0)
            flag = 1;
        if (*(int *)(param_1 + 0x720) != 0 && *(char *)(*(int *)(param_1 + 0x720) + 0x17c) != 0)
            flag = 1;
        param_2[3] = (param_2[3] & 0xfbffffff) | (flag << 0x1a);
    }
}

// Builds the world matrix of one light/effect record: the position comes from
// the record's quantised offset inside its sector, the right/forward basis is
// decoded from the two packed byte angles (12-bit table sin/cos) and the up
// vector from their cross product, and the record's flag/intensity word is
// decoded into the six output pointers.
// match 29%: implementada; los senos/cosenos de tabla y el armado de la matriz
// se reparten distinto (mismo resultado funcional)
// FUNCTION: CMR2 0x0046de20
void FUN_0046de20(unsigned int *param_1, unsigned int *param_2, unsigned int *param_3, int *param_4,
                  FixMatrix *param_5, int *param_6, unsigned int *param_7, int *param_8)
{
    FixVector pos;
    FixVector basis[2];
    FixVector up;
    Sector *pSector;
    int i;

    {
        float offX = (float)(int)(short)(param_8[1] & 0xffff) * g_unk0x0051137c * g_unk0x00511368;
        float offZ = (float)(int)(short)((unsigned int)param_8[1] >> 16) * g_unk0x0051137c * g_unk0x00511368;

        pos.x = (int)(offX * g_65536f);
        pos.z = (int)(offZ * g_65536f);
    }
    pos.y = param_8[0];

    pSector = g_sectors[(short)param_8[2]];
    pos.x += pSector->x;
    pos.z += pSector->z;
    FixMatrix_SetPosition(&pos, param_5);

    for (i = 0; i < 2; i++) {
        unsigned int bV = (i == 0) ? ((unsigned int)param_8[2] >> 24)
                                   : (((unsigned int)param_8[3] >> 8) & 0xff);
        unsigned int bU = (i == 0) ? (((unsigned int)param_8[2] >> 16) & 0xff)
                                   : ((unsigned int)param_8[3] & 0xff);
        float v = (float)bV * g_unk0x0051135c;
        float u = (float)bU * g_unk0x00511358;
        short angA = (short)(__int64)((double)(int)(v * g_65536f) * g_unk0x00511300);
        short angB = (short)(__int64)((double)(int)(u * g_65536f) * g_unk0x00511300);

        basis[i].x = FixMul(FixSin(angB), FixCos(angA));
        basis[i].y = FixCos(angB);
        basis[i].z = FixMul(FixSin(angB), FixSin(angA));
        FIX_NORMALIZE_INTO(basis[i], basis[i]);
    }

    up.x = FixMul(basis[1].y, basis[0].z) - FixMul(basis[1].z, basis[0].y);
    up.y = FixMul(basis[1].z, basis[0].x) - FixMul(basis[1].x, basis[0].z);
    up.z = FixMul(basis[1].x, basis[0].y) - FixMul(basis[1].y, basis[0].x);
    FIX_NORMALIZE_INTO(up, up);

    FixMatrix_SetRight(&basis[0], param_5);
    FixMatrix_SetUp(&up, param_5);
    FixMatrix_SetForward(&basis[1], param_5);

    *param_4 = FixMul((((unsigned int)param_8[3] >> 16) & 0x1f) << 16, 0x1083) - 0x10000;
    *param_3 = (unsigned int)param_8[3] >> 9 & 0x10000;
    *param_1 = (unsigned int)param_8[3] >> 0x18 & 1;
    *param_2 = (unsigned int)param_8[3] >> 0x1b & 1;
    *param_6 = FixMul((((unsigned int)param_8[3] >> 0x15) & 7) << 16, 0x2000);
    *param_7 = (unsigned int)param_8[3] >> 0x1a & 1;
}

void Glow_SetPosition(GlowLight *pLight, FixVector *pPos, FixVector *pDir);
void Glow_SetLayerPlane(GlowLight *pLight, FixVector *pPoint, FixVector *pNormal, int layerIntensity);
void FUN_0045a150(int texture, int side, int car);
void FUN_0045b530(int texture, int side, int car);
void FUN_00466870(int *pA, int *pB, Car *pCar);
int FUN_004789d0(int surface, int t);
SceneNode *Scene_CreateLight(int type, int r, int g, int b, FixVector *pPosition, FixAngles *pAngles,
                             SceneNode *pParent);

// GLOBAL: CMR2 0x00547ce0
int g_unk0x00547ce0[8];
// GLOBAL: CMR2 0x00547d00
int g_unk0x00547d00[0x28];

// Rebuilds the two light meshes of a car. First a glow light is created for
// every collision point of the car (the (object, vertex) slot is stored into
// the point afterwards), then for every object vertex each collision point
// keeps the object/vertex index of the closest vertex.
// match 29%: implementada (mallas de luces por punto de colision + asignacion
// del vertice mas cercano); difiere el codegen de los productos escalares 16.16
// FUNCTION: CMR2 0x00463fe0
void FUN_00463fe0(int param_1)
{
    int car;
    int count;
    int n;
    int off;
    int k;
    int i;
    int j;
    int side;
    int slot;
    int glow;
    int node;
    int projected;
    int unused1;
    int minDot[20];
    int *pRec;
    int *pObj;
    int *pPoints;
    int *pPoint;
    int *pVertex;
    int *pMin;
    FixVector position;
    FixAngles angles;

    car = *(char *)(param_1 + 0xb1a);
    unused1 = 0;
    angles.x = 0;
    angles.y = 0;
    angles.z = 0xff1d;
    angles.pad = 0;
    if (car == 0) {
        g_unk0x00547fe0.x = 0x1e0000;
        g_unk0x00547fe0.y = 0;
        g_unk0x00547fe0.z = 0;
        g_unk0x00547fa0.x = 0xf0000;
        g_unk0x00547fa0.y = 0;
        g_unk0x00547fa0.z = 0;
        position.x = 0;
        position.y = 0;
        position.z = 0;
        g_unk0x00547fec = Scene_CreateLight(0, 0x140000, 0x140000, 0x140000, &position, &angles,
                                            (SceneNode *)RallyData_FUN_00411060());
        g_unk0x00547ff0 = Scene_CreateLight(0, 0x140000, 0x140000, 0x140000, &position, &angles,
                                            (SceneNode *)RallyData_FUN_00411060());
    }
    ((int *)g_carLights)[car] = FUN_00457e10((BYTE *)param_1, 0);
    side = 0;
    ((int *)g_carLights)[0x10 + car] = ((int *)g_carLights)[car] + 4;
    count = *((int *)((int *)g_carLights)[car]);
    pPoints = (int *)((int *)g_carLights)[0x10 + car];
    n = 0;
    if (0 < count) {
        pMin = minDot;
        off = 0;
        do {
            pPoint = (int *)((BYTE *)pPoints + off);
            slot = *(BYTE *)((BYTE *)pPoint + 0x21);
            if (FUN_0046b4c0((BYTE *)param_1) == 0 || *(char *)((BYTE *)pPoint + 0x22) == -1) {
                node = *(int *)(param_1 + 0x720);
            } else {
                node = (int)FUN_00484de0((BYTE *)param_1, (int)*(char *)((BYTE *)pPoint + 0x22));
                if (node == 0)
                    node = *(int *)(param_1 + 0x720);
            }
            FUN_004a3e20((Unk0x004a3e20 *)((int *)g_carLights)[0xb + slot], 1);
            FUN_004a3e20((Unk0x004a3e20 *)((int *)g_carLights)[0x1e + slot], 1);
            unused1 = 0x10000;
            projected = (*(char *)((BYTE *)pPoint + 0x20) == '\t') ? 0xff : 0;
            position.x = 0;
            position.y = 0;
            position.z = 0;
            glow = (int)Glow_Add(2, (FixVector *)pPoint, (FixVector *)((BYTE *)pPoint + 0xc),
                                 (int)&unused1, *(int *)((BYTE *)pPoint + 0x18),
                                 *(int *)((BYTE *)pPoint + 0x18), ((int *)g_carLights)[0xb + slot],
                                 ((int *)g_carLights)[0x1e + slot], *(int *)((BYTE *)pPoint + 0x1c),
                                 node, (BYTE)projected, (int)&position, 0x10000);
            g_unk0x00547d00[n + car * 0x14] = glow;
            FUN_004ae3d0((BYTE *)glow, 1);
            *pMin = 0x3e80000;
            if (*(char *)((BYTE *)pPoint + 0x20) == '\t') {
                FUN_0045a150((int)pPoint, side, car);
                FUN_0045b530(glow, side, car);
                side++;
            }
            pMin++;
            off += 0x28;
            n++;
        } while (n < count);
    }
    if (FUN_0046b4c0((BYTE *)param_1) != 0) {
        pRec = FUN_00469680(car);
        if (0 < *(int *)((BYTE *)pRec + 0x45c)) {
            i = 0;
            pObj = (int *)((BYTE *)pRec + 0x420);
            do {
                if (0 < *pObj) {
                    j = 0;
                    do {
                        pMin = minDot;
                        off = 0;
                        if (0 < count) {
                            k = 0;
                            do {
                                int dx;
                                int dy;
                                int dz;
                                int dot;

                                pPoint = (int *)((BYTE *)pPoints + off);
                                pVertex = (int *)(*(int *)((BYTE *)pRec + 0x78 + i * 4) + j * 0x20);
                                dx = pPoint[0] - pVertex[0];
                                dy = pPoint[1] - pVertex[1];
                                dz = pPoint[2] - pVertex[2];
                                dot = FixMul(dx, dx) + FixMul(dy, dy) + FixMul(dz, dz);
                                if (dot < *pMin) {
                                    *pMin = dot;
                                    *(short *)((BYTE *)pPoint + 0x24) = (short)i;
                                    *(short *)((BYTE *)pPoint + 0x26) = (short)j;
                                }
                                pMin++;
                                off += 0x28;
                                k++;
                            } while (k < count);
                        }
                        j++;
                    } while (j < *pObj);
                }
                i++;
                pObj++;
            } while (i < *(int *)((BYTE *)pRec + 0x45c));
        }
    }
    g_unk0x00547ff4 = 0xffff0000;
    for (i = 0; i < 8; i++)
        g_unk0x00547ce0[i] = 0;
}

// Drives the two light flag bytes of a car (brake/reverse/hazard/head) and
// updates the glow light of every collision point: it enables/fades it from
// the per-lane tuning table, colours it by the point type and hangs it on the
// car body, or disables every glow when the car is not racing.
// match 18%: implementada (flags de luces del coche + glow por punto de colision);
// el codegen de la tabla de ajuste por carril y de las llamadas a FUN_00477c20 difiere
// FUNCTION: CMR2 0x004643f0
void FUN_004643f0(int param_1)
{
    int car;
    int n;
    int off;
    int glow;
    int slot;
    int lights[11];
    int *pRec;
    int *pPoint;
    FixVector planePos;

    car = *(char *)(param_1 + 0xb1a);
    pRec = FUN_00469680(car);
    if (*(int *)(param_1 + 0xb70) == 0) {
        int vA;
        int vB;

        FUN_00466870(&vA, &vB, (Car *)param_1);
        if (*(int *)(param_1 + 0xb54) == 0)
            FUN_00477c20(car, 0, 0, 2);
        else
            FUN_00477c20(car, vA == 0, vB == 0, 2);
        if (*(int *)(param_1 + 0xb5c) == 0)
            FUN_00477c20(car, 0, 0, 8);
        else
            FUN_00477c20(car, vA == 0, vB == 0, 8);
        if (*(int *)(param_1 + 0xb58) == 0) {
            FUN_00477c20(car, 0, 0, 1);
            FUN_00477c20(car, 0, 0, 0x10);
        } else {
            FUN_00477c20(car, vA == 0, vB == 0, 1);
            FUN_00477c20(car, 1, 1, 0x10);
        }
        if (0x4ccc < *(int *)((BYTE *)pRec + 0x29c)) {
            int old = g_unk0x00547ce0[car];

            g_unk0x00547ce0[car] = old + g_unk0x0051bd3c;
            if (0x140000 < g_unk0x00547ce0[car])
                g_unk0x00547ce0[car] = 0;
            if (g_unk0x00547ce0[car] == 0)
                FUN_00477c20(car, 0, 0, 4);
            if (old < 0xa0000 && 0x9ffff < g_unk0x00547ce0[car])
                FUN_00477c20(car, vA == 0, vB == 0, 4);
        }
        FUN_00477c80(car, &lights[0], &lights[1], 1);
        FUN_00477c80(car, &lights[2], &lights[3], 3);
        FUN_00477c80(car, &lights[4], &lights[5], 2);
        FUN_00477c80(car, &lights[6], &lights[7], 0);
        FUN_00477c80(car, &lights[8], &lights[8], 4);
        lights[10] = 0;
        lights[9] = 0;
        planePos.x = *(int *)(param_1 + 0x270);
        planePos.y = *(int *)(param_1 + 0x8dc);
        planePos.z = *(int *)(param_1 + 0x278);
        n = 0;
        if (0 < *((int *)((int *)g_carLights)[car])) {
            off = 0;
            do {
                pPoint = (int *)((BYTE *)((int *)g_carLights)[0x10 + car] + off);
                glow = g_unk0x00547d00[n + car * 0x14];
                if (glow != 0) {
                    slot = *(BYTE *)((BYTE *)pPoint + 0x20);
                    if (lights[slot] == 0) {
                        FUN_004ae3d0((BYTE *)glow, 0);
                    } else {
                        int c;
                        int v0;
                        int v1;
                        int v2;

                        FUN_004ae3d0((BYTE *)glow, 1);
                        c = FixMul(FixMul(*(int *)((BYTE *)pPoint + 0x1c), lights[slot]), 0x8000);
                        FUN_004ae3f0((BYTE *)glow,
                                     FixMul(*(int *)((BYTE *)pPoint + 0x1c), lights[slot]));
                        switch (*(char *)((BYTE *)pPoint + 0x21)) {
                        case 0:
                            v0 = 0xe000;
                            v1 = 0x1c28;
                            v2 = 0;
                            break;
                        case 1:
                        case 4:
                            v0 = 0x10000;
                            v1 = 0x10000;
                            v2 = 0x10000;
                            break;
                        default:
                            v0 = 0x10000;
                            v1 = 0x9893;
                            v2 = 0;
                            break;
                        }
                        v0 = FixMul(v0, c);
                        v1 = FixMul(v1, c);
                        v2 = FixMul(v2, c);
                        FUN_004ae410(glow, v0, v1, v2);
                    }
                    {
                        int idx = 0;
                        int t;

                        if (*(int *)(glow + 4) < 0)
                            idx = 2;
                        if (*(int *)(glow + 0xc) < 0)
                            idx++;
                        t = FUN_004789d0(*(short *)(param_1 + 0xaae + idx * 2),
                                         FUN_00460c80((BYTE *)param_1));
                        Glow_SetLayerPlane((GlowLight *)glow, &planePos,
                                           (FixVector *)(param_1 + 0x48c), t);
                        if (FUN_0046b4c0((BYTE *)param_1) != 0) {
                            int oi = *(unsigned short *)((BYTE *)pPoint + 0x24);
                            int ii = *(unsigned short *)((BYTE *)pPoint + 0x26);
                            int *pVertex =
                                (int *)(*(int *)((BYTE *)pRec + 0x78 + oi * 4) + ii * 0x20);
                            int vx = pVertex[0];
                            int vy = pVertex[1];
                            int vz = pVertex[2];
                            int out[3];
                            FixVector pos;

                            FUN_0046b440((Mesh **)pRec, oi, ii, out);
                            pos.x = (out[0] - vx) + pPoint[0];
                            pos.y = (out[1] - vy) + pPoint[1];
                            pos.z = (out[2] - vz) + pPoint[2];
                            Glow_SetPosition((GlowLight *)glow, &pos,
                                             (FixVector *)((BYTE *)pPoint + 0xc));
                        }
                    }
                }
                n++;
                off += 0x28;
            } while (n < *((int *)((int *)g_carLights)[car]));
        }
    } else {
        FUN_00477c20(car, 0, 0, 2);
        FUN_00477c20(car, 0, 0, 8);
        n = 0;
        if (0 < *((int *)((int *)g_carLights)[car])) {
            do {
                FUN_004ae3d0((BYTE *)g_unk0x00547d00[n + car * 0x14], 0);
                n++;
            } while (n < *((int *)((int *)g_carLights)[car]));
        }
    }
}

// Dependencias de la cadena de 0x46cce0 (0x46c2a0 / 0x46c410 / 0x469690):
// prototipos que no estan en ninguna cabecera incluida por esta unidad.
extern BYTE *g_unk0x00588b98;
void FUN_00466ef0(Car *pCar, int *param_2, FixVector *param_3, int param_4, unsigned char param_5,
                  int param_6);
void FUN_004698a0(int pCar);
void FUN_004692f0(Car *pCar, int param_2);
void FUN_004688b0(BYTE *p);
void FUN_00468c10(Car *pCar);
void FUN_00486630(int list, int index, int value);
void FUN_00480ac0(BYTE *pCar, int slot, int reset);
void FUN_00480b40(BYTE *pCar);
void FUN_004702a0(void);
void FUN_0045e610(void);
void FUN_00458480(void);
void FUN_004584d0(char param_1);
void FUN_00458100(int param_1);
void FUN_0047bdc0(char restart);
void FUN_0042b800(int, int, int);
int *FUN_00469680(int index);
void RallyData_FUN_004207a0(int index);
void RallyData_FUN_004207f0(void);
RaceRecord *RallyData_FUN_00421510(int index);
unsigned int RallyData_FUN_00407e70(void);
unsigned int RallyData_FUN_00407e90(void);
unsigned char RallyDataState(void);

// Resets the per-car stage-object block: clears the pose/timing fields, walks
// the object chain calling the pre-step of every entry, then recomputes the
// 0x22 light intensities from the stored bytes and mirrors three palette
// entries and four geometry offsets.
// match 22%: implementada; difiere el codegen del bucle de la cadena de objetos
// (indice*0xd + base) y de la division 64-bit de las intensidades
// FUNCTION: CMR2 0x00469690
void FUN_00469690(int param_1)
{
    BYTE *pBlock;
    BYTE *pInfo;
    int saved5dc, saved5e0, saved5e4, saved5c4, saved5c8, saved5cc;
    int i;
    int iVar13;
    unsigned int uVar12;
    BYTE bVar1;

    pBlock = g_unk0x00588b94 + *(char *)(param_1 + 0xb1a) * 0x4d0;
    pInfo = g_unk0x00588b98 + *(char *)(param_1 + 0xb1a) * 0x290;
    FUN_00480b40((BYTE *)param_1);
    FUN_004698a0(param_1);
    *(int *)(pBlock + 0x404) = 0x10000;
    *(int *)(pBlock + 0x3fc) = 0x10000;
    *(int *)(pBlock + 0x400) = 0x10000;
    *(int *)(pBlock + 0x3ec) = 0;
    *(int *)(pBlock + 0x3f0) = 0;
    *(int *)(pBlock + 0x3f4) = 0;
    *(int *)(pBlock + 0x3f8) = 0;
    *(int *)(pBlock + 0x408) = 0;
    *(int *)(pBlock + 0x3d8) = 0;
    *(BYTE *)(pBlock + 0x468) = 0;
    saved5dc = *(int *)(param_1 + 0x5dc);
    saved5e0 = *(int *)(param_1 + 0x5e0);
    saved5e4 = *(int *)(param_1 + 0x5e4);
    saved5c4 = *(int *)(param_1 + 0x5c4);
    saved5c8 = *(int *)(param_1 + 0x5c8);
    saved5cc = *(int *)(param_1 + 0x5cc);
    uVar12 = (unsigned int)*(BYTE *)(pInfo + 0x105);
    if (*(char *)(pInfo + 0x104) != '\0') {
        iVar13 = (int)(uVar12 * 0xd) + (int)pInfo;
        if (iVar13 != 0) {
            do {
                FUN_004688b0((BYTE *)iVar13);
                FUN_00466ef0((Car *)param_1, 0, 0, 0, 0, 1);
                if (*(char *)(iVar13 + 0xc) == -1)
                    break;
                uVar12 = (unsigned int)*(char *)(iVar13 + 0xc);
                iVar13 = (int)(uVar12 * 0xd) + (int)pInfo;
            } while (iVar13 != 0);
        }
    }
    *(int *)(param_1 + 0x5dc) = saved5dc;
    *(int *)(param_1 + 0x5e0) = saved5e0;
    *(int *)(param_1 + 0x5e4) = saved5e4;
    *(int *)(param_1 + 0x5c4) = saved5c4;
    *(int *)(param_1 + 0x5c8) = saved5c8;
    *(int *)(param_1 + 0x5cc) = saved5cc;
    for (i = 0; i < 0x22; i++) {
        int tmp;

        bVar1 = *(BYTE *)(pInfo + 0x20c + i);
        tmp = (int)((unsigned int)bVar1 << 0x10);
        *(int *)(pBlock + 0x350 + i * 4) = tmp;
        *(int *)(pBlock + 0x350 + i * 4) = (int)(((__int64)tmp << 0x10) / 0xff0000);
    }
    for (i = 0; i < 3; i++) {
        int v = *(int *)(pInfo + 0x240 + i * 4);

        *(int *)(pBlock + 0x4c0 + i * 4) = v;
        FUN_00486630((int)*(char *)(param_1 + 0xb1a), i, v);
    }
    for (i = 0; i < 4; i++) {
        int v = *(int *)(pInfo + 0x230 + i * 4);

        *(int *)(pBlock + 0x4b0 + i * 4) = v;
        FUN_00480ac0((BYTE *)param_1, i, v);
    }
    FUN_00468c10((Car *)param_1);
    FUN_004692f0((Car *)param_1, 1);
}

// Initialises the stage-object state of one car (list type 0): resets the car's
// per-type block, copies it into the stage block, mirrors the road book entry
// when the car has one, and refreshes the rally-data bookkeeping.
// FUNCTION: CMR2 0x0046c2a0
void FUN_0046c2a0(int param_1, BYTE param_2)
{
    Car *pCar;
    RaceRecord *pRecord;

    pCar = Car_Get((int)param_2);
    FUN_00469690((int)pCar);
    FUN_0046bfd0((Block0x309 *)(param_1 + 0x4d0), pCar);
    if (*(int *)((BYTE *)pCar + 0xb50) != 0) {
        FUN_0046c1a0((Block0x134 *)param_1, (Block0x134 *)FUN_00469680((int)param_2));
    }
    pRecord = RallyData_FUN_00421510((int)param_2);
    FUN_0046c220((Block6 *)(param_1 + 0x10f4), (Block6 *)pRecord);
    RallyData_FUN_004207a0((int)param_2);
    FUN_0045e610();
    FUN_004702a0();
    RallyData_ValidateIndex((int)param_2);
}

// Initialises the stage-object state of one car (list type 2): same reset as
// above but without the per-type block copy or the road book mirror.
// FUNCTION: CMR2 0x0046c410
void FUN_0046c410(int param_1, BYTE param_2)
{
    Car *pCar;

    pCar = Car_Get((int)param_2);
    FUN_00469690((int)pCar);
    RallyData_FUN_004207a0((int)param_2);
    FUN_0045e610();
    FUN_004702a0();
    RallyData_ValidateIndex((int)param_2);
}

// Selects one lane of one car's stage-object state and initialises it for the
// given type: validates the record, stores the lane/timer fields, dispatches to
// the type-specific reset (0x46c2a0 / 0x46c390 / 0x46c410), flushes pending
// events, copies the light byte and, for type 2, rebuilds the two pose matrices
// and the car's matrix/mirror state.
// match 63%: implementada; MSVC6 no emite el `mov eax,1` final (firma void por
// compatibilidad con las llamadas de Race.cpp) y reparte distinto los locales
// FUNCTION: CMR2 0x0046cce0
void FUN_0046cce0(int param_1, int param_2, int param_3, BYTE param_4)
{
    char cVar1;
    BYTE bVar2;
    short sVar4;
    int iVar5;
    unsigned char uVar3;
    short lane;

    if (param_1 == 0 || *(int *)(param_1 + 0xc) != 0 || *(int *)(param_1 + 4) != 0)
        return;
    lane = (short)param_2;
    if (*(short *)(param_1 + 0x100) <= lane)
        return;
    *(BYTE *)(param_1 + 0x20) = (BYTE)param_4;
    *(int *)(param_1 + 4) = 1;
    *(short *)(param_1 + 0x108) = lane;
    *(int *)(param_1 + 8) = 1;
    if ((short)param_3 == 0)
        *(short *)(param_1 + 0x10a) = 0;
    else
        *(short *)(param_1 + 0x10a) = (short)(-(short)param_3);
    *(BYTE *)(param_1 + 0x21) = 0;
    if (*(int *)(param_1 + 0x1c) == 0) {
        FUN_0046c2a0(*(int *)(param_1 + 0x24) + lane * 0x114c, (BYTE)param_4);
    } else if (*(int *)(param_1 + 0x1c) == 1) {
        FUN_0046c390((int *)(lane * 0x5c + *(int *)(param_1 + 0x30)), (BYTE)param_4);
    } else {
        FUN_0046c410(lane * 0x5c + *(int *)(param_1 + 0x30), (BYTE)param_4);
    }
    Events_Flush();
    *(int *)(param_1 + 0x14) = 0;
    if (*(int *)(param_1 + 0x1c) == 0)
        *(BYTE *)(param_1 + 0x10c) = *(BYTE *)(*(int *)(param_1 + 0x24) + 0x1110 + lane * 0x114c);
    else
        *(BYTE *)(param_1 + 0x10c) = *(BYTE *)(lane * 0x5c + 0x20 + *(int *)(param_1 + 0x30));
    if ((BYTE)param_4 == 0 &&
        ((cVar1 = (char)CGameInfo::FUN_00405e00()) != '\0' ||
         ((cVar1 = (char)RallyData_FUN_00407e90()) != '\0' &&
          (cVar1 = (char)CGameInfo::FUN_00405e00()) == '\0'))) {
        RallyData_FUN_004207f0();
        FUN_004584d0(1);
        cVar1 = (char)RallyData_FUN_00407e70();
        if (cVar1 != '\0') {
            bVar2 = (BYTE)RallyDataState();
            sVar4 = Car_GetOrderCount();
            iVar5 = (int)sVar4 - (unsigned int)bVar2;
            uVar3 = (unsigned char)RallyDataState();
            FUN_0042b800((int)uVar3, iVar5, 1);
            FUN_0047bdc0(0);
        }
        FUN_00458480();
        FUN_00458100(0);
    }
    *(BYTE *)(param_1 + 0xf8) = 0;
    if (*(int *)(param_1 + 0x1c) == 2) {
        FUN_0046de20((unsigned int *)(param_1 + 0xec), (unsigned int *)(param_1 + 0xf0),
                     (unsigned int *)(param_1 + 0xd8), (int *)(param_1 + 0xd0),
                     (FixMatrix *)(param_1 + 0x44), (int *)(param_1 + 0xe0),
                     (unsigned int *)(param_1 + 0xe8), (int *)*(int *)(param_1 + 0x40));
        FUN_0046de20((unsigned int *)(param_1 + 0xec), (unsigned int *)(param_1 + 0xf4),
                     (unsigned int *)(param_1 + 0xdc), (int *)(param_1 + 0xd4),
                     (FixMatrix *)(param_1 + 0x84), (int *)(param_1 + 0xe4),
                     (unsigned int *)(param_1 + 0xe8), (int *)(*(int *)(param_1 + 0x40) + 0x10));
        cVar1 = (char)CGameInfo::FUN_00405e00();
        if ((cVar1 != '\0' && *(BYTE *)(param_1 + 0x20) == 0) ||
            (cVar1 = (char)CGameInfo::FUN_00405e00()) == '\0') {
            Car *pCar = Car_Get((int)*(BYTE *)(param_1 + 0x20));

            *(int *)((BYTE *)pCar + 0xc18) = 1;
        }
        {
            Car *pCar = Car_Get((int)*(BYTE *)(param_1 + 0x20));

            *(int *)((BYTE *)pCar + 0x408) = 0;
        }
        {
            Car *pCar = Car_Get((int)*(BYTE *)(param_1 + 0x20));

            *(int *)((BYTE *)pCar + 0x40c) = 0;
        }
        {
            Car *pCar = Car_Get((int)*(BYTE *)(param_1 + 0x20));

            *(int *)((BYTE *)pCar + 0x410) = 0;
        }
        {
            Car *pCar = Car_Get((int)*(BYTE *)(param_1 + 0x20));

            *(int *)((BYTE *)pCar + 0x414) = 0;
        }
        {
            Car *pCar = Car_Get((int)*(BYTE *)(param_1 + 0x20));

            *(int *)((BYTE *)pCar + 0x418) = 0;
        }
        {
            Car *pCar = Car_Get((int)*(BYTE *)(param_1 + 0x20));

            *(int *)((BYTE *)pCar + 0x41c) = 0;
        }
        {
            Car *pCar = Car_Get((int)*(BYTE *)(param_1 + 0x20));

            FUN_0046e340((BYTE *)(param_1 + 0x44), (BYTE *)pCar);
        }
        {
            Car *pCar = Car_Get((int)*(BYTE *)(param_1 + 0x20));

            *(int *)((BYTE *)pCar + 0xbf8) = 1;
        }
        return;
    }
    return;
}
// ===========================================================================
// Stage-object pass, animation and collision code restored for the layer 1
// pass (see CONOCIMIENTO.md). Declarations of helper symbols that live in
// other translation units, hoisted so the functions below build.
// ===========================================================================
// ---- DECLS extras (integrar al principio de StageObjects.cpp si no existen ya) ----
void Car_SpawnDebris(int size, FixVector *pPos, Car *pCar, FixVector *pAxes, int count, int glassChance);
void FUN_0046a500(int param_1);
// ---- DECLS extras (integrar al principio de StageObjects.cpp si no existen ya) ----
// Nota: FUN_0047b0e0 debe colocarse DESPUES de FUN_0047bca0 y de las definiciones
// de g_carButtonMasks / g_unk0x0058e0a0 / g_unk0x0058e0a8 (todas en este fichero).
BYTE FUN_0044a130(void);
int FUN_0041f3d0(BYTE index);
// ---- DECLS extras (integrar al principio de StageObjects.cpp si no existen ya) ----
void FUN_00466ef0(Car *pCar, int *param_2, FixVector *param_3, int param_4,
                  unsigned char param_5, int param_6);
// ---- DECLS extras (integrar al principio de StageObjects.cpp si no existen ya) ----
struct Unk0x00590d74;
extern Unk0x00590d74 *g_unk0x00590d74;
extern int g_unk0x00590c00[8];
extern void **g_unk0x00590c6c;
extern int g_physicsTimeStep;
// ---- GLOBALS nuevos (no existen en el repo) ----
// Per-channel colour gains of the current frame, written by FUN_00485690
// (0x00590c54 = red, 0x00590c58 = green, 0x00590c5c = blue, each clamped to
// 0x10000). Not defined anywhere else in the project yet.
// GLOBAL: CMR2 0x00590c54
int g_unk0x00590c54;
// GLOBAL: CMR2 0x00590c58
int g_unk0x00590c58;
// GLOBAL: CMR2 0x00590c5c
int g_unk0x00590c5c;
// ---- DECLS extras (integrar al principio de StageObjects.cpp si no existen ya) ----
extern FixVector g_unk0x005914a8;
extern FixVector g_unk0x005914b8;
void FUN_004894b0(int *pA, int *pB, int *pDir, int amount, int scale);
int FUN_00488de0(FixVector *pVertsA, FixVector *pVertsB, FixVector *pDir, int *pDistance);
// ---- DECLS extras (integrar al principio de StageObjects.cpp si no existen ya) ----
struct CollisionFaceVertices;
extern double g_unk0x00511300;
extern Car *g_collisionCar;
extern CollisionFaceVertices *g_collisionFace;
void FUN_0048c870(BYTE index, BYTE other, int *pDelta, int flag);
void CarPhysics_ApplyImpulse(FixVector *pImpulse, FixVector *pPoint, int usePoint);
// ---- GLOBALS nuevos (definir UNA sola vez en el proyecto) ----
// GLOBAL: CMR2 0x005918e0
FixVector g_unk0x005918e0;
// GLOBAL: CMR2 0x005918f0
FixVector g_unk0x005918f0;
// GLOBAL: CMR2 0x00591900
FixVector g_unk0x00591900;
// GLOBAL: CMR2 0x0059190c
int *g_unk0x0059190c;
// GLOBAL: CMR2 0x00591920
FixVector g_unk0x00591920;
// GLOBAL: CMR2 0x00591930
int g_unk0x00591930;
// GLOBAL: CMR2 0x00591938
FixVector g_unk0x00591938;
// GLOBAL: CMR2 0x00591950
FixVector g_unk0x00591950;
// GLOBAL: CMR2 0x00591968
FixVector g_unk0x00591968;
// GLOBAL: CMR2 0x00591978
FixVector g_unk0x00591978;
// GLOBAL: CMR2 0x00591990
FixVector g_unk0x00591990;
// GLOBAL: CMR2 0x0059199c
BYTE g_unk0x0059199c;
// GLOBAL: CMR2 0x005919a0
int g_unk0x005919a0;
// GLOBAL: CMR2 0x00591ad0
FixVector g_unk0x00591ad0;
// GLOBAL: CMR2 0x0051fb00
int g_unk0x0051fb00[27] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0x4ccc, 0, 0, 0, 0, 0, 0, 0, 0, 0x28f
};
// ---- DECLS extras (integrar al principio de StageObjects.cpp si no existen ya) ----
void FUN_00418ba0(unsigned int view, int strength, int listener);
void FUN_0045f9d0(int param_1, int *param_2, int param_3);
int FUN_00427d50(unsigned int view, int listener);
bool FUN_00427ab0(int value, int *pRange);
unsigned int FUN_00427b70(int value, int *pCurve);
// unsigned short, no unsigned int: el original devuelve 16 bits (sus llamadores hacen 'and eax,0xffff').
// Corregido por la auditoria de W165 en NetRace.cpp; la declaracion tiene que seguir el mismo contrato.
unsigned short FUN_00427e20(int param_1, int param_2, unsigned short param_3);
int Sound_IsPlaying(unsigned int handle);
void FUN_004b79a0(unsigned int handle, int volume);
void Sound_SetPan(unsigned int handle, unsigned short pan);
void FUN_00484f40(unsigned int param_1);
extern BYTE g_unk0x0051ebca[146];
extern BYTE g_unk0x00538d2c[];
extern BYTE *g_unk0x00590d78;
extern int g_unk0x0058ddc8;
// ---- GLOBALS nuevos ----
// First engine sample slot of each loaded car sound set.
// GLOBAL: CMR2 0x0058ddb4
int g_unk0x0058ddb4[2];
// GLOBAL: CMR2 0x0051f27c
int g_unk0x0051f27c = 0x10000;
// GLOBAL: CMR2 0x0051f2d8
int g_unk0x0051f2d8[13] = {
    0x486a, 0x4ec0, 0xf6e0, 0x715a, 0x715a, 0x590e, 0x590e,
    0x5126, 0x3366, 0x0b42, 0x10f36, 0x4388, 0x280a
};
// Advances one stage-object record between two animation states: on entering the
// 1/2 states it re-derives the scaled component and notifies the timing module;
// on returning to idle it clears the derived components.
// FUNCTION: CMR2 0x00460b60
void FUN_00460b60(int *p, int unused)
{
    int next = p[1];
    int state = *p;
    if (next == state)
        return;
    if (state == 0) {
        if (next != 1 && next != 2)
            return;
        p[0x16] = 0;
        p[0x17] = FixMul(FixMul(g_unk0x00543d50, p[0x15]), (short)p[0x1d] << 16);
        *p = p[1];
        FUN_0045f9d0(0, p, unused);
        return;
    }
    if ((state == 1 || state == 2) && next == 0) {
        p[0x17] = 0;
        if (p[0x16] == 0)
            *p = 0;
    }
}
// Walks a short list of scene objects backwards and, for each of their eight
// corner records that is enabled, has an eligible surface under it and moves
// above 0x1999, rebuilds the orthonormal contact frame around the corner (or
// drops the hit point half a body below the car for the corners without a
// wheel) and throws debris from it; each object ends with its impact
// bookkeeping.
// match 60%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00469e40
void FUN_00469e40(int param_1, short *param_2, short param_3)
{
    int count;
    short *pIndex;
    Car *pCar;
    int idx;
    int size;
    int dot;
    int len;
    FixVector vDir;
    FixVector vProj;
    FixVector vSide;
    FixVector vUp;
    FixVector vLocal;
    FixVector vHit;
    FixVector axes[3];
    count = param_3;
    if (count - 1 < 0)
        return;
    pIndex = param_2 + count - 1;
    do {
        pCar = (Car *)(param_1 + *pIndex * 0xc24);
        for (idx = 0; idx < 8; idx++) {
            if (pCar->cornerFlags[idx] != 0)
                continue;
            if (StageObject_IsEligibleType((short)*(unsigned short *)((BYTE *)pCar + 0xaae + idx * 2),
                                           pCar->field_0xb74, (int)pCar->field_0xb29) == 0)
                continue;
            if (pCar->speed <= 0x1999)
                continue;
            // Impact strength: the wheel travel of the corners that own a
            // wheel, the object speed for the rest.
            if (idx < 4 && pCar->field_0xb74 != 0) {
                int travelA;
                int travelB;
                travelA = pCar->field_0x870[idx];
                if (travelA < 0)
                    travelA = -travelA;
                travelB = pCar->field_0x880[idx];
                if (travelB < 0)
                    travelB = -travelB;
                size = FixMul(travelB + travelA, 0x10000);
                if (size > 0x10000)
                    size = 0x10000;
                else if (size <= 0x4ccc)
                    continue;
            } else {
                size = pCar->speed;
                if (size <= 0x4ccc)
                    continue;
            }
            // Contact frame: axis 0 is the velocity put square to the corner
            // axis, axis 2 the cross of the two and axis 1 the cross of the
            // others.
            vDir.x = FixMul(pCar->velocity.x, -0x10000);
            vDir.y = FixMul(pCar->velocity.y, -0x10000);
            vDir.z = FixMul(pCar->velocity.z, -0x10000);
            dot = FixVecDot(&pCar->cornerAxis[idx], &vDir);
            vProj.x = FixMul(pCar->cornerAxis[idx].x, dot);
            vProj.y = FixMul(pCar->cornerAxis[idx].y, dot);
            vProj.z = FixMul(pCar->cornerAxis[idx].z, dot);
            vDir.x -= vProj.x;
            vDir.y -= vProj.y;
            vDir.z -= vProj.z;
            len = FixVecLength(&vDir);
            if (len == 0) {
                vDir.x = 0;
                vDir.y = 0;
                vDir.z = 0;
            } else {
                FixVecScaleRecip(&vDir, &vDir, len);
            }
            FixVecCross(&vSide, &vDir, &pCar->cornerAxis[idx]);
            len = FixVecLength(&vSide);
            if (len == 0) {
                vSide.x = 0;
                vSide.y = 0;
                vSide.z = 0;
            } else {
                FixVecScaleRecip(&vSide, &vSide, len);
            }
            FixVecCross(&vUp, &vSide, &vDir);
            len = FixVecLength(&vUp);
            if (len == 0) {
                vUp.x = 0;
                vUp.y = 0;
                vUp.z = 0;
            } else {
                FixVecScaleRecip(&vUp, &vUp, len);
            }
            axes[0] = vDir;
            axes[1] = vUp;
            axes[2] = vSide;
            if (idx < 4 && pCar->field_0xb74 != 0) {
                vLocal = pCar->wheelEmitter[idx];
                vLocal.y -= 0x5999;
                FixMatrix_RotateVector(&vHit, &vLocal, pCar->pWorld);
            } else {
                vHit.x = FixMul(pCar->pWorld->right.y, -0x8000);
                vHit.y = FixMul(pCar->pWorld->up.y, -0x8000);
                vHit.z = FixMul(pCar->pWorld->forward.y, -0x8000);
            }
            if (idx < 4 && pCar->field_0xb74 != 0)
                Car_SpawnDebris(size, &vHit, pCar, axes, 0, 0);
            else
                Car_SpawnDebris(size, &vHit, pCar, axes, 0x90000, 0x9999);
        }
        FUN_0046a500((int)pCar);
        pIndex--;
        count--;
    } while (count != 0);
}
// Advances a stage object's animation record: on a key frame boundary it snaps
// the interpolated matrix, steps the frame counter and re-derives the car's
// heading, body transform and world velocity.
// match 20%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046d610
void FUN_0046d610(BYTE *p)
{
    Car *pCar;
    int t;
    int value;
    int n;
    if (p == NULL)
        return;
    pCar = Car_Get((int)p[0x20]);
    if (*(int *)(p + 4) == 0 || *(int *)(p + 0x1c) != 2 || pCar->field_0xb43[0] == 0)
        return;
    if (p[0xf8] > 2) {
        int i;
        for (i = 0; i < 16; i++)
            ((int *)(p + 0x44))[i] = ((int *)(p + 0x84))[i];
        *(int *)(p + 0xd8) = *(int *)(p + 0xdc);
        n = *(int *)(p + 0xe4);
        *(int *)(p + 0xd0) = *(int *)(p + 0xd4);
        *(int *)(p + 0xe0) = n;
        *(int *)(p + 0xf0) = *(int *)(p + 0xf4);
        if (n > 0)
            FUN_00418ba0((unsigned int)pCar->field_0xb1a, n, pCar->field_0xb1a);
        *(short *)(p + 0x10a) = *(short *)(p + 0x10a) + 1;
        if (*(short *)(p + 0x10a) <
            *(short *)(*(int *)(p + 0x104) + *(short *)(p + 0x108) * 2))
            FUN_0046de20((unsigned int *)(p + 0xec), (unsigned int *)(p + 0xf4),
                         (unsigned int *)(p + 0xdc), (int *)(p + 0xd4),
                         (FixMatrix *)(p + 0x84), (int *)(p + 0xe4),
                         (unsigned int *)(p + 0xe8),
                         (int *)(((int)*(short *)(p + 0xfe) * (int)*(short *)(p + 0x108) +
                                  (int)*(short *)(p + 0x10a)) * 0x10 + *(int *)(p + 0x40)));
        else
            FUN_0046d2a0((int *)p);
        FUN_0046e340(p + 0x44, (BYTE *)pCar);
        p[0xf8] = 0;
        if (*(int *)(p + 0xf0) != 0)
            pCar->field_0xbf8 = 1;
    }
    t = (*(int *)(p + 0xf4) == 0)
            ? (int)(((unsigned __int64)p[0xf8] << 32) / 0x30000)
            : 0;
    FixMatrix_Interpolate(pCar->pWorld, (FixMatrix *)(p + 0x44), (FixMatrix *)(p + 0x84),
                          t, t, t, 1);
    value = FixMul(FixMul(*(int *)(p + 0xd4) - *(int *)(p + 0xd0), t) + *(int *)(p + 0xd0),
                   (int)pCar->field_0xb16 * 0x1680);
    pCar->field_0x7a4 = 0;
    pCar->heading = (unsigned short)(__int64)((double)value * g_unk0x00511300);
    pCar->field_0x79c =
        FixMul(*(int *)((BYTE *)pCar + 0x788),
               FixMul(*(int *)(p + 0xdc) - *(int *)(p + 0xd8), t) + *(int *)(p + 0xd8));
    pCar->velocityNext = pCar->velocity;
    pCar->velocity.x += *(int *)(p + 0xc4);
    pCar->velocity.y += *(int *)(p + 0xc8);
    pCar->velocity.z += *(int *)(p + 0xcc);
    *(int *)((BYTE *)pCar + 0xb54) = *(int *)(p + 0xec);
    p[0xf8] = p[0xf8] + 1;
}
// Dispatches one stage object's per-frame update when its stage-block slot is
// active, refreshing the car's order, light and mesh state.
// FUNCTION: CMR2 0x004765e0
void FUN_004765e0(BYTE *pObj, int a, int b)
{
    if (g_unk0x0058d6a8[pObj[2]] != 0) {
        FUN_00476e00(pObj, (int *)a, b);
        FUN_00476640(pObj[2]);
        FUN_00476a40(pObj[2]);
        FUN_00477460(pObj[2]);
        FUN_004778b0(pObj, a);
    }
}
// Picks the nearest stage cars to the player's view (up to two) and starts or
// updates their engine sounds, then adjusts pan and volume from the distance.
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047aa70
void FUN_0047aa70(void)
{
    int carIds[8];
    int distances[8];
    int used[16];
    FixMatrix rot;
    FixVector pos;
    FixVector viewPos;
    FixVector delta;
    int i;
    int count;
    int slots;
    int limit;
    int *pHandle;
    unsigned int chosen;
    int bestDist;
    int dist;
    int scale;
    int pitch;
    count = (int)Car_GetOrderCount() - (int)RallyDataState();
    if (count == 0)
        return;
    slots = (count < 2) ? count : 2;
    if (count > 0) {
        for (i = 0; i < count; i++)
            used[i] = 1;
        for (i = 0; i < count; i++) {
            Car *pCar = Car_Get((int)(RallyDataState() & 0xff) + i);
            FixMatrix_CopyRotationFrom(&rot, pCar->pWorld);
            FixMatrix_GetPosition(&pos, &rot);
            FixMatrix_GetPosition(&viewPos, (FixMatrix *)(g_unk0x00538d2c + 4));
            delta.x = pos.x - viewPos.x;
            delta.y = pos.y - viewPos.y;
            delta.z = pos.z - viewPos.z;
            distances[i] = (int)FixVec_Length(&delta);
        }
    }
    if (slots <= 0)
        return;
    pHandle = &g_unk0x0058ddc8;
    limit = slots;
    do {
        chosen = 0;
        bestDist = 0x42400000;
        if (count > 0) {
            for (i = 0; i < count; i++) {
                if (distances[i] < bestDist && used[i + 1] != 0) {
                    chosen = (unsigned int)(i + 1);
                    bestDist = distances[i];
                }
            }
        }
        used[chosen] = 0;
        dist = FUN_00427d50(chosen, 0);
        {
            Car *pCar = Car_Get(chosen);
            int car798 = *(int *)((BYTE *)pCar + 0x798);
            scale = FixMul(*(int *)((BYTE *)pCar + 0x7ac), car798);
            pitch = FixMulShift32(scale, 0x19640000);
            if (pitch < 2000)
                pitch = 2000;
            else if (pitch > 0x1964)
                pitch = 0x1964;
        }
        if (Sound_IsPlaying((unsigned int)*pHandle) == 0) {
            int volScale = FixMul(g_unk0x0058dda8, FixMul(dist, g_unk0x0051f27c));
            int dist2 = FUN_00427d50(chosen, 0);
            int idx = (int)CFrontend::FUN_0040ee90(RallyData_FUN_004086b0(0));
            *pHandle = FUN_004b7790((unsigned short)(g_unk0x0058ddb4[0] + 6),
                                    FixMul(dist2, volScale), 0x5622,
                                    g_unk0x0051f2d8[idx], 1, 0);
        }
        if (FUN_00427ab0(pitch, (int *)&g_unk0x0051ebca[0x86])) {
            unsigned int pan = FUN_00427b70(pitch, (int *)&g_unk0x0051ebca[0x86]);
            unsigned short pan2 = (unsigned short)FUN_00427e20(0, (int)chosen, (unsigned short)pan);
            int volScale;
            int dist2;
            Sound_SetPan((unsigned int)*pHandle, pan2);
            volScale = FixMul(g_unk0x0058dda8, FixMul(dist, g_unk0x0051f27c));
            dist2 = FUN_00427d50(chosen, 0);
            FUN_004b79a0((unsigned int)*pHandle, FixMul(dist2, volScale));
        } else {
            FUN_004b79a0((unsigned int)*pHandle, 0);
        }
        pHandle++;
    } while (--limit);
}
// Same as FUN_0047aa70 for the network race: gathers the active player cars by
// their network index instead of the local order list.
// match 38%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047ad20
void FUN_0047ad20(void)
{
    int carIds[8];
    int distances[8];
    int used[16];
    FixMatrix rot;
    FixVector pos;
    FixVector viewPos;
    FixVector delta;
    int i;
    int n;
    int count;
    int slots;
    int limit;
    int *pHandle;
    unsigned int chosen;
    int bestDist;
    int dist;
    int scale;
    int pitch;
    count = 0;
    for (i = 0; i < 7; i++) {
        if (FUN_00409cb0(i) != 0)
            carIds[count++] = FUN_0040b010(i);
    }
    if (count == 0)
        return;
    slots = (count < 2) ? count : 2;
    for (i = 0; i < 8; i++)
        used[i] = 0;
    for (i = 0; i < count; i++)
        used[i] = 1;
    for (i = 0; i < count; i++) {
        Car *pCar = Car_Get(carIds[i]);
        FixMatrix_CopyRotationFrom(&rot, pCar->pWorld);
        FixMatrix_GetPosition(&pos, &rot);
        FixMatrix_GetPosition(&viewPos, (FixMatrix *)(g_unk0x00538d2c + 4));
        delta.x = pos.x - viewPos.x;
        delta.y = pos.y - viewPos.y;
        delta.z = pos.z - viewPos.z;
        distances[i] = (int)FixVec_Length(&delta);
    }
    if (slots <= 0)
        return;
    pHandle = &g_unk0x0058ddc8;
    limit = slots;
    do {
        chosen = 0;
        bestDist = 0x42400000;
        for (i = 0; i < count; i++) {
            if (i == 0) {
                bestDist = distances[0];
                chosen = (unsigned int)carIds[0];
            }
            if (distances[i] < bestDist && used[i] != 0) {
                chosen = (unsigned int)carIds[i];
                bestDist = distances[i];
            }
        }
        used[chosen] = 0;
        dist = FUN_00427d50(chosen, 0);
        {
            Car *pCar = Car_Get(chosen);
            int car798 = *(int *)((BYTE *)pCar + 0x798);
            scale = FixMul(*(int *)((BYTE *)pCar + 0x7ac), car798);
            pitch = FixMulShift32(scale, 0x19640000);
            if (pitch < 2000)
                pitch = 2000;
            else if (pitch > 0x1964)
                pitch = 0x1964;
        }
        if (Sound_IsPlaying((unsigned int)*pHandle) == 0) {
            int volScale = FixMul(g_unk0x0058dda8, FixMul(dist, g_unk0x0051f27c));
            int dist2 = FUN_00427d50(chosen, 0);
            int idx = (int)CFrontend::FUN_0040ee90(RallyData_FUN_004086b0(0));
            *pHandle = FUN_004b7790((unsigned short)(g_unk0x0058ddb4[0] + 6),
                                    FixMul(dist2, volScale), 0x5622,
                                    g_unk0x0051f2d8[idx], 1, 0);
        }
        if (FUN_00427ab0(pitch, (int *)&g_unk0x0051ebca[0x86])) {
            unsigned int pan = FUN_00427b70(pitch, (int *)&g_unk0x0051ebca[0x86]);
            unsigned short pan2 = (unsigned short)FUN_00427e20(0, (int)chosen, (unsigned short)pan);
            int volScale;
            int dist2;
            Sound_SetPan((unsigned int)*pHandle, pan2);
            volScale = FixMul(g_unk0x0058dda8, FixMul(dist, g_unk0x0051f27c));
            dist2 = FUN_00427d50(chosen, 0);
            FUN_004b79a0((unsigned int)*pHandle, FixMul(dist2, volScale));
        } else {
            FUN_004b79a0((unsigned int)*pHandle, 0);
        }
        pHandle++;
    } while (--limit);
}
// Refreshes the on-screen control indicators of the active car (steer, throttle,
// brake, handbrake) from the input device driving a player: analogue axes scale
// the indicators, digital buttons light them full.
// match 33%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047b0e0
void FUN_0047b0e0(int player, int device)
{
    DeviceInfo *pDev;
    DeviceInfo *pSrc;
    Car *pCar;
    BYTE *p;
    int axisSteer;
    int axisThrottle;
    int axisBrake;
    int idx;
    int raw;
    int half;
    char c;
    if (CGameInfo::FUN_00404f20())
        return;
    if (CGameInfo::FUN_00405e00()) {
        if (player > 0)
            return;
    } else if (player >= (int)(RallyDataState() & 0xff)) {
        return;
    }
    FUN_0047bca0(player);
    pDev = CInput::FUN_0049ead0(device);
    if (pDev->field_0x0 != 1) {
        pSrc = CInput::FUN_0049ead0(player);
        pDev->field_0x4 |= pSrc->field_0x4;
        pDev->field_0x8 |= pSrc->field_0x8;
        pDev->field_0xc |= pSrc->field_0xc;
    }
    p = FUN_0041b390();
    if (*(char *)(*(int *)(p + 4) + player * 8) != '\n' || FUN_0044a130() != 0) {
        p = FUN_0041b390();
        if (*(char *)(*(int *)(p + 4) + player * 8) == '\n') {
            idx = FUN_0041f3a0() ? -1 : 0;
            if (FUN_00422f50((BYTE)idx) != 10 && FUN_00422f50((BYTE)idx) != 7) {
                if ((pDev->field_0x8 & g_carButtonMasks[7]) != 0)
                    FUN_0047b970(player);
            }
        } else {
            if (FUN_00422f50((BYTE)player) != 10 && FUN_00422f50((BYTE)player) != 7) {
                if ((pDev->field_0x8 & g_carButtonMasks[7]) != 0)
                    FUN_0047b970(player);
            }
        }
    }
    if (FUN_0041f3d0((BYTE)player) == 0) {
        if ((pDev->field_0x8 & g_carButtonMasks[7]) == 0 && FUN_00422f50((BYTE)player) != 8)
            FUN_0047bad0(pDev->field_0x4, player);
        pCar = g_unk0x0058e0a0;
        if (pDev->field_0x0 == 3 || pDev->field_0x0 == 2) {
            axisSteer = g_unk0x0058e0a8[0] ? (int)CInput::FUN_0040c210(player, 0) : -1;
            if (g_unk0x0058e0a8[1]) {
                axisThrottle = (int)CInput::FUN_0040c210(player, 2);
                axisBrake = (int)CInput::FUN_0040c210(player, 3);
            } else {
                axisThrottle = -1;
                axisBrake = -1;
            }
            if (axisSteer != -1) {
                *(int *)((BYTE *)pCar + 0xb88) = CInput::FUN_0040be60((short)pCar->field_0xb1a) ? 1 : 2;
                raw = ((int *)pDev)[axisSteer * 5 + 0x11f];
                half = raw < 0 ? -raw : raw;
                if (raw < 0)
                    pCar->flag0x1d0[0] = (char)FixMulShift32(half, 0x3f0000);
                else
                    pCar->flag0x1d0[1] = (char)FixMulShift32(half, 0x3f0000);
            } else if ((pDev->field_0x4 & g_carButtonMasks[0]) != 0) {
                pCar->flag0x1d0[0] = 0x3f;
            } else if ((pDev->field_0x4 & g_carButtonMasks[1]) != 0) {
                pCar->flag0x1d0[1] = 0x3f;
            }
            if (axisThrottle != -1) {
                *(int *)((BYTE *)pCar + 0xb8c) = 1;
                raw = ((int *)pDev)[axisThrottle * 5 + 0x11f];
                half = raw < 0 ? -raw : raw;
                if (axisBrake == axisThrottle) {
                    if ((raw > 0) == (CInput::FUN_0040be30(device) != 0)) {
                        pCar->flag0x1d0[2] = (char)FixMulShift32(half, 0x3f0000);
                    } else {
                        *(int *)((BYTE *)pCar + 0xb90) = 1;
                        pCar->flag0x1d0[3] = (char)FixMulShift32(half, 0x3f0000);
                    }
                } else {
                    half = raw / 2;
                    idx = half + 0x8000;
                    if (idx < 0)
                        idx = -0x8000 - half;
                    pCar->flag0x1d0[2] = (char)(0x3f - FixMulShift32(idx, 0x3f0000));
                }
            } else {
                *(int *)((BYTE *)pCar + 0xb8c) = 0;
                if ((pDev->field_0x4 & g_carButtonMasks[2]) != 0)
                    pCar->flag0x1d0[2] = 0x3f;
            }
            if (axisBrake != -1) {
                if (axisBrake != axisThrottle) {
                    *(int *)((BYTE *)pCar + 0xb90) = 1;
                    raw = ((int *)pDev)[axisBrake * 5 + 0x11f];
                    half = raw / 2;
                    idx = half + 0x8000;
                    if (idx < 0)
                        idx = -0x8000 - half;
                    c = (char)(0x3f - FixMulShift32(idx, 0x3f0000));
                    if (c != 0)
                        pCar->flag0x1d0[3] = c;
                }
            } else {
                *(int *)((BYTE *)pCar + 0xb90) = 0;
                if ((pDev->field_0x4 & g_carButtonMasks[3]) != 0)
                    pCar->flag0x1d0[3] = 0x3f;
            }
        } else {
            *(int *)((BYTE *)pCar + 0xb8c) = 0;
            *(int *)((BYTE *)pCar + 0xb90) = 0;
            *(int *)((BYTE *)pCar + 0xb88) = 0;
            if ((pDev->field_0x4 & g_carButtonMasks[2]) != 0)
                pCar->flag0x1d0[2] = 0x3f;
            if ((pDev->field_0x4 & g_carButtonMasks[3]) != 0)
                pCar->flag0x1d0[3] = 0x3f;
            if ((pDev->field_0x4 & g_carButtonMasks[0]) != 0)
                pCar->flag0x1d0[0] = 0x3f;
            else if ((pDev->field_0x4 & g_carButtonMasks[1]) != 0)
                pCar->flag0x1d0[1] = 0x3f;
        }
        if ((pDev->field_0x4 & g_carButtonMasks[4]) != 0)
            pCar->field_0x1d8 = 1;
        if (*(int *)((BYTE *)pCar + 0xb9c) != 0) {
            if ((pDev->field_0x8 & g_carButtonMasks[5]) != 0)
                pCar->field_0x1d4[0] = 1;
            if ((pDev->field_0x8 & g_carButtonMasks[6]) != 0)
                pCar->field_0x1d4[0] = 0xff;
        }
    }
    if (CGameInfo::FUN_004063f0(2) != 0) {
        c = g_unk0x0058e0a0->flag0x1d0[0];
        g_unk0x0058e0a0->flag0x1d0[0] = g_unk0x0058e0a0->flag0x1d0[1];
        g_unk0x0058e0a0->flag0x1d0[1] = c;
    }
}
// Tests every active headlight glow record of the other cars against the
// oriented bounding box of this car (extents along the two box axes and the
// world y range), and for the ones found inside it builds the impact direction
// from the glow position, accumulates the impulse in the deformation vector and
// torque of the car and applies the mode 2 collision deformation.
// match 77%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047d850
void FUN_0047d850(Car *pCar, int *param_2)
{
    FixVector delta;
    FixVector dir;
    FixVector localDelta;
    FixVector localDir;
    FixVector cross;
    FixVector *pAnchor;
    int *pRec;
    int i;
    int dot1;
    int dot2;
    pRec = (int *)g_unk0x0058e4c8;
    i = 100;
    do {
        if (pRec[0x15] != 0 && *(BYTE *)(pRec + 0x16) != pCar->field_0xb1a) {
            pAnchor = (FixVector *)param_2[0x25];
            delta.x = *(int *)(pRec + 3) - pAnchor->x;
            delta.y = *(int *)(pRec + 4) - pAnchor->y;
            delta.z = *(int *)(pRec + 5) - pAnchor->z;
            if (FIX_ABS(delta.x) < 0x640000) {
                if (FIX_ABS(delta.y) < 0x640000) {
                    if (FIX_ABS(delta.z) < 0x640000 && *(int *)(pRec + 4) >= param_2[3] &&
                        *(int *)(pRec + 4) <= param_2[2]) {
                        dot1 = FixVecDot((FixVector *)(param_2 + 4), &delta);
                        dot2 = FixVecDot((FixVector *)(param_2 + 7), &delta);
                        if (FIX_ABS(dot1) <= param_2[0] && FIX_ABS(dot2) <= param_2[1]) {
                            FixVecScale(&dir, (FixVector *)pRec, 0x20000);
                            FIX_NORMALIZE_INTO(dir, dir);
                            delta.y -= 0x38000;
                            *(int *)((BYTE *)pCar + 0x40c) += 0x4000;
                            FixMatrix_InverseRotateVector(&localDelta, &delta, pCar->pWorld);
                            FixMatrix_InverseRotateVector(&localDir, &dir, pCar->pWorld);
                            FixVecCross(&cross, &localDir, &localDelta);
                            pCar->field_0x5c4.x += dir.x;
                            pCar->field_0x5c4.y += dir.y;
                            pCar->field_0x5c4.z += dir.z;
                            pCar->field_0x5d0.x += cross.x;
                            pCar->field_0x5d0.y += cross.y;
                            pCar->field_0x5d0.z += cross.z;
                            pCar->field_0xc00 = 1;
                            pCar->field_0x96c = 0x10000;
                            FIX_NORMALIZE_INTO(dir, dir);
                            FUN_00466ef0(pCar, pRec + 3, &dir, 0, 2, 0);
                            pRec[0x15] = 0;
                            pRec[0x12] = 0;
                        }
                    }
                }
            }
        }
        pRec += 0x17;
        i--;
    } while (i != 0);
}
// Stops the stage-object records of every climbing car: cars resting on a very
// steep ground normal get the ground-aligned step, the rest the free step.
// match 42%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00484e00
void FUN_00484e00(int param_1, short count)
{
    FixVector impulse;
    short *pIndex;
    FUN_00460a30(&impulse);
    if (count - 1 < 0)
        return;
    pIndex = (short *)(param_1 + (count - 1) * 2);
    do {
        Car *pCar = Car_Get((int)*pIndex);
        int n;
        g_unk0x00590d74 = (Unk0x00590d74 *)pCar;
        g_unk0x00590d78 = (BYTE *)FUN_00469680((int)*pIndex);
        if (*(int *)((BYTE *)pCar + 0xc0c) == 0) {
            if (*(int *)((BYTE *)pCar + 0xb64) == 0 &&
                *(int *)((BYTE *)pCar + 0xc00) != 0 &&
                FixVecDot((FixVector *)((BYTE *)pCar + 0x36c),
                          (FixVector *)((BYTE *)pCar + 0x48c)) < -0xcccc &&
                (pCar->cornerFlags[5] == 0 || pCar->cornerFlags[4] == 0 ||
                 pCar->cornerFlags[7] == 0 || pCar->cornerFlags[6] == 0)) {
                n = *(int *)g_unk0x00590b30[pCar->field_0xb1a] - 1;
                for (; n >= 0; n--)
                    FUN_00484f40(n);
            } else {
                FUN_004853c0(&impulse);
                n = *(int *)g_unk0x00590b30[pCar->field_0xb1a] - 1;
                for (; n >= 0; n--)
                    FUN_004854a0(n);
            }
        }
        pIndex--;
    } while (--count);
}

int FUN_00423fc0(int view);
void FUN_00485860(unsigned int index, int *pTarget, int flag);
extern int g_unk0x00590c58;
extern int g_unk0x00590c5c;
#define CURRENT_CAR ((BYTE *)g_unk0x00590d74)

// Draws the light beams of every car in the order list for one view (not the
// player's own car from the in-car cameras): the beam colour follows the
// scene light, and each beam is flagged when the car's light node is lit for
// this view.
// FUNCTION: CMR2 0x00485690
void FUN_00485690(short *pOrder, short count, int view)
{
    FixVector viewPos;
    BYTE colour[4];
    int self;
    int inCar;
    int i;
    int n;
    int car;
    int lit;
    BYTE mask;

    FixMatrix_GetPosition(&viewPos, (FixMatrix *)(g_unk0x00538d2c + 4 + view * 100));
    inCar = FUN_00423fc0(view);
    if (inCar == 0 && FUN_00422f50((BYTE)view) == 10)
        inCar = 1;
    self = FUN_00422fb0((BYTE)view);
    for (i = count - 1; i >= 0; i--) {
        car = pOrder[i];
        g_unk0x00590d74 = (Unk0x00590d74 *)Car_Get(car);
        if (*(int *)(CURRENT_CAR + 0xc0c) != 0 || (inCar != 0 && self == car) ||
            *(int *)(CURRENT_CAR + 0xb68 + view * 4) != 0)
            continue;
        Scene_GetLightColour((DWORD *)colour, *(int *)(CURRENT_CAR + 0xa70));
        g_unk0x00590c54 = FixMul(colour[0] << 16, 0x106);
        if (g_unk0x00590c54 > 0x10000)
            g_unk0x00590c54 = 0x10000;
        g_unk0x00590c58 = FixMul(colour[1] << 16, 0x106);
        if (g_unk0x00590c58 > 0x10000)
            g_unk0x00590c58 = 0x10000;
        g_unk0x00590c5c = FixMul(colour[2] << 16, 0x106);
        if (g_unk0x00590c5c > 0x10000)
            g_unk0x00590c5c = 0x10000;
        mask = (BYTE)(1 << view);
        lit = 0;
        if (*(BYTE **)(CURRENT_CAR + 0x724) == NULL) {
            if ((*(BYTE **)(CURRENT_CAR + 0x738))[0x17c] & mask)
                lit = 1;
        } else if ((*(BYTE **)(CURRENT_CAR + 0x724))[0x17c] != 0) {
            if (mask & (*(BYTE **)(CURRENT_CAR + 0x724))[0x17c])
                lit = 1;
        } else if ((*(BYTE **)(CURRENT_CAR + 0x738))[0x17c] & mask) {
            lit = 1;
        }
        for (n = *(int *)g_unk0x00590b30[(signed char)CURRENT_CAR[0xb1a]] - 1; n >= 0; n--)
            FUN_00485860(n, (int *)&viewPos, lit);
    }
}
#undef CURRENT_CAR

// Draws the dust trail of one stage-object record. When the record's +0x38 flag
// is clear the record's position and axis are pushed through the car's
// suspension matrix and five short parabola segments (a curved direction plus a
// sideways sweep, both faded by distance to `pTarget`) are queued; when the flag
// is set a single segment from +0x0 to +0x9, clamped to one unit long, is queued.
// match 52%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00485860
void FUN_00485860(unsigned int index, int *pTarget, int flag)
{
    BYTE *pDesc;
    int *pEntry;
    FixVector *pAxis;
    FixMatrix *pMatrix;
    BYTE colour[4];
    FixVector base;
    FixVector end;
    FixVector delta;
    FixVector pos;
    FixVector dir;
    FixVector side;
    FixVector back;
    FixVector right;
    FixVector axis;
    int car;
    int fade;
    int len;
    int i;
    car = *(char *)((BYTE *)g_unk0x00590d74 + 0xb1a);
    pDesc = (BYTE *)g_unk0x00590c00[car] + (index & 0xff) * 0x20;
    pEntry = (int *)((BYTE *)g_unk0x00590c6c[car] + (index & 0xff) * 0x3c);
    pAxis = (FixVector *)(pDesc + 0xc);
    colour[0] = (BYTE)FixMul(g_unk0x00590c54, *(BYTE *)(pDesc + 0x1c));
    colour[1] = (BYTE)FixMul(g_unk0x00590c58, *(BYTE *)(pDesc + 0x1d));
    colour[2] = (BYTE)FixMul(g_unk0x00590c5c, *(BYTE *)(pDesc + 0x1e));
    colour[3] = *(BYTE *)(pDesc + 0x1f);
    if (pEntry[0xe] != 0) {
        // Flag set: one straight segment from +0x0 to +0x9, each end nudged at
        // most one unit towards the target.
        delta.x = pTarget[0] - pEntry[0];
        delta.y = pTarget[1] - pEntry[1];
        delta.z = pTarget[2] - pEntry[2];
        fade = StageObject_DistanceFade(&delta);
        if (fade <= 0)
            return;
        i = FixMulShift32(colour[3] << 16, fade);
        if (i >= 0x100)
            i = 0xff;
        else if (i < 0)
            i = 0;
        colour[3] = (BYTE)i;
        len = FixVecLength(&delta);
        if (len > 0x10000)
            FixVecScaleRecip(&delta, &delta, len);
        base.x = pEntry[0] + delta.x;
        base.y = pEntry[1] + delta.y;
        base.z = pEntry[2] + delta.z;
        delta.x = pTarget[0] - pEntry[9];
        delta.y = pTarget[1] - pEntry[10];
        delta.z = pTarget[2] - pEntry[0xb];
        len = FixVecLength(&delta);
        if (len > 0x10000)
            FixVecScaleRecip(&delta, &delta, len);
        end.x = pEntry[9] + delta.x;
        end.y = pEntry[10] + delta.y;
        end.z = pEntry[0xb] + delta.z;
        Line2D_Queue((int *)&base, (int *)&end, colour, colour);
        return;
    }
    if (flag == 0)
        return;
    {
        // Suspension matrix: the car's active wheel matrix (or the fallback one
        // when it is absent or inactive), past its 0xd8-byte header.
        BYTE *pObj = *(BYTE **)((BYTE *)g_unk0x00590d74 + 0x724);
        if (pObj == NULL)
            pObj = *(BYTE **)((BYTE *)g_unk0x00590d74 + 0x720);
        else if (pObj[0x17c] == 0)
            pObj = *(BYTE **)((BYTE *)g_unk0x00590d74 + 0x720);
        pMatrix = (FixMatrix *)(pObj + 0xd8);
    }
    // Record velocity (+0x18/+0x1c/+0x20) becomes this segment's direction.
    dir.x = pEntry[6];
    dir.y = pEntry[7];
    dir.z = pEntry[8];
    // Contact point: the record's local offset rotated into the matrix frame.
    FixMatrix_GetPosition(&pos, pMatrix);
    FixMatrix_RotateVector(&base, (FixVector *)pDesc, pMatrix);
    base.x += pos.x;
    base.y += pos.y;
    base.z += pos.z;
    delta.x = base.x - pTarget[0];
    delta.y = base.y - pTarget[1];
    delta.z = base.z - pTarget[2];
    fade = StageObject_DistanceFade(&delta);
    if (fade <= 0)
        return;
    i = FixMulShift32(colour[3] << 16, fade);
    if (i >= 0x100)
        i = 0xff;
    else if (i < 0)
        i = 0;
    colour[3] = (BYTE)i;
    // Direction: smoothed by the physics step on x/z only, normalised and
    // scaled by the record's +0x18 radius.
    dir.x = FixMul(dir.x, g_physicsTimeStep);
    dir.z = FixMul(dir.z, g_physicsTimeStep);
    len = FixVecLength(&dir);
    if (len == 0) {
        dir.x = 0;
        dir.y = 0;
        dir.z = 0;
    } else {
        FixVecScaleRecip(&dir, &dir, len);
    }
    FixVecScale(&dir, &dir, *(int *)(pDesc + 0x18));
    // Sideways sweep: the record's axis in matrix space, scaled by 0.2 * dir.y.
    FixMatrix_RotateVector(&side, pAxis, pMatrix);
    FixVecScale(&side, &side, FixMul(dir.y, FixDiv(0x10000, 0x50000)));
    // Two vectors perpendicular to the axis (the axis' x and z rows of the
    // "rotate a unit vector to the axis" basis).
    if (pAxis->x == 0) {
        back.x = 0x10000;
        back.y = 0;
        back.z = 0;
    } else {
        back.x = 0x10000 - FixMul(pAxis->x, pAxis->x);
        back.y = -FixMul(pAxis->y, pAxis->x);
        back.z = -FixMul(pAxis->z, pAxis->x);
        len = FixVecLength(&back);
        if (len == 0) {
            back.x = 0;
            back.y = 0;
            back.z = 0;
        } else {
            FixVecScaleRecip(&back, &back, len);
        }
    }
    if (pAxis->z == 0) {
        right.x = 0;
        right.y = 0;
        right.z = 0x10000;
    } else {
        right.x = -FixMul(pAxis->x, pAxis->z);
        right.y = -FixMul(pAxis->y, pAxis->z);
        right.z = 0x10000 - FixMul(pAxis->z, pAxis->z);
        len = FixVecLength(&right);
        if (len == 0) {
            right.x = 0;
            right.y = 0;
            right.z = 0;
        } else {
            FixVecScaleRecip(&right, &right, len);
        }
    }
    // The curved direction is the two basis vectors weighted by the direction's
    // x and z, rotated back into world space.
    FixVecScale(&back, &back, dir.x);
    FixVecScale(&right, &right, dir.z);
    delta.x = right.x + back.x;
    delta.y = right.y + back.y;
    delta.z = right.z + back.z;
    FixMatrix_RotateVector(&axis, &delta, pMatrix);
    {
        FixVector step;
        FixVector off;
        int bx = base.x;
        int by = base.y;
        int bz = base.z;
        // Five segments along a parabola (t^2 * axis) swept by t * side; every
        // segment starts where the previous one ended.
        for (i = 1; i < 6; i++) {
            int t = i << 16;
            int s = FixMul(t, FixDiv(0x10000, 0x50000));
            s = FixMul(s, s);
            step.x = FixMul(axis.x, s);
            step.y = FixMul(axis.y, s);
            step.z = FixMul(axis.z, s);
            off.x = FixMul(side.x, t);
            off.y = FixMul(side.y, t);
            off.z = FixMul(side.z, t);
            end.x = bx + step.x + off.x;
            end.y = by + step.y + off.y;
            end.z = bz + step.z + off.z;
            Line2D_Queue((int *)&base, (int *)&end, colour, colour);
            base = end;
        }
    }
}
// Overlap test between two oriented 2D collision boxes: the four corners of each
// box are checked against the other box's half extents in that box's own frame,
// and the deepest push distance found along the direction from `pOffset` to box
// B is handed to the collision resolver. When no corner overlaps, the box edge
// (quad) test is tried as a fallback. Returns 1 when a contact was resolved.
// match 49%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00488640
int FUN_00488640(int *pBoxA, int *pBoxB, FixVector *pOffset, int scale)
{
    FixVector dir;
    FixVector negDir;
    FixVector delta;
    FixVector corner;
    FixVector *posA;
    FixVector *posB;
    int projAxis0;
    int projAxis1;
    int projCorner0;
    int projCorner1;
    int t0;
    int t1;
    int best;
    int maxDist;
    int hit;
    int passHit;
    int hit0;
    int hit1;
    int length;
    int index;
    int i;
    if (pBoxA[2] < pBoxB[3] || pBoxB[2] < pBoxA[3])
        return 0;
    g_unk0x005914d4 = 0;
    g_unk0x005915f4 = 0;
    g_unk0x005914a8.x = 0;
    g_unk0x005914a8.y = 0;
    g_unk0x005914a8.z = 0;
    g_unk0x005914b8.x = 0;
    g_unk0x005914b8.y = 0;
    g_unk0x005914b8.z = 0;
    posA = (FixVector *)pBoxA[0x25];
    posB = (FixVector *)pBoxB[0x25];
    // Direction from the offset point to box B, flattened and normalised.
    delta.x = posB->x - pOffset->x;
    delta.y = posB->y - pOffset->y;
    delta.z = posB->z - pOffset->z;
    delta.y = 0;
    length = FixSqrt(FixMul(delta.x, delta.x) + FixMul(delta.z, delta.z));
    if (length == 0) {
        dir.x = 0;
        dir.y = 0;
        dir.z = 0;
    } else {
        FixVecScale(&dir, &delta, FixDiv(0x10000, length));
    }
    // Projections of box A's two axes on the direction.
    projAxis0 = FixVecDot((FixVector *)(pBoxA + 4), &dir);
    projAxis1 = FixVecDot((FixVector *)(pBoxA + 7), &dir);
    hit = 0;
    passHit = 0;
    maxDist = 0;
    // Pass 1: box B's corners tested in box A's frame.
    for (i = 0; i < 4; i++) {
        corner = ((FixVector *)(pBoxB + 0xc))[i];
        delta.x = corner.x - posA->x;
        delta.y = corner.y - posA->y;
        delta.z = corner.z - posA->z;
        delta.y = 0;
        projCorner0 = FixVecDot((FixVector *)(pBoxA + 4), &delta);
        projCorner1 = FixVecDot((FixVector *)(pBoxA + 7), &delta);
        if (FIX_ABS(projCorner0) > pBoxA[0] || FIX_ABS(projCorner1) > pBoxA[1])
            continue;
        hit0 = 0;
        hit1 = 0;
        t0 = 0;
        t1 = 0;
        best = 0x7d000000;
        // Distance to the box A face the corner leaves through, per axis; an
        // axis whose direction projection is ~0 (|cos| <= 0x41) can not slide.
        if (projAxis0 > 0x41) {
            t0 = FixDiv(pBoxA[0] - projCorner0, projAxis0);
            hit0 = 1;
        } else if (projAxis0 < -0x41) {
            t0 = FixDiv(FIX_ABS(pBoxA[0] + projCorner0), -projAxis0);
            hit0 = 1;
        }
        if (projAxis1 > 0x41) {
            t1 = FixDiv(pBoxA[1] - projCorner1, projAxis1);
            hit1 = 1;
        } else if (projAxis1 < -0x41) {
            t1 = FixDiv(FIX_ABS(pBoxA[1] + projCorner1), -projAxis1);
            hit1 = 1;
        }
        index = g_unk0x005914d4;
        if (hit0 && t0 < 0x7d000000) {
            best = t0;
            g_unk0x005914a4[index] = 0;
        }
        if (hit1 && t1 < best) {
            best = t1;
            g_unk0x005914a4[index] = 2;
        }
        if ((hit0 || hit1) && best > maxDist)
            maxDist = best;
        g_unk0x00590ecc[index] = (char)i;
        g_unk0x005914d4 = (char)(index + 1);
        passHit = 1;
        hit = 1;
    }
    if (passHit != 0)
        FUN_004894b0(pBoxA, pBoxB, (int *)&dir, maxDist, scale);
    // Pass 2: box A's corners tested in box B's frame, with the direction negated.
    FixVecScale(&negDir, &dir, -0x10000);
    projAxis0 = FixVecDot((FixVector *)(pBoxB + 4), &negDir);
    projAxis1 = FixVecDot((FixVector *)(pBoxB + 7), &negDir);
    passHit = 0;
    maxDist = 0;
    for (i = 0; i < 4; i++) {
        corner = ((FixVector *)(pBoxA + 0xc))[i];
        delta.x = corner.x - posB->x;
        delta.y = corner.y - posB->y;
        delta.z = corner.z - posB->z;
        delta.y = 0;
        projCorner0 = FixVecDot((FixVector *)(pBoxB + 4), &delta);
        projCorner1 = FixVecDot((FixVector *)(pBoxB + 7), &delta);
        if (FIX_ABS(projCorner0) > pBoxB[0] || FIX_ABS(projCorner1) > pBoxB[1])
            continue;
        hit0 = 0;
        hit1 = 0;
        t0 = 0;
        t1 = 0;
        best = 0x7d000000;
        if (projAxis0 > 0x41) {
            t0 = FixDiv(pBoxB[0] - projCorner0, projAxis0);
            hit0 = 1;
        } else if (projAxis0 < -0x41) {
            t0 = FixDiv(FIX_ABS(pBoxB[0] + projCorner0), -projAxis0);
            hit0 = 1;
        }
        if (projAxis1 > 0x41) {
            t1 = FixDiv(pBoxB[1] - projCorner1, projAxis1);
            hit1 = 1;
        } else if (projAxis1 < -0x41) {
            t1 = FixDiv(FIX_ABS(pBoxB[1] + projCorner1), -projAxis1);
            hit1 = 1;
        }
        index = g_unk0x005915f4;
        if (hit0 && t0 < 0x7d000000) {
            best = t0;
            g_unk0x00590ec8[index] = 0;
        }
        if (hit1 && t1 < best) {
            best = t1;
            g_unk0x00590ec8[index] = 2;
        }
        if ((hit0 || hit1) && best > maxDist)
            maxDist = best;
        g_unk0x005914c4[index] = (char)i;
        g_unk0x005915f4 = (char)(index + 1);
        passHit = 1;
        hit = 1;
    }
    if (passHit != 0)
        FUN_004894b0(pBoxA, pBoxB, (int *)&dir, maxDist, scale);
    if (hit == 0) {
        hit = FUN_00488de0((FixVector *)pBoxA, (FixVector *)pBoxB, &dir, &maxDist);
        if (hit != 0)
            FUN_004894b0(pBoxA, pBoxB, (int *)&dir, maxDist, scale);
    }
    return hit;
}
// Resolves the car's contact with the face tracked in g_collisionFace: picks
// the contact axis, slides the car (position, eight corners and the face's
// four vertices) out of the surface and latches the impact direction and point
// from the surface descriptor in g_unk0x0059190c.
// match 61%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048e730
int FUN_0048e730(int *param_1, int *param_2, int param_3, char param_4)
{
    int *pFace;
    FixVector *pFaceVerts;
    int xRatio;
    int yRatio;
    int axis;
    int bestRatio;
    int haveX;
    int haveY;
    int len;
    int dot0;
    int dot1;
    int dot;
    int impulse;
    int i;
    BYTE surface;
    unsigned short angle;
    pFace = (int *)g_collisionFace;
    pFaceVerts = (FixVector *)((BYTE *)g_collisionFace + 0x30);
    xRatio = 0;
    yRatio = 0;
    axis = -1;
    FUN_00486c30(pFace, (int *)((BYTE *)g_collisionCar + 0x360),
                 (int *)((BYTE *)g_collisionCar + 0x2d0),
                 (FixVector *)((BYTE *)g_collisionCar + 0x270));
    // Offset of the tracked point from the car, flattened to the ground plane.
    g_unk0x00591968.x = param_1[0] - g_collisionCar->position.x;
    g_unk0x00591968.y = param_1[1] - g_collisionCar->position.y;
    g_unk0x00591968.z = param_1[2] - g_collisionCar->position.z;
    g_unk0x00591968.y = 0;
    dot0 = FixVecDot((FixVector *)(pFace + 4), &g_unk0x00591968);
    dot1 = FixVecDot((FixVector *)(pFace + 7), &g_unk0x00591968);
    if (pFace[0] < FIX_ABS(dot0))
        return 0;
    if (pFace[1] < FIX_ABS(dot1))
        return 0;
    g_unk0x00591950.x = param_1[0] - *(int *)((BYTE *)g_collisionCar + 0x2e8);
    g_unk0x00591950.y = param_1[1] - *(int *)((BYTE *)g_collisionCar + 0x2ec);
    g_unk0x00591950.z = param_1[2] - *(int *)((BYTE *)g_collisionCar + 0x2f0);
    g_unk0x00591950.y = 0;
    len = FixVecLength(&g_unk0x00591950);
    if (len <= 0)
        return 0;
    FixVecScaleRecip(&g_unk0x00591920, &g_unk0x00591950, len);
    // Fraction of the move left before the car's box leaves each face axis; the
    // smallest of the two is the axis the car is pushed back along.
    dot0 = FixVecDot(&g_unk0x00591920, (FixVector *)(pFace + 4));
    dot1 = FixVecDot(&g_unk0x00591920, (FixVector *)(pFace + 7));
    haveX = 0;
    haveY = 0;
    bestRatio = 0x7d000000;
    if (dot0 > 0x41) {
        xRatio = FixDiv(pFace[0] - dot0, dot0);
        haveX = 1;
    } else if (dot0 < -0x40) {
        xRatio = FixDiv(FIX_ABS(pFace[0] + dot0), -dot0);
        haveX = 1;
    }
    if (dot1 > 0x41) {
        yRatio = FixDiv(pFace[1] - dot1, dot1);
        haveY = 1;
    } else if (dot1 < -0x40) {
        yRatio = FixDiv(FIX_ABS(pFace[1] + dot1), -dot1);
        haveY = 1;
    }
    if (haveX && xRatio < bestRatio) {
        axis = 0;
        bestRatio = xRatio;
    }
    if (haveY && yRatio < bestRatio) {
        axis = 1;
        bestRatio = yRatio;
    }
    FixVecScale(&g_unk0x00591920, &g_unk0x00591920, -0x10000);
    FixVecScale(&g_unk0x00591900, &g_unk0x00591920, bestRatio);
    g_collisionCar->position.x += g_unk0x00591900.x;
    g_collisionCar->position.y += g_unk0x00591900.y;
    g_collisionCar->position.z += g_unk0x00591900.z;
    for (i = 0; i < 8; i++) {
        g_collisionCar->corners[i].x += g_unk0x00591900.x;
        g_collisionCar->corners[i].y += g_unk0x00591900.y;
        g_collisionCar->corners[i].z += g_unk0x00591900.z;
    }
    for (i = 0; i < 4; i++) {
        pFaceVerts[i].x += g_unk0x00591900.x;
        pFaceVerts[i].y += g_unk0x00591900.y;
        pFaceVerts[i].z += g_unk0x00591900.z;
    }
    FUN_0048c870(*(BYTE *)((BYTE *)g_collisionCar + 0xb1a), 0xff, (int *)&g_unk0x00591900, 0);
    // Surface descriptor of the tracked point: 0xff means there is no contact
    // surface, in which case the second tracked point decides the direction.
    surface = *(BYTE *)((BYTE *)g_unk0x0059190c + 0x2e + (param_3 != 0));
    if (surface == 0xff)
        goto noSurface;
    // Impact direction from the surface angle (12-bit angle into the sine
    // table, with the +0x400 entry as the perpendicular component).
    angle = (unsigned short)(__int64)((double)FixMul((int)surface << 16, 0x1cccc) * g_unk0x00511300);
    g_unk0x00591990.x = g_sinTable[angle & 0xfff];
    g_unk0x00591990.y = 0;
    g_unk0x00591990.z = g_sinTable[(angle + 0x400) & 0xfff];
    // Only a surface the car is moving into produces a bounce.
    dot = FixVecDot(&g_collisionCar->velocity, &g_unk0x00591990);
    if (dot >= 0)
        return 1;
    impulse = -FixMul(dot, g_unk0x0051fb00[param_4] + 0x4ccc);
    FixVecScale(&g_unk0x005918f0, &g_unk0x00591990, impulse);
    g_unk0x00591978 = *(FixVector *)param_1;
    g_unk0x00591978.y = g_collisionCar->position.y;
    CarPhysics_ApplyImpulse(&g_unk0x005918f0, &g_unk0x00591978, 1);
    len = FixVecLength(&g_unk0x005918f0);
    if (g_unk0x00591930 != 0 && g_unk0x005919a0 >= len)
        return 1;
    g_unk0x005919a0 = len;
    g_unk0x005918e0 = g_unk0x005918f0;
    g_unk0x00591ad0 = g_unk0x00591978;
    g_unk0x00591938 = g_unk0x00591990;
    g_unk0x00591930 = 1;
    g_unk0x0059199c = 2;
    return 1;
noSurface:
    if (axis == -1)
        return 1;
    g_unk0x00591990.x = param_2[0] - param_1[0];
    g_unk0x00591990.y = param_2[1] - param_1[1];
    g_unk0x00591990.z = param_2[2] - param_1[2];
    if (FIX_ABS(g_unk0x00591990.x) > FIX_ABS(g_unk0x00591990.y) &&
        FIX_ABS(g_unk0x00591990.x) > FIX_ABS(g_unk0x00591990.z))
        FixVecScaleRecip(&g_unk0x00591990, &g_unk0x00591990, FIX_ABS(g_unk0x00591990.x));
    else if (FIX_ABS(g_unk0x00591990.y) > FIX_ABS(g_unk0x00591990.x) &&
             FIX_ABS(g_unk0x00591990.y) > FIX_ABS(g_unk0x00591990.z))
        FixVecScaleRecip(&g_unk0x00591990, &g_unk0x00591990, FIX_ABS(g_unk0x00591990.y));
    else
        FixVecScaleRecip(&g_unk0x00591990, &g_unk0x00591990, FIX_ABS(g_unk0x00591990.z));
    len = FixVecLength(&g_unk0x00591990);
    if (len == 0) {
        g_unk0x00591990.x = 0;
        g_unk0x00591990.y = 0;
        g_unk0x00591990.z = 0;
    } else {
        FixVecScaleRecip(&g_unk0x00591990, &g_unk0x00591990, len);
    }
    if (FixVecDot(&g_unk0x00591990, &g_collisionCar->velocity) <= 0)
        return 1;
    // The contact axis picked above doubles as the bounce direction.
    if (axis == 0)
        g_unk0x00591990 = *(FixVector *)((BYTE *)g_collisionFace + 0x10);
    else
        g_unk0x00591990 = *(FixVector *)((BYTE *)g_collisionFace + 0x1c);
    dot = FixVecDot((FixVector *)((BYTE *)g_collisionCar + 0x408), &g_unk0x00591990);
    impulse = -FixMul(dot, g_unk0x0051fb00[param_4] + 0x10000);
    FixVecScale(&g_unk0x005918f0, &g_unk0x00591990, impulse);
    g_unk0x00591978 = *(FixVector *)param_1;
    g_unk0x00591978.y = g_collisionCar->position.y;
    CarPhysics_ApplyImpulse(&g_unk0x005918f0, &g_unk0x00591978, 1);
    len = FixVecLength(&g_unk0x005918f0);
    if (g_unk0x00591930 != 0 && g_unk0x005919a0 >= len)
        return 1;
    g_unk0x005919a0 = len;
    g_unk0x005918e0 = g_unk0x005918f0;
    g_unk0x00591ad0 = g_unk0x00591978;
    g_unk0x00591938 = g_unk0x00591990;
    g_unk0x00591930 = 1;
    g_unk0x0059199c = 2;
    return 1;
}

// --- trackside cameras (camera type 7) --------------------------------------
// A view record (see Car.cpp) whose camera stands at one of the stage's
// camera spots: g_unk0x00591750 lists them (0x6c bytes each: +2 heading, +4/+0x10/+0x1c
// basis, +0x28 position, +0x34 end of its dolly track, +0x40 trigger range,
// +0x4c.. zoom and shake settings). Spots turned (almost) 90 degrees behave as
// chase cameras. Every array below is indexed by the record index (0..3).

// GLOBAL: CMR2 0x005916d0
int g_unk0x005916d0[4];
// GLOBAL: CMR2 0x005916e0
int g_unk0x005916e0[4];
// GLOBAL: CMR2 0x00591700
int g_unk0x00591700[4];
// GLOBAL: CMR2 0x00591720
int g_unk0x00591720[4];
// GLOBAL: CMR2 0x00591754
int g_unk0x00591754[4];

extern float g_oneOverRandMax;
extern double g_minus65536;
void FUN_00486740(BYTE *pObj, int *pSrc, BYTE index, BYTE value);
void FUN_00486810(BYTE *pObj, int *pSrc, int param_3);
void FUN_00486910(BYTE *pObj, int *pSrc);
void FUN_004869e0(BYTE *pObj, FixMatrix *pRef);
void FUN_0048d950(BYTE *pSurface, FixMatrix *pMatrix);
void FUN_0048db00(BYTE *p, int step);
void FUN_0048dc30(BYTE *pCar, int step);
void FUN_0048dca0(BYTE *pCar, int amount);
void FUN_0048dce0(FixVector *pOut, BYTE *pSurface, Car *pCar, FixMatrix *pMatrix);
void FUN_0048ce80(BYTE *pRecord, FixMatrix *pRef);
void FUN_0048d0f0(BYTE *pRecord, FixMatrix *pRef);

#define SPOT(i) (g_unk0x00591750 + g_unk0x00591740[i] * 0x6c)

// Places a trackside camera on spot `spot` for a view record.
// FUNCTION: CMR2 0x0048cae0
void FUN_0048cae0(BYTE *pRecord, FixMatrix *pRef, int spot)
{
    BYTE *pSpot;
    BYTE index;
    short heading;
    FixVector position;
    FixVector d;
    int near_;

    pSpot = g_unk0x00591750 + spot * 0x6c;
    index = pRecord[0];
    g_unk0x00591740[index] = spot;
    heading = *(short *)(pSpot + 2);
    if (heading > 0x3f4 && heading < 0x40b) {
        FUN_00486740(pRecord, (int *)pRef, *((BYTE *)Car_Get(pRecord[2]) + 0xb1a), 1);
        return;
    }
    if (heading < -0x3f4 && heading > -0x40b) {
        FUN_00486740(pRecord, (int *)pRef, *((BYTE *)Car_Get(pRecord[2]) + 0xb1a), 2);
        return;
    }
    FixMatrix_GetPosition(&position, pRef);
    g_unk0x00591754[index] = 0;
    g_unk0x00591720[index] = 0;
    d.x = position.x - *(int *)(pSpot + 0x28);
    d.y = position.y - *(int *)(pSpot + 0x2c);
    d.z = position.z - *(int *)(pSpot + 0x30);
    if ((int)FixVec_Length(&d) < *(int *)(pSpot + 0x40) ||
        (FixVecDot(&d, (FixVector *)(pSpot + 0x1c)) < 0 && *(int *)(pSpot + 0x40) > 0))
        near_ = 1;
    else
        near_ = 0;
    g_unk0x005916d0[index] = near_;
    g_unk0x005916f0[index] = near_;
    FUN_0048ce80(pRecord, pRef);
    g_unk0x00591730[index] = 0;
}

// Per-frame update of a trackside camera: eases its zoom and shake settings
// towards the spot's near/far values once the car is in range.
// FUNCTION: CMR2 0x0048cc30
void FUN_0048cc30(BYTE *pRecord, FixMatrix *pRef)
{
    BYTE *pSpot;
    BYTE *pCar;
    BYTE index;
    short heading;
    FixVector position;
    FixVector d;
    int distance;
    int zoom;
    int shake;

    index = pRecord[0];
    heading = *(short *)(g_unk0x00591750 + 2 + g_unk0x00591740[index] * 0x6c);
    pSpot = SPOT(index);
    if (heading > 0x3f4 && heading < 0x40b) {
        pCar = (BYTE *)Car_Get(pRecord[2]);
        if (*(int *)(pCar + 0xc04) == 0 && *(int *)(pCar + 0xb60) == 0) {
            FUN_00486810(pRecord, (int *)pRef, 0);
            return;
        }
        FUN_00486810(pRecord, (int *)pRef, 1);
        return;
    }
    if (heading < -0x3f4 && heading > -0x40b) {
        pCar = (BYTE *)Car_Get(pRecord[2]);
        if (*(int *)(pCar + 0xc04) == 0 && *(int *)(pCar + 0xb60) == 0) {
            FUN_00486810(pRecord, (int *)pRef, 0);
            return;
        }
        FUN_00486810(pRecord, (int *)pRef, 1);
        return;
    }
    FixMatrix_GetPosition(&position, pRef);
    d.x = *(int *)(pSpot + 0x28) - position.x;
    d.y = *(int *)(pSpot + 0x2c) - position.y;
    d.z = *(int *)(pSpot + 0x30) - position.z;
    distance = FixVec_Length(&d);
    if (g_unk0x005916d0[index] != 0) {
        if (distance < g_unk0x005916e0[index]) {
            zoom = *(int *)(pSpot + 0x4c);
            shake = *(int *)(pSpot + 0x64);
        } else {
            zoom = *(int *)(pSpot + 0x50);
            shake = *(int *)(pSpot + 0x68);
        }
        g_unk0x00591754[index] = FixMul(g_unk0x00591754[index], 0xcccc) + FixMul(zoom, 0x3333);
        g_unk0x00591720[index] = FixMul(g_unk0x00591720[index], 0xcccc) + FixMul(shake, 0x3333);
        g_unk0x00591700[index] = FixMul(g_unk0x00591700[index], 0xe666) + FixMul(*(int *)(pSpot + 0x60), 0x1999);
        FUN_0048d950(pRecord, pRef);
        FUN_0048db00(pRecord, g_unk0x00591754[index]);
        FUN_0048dc30(pRecord, g_unk0x00591720[index]);
        FUN_0048dca0(pRecord, g_unk0x00591700[index]);
        FUN_0048d0f0(pRecord, pRef);
        return;
    }
    if (distance < *(int *)(pSpot + 0x40))
        g_unk0x005916d0[index] = 1;
    FUN_0048d0f0(pRecord, pRef);
}

// Restarts a trackside camera: back to the start of its dolly track (or to the
// car when the car is already in range).
// FUNCTION: CMR2 0x0048ce80
void FUN_0048ce80(BYTE *pRecord, FixMatrix *pRef)
{
    BYTE *pSpot;
    BYTE index;
    short heading;
    FixVector position;
    FixVector d;

    index = pRecord[0];
    heading = *(short *)(g_unk0x00591750 + 2 + g_unk0x00591740[index] * 0x6c);
    pSpot = SPOT(index);
    if (heading > 0x3f4 && heading < 0x40b) {
        FUN_00486910(pRecord, (int *)pRef);
        return;
    }
    if (heading < -0x3f4 && heading > -0x40b) {
        FUN_00486910(pRecord, (int *)pRef);
        return;
    }
    g_unk0x00591730[index] = 0;
    if (g_unk0x005916d0[index] != 0) {
        FUN_0048dce0(&g_unk0x00591868[index], pRecord, Car_Get(pRecord[2]), pRef);
        FUN_0048d950(pRecord, pRef);
        if (g_unk0x00591754[index] > 0)
            g_unk0x00591730[index] = 0x10000;
        g_unk0x005916f0[index] = 1;
    } else {
        FixMatrix_GetPosition(&position, pRef);
        d.x = position.x - *(int *)(pSpot + 0x28);
        d.y = position.y - *(int *)(pSpot + 0x2c);
        d.z = position.z - *(int *)(pSpot + 0x30);
        if ((int)FixVec_Length(&d) < *(int *)(pSpot + 0x40) ||
            (FixVecDot(&d, (FixVector *)(pSpot + 0x1c)) < 0 && *(int *)(pSpot + 0x40) > 0)) {
            FUN_0048d950(pRecord, pRef);
        } else {
            FixVecScale(&g_unk0x005916a0[index], (FixVector *)(pSpot + 0x1c), *(int *)(pSpot + 0x40));
            g_unk0x005916a0[index].x += *(int *)(pSpot + 0x28);
            g_unk0x005916a0[index].y += *(int *)(pSpot + 0x2c);
            g_unk0x005916a0[index].z += *(int *)(pSpot + 0x30);
            g_unk0x00591710[index] = *(int *)(pSpot + 0x54);
        }
        g_unk0x00591868[index].x = 0;
        g_unk0x00591868[index].y = 0;
        g_unk0x00591868[index].z = 0;
    }
    g_unk0x00591700[index] = 0;
    g_unk0x00591898[index] = g_unk0x005916a0[index];
    g_unk0x00591690[index] = g_unk0x00591710[index];
    FUN_0048d0f0(pRecord, pRef);
}

// Builds a trackside camera's matrix: its position (moved along the dolly track
// and shaken while the car is close), looking along the spot's basis or at the
// car, turned by the spot's heading, with the spot's zoom.
// FUNCTION: CMR2 0x0048d0f0
void FUN_0048d0f0(BYTE *pRecord, FixMatrix *pRef)
{
    FixMatrix turn;
    FixVector right;
    FixVector up;
    FixVector offset;
    FixVector toCar;
    FixVector carPos;
    FixVector shake;
    FixVector camera;
    BYTE *pSpot;
    unsigned int index;
    short heading;
    int amplitude;
    int zoom;

    index = pRecord[0];
    heading = *(short *)(g_unk0x00591750 + 2 + g_unk0x00591740[index] * 0x6c);
    pSpot = SPOT(index);
    if (heading > 0x3f4 && heading < 0x40b) {
        FUN_004869e0(pRecord, pRef);
        return;
    }
    if (heading < -0x3f4 && heading > -0x40b) {
        FUN_004869e0(pRecord, pRef);
        return;
    }
    FixMatrix_GetPosition(&carPos, pRef);
    camera = *(FixVector *)(pSpot + 0x28);
    if (*(int *)(pSpot + 0x60) > 0) {
        right.x = *(int *)(pSpot + 0x34) - *(int *)(pSpot + 0x28);
        right.y = *(int *)(pSpot + 0x38) - *(int *)(pSpot + 0x2c);
        right.z = *(int *)(pSpot + 0x3c) - *(int *)(pSpot + 0x30);
        FixVecScale(&up, &right, g_unk0x00591730[index]);
        FixVecScale(&up, &up, 0x20000);
        camera.x += up.x;
        camera.y += up.y;
        camera.z += up.z;
    }
    if (g_unk0x005916e0[index] < *(int *)(pSpot + 0x58)) {
        amplitude = FixMul(*(int *)(pSpot + 0x5c), 0x10000 - FixDiv(g_unk0x005916e0[index], *(int *)(pSpot + 0x58)));
        amplitude = FixMul(amplitude, *(int *)((BYTE *)Car_Get(pRecord[2]) + 0x778) / 2);
        shake.x = -0x8000 - (int)(__int64)((float)rand() * g_oneOverRandMax * g_minus65536);
        shake.y = -0x8000 - (int)(__int64)((float)rand() * g_oneOverRandMax * g_minus65536);
        shake.z = -0x8000 - (int)(__int64)((float)rand() * g_oneOverRandMax * g_minus65536);
        FIX_NORMALIZE_INTO(shake, shake);
        FixVecScale(&shake, &shake, amplitude);
        camera.x += shake.x;
        camera.y += shake.y;
        camera.z += shake.z;
    }
    toCar.x = camera.x - carPos.x;
    toCar.y = camera.y - carPos.y;
    toCar.z = camera.z - carPos.z;
    g_unk0x005916e0[index] = FixVec_Length(&toCar);
    if (g_unk0x005916d0[index] != 0) {
        offset.x = g_unk0x00591898[index].x - camera.x;
        offset.y = g_unk0x00591898[index].y - camera.y;
        offset.z = g_unk0x00591898[index].z - camera.z;
        FixVec_Normalize(&offset, &offset);
        zoom = g_unk0x00591690[index];
        up.x = 0;
        up.z = 0;
        up.y = 0x10000;
        FixVecCross(&right, &up, &offset);
        FixVecCross(&up, &offset, &right);
        FixVec_Normalize(&right, &right);
        FixVec_Normalize(&up, &up);
        FixVec_Normalize(&offset, &offset);
        FixMatrix_SetRight(&right, (FixMatrix *)(pRecord + 8));
        FixMatrix_SetUp(&up, (FixMatrix *)(pRecord + 8));
        FixMatrix_SetForward(&offset, (FixMatrix *)(pRecord + 8));
    } else {
        FixMatrix_SetRight((FixVector *)(pSpot + 4), (FixMatrix *)(pRecord + 8));
        FixMatrix_SetUp((FixVector *)(pSpot + 0x10), (FixMatrix *)(pRecord + 8));
        FixMatrix_SetForward((FixVector *)(pSpot + 0x1c), (FixMatrix *)(pRecord + 8));
        zoom = *(int *)(pSpot + 0x54);
    }
    if (zoom < 0x10000)
        *(int *)(pRecord + 0x54) = FixMul(0xa000, 0x10000);
    else
        *(int *)(pRecord + 0x54) = FixMul(0xa000, zoom);
    *(int *)(pRecord + 0x38) = 0;
    *(int *)(pRecord + 0x3c) = 0;
    *(int *)(pRecord + 0x40) = 0;
    FixMatrix_Identity(&turn);
    turn.forward.z = 0x10000;
    turn.right.x = FixCos(*(unsigned short *)(pSpot + 2));
    turn.up.y = turn.right.x;
    turn.right.y = -FixSin(*(unsigned short *)(pSpot + 2));
    turn.right.z = 0;
    turn.up.x = FixSin(*(unsigned short *)(pSpot + 2));
    turn.up.z = 0;
    turn.forward.x = 0;
    turn.forward.y = 0;
    FixMatrix_Multiply((FixMatrix *)(pRecord + 8), &turn, (FixMatrix *)(pRecord + 8));
    FixMatrix_SetPosition(&camera, (FixMatrix *)(pRecord + 8));
    *(int *)(pRecord + 0x48) = 0;
    *(int *)(pRecord + 0x4c) = 0x1999;
    *(int *)(pRecord + 0x4c) = FixMul(*(int *)(pRecord + 0x4c), 0x50000);
    *(int *)(pRecord + 0x4c) = FixMul(*(int *)(pRecord + 0x4c), FixDiv(*(int *)(pRecord + 0x54), 0xa000));
    *(int *)(pRecord + 0x50) = 0;
    *(int *)(pRecord + 0x58) = 0;
    *(int *)(pRecord + 0x5c) = 0x10000;
}
#undef SPOT

// GLOBAL: CMR2 0x0051c9b4
char g_strCarModelC5Bfl[] = "%sc5.bfl";
// GLOBAL: CMR2 0x0051c9c0
char g_strCarModelA5Bfl[] = "%sa5.bfl";
// GLOBAL: CMR2 0x0051c9cc
char g_strCarModelC5C3d[] = "%sc5.c3d";
// GLOBAL: CMR2 0x0051c9d8
char g_strCarModelA5C3d[] = "%sa5.c3d";

char *FUN_004200d0(int car);
void FUN_00477340(int player);
int FUN_004b9380(unsigned int, unsigned int, unsigned int);

// Per-car node row at 0x58d528 (0x1c bytes; g_unk0x0058d530 is the same rows
// seen from +8): +0 node 0x1a, +4 node 0x1c, +8 new node, +0xc node, +0x10
// node 0x1b, +0x14 node 0x16, +0x18 node 0x17.
#define CAR_NODE_ROW(i) ((int *)(g_unk0x0058d530 - 8 + (i) * 0x1c))

// Loads the interior (cockpit) model of a player's car and hooks its nodes
// (steering wheel, dash, driver) into the car's scene graph.
// FUNCTION: CMR2 0x004760a0
void FUN_004760a0(int record, BYTE car)
{
    Car *pCar;
    int ok;
    int *pRow;
    int *pOffset;

    ok = 1;
    if (CGameInfo::FUN_00405e00() && car > 0)
        return;
    pCar = Car_Get(car);
    if ((short)car < Car_GetOrderCount() && *(SceneNode **)((BYTE *)pCar + 0x720) != NULL) {
        g_stageBlock_58d340[car] = (int)SceneNode_FindByType(*(SceneNode **)((BYTE *)pCar + 0x720), 9);
        g_stageBlock_58d47c[car] = (int)SceneNode_FindByType(*(SceneNode **)((BYTE *)pCar + 0x720), 5);
    } else {
        ok = 0;
    }
    if (car >= 2)
        return;
    pRow = CAR_NODE_ROW(car);
    if (pRow[3] != 0)
        return;
    if ((char)RallyData_GetFlag24() || ok == 0)
        return;
    if (CGameInfo::FUN_00405d10() == 0)
        sprintf(CFrontend::m_stringDest, g_strCarModelA5C3d,
                FUN_004200d0(RallyData_FUN_004086b0(FUN_0041b370() + car)));
    else
        sprintf(CFrontend::m_stringDest, g_strCarModelC5C3d,
                FUN_004200d0(RallyData_FUN_004086b0(FUN_0041b370() + car)));
    g_unk0x0058d6a0[car] = CFileBuffer::GetGenericFileBuffer(CFrontend::m_stringDest, FALSE);
    if (CGameInfo::FUN_00405d10() == 0)
        sprintf(CFrontend::m_stringDest, g_strCarModelA5Bfl,
                FUN_004200d0(RallyData_FUN_004086b0(FUN_0041b370() + car)));
    else
        sprintf(CFrontend::m_stringDest, g_strCarModelC5Bfl,
                FUN_004200d0(RallyData_FUN_004086b0(FUN_0041b370() + car)));
    CGenericFileLoader::FUN_004a9d70((GenericFile *)g_unk0x0058d3b8, CFrontend::m_stringDest);
    FUN_00476540(car);
    if (g_unk0x0058d6a0[car] == NULL)
        return;
    g_unk0x0058d49c[car] = (void *)FUN_004b9380((unsigned int)g_unk0x0058d6a0[car],
                                                *(unsigned int *)((BYTE *)pCar + 0x720),
                                                (unsigned int)g_unk0x0058d3b8);
    pRow[2] = (int)SceneNode_Create((SceneNode *)g_unk0x0058d49c[car]);
    pRow[0] = (int)SceneNode_FindByType((SceneNode *)g_unk0x0058d49c[car], 0x1a);
    pRow[1] = (int)SceneNode_FindByType((SceneNode *)g_unk0x0058d49c[car], 0x1c);
    pRow[3] = (int)SceneNode_Create(*(SceneNode **)((BYTE *)pCar + 0x720));
    pRow[4] = (int)SceneNode_FindByType((SceneNode *)g_unk0x0058d49c[car], 0x1b);
    pRow[5] = (int)SceneNode_FindByType((SceneNode *)g_unk0x0058d49c[car], 0x16);
    pRow[6] = (int)SceneNode_FindByType((SceneNode *)g_unk0x0058d49c[car], 0x17);
    memcpy((BYTE *)pRow[2] + 0x98, (BYTE *)pRow[1] + 0x98, 0x40);
    *(int *)(pRow[2] + 0xc) = 0;
    *(int *)(pRow[2] + 0x178) = 3;
    *(int *)(pRow[2] + 0x30) = *(int *)(pRow[1] + 0x30);
    SceneNode_Reparent((SceneNode *)pRow[1], (SceneNode *)g_unk0x0058d49c[car]);
    pOffset = *(int **)(g_unk0x0058d4f0 + car * 0x1c + 8);
    *(int *)(pRow[3] + 0xc8) = *(int *)(pRow[4] + 0xc8) + pOffset[0];
    *(int *)(pRow[3] + 0xcc) = *(int *)(pRow[4] + 0xcc) + pOffset[1];
    *(int *)(pRow[3] + 0xd0) = *(int *)(pRow[4] + 0xd0) + pOffset[2];
    g_unk0x0058d6a8[car] = 1;
    g_unk0x0058d4d0[(BYTE)record] = *((BYTE *)pCar + 0xb1b);
    *(int *)(g_unk0x0058d2f8 + car * 12) = *(int *)(pRow[3] + 0xc8);
    *(int *)(g_unk0x0058d2f8 + car * 12 + 4) = *(int *)(pRow[3] + 0xcc);
    *(int *)(g_unk0x0058d2f8 + car * 12 + 8) = *(int *)(pRow[3] + 0xd0);
    FUN_00477340(car);
}
#undef CAR_NODE_ROW

// --- CPU driver input (0x47b000-0x47c5ac) -----------------------------------

#define AI_INT(off) (*(int *)((BYTE *)g_unk0x0058e178 + (off)))
#define AI_CHAR(off) (*(char *)((BYTE *)g_unk0x0058e178 + (off)))

// AI data of the stage: base pointer and the per-car route headers.
// GLOBAL: CMR2 0x0058e378
BYTE *g_unk0x0058e378;
// GLOBAL: CMR2 0x0058e37c
int *g_unk0x0058e37c[6];
// GLOBAL: CMR2 0x0051f4c4
char g_strAi2Format[] = "%s.ai2";
// GLOBAL: CMR2 0x0051f4cc
char g_strAi1Format[] = "%s.ai1";
// GLOBAL: CMR2 0x0051f4d4
char g_strAi0Format[] = "%s.ai0";

int RallyData_FUN_00421370(BYTE *p);
void FUN_00498620(Car *pCar, unsigned int mask, int *pOut, int variant);
BYTE *FUN_00498590(BYTE *p, int unused, int count);
BYTE FUN_0042b710(int index);
void FUN_0043f570(Car *pCar);
void FUN_0047b0e0(int player, int device);
BYTE *FUN_0041f900(void);
int StageTiming_FUN_00455460(void);
unsigned int RallyData_GetFlag22(void);

// Works out the controls of a CPU car from the route data: the route
// segment's steering hint, the obstacle state (FUN_0047c5e0), the overtaking
// logic and the driver's errors. `preview` != 0 only updates the AI state.
// FUNCTION: CMR2 0x0047bdd0
void FUN_0047bdd0(Car *pCar, int car, int preview)
{
    int modes[3];
    int controls[4];
    int handbrake;
    int *pRoute;
    int table;
    int node;
    int variant;
    int ahead;
    int behind;
    int force;

    controls[0] = 0;
    controls[1] = 0;
    controls[2] = 0;
    pRoute = g_unk0x0058e37c[car];
    controls[3] = 0;
    AI_CHAR(0xa4) = 0;
    modes[0] = 0;
    *((BYTE *)g_unk0x0058e178 + 0xab + car) = 0;
    modes[1] = 0;
    handbrake = 0;
    table = *pRoute;
    if (table == 0)
        return;
    node = RallyData_FUN_00421370((BYTE *)pCar);
    AI_INT(0x54) = node;
    FUN_0047cbc0(car, table, node, &variant, &modes[2]);
    *(unsigned int *)g_unk0x0058e394[table] |= 0xfffffffc;
    FUN_00498620(pCar, *(unsigned int *)g_unk0x0058e394[table], g_unk0x0058e178, variant);
    ahead = -AI_INT(0x34) - (int)(__int64)((double)*(signed char *)(g_unk0x0058e4a4 + node * 0x10 + 0xa) * g_minus65536);
    behind = AI_INT(0x34) - (int)(__int64)((double)*(signed char *)(g_unk0x0058e4a4 + node * 0x10 + 0xe) * g_minus65536);
    if (ahead < behind)
        g_unk0x0058e230[car] = ahead;
    else
        g_unk0x0058e230[car] = behind;
    AI_CHAR(0xa4) = (char)FUN_0047c5e0((int)g_unk0x0058e178);
    if ((char)RallyData_FUN_00406940() == 2 && (char)RallyData_FUN_00406950() == 1 && (unsigned int)node > 0xdd &&
        (unsigned int)node < 0xe4)
        AI_CHAR(0xa4) = 0;
    FUN_0047cd10(car, modes, node, g_unk0x0058e178);
    if (CGameInfo::FUN_004063f0(3))
        *((BYTE *)g_unk0x0058e178 + 0xab + car) =
            (BYTE)FUN_0047d0e0(car, (BYTE *)g_unk0x0058e178 + 0xa5, &modes[1], (BYTE *)g_unk0x0058e178 + 0xb1 + car);
    if (CGameInfo::FUN_004063f0(0))
        StageObject_UpdateApproachingCar(car, node);
    if (AI_CHAR(0xa4) == 0) {
        FUN_0047c9a0(modes[2], (int)controls, g_unk0x0058e178);
        switch (modes[0]) {
        case 1:
            controls[0] = 0;
            controls[1] = 0x3f;
            break;
        case 2:
            controls[0] = 0x3f;
            controls[1] = 0;
            break;
        case 3:
            controls[0] = 0;
            controls[1] = 0x3f;
            controls[3] = 0x3f;
            break;
        case 4:
            controls[1] = 0;
            controls[0] = 0x3f;
            controls[3] = 0x3f;
            break;
        }
        if (*((char *)g_unk0x0058e178 + 0xb1 + car) == -1) {
            if (AI_INT(0x78) > -0x140000) {
                controls[0] = 0;
                controls[1] = 0x3f;
            }
        } else if (*((char *)g_unk0x0058e178 + 0xb1 + car) == 1 && AI_INT(0x78) < 0x140000) {
            controls[0] = 0x3f;
            controls[1] = 0;
        }
        if (AI_INT(0xc) < -30000 && AI_INT(0) > 0xa0000)
            controls[2] = 0;
        if (AI_INT(0x64) > -0xa0000 && AI_INT(8) < -0x7d)
            controls[0] = 0;
        if (AI_INT(0x64) < 0xa0000 && AI_INT(8) > 0x7d)
            controls[1] = 0;
    } else {
        switch (AI_CHAR(0xa4)) {
        case 1:
            controls[1] = 0x3f;
            controls[2] = 0x3f;
            break;
        case 2:
            controls[1] = 0x3f;
            controls[3] = 0x3f;
            break;
        case 3:
            controls[0] = 0x3f;
            controls[2] = 0x3f;
            break;
        case 4:
            controls[0] = 0x3f;
            controls[3] = 0x3f;
            break;
        case 5:
            controls[2] = 0x3f;
            break;
        case 6:
            controls[3] = 0x3f;
            break;
        case 7:
            controls[1] = 0x3f;
            break;
        case 8:
            controls[0] = 0x3f;
            break;
        }
        if ((AI_INT(0x38) > 0x2d0000 || AI_INT(0x38) < -0x2d0000) && AI_INT(0) >= 0x190000) {
            controls[2] = 0;
            controls[3] = 0;
        }
    }
    if (CGameInfo::FUN_004063f0(0) && FUN_0047cd00(car))
        force = 1;
    else
        force = handbrake;
    if (preview != 0)
        return;
    pCar->flag0x1d0[0] = 0;
    pCar->flag0x1d0[1] = 0;
    pCar->flag0x1d0[2] = 0;
    pCar->flag0x1d0[3] = 0;
    pCar->field_0x1d8 = 0;
    if (controls[0] > 0)
        pCar->flag0x1d0[0] = 0x3f;
    if (controls[1] > 0)
        pCar->flag0x1d0[1] = 0x3f;
    if (controls[2] > 0)
        pCar->flag0x1d0[2] = 0x3f;
    if (controls[3] > 0)
        pCar->flag0x1d0[3] = 0x3f;
    if (force > 0)
        pCar->field_0x1d8 = 1;
}

// CPU driving of the car in race order slot `slot`.
// FUNCTION: CMR2 0x0047b620
void FUN_0047b620(int slot)
{
    if ((char)RallyData_FUN_00407e70())
        FUN_0047bdd0(g_unk0x0058e0a0, slot, 0);
}

// Per-frame input of the car in race order slot `slot`: player cars read their
// device, CPU cars are driven by the AI; the automatic gearbox is engaged.
// FUNCTION: CMR2 0x0047b000
void FUN_0047b000(int slot)
{
    char device;

    g_unk0x0058e0a0 = Car_Get(Car_GetOrder()[slot]);
    *(int *)((BYTE *)g_unk0x0058e0a0 + 0x1dc) = 0;
    g_unk0x0058e0a0->field_0x1d8 = 0;
    *((BYTE *)g_unk0x0058e0a0 + 0x1d4) = 0;
    g_unk0x0058e0a0->flag0x1d0[3] = 0;
    g_unk0x0058e0a0->flag0x1d0[2] = 0;
    g_unk0x0058e0a0->flag0x1d0[1] = 0;
    g_unk0x0058e0a0->flag0x1d0[0] = 0;
    if ((char)RallyData_GetFlag22())
        CGameInfo::FUN_00405d80();
    device = (char)FUN_0042b710(g_unk0x0058e0a0->field_0xb1a);
    if (device != -1)
        FUN_0047b0e0(slot, device);
    else
        FUN_0047b620(slot);
    if (g_unk0x0058e0a0->field_0xb9c == 0) {
        if (g_unk0x0058e0a0->field_0xb48 != 1)
            FUN_0043f570(g_unk0x0058e0a0);
        g_unk0x0058e0a0->field_0xb9c = 1;
        return;
    }
    g_unk0x0058e0a0->field_0xb9c = 1;
}

// Per-frame input of a car waiting at the start line: players keep their
// device, CPU cars blip the throttle at random intervals.
// FUNCTION: CMR2 0x0047b640
void FUN_0047b640(int slot)
{
    char device;
    char count;

    Car_GetOrderCount();
    g_unk0x0058e0a0 = Car_Get(Car_GetOrder()[slot]);
    *(int *)((BYTE *)g_unk0x0058e0a0 + 0x1e0) = 0;
    *(int *)((BYTE *)g_unk0x0058e0a0 + 0x1dc) = 0;
    g_unk0x0058e0a0->field_0x1d8 = 0;
    *((BYTE *)g_unk0x0058e0a0 + 0x1d4) = 0;
    g_unk0x0058e0a0->flag0x1d0[3] = 0;
    g_unk0x0058e0a0->flag0x1d0[2] = 0;
    g_unk0x0058e0a0->flag0x1d0[1] = 0;
    g_unk0x0058e0a0->flag0x1d0[0] = 0;
    device = (char)FUN_0042b710(g_unk0x0058e0a0->field_0xb1a);
    if (device != -1) {
        FUN_0047b0e0(slot, device);
    } else {
        count = *((char *)g_unk0x0058e0a0 + 0xb47);
        if (count < 0) {
            *((char *)g_unk0x0058e0a0 + 0xb47) = count + 1;
            if (*((char *)g_unk0x0058e0a0 + 0xb47) == 0)
                goto reroll;
        } else if (count < 1) {
        reroll:
            *((char *)g_unk0x0058e0a0 + 0xb47) = (char)(rand() % 10) + 5;
        } else {
            *((char *)g_unk0x0058e0a0 + 0xb47) = count - 1;
            if (*((char *)g_unk0x0058e0a0 + 0xb47) == 0)
                *((char *)g_unk0x0058e0a0 + 0xb47) = -5 - (char)(rand() % 10);
        }
        if (*((char *)g_unk0x0058e0a0 + 0xb47) < 1)
            g_unk0x0058e0a0->flag0x1d0[2] = 0;
        else
            g_unk0x0058e0a0->flag0x1d0[2] = 0x3f;
    }
    g_unk0x0058e0a0->field_0x1d8 = 1;
    *(int *)((BYTE *)g_unk0x0058e0a0 + 0x1dc) = 0;
    g_unk0x0058e0a0->field_0xb9c = 0;
}

// Input of a car that has finished: driven as usual, then braked to a stop.
// FUNCTION: CMR2 0x0047b7b0
void FUN_0047b7b0(int slot)
{
    FUN_0047b000(slot);
    g_unk0x0058e0a0 = Car_Get(Car_GetOrder()[slot]);
    if (g_unk0x0058e0a0->field_0xb94 != 0)
        g_unk0x0058e0a0->flag0x1d0[3] = 0;
    else
        g_unk0x0058e0a0->flag0x1d0[3] = 1;
    g_unk0x0058e0a0->flag0x1d0[2] = 0;
    g_unk0x0058e0a0->field_0x1d8 = 1;
    g_unk0x0058e0a0->field_0xb9c = 0;
    *(int *)((BYTE *)g_unk0x0058e0a0 + 0x1e4) = 0;
    FixVecScale(&g_unk0x0058e0a0->velocity, &g_unk0x0058e0a0->velocity, 0xf851);
}

// Loads the stage's AI route data: the three difficulty files, the one for the
// current difficulty copied into the stage buffer, and the pointer tables into
// it (routes, 0x88-byte tables, 0x14-, 0x10- and 0x20-byte records). Returns
// the end of the data.
// FUNCTION: CMR2 0x0047c2f0
BYTE *FUN_0047c2f0(void)
{
    BYTE *files[3];
    DWORD sizes[3];
    DWORD size;
    BYTE *pData;
    BYTE *pEnd;
    BYTE level;
    BYTE *p;
    BYTE *pEntry;
    int i;
    int k;

    pData = (BYTE *)StageTiming_FUN_00455460();
    size = 0;
    sprintf(CFrontend::m_stringDest, g_strAi0Format, FUN_0041f900());
    files[0] = (BYTE *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), CFrontend::m_stringDest,
                                                    0, &size, 0);
    sizes[0] = size;
    size = 0;
    sprintf(CFrontend::m_stringDest, g_strAi1Format, FUN_0041f900());
    files[1] = (BYTE *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), CFrontend::m_stringDest,
                                                    0, &size, 0);
    sizes[1] = size;
    size = 0;
    sprintf(CFrontend::m_stringDest, g_strAi2Format, FUN_0041f900());
    files[2] = (BYTE *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), CFrontend::m_stringDest,
                                                    0, &size, 0);
    sizes[2] = size;
    if ((char)RallyData_GetFlag24() == 0)
        return pData;
    level = CGameInfo::FUN_00405d90();
    if (pData == NULL) {
        for (i = 0; i < 3; i++) {
            if (files[i] != NULL) {
                pData = files[i];
                break;
            }
        }
        if (i == 3)
            pData = pEnd;
    }
    if (pData != files[level])
        memcpy(pData, files[level], sizes[level]);
    pEnd = pData + sizes[level];
    if (pData == NULL)
        return pEnd;
    g_unk0x0058e378 = pData;
    p = pData;
    for (i = 0; i < 6; i++) {
        g_unk0x0058e37c[i] = (int *)p;
        p += 4;
    }
    *((char *)g_unk0x0058e394 + 0x109) = *p;
    p += 4;
    for (i = 1; i <= *((char *)g_unk0x0058e394 + 0x109); i++) {
        g_unk0x0058e394[i] = p;
        p += 0x88;
    }
    *((char *)g_unk0x0058e394 + 0x10a) = *p;
    p += 4;
    for (i = 1; i <= *((char *)g_unk0x0058e394 + 0x10a); i++) {
        g_unk0x0058e394[6 + i] = p;
        p += 0x14;
    }
    *((char *)g_unk0x0058e394 + 0x108) = *p;
    p += 4;
    for (i = 1; i <= *((char *)g_unk0x0058e394 + 0x108); i++) {
        g_unk0x0058e394[0x38 + i] = p;
        p += 0x10;
    }
    *((char *)g_unk0x0058e394 + 0x10b) = *p;
    p += 4;
    for (i = 1; i <= *((char *)g_unk0x0058e394 + 0x10b); i++) {
        g_unk0x0058e394[0x2e + i] = p;
        p += 0x20;
    }
    for (i = 1; i <= *((char *)g_unk0x0058e394 + 0x10b); i++) {
        pEntry = g_unk0x0058e394[0x2e + i];
        if (*(int *)(pEntry + 8) > 1) {
            for (k = 1; k < *(int *)(pEntry + 8); k++) {
                *(BYTE **)(pEntry + 0x10 + k * 4) = p;
                p += ((char)pEntry[0xc + k - 1] + 1) * (char)pEntry[0xc + k] * 4;
            }
        }
    }
    g_unk0x0058e394[0x43] = p;
    g_unk0x0058e4a4 = p + 0x100;
    FUN_00498590(g_unk0x0058e4a4 + 0x1720, (int)&g_unk0x0058e394[0x38], *((char *)g_unk0x0058e394 + 0x108));
    return pEnd;
}
#undef AI_INT
#undef AI_CHAR

void FUN_0046c2a0(int param_1, BYTE param_2);
void FUN_0046c390(int *pState, BYTE car);
int FUN_0046c4b0(int *pState, BYTE *pIn, BYTE car, BYTE *pCounter);
void FUN_0046d610(BYTE *p);
void FUN_00496e00(Car *pCar);
void FUN_00477ce0(int car);
void FUN_004643f0(int param_1);
void FUN_0045f4a0(void);

// Plays back one frame of a replay stream: at the start of a section it
// restores the recorded car state, then feeds the recorded controls and picks
// up the camera event of the current frame. Ends the replay after the last
// section.
// FUNCTION: CMR2 0x0046cfa0
void FUN_0046cfa0(int *pState)
{
    BYTE *p;
    Car *pCar;
    BYTE *pEvents;
    BYTE *pEvent;
    int count;
    int k;
    int section;
    char found;

    p = (BYTE *)pState;
    if (p == NULL)
        return;
    pCar = Car_Get(p[0x20]);
    if (*(int *)(p + 4) == 0 || *(int *)(p + 0x1c) == 2 || *((BYTE *)pCar + 0xb43) == 0)
        return;
    if (*(int *)(p + 8) == 0) {
        *(short *)(p + 0x10a) = 0;
        p[0x21] = 0;
        *(int *)(p + 0x14) = 1;
        if (*(int *)(p + 0x1c) != 0) {
            *(int *)(p + 8) = 1;
            *(int *)(p + 0x34) = *(short *)(p + 0x108) * 0x5c + *(int *)(p + 0x30);
            return;
        }
        *(int *)(p + 8) = 1;
        *(int *)(p + 0x28) = *(int *)(p + 0x24) + *(short *)(p + 0x108) * 0x114c;
        return;
    }
    if (*(short *)(p + 0x10a) < 0) {
        if (*(int *)(p + 0x1c) == 0)
            FUN_0046c2a0(*(int *)(p + 0x24) + *(short *)(p + 0x108) * 0x114c, p[0x20]);
        else
            FUN_0046c390((int *)(*(short *)(p + 0x108) * 0x5c + *(int *)(p + 0x30)), p[0x20]);
        if (*(short *)(p + 0x10a) != -1) {
            *(int *)((BYTE *)pCar + 0x860) = 0;
            *(int *)((BYTE *)pCar + 0x864) = 0;
            *(int *)((BYTE *)pCar + 0x868) = 0;
            *(int *)((BYTE *)pCar + 0x86c) = 0;
        }
        (*(short *)(p + 0x10a))++;
    }
    if (*(short *)(p + 0x10a) < 0)
        return;
    section = *(short *)(p + 0x108);
    if (*(short *)(*(BYTE **)(p + 0x104) + section * 2) != 0) {
        if (FUN_0046c4b0(pState,
                         (BYTE *)(*(int *)(p + 0x3c) + (*(short *)(p + 0xfe) * section + *(short *)(p + 0x10a)) * 4),
                         p[0x20], p + 0x21))
            (*(short *)(p + 0x10a))++;
        found = -1;
        pEvent = NULL;
        section = *(short *)(p + 0x108);
        if (*(int *)(p + 0x1c) == 0) {
            count = *(BYTE *)(*(int *)(p + 0x24) + section * 0x114c + 0x1148);
            pEvents = (BYTE *)(*(int *)(p + 0x24) + section * 0x114c + 0x110c);
        } else {
            count = *(BYTE *)(*(int *)(p + 0x30) + section * 0x5c + 0x58);
            pEvents = (BYTE *)(*(int *)(p + 0x30) + section * 0x5c + 0x1c);
        }
        for (k = 0; k < count; k++) {
            pEvent = pEvents + k * 6;
            if (*(short *)(p + 0x10a) < *(short *)pEvent ||
                (*(short *)(p + 0x10a) == *(short *)pEvent && (short)p[0x21] < *(short *)(pEvent + 2))) {
                found = (char)k;
                break;
            }
        }
        if (found == -1)
            found = (char)count;
        // the original indexes from the last event examined, not from the table start
        if ((char)(found - 1) >= 0)
            p[0x10c] = pEvent[(char)(found - 1) * 6 + 4];
    }
    if (*(short *)(p + 0x10a) == *(short *)(*(BYTE **)(p + 0x104) + *(short *)(p + 0x108) * 2)) {
        *(int *)(p + 8) = 0;
        (*(short *)(p + 0x108))++;
        if (*(short *)(p + 0x108) == *(short *)(p + 0x100))
            FUN_0046d2a0(pState);
    }
}

// Plays back one frame of every replay stream (in single-player time trial
// only the non-player ones).
// FUNCTION: CMR2 0x0046d270
void FUN_0046d270(void)
{
    void ***pp;

    for (pp = g_unk0x00588d40; (int)pp < (int)(g_unk0x00588d40 + 16); pp++) {
        if (CGameInfo::FUN_00406320() == 0 || CGameInfo::FUN_00405d80() != 6)
            FUN_0046cfa0(*(int **)*pp);
    }
}

// Records one frame of every replay stream (same condition as FUN_0046d270).
// FUNCTION: CMR2 0x0046d5e0
void FUN_0046d5e0(void)
{
    void ***pp;

    for (pp = g_unk0x00588d40; (int)pp < (int)(g_unk0x00588d40 + 16); pp++) {
        if (CGameInfo::FUN_00406320() == 0 || CGameInfo::FUN_00405d80() != 6)
            FUN_0046d610(*(BYTE **)*pp);
    }
}

// Restarts the replay of the ghost car, keeping its controller index.
// FUNCTION: CMR2 0x00466030
void FUN_00466030(int a, int b)
{
    int index;

    index = g_unk0x0058875c->field_0xb1a;
    FUN_0046cce0((int)g_unk0x00588758, a, b, index);
    g_unk0x0058875c->field_0xb1a = index;
    *(int *)((BYTE *)g_unk0x0058875c + 0xc0c) = 1;
}

// Per-frame physics of the cars in `pOrder` that are not replayed, then the
// stage weather.
// FUNCTION: CMR2 0x004664c0
void FUN_004664c0(short *pOrder, short count)
{
    int i;
    Car *pCar;

    for (i = 0; i < count; i++, pOrder++) {
        pCar = Car_Get(*pOrder);
        if (*(int *)((BYTE *)pCar + 0xc0c) == 0) {
            if (*(int *)((BYTE *)pCar + 0xb70) == 0)
                FUN_00496e00(pCar);
            FUN_00477ce0(i);
            FUN_004643f0((int)pCar);
        }
    }
    FUN_0045f4a0();
}

// Picks a random point on a random edge of a random triangle of mesh `index`
// of a car's damage parts (vertices are floats, 0x30 bytes apart). Returns 0
// when the part has no mesh.
// FUNCTION: CMR2 0x00469c30
int FUN_00469c30(FixVector *pOut, int *pParts, int index)
{
    BYTE *pTri;
    float *pA;
    float *pB;
    BYTE *pVerts;
    int t;
    int edge;
    FixVector a;
    FixVector d;

    if (pParts[index] == 0) {
        pOut->x = 0;
        pOut->y = 0;
        pOut->z = 0;
        return 0;
    }
    pTri = (BYTE *)(rand() % *(int *)(pParts[index] + 0x28));
    edge = rand() % 3;
    t = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
    pTri = *(BYTE **)(pParts[index] + 0x24) + (int)pTri * 0x4c;
    switch (edge) {
    case 0:
        pVerts = *(BYTE **)(pParts[index] + 0xc);
        pA = (float *)(pVerts + *(unsigned short *)(pTri + 0x40) * 0x30);
        pB = (float *)(pVerts + *(unsigned short *)(pTri + 0x42) * 0x30);
        break;
    case 1:
        pVerts = *(BYTE **)(pParts[index] + 0xc);
        pA = (float *)(pVerts + *(unsigned short *)(pTri + 0x42) * 0x30);
        pB = (float *)(pVerts + *(unsigned short *)(pTri + 0x44) * 0x30);
        break;
    default:
        pVerts = *(BYTE **)(pParts[index] + 0xc);
        pA = (float *)(pVerts + *(unsigned short *)(pTri + 0x44) * 0x30);
        pB = (float *)(pVerts + *(unsigned short *)(pTri + 0x40) * 0x30);
        break;
    }
    a.x = (int)(__int64)(pA[0] * CGraphics::m_65536);
    a.y = (int)(__int64)(pA[1] * CGraphics::m_65536);
    a.z = (int)(__int64)(pA[2] * CGraphics::m_65536);
    d.x = (int)(__int64)(pB[0] * CGraphics::m_65536) - a.x;
    d.y = (int)(__int64)(pB[1] * CGraphics::m_65536) - a.y;
    d.z = (int)(__int64)(pB[2] * CGraphics::m_65536) - a.z;
    FixVecScale(&d, &d, t);
    pOut->x = d.x + a.x;
    pOut->y = d.y + a.y;
    pOut->z = d.z + a.z;
    return 1;
}

// GLOBAL: CMR2 0x00547908
BillboardDef g_unk0x00547908;

extern int g_unk0x0051bd3c;
extern int g_unk0x005477f0;
int *FUN_00469680(int index);
int FixMatrix_RotateVector(FixVector *pOut, FixVector *pV, FixMatrix *pM);


struct Quad2DVertices;
struct Quad2D;
void Quad2D_Queue(Quad2DVertices *pVerts, Texture *pTexture, Quad2D *pDest);
int FUN_00460ca0(int amount, Car *pCar);
void Scene_GetLightColour(DWORD *pColour, int level);
extern BYTE g_unk0x00543da0Block[0x90];
#define g_unk0x00543da0 (*(int *)g_unk0x00543da0Block)
extern int g_unk0x00543e90;
extern int g_unk0x00543ea4;
extern int g_unk0x00543ea8[3];
extern BillboardDef g_unk0x00543ed0;
extern BillboardDef g_unk0x00543f00;
extern double g_unk0x00511300;
#define WEATHER_QUAD(o) (*(float *)(g_unk0x00543da0Block + (o)))

// Draws the precipitation of one view. Rain: each drop is a streak quad
// along the wind (+0x20) widened across the view, plus a splash billboard for
// the drops that hit the ground, then the car throws spray. Snow: each flake
// is a billboard swaying along +0x38 with the sine of its phase.
// FUNCTION: CMR2 0x00460390
void FUN_00460390(int index, int view)
{
    BYTE *pView;
    FixVector *pPos;
    int count;
    int i;
    int fade;
    int amount;
    BYTE alpha;
    FixVector saved;
    FixVector forward;
    FixVector side;
    FixVector trail;
    FixVector offset;
    float sideF[3];
    float trailF[3];
    StageDeformNode *pNode;

    alpha = 0x1e;
    pView = (BYTE *)g_unk0x00547ac8 + index * 0x178;
    pPos = (FixVector *)(pView + 8);
    count = *(int *)(pView + 0x58) >> 16;
    saved = *pPos;
    if (*(int *)pView == 1) {
        fade = *(int *)(pView + 0x64);
        if (fade == 0x10000) {
            g_unk0x00543ed0.a = 0xaa;
            g_unk0x00547908.a = 0xaa;
        } else if (fade == 0) {
            alpha = 0;
            g_unk0x00543ed0.a = 0;
            g_unk0x00547908.a = 0;
        } else {
            alpha = (BYTE)(FixMul(fade, 0x1e0000) >> 16);
            g_unk0x00543ed0.a = (BYTE)(FixMul(*(int *)(pView + 0x64), 0xaa0000) >> 16);
            g_unk0x00547908.a = (BYTE)(FixMul(*(int *)(pView + 0x64), 0xaa0000) >> 16);
        }
        *(DWORD *)(g_unk0x00543da0Block + 0x50) = *(DWORD *)(g_unk0x00543da0Block + 0x80) =
            (alpha << 24) | 0x969696;
        FixMatrix_GetForward(&forward, &g_viewNodes[view]->current);
        FixVecCross(&side, &forward, (FixVector *)(pView + 0x20));
        FIX_NORMALIZE_INTO(side, side);
        amount = FixMul(*(int *)(pView + 0x58), g_unk0x00543da0);
        if (amount > 0x10000)
            amount = 0x10000;
        FixVecScale(&side, &side, FixMul(0xccc, FixMul(0xcccd, amount) + 0x13333));
        sideF[0] = (float)(side.x * CGraphics::m_oneOver65536);
        sideF[1] = (float)(side.y * CGraphics::m_oneOver65536);
        sideF[2] = (float)(side.z * CGraphics::m_oneOver65536);
        FixVecScale(&trail, (FixVector *)(pView + 0x20), 0x20000);
        trailF[0] = (float)(trail.x * CGraphics::m_oneOver65536);
        trailF[1] = (float)(trail.y * CGraphics::m_oneOver65536);
        trailF[2] = (float)(trail.z * CGraphics::m_oneOver65536);
        for (i = 0; i < count; i++) {
            pNode = &g_unk0x00543fb0[*(short *)(pView + 0x76) + i];
            WEATHER_QUAD(0x68) = (float)(pNode->x * CGraphics::m_oneOver65536);
            WEATHER_QUAD(0x6c) = (float)(pNode->y * CGraphics::m_oneOver65536);
            WEATHER_QUAD(0x70) = (float)(pNode->z * CGraphics::m_oneOver65536);
            WEATHER_QUAD(0x08) = WEATHER_QUAD(0x68) - trailF[0];
            WEATHER_QUAD(0x0c) = WEATHER_QUAD(0x6c) - trailF[1];
            WEATHER_QUAD(0x10) = WEATHER_QUAD(0x70) - trailF[2];
            WEATHER_QUAD(0x38) = WEATHER_QUAD(0x68) + sideF[0];
            WEATHER_QUAD(0x3c) = WEATHER_QUAD(0x6c) + sideF[1];
            WEATHER_QUAD(0x40) = WEATHER_QUAD(0x70) + sideF[2];
            WEATHER_QUAD(0x68) -= sideF[0];
            WEATHER_QUAD(0x6c) -= sideF[1];
            WEATHER_QUAD(0x70) -= sideF[2];
            Quad2D_Queue((Quad2DVertices *)(g_unk0x00543da0Block + 8), (Texture *)g_unk0x00543e90, (Quad2D *)0x16);
            if (pNode->wrapped != 0) {
                forward = *(FixVector *)pNode;
                forward.y -= 0xc0000;
                FixVecScale(&offset, (FixVector *)(pView + 0x44),
                            FixMul(pNode->angle - forward.y, *(int *)(pView + 0x50)));
                g_unk0x00543ed0.pos.x = offset.x + forward.x;
                g_unk0x00543ed0.pos.y = offset.y + forward.y;
                g_unk0x00543ed0.pos.z = offset.z + forward.z;
                Billboard_Add(&g_unk0x00543ed0, (unsigned short *)g_unk0x00543ea4);
            }
        }
        if (CGameInfo::FUN_00404f20() == 0) {
            amount = FixMul(*(int *)(pView + 0x58), g_unk0x00543da0);
            if (amount > 0x10000)
                amount = 0x10000;
            FUN_00460ca0(amount, Car_Get(FUN_00422fb0((BYTE)view)));
        }
    } else {
        Scene_GetLightColour((DWORD *)&g_unk0x00543f00.r, 0xcccc);
        for (i = 0; i < count; i++) {
            pNode = &g_unk0x00543fb0[*(short *)(pView + 0x76) + i];
            g_unk0x00543f00.pos = *(FixVector *)pNode;
            FixVecScale(&offset, (FixVector *)(pView + 0x38),
                        FixMul(g_sinTable[(unsigned short)(__int64)(pNode->angle * g_unk0x00511300) & 0xfff],
                               pNode->field_0x10));
            g_unk0x00543f00.pos.x += offset.x;
            g_unk0x00543f00.pos.y += offset.y;
            g_unk0x00543f00.pos.z += offset.z;
            Billboard_Add(&g_unk0x00543f00, (unsigned short *)g_unk0x00543ea8[((BYTE *)&pNode->field_0x1c)[1]]);
        }
    }
    *pPos = saved;
}
#undef WEATHER_QUAD


// Per-view stage objects draw: the view's precipitation, then the object
// pass when racing, replaying or in the demo.
// FUNCTION: CMR2 0x00460330
void FUN_00460330(int param_1, int view)
{
    int *pType;

    pType = (int *)((BYTE *)g_unk0x00547ac8 + view * 0x178);
    if (*pType == 1 || *pType == 2)
        FUN_00460390(view, view);
    if ((char)RallyDataState() != 1 && FUN_0041f3a0() == 0 && CGameInfo::FUN_00405da0() == 0)
        return;
    FUN_00462d80(param_1, view);
}

// Sparks and glints thrown off a car's body: `amount` (per second) of random
// points on its damage parts get a billboard of the spark texture.
// FUNCTION: CMR2 0x00460ca0
int FUN_00460ca0(int amount, Car *pCar)
{
    int *pParts;
    int count;
    int result;
    int n;
    int part;
    FixVector point;
    FixVector rotated;
    FixVector origin;

    pParts = FUN_00469680(pCar->field_0xb1a);
    count = FixMul(0x1e0000, FixMul(g_unk0x0051bd3c, amount)) >> 16;
    result = count;
    if (count > 0) {
        result = rand();
        n = result % count;
        result /= count;
        for (; n > 0; n--) {
            part = rand() % pParts[0x117];
            result = FUN_00469c30(&point, pParts, part);
            if (result != 0) {
                result = point.y;
                if (point.y > 0) {
                    FixMatrix_RotateVector(&rotated, &point, (FixMatrix *)(pParts[0xf + part] + 0xd8));
                    FixMatrix_GetPosition(&origin, (FixMatrix *)(pParts[0xf + part] + 0xd8));
                    rotated.x += origin.x;
                    rotated.y += origin.y;
                    rotated.z += origin.z;
                    g_unk0x00547908.pos = rotated;
                    Billboard_Add(&g_unk0x00547908, (unsigned short *)g_unk0x005477f0);
                }
            }
        }
    }
    return result;
}

void FUN_0046c4e0(int param_1, BYTE index);
int Replay_StopRecording(BYTE *pBuffer);

// Records the car states of the replay streams of type 2 (every third frame):
// one 16-byte sample per call into the current section, until it is full.
// FUNCTION: CMR2 0x0046d510
void FUN_0046d510(void)
{
    void ***pp;
    BYTE *p;
    short *pCount;
    short n;

    for (pp = g_unk0x00588d40; (int)pp < (int)(g_unk0x00588d40 + 16); pp++) {
        if (*pp == NULL)
            continue;
        p = (BYTE *)**pp;
        if (p == NULL || *(int *)(p + 0xc) == 0 || *(int *)(p + 0x1c) != 2)
            continue;
        if (*((BYTE *)Car_Get(p[0x20]) + 0xb43) <= 0u)
            continue;
        if (p[0xf8] == 0) {
            pCount = (short *)(*(BYTE **)(p + 0x104) + *(short *)(p + 0x100) * 2);
            n = *pCount;
            if (n < *(short *)(p + 0xfe)) {
                FUN_0046c4e0((*(short *)(p + 0xfe) * *(short *)(p + 0x100) + n) * 0x10 + *(int *)(p + 0x40), p[0x20]);
                (*pCount)++;
            } else {
                Replay_StopRecording(p);
            }
        }
        p[0xf8]++;
        if (p[0xf8] >= 3)
            p[0xf8] = 0;
    }
}

extern FixVector g_unk0x005914a8;
extern FixVector g_unk0x005915e8;
extern int g_unk0x005915dc;
extern int g_unk0x00591468;
extern int g_unk0x005914d8;
int FUN_00488640(int *pBoxA, int *pBoxB, FixVector *pOffset, int scale);
int FixMatrix_InverseRotateVector(FixVector *pOut, FixVector *pV, FixMatrix *pM);

// Resolves a contact between a car's box and a turned stage object's box
// (0x5915f8): the push-out direction is the sum of the contact edge normals,
// the contact point the mean of the contact corners (in car space). Unless
// the object is pushable, a one-sided contact also moves the object. Returns
// whether the boxes overlapped.
// FUNCTION: CMR2 0x00487f60
int FUN_00487f60(Car *pCar, int *pEntry, int *pBox, int *pObject)
{
    int count;
    int solid;
    int hit;
    int i;
    int d;
    int k;
    FixVector sum;
    FixVector point;
    FixVector dir;
    FixVector *pAxis;
    BYTE *p = (BYTE *)pCar;

    count = 0;
    if ((*(unsigned int *)(*pEntry + 0x10) & 0x2001000) == 0) {
        solid = 0;
        hit = FUN_00488640(pBox, pObject, (FixVector *)(p + 0x2e8), 0);
    } else {
        solid = 1;
        hit = FUN_00488640(pBox, pObject, (FixVector *)(p + 0x2e8), 0x10000);
    }
    if (hit == 0)
        return hit;
    sum.x = 0;
    sum.y = 0;
    sum.z = 0;
    dir.x = 0;
    dir.y = 0;
    dir.z = 0;
    if (g_unk0x005915f4 > 0) {
        for (i = 0; i < g_unk0x005915f4; i++) {
            point.x = *(int *)(p + 0x270 + g_unk0x005914c4[i] * 0xc) - *(int *)(p + 0x2d0);
            point.y = *(int *)(p + 0x274 + g_unk0x005914c4[i] * 0xc) - *(int *)(p + 0x2d4);
            point.z = *(int *)(p + 0x278 + g_unk0x005914c4[i] * 0xc) - *(int *)(p + 0x2d8);
            sum.x += point.x;
            point.y = 0;
            sum.z += point.z;
            if (g_unk0x00590ec8[i] == 0)
                pAxis = (FixVector *)(pObject + 4);
            else
                pAxis = (FixVector *)(pObject + 7);
            if (FixVecDot(&dir, pAxis) < 0) {
                dir.x -= pAxis->x;
                dir.y -= pAxis->y;
                dir.z -= pAxis->z;
            } else {
                dir.x += pAxis->x;
                dir.y += pAxis->y;
                dir.z += pAxis->z;
            }
        }
        count = g_unk0x005915f4;
    }
    if (g_unk0x005914d4 > 0) {
        for (i = 0; i < g_unk0x005914d4; i++) {
            point = *(FixVector *)(pObject + (g_unk0x00590ecc[i] + 4) * 3);
            point.x -= *(int *)(p + 0x2d0);
            point.z -= *(int *)(p + 0x2d8);
            sum.x += point.x;
            sum.z += point.z;
            point.y = 0;
            if (g_unk0x005914a4[i] == 0)
                pAxis = (FixVector *)(pBox + 4);
            else
                pAxis = (FixVector *)(pBox + 7);
            if (FixVecDot(&dir, pAxis) < 0) {
                dir.y -= pAxis->y;
                dir.x -= pAxis->x;
                dir.z -= pAxis->z;
            } else {
                dir.y += pAxis->y;
                dir.x += pAxis->x;
                dir.z += pAxis->z;
            }
        }
        count += g_unk0x005914d4;
    }
    FixVecScaleRecip(&sum, &sum, count << 16);
    FIX_NORMALIZE_INTO(dir, dir);
    if (solid == 0 && (g_unk0x005915f4 == 0 || g_unk0x005914d4 == 0)) {
        d = FixVecDot(&dir, &g_unk0x005914a8);
        FixVecScale(&point, &dir, d);
        point.x -= g_unk0x005914a8.x;
        point.y -= g_unk0x005914a8.y;
        point.z -= g_unk0x005914a8.z;
        if (*(int **)(pBox + 0x25) != NULL && pBox[0x24] != 0) {
            (*(int **)(pBox + 0x25))[0] += point.x;
            (*(int **)(pBox + 0x25))[1] += point.y;
            (*(int **)(pBox + 0x25))[2] += point.z;
            for (k = 0; k < 0x60; k += 0xc) {
                *(int *)(pBox[0x24] + k) += point.x;
                *(int *)(pBox[0x24] + k + 4) += point.y;
                *(int *)(pBox[0x24] + k + 8) += point.z;
            }
            for (k = 0; k < 4; k++) {
                pBox[0xc + k * 3] += point.x;
                pBox[0xd + k * 3] += point.y;
                pBox[0xe + k * 3] += point.z;
            }
        }
        FUN_0048c870(pCar->field_0xb1a, 0xff, (int *)&point, 1);
    }
    g_unk0x005915e8 = dir;
    FixMatrix_InverseRotateVector((FixVector *)(p + 0x5dc), &sum, *(FixMatrix **)(p + 0x750));
    g_unk0x005915dc = 0x8000;
    g_unk0x00591468 = 0x1570a;
    return hit;
}

BYTE *Sector_GetListA(unsigned int sector, unsigned int *pCount);
BYTE *Sector_GetListB(unsigned int sector, unsigned int *pCount);
void RallyData_FUN_00471cc0(int *pDest, void **pParam1);
int FUN_00471d40(BYTE **pEntry, int bit);
int FUN_00489750(int car, int *pBox, int scale);
int FUN_0048be20(int param_1, int *param_2, int param_3, int param_4);

// Collides a car with the stage objects of the four sectors it touches
// (static objects, then moving ones): box test, then the plain, wall or
// turned-object response, and the object's reaction. Keeps the list of
// touched surfaces. Returns the last response.
// FUNCTION: CMR2 0x004878a0
int FUN_004878a0(Car *pCar)
{
    BYTE *p = (BYTE *)pCar;
    BYTE *pBox;
    short sectors[4];
    int moving;
    int n;
    int k;
    int i;
    int count;
    int sector;
    int result;
    int *pEntry;
    int *pObject;
    FixVector position;

    result = 0;
    p[0xb3f] = 0;
    pBox = g_unk0x00590ed0[pCar->field_0xb1a];
    memset(p + 0xbbc, 0, 0x20);
    *(int *)&sectors[1] = *(int *)(p + 0xb02);
    sectors[0] = *(short *)(p + 0xb00);
    sectors[3] = *(short *)(p + 0xb06);
    for (moving = *(int *)(p + 0xc18) != 0; moving < 2; moving++) {
        for (k = 0; k < 4; k++) {
            if (sectors[k] < 0)
                continue;
            sector = sectors[k];
            if (moving == 0)
                pEntry = (int *)Sector_GetListA(sector, (unsigned int *)&count);
            else
                pEntry = (int *)Sector_GetListB(sector, (unsigned int *)&count);
            pEntry = pEntry != NULL ? *(int **)pEntry : NULL;
            for (i = 0; i < count; i++, pEntry += 2) {
                pObject = (int *)pEntry[0];
                if (moving == 0)
                    position = *(FixVector *)pObject;
                else
                    RallyData_FUN_00471cc0((int *)&position, (void **)&pEntry);
                if ((pObject[4] & 0x8000) == 0 && (BYTE)RallyDataState() == 2)
                    continue;
                g_unk0x005914d8 = *(int *)(pEntry[1] + 4) != 0;
                result = FUN_00487b80(*(int *)(p + 0x758), *(int *)pEntry[1], (int *)(p + 0x2d0), (int *)&position);
                if (result == 0)
                    continue;
                result = 0;
                if (moving == 1 && FUN_00471d40((BYTE **)&pEntry, pCar->field_0xb1a) == 0)
                    continue;
                FUN_00486c30((int *)pBox, (int *)(p + 0x360), (int *)(p + 0x2d0), (FixVector *)(p + 0x270));
                result = 0;
                if (g_unk0x005914d8 != 0) {
                    g_unk0x005915f8[10] = 0;
                    FUN_00487c40(g_unk0x005915f8, (int)pEntry, (int *)&position);
                } else {
                    FUN_00487e00(&position, pEntry);
                }
                if (g_unk0x005914d8 != 0) {
                    result = FUN_00487f60(pCar, pEntry, (int *)pBox, g_unk0x005915f8);
                } else {
                    if (pObject[4] & 0x4000) {
                        if (*(int *)(p + 0xc20) == 0)
                            FUN_00487e50((int *)pBox, pCar);
                        continue;
                    }
                    result = FUN_00489750((int)pCar, (int *)pBox, (pObject[4] & 0x2001000) != 0 ? 0 : 0x10000);
                }
                if (result != 0 && FUN_0048be20((int)pCar, pEntry, (int)&position, 0) != 0)
                    FUN_0046fe70(pEntry, sector, pCar->field_0xb1a);
            }
        }
    }
    p[0xb3e] = p[0xb3f];
    for (i = 0; i < (char)p[0xb3e]; i++)
        ((short *)(p + 0xad6))[i] = ((short *)(p + 0xad6))[i + 5];
    return result;
}

// Collision scratch of the edge being tested (second end point offsets).
// GLOBAL: CMR2 0x005918d4
int g_unk0x005918d4;
// GLOBAL: CMR2 0x00591948
int g_unk0x00591948;
// GLOBAL: CMR2 0x00591960
int g_unk0x00591960;

extern Car *g_collisionCar;
extern FixVector g_collisionTarget;
extern FixVector g_collisionLineStart;
extern FixVector g_collisionDirection;
extern int g_collisionDirectionDirty;
extern int g_unk0x005918d0;
extern int g_unk0x0059195c;
extern int g_unk0x005919b8;
extern struct CollisionFaceVertices *g_collisionFace;
extern int g_unk0x0051fb00[27];
int FUN_0048e730(int *param_1, int *param_2, int param_3, char param_4);
int FUN_0048f400(void);
int FUN_0048fb80(char type, int param);
int FUN_00490b90(int param_1);
void FUN_004a3240(int unused);
void FUN_0048df50(Car *param_1);
void FUN_00466ef0(Car *pCar, int *param_2, FixVector *param_3, int param_4, unsigned char param_5, int param_6);

// Tests the end points of the current sector edge against the car's radius
// and runs the corner collision for each one inside it.
// FUNCTION: CMR2 0x0048e580
int FUN_0048e580(char type)
{
    int radius;
    int radius2;
    int hitA;
    int hitB;
    FixVector d;

    radius = *(int *)((BYTE *)g_collisionCar + 0x758);
    hitB = 0;
    hitA = 0;
    radius2 = FixMul(radius, radius);
    d.x = g_collisionTarget.x - *(int *)((BYTE *)g_collisionCar + 0x2d0);
    d.y = g_collisionTarget.y - *(int *)((BYTE *)g_collisionCar + 0x2d4);
    d.y = 0;
    d.z = g_collisionTarget.z - *(int *)((BYTE *)g_collisionCar + 0x2d8);
    if ((d.x < 0 ? -d.x : d.x) <= radius && radius >= 0 && (d.z < 0 ? -d.z : d.z) <= radius &&
        FixVecDot(&d, &d) < radius2)
        hitA = FUN_0048e730((int *)&g_collisionTarget, (int *)&g_collisionLineStart, 0, type);
    d.x = g_collisionLineStart.x - *(int *)((BYTE *)g_collisionCar + 0x2d0);
    d.y = g_collisionLineStart.y - *(int *)((BYTE *)g_collisionCar + 0x2d4);
    d.y = 0;
    d.z = g_collisionLineStart.z - *(int *)((BYTE *)g_collisionCar + 0x2d8);
    if ((d.x < 0 ? -d.x : d.x) <= radius && radius >= 0 && (d.z < 0 ? -d.z : d.z) <= radius &&
        FixVecDot(&d, &d) < radius2)
        hitB = FUN_0048e730((int *)&g_collisionLineStart, (int *)&g_collisionTarget, 1, type);
    if (hitA == 0 && hitB == 0)
        return 0;
    return 1;
}

// Collides a car with the edges of its sector: each edge near enough gets the
// response of its surface type (walls, water, sound triggers, finish, drop
// zones), then the scraping effects; ends a drop-out timer.
// FUNCTION: CMR2 0x0048e0a0
void FUN_0048e0a0(Car *pCar, int param)
{
    BYTE *p;
    int nearX;
    int nearZ;
    int a;
    int b;
    FixVector saved;

    g_collisionCar = pCar;
    g_collisionFace = (CollisionFaceVertices *)FUN_0048ca40(pCar->field_0xb1a);
    ((char *)g_collisionCar)[0xb42]--;
    if (((char *)g_collisionCar)[0xb42] < 0)
        ((char *)g_collisionCar)[0xb42] = 0;
    g_unk0x00591948 = 0;
    if (*(short *)((BYTE *)g_collisionCar + 0xb00) == -1)
        return;
    g_unk0x0059190c = *(int **)((BYTE *)g_sectors[*(short *)((BYTE *)g_collisionCar + 0xb00)] + 0x28);
    g_unk0x005919b8 = FixMul(*(int *)((BYTE *)g_collisionCar + 0x758), 0x13333);
    while (g_unk0x0059190c != NULL) {
        nearZ = 0;
        g_unk0x00591930 = 0;
        g_collisionTarget = *(FixVector *)g_unk0x0059190c;
        g_collisionLineStart = *(FixVector *)(g_unk0x0059190c + 3);
        g_unk0x005918d0 = g_collisionTarget.x - *(int *)((BYTE *)g_collisionCar + 0x2d0);
        g_unk0x0059195c = g_collisionTarget.z - *(int *)((BYTE *)g_collisionCar + 0x2d8);
        g_unk0x005918d4 = g_collisionLineStart.x - *(int *)((BYTE *)g_collisionCar + 0x2d0);
        g_unk0x00591960 = g_collisionLineStart.z - *(int *)((BYTE *)g_collisionCar + 0x2d8);
        nearX = 0;
        if (g_unk0x005918d0 < 0 ? g_unk0x005918d4 < 0 : g_unk0x005918d4 >= 0)
            nearX = 1;
        if (g_unk0x0059195c < 0 ? g_unk0x00591960 < 0 : g_unk0x00591960 >= 0)
            nearZ = 1;
        if (nearX) {
            a = g_unk0x005918d0 < 0 ? -g_unk0x005918d0 : g_unk0x005918d0;
            b = g_unk0x005918d4 < 0 ? -g_unk0x005918d4 : g_unk0x005918d4;
            if (a >= g_unk0x005919b8 && b >= g_unk0x005919b8)
                goto next;
        }
        if (nearZ) {
            a = g_unk0x0059195c < 0 ? -g_unk0x0059195c : g_unk0x0059195c;
            b = g_unk0x00591960 < 0 ? -g_unk0x00591960 : g_unk0x00591960;
            if (a >= g_unk0x005919b8 && b >= g_unk0x005919b8)
                goto next;
        }
        g_collisionDirection = *(FixVector *)(g_unk0x0059190c + 6);
        g_collisionDirectionDirty = 0;
        switch (*(char *)((BYTE *)g_unk0x0059190c + 0x2c)) {
        case 6:
            if (FUN_0048f400() == 0 && FUN_00490b90(1) != 0)
                *(int *)((BYTE *)g_collisionCar + 0xbf8) = 1;
            break;
        case 24:
            if (FUN_0048f400() == 0 && FUN_00490b90(1) != 0 && *(int *)((BYTE *)g_collisionCar + 0xa7c) == 0 &&
                *(int *)((BYTE *)g_collisionCar + 0xbf8) == 0)
                *(int *)((BYTE *)g_collisionCar + 0xa7c) = 0x190000;
            break;
        case 7:
            FUN_004a3240(0);
            break;
        case 8:
            FUN_004a3240(1);
            break;
        case 9:
            FUN_004a3240(2);
            break;
        case 10:
            FUN_004a3240(3);
            break;
        case 11:
            FUN_004a3240(4);
            break;
        case 23:
            if (*(int *)((BYTE *)g_collisionCar + 0xc20) != 0)
                goto next;
            if (FUN_0048f400() == 0 && FUN_00490b90(1) != 0)
                FUN_0048df50(g_collisionCar);
            break;
        case 0:
        case 1:
            goto next;
        default:
            if (FUN_0048f400() == 0) {
                if ((*((BYTE *)g_unk0x0059190c + 0x2d) & 1) && FUN_0048e580(*(char *)((BYTE *)g_unk0x0059190c + 0x2c)) &&
                    *(char *)((BYTE *)g_unk0x0059190c + 0x2c) == 0x19)
                    *(int *)((BYTE *)g_collisionCar + 0xbf8) = 1;
                if (FUN_0048fb80(*(char *)((BYTE *)g_unk0x0059190c + 0x2c), param) &&
                    *(char *)((BYTE *)g_unk0x0059190c + 0x2c) == 0x19)
                    *(int *)((BYTE *)g_collisionCar + 0xbf8) = 1;
            }
            break;
        }
        if (g_unk0x00591930 != 0) {
            p = (BYTE *)g_collisionCar + 0x5c4;
            saved = *(FixVector *)p;
            *(FixVector *)p = g_unk0x005918e0;
            FixVecScaleRecip((FixVector *)((BYTE *)g_collisionCar + 0x5c4), (FixVector *)((BYTE *)g_collisionCar + 0x5c4),
                             0x10000 - g_unk0x0051fb00[*(char *)((BYTE *)g_unk0x0059190c + 0x2c)]);
            if (((char *)g_collisionCar)[0xb42] <= 0)
                FUN_00466ef0(g_collisionCar, (int *)&g_unk0x00591ad0, &g_unk0x00591938, 0, g_unk0x0059199c, 0);
            *(FixVector *)((BYTE *)g_collisionCar + 0x5c4) = saved;
            ((char *)g_collisionCar)[0xb42] = 10;
        }
    next:
        g_unk0x0059190c = (int *)g_unk0x0059190c[10];
    }
    if (*(int *)((BYTE *)g_collisionCar + 0xa7c) > 0) {
        *(int *)((BYTE *)g_collisionCar + 0xa7c) -= 0x10000;
        if (*(int *)((BYTE *)g_collisionCar + 0xa7c) < 0)
            *(int *)((BYTE *)g_collisionCar + 0xa7c) = 0;
        if (*(int *)((BYTE *)g_collisionCar + 0xa7c) == 0)
            *(int *)((BYTE *)g_collisionCar + 0xbf8) = 1;
    }
}

int StageObject_UsesExtendedMode(void);
void FUN_0048a1f0(int param_1, short *param_2, short param_3);
int FUN_0047c5b0(int index);

// Collisions of the cars in `pOrder` for the frame: car against car (when the
// mode allows it), car against stage objects, car against the sector edges;
// then each car's contact box state is kept for the next frame.
// FUNCTION: CMR2 0x004877a0
void FUN_004877a0(BYTE *pCars, short *pOrder, short count)
{
    int n;
    int i;
    short *pIndex;
    BYTE *pCar;
    BYTE *pBox;
    int car;

    n = count;
    i = n - 1;
    if (i >= 0) {
        pIndex = pOrder + i;
        do {
            *(int *)&g_unk0x00590ed0[*pIndex][0x28] = 0;
            pIndex--;
        } while (--n);
    }
    if (StageObject_UsesExtendedMode())
        FUN_0048a1f0((int)pCars, pOrder, count);
    if (i >= 0) {
        pIndex = pOrder + i;
        n = i + 1;
        do {
            car = *pIndex;
            pCar = pCars + car * 0xc24;
            pBox = g_unk0x00590ed0[car];
            if (*(int *)(pCar + 0xb64) == 0)
                FUN_004878a0((Car *)pCar);
            if (*(int *)(pCar + 0xc18) == 0 &&
                (*(int *)(pCar + 0xb64) == 0 || FUN_0047c5b0(((Car *)pCar)->field_0xb1a) < 0x20000))
                FUN_0048e0a0((Car *)pCar, car);
            *(int *)(pBox + 0x2c) = *(int *)(pBox + 0x28);
            if (*(int *)(pBox + 0x28) != 0)
                memcpy(pBox + 0x60, pBox + 0x30, 0x30);
            pIndex--;
        } while (--n);
    }
}

// Where the fireworks go off, relative to the stage origin.
// GLOBAL: CMR2 0x0051f4e0
FixVector g_fireworksOrigin = { -0xcc0000, 0, 0x3d0000 };
// GLOBAL: CMR2 0x0051f51c
char g_strFireworksLaunch3[] = "\\Fireworks\\Launch_3.wav";
// GLOBAL: CMR2 0x0051f534
char g_strFireworksLaunch2[] = "\\Fireworks\\Launch_2.wav";
// GLOBAL: CMR2 0x0051f54c
char g_strFireworksLaunch1[] = "\\Fireworks\\Launch_1.wav";
// GLOBAL: CMR2 0x0051f564
char g_strFireworksBang3[] = "\\Fireworks\\Bang_3.wav";
// GLOBAL: CMR2 0x0051f57c
char g_strFireworksBang2[] = "\\Fireworks\\Bang_2.wav";
// GLOBAL: CMR2 0x0051f594
char g_strFireworksBang1[] = "\\Fireworks\\Bang_1.wav";
// GLOBAL: CMR2 0x0051f5ac
char g_strFireworksLoudBang3[] = "\\Fireworks\\Loud_Bang_3.wav";
// GLOBAL: CMR2 0x0051f5c8
char g_strFireworksLoudBang2[] = "\\Fireworks\\Loud_Bang_2.wav";
// GLOBAL: CMR2 0x0051f5e4
char g_strFireworksLoudBang1[] = "\\Fireworks\\Loud_Bang_1.wav";
// GLOBAL: CMR2 0x0051f600
char g_strFireworksFalling2[] = "\\Fireworks\\Falling_2.wav";
// GLOBAL: CMR2 0x0051f61c
char g_strFireworksFalling1[] = "\\Fireworks\\Falling_1.wav";

extern char g_strPathConcat[];
extern double g_unk0x00511308;
void FUN_004805f0(int value);
int FUN_004b7940(void);
BOOL Sound_LoadSample(char *name, BYTE flags, GenericFile *pFile);

#define LOAD_FIREWORK_SOUND(str)                                                  \
    sprintf(CFrontend::m_stringDest, g_strPathConcat, pDir, str);                 \
    Sound_LoadSample(CFrontend::m_stringDest, 0, (GenericFile *)StageTiming_GetStageFile4())

// Sets up the fireworks of the end-of-rally show: `count` rockets (0x938-byte
// records), the two spark billboards, the 3x3 table of burst directions
// (every 30 degrees), the four launch points and the sounds.
// FUNCTION: CMR2 0x0047e4d0
void FUN_0047e4d0(BYTE count)
{
    int i;
    int k;
    int step;
    int a;
    int b;
    int start;
    int *pDirs;
    FixVector *p;
    char *pDir;
    int sinA;
    int cosA;

    g_unk0x00590afc = count;
    if (count != 0) {
        g_unk0x00590af8 = CFileBuffer::AllocateLockedBuffer(count * 0x938);
        g_unk0x00590b04 = CFileBuffer::AllocateLockedBuffer(0x28);
        g_unk0x00590b08 = CFileBuffer::AllocateLockedBuffer(0x28);
        g_unk0x00590b0c = (void **)CFileBuffer::AllocateLockedBuffer(0xc);
        for (i = 0; i < 3; i++)
            g_unk0x00590b0c[i] = CFileBuffer::AllocateLockedBuffer(0x24);
        for (i = 0; i < g_unk0x00590afc; i++) {
            *(int *)((BYTE *)g_unk0x00590af8 + i * 0x938 + 0x694) = 0;
            *((BYTE *)g_unk0x00590af8 + i * 0x938 + 0x690) = 0;
        }
        ((int *)g_unk0x00590b04)[3] = -0x8000;
        ((int *)g_unk0x00590b04)[4] = 0x8000;
        ((int *)g_unk0x00590b04)[5] = 0x8000;
        ((int *)g_unk0x00590b04)[6] = -0x8000;
        ((short *)g_unk0x00590b04)[0x10] = 0;
        ((BYTE *)g_unk0x00590b04)[0x23] &= 0xfe;
        ((BYTE *)g_unk0x00590b04)[0x23] &= 0xfd;
        memcpy(g_unk0x00590b08, g_unk0x00590b04, 0x28);
        ((int *)g_unk0x00590b08)[3] = -0x6000;
        ((int *)g_unk0x00590b08)[4] = 0x6000;
        ((int *)g_unk0x00590b08)[5] = 0x6000;
        ((int *)g_unk0x00590b08)[6] = -0x6000;
        step = FixDiv(0x5a0000, 0x30000);
        k = FixDiv(0x5a0000, 0x30000);
        start = FixMul(k, 0x8000);
        a = FixMul(step, 0x8000);
        for (i = 0; i < 3; i++) {
            sinA = g_sinTable[(unsigned short)(int)(__int64)((double)a * g_unk0x00511300) & 0xfff];
            cosA = g_sinTable[(0x400 - (unsigned short)(int)(__int64)((double)a * g_unk0x00511308)) & 0xfff];
            pDirs = (int *)g_unk0x00590b0c[i];
            for (b = start, p = (FixVector *)pDirs; (int *)p < pDirs + 9; p++, b += k) {
                p->x = FixMul(sinA, g_sinTable[(unsigned short)(int)(__int64)((float)b * g_unk0x00511300) & 0xfff]);
                p->y = g_sinTable[(0x400 - (unsigned short)(int)(__int64)((float)b * g_unk0x00511308)) & 0xfff];
                p->z = FixMul(cosA, g_sinTable[(unsigned short)(int)(__int64)((float)b * g_unk0x00511300) & 0xfff]);
            }
            a += step;
            start = FixMul(k, 0x8000);
        }
    }
    g_unk0x00590b00 = (int)g_carLights[0].pReverse;
    g_unk0x005909c8[0].x = 0x5b50000;
    g_unk0x005909c8[0].y = 0x20000;
    g_unk0x005909c8[1].x = 0x5b50000;
    g_unk0x005909c8[1].y = 0x20000;
    g_unk0x005909c8[2].x = 0x5b50000;
    g_unk0x005909c8[2].y = 0x20000;
    g_unk0x005909c8[3].x = 0x5b50000;
    g_unk0x005909c8[3].y = 0x20000;
    g_unk0x005909c8[0].z = 0xc20000;
    g_unk0x005909c8[1].z = 0xce0000;
    g_unk0x005909c8[2].z = 0xda0000;
    g_unk0x005909c8[3].z = 0xe60000;
    for (p = g_unk0x005909c8; p < g_unk0x005909c8 + 4; p++) {
        p->x += g_fireworksOrigin.x;
        p->y += g_fireworksOrigin.y;
        p->z += g_fireworksOrigin.z;
    }
    pDir = CInstallInfo::GetSoundsDir();
    FUN_004805f0(FUN_004b7940());
    LOAD_FIREWORK_SOUND(g_strFireworksFalling1);
    LOAD_FIREWORK_SOUND(g_strFireworksFalling2);
    LOAD_FIREWORK_SOUND(g_strFireworksLoudBang1);
    LOAD_FIREWORK_SOUND(g_strFireworksLoudBang2);
    LOAD_FIREWORK_SOUND(g_strFireworksLoudBang3);
    LOAD_FIREWORK_SOUND(g_strFireworksBang1);
    LOAD_FIREWORK_SOUND(g_strFireworksBang2);
    LOAD_FIREWORK_SOUND(g_strFireworksBang3);
    LOAD_FIREWORK_SOUND(g_strFireworksLaunch1);
    LOAD_FIREWORK_SOUND(g_strFireworksLaunch2);
    LOAD_FIREWORK_SOUND(g_strFireworksLaunch3);
    CGame::RegisterCallback(FUN_0047ea20, NULL);
}
#undef LOAD_FIREWORK_SOUND

short RallyData_FUN_004213a0(BYTE *p);
int RallyData_FUN_00421430(void);

// Simple CPU driving used near the end of the route (the demo/attract drive):
// keeps the speed between two limits, steers back towards the route and
// brakes on a sharp heading error. Returns 0 when the route is about to end.
// `preview` != 0 only computes.
// FUNCTION: CMR2 0x0047d330
int FUN_0047d330(int car, int preview)
{
    Car *pCar;
    int state[0x2e];
    unsigned int node;
    int left;
    int right;
    int throttle;
    int brake;
    int handbrake;
    int mode;

    pCar = Car_Get(car);
    node = (unsigned short)RallyData_FUN_004213a0((BYTE *)pCar);
    if (node + 5 > (unsigned int)RallyData_FUN_00421430())
        return 0;
    state[0x15] = node;
    FUN_00498620(pCar, 0x342, state, 0);
    handbrake = 0;
    left = 0;
    right = 0;
    throttle = 0;
    brake = 0;
    if (state[0] < 0x320000)
        throttle = 0x3f;
    if (state[0] > 0x460000)
        brake = 0x3f;
    if (state[7] > 0x50000)
        right = 0x3f;
    if (state[7] < -0x50000)
        left = 0x3f;
    mode = 0;
    if (state[0xd] < 0) {
        if (state[0xe] > 0x5a0000)
            mode = 4;
    } else if (state[0xe] > 0x5a0000) {
        mode = 4;
    }
    if (state[0xe] < -0x5a0000)
        mode = 2;
    else if (mode == 0)
        goto apply;
    switch (mode) {
    case 1:
        throttle = 0x3f;
        right = throttle;
        break;
    case 2:
        brake = 0x3f;
        right = brake;
        break;
    case 3:
        left = 0x3f;
        throttle = left;
        break;
    case 4:
        left = 0x3f;
        brake = left;
        break;
    case 5:
        left = 0x3f;
        throttle = 0;
        brake = left;
        break;
    case 6:
        brake = 0x3f;
        throttle = 0;
        right = brake;
        break;
    case 7:
        left = 0x3f;
        break;
    case 8:
        right = 0x3f;
        break;
    }
    if (state[0] >= 0x190000)
        throttle = 0;
apply:
    if (preview != 0)
        return 1;
    pCar->flag0x1d0[0] = 0;
    pCar->flag0x1d0[1] = 0;
    pCar->flag0x1d0[2] = 0;
    pCar->flag0x1d0[3] = 0;
    pCar->field_0x1d8 = 0;
    if (left > 0)
        pCar->flag0x1d0[0] = 0x3f;
    if (right > 0)
        pCar->flag0x1d0[1] = 0x3f;
    if (throttle > 0)
        pCar->flag0x1d0[2] = 0x3f;
    if (brake > 0)
        pCar->flag0x1d0[3] = 0x3f;
    if (handbrake > 0)
        pCar->field_0x1d8 = 1;
    return 1;
}

void Particle_Spawn(int typeIndex, FixVector *pSource, FixVector *pPosition, int field0x48, int field0x40,
                    BYTE *pColour, BYTE field0x54, int callbackParam, BYTE field0x55);
#define RAND_FIX() ((int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536))

// Per-frame update of the flying debris (100 records of 0x5c bytes): ages
// each piece, moves it along its velocity over the ground and trails a dust
// particle off it, fading the last seconds; also counts down the 8 debris
// timers.
// FUNCTION: CMR2 0x0047dd70
void FUN_0047dd70(void)
{
    int *pTimer;
    BYTE *q;
    int left;
    short tri;
    FixVector old;
    FixVector d;
    FixVector n;
    FixVector r;
    FixVector m;
    int dot;
    int fade;
    int ground;

    for (pTimer = g_unk0x0058e4a8; pTimer < g_unk0x0058e4a8 + 8; pTimer++) {
        if (*pTimer > 0) {
            *pTimer -= g_physicsTimeStep;
            if (*pTimer < 0)
                *pTimer = 0;
        }
    }
    q = g_unk0x0058e4e0[0] + 0x1c;
    for (left = 100; left != 0; left--, q += 0x5c) {
        if (*(int *)(q + 0x20) == 0)
            continue;
        *(FixVector *)(q - 0xc) = *(FixVector *)(q - 0x28);
        *(FixVector *)q = *(FixVector *)(q - 0x1c);
        *(int *)(q + 0xc) = *(int *)(q - 0x10);
        *(int *)(q + 0x14) -= g_physicsTimeStep;
        if (*(int *)(q + 0x14) <= 0) {
            *(int *)(q + 0x20) = 0;
            continue;
        }
        old = *(FixVector *)(q - 0x28);
        *(int *)(q - 0x28) += *(int *)(q - 0x34);
        *(int *)(q - 0x24) += *(int *)(q - 0x30);
        *(int *)(q - 0x20) += *(int *)(q - 0x2c);
        ground = Track_GetGroundHeightSurface((FixVector *)(q - 0x28), (FixVector *)(q - 0x1c), (short *)(q + 0x18),
                                              &tri, (unsigned short *)&tri, *(int *)(q + 0x10));
        *(int *)(q + 0x10) = ground;
        *(int *)(q - 0x24) = ground + 0x8000;
        d.x = *(int *)(q - 0x28) - old.x;
        d.y = *(int *)(q - 0x24) - old.y;
        d.z = *(int *)(q - 0x20) - old.z;
        FIX_NORMALIZE_INTO(n, d);
        r.x = 0x8000 - RAND_FIX();
        r.y = 0x8000 - RAND_FIX();
        r.z = 0x8000 - RAND_FIX();
        dot = FixVecDot(&n, &r);
        FixVecScale(&m, &n, dot);
        r.x -= m.x;
        r.y -= m.y;
        r.z -= m.z;
        FixVecScale(&n, &n, -0x8000);
        r.x += n.x;
        r.y += n.y;
        r.z += n.z;
        FixVecScale(&r, &r, 0x4ccc);
        r.x += d.x;
        r.y += d.y;
        r.z += d.z;
        m.x = *(int *)(q - 0x28) - r.x;
        m.y = *(int *)(q - 0x24) - r.y;
        m.z = *(int *)(q - 0x20) - r.z;
        Particle_Spawn(0x1f, &m, &r, *(int *)(q - 0x24) - 0x50000, 0, NULL, 0, 0,
                       *(BYTE *)(*(BYTE **)((BYTE *)Car_Get(q[0x24]) + 0x720) + 0x17c));
        if (*(int *)(q + 0x14) > 0x50000) {
            *(int *)(q - 0x10) = 0x10000;
        } else {
            fade = FixMul(*(int *)(q + 0x14), 0x3333);
            if (fade > 0x10000)
                fade = 0x10000;
            else if (fade < 0)
                fade = 0;
            *(int *)(q - 0x10) = fade;
        }
    }
}
#undef RAND_FIX

// Flash colour of each firework colour index.
// GLOBAL: CMR2 0x0051f504
DWORD g_fireworkFlashColours[6] = {
    0xff3c1955, 0xff551f10, 0xff555514, 0xff1a4155, 0xff0c2355, 0xff0d5535
};

void Sound_Free(unsigned int handle);

#define RAND_FIX() ((int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536))
#define RAND_SIGNED()                                                                              \
    (RAND_FIX() <= 0x8000 ? RAND_FIX() : (int)(__int64)((float)rand() * g_oneOverRandMax * g_minus65536))

// Per-frame update of the fireworks. A rocket (0x938 bytes) rises trailing a
// 20-point spark trail; when its fuse runs out it bursts, either into debris
// or into 18 sparks thrown along the 3x3 burst directions (mirrored up and
// down) that then fall under gravity until the burst timer ends.
// FUNCTION: CMR2 0x0047eab0
void FUN_0047eab0(void)
{
    int rocket;
    int *p;
    int i;
    int j;
    int k;
    int n;
    int vy;
    int t;
    int speed;
    int dot;
    int *pSpark;
    FixVector *pDir;
    FixVector d;
    FixVector nrm;
    FixVector s;
    FixVector r;
    char idx;

    for (rocket = 0; rocket < g_unk0x00590afc; rocket++) {
        p = (int *)((BYTE *)g_unk0x00590af8 + rocket * 0x938);
        p[0x78] = p[0];
        p[0x79] = p[1];
        p[0x7a] = p[2];
        for (k = 0; k < 20; k++) {
            p[0x7e + k * 3] = p[6 + k * 3];
            p[0x7f + k * 3] = p[7 + k * 3];
            p[0x80 + k * 3] = p[8 + k * 3];
        }
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 6; j++) {
                p[0xf6 + (i * 6 + j) * 3] = p[0x42 + (i * 6 + j) * 3];
                p[0xf7 + (i * 6 + j) * 3] = p[0x43 + (i * 6 + j) * 3];
                p[0xf8 + (i * 6 + j) * 3] = p[0x44 + (i * 6 + j) * 3];
            }
        }
        if (p[0x1a5] == 1) {
            vy = p[4];
            p[4] = vy - p[0x19b];
            d.x = p[0];
            d.y = p[1];
            d.z = p[2];
            p[0] += p[3];
            p[1] += vy - p[0x19b];
            d.x -= p[0];
            p[2] += p[5];
            d.y -= p[1];
            d.z -= p[2];
            FIX_NORMALIZE_INTO(nrm, d);
            for (k = 4; k != 0; k--) {
                t = RAND_FIX();
                FixVecScale(&s, &d, t);
                idx = ((char *)p)[0x691];
                p[(idx + 2) * 3] = p[0] + s.x;
                p[idx * 3 + 7] = s.y + p[1];
                p[idx * 3 + 8] = p[2] + s.z;
                r.x = RAND_SIGNED();
                r.y = RAND_SIGNED();
                r.z = RAND_SIGNED();
                FixVecScale(&r, &r, 0xccc);
                dot = FixVecDot(&nrm, &r);
                FixVecScale(&s, &nrm, dot);
                r.x -= s.x;
                r.z -= s.z;
                r.y -= s.y;
                idx = ((char *)p)[0x691];
                p[(idx + 2) * 3] += r.x;
                p[idx * 3 + 7] += r.y;
                p[idx * 3 + 8] += r.z;
                if (RAND_FIX() <= 0x8000)
                    p[0x1a7 + ((char *)p)[0x691]] = 0;
                else
                    p[0x1a7 + ((char *)p)[0x691]] = 1;
                idx = ((char *)p)[0x691];
                ((char *)p)[0x691] = idx + 1;
                if ((char)(idx + 1) > 19)
                    ((char *)p)[0x691] = idx - 19;
            }
            ((char *)p)[0x690] += 4;
            if (((char *)p)[0x690] > 19)
                ((char *)p)[0x690] = 19;
            p[0x199] -= 0x10000;
            if (p[0x199] < 1) {
                if (((char *)p)[0x693] != -1 && Sound_IsPlaying(((char *)p)[0x693]))
                    Sound_Free(((char *)p)[0x693]);
                p[0x1a5] = 2;
                g_unk0x005909b8 = 0x10000;
                *(DWORD *)g_unk0x005909c4 = g_fireworkFlashColours[((BYTE *)p)[0x692]];
                if (p[0x24b] == 0) {
                    if (p[0x1a6] == 2) {
                        for (n = rand() % 4 + 1; n > 0; n--) {
                            s.x = RAND_SIGNED();
                            s.y = RAND_SIGNED();
                            s.z = RAND_SIGNED();
                            s.x = FixMul(s.x, 0xe666);
                            s.y = FixMul(s.y, 0xe666);
                            s.z = FixMul(s.z, 0xe666);
                            StageObject_SpawnDebris((FixVector *)p, &s, 1);
                        }
                    }
                } else {
                    p[0x78] = p[0];
                    p[0x79] = p[1];
                    p[0x7a] = p[2];
                    for (k = 0; k < 20; k++) {
                        p[0x7e + k * 3] = p[6 + k * 3];
                        p[0x7f + k * 3] = p[7 + k * 3];
                        p[0x80 + k * 3] = p[8 + k * 3];
                    }
                    for (i = 0; i < 3; i++) {
                        for (j = 0; j < 3; j++) {
                            pSpark = p + 0x42 + (i * 6 + j) * 3;
                            pSpark[0] = 0;
                            pSpark[1] = 0;
                            pSpark[2] = 0;
                            pSpark[9] = 0;
                            pSpark[10] = 0;
                            pSpark[11] = 0;
                            pSpark[0xb4] = 0;
                            pSpark[0xb5] = 0;
                            pSpark[0xb6] = 0;
                            pSpark[0xbd] = 0;
                            pSpark[0xbe] = 0;
                            pSpark[0xbf] = 0;
                            speed = FixMul(RAND_FIX(), 0xa3d) + p[0x198];
                            pDir = (FixVector *)g_unk0x00590b0c[i] + j;
                            pSpark[0x120] = FixMul(pDir->x, speed);
                            pSpark[0x121] = FixMul(pDir->y, speed);
                            pSpark[0x122] = FixMul(pDir->z, speed);
                            FixVecScale((FixVector *)(pSpark + 0x120), (FixVector *)(pSpark + 0x120), 0x60000);
                            pSpark[0x129] = pSpark[0x120];
                            pSpark[0x12a] = pSpark[0x121];
                            pSpark[0x12b] = pSpark[0x122];
                            pSpark[0x12a] = -pSpark[0x121];
                        }
                    }
                    FUN_004b7790((unsigned short)(rand() % 6 + 2 + g_unk0x005909bc), 0x10000, 0x5622, 0, 0, 0);
                }
            }
        } else if (p[0x1a5] == 2) {
            if (((char *)p)[0x690] > 0) {
                idx = ((char *)p)[0x691];
                ((char *)p)[0x690] -= 4;
                ((char *)p)[0x691] = idx + 4;
                if ((char)(idx + 4) > 19)
                    ((char *)p)[0x691] = idx - 16;
            }
            for (k = 0; k < 18; k++) {
                p[0x163 + k * 3] -= p[0x19c];
                p[0x42 + k * 3] += p[0x162 + k * 3];
                p[0x43 + k * 3] += p[0x163 + k * 3];
                p[0x44 + k * 3] += p[0x164 + k * 3];
            }
            p[0x19a] -= 0x10000;
            p[0x24d] = p[0x24d] == 0;
            if (p[0x19a] < 1)
                p[0x1a5] = 0;
        }
    }
}
#undef RAND_FIX
#undef RAND_SIGNED

void CarEffects_InitDebris(void);
void FUN_00494bb0(void);
void FUN_0045eca0(void);
void CarEffects_Init(void);
void __fastcall FUN_0045a170(int param_1);
int FUN_00407270(void);

// Sets up the stage objects of a race: object tables, the championship-end
// fireworks, the headlight glows, the wheel trails and lights of every car,
// the debris, weather, effects and stage lights.
// FUNCTION: CMR2 0x00466360
void FUN_00466360(void)
{
    short *pOrder;
    short count;
    int i;
    int car;

    FUN_00466490();
    g_unk0x0058896c = (char)FUN_00407270() != 0;
    if (((char)FUN_00407270() || (char)RallyData_GetFlag24() || (char)RallyData_FUN_00407e90()) &&
        CGameInfo::FUN_004063f0(0))
        FUN_0047d510();
    pOrder = Car_GetOrder();
    count = Car_GetOrderCount();
    for (i = 0; i < count; i++, pOrder++) {
        car = *pOrder;
        FUN_0045a150(0, 0, i);
        FUN_0045a150(0, 1, i);
        FUN_0045b530(0, 0, i);
        FUN_0045b530(0, 1, i);
        if (*(int *)((BYTE *)Car_Get(car) + 0xc0c) == 0) {
            FUN_00463fe0((int)Car_Get(car));
            if (Car_Get(car)->field_0xb1b[0] == 6 || Car_Get(car)->field_0xb1b[0] == 7 ||
                Car_Get(car)->field_0xb1b[0] == 10)
                FUN_00477b60(i, 0, 0, 0);
            else
                FUN_00477b60(i, 0, 0, 1);
        }
    }
    CarEffects_InitDebris();
    FUN_00494bb0();
    FUN_0045eca0();
    CarEffects_Init();
    StageLights_Create();
    FUN_0045a170(0);
    if (g_unk0x0058896c != 0)
        FUN_0047e4d0(0x14);
}

// Per-frame update of the stage objects: stage lights, the attract-mode
// debris, and the fireworks once they are on.
// FUNCTION: CMR2 0x00466520
void FUN_00466520(void)
{
    StageLights_Update();
    if (((char)FUN_00407270() || (char)RallyData_GetFlag24() || (char)RallyData_FUN_00407e90()) &&
        CGameInfo::FUN_004063f0(0))
        FUN_0047dd70();
    if (g_unk0x0058896c != 0) {
        FUN_0047eab0();
        FUN_00480220();
    }
}

int FUN_004483c0(int index);
void FUN_00407940(unsigned int first, unsigned int second);
void FUN_00407b10(void);
BYTE FUN_0041f380(void);

// Finishes the knockout match of the current round: stores the split times of
// the two drivers (or the fallback order when a driver is unknown), clears or
// marks the match entry, flags the loser, advances the round index of the
// championship state and re-propagates the bracket.
// match 96%: the only remaining differences are the stack slots of the four
// locals (the original keeps the two times in the two lower slots, this build
// puts them in the middle); every instruction and operand value is identical.
// Bit layout of KnockoutMatch::flags as the original manipulates it: two
// five-bit driver indices, the completion bit of the match and the two-bit
// winner flag (1 second driver faster, 2 first driver faster).
struct KnockoutMatchBits {
    unsigned int driver1 : 5;
    unsigned int driver2 : 5;
    unsigned int played : 1;
    unsigned int winner : 2;
    unsigned int pad : 19;
};

// Bit layout of KnockoutTable::state: the kind of bracket (1..4) at bits 3-5
// and the index of the round within that kind at bits 12-15.
struct KnockoutStateBits {
    unsigned int pad0 : 3;
    unsigned int kind : 3;
    unsigned int pad1 : 6;
    unsigned int round : 4;
    unsigned int pad2 : 16;
};

// FUNCTION: CMR2 0x00472a30
void FUN_00472a30(void)
{
    struct { int times[2]; unsigned int drivers[2]; } results;
    KnockoutTable *pState;
    unsigned int state;
    int round;
    char t;

    pState = (KnockoutTable *)RallyData_GetChampionshipState();
    RallyData_GetRoundDrivers(&results.drivers[0], &results.drivers[1]);
    if (RallyData_FUN_00408500((BYTE)results.drivers[0]) != -1 && RallyData_FUN_00408500((BYTE)results.drivers[1]) != -1) {
        StageTiming_GetSplitTimesForPositions(results.drivers[0], results.drivers[1], &results.times[0], &results.times[1]);
    } else if (RallyData_FUN_00408500((BYTE)results.drivers[0]) == -1) {
        results.times[0] = FUN_004483c0(0);
        results.times[1] = FUN_004483c0(1);
    } else {
        results.times[1] = FUN_004483c0(0);
        results.times[0] = FUN_004483c0(1);
    }
    if (FUN_00473680(RallyData_GetRoundEntry()) != 0) {
        ((KnockoutMatch *)RallyData_GetRoundEntry())->time1 = 0;
        ((KnockoutMatch *)RallyData_GetRoundEntry())->time2 = 0;
        if ((((KnockoutMatch *)RallyData_GetRoundEntry())->flags & 0x1f) == 0x1f)
            ((KnockoutMatchBits *)RallyData_GetRoundEntry())->winner = 2;
        else
            ((KnockoutMatchBits *)RallyData_GetRoundEntry())->winner = 1;
    } else {
        FUN_00407940(results.times[0], results.times[1]);
    }
    if (FUN_0041f380() != 0xff) {
        if ((BYTE)RallyDataState() == 2) {
            t = (char)FUN_0041f380();
            ((KnockoutMatchBits *)RallyData_GetRoundEntry())->winner = -2 - t;
        } else if (RallyData_FUN_00408500(
                       (BYTE)((KnockoutMatch *)RallyData_GetRoundEntry())->flags & 0x1f) == -1) {
            ((KnockoutMatchBits *)RallyData_GetRoundEntry())->winner = 2;
        } else {
            ((KnockoutMatchBits *)RallyData_GetRoundEntry())->winner = 1;
        }
    }
    state = pState->state;
    round = (state >> 12) & 0xf;
    ((KnockoutStateBits *)pState)->round++;
    switch ((pState->state >> 3) & 7) {
    case 1:
        pState->round1[round].flags |= 0x400;
        if ((pState->state & 0xf000) > 0x7000)
            pState->state |= 0x400000;
        break;
    case 2:
        pState->quarters[round].flags |= 0x400;
        if ((pState->state & 0xf000) > 0x3000)
            pState->state |= 0x400000;
        break;
    case 3:
        pState->semis[round].flags |= 0x400;
        if ((pState->state & 0xf000) > 0x1000)
            pState->state |= 0x400000;
        break;
    case 4:
        pState->state |= 0x400000;
        pState->final.flags |= 0x400;
        break;
    }
    FUN_00407b10();
}

int FUN_004055e0(void);
int FUN_004055f0(void);
void FUN_00474420(char *text, int fraction, unsigned int font, int x, unsigned int y, int *pColour,
                  unsigned int flags);
int FUN_00475970(int scale, int unused, short *pRect, BYTE *pColour, int layer);

// Draws the in-race pause menu: for every entry the background of the selected
// row, its label (the stage name with the round number in the knockout modes,
// the entry name otherwise) and the row separators, then the knockout bracket
// of the current round. The rows move down by KO_Y(6) on screens at least
// 1024 pixels wide whose texture limits accept that size.
// match 58%: the prologue, the constants, the call arguments and the control
// flow are identical; the residual diff is register/slot allocation inside the
// loop body (the original keeps the row height, the loop counter and the two
// rectangles in different registers than this build does). Kept as FUNCTION on
// purpose so reccmp measures it (see CONVENCIONES).
// FUNCTION: CMR2 0x00473d60
void FUN_00473d60(Menu *pMenu)
{
    int i;
    short rect[4];
    short rect2[4];
    int texture;
    int y0;
    int y;
    int halfHeight;
    BYTE *pColour;
    unsigned int *pState;

    rect[0] = (short)((int)(g_pGraphics->resX * 0x64) / 0x280);
    rect[1] = 0;
    texture = FUN_004055e0();
    rect[2] = *(short *)(texture + 0x120);
    texture = FUN_004055e0();
    rect[3] = *(short *)(texture + 0x122);
    y0 = (int)(g_pGraphics->resY * 0x17c) / 0x1e0;
    pState = RallyData_GetChampionshipState();
    for (i = 0; i < pMenu->itemCount; i++) {
        texture = FUN_004055e0();
        halfHeight = *(short *)(texture + 0x122) / 2;
        rect[1] = (short)((int)(g_pGraphics->resY * 0x24) / 0x1e0 * i +
                          (int)(g_pGraphics->resY * 0x14) / 0x1e0 - halfHeight + y0);
        y = (int)(g_pGraphics->resY * 0x24) / 0x1e0 * i
            - (int)(g_pGraphics->resY * halfHeight) / 0x1e0
            + (int)(g_pGraphics->resY * 0x14) / 0x1e0 + y0
            + (int)(g_pGraphics->resY * 0xe) / 0x1e0;
        if (CGameInfo::GetScreenWidth() >= 0x400 && CFrontend::FUN_004b7560(0x400) &&
            CFrontend::FUN_004b7590(0x400))
            y += (int)(g_pGraphics->resY * 6) / 0x1e0;
        if (pMenu->cursor == i) {
            pColour = g_barTextColour;
            texture = FUN_004055e0();
        } else {
            pColour = g_unk0x0051c994;
            texture = FUN_004055f0();
        }
        switch (g_unk0x0058cf7c) {
        case 1:
            if (i == 0)
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x77));
            else
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x78));
            break;
        case 2:
            if (i == 0)
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x79),
                        ((*pState >> 12) & 0xf) + 1);
            else
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x78));
            break;
        case 5:
            if (i == 0)
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x7a),
                        ((*pState >> 12) & 0xf) + 1);
            else
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x78));
            if ((*pState & 0x400000) == 0 && (*pState & 0x38) != 0x20)
                continue;
            break;
        default:
            continue;
        }
        if (i == 0 || pMenu->cursor == i) {
            if (FUN_004bc0c0(&g_unk0x0058cf60)) {
                rect2[0] = (short)((int)(g_pGraphics->resX * 0x63) / 0x280);
                rect2[1] = (short)((int)(g_pGraphics->resY * i * 0x24) / 0x1e0 + y0);
                rect2[2] = (short)((int)(g_pGraphics->resX * 0x11a) / 0x280);
                rect2[3] = 1;
                FUN_00475970(g_unk0x0058ce58, (int)g_pGraphics + 0x150, rect2, pColour, 1);
            } else {
                rect2[0] = (short)((int)(g_pGraphics->resX * 0x63) / 0x280);
                rect2[1] = (short)((int)(g_pGraphics->resY * i * 0x24) / 0x1e0 + y0);
                rect2[2] = (short)((int)(g_pGraphics->resX * 0x11a) / 0x280);
                rect2[3] = 1;
                Sprite_FillRect((int)g_pGraphics + 0x150, rect2, pColour, 1);
            }
        }
        rect2[0] = (short)((int)(g_pGraphics->resX * 0x63) / 0x280);
        rect2[1] = (short)((int)(g_pGraphics->resY * i * 0x24) / 0x1e0 + y0);
        rect2[2] = (short)((int)(g_pGraphics->resX * 0x11a) / 0x280);
        rect2[3] = 1;
        if (FUN_004bc0c0(&g_unk0x0058cf60)) {
            Sprite_Queue((SpriteRect *)(texture + 0x11c), (SpriteRect *)rect, (Texture *)texture, 2,
                         0, NULL, NULL, pColour, 8);
            FUN_00474420(CFrontend::m_stringDest, g_unk0x0058ce58, 0,
                         (int)(g_pGraphics->resX * 0x7a) / 0x280,
                         y, (int *)pColour, 0x21);
        } else {
            Sprite_Queue((SpriteRect *)(texture + 0x11c), (SpriteRect *)rect, (Texture *)texture, 2,
                         0, NULL, NULL, pColour, 8);
            Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x7a) / 0x280,
                          y, (int *)pColour, 0x21);
        }
        rect2[0] = (short)((int)(g_pGraphics->resX * 0x63) / 0x280);
        rect2[1] = (short)((int)(g_pGraphics->resY * i * 0x24) / 0x1e0 +
                           (int)(g_pGraphics->resY * 0x24) / 0x1e0 + y0);
        rect2[2] = (short)((int)(g_pGraphics->resX * 0x11a) / 0x280);
        rect2[3] = 1;
        if (FUN_004bc0c0(&g_unk0x0058cf60))
            FUN_00475970(g_unk0x0058ce58, (int)g_pGraphics + 0x150, rect2, pColour, 2);
        else
            Sprite_FillRect((int)g_pGraphics + 0x150, rect2, pColour, 2);
    }
    Font_SetBlendMode(2);
    FUN_004744f0();
}
// match 88%: logica, llamadas y constantes identicas al original; el residuo es
// reparto de registros y de slots de pila temporales (el original cachea el puntero
// del vector en esi para los tres stores del caso len==0 y materializa los dos
// operandos del FixMul del rotor) y el orden de los stores de angles[].


void FUN_00481560(unsigned short *pAngles);
void VehicleMotion_UpdateWorldPosition(void);
struct Unk0x00590c20;
extern Unk0x00590c20 *g_unk0x00590c20;
extern int g_unk0x00590c68;
extern const double g_unk0x00511380;

// FUNCTION: CMR2 0x00484310
void FUN_00484310(void)
{
    unsigned short angles[3];
    FixVector scratch;
    FixVector motion;
    FixVector cross;
    FixVector local;
    int modified;
    int type;
    int t;

    if (*(char *)((BYTE *)g_unk0x00590d74 + 0xb1b) != 9 &&
        *(char *)((BYTE *)g_unk0x00590d74 + 0xb1b) != 0xb) {
        motion.x = 0;
        motion.y = 0;
        motion.z = 0;
        scratch.x = *(int *)((BYTE *)g_unk0x00590d74 + 0x408) -
                    *(int *)((BYTE *)g_unk0x00590d74 + 0x414);
        scratch.y = *(int *)((BYTE *)g_unk0x00590d74 + 0x40c) -
                    *(int *)((BYTE *)g_unk0x00590d74 + 0x418);
        scratch.z = *(int *)((BYTE *)g_unk0x00590d74 + 0x410) -
                    *(int *)((BYTE *)g_unk0x00590d74 + 0x41c);
        FixVecLength(&scratch);
        FixVecScale(&scratch, &scratch, -0x60000);
        if (*(char *)((BYTE *)g_unk0x00590d74 + 0xb1b) != 8) {
            type = g_unk0x00590c24[2][*(char *)((BYTE *)g_unk0x00590d74 + 0xb1a)];
            if (*(BYTE *)(*(int *)(g_unk0x00590d78 + 0x3c + type * 4) + 0x30) == 0xc)
                scratch.x = -scratch.x;
        }
        motion.x += scratch.x;
        motion.y += scratch.y;
        motion.z += scratch.z;
        motion.y -= 0x53f7;
        FixMatrix_InverseRotateVector(&local, &motion,
                                      *(FixMatrix **)((BYTE *)g_unk0x00590c20 + 8));
        scratch.x = *(int *)((BYTE *)g_unk0x00590c20 + 0x12c) -
                    *(int *)((BYTE *)g_unk0x00590c20 + 0x120);
        scratch.y = *(int *)((BYTE *)g_unk0x00590c20 + 0x130) -
                    *(int *)((BYTE *)g_unk0x00590c20 + 0x124);
        scratch.z = *(int *)((BYTE *)g_unk0x00590c20 + 0x134) -
                    *(int *)((BYTE *)g_unk0x00590c20 + 0x128);
        if (*(char *)((BYTE *)g_unk0x00590d74 + 0xb1b) == 8)
            scratch.x = 0;
        scratch.y = 0x3333;
        FixVecCross(&cross, &local, &scratch);
        if ((*(BYTE *)((BYTE *)g_unk0x00590c20 + 0x150) & 8) == 0 && cross.z > 0)
            *(BYTE *)((BYTE *)g_unk0x00590c20 + 0x150) |= 8;
        if ((*(BYTE *)((BYTE *)g_unk0x00590c20 + 0x150) & 8) != 0) {
            FixVecScale((FixVector *)((BYTE *)g_unk0x00590c20 + 0x164),
                        (FixVector *)((BYTE *)g_unk0x00590c20 + 0x164), 0xe49b);
            t = -FixMul(cross.z, *(int *)((BYTE *)g_unk0x00590c20 + 0x178));
            t = FixMul(t, g_physicsTimeStep);
            *(int *)((BYTE *)g_unk0x00590c20 + 0x16c) += t;
            angles[0] = 0;
            angles[1] = 0;
            angles[2] = (short)((double)(FixMul(*(int *)((BYTE *)g_unk0x00590c20 + 0x16c),
                                                g_physicsTimeStep) -
                                          FixMul(t, g_physicsTimeStep / 2)) *
                                g_unk0x00511380);
            FixBasis_Rotate((FixBasis *)((BYTE *)g_unk0x00590c20 + 0x17c), angles);
            modified = 0;
            if (*(char *)((BYTE *)g_unk0x00590d74 + 0xb1b) == 8) {
                if (*(int *)((BYTE *)g_unk0x00590c20 + 0x180) < 0) {
                    *(int *)((BYTE *)g_unk0x00590c20 + 0x180) = 0;
                    modified = 1;
                    if (*(int *)((BYTE *)g_unk0x00590c20 + 0x16c) < 0) {
                        if (*(int *)((BYTE *)g_unk0x00590c20 + 0x16c) < -0xf5c)
                            *(int *)((BYTE *)g_unk0x00590c20 + 0x16c) =
                                -FixMul(0x9999, *(int *)((BYTE *)g_unk0x00590c20 + 0x16c));
                        else {
                            *(int *)((BYTE *)g_unk0x00590c20 + 0x16c) = 0;
                            *(BYTE *)((BYTE *)g_unk0x00590c20 + 0x150) &= 0xf7;
                        }
                    }
                }
                if (*(int *)((BYTE *)g_unk0x00590c20 + 0x17c) < 0x6666) {
                    *(int *)((BYTE *)g_unk0x00590c20 + 0x17c) = 0x6666;
                    if (*(int *)((BYTE *)g_unk0x00590c20 + 0x16c) > 0)
                        *(int *)((BYTE *)g_unk0x00590c20 + 0x16c) = 0;
                    goto renormalise;
                }
            } else {
                if (*(int *)((BYTE *)g_unk0x00590c20 + 0x180) > 0) {
                    *(int *)((BYTE *)g_unk0x00590c20 + 0x180) = 0;
                    modified = 1;
                    if (*(int *)((BYTE *)g_unk0x00590c20 + 0x16c) > 0) {
                        if (*(int *)((BYTE *)g_unk0x00590c20 + 0x16c) > 0xf5c)
                            *(int *)((BYTE *)g_unk0x00590c20 + 0x16c) =
                                -FixMul(0x9999, *(int *)((BYTE *)g_unk0x00590c20 + 0x16c));
                        else {
                            *(int *)((BYTE *)g_unk0x00590c20 + 0x16c) = 0;
                            *(BYTE *)((BYTE *)g_unk0x00590c20 + 0x150) &= 0xf7;
                        }
                    }
                }
                if (*(int *)((BYTE *)g_unk0x00590c20 + 0x17c) < 0xcccc) {
                    *(int *)((BYTE *)g_unk0x00590c20 + 0x17c) = 0xcccc;
                    if (*(int *)((BYTE *)g_unk0x00590c20 + 0x16c) < 0)
                        *(int *)((BYTE *)g_unk0x00590c20 + 0x16c) = 0;
                    goto renormalise;
                }
            }
            if (modified == 0) {
                goto anglesZ;
            }
        renormalise:
            FIX_NORMALIZE_INTO((*(FixVector *)((BYTE *)g_unk0x00590c20 + 0x17c)),
                               (*(FixVector *)((BYTE *)g_unk0x00590c20 + 0x17c)));
            FixVecScale(&scratch, (FixVector *)((BYTE *)g_unk0x00590c20 + 0x17c),
                        FixVecDot((FixVector *)((BYTE *)g_unk0x00590c20 + 0x17c),
                                  (FixVector *)((BYTE *)g_unk0x00590c20 + 0x194)));
            *(int *)((BYTE *)g_unk0x00590c20 + 0x194) -= scratch.x;
            *(int *)((BYTE *)g_unk0x00590c20 + 0x198) -= scratch.y;
            *(int *)((BYTE *)g_unk0x00590c20 + 0x19c) -= scratch.z;
            FIX_NORMALIZE_INTO((*(FixVector *)((BYTE *)g_unk0x00590c20 + 0x194)),
                               (*(FixVector *)((BYTE *)g_unk0x00590c20 + 0x194)));
            FixVecCross(&scratch, (FixVector *)((BYTE *)g_unk0x00590c20 + 0x194),
                        (FixVector *)((BYTE *)g_unk0x00590c20 + 0x17c));
            FIX_NORMALIZE_INTO((*(FixVector *)((BYTE *)g_unk0x00590c20 + 0x188)), scratch);
        }
    anglesZ:
        angles[0] = 0;
        angles[1] = 0;
        angles[2] = (short)((double)FixMul((short)g_unk0x00590c68 * 0x1680, 0x10000) *
                            g_unk0x00511308);
    } else {
        if (*(char *)((BYTE *)g_unk0x00590d74 + 0xb1b) == 9) {
            angles[0] = 0;
            angles[2] = 0;
            angles[1] = (short)((double)FixMul((short)g_unk0x00590c68 * 0x1680, 0x10000) *
                                g_unk0x00511308);
        } else {
            angles[1] = 0;
            angles[2] = 0;
            angles[0] = (short)((double)FixMul((short)g_unk0x00590c68 * 0x1680, 0x30000) *
                                g_unk0x00511308);
        }
    }
    FUN_00481560(angles);
    VehicleMotion_UpdateWorldPosition();
}
// Draws the fireworks every frame: the rising rocket as a single billboard, or
// for a burst the 18 spark clusters (each up to four mirrored billboards) plus
// the fading 20-point trail. See FUN_0047eab0 for the record layout.
// match 38%: the logic, calls, constants and loop bounds are identical; the
// residual is MSVC's register allocation, stack-slot placement (the original
// spills the spark Y to [ebp-0x44] and keeps the frame at 0x48) and its choice
// of cursor/induction-variable base (0x4b0 vs 0x4b8) in the 18-spark loop.
// FUNCTION: CMR2 0x0047f740
void FUN_0047f740(void)
{
    BYTE *pSlot;
    int i;
    int offset;
    int state;
    int colourIndex;
    int k;
    int j;
    int n;
    int fade;
    FixVector spark;
    int sparkX;
    int sparkY;
    int sparkZ;
    int *pSpark;
    int *pFlag;
    int *pTrail;
    int baseR;
    int baseG;
    int baseB;
    int litR;
    int litG;
    int litB;
    BYTE colourA[4];
    BYTE colourB[4];
    BYTE colourC[4];

    i = 0;
    if (g_unk0x00590afc == 0)
        return;
    offset = 0;
    for (; i < g_unk0x00590afc; i++) {
        pSlot = (BYTE *)g_unk0x00590af8 + offset;
        state = *(int *)(pSlot + 0x694);
        switch (state) {
        case 1:
            *(FixVector *)g_unk0x00590b04 = *(FixVector *)(pSlot + 0x1ec);
            *(int *)((BYTE *)g_unk0x00590b04 + 0x1c) = *(int *)(pSlot + 0x678);
            Billboard_Add((BillboardDef *)g_unk0x00590b04, (unsigned short *)g_unk0x00590b00);
            colourIndex = 0;
            break;
        case 2:
            FUN_0047f510((int)pSlot, colourA, pSlot + 0x680, pSlot + 0x684);
            FUN_0047f510((int)pSlot, colourB, pSlot + 0x688, pSlot + 0x68c);
            if (*(int *)(pSlot + 0x92c) != 0) {
                for (k = 0; k < 3; k++) {
                    for (j = 0; j < 6; j++) {
                        spark = *(FixVector *)(pSlot + 0x4b0 + (k * 6 + j) * 12);
                        sparkX = spark.x;
                        sparkY = spark.y;
                        sparkZ = spark.z;
                        if (*(int *)(pSlot + 0x6ec + (k * 6 + j) * 16) != 0) {
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x00) = *(int *)(pSlot + 0x1ec) + sparkX;
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x04) = *(int *)(pSlot + 0x1f0) + sparkY;
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x08) = *(int *)(pSlot + 0x1f4) + sparkZ;
                            if (*(int *)(pSlot + 0x80c + (k * 6 + j) * 16) != 0) {
                                if (*(int *)(pSlot + 0x934) == 0)
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourA;
                                else
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourB;
                            } else {
                                if (*(int *)(pSlot + 0x934) != 0)
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourA;
                                else
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourB;
                            }
                            Billboard_Add((BillboardDef *)g_unk0x00590b08, (unsigned short *)g_unk0x00590b00);
                        }
                        if (*(int *)(pSlot + 0x6f0 + (k * 6 + j) * 16) != 0) {
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x00) = *(int *)(pSlot + 0x1ec) - sparkX;
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x04) = *(int *)(pSlot + 0x1f0) + sparkY;
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x08) = *(int *)(pSlot + 0x1f4) + sparkZ;
                            if (*(int *)(pSlot + 0x810 + (k * 6 + j) * 16) != 0) {
                                if (*(int *)(pSlot + 0x934) == 0)
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourA;
                                else
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourB;
                            } else {
                                if (*(int *)(pSlot + 0x934) != 0)
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourA;
                                else
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourB;
                            }
                            Billboard_Add((BillboardDef *)g_unk0x00590b08, (unsigned short *)g_unk0x00590b00);
                        }
                        if (*(int *)(pSlot + 0x6f4 + (k * 6 + j) * 16) != 0) {
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x00) = *(int *)(pSlot + 0x1ec) - sparkX;
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x04) = *(int *)(pSlot + 0x1f0) + sparkY;
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x08) = *(int *)(pSlot + 0x1f4) - sparkZ;
                            if (*(int *)(pSlot + 0x814 + (k * 6 + j) * 16) != 0) {
                                if (*(int *)(pSlot + 0x934) == 0)
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourA;
                                else
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourB;
                            } else {
                                if (*(int *)(pSlot + 0x934) != 0)
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourA;
                                else
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourB;
                            }
                            Billboard_Add((BillboardDef *)g_unk0x00590b08, (unsigned short *)g_unk0x00590b00);
                        }
                        if (*(int *)(pSlot + 0x6f8 + (k * 6 + j) * 16) != 0) {
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x00) = *(int *)(pSlot + 0x1ec) + sparkX;
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x04) = *(int *)(pSlot + 0x1f0) + sparkY;
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x08) = *(int *)(pSlot + 0x1f4) - sparkZ;
                            if (*(int *)(pSlot + 0x818 + (k * 6 + j) * 16) != 0) {
                                if (*(int *)(pSlot + 0x934) == 0)
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourA;
                                else
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourB;
                            } else {
                                if (*(int *)(pSlot + 0x934) != 0)
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourA;
                                else
                                    *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourB;
                            }
                            Billboard_Add((BillboardDef *)g_unk0x00590b08, (unsigned short *)g_unk0x00590b00);
                        }
                    }
                }
            }
            colourIndex = 0x14 - *(signed char *)(pSlot + 0x690);
            break;
        }
        if (state != 0) {
            baseR = pSlot[0x67c] << 16;
            baseG = pSlot[0x67d] << 16;
            baseB = pSlot[0x67e] << 16;
            litR = baseR + 0x640000;
            litG = baseG + 0x640000;
            litB = baseB + 0x640000;
            colourC[3] = 0xff;
            fade = 0x3333;
            j = 0;
            if (*(signed char *)(pSlot + 0x690) > 0) {
                do {
                    colourA[0] = (BYTE)FixMulShift32(baseR, fade);
                    colourA[1] = (BYTE)FixMulShift32(baseG, fade);
                    colourA[2] = (BYTE)FixMulShift32(baseB, fade);
                    colourA[3] = 0xff;
                    colourC[0] = (BYTE)FixMulShift32(litR, fade);
                    colourC[1] = (BYTE)FixMulShift32(litG, fade);
                    colourC[2] = (BYTE)FixMulShift32(litB, fade);
                    fade += 0x3333;
                    pFlag = (int *)(pSlot + 0x69c) + colourIndex;
                    pTrail = (int *)(pSlot + 0x2e8) + colourIndex * 3;
                    n = 4;
                    do {
                        if (*pFlag != 0)
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourC;
                        else
                            *(int *)((BYTE *)g_unk0x00590b08 + 0x1c) = *(int *)colourA;
                        *(int *)((BYTE *)g_unk0x00590b08 + 0x00) = pTrail[0];
                        *(int *)((BYTE *)g_unk0x00590b08 + 0x04) = pTrail[1];
                        *(int *)((BYTE *)g_unk0x00590b08 + 0x08) = pTrail[2];
                        colourIndex++;
                        pFlag++;
                        pTrail += 3;
                        if (colourIndex >= 0x14) {
                            colourIndex -= 0x14;
                            pFlag -= 0x14;
                            pTrail -= 0x14;
                        }
                        Billboard_Add((BillboardDef *)g_unk0x00590b08, (unsigned short *)g_unk0x00590b00);
                    } while (--n != 0);
                    j += 4;
                } while (j < *(signed char *)(pSlot + 0x690));
            }
        }
        offset += 0x938;
    }
}

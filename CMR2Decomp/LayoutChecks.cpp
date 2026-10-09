#include <windows.h>
#include "Car.h"
#include "CarResources.h"
#include "CarPhysics.h"
#include "Game.h"
#include "CarParts.h"
#include "StageTiming.h"
#include <stddef.h>

// Preserve the original Win32 layouts used by array indexing, bulk copies
// and packed callback tables. Keep these checks out of shared headers:
// extra declarations there can alter MSVC6 code generation in unrelated TUs.
typedef char CarSizeCheck[sizeof(Car) == 0xc24 ? 1 : -1];
typedef char CarPositionOffsetCheck[offsetof(Car, position) == 0x2d0 ? 1 : -1];
typedef char CarWheelNodesOffsetCheck[offsetof(Car, pWheelNodes) == 0x738 ? 1 : -1];
typedef char CarWheelLoadOffsetCheck[offsetof(Car, wheelLoad) == 0x860 ? 1 : -1];
typedef char CarIndexOffsetCheck[offsetof(Car, index) == 0xb1a ? 1 : -1];
typedef char CarCameraRejectionOffsetCheck[offsetof(Car, firstCameraBlocked) == 0xb68 ? 1 : -1];
typedef char CarRemoteFlagOffsetCheck[offsetof(Car, field_0xc1c) == 0xc1c ? 1 : -1];
typedef char CallbackStateRecordSizeCheck[sizeof(CallbackStateRecord) == 8 ? 1 : -1];
typedef char CallbackStateTimerOffsetCheck[offsetof(CallbackStateRecord, elapsedTicks) == 4 ? 1 : -1];
typedef char StateCallbackPairSizeCheck[sizeof(StateCallbackPair) == 8 ? 1 : -1];
typedef char CallbackStateMachineSizeCheck[sizeof(CallbackStateMachine) == 0x10 ? 1 : -1];
typedef char CallbackStateRecordsOffsetCheck[offsetof(CallbackStateMachine, records) == 4 ? 1 : -1];
typedef char CallbackStateRulesOffsetCheck[offsetof(CallbackStateMachine, rules) == 0xc ? 1 : -1];
typedef char SessionPlayerRecordSizeCheck[sizeof(SessionPlayerRecord) == 0xd0 ? 1 : -1];
typedef char CarPartSetSizeCheck[sizeof(CarPartSet) == 0x4d0 ? 1 : -1];
typedef char CarPartDamageValuesCheck[offsetof(CarPartSet, damageValues) == 0x240 ? 1 : -1];
typedef char CarPartDamageScalesCheck[offsetof(CarPartSet, damageScales) == 0x2c8 ? 1 : -1];
typedef char CarPartDamageBiasesCheck[offsetof(CarPartSet, damageBiases) == 0x350 ? 1 : -1];
typedef char CarPartWobbleCheck[offsetof(CarPartSet, steeringWobble) == 0x3d8 ? 1 : -1];
typedef char CarPartSuspensionCheck[offsetof(CarPartSet, suspensionDamageOffset) == 0x3dc ? 1 : -1];
typedef char CarPartWheelDragCheck[offsetof(CarPartSet, wheelDamageDrag) == 0x3ec ? 1 : -1];
typedef char CarPartFrontBrakeCheck[offsetof(CarPartSet, frontBrakeScale) == 0x3fc ? 1 : -1];
typedef char CarPartRearBrakeCheck[offsetof(CarPartSet, rearBrakeScale) == 0x400 ? 1 : -1];
typedef char CarPartEngineTorqueCheck[offsetof(CarPartSet, engineTorqueScale) == 0x404 ? 1 : -1];
typedef char CarPartBodyDragCheck[offsetof(CarPartSet, bodyDamageDrag) == 0x408 ? 1 : -1];
typedef char CarPartBoundsCheck[offsetof(CarPartSet, maxX) == 0x410 ? 1 : -1];
typedef char CarPartBreakIndicesCheck[offsetof(CarPartSet, breakPartIndex) == 0x460 ? 1 : -1];
typedef char CarPartGearDamageCheck[offsetof(CarPartSet, gearShiftDamage) == 0x468 ? 1 : -1];
typedef char CarPartGlassCooldownCheck[offsetof(CarPartSet, glassCooldown) == 0x469 ? 1 : -1];
typedef char CarPartGlassEmittedCheck[offsetof(CarPartSet, glassDebrisEmitted) == 0x46c ? 1 : -1];
typedef char CarPartDamagedCheck[offsetof(CarPartSet, partDamaged) == 0x470 ? 1 : -1];
typedef char CarPartBrokenCheck[offsetof(CarPartSet, partBroken) == 0x490 ? 1 : -1];
typedef char CarPartStateSizeCheck[sizeof(PartState) == 0x1a0 ? 1 : -1];
typedef char CarPartPrevMatrixCheck[offsetof(PartState, prevMatrix) == 0x4c ? 1 : -1];
typedef char CarPartDrawMatrixCheck[offsetof(PartState, drawMatrix) == 0x8c ? 1 : -1];
typedef char CarPartUpdateCheck[offsetof(PartState, update) == 0x110 ? 1 : -1];
typedef char CarPartFlagsCheck[offsetof(PartState, flags) == 0x150 ? 1 : -1];
typedef char CarPartStateTablesCheck[sizeof(CarPartStateTables) == 0x14 ? 1 : -1];
typedef char CarPartStateModesCheck[offsetof(CarPartStateTables, modes) == 0x10 ? 1 : -1];
typedef char CarDamageRecordSizeCheck[sizeof(CarDamageRecord) == 0x290 ? 1 : -1];
typedef char CarLineDescriptorSizeCheck[sizeof(CarFlexibleLineDescriptor) == 0x20 ? 1 : -1];
typedef char CarLineAxisCheck[offsetof(CarFlexibleLineDescriptor, axis) == 0xc ? 1 : -1];
typedef char CarLineLengthCheck[offsetof(CarFlexibleLineDescriptor, length) == 0x18 ? 1 : -1];
typedef char CarLineColourCheck[offsetof(CarFlexibleLineDescriptor, colour) == 0x1c ? 1 : -1];
typedef char CarLineStateSizeCheck[sizeof(CarFlexibleLineState) == 0x3c ? 1 : -1];
typedef char CarLineStateAllocationCheck[3 * sizeof(CarFlexibleLineState) == 0xb4 ? 1 : -1];
typedef char CarLinePreviousCheck[offsetof(CarFlexibleLineState, previousPosition) == 0xc ? 1 : -1];
typedef char CarLineDrawCheck[offsetof(CarFlexibleLineState, drawPosition) == 0x18 ? 1 : -1];
typedef char CarLineEndCheck[offsetof(CarFlexibleLineState, groundEnd) == 0x24 ? 1 : -1];
typedef char CarLineVelocityXCheck[offsetof(CarFlexibleLineState, velocityX) == 0x30 ? 1 : -1];
typedef char CarLineVelocityZCheck[offsetof(CarFlexibleLineState, velocityZ) == 0x34 ? 1 : -1];
typedef char CarLineGroundedCheck[offsetof(CarFlexibleLineState, grounded) == 0x38 ? 1 : -1];
typedef char SceneNodeSizeCheck[sizeof(SceneNode) == 0x18c ? 1 : -1];
typedef char SceneNodeViewMaskCheck[offsetof(SceneNode, viewMask) == 0x17c ? 1 : -1];
typedef char CarPartHiddenCheck[offsetof(CarPartSet, partHidden) == 0x4b0 ? 1 : -1];
typedef char CarLineGroundFlagsCheck[offsetof(CarPartSet, lineGrounded) == 0x4c0 ? 1 : -1];
typedef char CarPartRadiusCheck[offsetof(PartState, boundsRadius) == 0x15c ? 1 : -1];
typedef char CarDamageLinkSizeCheck[sizeof(CarDamageLink) == 0xd ? 1 : -1];
typedef char CarDamageLinkPoolSizeCheck[sizeof(CarDamageLinkPool) == 0x106 ? 1 : -1];
typedef char CarDamageLinkCountCheck[offsetof(CarDamageLinkPool, count) == 0x104 ? 1 : -1];
typedef char CarDamageFirstLinkCheck[offsetof(CarDamageLinkPool, firstLink) == 0x105 ? 1 : -1];
typedef char CarDamageSnapshotSizeCheck[sizeof(CarDamageSnapshot) == 0x40 ? 1 : -1];
typedef char CarDamageHiddenCheck[offsetof(CarDamageSnapshot, partHidden) == 0x24 ? 1 : -1];
typedef char CarDamageGroundFlagsCheck[offsetof(CarDamageSnapshot, lineGrounded) == 0x34 ? 1 : -1];
typedef char CarDamageStageImpactsCheck[offsetof(CarDamageRecord, stageImpacts) == 0x106 ? 1 : -1];
typedef char CarDamageActiveStateCheck[offsetof(CarDamageRecord, damage) == 0x20c ? 1 : -1];
typedef char CarDamageStageStateCheck[offsetof(CarDamageRecord, stageDamage) == 0x24c ? 1 : -1];
typedef char CarDamageStageCapturedCheck[offsetof(CarDamageRecord, stageSnapshotCaptured) == 0x28c ? 1 : -1];

// Motor, controls and gearbox fields recovered from readers and writers.
typedef char CarRecovered_collisionRadius[offsetof(Car, collisionRadius) == 0x758 ? 1 : -1];
typedef char CarRecovered_mass[offsetof(Car, mass) == 0x75c ? 1 : -1];
typedef char CarRecovered_inverseMass[offsetof(Car, inverseMass) == 0x760 ? 1 : -1];
typedef char CarRecovered_engineNetTorque[offsetof(Car, engineNetTorque) == 0x780 ? 1 : -1];
typedef char CarRecovered_engineDragCoefficient[offsetof(Car, engineDragCoefficient) == 0x784 ? 1 : -1];
typedef char CarRecovered_maxThrottleTorque[offsetof(Car, maxThrottleTorque) == 0x788 ? 1 : -1];
typedef char CarRecovered_baseThrottleTorque[offsetof(Car, baseThrottleTorque) == 0x78c ? 1 : -1];
typedef char CarRecovered_throttleRampStep[offsetof(Car, throttleRampStep) == 0x790 ? 1 : -1];
typedef char CarRecovered_engineSpeedLimit[offsetof(Car, engineSpeedLimit) == 0x794 ? 1 : -1];
typedef char CarRecovered_throttleTorque[offsetof(Car, throttleTorque) == 0x79c ? 1 : -1];
typedef char CarRecovered_throttlePhase[offsetof(Car, throttlePhase) == 0x7a0 ? 1 : -1];
typedef char CarRecovered_engineSpeed[offsetof(Car, engineSpeed) == 0x7a4 ? 1 : -1];
typedef char CarRecovered_revLimiterTorque[offsetof(Car, revLimiterTorque) == 0x7b0 ? 1 : -1];
typedef char CarRecovered_baseDriveSplit[offsetof(Car, baseDriveSplit) == 0x7b8 ? 1 : -1];
typedef char CarRecovered_gearRatio[offsetof(Car, gearRatio) == 0x7bc ? 1 : -1];
typedef char CarRecovered_steeringInput[offsetof(Car, steeringInput) == 0x818 ? 1 : -1];
typedef char CarRecovered_steeringTorqueScale[offsetof(Car, steeringTorqueScale) == 0x824 ? 1 : -1];
typedef char CarRecovered_steeringSpeedScale[offsetof(Car, steeringSpeedScale) == 0x828 ? 1 : -1];
typedef char CarRecovered_maxBrakeForce[offsetof(Car, maxBrakeForce) == 0x82c ? 1 : -1];
typedef char CarRecovered_brakeRampStep[offsetof(Car, brakeRampStep) == 0x834 ? 1 : -1];
typedef char CarRecovered_brakePhase[offsetof(Car, brakePhase) == 0x83c ? 1 : -1];
typedef char CarRecovered_handbrakeRampStep[offsetof(Car, handbrakeRampStep) == 0x840 ? 1 : -1];
typedef char CarRecovered_maxHandbrakeForce[offsetof(Car, maxHandbrakeForce) == 0x844 ? 1 : -1];
typedef char CarRecovered_handbrakePhase[offsetof(Car, handbrakePhase) == 0x84c ? 1 : -1];
typedef char CarRecovered_wheelSpinForWheelLean[offsetof(Car, wheelSpinForWheelLean) == 0x890 ? 1 : -1];
typedef char CarRecovered_wheelSpinForBodyLean[offsetof(Car, wheelSpinForBodyLean) == 0x8a0 ? 1 : -1];
typedef char CarRecovered_groundHeightCorrection[offsetof(Car, groundHeightCorrection) == 0x958 ? 1 : -1];
typedef char CarRecovered_wheelLeanDamping[offsetof(Car, wheelLeanDamping) == 0x9b8 ? 1 : -1];
typedef char CarRecovered_bodyLeanDamping[offsetof(Car, bodyLeanDamping) == 0x9bc ? 1 : -1];
typedef char CarRecovered_cheatWheelDrop[offsetof(Car, cheatWheelDrop) == 0xa88 ? 1 : -1];
typedef char CarRecovered_cheatBodyLift[offsetof(Car, cheatBodyLift) == 0xa8c ? 1 : -1];
typedef char CarRecovered_previousWheelSurface[offsetof(Car, previousWheelSurface) == 0xabe ? 1 : -1];
typedef char CarRecovered_engineStartTimer[offsetof(Car, engineStartTimer) == 0xafe ? 1 : -1];
typedef char CarRecovered_requestedGear[offsetof(Car, requestedGear) == 0xb20 ? 1 : -1];
typedef char CarRecovered_autoShiftDelay[offsetof(Car, autoShiftDelay) == 0xb21 ? 1 : -1];
typedef char CarRecovered_lastShiftDirection[offsetof(Car, lastShiftDirection) == 0xb22 ? 1 : -1];
typedef char CarRecovered_damageShiftDelay[offsetof(Car, damageShiftDelay) == 0xb24 ? 1 : -1];
typedef char CarRecovered_engineRestartPending[offsetof(Car, engineRestartPending) == 0xb4c ? 1 : -1];
typedef char CarRecovered_braking[offsetof(Car, braking) == 0xb54 ? 1 : -1];
typedef char CarRecovered_reversing[offsetof(Car, reversing) == 0xb5c ? 1 : -1];
typedef char CarRecovered_revLimiterActive[offsetof(Car, revLimiterActive) == 0xb78 ? 1 : -1];
typedef char CarRecovered_shiftInProgress[offsetof(Car, shiftInProgress) == 0xb84 ? 1 : -1];
typedef char CarRecovered_automaticReverse[offsetof(Car, automaticReverse) == 0xb94 ? 1 : -1];
typedef char CarRecovered_gearAtOrBelowBest[offsetof(Car, gearAtOrBelowBest) == 0xb98 ? 1 : -1];
typedef char CarRecovered_automaticGearbox[offsetof(Car, automaticGearbox) == 0xb9c ? 1 : -1];
typedef char CarRecovered_useUpperCollisionCorners[offsetof(Car, useUpperCollisionCorners) == 0xc00 ? 1 : -1];
typedef char CarRecovered_bodySize[offsetof(Car, bodySize) == 0x1f8 ? 1 : -1];
typedef char CarRecovered_upperCornersLocal[offsetof(Car, upperCornersLocal) == 0x240 ? 1 : -1];
typedef char CarRecovered_groundRightDot[offsetof(Car, groundRightDot) == 0x91c ? 1 : -1];
typedef char CarRecovered_groundUpDot[offsetof(Car, groundUpDot) == 0x920 ? 1 : -1];
typedef char CarRecovered_groundForwardDot[offsetof(Car, groundForwardDot) == 0x924 ? 1 : -1];
typedef char CarRecovered_deepestCorner[offsetof(Car, deepestCorner) == 0xb2b ? 1 : -1];
typedef char CarRecovered_firstCameraBlocked[offsetof(Car, firstCameraBlocked) == 0xb68 ? 1 : -1];
typedef char CarRecovered_steeringAccumulator[offsetof(Car, steeringAccumulator) == 0x81c ? 1 : -1];
typedef char CarRecovered_steeringReturnRate[offsetof(Car, steeringReturnRate) == 0x820 ? 1 : -1];
typedef char CarRecovered_wheelSteeringAngle[offsetof(Car, wheelSteeringAngle) == 0xb10 ? 1 : -1];
typedef char CarRecovered_targetSteeringAngle[offsetof(Car, targetSteeringAngle) == 0xb12 ? 1 : -1];
typedef char CarRecovered_renderSteeringAngle[offsetof(Car, renderSteeringAngle) == 0xb14 ? 1 : -1];
typedef char CarRecovered_maxSteeringAngleDegrees[offsetof(Car, maxSteeringAngleDegrees) == 0xb16 ? 1 : -1];
typedef char CarNetRecordSizeCheck[sizeof(CarNetRecord) == 0xec ? 1 : -1];
typedef char CarNetSteeringCheck[offsetof(CarNetRecord, wheelSteeringAngle) == 0xc4 ? 1 : -1];
typedef char CarNetThrottleCheck[offsetof(CarNetRecord, throttleTorque) == 0xb8 ? 1 : -1];
typedef char CarNetBrakingCheck[offsetof(CarNetRecord, braking) == 0xd0 ? 1 : -1];
typedef char CarNetUpperCornersCheck[offsetof(CarNetRecord, useUpperCollisionCorners) == 0xd4 ? 1 : -1];

// Typed render snapshots and the four 0x18-byte wheel records they contain.
typedef char CarWheelRecordSizeCheck[sizeof(CarWheelRecord) == 0x18 ? 1 : -1];
typedef char CarWheelTiltOffsetCheck[offsetof(CarWheelRecord, tiltDegrees) == 0xc ? 1 : -1];
typedef char CarWheelSteeringOffsetCheck[offsetof(CarWheelRecord, steeringDegrees) == 0x10 ? 1 : -1];
typedef char CarWheelSpinOffsetCheck[offsetof(CarWheelRecord, spinDegrees) == 0x14 ? 1 : -1];
typedef char CarTransformsSizeCheck[sizeof(CarTransforms) == 0xfc ? 1 : -1];
typedef char CarTransformsBody2Check[offsetof(CarTransforms, body2) == 0x40 ? 1 : -1];
typedef char CarTransformsWheelsCheck[offsetof(CarTransforms, wheels) == 0x80 ? 1 : -1];
typedef char CarTransformsNormalCheck[offsetof(CarTransforms, groundNormal) == 0xe0 ? 1 : -1];
typedef char CarTransformsHeightsCheck[offsetof(CarTransforms, cornerHeight) == 0xec ? 1 : -1];
typedef char CarRenderBoxStrideCheck[8 * sizeof(FixVector) == 0x60 ? 1 : -1];

typedef char CarSimulationInterpolationCheck[offsetof(Car, simulationInterpolation) == 0xa90 ? 1 : -1];
typedef char CarSimulationRateCheck[offsetof(Car, simulationRateHz) == 0xa98 ? 1 : -1];
typedef char CarSimulationStepsCheck[offsetof(Car, simulationStepsRemaining) == 0xb43 ? 1 : -1];

// Car contact patches, shadows and skid range flags.
typedef char CarContactSizeCheck[sizeof(CarContact) == 0x2a4 ? 1 : -1];
typedef char CarContactFrontMidCheck[offsetof(CarContact, wheelFrontMid) == 0x198 ? 1 : -1];
typedef char CarContactRearMidCheck[offsetof(CarContact, wheelRearMid) == 0x1c8 ? 1 : -1];
typedef char CarContactShadowCheck[offsetof(CarContact, shadowLevel) == 0x250 ? 1 : -1];
typedef char CarContactGhostCheck[offsetof(CarContact, ghostContact) == 0x294 ? 1 : -1];
typedef char CarContactWheelsCheck[offsetof(CarContact, wheelPatchesEnabled) == 0x298 ? 1 : -1];
typedef char CarContactSkidOffsetCheck[offsetof(CarContact, skidIndexOffset) == 0x29c ? 1 : -1];
typedef char CarContactSkidDirectionCheck[offsetof(CarContact, skidRangeAscending) == 0x2a0 ? 1 : -1];

// Scene records and the composite stage-car resource block.
typedef char CarSceneRecordSizeCheck[sizeof(CarSceneRecord) == 0x24 ? 1 : -1];
typedef char CarSceneRecord_bodyNodeCheck[offsetof(CarSceneRecord, bodyNode) == 0x4 ? 1 : -1];
typedef char CarSceneRecord_rootNodeCheck[offsetof(CarSceneRecord, rootNode) == 0x8 ? 1 : -1];
typedef char CarSceneRecord_alternateWheelSceneCheck[offsetof(CarSceneRecord, alternateWheelScene) == 0xc ? 1 : -1];
typedef char CarSceneRecord_originalWheelObjectsCheck[offsetof(CarSceneRecord, originalWheelObjects) == 0x10 ? 1 : -1];
typedef char CarSceneRecord_detailCodeCheck[offsetof(CarSceneRecord, detailCode) == 0x20 ? 1 : -1];
typedef char CarSceneRecord_variantCodeCheck[offsetof(CarSceneRecord, variantCode) == 0x21 ? 1 : -1];
typedef char g_carScenesCapacityCheck[sizeof(g_carScenes) / sizeof(g_carScenes[0]) == 16 ? 1 : -1];
typedef char g_carAuxiliaryBuffersCapacityCheck[sizeof(g_carAuxiliaryBuffers) / sizeof(g_carAuxiliaryBuffers[0]) == 16 ? 1 : -1];
typedef char g_carAlternateBodyScenesCapacityCheck[sizeof(g_carAlternateBodyScenes) / sizeof(g_carAlternateBodyScenes[0]) == 16 ? 1 : -1];
typedef char g_carWheelModelBuffersCapacityCheck[sizeof(g_carWheelModelBuffers) / sizeof(g_carWheelModelBuffers[0]) == 16 ? 1 : -1];
typedef char g_carAlternateBodyBuffersCapacityCheck[sizeof(g_carAlternateBodyBuffers) / sizeof(g_carAlternateBodyBuffers[0]) == 16 ? 1 : -1];
typedef char g_carModelBuffersCapacityCheck[sizeof(g_carModelBuffers) / sizeof(g_carModelBuffers[0]) == 16 ? 1 : -1];
typedef char g_carInfoBuffersCapacityCheck[sizeof(g_carInfoBuffers) / sizeof(g_carInfoBuffers[0]) == 8 ? 1 : -1];

// Scene binding and matrix identities established by model load and reload.
typedef char CarBinding_physicsMatrixCheck[offsetof(Car, physicsMatrix) == 0x0 ? 1 : -1];
typedef char CarBinding_bodyMatrixCheck[offsetof(Car, bodyMatrix) == 0x40 ? 1 : -1];
typedef char CarBinding_pSceneRootCheck[offsetof(Car, pSceneRoot) == 0x71c ? 1 : -1];
typedef char CarBinding_pBodyNodeCheck[offsetof(Car, pBodyNode) == 0x720 ? 1 : -1];
typedef char CarBinding_pAlternateBodyNodeCheck[offsetof(Car, pAlternateBodyNode) == 0x724 ? 1 : -1];

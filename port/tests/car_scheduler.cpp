// Contract tests call the original scheduler, rather than a rewritten model.
#include "port/types.h"
#include "Car.h"
#include "main.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern int Car_UpdateEngineNoteFalloff(int *);
extern BYTE g_carSimulationClockFlags;
extern BYTE g_unk0x0052af41;
extern int g_unk0x00538114;
static Car cars[3];

static void Check(bool condition, const char *message) {
    if (!condition) { fprintf(stderr, "scheduler: %s\n", message); exit(1); }
}
static void Reset(unsigned time = 1000) {
    memset(cars, 0, sizeof(cars));
    g_carBuffer = cars;
    g_carOrderCount = 3;
    g_carSimulationClockFlags = g_unk0x0052af41 = g_unk0x00538114 = 0;
    for (int i = 0; i < 3; ++i) {
        g_carOrder[i] = short(i);
        float rate = i == 2 ? 50.f : 25.f;
        cars[i].simulationRateHz = rate;
    }
    CMain::m_frameTime = int(time);
    int phase;
    Check(Car_UpdateEngineNoteFalloff(&phase) == 0 && phase == 0, "initial sample must rebase");
}
static int Sample(unsigned time) {
    CMain::m_frameTime = int(time);
    int phase;
    const int count = Car_UpdateEngineNoteFalloff(&phase);
    Check(phase >= -32768 && phase <= 32768, "original signed interpolation phase range");
    Check(cars[0].simulationStepsRemaining == cars[1].simulationStepsRemaining &&
          cars[0].simulationInterpolation == cars[1].simulationInterpolation, "same-rate car must copy first car");
    return count;
}
int main() {
    const unsigned rates[] = {30, 60, 120, 144, 240};
    for (unsigned fps : rates) {
        Reset();
        unsigned normal = 0, ghost = 0;
        for (unsigned frame = 1; frame <= fps * 10; ++frame) {
            Sample(1000 + frame * 1000 / fps);
            normal += cars[0].simulationStepsRemaining;
            ghost += cars[2].simulationStepsRemaining;
        }
        Check(normal == 250 && ghost == 500, "ten seconds must retain 25 Hz and stored 50 Hz rates");
    }
    Reset();
    Sample(1020);
    Check(cars[0].simulationStepsRemaining == 1 && cars[0].simulationInterpolation == -32768,
          "nearest conversion retains the original half-step and negative phase");
    Sample(1040);
    Check(cars[0].simulationStepsRemaining == 0, "a counted half-step must not be repeated");
    Reset();
    Sample(1100);
    Check(cars[0].simulationStepsRemaining == 3, "100 ms rounds forward to the third step");
    Sample(1116);
    Check(cars[0].simulationStepsRemaining == 0, "float/double boundary must not duplicate the third step");
    Reset();
    unsigned normal = 0, ghost = 0, elapsed = 0;
    const unsigned pattern[] = {7, 13, 33, 5, 22, 48, 9, 23};
    for (unsigned frame = 0; elapsed < 10000; ++frame) {
        elapsed += pattern[frame % 8];
        if (elapsed > 10000) elapsed = 10000;
        Sample(1000 + elapsed);
        normal += cars[0].simulationStepsRemaining;
        ghost += cars[2].simulationStepsRemaining;
    }
    Check(normal == 250 && ghost == 500, "variable cadence must retain tick totals");
    Reset();
    Check(Sample(1000) == 0, "zero elapsed time");
    Check(Sample(1120) == 5 && cars[0].simulationStepsRemaining == 3, "multiple ticks and mixed-rate cap");
    Reset();
    Check(Sample(2000) == 5 && cars[0].simulationStepsRemaining == 5, "five-step stall cap");
    Check(Sample(2000) == 0, "legacy cap discards simulation debt");
    Check(Sample(2040) == 2 && cars[0].simulationStepsRemaining == 1, "next ordinary frame after stall");
    Reset();
    g_unk0x0052af41 = 1;
    Check(Sample(2000) == 0 && g_unk0x0052af41 == 0, "pause/menu rebase");
    Check(Sample(2040) == 2 && cars[0].simulationStepsRemaining == 1, "resume after menu rebase");
    g_unk0x00538114 = 1;
    Check(Sample(4000) == 0 && g_unk0x00538114 == 0, "race transition rebase");
    Reset(0xfffffff0u);
    Check(Sample(0x18u) == 2 && cars[0].simulationStepsRemaining == 1, "32-bit millisecond wrap");
    puts("scheduler: fixed/variable cadence, mixed rates, zero/multiple steps, cap, rebase and wrap passed");
}

// Run the real menu/load/update path with a controlled clock and tick inputs.
// Original assets are required; run through tools/stage_baseline.py to isolate saves.
#include "port/types.h"
#include "platform/platform.h"
#include "platform/testing.h"
#include "port/diagnostics.h"
#include "diagnostics/timing.h"
#include "diagnostics/state_capture.h"
#include "port/sys.h"
#include "port/input.h"
#include "main.h"
#include "Game.h"
#include "GameInfo.h"
#include "FrontendMenus.h"
#include "Menu.h"
#include "Car.h"
#include "CarParts.h"
#include "RallyData.h"
#define SDL_MAIN_HANDLED
#include <SDL3/SDL_main.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

extern void Main_InitD3DX();
extern void Input_ClearKeyPressQueue();
extern int Args_Parse(char *);
extern Menu *FrontendMenu_GetActiveMenu();
extern Menu *g_pMenu0x00831778;
extern void RallyData_SetEditedDriverOrCategoryName(unsigned int, char *);
extern "C" unsigned int Crt_GetRandState();
extern BYTE *g_unk0x00588b94;
extern BYTE *g_unk0x00588b98;
extern int StageTiming_GetCheckpointGroupIndex(int);
extern int StageTiming_GetCheckpointSplitIndex(int);
extern BYTE StageTiming_GetCheckpointField19(int);
extern int StageTiming_GetCarSplitTime(int, int);
extern void *g_unk0x00592734;
extern unsigned char g_unk0x00542e78[];
extern unsigned char g_carStageTiming[];
extern int g_unk0x0053d1b0;

static DWORD virtualTime = 1000;
static DWORD Ticks() { return virtualTime; }
static void Sleep(DWORD ms) { virtualTime += ms; }

struct Probe : Diagnostics::TimingObserver {
    struct Controls {
        unsigned tick, state;
        int digital[4], handbrake, steering, pedal, shift[4];
    } input{};
    std::vector<Controls> playback;
    FILE *trace;
    FILE *memory = nullptr;
    unsigned tick = 0, drivingTick = 0, target;
    unsigned frameTickStart = 0, frameRng = 0, renderOnlyRngChanges = 0;
    bool scenarioCorrect = true;
    bool playbackCorrect = true;
    BYTE state = 0;
    explicit Probe(FILE *file, unsigned count) : trace(file), target(count) {
        if (const char *path = getenv("OPENCMR2_STATE_CAPTURE")) {
            memory = fopen(path, "wb");
            if (!memory) { perror(path); exit(2); }
            CaptureHeader(memory);
        }
    }
    ~Probe() {
        if (memory) { CaptureFinish(memory, tick); fclose(memory); }
    }
    bool LoadInputs(const char *path) {
        std::ifstream file(path);
        std::string line;
        if (!std::getline(file, line) || line != "tick,state,left,right,throttle,brake,handbrake,steering,pedal,shift0,shift1,shift2,shift3") return false;
        while (std::getline(file, line)) {
            for (char &ch : line) if (ch == ',') ch = ' ';
            std::istringstream row(line);
            Controls controls{};
            if (!(row >> controls.tick >> controls.state)) return false;
            for (int &v : controls.digital) if (!(row >> v) || v < 0 || v > 63) return false;
            if (!(row >> controls.handbrake >> controls.steering >> controls.pedal) || controls.handbrake < 0 || controls.handbrake > 1) return false;
            for (int &v : controls.shift) if (!(row >> v) || v < 0 || v > 255) return false;
            std::string extra;
            if (row >> extra || controls.tick != playback.size() || controls.state > 255 || playback.size() >= target) return false;
            playback.push_back(controls);
        }
        return file.eof() && playback.size() == target;
    }
    void BeginFrame() override {
        TimingObserver::BeginFrame();
        frameTickStart = tick;
        frameRng = Crt_GetRandState();
    }
    void EndFrame() override {
        TimingObserver::EndFrame();
        if (CGame::GetFrontendResourceMode() == 3 && tick == frameTickStart && Crt_GetRandState() != frameRng)
            ++renderOnlyRngChanges;
    }
    void BeginTick(unsigned char value) override {
        TimingObserver::BeginTick(value);
        state = value;
        if (tick == 0) {
            fprintf(stderr, "probe: country=%u stage=%u mode=%u cars=%d model=%d\n", RallyDataCountryIndex(), RallyDataStageIndex(), CGameInfo::GetConfiguredGameMode(), g_carOrderCount, Car_Get(0)->type);
            scenarioCorrect = RallyDataCountryIndex() == 0 && RallyDataStageIndex() == 0 && g_carOrderCount == 1;
        }
    }
    void CaptureMemory() {
        if (tick >= target) return;
        if (memory) {
            StateCaptureView view{};
            view.cars = reinterpret_cast<const unsigned char *>(g_carBuffer);
            view.parts = g_unk0x00588b94; view.damage = g_unk0x00588b98;
            view.contacts = static_cast<const unsigned char *>(g_unk0x00592734);
            view.timing = g_carStageTiming; view.checkpoints = g_unk0x00542e78;
            view.transforms = reinterpret_cast<const unsigned char *>(g_carTransforms);
            view.shadow = reinterpret_cast<const unsigned char *>(g_carTransformsShadow);
            view.wheels = reinterpret_cast<const unsigned char *>(g_carWheelTransforms);
            view.order = g_carOrder; view.count = g_carOrderCount;
            view.physicsStep = g_physicsTimeStep; view.physicsScale = g_physicsScale;
            view.stageClock = g_unk0x0053d1b0;
            StateCaptureInput controls{};
            controls.tick = input.tick; controls.state = input.state;
            memcpy(controls.digital, input.digital, sizeof(controls.digital));
            memcpy(controls.shift, input.shift, sizeof(controls.shift));
            controls.handbrake = input.handbrake; controls.steering = input.steering; controls.pedal = input.pedal;
            CaptureTick(memory, tick, state, Crt_GetRandState(), CMain::GetFrameTime(), drivingTick, controls, view);
        }
    }
    void ControlsReady() override {
        if (tick >= target) return;
        // Resolved digital controls, after the game's device mapping. No input
        // timing is derived from the number of drawn frames.
        Car &c = *Car_Get(0);
        if (!playback.empty()) {
            const Controls &controls = playback[tick];
            playbackCorrect = playbackCorrect && state == controls.state;
            for (int i = 0; i < 4; ++i) {
                c.flag0x1d0[i] = char(controls.digital[i]);
                c.field_0x1d4[i] = BYTE(controls.shift[i]);
            }
            c.handbrake = controls.handbrake;
            c.field_0x1dc = controls.steering;
            c.field_0x1e0 = controls.pedal;
        } else if (state == 8) {
            c.flag0x1d0[0] = drivingTick >= 100 && drivingTick < 125 ? 32 : 0;
            c.flag0x1d0[1] = drivingTick >= 150 && drivingTick < 175 ? 32 : 0;
            c.flag0x1d0[2] = drivingTick < 225 || drivingTick >= 275 ? 63 : 0;
            c.flag0x1d0[3] = drivingTick >= 225 && drivingTick < 275 ? 63 : 0;
            c.handbrake = drivingTick >= 300 && drivingTick < 310;
            c.field_0x1dc = c.field_0x1e0 = 0;
            memset(c.field_0x1d4, 0, sizeof(c.field_0x1d4));
        }
        input.tick = tick; input.state = state;
        for (int i = 0; i < 4; ++i) {
            input.digital[i] = c.flag0x1d0[i];
            input.shift[i] = c.field_0x1d4[i];
        }
        input.handbrake = c.handbrake; input.steering = c.field_0x1dc; input.pedal = c.field_0x1e0;
        if (state == 8) ++drivingTick;
    }
    void Vector(const char *name, const FixVector &v) {
        fprintf(trace, ",\"%s\":[%d,%d,%d]", name, v.x, v.y, v.z);
    }
    template<class T, size_t N> void Array(const char *name, const T (&v)[N]) {
        fprintf(trace, ",\"%s\":[", name);
        for (size_t i = 0; i < N; ++i) fprintf(trace, "%s%d", i ? "," : "", int(v[i]));
        fputc(']', trace);
    }
    void EndTick() override {
        TimingObserver::EndTick();
        if (tick >= target) return;
        CaptureMemory();
        fprintf(trace, "{\"tick\":%u,\"state\":%u,\"driving_tick\":%u,\"rng\":%u,\"input\":{\"digital\":[%d,%d,%d,%d],\"handbrake\":%d,\"steering\":%d,\"pedal\":%d,\"shift\":[%d,%d,%d,%d]},\"cars\":[",
                tick, state, drivingTick, Crt_GetRandState(), input.digital[0], input.digital[1], input.digital[2], input.digital[3], input.handbrake, input.steering, input.pedal,
                input.shift[0], input.shift[1], input.shift[2], input.shift[3]);
        for (int i = 0; i < g_carOrderCount; ++i) {
            const Car &c = g_carBuffer[g_carOrder[i]];
            fprintf(trace, "%s{\"id\":%d,\"type\":%d,\"gear\":%d,\"speed\":%d,\"sector\":%d,\"heading\":%u",
                    i ? "," : "", g_carOrder[i], c.type, c.gear, c.speed, c.sector, c.heading);
            Vector("position", c.position); Vector("position_prev", c.positionPrev);
            Vector("velocity", c.velocity); Vector("velocity_next", c.velocityNext);
            Vector("angular_velocity", c.angularVelocity);
            Vector("right", c.right); Vector("up", c.up); Vector("forward", c.forward);
            Vector("ground_normal", c.groundNormal);
            Array("controls", c.flag0x1d0); Array("contacts", c.cornerOnGround);
            Array("contact_triangles", c.cornerTriangle); Array("surface", c.wheelSurfaceType);
            Array("wheel_load", c.wheelLoad); Array("wheel_torque", c.wheelTorque);
            Array("wheel_slip", c.wheelSlip); Array("wheel_slip_lateral", c.wheelSlipLateral);
            Array("suspension", c.wheel0x988); Array("suspension_body", c.wheel0x9a8);
            Array("corner_height", c.cornerHeight); Array("disabled_corners", c.cornerFlags);
            if (g_unk0x00588b94) {
                const CarPartSet &parts = reinterpret_cast<const CarPartSet *>(g_unk0x00588b94)[int(c.index)];
                Array("damage_front", parts.damageGrid[0]); Array("damage_middle", parts.damageGrid[1]); Array("damage_rear", parts.damageGrid[2]);
                Array("damage_parts", parts.field_0x240);
            }
            if (g_unk0x00588b98) {
                const CarDamageRecord &damage = reinterpret_cast<const CarDamageRecord *>(g_unk0x00588b98)[int(c.index)];
                Array("damage_intensity", damage.intensity);
            }
            fprintf(trace, ",\"checkpoint_group\":%d,\"checkpoint_split\":%d,\"finish\":%u,\"split_times\":[",
                    StageTiming_GetCheckpointGroupIndex(c.index), StageTiming_GetCheckpointSplitIndex(c.index), StageTiming_GetCheckpointField19(c.index));
            for (int split = 0; split < 12; ++split) fprintf(trace, "%s%d", split ? "," : "", StageTiming_GetCarSplitTime(c.index, split));
            fputc(']', trace);
            fprintf(trace, ",\"handbrake\":%d,\"steering_filter\":%d,\"steering_torque\":%d,\"brake\":%d,\"engine\":%d,\"shift_delay\":%d,\"auto_gearbox\":%d}",
                    c.handbrake, c.field_0x818, c.field_0x824, c.brakeInput, c.field_0x7a4, c.field_0xb21, c.field_0xb9c);
        }
        fputs("]}\n", trace);
        ++tick;
    }
};

static void ConfigureMenu(Menu *menu) {
    if (!menu) return;
    if (menu == FrontendMenu_GetMain()) menu->cursor = 1;
    else if (menu == FrontendMenu_GetRallyModes()) menu->cursor = 2;
    else if (menu == FrontendMenu_GetTimeTrialDifficulty()) { menu->cursor = 0; menu->items[0].max = 0; }
    else if (menu == FrontendMenu_GetRallySelection()) menu->cursor = 0;
    else if (menu == FrontendMenu_GetRallyStageSelection()) menu->cursor = 0;
    else if (menu == FrontendMenu_GetAlternateRallyStageSelection()) menu->cursor = 0;
    else if (menu == FrontendMenu_GetProfileNameEntry()) {
        char name[] = "TST";
        RallyData_SetEditedDriverOrCategoryName(FrontendProfile_GetCurrentPlayer(), name);
        menu->cursor = 2;
        menu->items[2].max = 9;
    }
}

int main(int argc, char **argv) {
    // Keep probe arguments out of the legacy command-line parser.
    if (argc != 5 && argc != 6) { fprintf(stderr, "usage: stage_probe DATA TRACE.jsonl FPS|variable TICKS [INPUTS.csv]\n"); return 2; }
    if (!getenv("XDG_DATA_HOME")) {
        fprintf(stderr, "Set XDG_DATA_HOME to a disposable directory, or use tools/stage_baseline.py\n");
        return 2;
    }
    const bool variable = strcmp(argv[3], "variable") == 0;
    const unsigned fps = variable ? 60 : unsigned(strtoul(argv[3], nullptr, 10));
    const unsigned ticks = unsigned(strtoul(argv[4], nullptr, 10));
    if (fps < 10 || fps > 1000 || !ticks || ticks > 100000) return 2;
    FILE *file = fopen(argv[2], "w");
    if (!file) { perror(argv[2]); return 2; }
    Probe probe(file, ticks);
    if (argc == 6 && !probe.LoadInputs(argv[5])) {
        fprintf(stderr, "Invalid playback file: %s (requires exactly %u ordered control records)\n", argv[5], ticks);
        fclose(file);
        return 2;
    }
    SDL_SetMainReady();
    char dataArg[] = "--data";
    char *platformArgs[] = {argv[0], dataArg, argv[1]};
    if (!Platform_Init(3, platformArgs)) { fclose(file); return 1; }
    const PlatformTestClock clock{Ticks, Sleep, 1700000000};
    BYTE keys[256]{};
    Platform_SetTestClock(&clock);
    Platform_SetTestKeyboard(keys);
    Main_InitD3DX();
    CGame::RegisterNetworkResourceRelease();
    Input_ClearKeyPressQueue();
    char command[] = "";
    Args_Parse(command);
    if (!CMain::CreateGameWindow("OpenCMR2 stage probe")) return 1;
    Diagnostics::observer = &probe;
    DWORD raceOrigin = 0;
    unsigned raceFrame = 0;
    Menu *lastMenu = nullptr;
    int lastMode = -1;
    for (unsigned frame = 0; frame < 200000 && probe.tick < ticks && !CGame::m_shouldExit; ++frame) {
        SDL_PumpEvents();
        // The dummy driver does not focus windows. This test dispatches frames
        // explicitly, so SDL focus and wall time cannot influence its clock.
        const int mode = CGame::GetFrontendResourceMode();
        if (mode == 3 && !raceOrigin && getenv("OPENCMR2_REFERENCE_CLOCK")) {
            virtualTime = 100000;
            CMain::UpdateFrameTime();
            raceOrigin = virtualTime;
        }
        if (mode != lastMode) { fprintf(stderr, "probe: resource mode %d at %u ms\n", mode, virtualTime); lastMode = mode; }
        memset(keys, 0, sizeof(keys));
        if (mode != 3) {
            Menu *menu = mode == 2 ? g_pMenu0x00831778 : FrontendMenu_GetActiveMenu();
            if (menu && menu != lastMenu) {
                fprintf(stderr, "probe: menu %d (%d entries)\n", menu->field_0x4, menu->itemCount);
                lastMenu = menu;
            }
            if (mode == 2 && menu && menu->itemCount >= 4) menu->cursor = 3;
            else ConfigureMenu(menu);
            if (frame % 12 == 6 || !menu) {
                keys[INPUT_KEY_RETURN] = 0x80;
                // Blocking movie playback consumes SDL events itself.
                SDL_Event event{};
                event.type = SDL_EVENT_KEY_DOWN;
                event.key.scancode = SDL_SCANCODE_RETURN;
                SDL_PushEvent(&event);
            }
        }
        CGame::UpdateActiveSoundSlots();
        probe.BeginFrame();
        CGame::DispatchFrontendResourceState();
        probe.EndFrame();
        if (mode == 3) {
            if (!raceOrigin) raceOrigin = virtualTime;
            ++raceFrame;
            if (variable) {
                const DWORD pattern[] = {7, 13, 33, 5, 22, 48, 9, 23};
                virtualTime += pattern[(raceFrame - 1) % 8];
            } else virtualTime = raceOrigin + DWORD(uint64_t(raceFrame) * 1000 / fps);
        } else virtualTime += 33;
    }
    Diagnostics::observer = nullptr;
    if (!probe.Write((std::string(argv[2]) + ".frames.csv").c_str())) return 1;
    const bool ok = probe.tick == ticks && probe.drivingTick > 0 && probe.scenarioCorrect && probe.playbackCorrect && !ferror(file);
    fprintf(stderr, "probe: RNG changed in %u race frames without a simulation step\n", probe.renderOnlyRngChanges);
    fprintf(stderr, "probe: %u ticks, %u driving ticks, %u ms; %s\n", probe.tick, probe.drivingTick, virtualTime, ok ? "complete" : "FAILED");
    CMain::UnwindGameCallbacks();
    Platform_SetTestKeyboard(nullptr);
    Platform_SetTestClock(nullptr);
    fclose(file);
    Platform_Shutdown();
    return ok ? 0 : 1;
}

// MSVC6 recorder/proxy for the original executable with real SilentPatch.
// Detours observe the existing whole-tick boundary; they never replace physics.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/diagnostics/state_capture.h"

struct Symbol { char name[64]; unsigned address, size; unsigned char bytes[32]; };
static Symbol symbols[128];
static unsigned symbolCount;
static HMODULE silentPatch;
static FILE *logFile, *capture;
static unsigned now = 1000, frame, tick, drivingTick, target = 500, fps = 60, rng = 1;
static unsigned raceOrigin, raceFrame, raceState;
static int inRace, controlsReady;
static int clockEnabled;
static DWORD (WINAPI *realClock)();
static BYTE *textBefore, *textStart;
static unsigned textSize, patchChecked;
static StateCaptureInput input;
static BYTE keys[256];
typedef int (WINAPI *NoArgs)();
static NoArgs realDispatch;
static void (WINAPI *realRace)(BYTE, int);
static void (WINAPI *realCars)();
static void (WINAPI *realDecrement)(short *, short);
static int (__cdecl *realRand)();
static void (__cdecl *realSrand)(unsigned);

static void Fail(const char *message) {
    if (logFile) { fprintf(logFile, "ERROR: %s (%lu)\n", message, GetLastError()); fflush(logFile); }
    ExitProcess(2);
}
static Symbol &Find(const char *name) {
    for (unsigned i = 0; i < symbolCount; ++i) if (!strcmp(symbols[i].name, name)) return symbols[i];
    Fail(name); return symbols[0];
}
static unsigned Address(const char *name) { return Find(name).address; }
static unsigned Read(const char *name) { return *(unsigned *)Address(name); }
static int Call(const char *name) { return ((NoArgs)Address(name))(); }
static void ReadConfiguration() {
    FILE *file = fopen("capture.map", "r");
    char line[512], hex[128];
    if (!file) Fail("capture.map missing");
    while (fgets(line, sizeof(line), file)) {
        if (symbolCount == 128) Fail("symbol limit");
        Symbol &s = symbols[symbolCount];
        s.size = 0; hex[0] = 0;
        if (sscanf(line, "%63s %x %u %127s", s.name, &s.address, &s.size, hex) < 2) continue;
        if (s.size > sizeof(s.bytes) || (s.size && strlen(hex) != s.size * 2)) Fail("invalid detour signature");
        for (unsigned i = 0; i < s.size; ++i) {
            unsigned byte;
            if (sscanf(hex + i * 2, "%2x", &byte) != 1) Fail("invalid signature byte");
            s.bytes[i] = (BYTE)byte;
        }
        ++symbolCount;
    }
    fclose(file);
}
static void *Detour(const char *name, void *replacement) {
    Symbol &s = Find(name);
    if (s.size < 5 || memcmp((void *)s.address, s.bytes, s.size)) Fail("detour signature mismatch (build/patch mismatch)");
    BYTE *trampoline = (BYTE *)VirtualAlloc(NULL, s.size + 5, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!trampoline) Fail("trampoline allocation");
    memcpy(trampoline, s.bytes, s.size);
    // Generator permits only a first rel32 CALL/JMP or position-independent
    // instructions in the displaced prologue, and rejects short branches.
    if (s.bytes[0] == 0xe8 || s.bytes[0] == 0xe9)
        *(unsigned *)(trampoline + 1) = s.address + 5 + *(unsigned *)(s.bytes + 1) - (unsigned)trampoline - 5;
    trampoline[s.size] = 0xe9;
    *(unsigned *)(trampoline + s.size + 1) = s.address + s.size - (unsigned)(trampoline + s.size) - 5;
    DWORD old;
    if (!VirtualProtect((void *)s.address, s.size, PAGE_EXECUTE_READWRITE, &old)) Fail("detour protection");
    memset((void *)s.address, 0x90, s.size);
    *(BYTE *)s.address = 0xe9;
    *(unsigned *)(s.address + 1) = (unsigned)replacement - s.address - 5;
    VirtualProtect((void *)s.address, s.size, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void *)s.address, s.size);
    fprintf(logFile, "observe %s %08x (%u bytes)\n", name, s.address, s.size); fflush(logFile);
    return trampoline;
}
static void *Import(const char *dll, const char *name, void *replacement) {
    BYTE *base = (BYTE *)GetModuleHandleA(NULL);
    IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(base + ((IMAGE_DOS_HEADER *)base)->e_lfanew);
    IMAGE_DATA_DIRECTORY dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    IMAGE_IMPORT_DESCRIPTOR *imp = (IMAGE_IMPORT_DESCRIPTOR *)(base + dir.VirtualAddress);
    for (; dir.VirtualAddress && imp->Name; ++imp) {
        if (lstrcmpiA((char *)base + imp->Name, dll)) continue;
        IMAGE_THUNK_DATA *names = (IMAGE_THUNK_DATA *)(base + imp->OriginalFirstThunk);
        IMAGE_THUNK_DATA *iat = (IMAGE_THUNK_DATA *)(base + imp->FirstThunk);
        for (; names->u1.AddressOfData; ++names, ++iat) {
            if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal)) continue;
            if (strcmp((char *)((IMAGE_IMPORT_BY_NAME *)(base + (DWORD)names->u1.AddressOfData))->Name, name)) continue;
            void *previous = (void *)iat->u1.Function;
            DWORD old;
            if (!VirtualProtect(&iat->u1.Function, 4, PAGE_READWRITE, &old)) Fail("import protection");
            *(void **)&iat->u1.Function = replacement;
            VirtualProtect(&iat->u1.Function, 4, old, &old);
            return previous;
        }
    }
    Fail(name); return NULL;
}
static DWORD WINAPI Clock() { return clockEnabled ? now : realClock(); }
static int WINAPI Message(HWND, LPCSTR message, LPCSTR, UINT type) {
    fprintf(logFile, "message: %s\n", message); fflush(logFile);
    if (!strcmp(message, "Setting configuration to defaults") && (type & 15) == MB_OK) return IDOK;
    Fail("unexpected modal message"); return IDCANCEL;
}
static long __cdecl UnixTime(long *out) { if (out) *out = 1700000000; return 1700000000; }
// Intro movies use their own real-time wait while the dispatcher is blocked.
// Decode them normally, but remove that wait in this automated fixture.
static int WINAPI MovieWait(void *) { return 0; }
static int __cdecl Random() {
    rng = rng * 214013u + 2531011u;
    int result = realRand();
    if (result != (int)((rng >> 16) & 32767)) Fail("CRT RNG differs from tracked MSVC sequence");
    return result;
}
static void __cdecl Seed(unsigned value) { rng = value; realSrand(value); }
static void WINAPI Keyboard() { memcpy((void *)Address("keyboard"), keys, sizeof(keys)); }
static SHORT WINAPI AsyncKey(int key) { return key == VK_RETURN && keys[0x1c] ? (SHORT)0x8000 : 0; }
static BOOL CALLBACK SendKeys(HWND window, LPARAM) {
    if (keys[0x1c]) PostMessageA(window, WM_KEYDOWN, VK_RETURN, 0x001c0001);
    else PostMessageA(window, WM_KEYUP, VK_RETURN, 0xc01c0001);
    return TRUE;
}
static void VerifyPatch() {
    if (patchChecked++) return;
    unsigned changed = 0;
    for (unsigned i = 0; i < textSize; ++i) {
        if (textBefore[i] == textStart[i]) continue;
        unsigned address = (unsigned)textStart + i;
        int observed = 0;
        for (unsigned s = 0; s < symbolCount; ++s)
            if (symbols[s].size && address >= symbols[s].address && address < symbols[s].address + symbols[s].size) observed = 1;
        if (!observed) ++changed;
    }
    fprintf(logFile, "SilentPatch changed %u code bytes (observer detours excluded)\n", changed); fflush(logFile);
    if (silentPatch && !changed) Fail("SilentPatch loaded but no code patches applied");
}

static BYTE *Menu(const char *name) { return (BYTE *)Call(name); }
static void ConfigureMenu(BYTE *menu) {
    if (!menu) return;
    if (menu == Menu("menu_main")) menu[7] = 1;
    else if (menu == Menu("menu_modes")) menu[7] = 2;
    else if (menu == Menu("menu_difficulty")) { menu[7] = 0; menu[0x1f] = 0; }
    else if (menu == Menu("menu_country") || menu == Menu("menu_stage") || menu == Menu("menu_alternate")) menu[7] = 0;
    else if (menu == Menu("menu_name")) {
        ((void (WINAPI *)(unsigned, char *))Address("set_name"))(Call("profile_slot"), "TST");
        menu[7] = 2; menu[0x1f + 2 * 20] = 9;
    }
}
static void WINAPI Race(BYTE state, int value) {
    raceState = state; inRace = 1;
    realRace(state, value);
    inRace = 0;
}
static void WINAPI Cars() {
    if (inRace && tick < target) {
        BYTE *car = (BYTE *)Read("cars");
        if (!car) Fail("car missing at simulation step");
        if (raceState == 8) {
            car[0x1d0] = drivingTick >= 100 && drivingTick < 125 ? 32 : 0;
            car[0x1d1] = drivingTick >= 150 && drivingTick < 175 ? 32 : 0;
            car[0x1d2] = drivingTick < 225 || drivingTick >= 275 ? 63 : 0;
            car[0x1d3] = drivingTick >= 225 && drivingTick < 275 ? 63 : 0;
            *(int *)(car + 0x1d8) = drivingTick >= 300 && drivingTick < 310;
            *(int *)(car + 0x1dc) = *(int *)(car + 0x1e0) = 0;
            memset(car + 0x1d4, 0, 4);
            ++drivingTick;
        }
        input.tick = tick; input.state = raceState;
        for (unsigned i = 0; i < 4; ++i) { input.digital[i] = car[0x1d0+i]; input.shift[i] = car[0x1d4+i]; }
        input.handbrake = *(int *)(car+0x1d8); input.steering = *(int *)(car+0x1dc); input.pedal = *(int *)(car+0x1e0);
        controlsReady = 1;
    }
    realCars();
}
static void WINAPI Decrement(short *order, short count) {
    realDecrement(order, count);
    if (!inRace || !controlsReady || tick >= target) return;
    controlsReady = 0;
    if (!tick) {
        unsigned short control;
        __asm fnstcw control
        fprintf(logFile, "race x87 control=%04x\n", control); fflush(logFile);
        FILE *tables = fopen("math-tables.bin", "wb");
        if (!tables) Fail("cannot record math tables");
        fwrite((void *)Address("sqrt_table"), 2, 4096, tables);
        fwrite((void *)Address("sin_table"), 4, 4096, tables);
        fwrite((void *)Address("acos_table"), 2, 4096, tables);
        fwrite((void *)Address("atan_table"), 2, 512, tables);
        fwrite((void *)Address("tan_table"), 4, 4096, tables);
        fclose(tables);
    }
    StateCaptureView view;
    memset(&view, 0, sizeof(view));
    view.cars = (BYTE *)Read("cars"); view.parts = (BYTE *)Read("parts"); view.damage = (BYTE *)Read("damage");
    view.contacts = (BYTE *)Read("contacts"); view.timing = (BYTE *)Address("timing"); view.checkpoints = (BYTE *)Address("checkpoints");
    view.transforms = (BYTE *)Address("transforms"); view.shadow = (BYTE *)Address("shadow"); view.wheels = (BYTE *)Address("wheels");
    view.order = (short *)Address("order"); view.count = *(short *)Address("order_count");
    view.physicsStep = *(int *)Address("physics_step"); view.physicsScale = *(int *)Address("physics_scale"); view.stageClock = *(int *)Address("stage_clock");
    if (!view.count || view.count > 8) Fail("unsupported car order count");
    if (!tick) { fprintf(logFile, "scenario country=%d stage=%d count=%u\n", Call("country"), Call("stage"), view.count); fflush(logFile); }
    CaptureTick(capture, tick, raceState, rng, Read("frame_time"), drivingTick, input, view);
    ++tick;
    if (ferror(capture)) Fail("capture write");
    if (tick == target) {
        CaptureFinish(capture, tick); if (fclose(capture)) Fail("capture close"); capture = NULL;
        fprintf(logFile, "COMPLETE %u ticks (%u driving)\n", tick, drivingTick); fflush(logFile);
        // End this isolated process after the last captured tick; avoids any
        // configuration writes during game shutdown in the asset directory.
        ExitProcess(0);
    }
}
static int WINAPI Dispatch() {
    VerifyPatch();
    int mode = Call("mode");
    memset(keys, 0, sizeof(keys));
    if (mode != 3) {
        BYTE *menu = mode == 2 ? (BYTE *)Read("garage_menu") : Menu("active_menu");
        if (mode == 2 && menu && menu[6] >= 4) menu[7] = 3; else ConfigureMenu(menu);
        if (frame % 12 == 6 || !menu) keys[0x1c] = 0x80;
        if (frame % 60 == 0) { fprintf(logFile, "frame=%u mode=%d time=%u menu=%d\n", frame, mode, now, menu ? *(short *)(menu+4) : -999); fflush(logFile); }
    } else if (!raceOrigin) {
        raceOrigin = now = 100000;
        *(unsigned *)Address("frame_time") = now;
    }
    EnumThreadWindows(GetCurrentThreadId(), SendKeys, 0);
    int result = realDispatch();
    if (!clockEnabled) {
        clockEnabled = 1;
        *(unsigned *)Address("frame_time") = now;
    }
    ++frame;
    if (mode == 3) now = raceOrigin + ++raceFrame * 1000 / fps; else now += 33;
    if (frame > 20000) Fail("stage navigation timeout");
    return result;
}

extern "C" BOOL WINAPI CaptureEntry(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason != DLL_PROCESS_ATTACH) return TRUE;
    DisableThreadLibraryCalls(instance);
    logFile = fopen("capture.log", "w");
    ReadConfiguration();
    if (getenv("CMR_CAPTURE_TICKS")) target = atoi(getenv("CMR_CAPTURE_TICKS"));
    if (getenv("CMR_CAPTURE_FPS")) fps = atoi(getenv("CMR_CAPTURE_FPS"));
    if (!target || target > 100000 || fps < 10 || fps > 1000) Fail("invalid capture parameters");
    BYTE *base = (BYTE *)GetModuleHandleA(NULL);
    IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(base + ((IMAGE_DOS_HEADER *)base)->e_lfanew);
    textStart = base + nt->OptionalHeader.BaseOfCode; textSize = nt->OptionalHeader.SizeOfCode;
    textBefore = (BYTE *)VirtualAlloc(NULL, textSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!textBefore) Fail("code snapshot allocation");
    memcpy(textBefore, textStart, textSize);
    silentPatch = LoadLibraryA("SPCMR2_sp.dll");
    if (!silentPatch && !getenv("CMR_CAPTURE_DECOMP")) Fail("SilentPatch is required for the original reference");
    fprintf(logFile, "SilentPatch module=%08x\n", (unsigned)silentPatch); fflush(logFile);
    capture = fopen("state.bin", "wb"); if (!capture) Fail("cannot open state.bin"); CaptureHeader(capture);
    realClock = (DWORD (WINAPI *)())Import("WINMM.dll", "timeGetTime", (void *)Clock);
    Import("USER32.dll", "MessageBoxA", (void *)Message);
    Import("binkw32.dll", "_BinkWait@4", (void *)MovieWait);
    Import("MSVCRT.dll", "time", (void *)UnixTime);
    realRand = (int (__cdecl *)())Import("MSVCRT.dll", "rand", (void *)Random);
    realSrand = (void (__cdecl *)(unsigned))Import("MSVCRT.dll", "srand", (void *)Seed);
    Import("USER32.dll", "GetAsyncKeyState", (void *)AsyncKey);
    realDispatch = (NoArgs)Detour("dispatch", (void *)Dispatch);
    realRace = (void (WINAPI *)(BYTE,int))Detour("race", (void *)Race);
    realCars = (void (WINAPI *)())Detour("car_step", (void *)Cars);
    realDecrement = (void (WINAPI *)(short *,short))Detour("end_tick", (void *)Decrement);
    Detour("read_keyboard", (void *)Keyboard);
    return TRUE;
}
extern "C" HRESULT WINAPI CaptureSound(GUID *guid, void **out, void *outer) {
    HMODULE module = silentPatch ? silentPatch : LoadLibraryA("dsound.dll");
    typedef HRESULT (WINAPI *Sound)(GUID *,void **,void *);
    Sound function = (Sound)GetProcAddress(module, (LPCSTR)1);
    if (!function) Fail("DirectSoundCreate export");
    return function(guid,out,outer);
}

#include "diagnostics/timing.h"
#include <SDL3/SDL.h>
#include <cstdio>

namespace Diagnostics {
static TimingObserver profile;
static const char *profilePath = nullptr;
void TimingObserver::BeginFrame() {
    frameStart = SDL_GetTicksNS();
    current = {};
    if (previousStart) current.interval = frameStart - previousStart;
    previousStart = frameStart;
}
void TimingObserver::EndFrame() {
    current.work = SDL_GetTicksNS() - frameStart;
    if (frames.size() < maximumFrames) frames.push_back(current);
}
void TimingObserver::BeginTick(unsigned char) { tickStart = SDL_GetTicksNS(); }
void TimingObserver::EndTick() {
    current.ticks += SDL_GetTicksNS() - tickStart;
    ++current.tickCount;
}
void TimingObserver::BeginPresent() { presentStart = SDL_GetTicksNS(); }
void TimingObserver::EndPresent() { current.present += SDL_GetTicksNS() - presentStart; }
bool TimingObserver::Write(const char *path) const {
    FILE *file = fopen(path, "w");
    if (!file) return false;
    fputs("frame,interval_ns,work_ns,tick_ns,present_ns,tick_count\n", file);
    for (size_t i = 0; i < frames.size(); ++i) {
        const Frame &f = frames[i];
        fprintf(file, "%zu,%llu,%llu,%llu,%llu,%u\n", i,
                static_cast<unsigned long long>(f.interval), static_cast<unsigned long long>(f.work),
                static_cast<unsigned long long>(f.ticks), static_cast<unsigned long long>(f.present), f.tickCount);
    }
    const bool ok = !ferror(file);
    return fclose(file) == 0 && ok;
}
void StartProfileFromEnvironment() {
    profilePath = SDL_getenv("OPENCMR2_PROFILE");
    if (profilePath && *profilePath && !observer) {
        observer = &profile;
        fprintf(stderr, "CPU profile enabled: %s (up to 1000000 frames)\n", profilePath);
    } else profilePath = nullptr;
}
void StopProfile() {
    if (!profilePath) return;
    observer = nullptr;
    fprintf(stderr, "CPU profile %s: %s\n", profile.Write(profilePath) ? "saved" : "FAILED", profilePath);
    profilePath = nullptr;
}
}

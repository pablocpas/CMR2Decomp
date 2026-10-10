#ifndef OPENCMR2_DIAGNOSTICS_TIMING_H
#define OPENCMR2_DIAGNOSTICS_TIMING_H
#include "port/diagnostics.h"
#include <cstdint>
#include <vector>

namespace Diagnostics {
// CPU timings use SDL's real monotonic clock, even during fake-clock tests.
// Present measures the CPU executor call including waits, not GPU execution.
class TimingObserver : public Observer {
public:
    struct Frame {
        uint64_t interval = 0, work = 0, ticks = 0, present = 0;
        unsigned tickCount = 0;
    };
    void BeginFrame() override;
    void EndFrame() override;
    void BeginTick(unsigned char) override;
    void EndTick() override;
    void BeginPresent() override;
    void EndPresent() override;
    bool Write(const char *path) const;
private:
    uint64_t frameStart = 0, previousStart = 0, tickStart = 0, presentStart = 0;
    Frame current;
    std::vector<Frame> frames;
    static constexpr size_t maximumFrames = 1000000;
};
void StartProfileFromEnvironment();
void StopProfile();
}
#endif

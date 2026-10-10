/* Optional observation of the existing update loop; no scheduler replacement. */
#ifndef OPENCMR2_PORT_DIAGNOSTICS_H
#define OPENCMR2_PORT_DIAGNOSTICS_H

namespace Diagnostics {
struct Observer {
    virtual ~Observer() = default;
    virtual void BeginTick(unsigned char state) {}
    virtual void ControlsReady() {}
    virtual void EndTick() {}
    virtual void BeginFrame() {}
    virtual void EndFrame() {}
    virtual void BeginPresent() {}
    virtual void EndPresent() {}
};
extern Observer *observer;
}
#endif

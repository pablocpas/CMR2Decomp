// Binary observation format shared with the MSVC6 Windows reference recorder.
// Raw bytes are retained for investigation; comparison must use the schema and
// exclude addresses/padding. No game structures or simulation functions here.
#ifndef OPENCMR2_STATE_CAPTURE_H
#define OPENCMR2_STATE_CAPTURE_H
#include <stdio.h>
#include <string.h>

struct StateCaptureView {
    const unsigned char *cars, *parts, *damage, *contacts, *timing, *checkpoints;
    const unsigned char *transforms, *shadow, *wheels;
    const short *order;
    unsigned int count;
    int physicsStep, physicsScale, stageClock;
};
struct StateCaptureInput {
    unsigned int tick, state;
    int digital[4], handbrake, steering, pedal, shift[4];
};
inline void CaptureWord(FILE *file, unsigned int value) { fwrite(&value, 4, 1, file); }
inline void CaptureBlock(FILE *file, const char *name, const void *data, unsigned int size) {
    CaptureWord(file, (unsigned int)strlen(name));
    fwrite(name, strlen(name), 1, file);
    CaptureWord(file, data ? size : 0);
    if (data && size) fwrite(data, size, 1, file);
}
inline void CaptureHeader(FILE *file) { fwrite("CMRSTAT1", 8, 1, file); }
inline void CaptureTick(FILE *file, unsigned int tick, unsigned int state,
                        unsigned int rng, unsigned int time, unsigned int drivingTick,
                        const StateCaptureInput &input, const StateCaptureView &view) {
    CaptureWord(file, 0x5449434b); CaptureWord(file, tick); CaptureWord(file, state);
    CaptureWord(file, rng); CaptureWord(file, time); CaptureWord(file, drivingTick);
    CaptureBlock(file, "input", &input, sizeof(input));
    int physics[3] = {view.physicsStep, view.physicsScale, view.stageClock};
    CaptureBlock(file, "physics", physics, sizeof(physics));
    CaptureBlock(file, "order", view.order, view.count * 2);
    for (unsigned int slot = 0; slot < view.count; ++slot) {
        const int id = view.order[slot];
        char name[64];
        #define CAPTURE_CAR_BLOCK(label, base, stride) \
            sprintf(name, label ".%d", id); \
            CaptureBlock(file, name, base ? base + id * stride : NULL, stride)
        CAPTURE_CAR_BLOCK("car", view.cars, 0xc24);
        CAPTURE_CAR_BLOCK("parts", view.parts, 0x4d0);
        CAPTURE_CAR_BLOCK("damage", view.damage, 0x290);
        CAPTURE_CAR_BLOCK("contact", view.contacts, 0x2a4);
        CAPTURE_CAR_BLOCK("timing", view.timing, 0x88);
        CAPTURE_CAR_BLOCK("checkpoint", view.checkpoints, 0x1c);
        CAPTURE_CAR_BLOCK("transforms", view.transforms, 0xfc);
        CAPTURE_CAR_BLOCK("shadow", view.shadow, 0xfc);
        CAPTURE_CAR_BLOCK("wheels", view.wheels, 0x100);
        #undef CAPTURE_CAR_BLOCK
        if (view.cars) {
            unsigned int world, body;
            memcpy(&world, view.cars + id * 0xc24 + 0x750, 4);
            memcpy(&body, view.cars + id * 0xc24 + 0x754, 4);
            sprintf(name, "world.%d", id); CaptureBlock(file, name, (void *)world, 0x40);
            sprintf(name, "body.%d", id); CaptureBlock(file, name, (void *)body, 0x40);
        }
    }
    CaptureWord(file, 0); // end of this tick's blocks
}
inline void CaptureFinish(FILE *file, unsigned int ticks) {
    CaptureWord(file, 0x444f4e45); CaptureWord(file, ticks);
}
#endif

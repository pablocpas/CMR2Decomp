#ifndef _CAR_IMPACT_H
#define _CAR_IMPACT_H

// Packed damage impulse: positions use 127/10, vectors use 127 and
// strength/axisRadius use 255 as their quantisation scales.
struct CarImpactPayload {
    unsigned char strength;     // 0x00
    unsigned char mode;         // 0x01
    unsigned char axisRadius;   // 0x02 only read/written for mode 1
    signed char normal[3];      // 0x03
    signed char axis[3];        // 0x06
    signed char position[3];    // 0x09
};

struct CarDamageLink {
    CarImpactPayload impact;    // 0x00
    signed char next;           // 0x0c index, -1 terminates the list
};

struct CarDamageLinkPool {
    CarDamageLink links[20];
    unsigned char count;        // 0x104
    unsigned char firstLink;    // 0x105
};

struct CarDamageSnapshot {
    unsigned char intensity[34]; // 0x00 quantised per-part damage
    unsigned char padding[2];    // 0x22 alignment before flags
    int partHidden[4];           // 0x24
    int lineGrounded[3];         // 0x34
};

// Leading damage section of a persistent driver-group record. The two
// alignment bytes after the packed pool are preserved by the two-copy API.
struct RallyCarDamageState {
    CarDamageLinkPool impacts;  // 0x000
    CarDamageSnapshot damage;   // 0x108
};

#endif

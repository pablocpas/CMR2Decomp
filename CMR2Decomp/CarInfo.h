#ifndef _CAR_INFO_H
#define _CAR_INFO_H

#include "FixedPoint.h"

// CIN directory view. The trailing array is variable length; six section
// indices are observed, but neither the directory nor the file has a fixed
// total size established here. Offsets are unsigned and relative to the file.
struct CarInfoDirectory {
    unsigned short sectionOffsets[1];
};

enum CarInfoSection {
    CAR_INFO_LIGHTS = 0,
    CAR_INFO_CONTACT = 1,
    CAR_INFO_GLASS = 2,
    CAR_INFO_FLEXIBLE_LINES = 3,
    CAR_INFO_CAMERA_OFFSETS = 4,
    CAR_INFO_INTERIOR = 5
};

// One serialized light point (0x28 bytes), also updated with the closest
// object/vertex while rebuilding the light meshes.
struct CarLightPoint {
    FixVector pos;
    FixVector dir;
    int size;
    int intensity;
    unsigned char type;
    unsigned char slot;
    signed char part;
    unsigned char pad_0x23;
    short object;
    short vertex;
};

// One-element trailing arrays are variable-length views, not capacities.
struct CarLightProfile {
    int count;
    CarLightPoint points[1];
};

struct CarSkidProfilePoint {
    int longitudinalDistance;
    int lateralWidth;
};

struct CarContactProfile {
    unsigned char pointCount;
    unsigned char visibleRangeStart;
    unsigned char visibleRangeCount;
    unsigned char field_0x3;
    int longitudinalOffset;
    int wheelPatchLength;
    CarSkidProfilePoint points[1];
};

// Consumed glass prefix: three RGB/fourth-byte tint records followed by six
// vertices selected by the six window descriptors. No full section size is
// inferred from the consumed prefix.
struct CarGlassProfile {
    unsigned char tint[3][4];
    FixVector windowVertices[6];
};

struct CarFlexibleLineDescriptor {
    FixVector position;
    FixVector axis;
    int length;
    unsigned char colour[4];
};

struct CarFlexibleLineProfile {
    FixVector constraintAxis;
    int count;
    CarFlexibleLineDescriptor descriptors[1];
};

struct Car;
CarInfoDirectory *CarInfo_GetDirectory(int index);
void *CarInfo_GetSection(Car *pCar, int section);

#endif

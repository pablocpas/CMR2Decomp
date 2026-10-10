#ifndef OPENCMR2_PRESENTATION_H
#define OPENCMR2_PRESENTATION_H

#include <algorithm>

struct PresentationRect { int x, y, width, height; };

inline PresentationRect GetPresentationRect(int sourceWidth, int sourceHeight,
                                           int outputWidth, int outputHeight, bool stretch)
{
    if (stretch)
        return {0, 0, outputWidth, outputHeight};
    float scale = std::min(float(outputWidth) / sourceWidth, float(outputHeight) / sourceHeight);
    int width = int(sourceWidth * scale), height = int(sourceHeight * scale);
    return {(outputWidth - width) / 2, (outputHeight - height) / 2, width, height};
}

#endif

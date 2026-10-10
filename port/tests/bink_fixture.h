#ifndef OPENCMR2_TEST_BINK_FIXTURE_H
#define OPENCMR2_TEST_BINK_FIXTURE_H
#include "video/bink_decoder.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace BinkTest {
void Check(bool value, const char *message) {
    if (!value) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}
void Put32(std::vector<uint8_t> &data, size_t offset, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) data[offset + i] = value >> (8 * i);
}
struct Writer {
    std::vector<uint8_t> data;
    unsigned position = 0;
    void Write(uint32_t value, unsigned count) {
        for (unsigned i = 0; i < count; ++i, ++position) {
            if (position / 8 == data.size()) data.push_back(0);
            data[position / 8] |= ((value >> i) & 1) << (position & 7);
        }
    }
    void Align() { Write(0, (-position) & 31); }
};
unsigned Length(unsigned width, int source) {
    unsigned bw = (width + 7) / 8, aligned = (width + 7) & ~7u;
    unsigned value = (source == 1 ? aligned / 16 : bw) + 511;
    if (source == 2) value = bw * 64 + 511;
    if (source == 3) value = bw * 8 + 511;
    if (source == 8) value = bw * 48 + 511;
    unsigned result = 0; do { ++result; } while (value >>= 1);
    return result;
}
// Real BIKi bundle encoding: identity Huffman trees, constant blocks/colours.
void Plane(Writer &bits, unsigned width, unsigned height, unsigned type, uint8_t color) {
    for (int source = 0; source < 9; ++source) {
        if (source == 2) for (int i = 0; i < 16; ++i) bits.Write(0, 4);
        if (source != 6 && source != 7) bits.Write(0, 4);
    }
    unsigned blocks = ((width + 7) / 8) * ((height + 7) / 8);
    for (int source = 0; source < 9; ++source) {
        unsigned count = source == 0 ? blocks : 0;
        if (source == 2 && type == 6) count = blocks;
        if ((source == 4 || source == 5) && type == 2) count = blocks;
        bits.Write(count, Length(width, source));
        if (!count) continue;
        bits.Write(1, 1);
        if (source == 0) bits.Write(type, 4);
        else if (source == 2) { bits.Write(color >> 4, 4); bits.Write(color & 15, 4); }
        else bits.Write(0, 4); // zero motion vectors
    }
    bits.Align();
}
std::vector<uint8_t> Video(unsigned width, unsigned height, unsigned type, uint8_t luma) {
    Writer bits;
    bits.Write(0, 32);
    Plane(bits, width, height, type, luma);
    Plane(bits, (width + 1) / 2, (height + 1) / 2, type, 128);
    Plane(bits, (width + 1) / 2, (height + 1) / 2, type, 128);
    return bits.data;
}
// Constant non-zero DC audio exercises the inverse transform and overlap.
std::vector<uint8_t> Audio(bool dct) {
    Writer bits;
    unsigned length = dct ? 512 : 1024;
    unsigned count = (length - length / 16) * (dct ? 2 : 1);
    bits.Write(count * 2, 32);
    if (dct) bits.Write(0, 2);
    for (unsigned ch = 0; ch < (dct ? 2u : 1u); ++ch) {
        bits.Write(23, 5); bits.Write(1, 23); bits.Write(0, 1); // 1.0
        bits.Write(23, 5); bits.Write(1, 23); bits.Write(ch, 1); // Nyquist/RDFT or first DCT AC
        // RDFT effective sample rate is 16000; DCT is 8000.
        unsigned bands = dct ? 18 : 22;
        for (unsigned i = 0; i < bands; ++i) bits.Write(0, 8);
        for (unsigned i = 2; i < length; i += 8) { bits.Write(0, 1); bits.Write(0, 4); }
    }
    bits.Align();
    return bits.data;
}
std::vector<uint8_t> Container(unsigned width, unsigned height,
                               std::vector<std::vector<uint8_t>> frames,
                               int audioFlags = -1) {
    unsigned tracks = audioFlags < 0 ? 0 : 1;
    std::vector<uint8_t> data(44 + tracks * 12 + frames.size() * 4);
    std::memcpy(data.data(), "BIKi", 4);
    Put32(data, 8, frames.size()); Put32(data, 20, width); Put32(data, 24, height);
    Put32(data, 28, 30000); Put32(data, 32, 1001); Put32(data, 40, tracks);
    if (tracks) { Put32(data, 44, 4096); Put32(data, 48, 8000 | unsigned(audioFlags) << 16); Put32(data, 52, 7); }
    unsigned largest = 0;
    for (size_t i = 0; i < frames.size(); ++i) {
        largest = std::max(largest, unsigned(frames[i].size()));
        Put32(data, 44 + tracks * 12 + i * 4, data.size() | (i == 0));
        data.insert(data.end(), frames[i].begin(), frames[i].end());
    }
    Put32(data, 4, data.size() - 8); Put32(data, 12, largest);
    return data;
}
bool Read(void *context, uint32_t offset, void *dst, uint32_t bytes) {
    const auto &data = *static_cast<std::vector<uint8_t> *>(context);
    if (offset > data.size() || bytes > data.size() - offset) return false;
    std::memcpy(dst, data.data() + offset, bytes);
    return true;
}
bool Open(Bink::Decoder &decoder, std::vector<uint8_t> &data, int track = 0) {
    return decoder.Open({&data, uint32_t(data.size()), Read}, track);
}
}

#endif

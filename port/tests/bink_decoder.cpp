#include "bink_fixture.h"

using namespace BinkTest;

int main() {
    Bink::Decoder decoder;
    Bink::Frame frame;
    auto data = Container(9, 9, {Video(9, 9, 6, 81), Video(9, 9, 0, 0), Video(9, 9, 2, 0)});
    Check(Open(decoder, data), "cannot open synthetic movie");
    Check(decoder.GetInfo().fpsNumerator == 30000 && decoder.GetInfo().fpsDenominator == 1001, "fractional fps");
    std::vector<uint8_t> previous;
    for (int i = 0; i < 3; ++i) {
        Check(decoder.Decode(frame), decoder.Error().c_str());
        Check(frame.bgrx.size() == 9 * 9 * 4, "cropping padded planes");
        for (size_t pixel = 0; pixel < 81; ++pixel) {
            Check(frame.bgrx[pixel * 4] == 76 && frame.bgrx[pixel * 4 + 1] == 76 &&
                  frame.bgrx[pixel * 4 + 2] == 76 && frame.bgrx[pixel * 4 + 3] == 255, "fill/skip/motion pixels");
        }
        if (i) Check(previous == frame.bgrx, "delta frame reference");
        previous = frame.bgrx;
    }
    Check(!decoder.Decode(frame), "must stop after final frame");
    for (bool dct : {false, true}) {
        auto audio = Audio(dct), video = Video(8, 8, 6, 16);
        std::vector<uint8_t> packet(4); Put32(packet, 0, audio.size());
        packet.insert(packet.end(), audio.begin(), audio.end()); packet.insert(packet.end(), video.begin(), video.end());
        auto sound = Container(8, 8, {packet, packet}, 0x2000 | (dct ? 0x1000 : 0));
        Check(Open(decoder, sound, 7), "open audio track by ID");
        Check(decoder.GetInfo().sampleRate == 8000 && decoder.GetInfo().channels == 2, "audio format");
        for (int i = 0; i < 2; ++i) {
            Check(decoder.Decode(frame), decoder.Error().c_str());
            Check(frame.audio.size() == 960, "stereo audio sample count");
            auto sampleAt = [&](unsigned index, unsigned channel) {
                constexpr double pi = 3.14159265358979323846;
                if (!dct) return float((index % 2 ? 0 : 2.0) / std::sqrt(1024.0) / 32768);
                double sign = channel ? -1 : 1;
                return float(2 * (1 + sign * std::cos(pi * (index + 0.5) / 512)) / std::sqrt(512.0) / 32768);
            };
            for (unsigned sample = 0; sample < frame.audio.size(); ++sample) {
                unsigned index = dct ? sample / 2 : sample, channel = dct ? sample % 2 : 0;
                float expected = sampleAt(index, channel);
                unsigned overlap = dct ? 32 : 64;
                if (i && index < overlap) {
                    unsigned weight = dct ? index * 2 + channel : index;
                    float previous = sampleAt((dct ? 512 : 1024) - overlap + index, channel);
                    expected = (previous * (64 - weight) + expected * weight) / 64;
                }
                Check(std::abs(frame.audio[sample] - expected) < 1e-9, "audio transform/interleave/overlap");
            }
        }
        Check(Open(decoder, sound, 123) && decoder.Decode(frame) && frame.audio.size() == 960, "track fallback");
        Check(Open(decoder, sound, -1) && decoder.Decode(frame) && frame.audio.empty(), "silent track");
        auto broken = sound; Put32(broken, 64, 0xffffffff); // first packet audio length
        Check(Open(decoder, broken) && !decoder.Decode(frame), "audio length bounds");
    }
    // Every truncation must fail to open or decode, never read outside the input.
    for (size_t size = 0; size < data.size(); ++size) {
        auto truncated = std::vector<uint8_t>(data.begin(), data.begin() + size);
        Check(!Open(decoder, truncated), "truncated movie accepted");
    }
    for (auto field : {4, 8, 12, 20, 24, 28, 32, 40, 44}) {
        auto corrupt = data; Put32(corrupt, field, 0xffffffff);
        Check(!Open(decoder, corrupt), "invalid header/index accepted");
    }
    auto invalidType = Container(8, 8, {Video(8, 8, 15, 0)});
    Check(Open(decoder, invalidType) && !decoder.Decode(frame), "invalid block type accepted");
    Check(frame.audio.empty() && frame.bgrx.empty(), "partial frame exposed after corruption");
    Check(!decoder.Decode(frame), "corrupt decoder resumed");
    // Deterministic mutations cover trees, bundles, bit exhaustion and coefficient lists.
    uint32_t random = 0x51f15e;
    for (unsigned i = 0; i < 2000; ++i) {
        auto mutated = data;
        random = random * 1664525 + 1013904223;
        size_t offset = 56 + random % (mutated.size() - 56);
        random = random * 1664525 + 1013904223;
        mutated[offset] ^= 1u << (random & 7);
        if (Open(decoder, mutated)) while (decoder.Decode(frame)) {}
    }
    std::puts("Bink container, video, audio, truncation and corruption tests passed");
}

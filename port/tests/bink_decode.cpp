// Optional real-file check/dump tool. Original game data is never bundled.
#include "video/bink_decoder.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

static bool Read(void *context, uint32_t offset, void *dst, uint32_t bytes)
{
    FILE *file = static_cast<FILE *>(context);
    return std::fseek(file, offset, SEEK_SET) == 0 && std::fread(dst, 1, bytes, file) == bytes;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::fprintf(stderr, "usage: bink_decode FILE [TRACK_ID [BGRX_FILE F32_FILE [FRAME_LIMIT]]]\n");
        return 1;
    }
    FILE *file = std::fopen(argv[1], "rb");
    if (!file || std::fseek(file, 0, SEEK_END)) {
        std::fprintf(stderr, "cannot open %s\n", argv[1]);
        if (file) std::fclose(file);
        return 1;
    }
    long size = std::ftell(file);
    Bink::Decoder decoder;
    int track = argc > 2 ? std::atoi(argv[2]) : 0;
    if (size < 0 || size > 0x7fffffff || !decoder.Open({file, uint32_t(size), Read}, track)) {
        std::fprintf(stderr, "%s\n", decoder.Error().c_str());
        std::fclose(file);
        return 1;
    }
    FILE *video = argc > 3 && argv[3][0] ? std::fopen(argv[3], "wb") : nullptr;
    FILE *audio = argc > 4 && argv[4][0] ? std::fopen(argv[4], "wb") : nullptr;
    if ((argc > 3 && argv[3][0] && !video) || (argc > 4 && argv[4][0] && !audio)) {
        std::fprintf(stderr, "cannot open output\n");
        if (video) std::fclose(video);
        if (audio) std::fclose(audio);
        std::fclose(file);
        return 1;
    }
    Bink::Info info = decoder.GetInfo();
    unsigned count = argc > 5 ? std::min(unsigned(std::atoi(argv[5])), info.frames) : info.frames;
    Bink::Frame frame;
    uint64_t hash = 14695981039346656037ull, samples = 0;
    float peak = 0;
    bool ok = true;
    for (unsigned i = 0; i < count; ++i) {
        if (!decoder.Decode(frame)) {
            std::fprintf(stderr, "frame %u: %s\n", i + 1, decoder.Error().c_str());
            ok = false;
            break;
        }
        for (uint8_t byte : frame.bgrx) { hash ^= byte; hash *= 1099511628211ull; }
        samples += frame.audio.size();
        for (float sample : frame.audio) peak = std::max(peak, std::abs(sample));
        if (video && std::fwrite(frame.bgrx.data(), 1, frame.bgrx.size(), video) != frame.bgrx.size()) ok = false;
        if (audio && std::fwrite(frame.audio.data(), sizeof(float), frame.audio.size(), audio) != frame.audio.size()) ok = false;
        if (!ok) break;
    }
    std::printf("%ux%u, %u frames, %u/%u fps, %u Hz/%u channels: %llu samples, peak %.6f, BGRX FNV64 %016llx\n",
                info.width, info.height, count, info.fpsNumerator, info.fpsDenominator,
                info.sampleRate, info.channels, (unsigned long long)samples, peak, (unsigned long long)hash);
    if (video && std::fclose(video)) ok = false;
    if (audio && std::fclose(audio)) ok = false;
    std::fclose(file);
    return ok ? 0 : 1;
}

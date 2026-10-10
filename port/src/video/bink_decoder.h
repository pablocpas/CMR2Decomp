// Standalone Bink 1 decoder. No platform, graphics or audio dependencies.
#ifndef OPENCMR2_BINK_DECODER_H
#define OPENCMR2_BINK_DECODER_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Bink {

struct Reader {
    void *context;
    uint32_t size;
    bool (*readAt)(void *context, uint32_t offset, void *dst, uint32_t bytes);
};

struct Info {
    uint32_t width = 0, height = 0, frames = 0;
    uint32_t fpsNumerator = 0, fpsDenominator = 0;
    uint32_t sampleRate = 0;
    unsigned channels = 0;
};

struct Frame {
    std::vector<uint8_t> bgrx;
    // Interleaved float PCM for the selected track, at Info::sampleRate.
    std::vector<float> audio;
};

class Decoder {
public:
    Decoder();
    ~Decoder();
    Decoder(const Decoder &) = delete;
    Decoder &operator=(const Decoder &) = delete;
    // Reader remains owned by caller. Track is a Bink track ID (language);
    // unavailable IDs fall back to the first track, -1 selects silence.
    bool Open(Reader reader, int track);
    // Sequential decode; on corruption decoding stops until Open is called.
    bool Decode(Frame &frame);
    const Info &GetInfo() const;
    const std::string &Error() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace Bink
#endif

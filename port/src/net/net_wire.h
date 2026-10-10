// Little-endian serialisation for the network protocol. The reader checks
// every length against what is left, so packets from the network can be
// parsed without trusting them: after a failed read ok() is false and every
// later read returns zeros.

#ifndef OPENCMR2_NET_WIRE_H
#define OPENCMR2_NET_WIRE_H

#include <stdint.h>
#include <string.h>

#include <string>
#include <vector>

namespace net {

class Writer {
public:
    void U8(uint8_t v) { data.push_back(v); }
    void U16(uint16_t v)
    {
        U8((uint8_t)v);
        U8((uint8_t)(v >> 8));
    }
    void U32(uint32_t v)
    {
        U16((uint16_t)v);
        U16((uint16_t)(v >> 16));
    }
    void Bytes(const void *p, size_t n)
    {
        const uint8_t *b = (const uint8_t *)p;
        data.insert(data.end(), b, b + n);
    }
    // Length-prefixed (32-bit) blob and string.
    void Blob(const void *p, size_t n)
    {
        U32((uint32_t)n);
        Bytes(p, n);
    }
    void Blob(const std::vector<uint8_t> &v) { Blob(v.data(), v.size()); }
    void Str(const std::string &s) { Blob(s.data(), s.size()); }

    std::vector<uint8_t> data;
};

class Reader {
public:
    Reader(const void *p, size_t n) : p((const uint8_t *)p), left(n) {}

    bool ok() const { return good; }
    bool empty() const { return left == 0; }

    uint8_t U8()
    {
        uint8_t v = 0;
        Take(&v, 1);
        return v;
    }
    uint16_t U16()
    {
        uint8_t b[2] = {};
        Take(b, 2);
        return (uint16_t)(b[0] | b[1] << 8);
    }
    uint32_t U32()
    {
        uint8_t b[4] = {};
        Take(b, 4);
        return (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
    }
    void Bytes(void *out, size_t n)
    {
        if (!Take(out, n))
            memset(out, 0, n);
    }
    // A blob of at most max bytes; longer ones fail the reader.
    std::vector<uint8_t> Blob(size_t max)
    {
        uint32_t n = U32();
        std::vector<uint8_t> v;
        if (!good || n > max || n > left) {
            good = false;
            return v;
        }
        v.assign(p, p + n);
        p += n;
        left -= n;
        return v;
    }
    // A string of at most max bytes, cut at an embedded NUL.
    std::string Str(size_t max)
    {
        std::vector<uint8_t> v = Blob(max);
        std::string s(v.begin(), v.end());
        size_t nul = s.find('\0');
        if (nul != std::string::npos)
            s.resize(nul);
        return s;
    }

private:
    bool Take(void *out, size_t n)
    {
        if (!good || n > left) {
            good = false;
            return false;
        }
        memcpy(out, p, n);
        p += n;
        left -= n;
        return true;
    }

    const uint8_t *p;
    size_t left;
    bool good = true;
};

} // namespace net

#endif

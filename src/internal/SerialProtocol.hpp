#pragma once
#include "../CANBridge/Types.h"
#include <cstddef>
namespace canbridge { namespace detail {
inline int hexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}
// S 123#1122, E 001ABCDE#0102, S 123#R8. Output unchanged on failure.
inline bool parseFrame(const char *line, Frame &out) {
    Frame f;
    if ((line[0] != 'S' && line[0] != 'E') || line[1] != ' ') return false;
    f.extended = line[0] == 'E';
    const char *p = line + 2;
    unsigned digits = 0;
    while (*p && *p != '#') {
        const int h = hexDigit(*p++);
        if (h < 0 || ++digits > (f.extended ? 8U : 3U)) return false;
        f.id = (f.id << 4) | static_cast<unsigned>(h);
    }
    if (!digits || *p++ != '#') return false;
    if (*p == 'R') {
        ++p;
        if (*p < '0' || *p > '8' || p[1]) return false;
        f.remote = true;
        f.length = static_cast<std::uint8_t>(*p - '0');
    } else {
        while (*p) {
            if (!p[1] || f.length == 8) return false;
            const int a = hexDigit(p[0]), b = hexDigit(p[1]);
            if (a < 0 || b < 0) return false;
            f.data[f.length++] = static_cast<std::uint8_t>((a << 4) | b);
            p += 2;
        }
    }
    if (!valid(f)) return false;
    out = f;
    return true;
}
inline void formatFrame(const Frame &f, char *out) {
    const char *hex = "0123456789ABCDEF";
    *out++ = f.extended ? 'E' : 'S'; *out++ = ' ';
    for (int n = f.extended ? 7 : 2; n >= 0; --n) *out++ = hex[(f.id >> (n * 4)) & 15];
    *out++ = '#';
    if (f.remote) { *out++ = 'R'; *out++ = static_cast<char>('0' + f.length); }
    else for (unsigned i = 0; i < f.length; ++i) { *out++ = hex[f.data[i] >> 4]; *out++ = hex[f.data[i] & 15]; }
    *out = 0;
}
} }

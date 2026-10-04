#pragma once
#include "Types.hpp"
namespace canbridge {
struct EspConfig {
    std::uint32_t bitrate = 0;
    int txPin = -1;
    int rxPin = -1;
    bool listenOnly = false;
};
inline Result validate(const EspConfig &c) {
    if (!c.bitrate) return Result::MissingBitrate;
    if (c.txPin == -1) return Result::MissingTxPin;
    if (c.rxPin == -1) return Result::MissingRxPin;
    if (c.txPin < 0 || c.rxPin < 0 || c.txPin == c.rxPin) return Result::InvalidPin;
    return Result::Ok;
}
}

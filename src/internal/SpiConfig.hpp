#pragma once
#include "../CANBridge/Types.h"
#include <SPI.h>
namespace canbridge {
struct SpiConfig {
    std::uint32_t bitrate = 0;
    SPIClass *spi = nullptr;
    int csPin = -1;
    std::uint32_t oscillatorHz = 0;
    bool listenOnly = false;
};
inline Result validate(const SpiConfig &c) {
    if (!c.bitrate) return Result::MissingBitrate;
    if (!c.spi) return Result::MissingSpi;
    if (c.csPin == -1) return Result::MissingCsPin;
    if (c.csPin < 0 || c.csPin >= 255) return Result::InvalidPin;
    if (!c.oscillatorHz) return Result::MissingOscillator;
    return Result::Ok;
}
}

#pragma once
#if defined(CANBRIDGE_SELECTED_BACKEND)
#error "Include only one CANBridge controller selection header per translation unit"
#endif
#define CANBRIDGE_SELECTED_BACKEND 1
#include "../detail/SpiControllers.hpp"
#include "../detail/SpiConfiguration.hpp"
#include "../detail/ConfiguredBus.hpp"
namespace canbridge {
namespace detail {
struct Mcp2518Factory {
    using Backend = Mcp2518;
    using Configuration = SpiConfig;
    static Result check(const Configuration &c) {
        const Result checked = validate(c);
        if (checked != Result::Ok) return checked;
        if (c.oscillatorHz != 4000000 && c.oscillatorHz != 20000000 && c.oscillatorHz != 40000000)
            return Result::UnsupportedOscillator;
        return Result::Ok;
    }
    static Backend *create(const Configuration &c) {
        return new (std::nothrow) Mcp2518(*c.spi, static_cast<std::uint8_t>(c.csPin), c.oscillatorHz);
    }
};
}
using Config = SpiConfig;
using Bus = ConfiguredBus<detail::Mcp2518Factory>;
}

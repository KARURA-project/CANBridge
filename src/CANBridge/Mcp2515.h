#pragma once
#if defined(CANBRIDGE_SELECTED_BACKEND)
#error "Include only one CANBridge controller selection header per translation unit"
#endif
#define CANBRIDGE_SELECTED_BACKEND 1
#include "../drivers/mcp/SpiControllers.hpp"
#include "../internal/SpiConfig.hpp"
#include "../internal/ConfiguredBus.hpp"
namespace canbridge {
namespace detail {
struct Mcp2515Factory {
    using Backend = Mcp2515;
    using Configuration = SpiConfig;
    static Result check(const Configuration &c) {
        return validate(c);
    }
    static Backend *create(const Configuration &c) {
        return new (std::nothrow) Mcp2515(*c.spi, static_cast<std::uint8_t>(c.csPin), c.oscillatorHz);
    }
};
}
using Config = SpiConfig;
using Bus = ConfiguredBus<detail::Mcp2515Factory>;
}

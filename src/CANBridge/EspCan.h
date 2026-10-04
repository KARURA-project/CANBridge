#pragma once
#if defined(CANBRIDGE_SELECTED_BACKEND)
#error "Include only one CANBridge controller selection header per translation unit"
#endif
#define CANBRIDGE_SELECTED_BACKEND 1
#ifndef ARDUINO_ARCH_ESP32
#error "CANBridge/EspCan.h requires an Arduino ESP32 target"
#endif
#include "../detail/EspCan.hpp"
#include "../detail/Configurations.hpp"
#include "../detail/ConfiguredBus.hpp"
namespace canbridge {
namespace detail {
struct EspCanFactory {
    using Backend = EspCan;
    using Configuration = EspConfig;
    static Result check(const Configuration &c) {
        return validate(c);
    }
    static Backend *create(const Configuration &c) {
        return new (std::nothrow) EspCan(c.txPin, c.rxPin);
    }
};
}
using Config = EspConfig;
using Bus = ConfiguredBus<detail::EspCanFactory>;
}

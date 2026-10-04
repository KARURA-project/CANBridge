#pragma once
#include "Controller.hpp"
#include <memory>
#include <new>
namespace canbridge {
// Factory validation occurs before allocating or touching hardware.
template <typename Factory> class ConfiguredBus {
    std::unique_ptr<typename Factory::Backend> driver_;
public:
    ConfiguredBus() = default;
    ConfiguredBus(const ConfiguredBus &) = delete;
    ConfiguredBus &operator=(const ConfiguredBus &) = delete;
    ~ConfiguredBus() { if (driver_) driver_->end(); }

    Result begin(const typename Factory::Configuration &config) {
        if (driver_) return Result::AlreadyStarted;
        const Result checked = Factory::check(config);
        if (checked != Result::Ok) return checked;
        std::unique_ptr<typename Factory::Backend> candidate(Factory::create(config));
        if (!candidate) return Result::AllocationFailed;
        CommonConfig common;
        common.bitrate = config.bitrate;
        common.listenOnly = config.listenOnly;
        const Result result = candidate->begin(common);
        if (result == Result::Ok) driver_ = std::move(candidate);
        return result;
    }
    Result end() {
        if (!driver_) return Result::NotStarted;
        const Result result = driver_->end();
        if (result == Result::Ok) driver_.reset();
        return result;
    }
    Result send(const Frame &frame) {
        return driver_ ? driver_->send(frame) : Result::NotStarted;
    }
    Result receive(Frame &frame) {
        return driver_ ? driver_->receive(frame) : Result::NotStarted;
    }
    Result pollHealth(Health &health) {
        return driver_ ? driver_->pollHealth(health) : Result::NotStarted;
    }
};
}

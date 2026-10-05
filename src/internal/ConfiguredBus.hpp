#pragma once
#include "Controller.hpp"
#include <memory>
#include <new>
#include <Arduino.h>
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
        if (result == Result::Ok || candidate->end() != Result::Ok) driver_ = std::move(candidate);
        return result;
    }
    // Startup-only synchronous test. A failed cleanup retains the backend and
    // blocks begin/diagnose until end() confirms shutdown.
    Result diagnose(const typename Factory::Configuration &config, DiagnosticReport &report) {
        report = DiagnosticReport{};
        if (driver_) return Result::AlreadyStarted;
        const Result checked = Factory::check(config);
        if (checked != Result::Ok) return checked;
        driver_.reset(Factory::create(config));
        if (!driver_) return Result::AllocationFailed;
        CommonConfig common;
        common.bitrate = config.bitrate;
        common.selfTest = true; // Diagnostic overrides listen-only, not the caller's config.
        Result result = driver_->beginDiagnostic(common, report);
        if (report.controllerAccess.status != TestStatus::Failed)
            report.controllerOperation = {result == Result::Ok ? TestStatus::Passed : result == Result::Unsupported ? TestStatus::Unsupported : TestStatus::Failed, result, report.controllerAccess.raw};
        if (result == Result::Ok) {
            result = checkLoopback(report);
            report.internalLoopback = {result == Result::Ok ? TestStatus::Passed : TestStatus::Failed, result, 0};
        }
        const Result stopped = driver_->end();
        report.cleanup = {stopped == Result::Ok ? TestStatus::Passed : TestStatus::Failed, stopped, 0};
        if (stopped == Result::Ok) driver_.reset();
        return stopped != Result::Ok ? stopped : result;
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
private:
    Result checkLoopback(DiagnosticReport &report) {
        // Cover standard/extended, zero/eight bytes and RTR/DLC semantics.
        for (unsigned n = 0; n < 4; ++n) {
            Frame sent;
            sent.id = n == 1 ? 0x1234567U : 0x321U + n;
            sent.extended = n == 1;
            sent.remote = n == 3;
            sent.length = n == 0 ? 0 : 8;
            for (unsigned i = 0; i < 8; ++i) sent.data[i] = static_cast<std::uint8_t>(0x55U ^ (i * 17U) ^ n);
            const auto start = millis();
            bool accepted = false;
            bool matched = false;
            while (static_cast<std::uint32_t>(millis() - start) < 250U) {
                if (!accepted) {
                    const Result tx = driver_->send(sent);
                    if (tx != Result::Ok && tx != Result::Busy) return tx;
                    accepted = tx == Result::Ok;
                }
                if (accepted) {
                    Frame received;
                    const Result rx = driver_->receive(received);
                    if (rx == Result::Ok) {
                        if (received.id != sent.id || received.length != sent.length ||
                            received.extended != sent.extended || received.remote != sent.remote)
                            return Result::DataMismatch;
                        if (!sent.remote)
                            for (unsigned i = 0; i < sent.length; ++i)
                                if (received.data[i] != sent.data[i]) return Result::DataMismatch;
                        matched = true;
                        ++report.framesChecked;
                        break;
                    }
                    if (rx != Result::Empty) return rx;
                }
                delay(1); // Bounded startup test; allow RTOS/background receive work.
            }
            if (!matched) return Result::Timeout;
        }
        return Result::Ok;
    }
};
}

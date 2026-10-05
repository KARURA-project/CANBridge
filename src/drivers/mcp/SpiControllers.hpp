#pragma once
#include "../../internal/Controller.hpp"
#include <ACAN2515.h>
#include <ACAN2517FD.h>
namespace canbridge {
// Polling mode: no user ISR and no library-global singleton needed.
// Configure the SPI pins and call SPI.begin() before begin().
class Mcp2515 final : public Controller {
    ACAN2515 driver_;
    std::uint32_t oscillator_;
    bool started_ = false;
    std::uint32_t lastError_ = 0;
    bool stopping_ = false;
    bool attempted_ = false;
public:
    Mcp2515(SPIClass &spi, std::uint8_t cs, std::uint32_t oscillatorHz)
        : driver_(cs, spi, 255), oscillator_(oscillatorHz) {}
    Result begin(const CommonConfig &c) override {
        if (started_) return Result::AlreadyStarted;
        if (!c.bitrate || !oscillator_) return Result::InvalidConfig;
        ACAN2515Settings settings(oscillator_, c.bitrate);
        settings.mRequestedMode = c.selfTest ? ACAN2515Settings::LoopBackMode : c.listenOnly ? ACAN2515Settings::ListenOnlyMode : ACAN2515Settings::NormalMode;
        attempted_ = true;
        lastError_ = driver_.begin(settings, nullptr);
        started_ = true; // end() also cleans partially initialized buffers.
        if (lastError_) return Result::DriverError;
        return Result::Ok;
    }
    Result beginDiagnostic(const CommonConfig &c, DiagnosticReport &report) override {
        lastError_ = 0; attempted_ = false;
        const Result result = begin(c);
        if (!attempted_) return result;
        const bool accessFailed = (lastError_ & ACAN2515::kNoMCP2515) != 0;
        report.controllerAccess = {accessFailed ? TestStatus::Failed : TestStatus::Passed,
                                   accessFailed ? Result::DriverError : Result::Ok, lastError_};
        return result;
    }
    Result end() override {
        if (!started_) return Result::Ok;
        stopping_ = true;
        const auto error = driver_.changeModeOnTheFly(static_cast<ACAN2515Settings::RequestedMode>(4U << 5));
        driver_.end();
        if (error) return Result::DriverError;
        started_ = false; stopping_ = false; return Result::Ok;
    }
    Result send(const Frame &f) override {
        if (!started_ || stopping_) return Result::NotStarted;
        if (!valid(f)) return Result::InvalidFrame;
        driver_.poll(); CANMessage m;
        m.id=f.id; m.ext=f.extended; m.rtr=f.remote; m.len=f.length;
        for (unsigned i=0;i<f.length;++i) m.data[i]=f.data[i];
        return driver_.tryToSend(m) ? Result::Ok : Result::Busy;
    }
    Result receive(Frame &f) override {
        if (!started_ || stopping_) return Result::NotStarted;
        driver_.poll(); CANMessage m;
        if (!driver_.receive(m)) return Result::Empty;
        if (m.len>8) return Result::DriverError;
        Frame out; out.id=m.id; out.length=m.len; out.extended=m.ext; out.remote=m.rtr;
        for (unsigned i=0;i<m.len;++i) out.data[i]=m.data[i];
        f=out; return Result::Ok;
    }
    Result pollHealth(Health &h) override {
        if (!started_ || stopping_) return Result::NotStarted;
        driver_.poll(); h=Health{}; h.raw=driver_.errorFlagRegister();
        h.busOff=(h.raw & 0x20U)!=0; h.errorPassive=(h.raw & 0x18U)!=0;
        h.receiveLoss=(h.raw & 0xC0U)!=0 || driver_.receiveBufferPeakCount()>driver_.receiveBufferSize();
        // Upstream software-buffer loss is not fully observable.
        return Result::Ok;
    }
};
class Mcp2518 final : public Controller {
    ACAN2517FD driver_;
    std::uint32_t oscillator_;
    bool started_ = false;
    std::uint32_t lastError_ = 0;
    bool stopping_ = false;
    bool attempted_ = false;
public:
    Mcp2518(SPIClass &spi, std::uint8_t cs, std::uint32_t oscillatorHz)
        : driver_(cs, spi, 255), oscillator_(oscillatorHz) {}
    ~Mcp2518() override {
        if (started_) driver_.end();
#ifdef ARDUINO_ARCH_ESP32
        if (driver_.mISRSemaphore) vSemaphoreDelete(driver_.mISRSemaphore);
#endif
    }
    Result begin(const CommonConfig &c) override {
        if (started_) return Result::AlreadyStarted;
#ifdef ARDUINO_ARCH_ESP32
        if (!driver_.mISRSemaphore) return Result::AllocationFailed;
#endif
        if (!c.bitrate) return Result::InvalidConfig;
        ACAN2517FDSettings::Oscillator osc;
        if (oscillator_==4000000) osc=ACAN2517FDSettings::OSC_4MHz;
        else if (oscillator_==20000000) osc=ACAN2517FDSettings::OSC_20MHz;
        else if (oscillator_==40000000) osc=ACAN2517FDSettings::OSC_40MHz;
        else return Result::Unsupported;
        ACAN2517FDSettings settings(osc,c.bitrate,DataBitRateFactor::x1);
        settings.mRequestedMode=c.selfTest ? ACAN2517FDSettings::InternalLoopBack : c.listenOnly ? ACAN2517FDSettings::ListenOnly : ACAN2517FDSettings::Normal20B;
        attempted_ = true;
        lastError_ = driver_.begin(settings,nullptr);
        started_=true;
        if (lastError_) return Result::DriverError;
        return Result::Ok;
    }
    Result beginDiagnostic(const CommonConfig &c, DiagnosticReport &report) override {
        lastError_ = 0; attempted_ = false;
        const Result result = begin(c);
        if (!attempted_) return result;
        const auto accessErrors = ACAN2517FD::kRequestedConfigurationModeTimeOut |
            ACAN2517FD::kReadBackErrorWith1MHzSPIClock | ACAN2517FD::kReadBackErrorWithFullSpeedSPIClock;
        const bool failed = (lastError_ & accessErrors) != 0;
        const bool reached = result == Result::Ok || (lastError_ & ~(ACAN2517FD::kRequestedModeTimeOut | ACAN2517FD::kX10PLLNotReadyWithin1MS)) == 0;
        report.controllerAccess = {failed ? TestStatus::Failed : reached ? TestStatus::Passed : TestStatus::NotRun,
                                   failed ? Result::DriverError : Result::Ok, lastError_};
        return result;
    }
    Result end() override {
        // Upstream frees its buffers and stops its task even if mode confirmation fails.
        if (!started_) return Result::Ok;
        stopping_ = true;
        const bool stopped = driver_.end();
        if (stopped) { started_=false; stopping_=false; }
        return stopped ? Result::Ok : Result::DriverError;
    }
    Result send(const Frame &f) override {
        if (!started_ || stopping_) return Result::NotStarted;
        if (!valid(f)) return Result::InvalidFrame;
        driver_.poll(); CANFDMessage m;
        m.id=f.id; m.ext=f.extended; m.len=f.length;
        m.type=f.remote ? CANFDMessage::CAN_REMOTE : CANFDMessage::CAN_DATA;
        for (unsigned i=0;i<f.length;++i) m.data[i]=f.data[i];
        return driver_.tryToSend(m) ? Result::Ok : Result::Busy;
    }
    Result receive(Frame &f) override {
        if (!started_ || stopping_) return Result::NotStarted;
        driver_.poll(); CANFDMessage m;
        if (!driver_.receive(m)) return Result::Empty;
        if (m.len>8 || (m.type!=CANFDMessage::CAN_DATA && m.type!=CANFDMessage::CAN_REMOTE)) return Result::Unsupported;
        Frame out; out.id=m.id; out.length=m.len; out.extended=m.ext;
        out.remote=m.type==CANFDMessage::CAN_REMOTE;
        for (unsigned i=0;i<m.len;++i) out.data[i]=m.data[i];
        f=out; return Result::Ok;
    }
    Result pollHealth(Health &h) override {
        if (!started_ || stopping_) return Result::NotStarted;
        driver_.poll(); h=Health{}; h.raw=driver_.errorCounters();
        h.busOff=(h.raw & (1UL<<21))!=0;
        h.errorPassive=(h.raw & ((1UL<<19)|(1UL<<20)))!=0;
        h.receiveLoss=driver_.hardwareReceiveBufferOverflowCount()!=0 || driver_.driverReceiveBufferPeakCount()>32U;
        return Result::Ok;
    }
};
}

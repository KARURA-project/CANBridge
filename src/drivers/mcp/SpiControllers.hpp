#pragma once
#include "../../CANBridge/Types.h"
#include <ACAN2515.h>
#include <ACAN2517FD.h>
namespace canbridge {
// Polling mode: no user ISR and no library-global singleton needed.
// Configure the SPI pins and call SPI.begin() before begin().
class Mcp2515 final : public Controller {
    ACAN2515 driver_;
    std::uint32_t oscillator_;
    bool started_ = false;
public:
    Mcp2515(SPIClass &spi, std::uint8_t cs, std::uint32_t oscillatorHz)
        : driver_(cs, spi, 255), oscillator_(oscillatorHz) {}
    Result begin(const CommonConfig &c) override {
        if (started_) return Result::AlreadyStarted;
        if (!c.bitrate || !oscillator_) return Result::InvalidConfig;
        ACAN2515Settings settings(oscillator_, c.bitrate);
        settings.mRequestedMode = c.listenOnly ? ACAN2515Settings::ListenOnlyMode : ACAN2515Settings::NormalMode;
        if (driver_.begin(settings, nullptr)) { driver_.end(); return Result::DriverError; }
        started_ = true; return Result::Ok;
    }
    Result end() override {
        if (!started_) return Result::NotStarted;
        driver_.end(); started_ = false; return Result::Ok;
    }
    Result send(const Frame &f) override {
        if (!started_) return Result::NotStarted;
        if (!valid(f)) return Result::InvalidFrame;
        driver_.poll(); CANMessage m;
        m.id=f.id; m.ext=f.extended; m.rtr=f.remote; m.len=f.length;
        for (unsigned i=0;i<f.length;++i) m.data[i]=f.data[i];
        return driver_.tryToSend(m) ? Result::Ok : Result::Busy;
    }
    Result receive(Frame &f) override {
        if (!started_) return Result::NotStarted;
        driver_.poll(); CANMessage m;
        if (!driver_.receive(m)) return Result::Empty;
        if (m.len>8) return Result::DriverError;
        Frame out; out.id=m.id; out.length=m.len; out.extended=m.ext; out.remote=m.rtr;
        for (unsigned i=0;i<m.len;++i) out.data[i]=m.data[i];
        f=out; return Result::Ok;
    }
    Result pollHealth(Health &h) override {
        if (!started_) return Result::NotStarted;
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
public:
    Mcp2518(SPIClass &spi, std::uint8_t cs, std::uint32_t oscillatorHz)
        : driver_(cs, spi, 255), oscillator_(oscillatorHz) {}
    Result begin(const CommonConfig &c) override {
        if (started_) return Result::AlreadyStarted;
        if (!c.bitrate) return Result::InvalidConfig;
        ACAN2517FDSettings::Oscillator osc;
        if (oscillator_==4000000) osc=ACAN2517FDSettings::OSC_4MHz;
        else if (oscillator_==20000000) osc=ACAN2517FDSettings::OSC_20MHz;
        else if (oscillator_==40000000) osc=ACAN2517FDSettings::OSC_40MHz;
        else return Result::Unsupported;
        ACAN2517FDSettings settings(osc,c.bitrate,DataBitRateFactor::x1);
        settings.mRequestedMode=c.listenOnly ? ACAN2517FDSettings::ListenOnly : ACAN2517FDSettings::Normal20B;
        if (driver_.begin(settings,nullptr)) { driver_.end(); return Result::DriverError; }
        started_=true; return Result::Ok;
    }
    Result end() override {
        if (!started_) return Result::NotStarted;
        if (!driver_.end()) return Result::DriverError;
        started_=false; return Result::Ok;
    }
    Result send(const Frame &f) override {
        if (!started_) return Result::NotStarted;
        if (!valid(f)) return Result::InvalidFrame;
        driver_.poll(); CANFDMessage m;
        m.id=f.id; m.ext=f.extended; m.len=f.length;
        m.type=f.remote ? CANFDMessage::CAN_REMOTE : CANFDMessage::CAN_DATA;
        for (unsigned i=0;i<f.length;++i) m.data[i]=f.data[i];
        return driver_.tryToSend(m) ? Result::Ok : Result::Busy;
    }
    Result receive(Frame &f) override {
        if (!started_) return Result::NotStarted;
        driver_.poll(); CANFDMessage m;
        if (!driver_.receive(m)) return Result::Empty;
        if (m.len>8 || (m.type!=CANFDMessage::CAN_DATA && m.type!=CANFDMessage::CAN_REMOTE)) return Result::Unsupported;
        Frame out; out.id=m.id; out.length=m.len; out.extended=m.ext;
        out.remote=m.type==CANFDMessage::CAN_REMOTE;
        for (unsigned i=0;i<m.len;++i) out.data[i]=m.data[i];
        f=out; return Result::Ok;
    }
    Result pollHealth(Health &h) override {
        if (!started_) return Result::NotStarted;
        driver_.poll(); h=Health{}; h.raw=driver_.errorCounters();
        h.busOff=(h.raw & (1UL<<21))!=0;
        h.errorPassive=(h.raw & ((1UL<<19)|(1UL<<20)))!=0;
        h.receiveLoss=driver_.hardwareReceiveBufferOverflowCount()!=0 || driver_.driverReceiveBufferPeakCount()>32U;
        return Result::Ok;
    }
};
}

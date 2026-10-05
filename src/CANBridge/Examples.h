#pragma once
// Shared implementation for the test sketches; not required for normal Bus use.
#include <Arduino.h>
#include <cstring>
#include <cstdio>
#include "../internal/SerialProtocol.hpp"
namespace canbridge { namespace examples {
enum class Mode { Echo, Periodic, SerialInput, Diagnostic };
template <typename BusType> class Runner {
    BusType &bus_;
    Mode mode_;
    bool ready_ = false, pending_ = false, discard_ = false;
    Frame tx_;
    char input_[40]{};
    unsigned inputLength_ = 0;
    char output_[1024]{};
    unsigned head_ = 0, size_ = 0;
    std::uint32_t dropped_ = 0, sequence_ = 0, lastSend_ = 0, lastHealth_ = 0;
    bool haveHealth_ = false;
    Health health_;
    void line(const char *s) {
        const unsigned length = static_cast<unsigned>(std::strlen(s));
        if (length + 1 > sizeof(output_) - size_) { ++dropped_; return; }
        for (unsigned i = 0; i < length; ++i) output_[(head_ + size_++) % sizeof(output_)] = s[i];
        output_[(head_ + size_++) % sizeof(output_)] = '\n';
    }
    void frameLine(const char *prefix, const Frame &f) {
        char frame[32], text[64]; detail::formatFrame(f, frame);
        std::snprintf(text, sizeof(text), "%s %s", prefix, frame); line(text);
    }
    void flush() {
        int room = Serial.availableForWrite();
        unsigned budget = room > 0 ? static_cast<unsigned>(room) : 0;
        if (budget > 64) budget = 64;
        while (size_ && budget--) { Serial.write(static_cast<std::uint8_t>(output_[head_])); head_ = (head_ + 1) % sizeof(output_); --size_; }
        if (dropped_ && sizeof(output_) - size_ >= 80) {
            char text[80]; std::snprintf(text, sizeof(text), "LOG dropped=%lu (serial output overloaded)", static_cast<unsigned long>(dropped_));
            dropped_ = 0; line(text);
        }
    }
    void input() {
        for (unsigned budget = 0; budget < 32 && Serial.available(); ++budget) {
            const char c = static_cast<char>(Serial.read());
            if (c == '\r') continue;
            if (c == '\n') {
                if (discard_) line("INPUT rejected: line too long");
                else if (inputLength_) {
                    input_[inputLength_] = 0;
                    if (pending_) line("INPUT rejected: transmission pending");
                    else if (!detail::parseFrame(input_, tx_)) line("INPUT invalid: use S 123#1122, E 001ABCDE#0102 or S 123#R8");
                    else pending_ = true;
                }
                inputLength_ = 0; discard_ = false;
            } else if (!discard_) {
                if (inputLength_ + 1 >= sizeof(input_)) discard_ = true;
                else input_[inputLength_++] = c;
            }
        }
    }
    void diagnosticLine(const char *name, const DiagnosticStep &step) {
        char text[150]; std::snprintf(text, sizeof(text), "DIAG %s: %s; %s; raw=0x%08lX", name,
            toString(step.status), toString(step.reason), static_cast<unsigned long>(step.raw)); line(text);
    }
public:
    std::uint32_t periodMs = 1000;
    std::uint32_t periodicId = 0x123;
    Runner(BusType &bus, Mode mode) : bus_(bus), mode_(mode) {}
    template <typename Configuration> void begin(const Configuration &config) {
        if (mode_ == Mode::Diagnostic) {
            DiagnosticReport report;
            const Result result = bus_.diagnose(config, report);
            diagnosticLine("access", report.controllerAccess);
            diagnosticLine("operation", report.controllerOperation);
            diagnosticLine("loopback", report.internalLoopback);
            diagnosticLine("cleanup", report.cleanup);
            char text[100]; std::snprintf(text, sizeof(text), "DIAG result: %s; frames=%lu", toString(result), static_cast<unsigned long>(report.framesChecked)); line(text);
            return;
        }
        const Result result = bus_.begin(config);
        ready_ = result == Result::Ok;
        line(toString(result));
        if (mode_ == Mode::SerialInput) line("INPUT: S 123#1122 / E 001ABCDE#0102 / S 123#R8; newline sends");
        lastSend_ = millis(); lastHealth_ = millis();
    }
    void loop() {
        flush();
        if (!ready_) return;
        if (mode_ == Mode::SerialInput) input();
        const std::uint32_t now = millis();
        if (mode_ == Mode::Periodic && !pending_ && now - lastSend_ >= periodMs) {
            lastSend_ = now; tx_ = Frame{}; tx_.id = periodicId; tx_.length = 4;
            for (unsigned i = 0; i < 4; ++i) tx_.data[i] = static_cast<std::uint8_t>(sequence_ >> (i * 8));
            ++sequence_; pending_ = true;
        }
        // Echo retains its one pending frame. Other modes keep receiving while TX is busy.
        if (mode_ != Mode::Echo || !pending_) {
            for (unsigned budget = 0; budget < 8; ++budget) {
                Frame f;
                const Result rx = bus_.receive(f);
                if (rx == Result::Empty) break;
                if (rx != Result::Ok) { line(toString(rx)); break; }
                if (mode_ == Mode::Echo) { tx_ = f; pending_ = true; break; }
                frameLine("RX", f);
            }
        }
        if (pending_) {
            const Result tx = bus_.send(tx_);
            if (tx == Result::Ok) { if (mode_ != Mode::Echo) frameLine("TX accepted", tx_); pending_ = false; }
            else if (tx != Result::Busy) { line(toString(tx)); ready_ = false; }
        }
        if (now - lastHealth_ >= 500) {
            lastHealth_ = now; Health h;
            const Result result = bus_.pollHealth(h);
            if (result != Result::Ok) line(toString(result));
            else if (!haveHealth_ || h.busOff != health_.busOff || h.errorPassive != health_.errorPassive || h.receiveLoss != health_.receiveLoss) {
                char text[120]; std::snprintf(text, sizeof(text), "HEALTH busOff=%u passive=%u receiveLoss=%u raw=0x%08lX", h.busOff, h.errorPassive, h.receiveLoss, static_cast<unsigned long>(h.raw));
                line(text); health_ = h; haveHealth_ = true;
            }
        }
    }
};
} }

#pragma once
#include <array>
#include <cstdint>
namespace canbridge {
struct Frame {
    std::uint32_t id = 0;
    std::uint8_t length = 0;
    std::array<std::uint8_t, 8> data{};
    bool extended = false;
    bool remote = false;
};
enum class Result { Ok, Empty, Busy, NotStarted, AlreadyStarted,
    InvalidFrame, InvalidConfig, Unsupported, DriverError,
    MissingBitrate, MissingTxPin, MissingRxPin, MissingSpi, MissingCsPin,
    MissingOscillator, InvalidPin, UnsupportedOscillator, AllocationFailed };
inline const char *toString(Result r) {
    switch (r) {
    case Result::Ok: return "Ok";
    case Result::Empty: return "No received frame";
    case Result::Busy: return "Transmit queue is busy";
    case Result::NotStarted: return "CAN is not started";
    case Result::AlreadyStarted: return "CAN is already started";
    case Result::InvalidFrame: return "Invalid CAN frame";
    case Result::InvalidConfig: return "Invalid CAN configuration";
    case Result::Unsupported: return "Configuration is not supported";
    case Result::DriverError: return "CAN driver initialization or operation failed";
    case Result::MissingBitrate: return "Bitrate is not specified";
    case Result::MissingTxPin: return "TX pin is not specified";
    case Result::MissingRxPin: return "RX pin is not specified";
    case Result::MissingSpi: return "SPI is not specified";
    case Result::MissingCsPin: return "CS pin is not specified";
    case Result::MissingOscillator: return "Oscillator frequency is not specified";
    case Result::InvalidPin: return "Invalid pin configuration";
    case Result::UnsupportedOscillator: return "Oscillator frequency is not supported";
    case Result::AllocationFailed: return "Cannot allocate CAN driver";
    }
    return "Unknown CAN result";
}
struct CommonConfig { std::uint32_t bitrate = 0; bool listenOnly = false; };
struct Health {
    bool busOff = false;
    bool errorPassive = false;
    bool receiveLoss = false;
    bool lossDetectionComplete = false;
    std::uint32_t raw = 0;
};
inline bool valid(const Frame &f) {
    return f.length <= 8 && f.id <= (f.extended ? 0x1FFFFFFFU : 0x7FFU);
}
// Methods are called from one application task, never an ISR.
// send(Ok) means accepted, not delivered; receive(Empty) preserves its output.
class Controller {
public:
    virtual ~Controller() = default;
    virtual Result begin(const CommonConfig &) = 0;
    virtual Result end() = 0;
    virtual Result send(const Frame &) = 0;
    virtual Result receive(Frame &) = 0;
    virtual Result pollHealth(Health &) = 0;
    Controller(const Controller &) = delete;
    Controller &operator=(const Controller &) = delete;
protected:
    Controller() = default;
};
}

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
    InvalidFrame, InvalidConfig, Unsupported, DriverError };
struct Config { std::uint32_t bitrate = 1000000; bool listenOnly = false; };
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
    virtual Result begin(const Config &) = 0;
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

#pragma once
#include "../CANBridge/Types.h"
namespace canbridge {
struct CommonConfig { std::uint32_t bitrate = 0; bool listenOnly = false; };
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

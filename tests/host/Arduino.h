#pragma once
#include <cstdint>
#include <string>
#include <deque>
static std::uint32_t testMillis = 0;
inline unsigned long millis() { return testMillis; }
inline void delay(unsigned long ms) { testMillis += ms; }
struct TestSerial {
    std::string output;
    std::deque<char> input;
    int capacity = 64;
    int availableForWrite() { return capacity; }
    unsigned write(std::uint8_t c) { output += static_cast<char>(c); return 1; }
    int available() { return input.size(); }
    int read() { char c = input.front(); input.pop_front(); return c; }
};
static TestSerial Serial;

#pragma once
#include "canbridge/Types.hpp"
#ifdef ARDUINO
#include "canbridge/SpiControllers.hpp"
#ifdef ARDUINO_ARCH_ESP32
#include "canbridge/EspCan.hpp"
#endif
#endif

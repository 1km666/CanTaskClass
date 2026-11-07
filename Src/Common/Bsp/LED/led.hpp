#pragma once

#include <cstdint>

namespace Led {

enum class Color : uint8_t {
    Red = 0,
    Green = 1,
    Blue = 2,
};

// Call after MX_GPIO_Init(); sets all LEDs to the off state.
void init();

// Invalid colors are ignored. on=true turns the selected LED on.
void set(Color color, bool on);

} // namespace Led

#include "led.hpp"
#include "main.h"

namespace {

// Assumed active-low: confirm against the board schematic or hardware.
// Change to GPIO_PIN_SET if the LEDs are active-high.
constexpr GPIO_PinState LED_ON_STATE = GPIO_PIN_RESET;
constexpr GPIO_PinState LED_OFF_STATE =
    LED_ON_STATE == GPIO_PIN_RESET ? GPIO_PIN_SET : GPIO_PIN_RESET;

} // namespace

namespace Led {

void init()
{
    set(Color::Red, false);
    set(Color::Green, false);
    set(Color::Blue, false);
}

void set(Color color, bool on)
{
    const GPIO_PinState state = on ? LED_ON_STATE : LED_OFF_STATE;

    switch (color) {
    case Color::Red:
        HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, state);
        break;
    case Color::Green:
        HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, state);
        break;
    case Color::Blue:
        HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, state);
        break;
    default:
        break;
    }
}

} // namespace Led

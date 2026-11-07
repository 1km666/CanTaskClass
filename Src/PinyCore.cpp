#include "StmLog.hpp"
#include "Bsp.hpp"
#include "PinyCore.hpp"
#include "AppManager.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "led.hpp"

#if SOC_USE_CUSTOM
#include "Soc.hpp"
#endif

void PinyCore::bspInit()
{
#if SOC_USE_CUSTOM
    SOC_CUSTOM_INIT();
#endif

    LOG::Logger::instance().init();

#if BSP_USE_CAN
    if (Can::instance().init() != HAL_OK) {
        Error_Handler();
    }
#endif
#if BSP_USE_LED
    Led::init();
#endif
}

void PinyCore::coreInit()
{
    AppManager::instance().initApp();

    LOG::info("Piny", "kernal start");

    vTaskStartScheduler();
}

void PinyCore::init()
{
    /* Welcome to Piny */
    bspInit();
    coreInit();
}

void initPinyCore() { PinyCore::instance().init(); }

extern "C" void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

extern "C" void vApplicationStackOverflowHook(TaskHandle_t, char *)
{
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

// DWT (Data Watchpoint and Trace) delay utilities for STM32H743
// Michael Kaa
// 27.11.2025

#ifndef DWT_DELAY_H
#define DWT_DELAY_H

#include "stm32h743xx.h"

#ifndef SystemCoreClock
// STM32H743 can run up to 480 MHz, adjust based on your clock configuration
#define SystemCoreClock (480000000U) 
#endif

// DWT (Data Watchpoint and Trace) delay utilities for STM32H743
static inline void dwt_delay_init(void)
{
    // Enable Trace Port and DWT access
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    
    // STM32H7 requires unlock sequence for DWT registers [citation:1][citation:2][citation:8]
    DWT->LAR = 0xC5ACCE55;
    
    // Reset and enable cycle counter
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

// Delay in micro seconds
static inline void dwt_delay_us(uint32_t us) 
{
    uint32_t start_tick = DWT->CYCCNT;
    // Convert microseconds to CPU ticks
    uint32_t us_count_ticks = us * (SystemCoreClock / 1000000U);
    
    // Wait with 32-bit counter overflow consideration
    while ((DWT->CYCCNT - start_tick) < us_count_ticks) {
        // Empty instruction
    }
}

// Delay in milli seconds
static inline void dwt_delay_ms(uint32_t ms) 
{
    for (uint32_t i = 0; i < ms; i++) {
        dwt_delay_us(1000);
    }
}

static inline uint32_t dwt_micros(void) {
    return DWT->CYCCNT / (SystemCoreClock / 1000000U);
}

static inline uint32_t dwt_millis(void) {
    return DWT->CYCCNT / (SystemCoreClock / 1000U);
}

// Check init or not
static inline uint32_t dwt_is_ready(void) {
    return (DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) ? 1 : 0;
}

#endif // DWT_DELAY_H
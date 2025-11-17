/* SPDX-License-Identifier: MIT */
/*
 * dev_mco1.c - Microcontroller Clock Output 1 (MCO1) implementation for STM32H743
 * 
 * Copyright (c) 2025 Michael Kaa
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "inc/dev_mco_prescaler.h"
#include "dev_mco1.h"
#include "stm32h743xx.h"
#include <string.h>
#include <errno.h>

// RCC_MCO1_Clock_Source
#define RCC_MCO1SOURCE_HSI               (0x00000000U)
#define RCC_MCO1SOURCE_LSE               RCC_CFGR_MCO1_0
#define RCC_MCO1SOURCE_HSE               RCC_CFGR_MCO1_1
#define RCC_MCO1SOURCE_PLL1QCLK          ((uint32_t)RCC_CFGR_MCO1_0 | RCC_CFGR_MCO1_1)
#define RCC_MCO1SOURCE_HSI48             RCC_CFGR_MCO1_2

// Default configuration
static dev_mco1_config_t mco1_config = {
    .source = mco1_source_hse,
    .prescaler = mco1_prescaler_1
};

// Convert internal source to register value
static uint32_t source_to_reg(dev_mco1_source_t source) {
    switch (source) {
        case mco1_source_hsi: return RCC_MCO1SOURCE_HSI;
        case mco1_source_lse: return RCC_MCO1SOURCE_LSE;
        case mco1_source_hse: return RCC_MCO1SOURCE_HSE;
        case mco1_source_pll1qclk: return RCC_MCO1SOURCE_PLL1QCLK;
        case mco1_source_hsi48: return RCC_MCO1SOURCE_HSI48;
        default: return RCC_MCO1SOURCE_HSE; // Default to HSE
    }
}

// Convert internal prescaler to register value  
static uint32_t prescaler_to_reg(dev_mco1_prescaler_t prescaler) {
    switch (prescaler) {
        case mco1_prescaler_1: return RCC_MCODIV_1;
        case mco1_prescaler_2: return RCC_MCODIV_2;
        case mco1_prescaler_3: return RCC_MCODIV_3;
        case mco1_prescaler_4: return RCC_MCODIV_4;
        case mco1_prescaler_5: return RCC_MCODIV_5;
        case mco1_prescaler_6: return RCC_MCODIV_6;
        case mco1_prescaler_7: return RCC_MCODIV_7;
        case mco1_prescaler_8: return RCC_MCODIV_8;
        case mco1_prescaler_9: return RCC_MCODIV_9;
        case mco1_prescaler_10: return RCC_MCODIV_10;
        case mco1_prescaler_11: return RCC_MCODIV_11;
        case mco1_prescaler_12: return RCC_MCODIV_12;
        case mco1_prescaler_13: return RCC_MCODIV_13;
        case mco1_prescaler_14: return RCC_MCODIV_14;
        case mco1_prescaler_15: return RCC_MCODIV_15;
        default: return RCC_MCODIV_1; // Default no division
    }
}

// Open MCO1 (interface implementation)
static int mco1_open(void) {
    // Enable GPIOA clock
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;
    
    // Delay after clock enable
    asm("nop");
    asm("nop");

    // Configure PA8 for MCO1
    // Very high speed
    GPIOA->OSPEEDR |= (0x3U << GPIO_OSPEEDR_OSPEED8_Pos);
    
    // Alternate function mode
    GPIOA->MODER &= ~(0x3U << GPIO_MODER_MODE8_Pos);
    GPIOA->MODER |= (0x2U << GPIO_MODER_MODE8_Pos);

    // AF0 for MCO1
    GPIOA->AFR[1] &= ~(0xFU << GPIO_AFRH_AFSEL8_Pos);
    GPIOA->AFR[1] |= (0x0U << GPIO_AFRH_AFSEL8_Pos);

    // Apply configuration
    uint32_t reg_value = source_to_reg(mco1_config.source) | prescaler_to_reg(mco1_config.prescaler);
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE)) | reg_value;

    return 0;
}

// Close MCO1 (interface implementation)
static int mco1_close(void) {
    // Disable MCO1 output
    RCC->CFGR &= ~(RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE);
    
    // Reset GPIO configuration
    GPIOA->MODER &= ~(0x3U << GPIO_MODER_MODE8_Pos);
    GPIOA->OSPEEDR &= ~(0x3U << GPIO_OSPEEDR_OSPEED8_Pos);
    
    return 0;
}

// Read from MCO1 (interface implementation) - not supported
static int mco1_read(void *buf, size_t count) {
    (void)(buf);
    (void)(count);
    return -ENOTSUP;
}

// Write to MCO1 (interface implementation) - not supported  
static int mco1_write(const void *buf, size_t count) {
    (void)(buf);
    (void)(count);
    return -ENOTSUP;
}

// Set MCO1 configuration
static int mco1_set_config(dev_mco1_config_t* config) {
    if (config->source > mco1_source_hsi48) {
        return -EINVAL;
    }

    if (config->prescaler > mco1_prescaler_15) {
        return -EINVAL;
    }

    mco1_config.source = config->source;
    mco1_config.prescaler = config->prescaler;

    // Update hardware if device is open
    uint32_t reg_value = source_to_reg(mco1_config.source) | prescaler_to_reg(mco1_config.prescaler);
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE)) | reg_value;
    
    return 0;
}

// Get MCO1 configuration
static int mco1_get_config(dev_mco1_config_t *config) {
    if (config == NULL) {
        return -EINVAL;
    }
    
    *config = mco1_config;
    return 0;
}

// IO Control for MCO1
static int mco1_ioctrl(int cmd, void *arg) {
    switch (cmd) {
        case MCO1_SET_CONFIG:
            if (arg == NULL) return -EINVAL;
            return mco1_set_config((dev_mco1_config_t *)arg);
            
        case MCO1_GET_CONFIG:
            if (arg == NULL) return -EINVAL;
            return mco1_get_config((dev_mco1_config_t *)arg);
            
        default:
            return -ENOTSUP;
    }
}

// MCO1 device instance
static const interface_t dev_mco1 = {
    .open = mco1_open,
    .close = mco1_close,
    .read = mco1_read,
    .write = mco1_write,
    .ioctrl = mco1_ioctrl
};

// MCO1 device instance accessor
interface_t* dev_mco1_get(void) {
    return (interface_t*)&dev_mco1;
}
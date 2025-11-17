/* SPDX-License-Identifier: MIT */
/*
 * dev_mco2.c - Microcontroller Clock Output 2 (MCO2) implementation for STM32H743
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
#include "dev_mco2.h"
#include "stm32h743xx.h"
#include <string.h>
#include <errno.h>

// RCC_MCO2_Clock_Source
#define RCC_MCO2SOURCE_SYSCLK            (0x00000000U)
#define RCC_MCO2SOURCE_PLL2PCLK          RCC_CFGR_MCO2_0
#define RCC_MCO2SOURCE_HSE               RCC_CFGR_MCO2_1
#define RCC_MCO2SOURCE_PLLCLK            ((uint32_t)RCC_CFGR_MCO2_0 | RCC_CFGR_MCO2_1)
#define RCC_MCO2SOURCE_CSICLK            RCC_CFGR_MCO2_2
#define RCC_MCO2SOURCE_LSICLK            ((uint32_t)RCC_CFGR_MCO2_0 | RCC_CFGR_MCO2_2)

// Default configuration
static dev_mco2_config_t mco2_config = {
    .source = mco2_source_sysclk,
    .prescaler = mco2_prescaler_15  // SYSCLK/15 for 32MHz output from 480MHz
};

// Convert internal source to register value
static uint32_t source_to_reg(dev_mco2_source_t source) {
    switch (source) {
        case mco2_source_sysclk: return RCC_MCO2SOURCE_SYSCLK;
        case mco2_source_pll2pclk: return RCC_MCO2SOURCE_PLL2PCLK;
        case mco2_source_hse: return RCC_MCO2SOURCE_HSE;
        case mco2_source_pllclk: return RCC_MCO2SOURCE_PLLCLK;
        case mco2_source_csiclk: return RCC_MCO2SOURCE_CSICLK;
        case mco2_source_lsiclk: return RCC_MCO2SOURCE_LSICLK;
        default: return RCC_MCO2SOURCE_SYSCLK; // Default to SYSCLK
    }
}

// Convert internal prescaler to register value  
static uint32_t prescaler_to_reg(dev_mco2_prescaler_t prescaler) {
    switch (prescaler) {
        case mco2_prescaler_1: return RCC_MCODIV_1;
        case mco2_prescaler_2: return RCC_MCODIV_2;
        case mco2_prescaler_3: return RCC_MCODIV_3;
        case mco2_prescaler_4: return RCC_MCODIV_4;
        case mco2_prescaler_5: return RCC_MCODIV_5;
        case mco2_prescaler_6: return RCC_MCODIV_6;
        case mco2_prescaler_7: return RCC_MCODIV_7;
        case mco2_prescaler_8: return RCC_MCODIV_8;
        case mco2_prescaler_9: return RCC_MCODIV_9;
        case mco2_prescaler_10: return RCC_MCODIV_10;
        case mco2_prescaler_11: return RCC_MCODIV_11;
        case mco2_prescaler_12: return RCC_MCODIV_12;
        case mco2_prescaler_13: return RCC_MCODIV_13;
        case mco2_prescaler_14: return RCC_MCODIV_14;
        case mco2_prescaler_15: return RCC_MCODIV_15;
        default: return RCC_MCODIV_1; // Default no division
    }
}

// Open MCO2 (interface implementation)
static int mco2_open(void) {
    // Enable GPIOC clock
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN;
    
    // Delay after clock enable
    asm("nop");
    asm("nop");

    // Configure PC9 for MCO2
    // Very high speed
    GPIOC->OSPEEDR |= (0x3U << GPIO_OSPEEDR_OSPEED9_Pos);
    
    // Alternate function mode
    GPIOC->MODER &= ~(0x3U << GPIO_MODER_MODE9_Pos);
    GPIOC->MODER |= (0x2U << GPIO_MODER_MODE9_Pos);
    
    // AF0 for MCO2 (critical!)
    GPIOC->AFR[1] &= ~(0xFU << GPIO_AFRH_AFSEL9_Pos);
    GPIOC->AFR[1] |= (0x0U << GPIO_AFRH_AFSEL9_Pos);
    
    // Apply configuration
    uint32_t reg_value = source_to_reg(mco2_config.source) | (prescaler_to_reg(mco2_config.prescaler) << 7U);
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_MCO2 | RCC_CFGR_MCO2PRE)) | reg_value;
    
    return 0;
}

// Close MCO2 (interface implementation)
static int mco2_close(void) {
    // Disable MCO2 output
    RCC->CFGR &= ~(RCC_CFGR_MCO2 | RCC_CFGR_MCO2PRE);
    
    // Reset GPIO configuration
    GPIOC->MODER &= ~(0x3U << GPIO_MODER_MODE9_Pos);
    GPIOC->OSPEEDR &= ~(0x3U << GPIO_OSPEEDR_OSPEED9_Pos);
    GPIOC->AFR[1] &= ~(0xFU << GPIO_AFRH_AFSEL9_Pos);
    
    return 0;
}

// Read from MCO2 (interface implementation) - not supported
static int mco2_read(void *buf, size_t count) {
    (void)(buf);
    (void)(count);
    return -ENOTSUP;
}

// Write to MCO2 (interface implementation) - not supported  
static int mco2_write(const void *buf, size_t count) {
    (void)(buf);
    (void)(count);
    return -ENOTSUP;
}

// Set MCO2 configuration
static int mco2_set_config(dev_mco2_config_t* config) {
    if (config->source > mco2_source_lsiclk) {
        return -EINVAL;
    }

    if (config->prescaler > mco2_prescaler_15) {
        return -EINVAL;
    }

    mco2_config.source = config->source;
    mco2_config.prescaler = config->prescaler;

    // Update hardware if device is open
    uint32_t reg_value = source_to_reg(mco2_config.source) | (prescaler_to_reg(mco2_config.prescaler) << 7U);
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_MCO2 | RCC_CFGR_MCO2PRE)) | reg_value;
    
    return 0;
}

// Get MCO2 configuration
static int mco2_get_config(dev_mco2_config_t *config) {
    if (config == NULL) {
        return -EINVAL;
    }
    
    *config = mco2_config;
    return 0;
}

// IO Control for MCO2
static int mco2_ioctrl(int cmd, void *arg) {
    switch (cmd) {
        case MCO2_SET_CONFIG:
            if (arg == NULL) return -EINVAL;
            return mco2_set_config((dev_mco2_config_t *)arg);
            
        case MCO2_GET_CONFIG:
            if (arg == NULL) return -EINVAL;
            return mco2_get_config((dev_mco2_config_t *)arg);
            
        default:
            return -ENOTSUP;
    }
}

// MCO2 device instance
static const interface_t dev_mco2 = {
    .open = mco2_open,
    .close = mco2_close,
    .read = mco2_read,
    .write = mco2_write,
    .ioctrl = mco2_ioctrl
};

// MCO2 device instance accessor
interface_t* dev_mco2_get(void) {
    return (interface_t*)&dev_mco2;
}
/* SPDX-License-Identifier: MIT */
/*
 * dev_mco2.h - Microcontroller Clock Output 2 (MCO2) interface for STM32H743
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

#ifndef DEV_MCO2_H
#define DEV_MCO2_H

#include "dev_interface.h"
#include <stdint.h>

// MCO2-specific ioctrl commands
#define MCO2_SET_CONFIG       (INTERFACE_CMD_DEVICE + 0)
#define MCO2_GET_CONFIG       (INTERFACE_CMD_DEVICE + 1)

typedef enum {
    mco2_source_sysclk    = 0,
    mco2_source_pll2pclk  = 1,
    mco2_source_hse       = 2,
    mco2_source_pllclk    = 3,
    mco2_source_csiclk    = 4,
    mco2_source_lsiclk    = 5
} dev_mco2_source_t;

typedef enum {
    mco2_prescaler_1 = 0,
    mco2_prescaler_2,
    mco2_prescaler_3,
    mco2_prescaler_4,
    mco2_prescaler_5,
    mco2_prescaler_6,
    mco2_prescaler_7,
    mco2_prescaler_8,
    mco2_prescaler_9,
    mco2_prescaler_10,
    mco2_prescaler_11,
    mco2_prescaler_12,
    mco2_prescaler_13,
    mco2_prescaler_14,
    mco2_prescaler_15,
} dev_mco2_prescaler_t;

// MCO2 configuration structure
typedef struct {
    dev_mco2_source_t source;
    dev_mco2_prescaler_t prescaler;
} dev_mco2_config_t;

// Global MCO2 device instance accessor
interface_t* dev_mco2_get(void);

#endif /* DEV_MCO2_H */
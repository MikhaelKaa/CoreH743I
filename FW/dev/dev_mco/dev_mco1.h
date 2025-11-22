/* SPDX-License-Identifier: MIT */
/*
 * dev_mco1.h - Microcontroller Clock Output 1 (MCO1) interface for STM32H743
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

#ifndef DEV_MCO1_H
#define DEV_MCO1_H

#include "dev_interface.h"
#include <stdint.h>

// MCO1-specific ioctrl commands
#define MCO1_SET_CONFIG       (INTERFACE_CMD_DEVICE + 0)
#define MCO1_GET_CONFIG       (INTERFACE_CMD_DEVICE + 1)

typedef enum {
    mco1_source_hsi       = 0,
    mco1_source_lse       = 1,
    mco1_source_hse       = 2,
    mco1_source_pll1qclk  = 3,
    mco1_source_hsi48     = 4
}dev_mco1_source_t;

typedef enum {
    mco1_prescaler_1 = 0,
    mco1_prescaler_2,
    mco1_prescaler_3,
    mco1_prescaler_4,
    mco1_prescaler_5,
    mco1_prescaler_6,
    mco1_prescaler_7,
    mco1_prescaler_8,
    mco1_prescaler_9,
    mco1_prescaler_10,
    mco1_prescaler_11,
    mco1_prescaler_12,
    mco1_prescaler_13,
    mco1_prescaler_14,
    mco1_prescaler_15,
} dev_mco1_prescaler_t;

// MCO1 configuration structure
typedef struct {
    dev_mco1_source_t source;
    dev_mco1_prescaler_t prescaler;
} dev_mco1_config_t;

// Global MCO1 device instance accessor
interface_t* dev_mco1_get(void);

#endif /* DEV_MCO1_H */
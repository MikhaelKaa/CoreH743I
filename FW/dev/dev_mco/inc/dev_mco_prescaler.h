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

#ifndef DEV_MCOx_PRESCALER_H
#define DEV_MCOx_PRESCALER_H

#include "stm32h743xx.h"

// RCC_MCOx_Clock_Prescaler
#define RCC_MCODIV_1                    RCC_CFGR_MCO1PRE_0
#define RCC_MCODIV_2                    RCC_CFGR_MCO1PRE_1
#define RCC_MCODIV_3                    ((uint32_t)RCC_CFGR_MCO1PRE_0 | RCC_CFGR_MCO1PRE_1)
#define RCC_MCODIV_4                    RCC_CFGR_MCO1PRE_2
#define RCC_MCODIV_5                    ((uint32_t)RCC_CFGR_MCO1PRE_0 | RCC_CFGR_MCO1PRE_2)
#define RCC_MCODIV_6                    ((uint32_t)RCC_CFGR_MCO1PRE_1 | RCC_CFGR_MCO1PRE_2)
#define RCC_MCODIV_7                    ((uint32_t)RCC_CFGR_MCO1PRE_0 | RCC_CFGR_MCO1PRE_1 | RCC_CFGR_MCO1PRE_2)
#define RCC_MCODIV_8                    RCC_CFGR_MCO1PRE_3
#define RCC_MCODIV_9                    ((uint32_t)RCC_CFGR_MCO1PRE_0 | RCC_CFGR_MCO1PRE_3)
#define RCC_MCODIV_10                   ((uint32_t)RCC_CFGR_MCO1PRE_1 | RCC_CFGR_MCO1PRE_3)
#define RCC_MCODIV_11                   ((uint32_t)RCC_CFGR_MCO1PRE_0 | RCC_CFGR_MCO1PRE_1 | RCC_CFGR_MCO1PRE_3)
#define RCC_MCODIV_12                   ((uint32_t)RCC_CFGR_MCO1PRE_2 | RCC_CFGR_MCO1PRE_3)
#define RCC_MCODIV_13                   ((uint32_t)RCC_CFGR_MCO1PRE_0 | RCC_CFGR_MCO1PRE_2 | RCC_CFGR_MCO1PRE_3)
#define RCC_MCODIV_14                   ((uint32_t)RCC_CFGR_MCO1PRE_1 | RCC_CFGR_MCO1PRE_2 | RCC_CFGR_MCO1PRE_3)
#define RCC_MCODIV_15                   RCC_CFGR_MCO1PRE

#endif /* DEV_MCOx_PRESCALER_H */
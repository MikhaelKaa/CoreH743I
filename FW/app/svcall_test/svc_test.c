/* SPDX-License-Identifier: MIT */
/*
 * svc_test.c - SVC_Handler demo utility
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

typedef struct {
    uint32_t svc_num;   // Извлеченный номер SVC 
    uint32_t args[4];   // Аргументы
} svc_debug_t;

volatile svc_debug_t svc_debug_info = {0};

void print_svc_debug(void);

// Номер svc должен быть константой известной на этапе компиляции
#define SVC_NUM (42)

// Макрос для вызова SVC с номером вызова
#define SVC_CALL(num, arg0, arg1, arg2, arg3) \
    __asm volatile(                           \
        "mov r0, %0\n"                        \
        "mov r1, %1\n"                        \
        "mov r2, %2\n"                        \
        "mov r3, %3\n"                        \
        "svc %4\n"                            \
        :                                     \
        : "r" (arg0), "r" (arg1), "r" (arg2), "r" (arg3), "I" (num) \
        : "r0", "r1", "r2", "r3", "memory"    \
    )
    
#ifdef BAREMETAL
int ucmd_tscv(int argc, char* argv[])
#define ENDL "\r\n"
#else
int main(int argc, char* argv[])
#define ENDL "\n"
#endif // BAREMETAL

{
    uint32_t args[4] = {0};

    if (argc < 2) {
        printf("Usage: scv [arg0] [arg1] [arg2] [arg3]" ENDL);
        printf("  [argN] - decimal args" ENDL);
        return -EINVAL;
    }
    
    // Парсим аргументы
    for (int i = 0; i < (argc - 1); i++) {
        if (sscanf(argv[i+1], "%lu", &args[i]) != 1) {
            printf("EINVAL %d: %s" ENDL, i, argv[i+1]);
            return -EINVAL;
        }
    }
    
    /* Вызываем SVC с переданными аргументами */
    printf("super visor call: num=%u, args=[%lu, %lu, %lu, %lu]" ENDL, SVC_NUM, args[0], args[1], args[2], args[3]);

    // svc_num пока не используем никак - хардкодим 85, оно должно быть константой времени компиляции
    SVC_CALL(SVC_NUM, args[0], args[1], args[2], args[3]);
    
    print_svc_debug(); 

    return 0;
}

void print_svc_debug(void)
{
    printf("svc %lu (0x%02lX)" ENDL, svc_debug_info.svc_num, svc_debug_info.svc_num);
    printf("arg[0]: %lu" ENDL, svc_debug_info.args[0]);
    printf("arg[1]: %lu" ENDL, svc_debug_info.args[1]);
    printf("arg[2]: %lu" ENDL, svc_debug_info.args[2]);
    printf("arg[3]: %lu" ENDL, svc_debug_info.args[3]);
}

void SVC_Handler(void)
{
    __asm volatile(

        "push {r4}\n"

        // Сохраняем аргументы
        "ldr r12, =svc_debug_info\n"
        "str r0, [r12, #4]\n"   // r0
        "str r1, [r12, #8]\n"   // r1
        "str r2, [r12, #12]\n"  // r2
        "str r3, [r12, #16]\n"  // r3

        // MSP or PSP?
        "tst lr, #4\n"
        "ite eq\n"
        "mrseq r4, msp\n"    // MSP
        "mrsne r4, psp\n"    // PSP 
    
        // Получаем PC из стека
        "ldr r12, [r4, #28]\n"   // PC находится по смещению 24 байта (6 слов), плюс 4 байта для r4 (7 word)

        // Получаем адрес инструкции SVC
        "subs r12, #2\n"         // PC указывает на следующую инструкцию после SVC
        
        // Читаем инструкцию SVC (16 бит в Thumb)
        "ldrh r4, [r12]\n"
        
        // Извлекаем номер SVC 
        "and r4, #0xff\n"
        
        // Сохраняем номер SVC 
        "ldr r12, =svc_debug_info\n"
        "str r4, [r12, #0]\n"   // Извлеченный номер SVC

        // <---
        
        // Возвращаемся из обработчика 
        "pop {r4}\n"
        "bx lr\n"
    );
}

#undef ENDL
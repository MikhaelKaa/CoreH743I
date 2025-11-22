
#include <stdio.h>

#include "dev_list.h"
#include "ucmd.h"

int main(void)
{
    const interface_t* uart1 = dev_uart1_get();
    uart1->ioctrl(UART_INIT, NULL);
    setvbuf(stdin, NULL, _IONBF, 0);  // Отключаем буферизацию stdin
    // setvbuf(stdout, NULL, _IONBF, 0); // Отключаем буферизацию stdout
    
    printf("Its work!!!\r\n");

    ucmd_default_init();

    while (1)
    {
        ucmd_default_proc();

        for(volatile int i = 0; i < 100000; i++){
            asm("nop");
            asm("nop");
            asm("nop");
        }
    }
}

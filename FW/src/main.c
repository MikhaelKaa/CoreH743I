
#include <stdio.h>
#include <fcntl.h>
#include <devctl.h>
#include "dev_list.h"
#include "ucmd.h"

#define SDRAM_BANK_ADDR                 (0xD0000000U)
 volatile uint32_t* test = (uint32_t*) SDRAM_BANK_ADDR;
extern  void FMC_Init(void);

int main(void)
{
    const interface_t* uart1 = dev_uart1_get();
    uart1->ioctrl(UART_INIT, NULL);
    setvbuf(stdin, NULL, _IONBF, 0);  // Отключаем буферизацию stdin
    
    printf("Its start!!!\r\n");
    
    ucmd_default_init();

    FMC_Init();

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

void HardFault_Handler(void)
{
  while (1)
  {
    asm("nop");
  }
}


#include <stdio.h>
#include <fcntl.h>
#include <devctl.h>
#include "dev_list.h"
#include "ucmd.h"

#include "dwt_delay.h"

#define SDRAM_BANK_ADDR                 (0xD0000000U)
 volatile uint32_t* test = (uint32_t*) SDRAM_BANK_ADDR;
extern  void FMC_Init(void);

int main(void)
{
    // SCB_EnableICache();
    // SCB_EnableDCache();

    const interface_t* uart1 = dev_uart1_get();
    uart1->ioctrl(UART_INIT, NULL);
    setvbuf(stdin, NULL, _IONBF, 0);  // Отключаем буферизацию stdin
    
    printf("Its start!!!\r\n");
    
    ucmd_default_init();

    FMC_Init();

    dwt_delay_init();

    while (1)
    {
        ucmd_default_proc();

        dwt_delay_ms(1);
    }
}

void HardFault_Handler(void)
{
  while (1)
  {
    asm("nop");
  }
}

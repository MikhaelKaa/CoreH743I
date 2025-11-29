
#include <stdio.h>
#include <fcntl.h>
#include <devctl.h>

#include "dev_list.h"
#include "ucmd.h"
#include "dwt_delay.h"

int main(void)
{
    // SCB_EnableICache();
    // SCB_EnableDCache();

    dwt_delay_init();

    const interface_t* uart1 = dev_uart1_get();
    uart1->ioctrl(UART_INIT, NULL);
    setvbuf(stdin, NULL, _IONBF, 0);  // Отключаем буферизацию stdin
    
    printf("System start\r\n");

    dev_memory_print_info();

    dev_memory_t* mem_info = NULL;
    dev_memory_get()->ioctrl(MEMORY_GET_INFO, &mem_info);
    size_t pointer = mem_info->regions[6].start+8;
    dev_memory_get()->ioctrl(MEMORY_SET_ADDRESS, &pointer);
    uint8_t test_data[300] = {0};
    for(size_t  i = 0; i < 257; i++){
        test_data[i] = (uint8_t)i;
    }
    dev_memory_get()->write(test_data, 0xff);
  
    // const interface_t* mem = dev_memory_get();

    // // Установить адрес во Flash и записать данные
    // uint32_t address = (0x08000000U + 256U*1024U) + 256;
    // // uint32_t address = 0x08010000;
    // uint32_t data[4] = {0x12345678, 0xABCDEF00, 0x11223344, 0x55667788};

    // mem->ioctrl(MEMORY_SET_ADDRESS, &address);
    // mem->write(data, sizeof(data)); // Автоматически: прочитает сектор, обновит данные, сотрет сектор, запишет обратно

    // mem dump 08040000 100

    ucmd_default_init();

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

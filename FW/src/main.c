
#include <stdio.h>
#include <fcntl.h>
#include <devctl.h>

#include "dev_list.h"
#include "ucmd.h"
#include "dwt_delay.h"
#include "memory_man.h"

static const char test_array[] = {"lorem ipsum test arrray data with some random data kdjvnkdfjsbvnldksjbnkdgjbn"};

int main(void)
{
    // SCB_EnableICache();
    // SCB_EnableDCache();

    dwt_delay_init();

    const interface_t* uart1 = dev_uart1_get();
    uart1->ioctl(UART_INIT, NULL);
    setvbuf(stdin, NULL, _IONBF, 0);  // Отключаем буферизацию stdin
    
    printf("System start\r\n");
    printf("test_array adr: %08lx\r\n", (uint32_t)test_array);
    dev_memory_print_info();

    #ifdef TEST_WRITE_SDRAM
    dev_memory_t* mem_info = NULL;
    dev_memory_get()->ioctl(MEMORY_GET_INFO, &mem_info);
    size_t pointer = mem_info->regions[6].start+8;
    dev_memory_get()->ioctl(MEMORY_SET_ADDRESS, &pointer);
    uint8_t test_data[300] = {0};
    for(size_t  i = 0; i < 257; i++){
        test_data[i] = (uint8_t)i;
    }
    dev_memory_get()->write(test_data, 0xff);
    #endif // TEST_WRITE_SDRAM

    // #define TEST_WRITE_FLASH
    #ifdef TEST_WRITE_FLASH
    const interface_t* mem = dev_memory_get();
    // Установить адрес во Flash и записать данные
    uint32_t address = (0x08000000U + 256U*1024U);
    // uint32_t address = 0x08010000;
    uint32_t data[4] = {0xbebebebe, 0xABCDEF00, 0x11223344, 0x55667788};
    mem->ioctl(MEMORY_SET_ADDRESS, &address);
    int mem_result = mem->write(data, sizeof(data)); // Автоматически: прочитает сектор, обновит данные, сотрет сектор, запишет обратно
    printf("mem->write ret %d\r\n", mem_result);
    #endif // TEST_WRITE_FLASH

    // mem dump 08040000 100
    // mem write 08040007 57
    // mem cpy 08040000 0800877c 64
    mem_set_interface((interface_t *)dev_memory_get());

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

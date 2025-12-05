
#include <stdio.h>
#include <fcntl.h>
#include <devctl.h>

#include "dev_list.h"
#include "ucmd.h"
#include "dwt_delay.h"
#include "memory_man.h"
#include "vfs.h"

void mpu_init(void);

int main(void)
{
	// mpu_init(); нужно изучить как инициализировать и использовать
    SCB_EnableICache();
    // SCB_EnableDCache();
	// __DSB();
    __ISB();

    dwt_delay_init();
    uint32_t dwt_time = dwt_micros();
	
    const interface_t* uart1 = dev_uart1_get();
    uart1->ioctl(UART_INIT, NULL);
    setvbuf(stdin, NULL, _IONBF, 0);  // Отключаем буферизацию stdin
    
    printf("System start\r\n");
    printf("start time: %lu\r\n", dwt_time);
    #ifdef TEST_MEM_INFO
	static const char test_array[] = {"lorem ipsum test arrray data with some random data kdjvnkdfjsbvnldksjbnkdgjbn"};
    printf("test_array adr: %08lx\r\n", (uint32_t)test_array);
    dev_memory_print_info();
    #endif // TEST_MEM_INFO
    
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
    
    vfs_bootstrap();

    printf("CPUID: 0x%08lx\r\n", SCB->CPUID);

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



#define MPU_REGION_ENABLE      (1U << 0U)
#define MPU_SIZE_2MB           (20U << 1U)   // 2^(20+1) = 2MB
#define MPU_SIZE_128KB         (15U << 1U)   // 2^(16+1) = 64KB
#define MPU_SIZE_512KB         (18U << 1U)   // 2^(18+1) = 512KB
#define MPU_AP_FULL_ACCESS     (0x3U << 3U)
#define MPU_TEX_NORMAL         (0x1U << 16U)
#define MPU_TEX_DEVICE         (0x0U << 16U)
#define MPU_SHAREABLE          (1U << 17U)
#define MPU_CACHEABLE          (1U << 18U)
#define MPU_BUFFERABLE         (1U << 19U)

void mpu_init(void)
{
    MPU->CTRL = 0U;
    
    // Flash
    MPU->RNR = 0U;
    MPU->RBAR = 0x08000000U;
    MPU->RASR = MPU_SIZE_2MB | MPU_AP_FULL_ACCESS | MPU_TEX_NORMAL | 
                MPU_SHAREABLE | MPU_CACHEABLE | MPU_REGION_ENABLE;
    
    // DTCM (стек)
    MPU->RNR = 1U;
    MPU->RBAR = 0x20000000U;
    MPU->RASR = MPU_SIZE_128KB | MPU_AP_FULL_ACCESS | MPU_TEX_DEVICE |
                MPU_BUFFERABLE | MPU_REGION_ENABLE;
    
    // AXI RAM (данные)
    MPU->RNR = 2U;
    MPU->RBAR = 0x24000000U;
    MPU->RASR = MPU_SIZE_512KB | MPU_AP_FULL_ACCESS | MPU_TEX_NORMAL |
                MPU_CACHEABLE | MPU_REGION_ENABLE;
    
    MPU->CTRL = MPU_CTRL_ENABLE_Msk | MPU_CTRL_PRIVDEFENA_Msk;
    // __DSB();
    // __ISB();
}
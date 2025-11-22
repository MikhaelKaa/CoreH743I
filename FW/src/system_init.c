#include "stm32h743xx.h"
#include "dev_mco1.h"
#include "dev_mco2.h"

uint32_t SystemCoreClock = 0;

void enable_mco1(void);
void enable_mco2(void);

void SystemInit(void)
{
    // Set flash latency to 4 wait states
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY_4WS;
    while((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_4WS) { }

    // Configure power supply for LDO
    PWR->CR3 = (PWR->CR3 & ~(PWR_CR3_SCUEN | PWR_CR3_LDOEN | PWR_CR3_BYPASS)) | PWR_CR3_LDOEN;

    // Set voltage scaling to scale 0
    PWR->D3CR = (PWR->D3CR & ~PWR_D3CR_VOS) | (PWR_D3CR_VOS_0 | PWR_D3CR_VOS_1);
    while ((PWR->D3CR & PWR_D3CR_VOSRDY) == 0) { }

    // Enable HSE oscillator
    RCC->CR |= RCC_CR_HSEON;
    while((RCC->CR & RCC_CR_HSERDY) == 0) { }

    // Enable backup domain access
    PWR->CR1 |= PWR_CR1_DBP;

    // Configure LSE drive capability to low
    RCC->BDCR = (RCC->BDCR & ~RCC_BDCR_LSEDRV) | (0x00000000U);
    
    // Enable LSE oscillator
    RCC->BDCR |= RCC_BDCR_LSEON;
    while((RCC->BDCR & RCC_BDCR_LSERDY) == 0) { }

    // Configure PLL source to HSE
    RCC->PLLCKSELR = (RCC->PLLCKSELR & ~RCC_PLLCKSELR_PLLSRC) | RCC_PLLCKSELR_PLLSRC_HSE;

    // Enable PLL1 outputs
    RCC->PLLCFGR |= RCC_PLLCFGR_DIVP1EN;
    RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ1EN;
    RCC->PLLCFGR |= RCC_PLLCFGR_DIVR1EN;

    // Configure PLL1 VCO input range (8-16 MHz)
    RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLL1RGE) | (0x00000003U << RCC_PLLCFGR_PLL1RGE_Pos);

    // Configure PLL1 VCO output range (wide)
    RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLL1VCOSEL) | (0x00000000U << RCC_PLLCFGR_PLL1VCOSEL_Pos);

    // Configure PLL1 dividers
    RCC->PLLCKSELR = (RCC->PLLCKSELR & ~RCC_PLLCKSELR_DIVM1) | (2U << RCC_PLLCKSELR_DIVM1_Pos);
    RCC->PLL1DIVR = (RCC->PLL1DIVR & ~RCC_PLL1DIVR_N1) | ((80U - 1UL) << RCC_PLL1DIVR_N1_Pos);
    RCC->PLL1DIVR = (RCC->PLL1DIVR & ~RCC_PLL1DIVR_P1) | ((2U - 1UL) << RCC_PLL1DIVR_P1_Pos);
    RCC->PLL1DIVR = (RCC->PLL1DIVR & ~RCC_PLL1DIVR_Q1) | ((15U - 1UL) << RCC_PLL1DIVR_Q1_Pos);
    RCC->PLL1DIVR = (RCC->PLL1DIVR & ~RCC_PLL1DIVR_R1) | ((2U - 1UL) << RCC_PLL1DIVR_R1_Pos);

    // Enable PLL1
    RCC->CR |= RCC_CR_PLL1ON;
    while((RCC->CR & RCC_CR_PLL1RDY) == 0) { }

    // Set AHB prescaler to 2 for frequencies above 80 MHz
    RCC->D1CFGR = (RCC->D1CFGR & ~RCC_D1CFGR_HPRE) | RCC_D1CFGR_HPRE_DIV2;

    // Set system clock source to PLL1
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL1;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL1) { }

    // Configure system and bus prescalers
    RCC->D1CFGR = (RCC->D1CFGR & ~RCC_D1CFGR_D1CPRE) | RCC_D1CFGR_D1CPRE_DIV1;
    RCC->D1CFGR = (RCC->D1CFGR & ~RCC_D1CFGR_HPRE) | RCC_D1CFGR_HPRE_DIV2;
    RCC->D2CFGR = (RCC->D2CFGR & ~RCC_D2CFGR_D2PPRE1) | RCC_D2CFGR_D2PPRE1_DIV2;
    RCC->D2CFGR = (RCC->D2CFGR & ~RCC_D2CFGR_D2PPRE2) | RCC_D2CFGR_D2PPRE2_DIV2;
    RCC->D1CFGR = (RCC->D1CFGR & ~RCC_D1CFGR_D1PPRE) | RCC_D1CFGR_D1PPRE_DIV2;
    RCC->D3CFGR = (RCC->D3CFGR & ~RCC_D3CFGR_D3PPRE) | RCC_D3CFGR_D3PPRE_DIV2;

    SystemCoreClock = 480000000U;

    // Есть взаимное влияние делителей MCO, вероятно ошибка в том, что mco2_prescaler максимум 8, но не проверял...
    // enable mco1
    // dev_mco1_config_t mco1_setings = {.source = mco1_source_hse, .prescaler = mco1_prescaler_4};
    // interface_t* dev_mco1 = dev_mco1_get();
    // dev_mco1->ioctrl(MCO1_SET_CONFIG, &mco1_setings);
    // dev_mco1->open();

    // enable mco2
    dev_mco2_config_t mco2_settings = {.source =  mco2_source_pllclk, .prescaler =  mco2_prescaler_15};
    interface_t* dev_mco2 = dev_mco2_get();
    dev_mco2->ioctrl(MCO2_SET_CONFIG, &mco2_settings);
    dev_mco2->open();
    
}

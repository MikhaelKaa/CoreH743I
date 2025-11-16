#include "stm32h743xx.h"

void enable_mco1(void);
void enable_mco2(void);

void SystemInit(void)
{
    RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;
    
    RCC->CR |= RCC_CR_HSEON;
    
    uint32_t timeout = 1000000;
    while(!(RCC->CR & RCC_CR_HSERDY)) {
        if (timeout-- == 0) {
            
            RCC->CR |= RCC_CR_HSION;
            while(!(RCC->CR & RCC_CR_HSIRDY)) {}
            break;
        }
    }
    
    
    PWR->D3CR = (PWR->D3CR & ~PWR_D3CR_VOS) | PWR_D3CR_VOS_0;
    timeout = 1000000;
    while(!(PWR->D3CR & PWR_D3CR_VOSRDY)) {
        if (timeout-- == 0) break;
    }
    
    
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY_4WS;
    
    
    RCC->CR &= ~RCC_CR_PLL1ON;
    timeout = 1000000;
    while((RCC->CR & RCC_CR_PLL1RDY)) {
        if (timeout-- == 0) break;
    }
    
    
    // PLLM = 2, PLLN = 80, PLLP = 2 -> (24/2)*80/2 = 480 МГц
    RCC->PLLCKSELR = (0x1 << RCC_PLLCKSELR_PLLSRC_Pos) |  // HSE как источник PLL
                     (0x1 << RCC_PLLCKSELR_DIVM1_Pos);    // PLLM = 2
    
    RCC->PLLCFGR = (0x3 << RCC_PLLCFGR_PLL1RGE_Pos) |     // Диапазон 8-16 МГц
                   (0x0 << RCC_PLLCFGR_PLL1VCOSEL_Pos);   // Широкий диапазон VCO
    
    RCC->PLL1DIVR = (79 << RCC_PLL1DIVR_N1_Pos) |         // PLLN = 80 (79 + 1)
                    (1 << RCC_PLL1DIVR_P1_Pos) |          // PLLP = 2 (1 + 1)
                    (14 << RCC_PLL1DIVR_Q1_Pos) |         // PLLQ = 15 (14 + 1)
                    (1 << RCC_PLL1DIVR_R1_Pos);           // PLLR = 2 (1 + 1)
    
    RCC->PLL1FRACR = 0;  // Дробная часть = 0
    
    // Включаем выходы PLL
    // RCC->PLLCFGR |= RCC_PLLCFGR_PLL1REN | RCC_PLLCFGR_PLL1QEN | RCC_PLLCFGR_PLL1PEN;
    
    
    RCC->CR |= RCC_CR_PLL1ON;
    
    timeout = 1000000;
    while(!(RCC->CR & RCC_CR_PLL1RDY)) {
        if (timeout-- == 0) {
            
            RCC->CR &= ~RCC_CR_PLL1ON;
            break;
        }
    }
    
    RCC->D1CFGR = (0x0 << RCC_D1CFGR_D1CPRE_Pos) |     // SYSCLK не делится
                  (0x8 << RCC_D1CFGR_HPRE_Pos) |       // AHB /2 = 240 МГц
                  (0x4 << RCC_D1CFGR_D1PPRE_Pos);      // APB3 /2 = 120 МГц
    
    RCC->D2CFGR = (0x4 << RCC_D2CFGR_D2PPRE1_Pos) |    // APB1 /2 = 120 МГц
                  (0x4 << RCC_D2CFGR_D2PPRE2_Pos);     // APB2 /2 = 120 МГц
    
    RCC->D3CFGR = (0x4 << RCC_D3CFGR_D3PPRE_Pos);      // APB4 /2 = 120 МГц
    
    
    if (RCC->CR & RCC_CR_PLL1RDY) {
        RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | (0x3 << RCC_CFGR_SW_Pos);
        timeout = 1000000;
        while((RCC->CFGR & RCC_CFGR_SWS) != (0x3 << RCC_CFGR_SWS_Pos)) {
            if (timeout-- == 0) break;
        }
    }
    
    enable_mco1();
    enable_mco2();
}



// RCC_MCOx_Clock_Prescaler  RCC MCOx Clock Prescaler
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

// RCC_MCO1_Clock_Source  RCC MCO1 Clock Source
#define RCC_MCO1SOURCE_HSI               (0x00000000U)
#define RCC_MCO1SOURCE_LSE               RCC_CFGR_MCO1_0
#define RCC_MCO1SOURCE_HSE               RCC_CFGR_MCO1_1
#define RCC_MCO1SOURCE_PLL1QCLK          ((uint32_t)RCC_CFGR_MCO1_0 | RCC_CFGR_MCO1_1)
#define RCC_MCO1SOURCE_HSI48              RCC_CFGR_MCO1_2

// RCC_MCO2_Clock_Source  RCC MCO2 Clock Source
#define RCC_MCO2SOURCE_SYSCLK            (0x00000000U)
#define RCC_MCO2SOURCE_PLL2PCLK          RCC_CFGR_MCO2_0
#define RCC_MCO2SOURCE_HSE               RCC_CFGR_MCO2_1
#define RCC_MCO2SOURCE_PLLCLK            ((uint32_t)RCC_CFGR_MCO2_0 | RCC_CFGR_MCO2_1)
#define RCC_MCO2SOURCE_CSICLK            RCC_CFGR_MCO2_2
#define RCC_MCO2SOURCE_LSICLK            ((uint32_t)RCC_CFGR_MCO2_0 | RCC_CFGR_MCO2_2)

// 
#define WRITE_REG(REG, VAL)   ((REG) = (VAL))
#define READ_REG(REG)         ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

// HSE - 24Mhz -> MCO1 (PA8)
void enable_mco1(void)
{
    // Enable GPIOA clock
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;

    // Delay after cloxk enable
    asm("nop");

    // Very high speed
    GPIOA->OSPEEDR |= (0x3U << GPIO_OSPEEDR_OSPEED8_Pos);

    // Alternate function mode
    GPIOA->MODER &= ~(0x3U << GPIO_MODER_MODE8_Pos);
    GPIOA->MODER |= (0x2U << GPIO_MODER_MODE8_Pos);

    // MCO1 source = HSE, no divider
    MODIFY_REG(RCC->CFGR, (RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE), (RCC_MCO1SOURCE_HSE | RCC_MCODIV_1));
}

// SYSCLK/15 -> MCO2 (PC9)
void enable_mco2(void)
{
    // Enable GPIOC clock
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN;

    // Delay after clock enable
    asm("nop");

    // Very high speed
    GPIOC->OSPEEDR |= (0x3U << GPIO_OSPEEDR_OSPEED9_Pos);

    // Alternate function mode
    GPIOC->MODER &= ~(0x3U << GPIO_MODER_MODE9_Pos);
    GPIOC->MODER |= (0x2U << GPIO_MODER_MODE9_Pos);

    // AF0 для MCO2 (это критически важно!)
    GPIOC->AFR[1] &= ~(0xFU << GPIO_AFRH_AFSEL9_Pos);
    GPIOC->AFR[1] |= (0x0U << GPIO_AFRH_AFSEL9_Pos);

    // MCO2 source = SYSCLK, divider 15 -> 480/15 = 32MHz
    MODIFY_REG(RCC->CFGR, (RCC_CFGR_MCO2 | RCC_CFGR_MCO2PRE), (RCC_MCO2SOURCE_SYSCLK | (RCC_MCODIV_15 << 7U)));
}
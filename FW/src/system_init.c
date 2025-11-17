#include "stm32h743xx.h"
#include "dev_mco1.h"
#include "dev_mco2.h"


#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))
#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)    ((REG) & (BIT))
#define CLEAR_REG(REG)        ((REG) = (0x0))
#define WRITE_REG(REG, VAL)   ((REG) = (VAL))
#define READ_REG(REG)         ((REG))
#define UNUSED(x) ((void)(x))

#define PWR_SUPPLY_CONFIG_MASK (PWR_CR3_SCUEN | PWR_CR3_LDOEN | PWR_CR3_BYPASS)
#define PWR_LDO_SUPPLY                      PWR_CR3_LDOEN  
#define PWR_FLAG_ACTVOSRDY  (0x0DU)

#define __HAL_PWR_VOLTAGESCALING_CONFIG(__REGULATOR__)                       \
do {                                                                         \
    __IO uint32_t tmpreg = 0x00;                                             \
    /* Configure the Voltage Scaling */                                      \
    MODIFY_REG (PWR->D3CR, PWR_D3CR_VOS, (__REGULATOR__));                   \
    /* Delay after setting the voltage scaling */                            \
    tmpreg = READ_BIT(PWR->D3CR, PWR_D3CR_VOS);                              \
    UNUSED(tmpreg);                                                          \
} while(0)
#define PWR_REGULATOR_VOLTAGE_SCALE0  (0U)
#define PWR_REGULATOR_VOLTAGE_SCALE1  (PWR_D3CR_VOS_1 | PWR_D3CR_VOS_0)
#define PWR_FLAG_VOSRDY     (0x10U)


#define RCC_FLAG_MASK  ((uint8_t)0x1F)
#define __HAL_RCC_GET_FLAG(__FLAG__) (((((((__FLAG__) >> 5U) == 1U)? RCC->CR :((((__FLAG__) >> 5U) == 2U) ? RCC->BDCR : \
((((__FLAG__) >> 5U) == 3U)? RCC->CSR : ((((__FLAG__) >> 5U) == 4U)? RCC->RSR :RCC->CIFR))))  & (1UL << ((__FLAG__) & RCC_FLAG_MASK)))!= 0U)? 1U : 0U)

#define RCC_FLAG_PLLRDY                ((uint8_t)0x39)

#define __HAL_RCC_PLL_CONFIG(__RCC_PLLSOURCE__, __PLLM1__, __PLLN1__, __PLLP1__, __PLLQ1__,__PLLR1__ ) \
                  do{ MODIFY_REG(RCC->PLLCKSELR, (RCC_PLLCKSELR_PLLSRC | RCC_PLLCKSELR_DIVM1) , ((__RCC_PLLSOURCE__) | ( (__PLLM1__) <<4U)));  \
                      WRITE_REG (RCC->PLL1DIVR , ( (((__PLLN1__) - 1U )& RCC_PLL1DIVR_N1) | ((((__PLLP1__) -1U ) << 9U) & RCC_PLL1DIVR_P1) | \
                                ((((__PLLQ1__) -1U) << 16U)& RCC_PLL1DIVR_Q1) | ((((__PLLR1__) - 1U) << 24U)& RCC_PLL1DIVR_R1))); \
                    } while(0)

#define RCC_PLLSOURCE_HSE              (0x00000002U)



void SystemInit(void)
{

    MODIFY_REG (PWR->CR3, PWR_SUPPLY_CONFIG_MASK, PWR_LDO_SUPPLY);

    SET_BIT(RCC->CR, RCC_CR_HSEON);
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");

    while (((PWR->CSR1 & PWR_CSR1_ACTVOSRDY) == PWR_CSR1_ACTVOSRDY) == 0U) {
        asm("nop");
    }

    SET_BIT(RCC->APB4ENR, RCC_APB4ENR_SYSCFGEN);
    asm("nop");
    
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
    
    // PLL конфигурация - твой оригинальный код
    CLEAR_BIT(RCC->CR, RCC_CR_PLL1ON);
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) != 0U)
    {
        asm("nop");
    }

    __HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, 2, 80, 2, 15, 2);

    CLEAR_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLL1FRACEN);
    MODIFY_REG(RCC->PLL1FRACR, RCC_PLL1FRACR_FRACN1, (uint32_t)(0U) << RCC_PLL1FRACR_FRACN1_Pos);

    MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLL1RGE, (RCC_PLLCFGR_PLL1RGE_3));
    MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLL1VCOSEL, (0x00000000U));

    SET_BIT(RCC->PLLCFGR, (RCC_PLLCFGR_DIVP1EN | RCC_PLLCFGR_DIVQ1EN | RCC_PLLCFGR_DIVR1EN));
    SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLL1FRACEN);

    SET_BIT(RCC->CR, RCC_CR_PLL1ON);

    while (__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) == 0U)
    {
        asm("nop");
    }
    
    
    // Переключаем SYSCLK на PLL
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL1);
    
    // Ждём переключения
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL1) {
        asm("nop");
    }

    dev_mco1_config_t mco1_setings = {.source = mco1_source_hsi, .prescaler = mco1_prescaler_1};
    interface_t* dev_mco1 = dev_mco1_get();
    dev_mco1->ioctrl(MCO1_SET_CONFIG, &mco1_setings);
    dev_mco1->open();

    dev_mco2_config_t mco2_settings = {.source =  mco2_source_sysclk, .prescaler =  mco2_prescaler_15};
    interface_t* dev_mco2 = dev_mco2_get();
    dev_mco2->ioctrl(MCO1_SET_CONFIG, &mco2_settings);
    dev_mco2->open();
}

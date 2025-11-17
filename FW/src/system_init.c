#include "stm32h743xx.h"
#include "dev_mco1.h"
#include "dev_mco2.h"

#define SET_BIT(REG, BIT)     ((REG) |= (BIT))

#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))

#define READ_BIT(REG, BIT)    ((REG) & (BIT))

#define CLEAR_REG(REG)        ((REG) = (0x0))

#define WRITE_REG(REG, VAL)   ((REG) = (VAL))

#define READ_REG(REG)         ((REG))

#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

#define POSITION_VAL(VAL)     (__CLZ(__RBIT(VAL)))


#define LL_RCC_LSEDRIVE_LOW                (uint32_t)(0x00000000U)
#define LL_RCC_LSEDRIVE_MEDIUMLOW          (uint32_t)(RCC_BDCR_LSEDRV_0)
#define LL_RCC_LSEDRIVE_MEDIUMHIGH         (uint32_t)(RCC_BDCR_LSEDRV_1)
#define LL_RCC_LSEDRIVE_HIGH               (uint32_t)(RCC_BDCR_LSEDRV)
#define LL_RCC_PLLINPUTRANGE_8_16          (uint32_t)(0x00000003)
#define LL_RCC_PLLVCORANGE_WIDE            (uint32_t)(0x00000000U)      /* VCO output range: 192 to 836 MHz   OR  128 to 544 MHz (*) */
#define LL_RCC_PLLVCORANGE_MEDIUM          (uint32_t)(0x00000001)       /* VCO output range: 150 to 420 MHz */


uint32_t SystemCoreClock = 0;

void enable_mco1(void);
void enable_mco2(void);


void SystemInit(void)
{
//   LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
  MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_4WS);


  while(((uint32_t)(READ_BIT(FLASH->ACR, FLASH_ACR_LATENCY))) != FLASH_ACR_LATENCY_4WS)  {  }

//   LL_PWR_ConfigSupply(LL_PWR_LDO_SUPPLY);
  MODIFY_REG(PWR->CR3, (PWR_CR3_SCUEN | PWR_CR3_LDOEN | PWR_CR3_BYPASS), PWR_CR3_LDOEN);

//   LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE0);
  MODIFY_REG(PWR->D3CR, PWR_D3CR_VOS, (PWR_D3CR_VOS_0 | PWR_D3CR_VOS_1));


  while ((((READ_BIT(PWR->D3CR, PWR_D3CR_VOSRDY) == (PWR_D3CR_VOSRDY)) ? 1UL : 0UL)) == 0)
  {
  }

//   LL_RCC_HSE_Enable();
  SET_BIT(RCC->CR, RCC_CR_HSEON);

   /* Wait till HSE is ready */
  while((((READ_BIT(RCC->CR, RCC_CR_HSERDY) == (RCC_CR_HSERDY)) ? 1UL : 0UL)) != 1)
  {

  }


    // LL_PWR_EnableBkUpAccess();
    SET_BIT(PWR->CR1, PWR_CR1_DBP);

    // LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);
    MODIFY_REG(RCC->BDCR, RCC_BDCR_LSEDRV, LL_RCC_LSEDRIVE_LOW);

    // LL_RCC_LSE_Enable();
    SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);

    /* Wait till LSE is ready */
    while((((READ_BIT(RCC->BDCR, RCC_BDCR_LSERDY) == (RCC_BDCR_LSERDY)) ? 1UL : 0UL)) != 1)  {  }


    // LL_RCC_PLL_SetSource(LL_RCC_PLLSOURCE_HSE);
    MODIFY_REG(RCC->PLLCKSELR, RCC_PLLCKSELR_PLLSRC, RCC_PLLCKSELR_PLLSRC_HSE);

    // LL_RCC_PLL1P_Enable();
    SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_DIVP1EN);

    // LL_RCC_PLL1Q_Enable();
    SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_DIVQ1EN);

    // LL_RCC_PLL1R_Enable();
    SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_DIVR1EN);

    // LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
    MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLL1RGE, LL_RCC_PLLINPUTRANGE_8_16 << RCC_PLLCFGR_PLL1RGE_Pos);

    // LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
    MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLL1VCOSEL, LL_RCC_PLLVCORANGE_WIDE << RCC_PLLCFGR_PLL1VCOSEL_Pos);

    // LL_RCC_PLL1_SetM(2);
    MODIFY_REG(RCC->PLLCKSELR, RCC_PLLCKSELR_DIVM1, 2U << RCC_PLLCKSELR_DIVM1_Pos);

    // LL_RCC_PLL1_SetN(80);
    MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_N1, (80U - 1UL) << RCC_PLL1DIVR_N1_Pos);

    // LL_RCC_PLL1_SetP(2);
    MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_P1, (2U - 1UL) << RCC_PLL1DIVR_P1_Pos);

    // LL_RCC_PLL1_SetQ(15);
    MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_Q1, (15U - 1UL) << RCC_PLL1DIVR_Q1_Pos);

    // LL_RCC_PLL1_SetR(2);
    MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_R1, (2U - 1UL) << RCC_PLL1DIVR_R1_Pos);

    // LL_RCC_PLL1_Enable();
    SET_BIT(RCC->CR, RCC_CR_PLL1ON);

    /* Wait till PLL is ready */
    while((((READ_BIT(RCC->CR, RCC_CR_PLL1RDY) == (RCC_CR_PLL1RDY)) ? 1UL : 0UL)) != 1)
    {
    }

    /* Intermediate AHB prescaler 2 when target frequency clock is higher than 80 MHz */
    // LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);
    MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_HPRE, RCC_D1CFGR_HPRE_DIV2);

    // LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL1);

    /* Wait till System clock is ready */
    while(((uint32_t)(READ_BIT(RCC->CFGR, RCC_CFGR_SWS))) != RCC_CFGR_SWS_PLL1)    {    }

    // LL_RCC_SetSysPrescaler(LL_RCC_SYSCLK_DIV_1);
    MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_D1CPRE, RCC_D1CFGR_D1CPRE_DIV1);

    // LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);
    MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_HPRE, RCC_D1CFGR_HPRE_DIV2);

    // LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1, RCC_D2CFGR_D2PPRE1_DIV2);

    // LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
    MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2, RCC_D2CFGR_D2PPRE2_DIV2);

    // LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_2);
    MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_D1PPRE, RCC_D1CFGR_D1PPRE_DIV2);

    // LL_RCC_SetAPB4Prescaler(LL_RCC_APB4_DIV_2);
    MODIFY_REG(RCC->D3CFGR, RCC_D3CFGR_D3PPRE, RCC_D3CFGR_D3PPRE_DIV2);

    SystemCoreClock = (480000000U);


    // dev_mco1_config_t mco1_setings = {.source = mco1_source_hsi, .prescaler = mco1_prescaler_1};
    // interface_t* dev_mco1 = dev_mco1_get();
    // dev_mco1->ioctrl(MCO1_SET_CONFIG, &mco1_setings);
    // dev_mco1->open();

    // dev_mco2_config_t mco2_settings = {.source =  mco2_source_sysclk, .prescaler =  mco2_prescaler_15};
    // interface_t* dev_mco2 = dev_mco2_get();
    // dev_mco2->ioctrl(MCO1_SET_CONFIG, &mco2_settings);
    // dev_mco2->open();

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
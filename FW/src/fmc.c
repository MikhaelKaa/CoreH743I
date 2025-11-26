#include "stm32h743xx.h"

#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)    ((REG) & (BIT))
#define CLEAR_REG(REG)        ((REG) = (0x0))
#define WRITE_REG(REG, VAL)   ((REG) = (VAL))
#define READ_REG(REG)         ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))
#define POSITION_VAL(VAL)     (__CLZ(__RBIT(VAL)))

typedef struct
{
  uint32_t SDBank;                      /*!< Specifies the SDRAM memory device that will be used.
                                             This parameter can be a value of @ref FMC_SDRAM_Bank                */

  uint32_t ColumnBitsNumber;            /*!< Defines the number of bits of column address.
                                             This parameter can be a value of @ref FMC_SDRAM_Column_Bits_number. */

  uint32_t RowBitsNumber;               /*!< Defines the number of bits of column address.
                                             This parameter can be a value of @ref FMC_SDRAM_Row_Bits_number.    */

  uint32_t MemoryDataWidth;             /*!< Defines the memory device width.
                                             This parameter can be a value of @ref FMC_SDRAM_Memory_Bus_Width.   */

  uint32_t InternalBankNumber;          /*!< Defines the number of the device's internal banks.
                                             This parameter can be of @ref FMC_SDRAM_Internal_Banks_Number.      */

  uint32_t CASLatency;                  /*!< Defines the SDRAM CAS latency in number of memory clock cycles.
                                             This parameter can be a value of @ref FMC_SDRAM_CAS_Latency.        */

  uint32_t WriteProtection;             /*!< Enables the SDRAM device to be accessed in write mode.
                                             This parameter can be a value of @ref FMC_SDRAM_Write_Protection.   */

  uint32_t SDClockPeriod;               /*!< Define the SDRAM Clock Period for both SDRAM devices and they allow
                                             to disable the clock before changing frequency.
                                             This parameter can be a value of @ref FMC_SDRAM_Clock_Period.       */

  uint32_t ReadBurst;                   /*!< This bit enable the SDRAM controller to anticipate the next read
                                             commands during the CAS latency and stores data in the Read FIFO.
                                             This parameter can be a value of @ref FMC_SDRAM_Read_Burst.         */

  uint32_t ReadPipeDelay;               /*!< Define the delay in system clock cycles on read data path.
                                             This parameter can be a value of @ref FMC_SDRAM_Read_Pipe_Delay.    */
} FMC_SDRAM_InitTypeDef;

#define FMC_SDRAM_COLUMN_BITS_NUM_8             (0x00000000U)
#define FMC_SDRAM_ROW_BITS_NUM_12               (0x00000004U)
#define FMC_SDRAM_MEM_BUS_WIDTH_16              (0x00000010U)
#define FMC_SDRAM_INTERN_BANKS_NUM_4            (0x00000040U)
#define FMC_SDRAM_CAS_LATENCY_2                 (0x00000100U)
#define FMC_SDRAM_WRITE_PROTECTION_DISABLE      (0x00000000U)
#define FMC_SDRAM_CLOCK_PERIOD_2                (0x00000800U)
#define FMC_SDRAM_RBURST_DISABLE                (0x00000000U)
#define FMC_SDRAM_RPIPE_DELAY_1                 (0x00002000U)

#define FMC_SDRAM_BANK1                         (0x00000000U)
#define FMC_SDRAM_BANK2                         (0x00000001U)

#define SDCR_CLEAR_MASK   ((uint32_t)(FMC_SDCRx_NC    | FMC_SDCRx_NR     | \
                                      FMC_SDCRx_MWID  | FMC_SDCRx_NB     | \
                                      FMC_SDCRx_CAS   | FMC_SDCRx_WP     | \
                                      FMC_SDCRx_SDCLK | FMC_SDCRx_RBURST | \
                                      FMC_SDCRx_RPIPE))

typedef struct
{
  uint32_t LoadToActiveDelay;            /*!< Defines the delay between a Load Mode Register command and
                                              an active or Refresh command in number of memory clock cycles.
                                              This parameter can be a value between Min_Data = 1 and Max_Data = 16  */

  uint32_t ExitSelfRefreshDelay;         /*!< Defines the delay from releasing the self refresh command to
                                              issuing the Activate command in number of memory clock cycles.
                                              This parameter can be a value between Min_Data = 1 and Max_Data = 16  */

  uint32_t SelfRefreshTime;              /*!< Defines the minimum Self Refresh period in number of memory clock
                                              cycles.
                                              This parameter can be a value between Min_Data = 1 and Max_Data = 16  */

  uint32_t RowCycleDelay;                /*!< Defines the delay between the Refresh command and the Activate command
                                              and the delay between two consecutive Refresh commands in number of
                                              memory clock cycles.
                                              This parameter can be a value between Min_Data = 1 and Max_Data = 16  */

  uint32_t WriteRecoveryTime;            /*!< Defines the Write recovery Time in number of memory clock cycles.
                                              This parameter can be a value between Min_Data = 1 and Max_Data = 16  */

  uint32_t RPDelay;                      /*!< Defines the delay between a Precharge Command and an other command
                                              in number of memory clock cycles.
                                              This parameter can be a value between Min_Data = 1 and Max_Data = 16  */

  uint32_t RCDDelay;                     /*!< Defines the delay between the Activate Command and a Read/Write
                                              command in number of memory clock cycles.
                                              This parameter can be a value between Min_Data = 1 and Max_Data = 16  */
} FMC_SDRAM_TimingTypeDef;

#define SDTR_CLEAR_MASK   ((uint32_t)(FMC_SDTRx_TMRD  | FMC_SDTRx_TXSR   | \
                                      FMC_SDTRx_TRAS  | FMC_SDTRx_TRC    | \
                                      FMC_SDTRx_TWR   | FMC_SDTRx_TRP    | \
                                      FMC_SDTRx_TRCD))


static void FMC_SDRAM_InitRegisters(void)
{
    FMC_SDRAM_InitTypeDef Init = {0};
    FMC_SDRAM_TimingTypeDef Timing = {0};

    Init.SDBank             = FMC_SDRAM_BANK2;
    Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
    Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
    Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_16;
    Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_2;
    Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_2;
    Init.ReadBurst          = FMC_SDRAM_RBURST_DISABLE;
    Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_1;

    Timing.LoadToActiveDelay    = 2;
    Timing.ExitSelfRefreshDelay = 7;
    Timing.SelfRefreshTime      = 4;
    Timing.RowCycleDelay        = 6;
    Timing.WriteRecoveryTime    = 2;
    Timing.RPDelay              = 2;
    Timing.RCDDelay             = 2;

    MODIFY_REG(FMC_Bank5_6_R->SDCR[FMC_SDRAM_BANK1],
               FMC_SDCRx_SDCLK           |
               FMC_SDCRx_RBURST          |
               FMC_SDCRx_RPIPE,
               (Init.SDClockPeriod      |
                Init.ReadBurst          |
                Init.ReadPipeDelay));

    MODIFY_REG(FMC_Bank5_6_R->SDCR[FMC_SDRAM_BANK2],
               SDCR_CLEAR_MASK,
               (Init.ColumnBitsNumber   |
                Init.RowBitsNumber      |
                Init.MemoryDataWidth    |
                Init.InternalBankNumber |
                Init.CASLatency         |
                Init.WriteProtection));

    MODIFY_REG(FMC_Bank5_6_R->SDTR[FMC_SDRAM_BANK1],
               FMC_SDTRx_TRC |
               FMC_SDTRx_TRP,
               (((Timing.RowCycleDelay) - 1U)         << FMC_SDTRx_TRC_Pos)  |
               (((Timing.RPDelay) - 1U)               << FMC_SDTRx_TRP_Pos));

    MODIFY_REG(FMC_Bank5_6_R->SDTR[FMC_SDRAM_BANK2],
               SDTR_CLEAR_MASK,
               (((Timing.LoadToActiveDelay) - 1U)                                      |
                (((Timing.ExitSelfRefreshDelay) - 1U) << FMC_SDTRx_TXSR_Pos) |
                (((Timing.SelfRefreshTime) - 1U)      << FMC_SDTRx_TRAS_Pos) |
                (((Timing.WriteRecoveryTime) - 1U)    << FMC_SDTRx_TWR_Pos)  |
                (((Timing.RCDDelay) - 1U)             << FMC_SDTRx_TRCD_Pos)));
}

#define REFRESH_COUNT           ((uint32_t)1386)   /* SDRAM refresh counter */
#define SDRAM_TIMEOUT           ((uint32_t)0xFFFF)

#define FMC_SDRAM_CMD_NORMAL_MODE               (0x00000000U)
#define FMC_SDRAM_CMD_CLK_ENABLE                (0x00000001U)
#define FMC_SDRAM_CMD_PALL                      (0x00000002U)
#define FMC_SDRAM_CMD_AUTOREFRESH_MODE          (0x00000003U)
#define FMC_SDRAM_CMD_LOAD_MODE                 (0x00000004U)
#define FMC_SDRAM_CMD_SELFREFRESH_MODE          (0x00000005U)
#define FMC_SDRAM_CMD_POWERDOWN_MODE            (0x00000006U)
#define FMC_SDRAM_CMD_TARGET_BANK2              FMC_SDCMR_CTB2
#define FMC_SDRAM_CMD_TARGET_BANK1              FMC_SDCMR_CTB1
#define FMC_SDRAM_CMD_TARGET_BANK1_2            (0x00000018U)

typedef struct
{
  uint32_t CommandMode;                  /*!< Defines the command issued to the SDRAM device.
                                              This parameter can be a value of @ref FMC_SDRAM_Command_Mode.          */

  uint32_t CommandTarget;                /*!< Defines which device (1 or 2) the command will be issued to.
                                              This parameter can be a value of @ref FMC_SDRAM_Command_Target.        */

  uint32_t AutoRefreshNumber;            /*!< Defines the number of consecutive auto refresh command issued
                                              in auto refresh mode.
                                              This parameter can be a value between Min_Data = 1 and Max_Data = 15   */

  uint32_t ModeRegisterDefinition;       /*!< Defines the SDRAM Mode register content                                */
} FMC_SDRAM_CommandTypeDef;

static void FMC_GPIO_Init();
static void FMC_SDRAM_InitRegisters();
static void FMC_SDRAM_InitializationSequence();

void FMC_SDRAM_SendCommand(FMC_SDRAM_CommandTypeDef *Command)
{
MODIFY_REG(FMC_Bank5_6_R->SDCMR, (FMC_SDCMR_MODE | FMC_SDCMR_CTB2 | FMC_SDCMR_CTB1 | FMC_SDCMR_NRFS | FMC_SDCMR_MRD),
             ((Command->CommandMode) | (Command->CommandTarget) |
              (((Command->AutoRefreshNumber) - 1U) << FMC_SDCMR_NRFS_Pos) |
              ((Command->ModeRegisterDefinition) << FMC_SDCMR_MRD_Pos)));
}

void FMC_delay(void){
    for(volatile int i = 0; i < 100000; i++)
    {
        asm("nop");
    }
}

void FMC_Init(void)
{
    // Enable GPIO clocks
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOEEN | RCC_AHB4ENR_GPIODEN |
                    RCC_AHB4ENR_GPIOFEN | RCC_AHB4ENR_GPIOGEN |
                    RCC_AHB4ENR_GPIOHEN;

    // Set FMC clock souce
    MODIFY_REG(RCC->D1CCIPR, RCC_D1CCIPR_FMCSEL, 0x00000000U);
    
    // Enable FMC clock
    RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;
    
    FMC_GPIO_Init();
    
    FMC_SDRAM_InitRegisters();

    // Enable FMC Peripheral 
    FMC_Bank1_R->BTCR[0] |= FMC_BCR1_FMCEN;
    
    FMC_SDRAM_InitializationSequence();
}


static void FMC_GPIO_Init(void) 
{   
    // PF0   ------> FMC_A0
    // PF1   ------> FMC_A1
    // PF2   ------> FMC_A2
    // PF3   ------> FMC_A3
    // PF4   ------> FMC_A4
    // PF5   ------> FMC_A5
    // PF11   ------> FMC_SDNRAS
    // PF12   ------> FMC_A6
    // PF13   ------> FMC_A7
    // PF14   ------> FMC_A8
    // PF15   ------> FMC_A9

    // Configure GPIOF pins: PF0, PF1, PF2, PF3, PF4, PF5, PF11, PF12, PF13, PF14, PF15
    GPIOF->MODER = (GPIOF->MODER & ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk |
                                     GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk |
                                     GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk |
                                     GPIO_MODER_MODE11_Msk | GPIO_MODER_MODE12_Msk |
                                     GPIO_MODER_MODE13_Msk | GPIO_MODER_MODE14_Msk |
                                     GPIO_MODER_MODE15_Msk)) |
                   (2U << GPIO_MODER_MODE0_Pos) | (2U << GPIO_MODER_MODE1_Pos) |
                   (2U << GPIO_MODER_MODE2_Pos) | (2U << GPIO_MODER_MODE3_Pos) |
                   (2U << GPIO_MODER_MODE4_Pos) | (2U << GPIO_MODER_MODE5_Pos) |
                   (2U << GPIO_MODER_MODE11_Pos) | (2U << GPIO_MODER_MODE12_Pos) |
                   (2U << GPIO_MODER_MODE13_Pos) | (2U << GPIO_MODER_MODE14_Pos) |
                   (2U << GPIO_MODER_MODE15_Pos);

    GPIOF->OSPEEDR = (GPIOF->OSPEEDR & ~(GPIO_OSPEEDR_OSPEED0_Msk | GPIO_OSPEEDR_OSPEED1_Msk |
                                         GPIO_OSPEEDR_OSPEED2_Msk | GPIO_OSPEEDR_OSPEED3_Msk |
                                         GPIO_OSPEEDR_OSPEED4_Msk | GPIO_OSPEEDR_OSPEED5_Msk |
                                         GPIO_OSPEEDR_OSPEED11_Msk | GPIO_OSPEEDR_OSPEED12_Msk |
                                         GPIO_OSPEEDR_OSPEED13_Msk | GPIO_OSPEEDR_OSPEED14_Msk |
                                         GPIO_OSPEEDR_OSPEED15_Msk)) |
                     (3U << GPIO_OSPEEDR_OSPEED0_Pos) | (3U << GPIO_OSPEEDR_OSPEED1_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED2_Pos) | (3U << GPIO_OSPEEDR_OSPEED3_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED4_Pos) | (3U << GPIO_OSPEEDR_OSPEED5_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED11_Pos) | (3U << GPIO_OSPEEDR_OSPEED12_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED13_Pos) | (3U << GPIO_OSPEEDR_OSPEED14_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED15_Pos);

    GPIOF->PUPDR &= ~(GPIO_PUPDR_PUPD0_Msk | GPIO_PUPDR_PUPD1_Msk |
                      GPIO_PUPDR_PUPD2_Msk | GPIO_PUPDR_PUPD3_Msk |
                      GPIO_PUPDR_PUPD4_Msk | GPIO_PUPDR_PUPD5_Msk |
                      GPIO_PUPDR_PUPD11_Msk | GPIO_PUPDR_PUPD12_Msk |
                      GPIO_PUPDR_PUPD13_Msk | GPIO_PUPDR_PUPD14_Msk |
                      GPIO_PUPDR_PUPD15_Msk);

    // Configure Alternate Function for GPIOF pins
    GPIOF->AFR[0] = (GPIOF->AFR[0] & ~(GPIO_AFRL_AFSEL0_Msk | GPIO_AFRL_AFSEL1_Msk |
                                       GPIO_AFRL_AFSEL2_Msk | GPIO_AFRL_AFSEL3_Msk |
                                       GPIO_AFRL_AFSEL4_Msk | GPIO_AFRL_AFSEL5_Msk)) |
                    (12U << GPIO_AFRL_AFSEL0_Pos) | (12U << GPIO_AFRL_AFSEL1_Pos) |
                    (12U << GPIO_AFRL_AFSEL2_Pos) | (12U << GPIO_AFRL_AFSEL3_Pos) |
                    (12U << GPIO_AFRL_AFSEL4_Pos) | (12U << GPIO_AFRL_AFSEL5_Pos);

    GPIOF->AFR[1] = (GPIOF->AFR[1] & ~(GPIO_AFRH_AFSEL11_Msk | GPIO_AFRH_AFSEL12_Msk |
                                       GPIO_AFRH_AFSEL13_Msk | GPIO_AFRH_AFSEL14_Msk |
                                       GPIO_AFRH_AFSEL15_Msk)) |
                    (12U << GPIO_AFRH_AFSEL11_Pos) | (12U << GPIO_AFRH_AFSEL12_Pos) |
                    (12U << GPIO_AFRH_AFSEL13_Pos) | (12U << GPIO_AFRH_AFSEL14_Pos) |
                    (12U << GPIO_AFRH_AFSEL15_Pos);

    // PH5   ------> FMC_SDNWE
    // PH6   ------> FMC_SDNE1
    // PH7   ------> FMC_SDCKE1

    // Configure GPIOH pins: PH5, PH6, PH7
    GPIOH->MODER = (GPIOH->MODER & ~(GPIO_MODER_MODE5_Msk | GPIO_MODER_MODE6_Msk |
                                     GPIO_MODER_MODE7_Msk)) |
                   (2U << GPIO_MODER_MODE5_Pos) | (2U << GPIO_MODER_MODE6_Pos) |
                   (2U << GPIO_MODER_MODE7_Pos);

    GPIOH->OSPEEDR = (GPIOH->OSPEEDR & ~(GPIO_OSPEEDR_OSPEED5_Msk | GPIO_OSPEEDR_OSPEED6_Msk |
                                         GPIO_OSPEEDR_OSPEED7_Msk)) |
                     (3U << GPIO_OSPEEDR_OSPEED5_Pos) | (3U << GPIO_OSPEEDR_OSPEED6_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED7_Pos);

    GPIOH->PUPDR &= ~(GPIO_PUPDR_PUPD5_Msk | GPIO_PUPDR_PUPD6_Msk | GPIO_PUPDR_PUPD7_Msk);

    GPIOH->AFR[0] = (GPIOH->AFR[0] & ~(GPIO_AFRL_AFSEL5_Msk | GPIO_AFRL_AFSEL6_Msk |
                                       GPIO_AFRL_AFSEL7_Msk)) |
                    (12U << GPIO_AFRL_AFSEL5_Pos) | (12U << GPIO_AFRL_AFSEL6_Pos) |
                    (12U << GPIO_AFRL_AFSEL7_Pos);

    // PG0   ------> FMC_A10
    // PG1   ------> FMC_A11
    // PG4   ------> FMC_BA0
    // PG5   ------> FMC_BA1
    // PG8   ------> FMC_SDCLK
    // PG15   ------> FMC_SDNCAS

    // Configure GPIOG pins: PG0, PG1, PG4, PG5, PG8, PG15
    GPIOG->MODER = (GPIOG->MODER & ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk |
                                     GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk |
                                     GPIO_MODER_MODE8_Msk | GPIO_MODER_MODE15_Msk)) |
                   (2U << GPIO_MODER_MODE0_Pos) | (2U << GPIO_MODER_MODE1_Pos) |
                   (2U << GPIO_MODER_MODE4_Pos) | (2U << GPIO_MODER_MODE5_Pos) |
                   (2U << GPIO_MODER_MODE8_Pos) | (2U << GPIO_MODER_MODE15_Pos);

    GPIOG->OSPEEDR = (GPIOG->OSPEEDR & ~(GPIO_OSPEEDR_OSPEED0_Msk | GPIO_OSPEEDR_OSPEED1_Msk |
                                         GPIO_OSPEEDR_OSPEED4_Msk | GPIO_OSPEEDR_OSPEED5_Msk |
                                         GPIO_OSPEEDR_OSPEED8_Msk | GPIO_OSPEEDR_OSPEED15_Msk)) |
                     (3U << GPIO_OSPEEDR_OSPEED0_Pos) | (3U << GPIO_OSPEEDR_OSPEED1_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED4_Pos) | (3U << GPIO_OSPEEDR_OSPEED5_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED8_Pos) | (3U << GPIO_OSPEEDR_OSPEED15_Pos);

    GPIOG->PUPDR &= ~(GPIO_PUPDR_PUPD0_Msk | GPIO_PUPDR_PUPD1_Msk |
                      GPIO_PUPDR_PUPD4_Msk | GPIO_PUPDR_PUPD5_Msk |
                      GPIO_PUPDR_PUPD8_Msk | GPIO_PUPDR_PUPD15_Msk);

    GPIOG->AFR[0] = (GPIOG->AFR[0] & ~(GPIO_AFRL_AFSEL0_Msk | GPIO_AFRL_AFSEL1_Msk |
                                       GPIO_AFRL_AFSEL4_Msk | GPIO_AFRL_AFSEL5_Msk)) |
                    (12U << GPIO_AFRL_AFSEL0_Pos) | (12U << GPIO_AFRL_AFSEL1_Pos) |
                    (12U << GPIO_AFRL_AFSEL4_Pos) | (12U << GPIO_AFRL_AFSEL5_Pos);

    GPIOG->AFR[1] = (GPIOG->AFR[1] & ~(GPIO_AFRH_AFSEL8_Msk | GPIO_AFRH_AFSEL15_Msk)) |
                    (12U << GPIO_AFRH_AFSEL8_Pos) | (12U << GPIO_AFRH_AFSEL15_Pos);

    // PE0   ------> FMC_NBL0
    // PE1   ------> FMC_NBL1
    // PE7   ------> FMC_D4
    // PE8   ------> FMC_D5
    // PE9   ------> FMC_D6
    // PE10   ------> FMC_D7
    // PE11   ------> FMC_D8
    // PE12   ------> FMC_D9
    // PE13   ------> FMC_D10
    // PE14   ------> FMC_D11
    // PE15   ------> FMC_D12

    // Configure GPIOE pins: PE0, PE1, PE7, PE8, PE9, PE10, PE11, PE12, PE13, PE14, PE15
    GPIOE->MODER = (GPIOE->MODER & ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk |
                                     GPIO_MODER_MODE7_Msk | GPIO_MODER_MODE8_Msk |
                                     GPIO_MODER_MODE9_Msk | GPIO_MODER_MODE10_Msk |
                                     GPIO_MODER_MODE11_Msk | GPIO_MODER_MODE12_Msk |
                                     GPIO_MODER_MODE13_Msk | GPIO_MODER_MODE14_Msk |
                                     GPIO_MODER_MODE15_Msk)) |
                   (2U << GPIO_MODER_MODE0_Pos) | (2U << GPIO_MODER_MODE1_Pos) |
                   (2U << GPIO_MODER_MODE7_Pos) | (2U << GPIO_MODER_MODE8_Pos) |
                   (2U << GPIO_MODER_MODE9_Pos) | (2U << GPIO_MODER_MODE10_Pos) |
                   (2U << GPIO_MODER_MODE11_Pos) | (2U << GPIO_MODER_MODE12_Pos) |
                   (2U << GPIO_MODER_MODE13_Pos) | (2U << GPIO_MODER_MODE14_Pos) |
                   (2U << GPIO_MODER_MODE15_Pos);

    GPIOE->OSPEEDR = (GPIOE->OSPEEDR & ~(GPIO_OSPEEDR_OSPEED0_Msk | GPIO_OSPEEDR_OSPEED1_Msk |
                                         GPIO_OSPEEDR_OSPEED7_Msk | GPIO_OSPEEDR_OSPEED8_Msk |
                                         GPIO_OSPEEDR_OSPEED9_Msk | GPIO_OSPEEDR_OSPEED10_Msk |
                                         GPIO_OSPEEDR_OSPEED11_Msk | GPIO_OSPEEDR_OSPEED12_Msk |
                                         GPIO_OSPEEDR_OSPEED13_Msk | GPIO_OSPEEDR_OSPEED14_Msk |
                                         GPIO_OSPEEDR_OSPEED15_Msk)) |
                     (3U << GPIO_OSPEEDR_OSPEED0_Pos) | (3U << GPIO_OSPEEDR_OSPEED1_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED7_Pos) | (3U << GPIO_OSPEEDR_OSPEED8_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED9_Pos) | (3U << GPIO_OSPEEDR_OSPEED10_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED11_Pos) | (3U << GPIO_OSPEEDR_OSPEED12_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED13_Pos) | (3U << GPIO_OSPEEDR_OSPEED14_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED15_Pos);

    GPIOE->PUPDR &= ~(GPIO_PUPDR_PUPD0_Msk | GPIO_PUPDR_PUPD1_Msk |
                      GPIO_PUPDR_PUPD7_Msk | GPIO_PUPDR_PUPD8_Msk |
                      GPIO_PUPDR_PUPD9_Msk | GPIO_PUPDR_PUPD10_Msk |
                      GPIO_PUPDR_PUPD11_Msk | GPIO_PUPDR_PUPD12_Msk |
                      GPIO_PUPDR_PUPD13_Msk | GPIO_PUPDR_PUPD14_Msk |
                      GPIO_PUPDR_PUPD15_Msk);

    GPIOE->AFR[0] = (GPIOE->AFR[0] & ~(GPIO_AFRL_AFSEL0_Msk | GPIO_AFRL_AFSEL1_Msk |
                                       GPIO_AFRL_AFSEL7_Msk)) |
                    (12U << GPIO_AFRL_AFSEL0_Pos) | (12U << GPIO_AFRL_AFSEL1_Pos) |
                    (12U << GPIO_AFRL_AFSEL7_Pos);

    GPIOE->AFR[1] = (GPIOE->AFR[1] & ~(GPIO_AFRH_AFSEL8_Msk | GPIO_AFRH_AFSEL9_Msk |
                                       GPIO_AFRH_AFSEL10_Msk | GPIO_AFRH_AFSEL11_Msk |
                                       GPIO_AFRH_AFSEL12_Msk | GPIO_AFRH_AFSEL13_Msk |
                                       GPIO_AFRH_AFSEL14_Msk | GPIO_AFRH_AFSEL15_Msk)) |
                    (12U << GPIO_AFRH_AFSEL8_Pos) | (12U << GPIO_AFRH_AFSEL9_Pos) |
                    (12U << GPIO_AFRH_AFSEL10_Pos) | (12U << GPIO_AFRH_AFSEL11_Pos) |
                    (12U << GPIO_AFRH_AFSEL12_Pos) | (12U << GPIO_AFRH_AFSEL13_Pos) |
                    (12U << GPIO_AFRH_AFSEL14_Pos) | (12U << GPIO_AFRH_AFSEL15_Pos);

    // PD0    ------> FMC_D2
    // PD1    ------> FMC_D3
    // PD8    ------> FMC_D13
    // PD9    ------> FMC_D14
    // PD10   ------> FMC_D15
    // PD14   ------> FMC_D0
    // PD15   ------> FMC_D1

    // Configure GPIOD pins: PD0, PD1, PD8, PD9, PD10, PD14, PD15
    GPIOD->MODER = (GPIOD->MODER & ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk |
                                     GPIO_MODER_MODE8_Msk | GPIO_MODER_MODE9_Msk |
                                     GPIO_MODER_MODE10_Msk | GPIO_MODER_MODE14_Msk |
                                     GPIO_MODER_MODE15_Msk)) |
                   (2U << GPIO_MODER_MODE0_Pos) | (2U << GPIO_MODER_MODE1_Pos) |
                   (2U << GPIO_MODER_MODE8_Pos) | (2U << GPIO_MODER_MODE9_Pos) |
                   (2U << GPIO_MODER_MODE10_Pos) | (2U << GPIO_MODER_MODE14_Pos) |
                   (2U << GPIO_MODER_MODE15_Pos);

    GPIOD->OSPEEDR = (GPIOD->OSPEEDR & ~(GPIO_OSPEEDR_OSPEED0_Msk | GPIO_OSPEEDR_OSPEED1_Msk |
                                         GPIO_OSPEEDR_OSPEED8_Msk | GPIO_OSPEEDR_OSPEED9_Msk |
                                         GPIO_OSPEEDR_OSPEED10_Msk | GPIO_OSPEEDR_OSPEED14_Msk |
                                         GPIO_OSPEEDR_OSPEED15_Msk)) |
                     (3U << GPIO_OSPEEDR_OSPEED0_Pos) | (3U << GPIO_OSPEEDR_OSPEED1_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED8_Pos) | (3U << GPIO_OSPEEDR_OSPEED9_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED10_Pos) | (3U << GPIO_OSPEEDR_OSPEED14_Pos) |
                     (3U << GPIO_OSPEEDR_OSPEED15_Pos);

    GPIOD->PUPDR &= ~(GPIO_PUPDR_PUPD0_Msk | GPIO_PUPDR_PUPD1_Msk |
                      GPIO_PUPDR_PUPD8_Msk | GPIO_PUPDR_PUPD9_Msk |
                      GPIO_PUPDR_PUPD10_Msk | GPIO_PUPDR_PUPD14_Msk |
                      GPIO_PUPDR_PUPD15_Msk);

    GPIOD->AFR[0] = (GPIOD->AFR[0] & ~(GPIO_AFRL_AFSEL0_Msk | GPIO_AFRL_AFSEL1_Msk)) |
                    (12U << GPIO_AFRL_AFSEL0_Pos) | (12U << GPIO_AFRL_AFSEL1_Pos);

    GPIOD->AFR[1] = (GPIOD->AFR[1] & ~(GPIO_AFRH_AFSEL8_Msk | GPIO_AFRH_AFSEL9_Msk |
                                       GPIO_AFRH_AFSEL10_Msk | GPIO_AFRH_AFSEL14_Msk |
                                       GPIO_AFRH_AFSEL15_Msk)) |
                    (12U << GPIO_AFRH_AFSEL8_Pos) | (12U << GPIO_AFRH_AFSEL9_Pos) |
                    (12U << GPIO_AFRH_AFSEL10_Pos) | (12U << GPIO_AFRH_AFSEL14_Pos) |
                    (12U << GPIO_AFRH_AFSEL15_Pos);
}



static void FMC_SDRAM_InitializationSequence(void)
{

    __IO uint32_t tmpmrd =0;
    FMC_SDRAM_CommandTypeDef Command;
    /* Step 1:  Configure a clock configuration enable command */
    Command.CommandMode             = FMC_SDRAM_CMD_CLK_ENABLE;
    Command.CommandTarget           = FMC_SDRAM_CMD_TARGET_BANK2;
    Command.AutoRefreshNumber       = 1;
    Command.ModeRegisterDefinition  = 0;
    
    FMC_delay();
      FMC_delay();

    /* Send the command */
    FMC_SDRAM_SendCommand(&Command);

    /* Step 2: Insert 100 us minimum delay */ 
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    FMC_delay();

    /* Step 3: Configure a PALL (precharge all) command */ 
    Command.CommandMode             = FMC_SDRAM_CMD_PALL;
    Command.CommandTarget           = FMC_SDRAM_CMD_TARGET_BANK2;
    Command.AutoRefreshNumber       = 1;
    Command.ModeRegisterDefinition  = 0;

    /* Send the command */
    FMC_SDRAM_SendCommand(&Command);  
    
    /* Step 4: Configure an Auto Refresh command */ 
    Command.CommandMode             = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command.CommandTarget           = FMC_SDRAM_CMD_TARGET_BANK2;
    Command.AutoRefreshNumber       = 4;
    Command.ModeRegisterDefinition  = 0;
    
    FMC_delay();

    /* Send the command */
    FMC_SDRAM_SendCommand(&Command);
    
    //Second??
    FMC_delay();
    FMC_SDRAM_SendCommand(&Command);
    
    tmpmrd  = (
              (1U << 9U)      |    // Use bursts for read and write data (0 = Burst-Read-Burst-Write, 1 = Burst-Read-Single-Write)
              (0U << 7U)      |    // Normal mode (other values are reserved)
              (2U << 4U)      |    // CAS latency = 2
              (0U << 3U)      |    // Burst mode: Sequential
              (2U << 0U)           // Burst length: 4
              );
    
    
    Command.CommandMode             = FMC_SDRAM_CMD_LOAD_MODE;
    Command.CommandTarget           = FMC_SDRAM_CMD_TARGET_BANK2;
    Command.AutoRefreshNumber       = 1;
    Command.ModeRegisterDefinition  = tmpmrd;
    
    FMC_delay();

    /* Send the command */
    FMC_SDRAM_SendCommand(&Command);
    
    /* Step 6: Set the refresh rate counter */
    /* Set the device refresh rate */
    MODIFY_REG(FMC_Bank5_6_R->SDRTR, FMC_SDRTR_COUNT, (REFRESH_COUNT << FMC_SDRTR_COUNT_Pos));
}

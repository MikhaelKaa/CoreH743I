/* SPDX-License-Identifier: MIT */
/*
 * dev_uart1.c - POSIX-style UART interface implementation for STM32H743
 */

#include <string.h>
#include <errno.h>
#include "dev_uart1.h"
#include "stm32h743xx.h"

#define TX_TIMEOUT (10000000U)

// Static buffers
RAM_D1 static uint8_t tx_buffer[UART_TX_BUFFER_SIZE];
RAM_D1  static volatile uint8_t rx_buffer[UART_RX_BUFFER_SIZE];

// Ring buffer pointers for RX
static volatile uint32_t rx_read_pos = 0;

// DMA transfer state
static volatile uint8_t tx_in_progress = 0;
static uint32_t current_baudrate = UART_DEFAULT_BAUDRATE;
static uint8_t uart_initialized = 0;

static int uart_available(void);
static int uart_init(void);
static int uart_deinit(void);
static int uart_set_baudrate(uint32_t baudrate);

// Open UART (interface implementation)
static int uart_open(void) {
    if (!uart_initialized) {
        return uart_init();
    }
    return 0;
}

// Close UART (interface implementation)  
static int uart_close(void) {
    return uart_deinit();
}

// Write data to UART (interface implementation)
static int uart_write(const void *buf, size_t count) {
    if (buf == NULL || count == 0) {
        return -EINVAL;
    }
    
    if (!uart_initialized) {
        return -ENODEV;
    }
    
    if (count > UART_TX_BUFFER_SIZE) {
        count = UART_TX_BUFFER_SIZE;
    }

    // Wait for previous transmission to complete
    uint32_t timeout = TX_TIMEOUT;
    while (tx_in_progress && timeout--) {
        __asm__("nop");
    }
    
    if (timeout == 0) {
        return -ETIMEDOUT;
    }

    // Copy data to buffer
    memcpy(tx_buffer, buf, count);
    tx_in_progress = 1;
    
    // Disable DMA stream before reconfiguration
    DMA1_Stream1->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream1->CR & DMA_SxCR_EN);
    
    // Configure and start DMA transfer
    DMA1_Stream1->M0AR = (uint32_t)tx_buffer;
    DMA1_Stream1->NDTR = count;
    
    // Clear transfer complete flag
    DMA1->LIFCR |= DMA_LIFCR_CTCIF1;
    
    // Enable DMA stream
    DMA1_Stream1->CR |= DMA_SxCR_EN;
    
    // Enable USART TX DMA
    USART1->CR3 |= USART_CR3_DMAT;
    
    return (int)count;
}

// Read data from UART (interface implementation)
static int uart_read(void *buf, size_t count) {
    if (buf == NULL) {
        return -EINVAL;
    }
    
    if (!uart_initialized) {
        return -ENODEV;
    }
    
    uint8_t *buffer = (uint8_t *)buf;
    size_t bytes_read = 0;
    
    // Calculate available bytes in ring buffer
    int available = uart_available();
    if (available == 0) {
        return 0;  // No data available
    }
    
    if (count > (size_t)available) {
        count = (size_t)available;
    }
    
    // Read data from ring buffer
    for (size_t i = 0; i < count; i++) {
        buffer[i] = rx_buffer[rx_read_pos];
        rx_read_pos = (rx_read_pos + 1) % UART_RX_BUFFER_SIZE;
        bytes_read++;
    }
    
    return (int)bytes_read;
}

// Check how many bytes are available to read
static int uart_available(void) {
    if (!uart_initialized) {
        return 0;
    }
    
    uint32_t current_ndtr = DMA1_Stream0->NDTR;
    uint32_t bytes_received = UART_RX_BUFFER_SIZE - current_ndtr;
    
    if (bytes_received >= rx_read_pos) {
        return (int)(bytes_received - rx_read_pos);
    } else {
        return (int)((UART_RX_BUFFER_SIZE - rx_read_pos) + bytes_received);
    }
}

// Flush RX buffer
static int uart_flush(void) {
    if (!uart_initialized) {
        return -ENODEV;
    }
    
    // Disable DMA stream
    DMA1_Stream0->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream0->CR & DMA_SxCR_EN);
    
    // Reset buffer position
    rx_read_pos = 0;
    
    // Reconfigure DMA stream
    DMA1_Stream0->M0AR = (uint32_t)rx_buffer;
    DMA1_Stream0->NDTR = UART_RX_BUFFER_SIZE;
    
    // Clear all flags
    DMA1->LIFCR |= DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTCIF0;
    
    // Enable DMA stream
    DMA1_Stream0->CR |= DMA_SxCR_EN;
    
    return 0;
}

// Initialize UART hardware
static int uart_init(void) {
    if (uart_initialized) {
        return 0; // Already initialized
    }

    // Set USART1 clock source to PCLK2
    RCC->D2CCIP2R &= ~RCC_D2CCIP2R_USART16SEL;
    RCC->D2CCIP2R |= 0x0U; // 00: pclk2 selected

    // Enable clocks
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

    // Configure GPIO for USART1 (PB14 - TX, PB15 - RX)
    GPIOB->MODER &= ~(GPIO_MODER_MODE14 | GPIO_MODER_MODE15);
    GPIOB->MODER |= (2U << GPIO_MODER_MODE14_Pos) | (2U << GPIO_MODER_MODE15_Pos);
    
    GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL14 | GPIO_AFRH_AFSEL15);
    GPIOB->AFR[1] |= (4U << (4 * (14 - 8))) | (4U << (4 * (15 - 8)));
    
    GPIOB->OTYPER &= ~(GPIO_OTYPER_OT14 | GPIO_OTYPER_OT15);
    GPIOB->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED14 | GPIO_OSPEEDR_OSPEED15);
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD14 | GPIO_PUPDR_PUPD15);

    // Configure USART1_RX DMA (DMA1 Stream0)
    // DMAMUX configuration for USART1_RX
    DMAMUX1_Channel0->CCR &= ~DMAMUX_CxCR_DMAREQ_ID;
    DMAMUX1_Channel0->CCR |= 41U; // USART1_RX DMAMUX request
    
    // DMA Stream configuration
    DMA1_Stream0->CR &= ~(DMA_SxCR_DIR | DMA_SxCR_PL | DMA_SxCR_CIRC | DMA_SxCR_PINC | DMA_SxCR_MINC | 
                         DMA_SxCR_PSIZE | DMA_SxCR_MSIZE | DMA_SxCR_PFCTRL);
    DMA1_Stream0->CR |= (0x0U << DMA_SxCR_DIR_Pos) |    // Peripheral to memory
                       (0x0U << DMA_SxCR_PL_Pos) |      // Low priority  
                       DMA_SxCR_CIRC |                  // Circular mode
                       DMA_SxCR_MINC |                  // Memory increment
                       (0x0U << DMA_SxCR_PSIZE_Pos) |   // Peripheral byte
                       (0x0U << DMA_SxCR_MSIZE_Pos);    // Memory byte
    
    // Disable FIFO mode
    DMA1_Stream0->FCR &= ~DMA_SxFCR_DMDIS;

    // Configure USART1_TX DMA (DMA1 Stream1)
    // DMAMUX configuration for USART1_TX
    DMAMUX1_Channel1->CCR &= ~DMAMUX_CxCR_DMAREQ_ID;
    DMAMUX1_Channel1->CCR |= 42U; // USART1_TX DMAMUX request
    
    // DMA Stream configuration
    DMA1_Stream1->CR &= ~(DMA_SxCR_DIR | DMA_SxCR_PL | DMA_SxCR_CIRC | DMA_SxCR_PINC | DMA_SxCR_MINC | 
                         DMA_SxCR_PSIZE | DMA_SxCR_MSIZE | DMA_SxCR_PFCTRL);
    DMA1_Stream1->CR |= (0x1U << DMA_SxCR_DIR_Pos) |    // Memory to peripheral
                       (0x0U << DMA_SxCR_PL_Pos) |      // Low priority
                       DMA_SxCR_MINC |                  // Memory increment
                       (0x0U << DMA_SxCR_PSIZE_Pos) |   // Peripheral byte
                       (0x0U << DMA_SxCR_MSIZE_Pos);    // Memory byte
    
    // Disable FIFO mode
    DMA1_Stream1->FCR &= ~DMA_SxFCR_DMDIS;

    // Initialize DMA streams
    DMA1_Stream0->PAR = (uint32_t)&USART1->RDR;
    DMA1_Stream0->M0AR = (uint32_t)rx_buffer;
    DMA1_Stream0->NDTR = UART_RX_BUFFER_SIZE;
    DMA1_Stream0->CR |= (DMA_SxCR_TCIE | DMA_SxCR_HTIE); // Enable interrupts
    
    DMA1_Stream1->PAR = (uint32_t)&USART1->TDR;
    DMA1_Stream1->M0AR = (uint32_t)tx_buffer;
    DMA1_Stream1->NDTR = 0;
    DMA1_Stream1->CR |= DMA_SxCR_TCIE; // Enable transfer complete interrupt

    // Enable DMA streams
    DMA1_Stream0->CR |= DMA_SxCR_EN;
    // Stream1 will be enabled during transmission

    // Configure USART1
    // Disable USART before configuration
    USART1->CR1 &= ~USART_CR1_UE;

    uart_set_baudrate(UART_DEFAULT_BAUDRATE);

    // Configure USART parameters
    USART1->CR1 &= ~(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_OVER8 | USART_CR1_FIFOEN);
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE; // Transmitter and Receiver enabled, 8 data bits, no parity, 16x oversampling

    USART1->CR2 &= ~USART_CR2_STOP; // 1 stop bit

    USART1->CR3 &= ~(USART_CR3_RTSE | USART_CR3_CTSE); // No hardware flow control

    // Enable DMA for TX and RX
    USART1->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR;

    // Enable USART1
    USART1->CR1 |= USART_CR1_UE;

    // Wait for USART to be ready
    while((!(USART1->ISR & USART_ISR_TEACK)) || (!(USART1->ISR & USART_ISR_REACK))) {
        // Wait for transmit and receive enable acknowledgement
    }

    // Enable DMA interrupts
    NVIC_SetPriority(DMA1_Stream0_IRQn, 5);
    NVIC_SetPriority(DMA1_Stream1_IRQn, 5);
    NVIC_EnableIRQ(DMA1_Stream0_IRQn);
    NVIC_EnableIRQ(DMA1_Stream1_IRQn);

    // Clear buffers
    memset(tx_buffer, 0, UART_TX_BUFFER_SIZE);
    memset((void *)rx_buffer, 0, UART_RX_BUFFER_SIZE);
    
    // Reset buffer pointers and state
    rx_read_pos = 0;
    tx_in_progress = 0;
    uart_initialized = 1;
    
    return 0;
}

// Deinitialize UART hardware
static int uart_deinit(void) {
    if (!uart_initialized) {
        return 0;
    }
    
    // Disable USART
    USART1->CR1 &= ~USART_CR1_UE;
    
    // Disable DMA streams
    DMA1_Stream0->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream0->CR & DMA_SxCR_EN);
    
    DMA1_Stream1->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream1->CR & DMA_SxCR_EN);
    
    // Disable DMA requests in USART
    USART1->CR3 &= ~(USART_CR3_DMAT | USART_CR3_DMAR);
    
    // Disable interrupts
    NVIC_DisableIRQ(DMA1_Stream0_IRQn);
    NVIC_DisableIRQ(DMA1_Stream1_IRQn);
    
    // Reset state
    rx_read_pos = 0;
    tx_in_progress = 0;
    uart_initialized = 0;
    
    return 0;
}

// Set baudrate
static int uart_set_baudrate(uint32_t baudrate) {
    if (baudrate == 0) {
        return -EINVAL;
    }
    
    // For STM32H743, USART1 is on APB2 (PCLK2) = 120MHz
    uint32_t pclk2_freq = 120000000U;
    
    // Calculate BRR value for oversampling by 16
    uint32_t usartdiv = (pclk2_freq + (baudrate / 2U)) / baudrate;
    USART1->BRR = usartdiv;

    current_baudrate = baudrate;
    return 0;
}

// Get current baudrate
static int uart_get_baudrate(uint32_t *baudrate) {
    if (baudrate == NULL) {
        return -EINVAL;
    }
    
    *baudrate = current_baudrate;
    return 0;
}

// IO Control for UART (interface implementation)
static int uart_ioctrl(int cmd, void *arg) {
    switch (cmd) {
        case UART_INIT:
            return uart_init();
            
        case UART_DEINIT:
            return uart_deinit();
            
        case UART_GET_AVAILABLE:
            if (arg != NULL) {
                *(int *)arg = uart_available();
                return 0;
            }
            return -EINVAL;
            
        case UART_FLUSH:
            return uart_flush();
            
        case UART_SET_BAUDRATE:
            if (arg == NULL) return -EINVAL;
            return uart_set_baudrate(*(uint32_t *)arg);
            
        case UART_GET_BAUDRATE:
            if (arg == NULL) return -EINVAL;
            return uart_get_baudrate((uint32_t *)arg);
            
        default:
            return -ENOTSUP;
    }
}

// UART device instance (static - hidden inside module)
static const interface_t dev_uart1 = {
    .open = uart_open,
    .close = uart_close, 
    .read = uart_read,
    .write = uart_write,
    .ioctrl = uart_ioctrl
};

// UART device instance accessor
const interface_t* dev_uart1_get(void) {
    return &dev_uart1;
}

// DMA1 Stream0 Interrupt Handler (Reception - USART1_RX)
void DMA1_Stream0_IRQHandler(void) {
    // Half transfer complete
    if (DMA1->LISR & DMA_LISR_HTIF0) {
        DMA1->LIFCR |= DMA_LIFCR_CHTIF0;
    }
    
    // Transfer complete
    if (DMA1->LISR & DMA_LISR_TCIF0) {
        DMA1->LIFCR |= DMA_LIFCR_CTCIF0;
    }
}

// DMA1 Stream1 Interrupt Handler (Transmission - USART1_TX)
void DMA1_Stream1_IRQHandler(void) {
    // Transfer complete
    if (DMA1->LISR & DMA_LISR_TCIF1) {
        DMA1->LIFCR |= DMA_LIFCR_CTCIF1;
        tx_in_progress = 0;
        
        // Disable TX DMA after transfer complete
        USART1->CR3 &= ~USART_CR3_DMAT;
    }
}
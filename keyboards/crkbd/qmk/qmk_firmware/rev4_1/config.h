#pragma once

// Hardware pin definitions (not in info.json)
#define USB_VBUS_PIN GP13
#define SPLIT_HAND_PIN GP21

/* Serial settings - Using standard UART for reliability */
#define SERIAL_DRIVER SERIAL_DRIVER_UART
#define SERIAL_USART_DRIVER SD0  // UART0 driver

// Master (left) uses GP4/GP5
// Slave (right) uses GP24/GP25
#if defined(SPLIT_HAND_PIN) && defined(SPLIT_HAND_PIN_LOW_IS_LEFT)
    // Right side (HIGH)
    #define SERIAL_USART_TX_PIN GP24
    #define SERIAL_USART_RX_PIN GP25
#else
    // Left side (LOW or floating)
    #define SERIAL_USART_TX_PIN GP4
    #define SERIAL_USART_RX_PIN GP5
#endif

#define SERIAL_USART_TIMEOUT 100    // Short timeout for faster detection
#define SERIAL_USART_SPEED 115200   // Baud rate for split communication

/* RP2040-specific config */
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 500U
#define PICO_XOSC_STARTUP_DELAY_MULTIPLIER 64


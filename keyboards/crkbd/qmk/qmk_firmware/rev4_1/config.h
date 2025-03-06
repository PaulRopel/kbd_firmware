#pragma once

// Hardware pin definitions
#define USB_VBUS_PIN GP13
#define SPLIT_HAND_PIN GP21  // Determines left/right side
#define SPLIT_HAND_PIN_HIGH_IS_LEFT  // HIGH = left side, LOW = right side
// #define MASTER_LEFT

// Add hardware pull-up/pull-down resistor
#define SPLIT_HAND_PIN_PULLUP_RESISTOR 1000  // Stronger pull-up

// Enable PAL wait functionality for ChibiOS
#define PAL_USE_WAIT TRUE

// Serial driver configuration (using RP2040 UART)
#define SERIAL_DRIVER SERIAL_DRIVER_VENDOR
#define SERIAL_USART_FULL_DUPLEX  // Enable full-duplex UART communication
#define SERIAL_USART_DRIVER SD1   // UART1 (GP4/GP5)

// Define UART pins for split communication
#define SERIAL_USART_TX_PIN GP4  // Primary UART TX pin
#define SERIAL_USART_RX_PIN GP5  // Primary UART RX pin

// Alternative UART pins for the right side
#define SERIAL_USART_TX_PIN_RIGHT GP24
#define SERIAL_USART_RX_PIN_RIGHT GP25

// Serial configuration
#define SERIAL_USART_SPEED 115200  // Higher speed for better performance
#define SERIAL_USART_TX_TIMEOUT 50000
#define SERIAL_USART_RX_TIMEOUT 50000

// USB Split Detection Timings
#define SPLIT_USB_TIMEOUT 10000              // Increased timeout
#define SPLIT_USB_DETECT_POLL_RATE 100       // More frequent polling
#define SPLIT_MAX_CONNECTION_ERRORS 10  // More lenient error threshold
#define SPLIT_CONNECTION_CHECK_INTERVAL 100  // Check connection more frequently

// Enable serial debugging
#define SERIAL_DEBUG  

/* RP2040-specific config */
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 500U
#define PICO_XOSC_STARTUP_DELAY_MULTIPLIER 64

// Add startup delay to ensure split detection stabilizes
#define SPLIT_STARTUP_DELAY 2000  // 2 seconds delay at startup


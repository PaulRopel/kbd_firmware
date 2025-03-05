#pragma once

// Hardware pin definitions
#define USB_VBUS_PIN GP13
#define SPLIT_HAND_PIN GP21
#define SPLIT_HAND_PIN_HIGH_IS_LEFT  // HIGH = left side, LOW = right side

// Enable PAL wait functionality for ChibiOS
#define PAL_USE_WAIT TRUE

// Define soft serial pin - using an unused pin since we're actually using UART
#define SOFT_SERIAL_PIN GP12

// Define UART pins for split communication
#define SERIAL_USART_TX_PIN_RIGHT GP24
#define SERIAL_USART_RX_PIN_RIGHT GP25
#define SERIAL_USART_TX_PIN_LEFT GP4
#define SERIAL_USART_RX_PIN_LEFT GP5

// Serial driver configuration
#undef SERIAL_DRIVER
#define SERIAL_DRIVER SERIAL_DRIVER_VENDOR  // Go back to vendor driver
#define SERIAL_USE_MULTI_TRANSACTION

// Additional UART configuration for improved reliability
#define SERIAL_USART_DRIVER SD0  // Use the first UART peripheral
#define SERIAL_USART_FULL_DUPLEX // Enable full-duplex mode
#define SERIAL_USART_TX_TIMEOUT 1000 // Much longer timeout for TX
#define SERIAL_USART_RX_TIMEOUT 1000 // Much longer timeout for RX

// Try a much lower baud rate for more reliable communication
#define SERIAL_USART_SPEED 9600  // Very low baud rate for reliability

// Force serial driver to initialize
#define SPLIT_TRANSPORT_MIRROR
#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_LED_STATE_ENABLE
#define SPLIT_MODS_ENABLE

// Remove the problematic matrix grid detection
// #define SPLIT_HAND_MATRIX_GRID GP22, GP20  // Use matrix pins for detection
#define EE_HANDS  // Try EEPROM-based hand detection as fallback

// Debug configuration
#define DEBUG_MATRIX_SCAN_RATE
#define CONSOLE_ENABLE_CDC

// Force debug to be enabled
#define DEBUG_ENABLE

/* RP2040-specific config */
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 500U
#define PICO_XOSC_STARTUP_DELAY_MULTIPLIER 64


#pragma once

// Hardware pin definitions (not in info.json)
#define USB_VBUS_PIN GP13
#define SPLIT_HAND_PIN GP21

/* Debug options - only keep what's needed in config.h */
#define DEBUG_MATRIX_SCAN_RATE
#define DEBUG_SPLIT_ENABLE
#define SERIAL_DEBUG

/* Serial settings - Using PIO driver for optimal RP2040 performance */
#define SERIAL_DRIVER SERIAL_DRIVER_PIO

// Master (left) uses GP4/GP5
// Slave (right) uses GP24/GP25
#if defined(SPLIT_HAND_PIN) && defined(SPLIT_HAND_PIN_LOW_IS_LEFT)
    // Right side (HIGH)
    #define SERIAL_PIO_TX_PIN GP24
    #define SERIAL_PIO_RX_PIN GP25
#else
    // Left side (LOW or floating)
    #define SERIAL_PIO_TX_PIN GP4
    #define SERIAL_PIO_RX_PIN GP5
#endif

/* Optional: Use PIO1 instead of default PIO0 if you have conflicts */
// #define SERIAL_PIO_USE_PIO1

/* Try a longer timeout and slower speed for debugging */
#define SERIAL_PIO_TIMEOUT 150    // Increased timeout for debugging
#define SELECT_SOFT_SERIAL_SPEED 2  // Slower speed (115200 baud) for reliability

/* RP2040-specific config */
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 500U
#define PICO_XOSC_STARTUP_DELAY_MULTIPLIER 64


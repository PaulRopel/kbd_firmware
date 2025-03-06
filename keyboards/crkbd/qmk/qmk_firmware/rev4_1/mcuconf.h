#pragma once

#include_next "mcuconf.h"

// I2C configuration
#undef RP_I2C_USE_I2C1
#define RP_I2C_USE_I2C1 TRUE

// Enable hardware UART1 for split communication
#undef RP_UART_USE_UART1
#define RP_UART_USE_UART1 TRUE

// UART1 configuration for split
#define RP_UART1_PARITY UART_PARITY_NONE
#define RP_UART1_STOP_BITS UART_STOP_BITS_1
#define RP_UART1_DATA_BITS UART_DATA_BITS_8
#define RP_UART1_FLOW_CONTROL UART_FLOW_CONTROL_NONE
#define RP_UART1_BAUDRATE 115200  // Match the baudrate in config.h

// Enable USB for CDC
#undef RP_USB_USE_USBD
#define RP_USB_USE_USBD TRUE

// Enable PIO0 for LEDs and PIO1 for split communication
#undef RP_PIO_USE_PIO0
#define RP_PIO_USE_PIO0 TRUE   // Keep enabled for LED control


// Enable USB CDC for debug
#undef RP_USB_USE_USB1
#define RP_USB_USE_USB1 TRUE

#pragma once

#include_next "mcuconf.h"

// I2C configuration
#undef RP_I2C_USE_I2C1
#define RP_I2C_USE_I2C1 TRUE

// Enable SIO for split communication (UART0)
#undef RP_SIO_USE_UART0
#define RP_SIO_USE_UART0 TRUE

// SIO configuration for split
#define RP_UART0_PARITY UART_PARITY_NONE
#define RP_UART0_STOP_BITS UART_STOP_BITS_1
#define RP_UART0_DATA_BITS UART_DATA_BITS_8
#define RP_UART0_FLOW_CONTROL UART_FLOW_CONTROL_NONE

// Enable USB for CDC
#undef RP_USB_USE_USBD
#define RP_USB_USE_USBD TRUE

// Enable PIO0 for LEDs only
#undef RP_PIO_USE_PIO0
#define RP_PIO_USE_PIO0 TRUE   // Keep enabled for LED control

#undef RP_PIO_USE_PIO1
#define RP_PIO_USE_PIO1 FALSE  // Not needed since we're using SIO

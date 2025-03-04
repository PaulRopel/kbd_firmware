#pragma once

#include_next "mcuconf.h"

// I2C configuration
#undef RP_I2C_USE_I2C1
#define RP_I2C_USE_I2C1 TRUE

// Configure UART for split communication
#undef RP_SIO_USE_UART0
#define RP_SIO_USE_UART0 TRUE

#undef RP_SIO_USE_UART1
#define RP_SIO_USE_UART1 FALSE

// Disable PIO as we're using hardware UART
#undef RP_PIO_USE_PIO0
#define RP_PIO_USE_PIO0 FALSE

#undef RP_PIO_USE_PIO1
#define RP_PIO_USE_PIO1 FALSE

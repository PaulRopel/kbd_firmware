#pragma once

#include_next "mcuconf.h"

// I2C configuration
#undef RP_I2C_USE_I2C1
#define RP_I2C_USE_I2C1 TRUE

// Disable UART as we're using PIO for split communication
#undef RP_SIO_USE_UART0
#define RP_SIO_USE_UART0 FALSE

#undef RP_SIO_USE_UART1
#define RP_SIO_USE_UART1 FALSE

// Enable PIO0 for split communication
#undef RP_PIO_USE_PIO0
#define RP_PIO_USE_PIO0 TRUE

#undef RP_PIO_USE_PIO1
#define RP_PIO_USE_PIO1 FALSE  // Keep PIO1 disabled unless needed

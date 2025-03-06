#pragma once

// Enable required HAL subsystems
#define HAL_USE_I2C TRUE    // For OLED displays and other I2C devices
#define HAL_USE_PAL TRUE    // Required for GPIO control
#define HAL_USE_PIO TRUE    // For LED control
#define HAL_USE_USB TRUE    // For USB CDC debug
#define HAL_USE_SERIAL_USB TRUE  // For USB CDC ACM

// Enable callbacks
#define PAL_USE_CALLBACKS TRUE

// Disable standard serial driver since we're using the vendor driver
#define HAL_USE_SERIAL FALSE

// Enable USB serial for debug output
#define HAL_USE_USB_CDC TRUE

#include_next "halconf.h"
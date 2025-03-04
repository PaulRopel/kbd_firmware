#pragma once

// Enable required HAL subsystems
#define HAL_USE_I2C TRUE    // For OLED displays and other I2C devices
#define HAL_USE_SIO FALSE   // Disable SIO since we're using PIO
#define HAL_USE_PAL TRUE    // Required for GPIO control
#define HAL_USE_PIO TRUE    // For PIO-based serial communication

// Enable callbacks for PIO and PAL
#define PAL_USE_CALLBACKS TRUE
#define PIO_USE_CALLBACKS TRUE

#include_next "halconf.h"
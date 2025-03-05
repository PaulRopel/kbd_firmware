# Debug and Console options
CONSOLE_ENABLE = yes        # Enable debug console
COMMAND_ENABLE = yes        # Commands for debug and configuration
DEBUG_ENABLE = yes         # Enable debug print

# Force debug output to be enabled
# Remove problematic debug flags

# Keyboard specific options
SERIAL_DRIVER = vendor     # Use hardware UART driver

# MCU specific options
MCU_FAMILY = CHIBIOS
MCU = RP2040

# Additional compiler flags
EXTRAFLAGS += -flto       # Additional optimization flags

# Enable CDC for debug console
VIRTSER_ENABLE = yes      # Enable Virtual Serial Port
CDC_ENABLE = yes         # Enable CDC ACM for debug



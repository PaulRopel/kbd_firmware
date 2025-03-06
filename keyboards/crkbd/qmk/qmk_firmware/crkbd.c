#include QMK_KEYBOARD_H
#include "quantum.h"
#include "debug.h"
#include "print.h"
#include "split_util.h"
#include "transport.h"
#include "gpio.h"
#include "wait.h"
#include "platforms/chibios/gpio.h"
#include "platforms/chibios/vendors/RP/_pin_defs.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>  // For va_list, va_start, va_end
#include "config.h"
#include "atomic_util.h"

// Force even more reliable (but slower) settings
#define SERIAL_USART_FULL_DUPLEX

// Buffer for pre-init logs
#define PRE_INIT_LOG_SIZE 1024
static char pre_init_log_buffer[PRE_INIT_LOG_SIZE];
static int pre_init_log_pos = 0;

// Helper function to safely append to pre-init log buffer
static void append_pre_init_log(const char* format, ...) {
    if (pre_init_log_pos >= PRE_INIT_LOG_SIZE - 1) return; // Buffer full
    
    va_list args;
    va_start(args, format);
    int remaining = PRE_INIT_LOG_SIZE - pre_init_log_pos;
    int written = vsnprintf(&pre_init_log_buffer[pre_init_log_pos], remaining, format, args);
    va_end(args);
    
    if (written > 0 && written < remaining) {
        pre_init_log_pos += written;
    }
}

__attribute__((weak)) void keyboard_pre_init_user(void) {}
__attribute__((weak)) void keyboard_post_init_user(void) {}

static inline void setPinOutput_writeLow(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        setPinOutput(pin);
        writePinLow(pin);
    }
}

static inline void setPinOutput_writeHigh(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        setPinOutput(pin);
        writePinHigh(pin);
    }
}

static inline void setPinInputHigh_atomic(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        setPinInputHigh(pin);
    }
}

void keyboard_pre_init_kb(void) {
    // Initialize debug output first
    debug_enable = true;
    debug_matrix = true;
    debug_keyboard = true;
    
    // Add a very long delay at the beginning to ensure console is ready
    for (int i = 0; i < 20; i++) {
        wait_ms(100);  // 2 seconds total delay
    }
    
    // Buffer pre-init logs instead of printing directly
    append_pre_init_log("\n\n\n");  // Clear line buffer
    append_pre_init_log("!!!!! CRKBD-PRE-INIT-START !!!!!\n");
    
    // Add another delay after initial debug output
    wait_ms(200);
    
    append_pre_init_log("CRKBD-PRE-INIT: Debug enabled\n");
    
    // Initialize split detection pin with extra safeguards
    append_pre_init_log("CRKBD-PRE-INIT: Initializing split hand pin GP21\n");
    
    // First, try to reset the pin to a known state
    setPinOutput_writeLow(SPLIT_HAND_PIN);
    wait_ms(20);
    
    // Then set it as input with pull-up
    setPinInputHigh_atomic(SPLIT_HAND_PIN);
    wait_ms(200);  // Longer delay for pin stabilization
    
    // Read the pin multiple times to ensure stability
    uint8_t pin_reads[5] = {0};
    uint8_t consistent_reads = 0;
    
    for (int i = 0; i < 5; i++) {
        pin_reads[i] = readPin(SPLIT_HAND_PIN);
        wait_ms(20);
        
        // Check if this read matches the previous one
        if (i > 0 && pin_reads[i] == pin_reads[i-1]) {
            consistent_reads++;
        }
    }
    
    // Determine if we have stable readings
    bool stable_reading = (consistent_reads >= 3);
    
    // With SPLIT_HAND_PIN_HIGH_IS_LEFT, HIGH (1) = left side, LOW (0) = right side
    bool is_left = pin_reads[4] == 1;  // Use the last reading
    
    append_pre_init_log("CRKBD-PRE-INIT: Split hand pin readings: %d %d %d %d %d (stable: %d)\n", 
            pin_reads[0], pin_reads[1], pin_reads[2], pin_reads[3], pin_reads[4], stable_reading);
    append_pre_init_log("CRKBD-PRE-INIT: Final GP21 state: %d (is_left: %d)\n", pin_reads[4], is_left);
    
    if (!stable_reading) {
        append_pre_init_log("CRKBD-PRE-INIT: WARNING - Unstable split hand pin readings!\n");
    }
    
    // Configure UART pins based on whether we're left or right half
    append_pre_init_log("CRKBD-PRE-INIT: Configuring as %s half\n", is_left ? "LEFT" : "RIGHT");
    
    if (is_left) {
        // Left side UART configuration
        append_pre_init_log("CRKBD-PRE-INIT: LEFT half uses TX: GP4, RX: GP5\n");
        
        // Configure TX pin (GP4 for left)
        setPinOutput_writeLow(SERIAL_USART_TX_PIN_LEFT);  // First set LOW
        wait_ms(20);
        setPinOutput_writeHigh(SERIAL_USART_TX_PIN_LEFT); // Then set HIGH
        wait_ms(20);
        
        // Configure RX pin (GP5 for left)
        setPinInputHigh_atomic(SERIAL_USART_RX_PIN_LEFT);
        wait_ms(20);
        
        // Verify pin states
        uint8_t tx_state = readPin(SERIAL_USART_TX_PIN_LEFT);
        uint8_t rx_state = readPin(SERIAL_USART_RX_PIN_LEFT);
        
        append_pre_init_log("CRKBD-PRE-INIT: LEFT TX pin (GP4) state: %d\n", tx_state);
        append_pre_init_log("CRKBD-PRE-INIT: LEFT RX pin (GP5) state: %d\n", rx_state);
        
        if (tx_state != 1) {
            append_pre_init_log("CRKBD-PRE-INIT: WARNING - LEFT TX pin not HIGH!\n");
        }
    } else {
        // Right side UART configuration
        append_pre_init_log("CRKBD-PRE-INIT: RIGHT half uses TX: GP24, RX: GP25\n");
        
        // Configure TX pin (GP24 for right)
        setPinOutput_writeLow(SERIAL_USART_TX_PIN_RIGHT);  // First set LOW
        wait_ms(20);
        setPinOutput_writeHigh(SERIAL_USART_TX_PIN_RIGHT); // Then set HIGH
        wait_ms(20);
        
        // Configure RX pin (GP25 for right)
        setPinInputHigh_atomic(SERIAL_USART_RX_PIN_RIGHT);
        wait_ms(20);
        
        // Verify pin states
        uint8_t tx_state = readPin(SERIAL_USART_TX_PIN_RIGHT);
        uint8_t rx_state = readPin(SERIAL_USART_RX_PIN_RIGHT);
        
        append_pre_init_log("CRKBD-PRE-INIT: RIGHT TX pin (GP24) state: %d\n", tx_state);
        append_pre_init_log("CRKBD-PRE-INIT: RIGHT RX pin (GP25) state: %d\n", rx_state);
        
        if (tx_state != 1) {
            append_pre_init_log("CRKBD-PRE-INIT: WARNING - RIGHT TX pin not HIGH!\n");
        }
    }
    
    append_pre_init_log("CRKBD-PRE-INIT: UART pins configured\n");
    
    // Call the user function last
    keyboard_pre_init_user();
}

void keyboard_post_init_kb(void) {
    // Add distinctive markers
    xprintf("\n\n\n");  // Clear line buffer
    xprintf("!!!!! CRKBD-POST-INIT-START !!!!!\n");
    
    // Add a delay to ensure console is ready
    wait_ms(100);
    
    // Output the buffered pre-init logs
    if (pre_init_log_pos > 0) {
        xprintf("%s", pre_init_log_buffer);
        pre_init_log_pos = 0;  // Reset buffer position
    }
    
    // Check split hand pin to determine if we're left or right
    // With SPLIT_HAND_PIN_HIGH_IS_LEFT, HIGH (1) = left side, LOW (0) = right side
    bool is_left = readPin(SPLIT_HAND_PIN) == 1;
    xprintf("CRKBD-POST-INIT: Split hand pin GP21 state: %lu (is_left: %d)\n", readPin(SPLIT_HAND_PIN), is_left);
    
    // Verify UART pins are properly configured
    xprintf("CRKBD-POST-INIT: Verifying UART pins\n");
    
    if (is_left) {
        // Verify left side UART pins
        xprintf("CRKBD-POST-INIT: LEFT TX pin (GP4) state: %lu\n", readPin(SERIAL_USART_TX_PIN_LEFT));
        xprintf("CRKBD-POST-INIT: LEFT RX pin (GP5) state: %lu\n", readPin(SERIAL_USART_RX_PIN_LEFT));
        
        // If TX pin is not HIGH, reset it
        if (readPin(SERIAL_USART_TX_PIN_LEFT) != 1) {
            xprintf("CRKBD-POST-INIT: Resetting LEFT TX pin (GP4)\n");
            setPinOutput_writeHigh(SERIAL_USART_TX_PIN_LEFT);
        }
    } else {
        // Verify right side UART pins
        xprintf("CRKBD-POST-INIT: RIGHT TX pin (GP24) state: %lu\n", readPin(SERIAL_USART_TX_PIN_RIGHT));
        xprintf("CRKBD-POST-INIT: RIGHT RX pin (GP25) state: %lu\n", readPin(SERIAL_USART_RX_PIN_RIGHT));
        
        // If TX pin is not HIGH, reset it
        if (readPin(SERIAL_USART_TX_PIN_RIGHT) != 1) {
            xprintf("CRKBD-POST-INIT: Resetting RIGHT TX pin (GP24)\n");
            setPinOutput_writeHigh(SERIAL_USART_TX_PIN_RIGHT);
        }
    }
    
    // Simplify the function to avoid potential freeze points
    xprintf("CRKBD-POST-INIT: Calling keyboard_post_init_user\n");
    
    // Call the user function
    keyboard_post_init_user();
}

void housekeeping_task_kb(void) {
    static uint32_t counter = 0;
    static uint8_t last_split_pin_state = 255; // Invalid initial value
    static bool last_is_left = false;
    static bool first_check_done = false;
    
    // Print alive message less frequently to reduce log spam
    if (counter % 5000 == 0) {
        xprintf("HOUSEKEEPING-ALIVE: %lu\n", counter);
    }
    
    // Check split hand pin and UART pins every 10000 iterations
    if (counter % 10000 == 0) {
        // Check split hand pin
        uint8_t pin_state = readPin(SPLIT_HAND_PIN);
        bool is_left = pin_state == 1;  // With SPLIT_HAND_PIN_HIGH_IS_LEFT, HIGH (1) = left side, LOW (0) = right side
        
        // Only log if this is the first check or if the state has changed
        if (!first_check_done || pin_state != last_split_pin_state) {
            xprintf("SPLIT-HAND: Pin GP21 state: %d (is_left: %d)", pin_state, is_left);
            
            if (first_check_done && pin_state != last_split_pin_state) {
                xprintf(" - CHANGED from previous state: %d!\n", last_split_pin_state);
            } else {
                xprintf("\n");
            }
            
            // If the left/right status changed, this is a serious issue
            if (first_check_done && is_left != last_is_left) {
                xprintf("SPLIT-HAND: WARNING - LEFT/RIGHT STATUS CHANGED! Was: %s, Now: %s\n",
                        last_is_left ? "LEFT" : "RIGHT", is_left ? "LEFT" : "RIGHT");
                
                // Reconfigure UART pins if the side changed
                if (is_left) {
                    xprintf("SPLIT-HAND: Reconfiguring as LEFT half\n");
                    setPinOutput_writeHigh(SERIAL_USART_TX_PIN_LEFT);
                    setPinInputHigh_atomic(SERIAL_USART_RX_PIN_LEFT);
                } else {
                    xprintf("SPLIT-HAND: Reconfiguring as RIGHT half\n");
                    setPinOutput_writeHigh(SERIAL_USART_TX_PIN_RIGHT);
                    setPinInputHigh_atomic(SERIAL_USART_RX_PIN_RIGHT);
                }
            }
            
            // Update stored state
            last_split_pin_state = pin_state;
            last_is_left = is_left;
            first_check_done = true;
        }
        
        // Check UART pin states based on which half we are
        if (is_left) {
            // Check left half UART pins
            uint8_t tx_state = readPin(SERIAL_USART_TX_PIN_LEFT);
            uint8_t rx_state = readPin(SERIAL_USART_RX_PIN_LEFT);
            
            // Only log if there's an issue with the pins
            if (tx_state != 1 || rx_state != 1) {
                xprintf("UART-PINS-LEFT: TX (GP4): %d, RX (GP5): %d - NOT IN EXPECTED STATE\n", 
                        tx_state, rx_state);
                
                // If TX pin is not HIGH, reset it
                if (tx_state != 1) {
                    xprintf("UART-PINS-LEFT: Resetting TX pin (GP4)\n");
                    setPinOutput_writeHigh(SERIAL_USART_TX_PIN_LEFT);
                }
                
                // If RX pin is not HIGH (due to pull-up), reset it
                if (rx_state != 1) {
                    xprintf("UART-PINS-LEFT: Resetting RX pin (GP5)\n");
                    setPinInputHigh_atomic(SERIAL_USART_RX_PIN_LEFT);
                }
            }
        } else {
            // Check right half UART pins
            uint8_t tx_state = readPin(SERIAL_USART_TX_PIN_RIGHT);
            uint8_t rx_state = readPin(SERIAL_USART_RX_PIN_RIGHT);
            
            // Only log if there's an issue with the pins
            if (tx_state != 1 || rx_state != 1) {
                xprintf("UART-PINS-RIGHT: TX (GP24): %d, RX (GP25): %d - NOT IN EXPECTED STATE\n", 
                        tx_state, rx_state);
                
                // If TX pin is not HIGH, reset it
                if (tx_state != 1) {
                    xprintf("UART-PINS-RIGHT: Resetting TX pin (GP24)\n");
                    setPinOutput_writeHigh(SERIAL_USART_TX_PIN_RIGHT);
                }
                
                // If RX pin is not HIGH (due to pull-up), reset it
                if (rx_state != 1) {
                    xprintf("UART-PINS-RIGHT: Resetting RX pin (GP25)\n");
                    setPinInputHigh_atomic(SERIAL_USART_RX_PIN_RIGHT);
                }
            }
        }
    }
    
    // Increment counter before any potential freeze point
    counter++;
    
    // Call the user function
    housekeeping_task_user();
}

#ifdef OLED_ENABLE
oled_rotation_t oled_init_kb(oled_rotation_t rotation) {
    if (!is_keyboard_master()) {
        return OLED_ROTATION_180; // flips the display 180 degrees if offhand
    }
    return rotation;
}

static void oled_render_layer_state(void) {
    oled_write_P(PSTR("Layer: "), false);
    switch (get_highest_layer(layer_state)) {
        case 0:
            oled_write_ln_P(PSTR("Default"), false);
            break;
        case 1:
            oled_write_ln_P(PSTR("Lower"), false);
            break;
        case 2:
            oled_write_ln_P(PSTR("Raise"), false);
            break;
        case 3:
            oled_write_ln_P(PSTR("Adjust"), false);
            break;
        default:
            oled_write_ln_P(PSTR("Undef"), false);
            break;
    }
}

char     key_name = ' ';
uint16_t last_keycode;
uint8_t  last_row;
uint8_t  last_col;

static const char PROGMEM code_to_name[60] = {' ', ' ', ' ', ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'R', 'E', 'B', 'T', '_', '-', '=', '[', ']', '\\', '#', ';', '\'', '`', ',', '.', '/', ' ', ' ', ' '};

static void set_keylog(uint16_t keycode, keyrecord_t *record) {
    // save the row and column (useful even if we can't find a keycode to show)
    last_row = record->event.key.row;
    last_col = record->event.key.col;

    key_name     = ' ';
    last_keycode = keycode;
    if (IS_QK_MOD_TAP(keycode)) {
        if (record->tap.count) {
            keycode = QK_MOD_TAP_GET_TAP_KEYCODE(keycode);
        } else {
            keycode = 0xE0 + biton(QK_MOD_TAP_GET_MODS(keycode) & 0xF) + biton(QK_MOD_TAP_GET_MODS(keycode) & 0x10);
        }
    } else if (IS_QK_LAYER_TAP(keycode) && record->tap.count) {
        keycode = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
    } else if (IS_QK_MODS(keycode)) {
        keycode = QK_MODS_GET_BASIC_KEYCODE(keycode);
    } else if (IS_QK_ONE_SHOT_MOD(keycode)) {
        keycode = 0xE0 + biton(QK_ONE_SHOT_MOD_GET_MODS(keycode) & 0xF) + biton(QK_ONE_SHOT_MOD_GET_MODS(keycode) & 0x10);
    }
    if (keycode > ARRAY_SIZE(code_to_name)) {
        return;
    }

    // update keylog
    key_name = pgm_read_byte(&code_to_name[keycode]);
}

static const char *depad_str(const char *depad_str, char depad_char) {
    while (*depad_str == depad_char)
        ++depad_str;
    return depad_str;
}

static void oled_render_keylog(void) {
    oled_write_char('0' + last_row, false);
    oled_write_P(PSTR("x"), false);
    oled_write_char('0' + last_col, false);
    oled_write_P(PSTR(", k"), false);
    const char *last_keycode_str = get_u16_str(last_keycode, ' ');
    oled_write(depad_str(last_keycode_str, ' '), false);
    oled_write_P(PSTR(":"), false);
    oled_write_char(key_name, false);
}

// static void render_bootmagic_status(bool status) {
//     /* Show Ctrl-Gui Swap options */
//     static const char PROGMEM logo[][2][3] = {
//         {{0x97, 0x98, 0}, {0xb7, 0xb8, 0}},
//         {{0x95, 0x96, 0}, {0xb5, 0xb6, 0}},
//     };
//     if (status) {
//         oled_write_ln_P(logo[0][0], false);
//         oled_write_ln_P(logo[0][1], false);
//     } else {
//         oled_write_ln_P(logo[1][0], false);
//         oled_write_ln_P(logo[1][1], false);
//     }
// }

__attribute__((weak)) void oled_render_logo(void) {
    // clang-format off
    static const char PROGMEM crkbd_logo[] = {
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94,
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4,
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4,
        0};
    // clang-format on
    oled_write_P(crkbd_logo, false);
}

bool oled_task_kb(void) {
    if (!oled_task_user()) {
        return false;
    }
    if (is_keyboard_master()) {
        oled_render_layer_state();
        oled_render_keylog();
    } else {
        oled_render_logo();
    }
    return false;
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    // If console is enabled, it will print the matrix position and status of each key pressed
    #ifdef CONSOLE_ENABLE
        uprintf("KL: kc: 0x%04X, col: %2u, row: %2u, pressed: %u\n", 
                keycode, record->event.key.col, record->event.key.row, record->event.pressed);
    #endif 
    if (record->event.pressed) {
        set_keylog(keycode, record);
    }
    return process_record_user(keycode, record);
}
#endif // OLED_ENABLE

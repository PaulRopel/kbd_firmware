#include QMK_KEYBOARD_H
#include "quantum.h"
#include "debug.h"
#include "print.h"
#include "split_util.h"
#include "gpio.h"
#include "wait.h"
#include "platforms/chibios/gpio.h"
#include "platforms/chibios/vendors/RP/_pin_defs.h"
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "atomic_util.h"

static uint32_t last_debug_time = 0;

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
    // Force debug to be enabled using the macro directly
    debug_enable = true;
    debug_matrix = true;
    debug_keyboard = true;
    
    // Add direct console output that doesn't rely on debug_enable
    xprintf("CRKBD: Console test message from keyboard_pre_init_kb\n");
    xprintf("DEBUG: Debug enabled on %s side\n", is_keyboard_master() ? "master" : "slave");
    
    // Initialize split detection pin
    setPinInputHigh_atomic(SPLIT_HAND_PIN);
    xprintf("DEBUG: Split hand pin GP%d initialized\n", SPLIT_HAND_PIN);
    wait_ms(1);

    // Read split detection pin to determine if we're left or right
    // SPLIT_HAND_PIN_HIGH_IS_LEFT is defined, so HIGH (1) = left side, LOW (0) = right side
    bool is_left = readPin(SPLIT_HAND_PIN) == 1;  // HIGH (1) = left side with SPLIT_HAND_PIN_HIGH_IS_LEFT
    xprintf("DEBUG: Split hand pin GP%d read: %lu (is_left: %d)\n", SPLIT_HAND_PIN, readPin(SPLIT_HAND_PIN), is_left);
    xprintf("DEBUG: is_keyboard_left() reports: %d\n", is_keyboard_left());
    xprintf("DEBUG: is_keyboard_right() reports: %d\n", !is_keyboard_left());

    if (is_left) {
        xprintf("DEBUG: Left side detected, configuring TX:GP%d RX:GP%d\n", SERIAL_USART_TX_PIN_LEFT, SERIAL_USART_RX_PIN_LEFT);
        setPinOutput_writeHigh(SERIAL_USART_TX_PIN_LEFT);
        setPinInputHigh_atomic(SERIAL_USART_RX_PIN_LEFT);
        xprintf("DEBUG: Left side UART pins configured\n");
    } else {
        xprintf("DEBUG: Right side detected, configuring TX:GP%d RX:GP%d\n", SERIAL_USART_TX_PIN_RIGHT, SERIAL_USART_RX_PIN_RIGHT);
        setPinOutput_writeHigh(SERIAL_USART_TX_PIN_RIGHT);
        setPinInputHigh_atomic(SERIAL_USART_RX_PIN_RIGHT);
        xprintf("DEBUG: Right side UART pins configured\n");
    }

    xprintf("DEBUG: Keyboard pre-init complete on %s side\n", is_keyboard_master() ? "master" : "slave");
    keyboard_pre_init_user();
}

void keyboard_post_init_kb(void) {
    // Add direct console output that doesn't rely on debug_enable
    xprintf("CRKBD: Console test message from keyboard_post_init_kb\n");
    
    xprintf("DEBUG: Keyboard post-init starting on %s side...\n", is_keyboard_master() ? "master" : "slave");
    keyboard_post_init_user();
    xprintf("DEBUG: Keyboard post-init complete on %s side\n", is_keyboard_master() ? "master" : "slave");
}

void housekeeping_task_kb(void) {
    // Add direct console output that doesn't rely on debug_enable
    static uint32_t last_print = 0;
    if (timer_elapsed32(last_print) > 5000) {  // Print every 5 seconds
        // Force debug to be enabled using the macro directly
        debug_enable = true;
        debug_matrix = true;
        debug_keyboard = true;
        
        xprintf("CRKBD: Console test message from housekeeping_task_kb\n");
        xprintf("DEBUG: This is a test debug message from housekeeping_task_kb\n");
        last_print = timer_read32();
    }
    
    // Print debug message every 5 seconds
    if (timer_elapsed32(last_debug_time) > 5000) {
        xprintf("DEBUG: Keyboard is alive - %lu ms\n", timer_read32());
        xprintf("DEBUG: Split hand pin GP%d read: %lu\n", SPLIT_HAND_PIN, readPin(SPLIT_HAND_PIN));
        xprintf("DEBUG: is_keyboard_left(): %d, is_keyboard_master(): %d\n", 
                is_keyboard_left(), 
                is_keyboard_master());
        last_debug_time = timer_read32();
    }
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

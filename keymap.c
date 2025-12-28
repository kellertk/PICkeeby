/*
  Copyright (c) 2007 Free Software Foundation.  All right reserved.
  Written by Christian Weichel <info@32leaves.net>

  ** Adapted for PIC by Tom Keller <tom@tompkel.net> 2025
  ** Mostly rewritten Paul Stoffregen <paul@pjrc.com> 2010, 2011
  ** Modified for use beginning with Arduino 13 by L. Abraham Smith, <n3bah@microcompdesign.com> *
  ** Modified for easy interrup pin assignement on method begin(datapin,irq_pin). Cuningan <cuninganreset@gmail.com> **

  for more information you can read the original wiki in arduino.cc
  at http://www.arduino.cc/playground/Main/PS2Keyboard
  or http://www.pjrc.com/teensy/td_libs_PS2Keyboard.html

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "keymap.h"
#include "ps2_send.h"
#include <stdint.h>

// PS/2 Scan Code Set 2 - Input scancodes for modifier key detection
#define PS2_LSHIFT  0x12
#define PS2_RSHIFT  0x59
#define PS2_CAPS    0x58
#define PS2_NUM     0x77
#define PS2_SCROLL  0x7E

// PS/2 Extended key scancodes (0xE0 prefix)
#define PS2_EXT_INS    0x70
#define PS2_EXT_HOME   0x6C
#define PS2_EXT_PGUP   0x7D
#define PS2_EXT_DEL    0x71
#define PS2_EXT_END    0x69
#define PS2_EXT_PGDN   0x7A
#define PS2_EXT_UP     0x75
#define PS2_EXT_DOWN   0x72
#define PS2_EXT_LEFT   0x6B
#define PS2_EXT_RIGHT  0x74
#define PS2_EXT_ENTER  0x5A
#define PS2_EXT_SLASH  0x4A
#define PS2_EXT_CTRL_R 0x14
#define PS2_EXT_ALT_R  0x11
#define PS2_EXT_WIN_L  0x1F
#define PS2_EXT_WIN_R  0x27
#define PS2_EXT_MENU   0x2F

const Keymap EN_US = {
    {
        0,  F9, 0, F5, F3, F1, F2, F12,
        0, F10, F8, F6, F4, TAB, '`', 0,
        0, 0, 0, 0, 0, 'q', '1', 0,
        0, 0, 'z', 's', 'a', 'w', '2', 0,
        0, 'c', 'x', 'd', 'e', '4', '3', 0,
        0, ' ', 'v', 'f', 't', 'r', '5', 0,
        0, 'n', 'b', 'h', 'g', 'y', '6', 0,
        0, 0, 'm', 'j', 'u', '7', '8', 0,
        0, ',', '.', 'k', 'i', '0', '9', 0,
        0, '.', '/', 'l', ';', 'p', '-', 0,
        0, 0, '\'', 0, '[', '=', 0, 0,
        CAPS, 0, ENTER, ']', 0, '\\', 0, 0,
        0, 0, 0, 0, 0, 0, BKSP, 0,
        0, '1', 0, '4', '7', 0, 0, 0,
        0, '.', '2', '5', '6', '8', ESC, NUM,
        F11, '+', '3', '-', '*', '9', SCROL, 0,
        0, 0, 0, F7
    },
    {
        0, F9, 0, F5, F3, F1, F2, F12,
        0, F10, F8, F6, F4, TAB, '~', 0,
        0, 0, 0, 0, 0, 'Q', '!', 0,
        0, 0, 'Z', 'S', 'A', 'W', '@', 0,
        0, 'C', 'X', 'D', 'E', '$', '#', 0,
        0, ' ', 'V', 'F', 'T', 'R', '%', 0,
        0, 'N', 'B', 'H', 'G', 'Y', '^', 0,
        0, 0, 'M', 'J', 'U', '&', '*', 0,
        0, '<', '>', 'K', 'I', ')', '(', 0,
        0, '?', '/', 'L', ':', 'P', '_', 0,
        0, 0, '"', 0, '{', '+', 0, 0,
        CAPS, 0, ENTER, '}', 0, '|', 0, 0,
        0, 0, 0, 0, 0, 0, BKSP, 0,
        0, END, 0, '4', HOME, 0, 0, 0,
        INS, DEL, '2', '5', '6', '8', ESC, NUM,
        F11, '+', PGDN, '-', '*', PGUP, SCROL, 0,
        0, 0, 0, F7
    }
};

// Modifier bit flags for modifiers variable
#define BREAK       0b0001
#define EXTEND      0b0010
#define SHIFT_L_BIT 0b0100
#define SHIFT_R_BIT 0b1000

// Static state that persists across calls
static uint8_t modifier_flags = 0;
static uint8_t caps_lock = 0;
static uint8_t num_lock = 0;
static uint8_t scroll_lock = 0;

static void updateLEDs(void) {
    uint8_t led_byte = (uint8_t)((scroll_lock) | (num_lock << 1) | (caps_lock << 2));
    ps2_setLEDs(led_byte);
}

static int get8859Code(uint8_t code) {
    if (!code) return 0;
    int c;

    // Handle keyboard power-on/reset (BAT complete)
    if (code == 0xAA) {
        num_lock = 1;
        ps2_initKeyboard();
        return -1;
    }

    // Handle BAT fail - attempt reset
    if (code == 0xFC) {
        ps2_reset();
        return -1;
    }

    // Ignore other PS/2 status bytes
    if (code == 0xEE || code == 0xFA ||
        code == 0xFD || code == 0xFE || code == 0xFF) {
        return -1;
    }

    // Handle multibyte prefixes
    if (code == 0xF0) {
        // Key release, wait for next scancode
        modifier_flags |= BREAK;
        return -1;
    } else if (code == 0xE0) {
        // Extended scancode prefix, wait for next scancode
        modifier_flags |= EXTEND;
        return -1;
    } else {
        // Key event to handle - save break and extend flags
        uint8_t is_release = modifier_flags & BREAK;
        uint8_t is_extended = modifier_flags & EXTEND;

        // Handle state-modifying keys (shift, lock keys)
        if (!is_release) {
            // Press events
            if (code == PS2_LSHIFT) {
                modifier_flags |= SHIFT_L_BIT;
                modifier_flags &= ~(BREAK | EXTEND);
                return SHIFT_L;
            } else if (code == PS2_RSHIFT) {
                modifier_flags |= SHIFT_R_BIT;
                modifier_flags &= ~(BREAK | EXTEND);
                return SHIFT_R;
            } else if (code == PS2_CAPS) {
                caps_lock ^= 1;
                updateLEDs();
                modifier_flags &= ~(BREAK | EXTEND);
                return CAPS;
            } else if (code == PS2_NUM) {
                num_lock ^= 1;
                updateLEDs();
                modifier_flags &= ~(BREAK | EXTEND);
                return NUM;
            } else if (code == PS2_SCROLL) {
                scroll_lock ^= 1;
                updateLEDs();
                modifier_flags &= ~(BREAK | EXTEND);
                return SCROL;
            }
        } else {
            // Release events
            if (code == PS2_LSHIFT) {
                modifier_flags &= ~SHIFT_L_BIT;
                modifier_flags &= ~(BREAK | EXTEND);
                return SHIFT_L | 0x40;
            } else if (code == PS2_RSHIFT) {
                modifier_flags &= ~SHIFT_R_BIT;
                modifier_flags &= ~(BREAK | EXTEND);
                return SHIFT_R | 0x40;
            } else if (code == PS2_CAPS) {
                modifier_flags &= ~(BREAK | EXTEND);
                return CAPS | 0x40;
            } else if (code == PS2_NUM) {
                modifier_flags &= ~(BREAK | EXTEND);
                return NUM | 0x40;
            } else if (code == PS2_SCROLL) {
                modifier_flags &= ~(BREAK | EXTEND);
                return SCROL | 0x40;
            }
        }

        // Not a state-modifying key - look up character
        c = 0;
        if (!is_extended) {
            // Regular keys (not extended)
            uint8_t shift_pressed = modifier_flags & (SHIFT_L_BIT | SHIFT_R_BIT);

            // Numpad keys: num_lock controls which map to use, shift inverts
            if (code >= 0x69 && code <= 0x7D) {
                if ((num_lock && !shift_pressed) || (!num_lock && shift_pressed)) {
                    c = EN_US.normal[code];
                } else {
                    c = EN_US.shifted[code];
                }
            } else {
                // Regular keys: check if we need to invert due to caps lock
                uint8_t use_shifted = shift_pressed;
                if (caps_lock) {
                    // Check if either map has a letter for this scancode
                    uint8_t normal_char = EN_US.normal[code];
                    uint8_t shifted_char = EN_US.shifted[code];
                    if ((normal_char >= 'a' && normal_char <= 'z') ||
                        (normal_char >= 'A' && normal_char <= 'Z') ||
                        (shifted_char >= 'a' && shifted_char <= 'z') ||
                        (shifted_char >= 'A' && shifted_char <= 'Z')) {
                        use_shifted = !use_shifted;  // Caps inverts the map
                    }
                }
                c = use_shifted ? EN_US.shifted[code] : EN_US.normal[code];
            }
        } else {
            // Extended keys (0xE0 prefix): navigation, numpad, modifiers
            switch (code) {
                // Navigation cluster
                case PS2_EXT_INS:   c = INS;   break;
                case PS2_EXT_HOME:  c = HOME;  break;
                case PS2_EXT_PGUP:  c = PGUP;  break;
                case PS2_EXT_DEL:   c = DEL;   break;
                case PS2_EXT_END:   c = END;   break;
                case PS2_EXT_PGDN:  c = PGDN;  break;
                case PS2_EXT_UP:    c = UP;    break;
                case PS2_EXT_DOWN:  c = DOWN;  break;
                case PS2_EXT_LEFT:  c = LEFT;  break;
                case PS2_EXT_RIGHT: c = RIGHT; break;
                // Extended numpad
                case PS2_EXT_ENTER: c = ENTER; break;
                case PS2_EXT_SLASH: c = '/';   break;
                // Extended modifier keys
                case PS2_EXT_CTRL_R: c = CTRL_R; break;
                case PS2_EXT_ALT_R:  c = ALT_R;  break;
                case PS2_EXT_WIN_L:  c = WIN_L;  break;
                case PS2_EXT_WIN_R:  c = WIN_R;  break;
                case PS2_EXT_MENU:   c = MENU;   break;
                default: break;
            }
        }

        // Clear flags now that we've handled the key
        modifier_flags &= ~(BREAK | EXTEND);

        // Return character with release bit set if needed
        if (c) {
            if (is_release) {
                // Only special keys (bit 7 set) send release events
                if (c & 0x80) {
                    return c | 0x40;  // Set bit 6 for release
                }
                return -1;  // Ignore release for regular keys
            }
            return c;  // Press event
        }
        return -1;
    }
}


static uint8_t UTF8buffer = 0;

int getkbdchar(uint8_t code) {
    int result;
    result = UTF8buffer;
    if (result) {
        UTF8buffer = 0;
    } else {
        result = get8859Code(code);
        if (result >= 128) {
            UTF8buffer = (result & 0x3F) | 0x80;
            result = (result >> 6) | 0xC0;
        }
    }
    if (!result) return -1;
    return result;
}

int getkbdcharn(uint8_t code) {
    int result = get8859Code(code);
    if(!result) return -1;
    UTF8buffer = 0;
    return result;
}

int hasUTF8Buffered(void) {
    return UTF8buffer != 0;
}

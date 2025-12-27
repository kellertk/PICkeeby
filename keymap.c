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
#include <stdint.h>

// PS/2 Scan Code Set 2 - Input scancodes for modifier key detection
#define PS2_LSHIFT  0x12
#define PS2_RSHIFT  0x59

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
#define BREAK      0b0001
#define EXTEND     0b0010
#define SHIFT_L_BIT 0b0100
#define SHIFT_R_BIT 0b1000

// Static state that persists across calls
static uint8_t modifiers = 0;


static int get8859Code(uint8_t code) {
    if (!code) return 0;
    int c;

    if (code == 0xF0) {
        // Key release, wait for next scancode
        modifiers |= BREAK;
        return -1;
    } else if (code == 0xE0) {
        // Extended scancode prefix, wait for next scancode
        modifiers |= EXTEND;
        return -1;
    } else {
        // Save break and extend flags before clearing them
        uint8_t is_release = modifiers & BREAK;
        uint8_t is_extended = modifiers & EXTEND;
        modifiers &= ~(BREAK | EXTEND);

        // Handle shift keys: update modifier state and send to host
        if (is_release) {
            if (code == PS2_LSHIFT) {
                modifiers &= ~SHIFT_L_BIT;
                return SHIFT_L | 0x40;  // Release: bit 7=1, bit 6=1
            } else if (code == PS2_RSHIFT) {
                modifiers &= ~SHIFT_R_BIT;
                return SHIFT_R | 0x40;
            }
        } else {
            if (code == PS2_LSHIFT) {
                modifiers |= SHIFT_L_BIT;
                return SHIFT_L;         // Press: bit 7=1, bit 6=0
            } else if (code == PS2_RSHIFT) {
                modifiers |= SHIFT_R_BIT;
                return SHIFT_R;
            }
        }

        // Look up character based on key type
        c = 0;
        if (is_extended) {
            // Extended keys (0xE0 prefix): navigation, numpad, modifiers
            switch (code) {
                // Navigation cluster
                case 0x70: c = INS;  break;
                case 0x6C: c = HOME; break;
                case 0x7D: c = PGUP; break;
                case 0x71: c = DEL;  break;
                case 0x69: c = END;  break;
                case 0x7A: c = PGDN; break;
                case 0x75: c = UP;   break;
                case 0x72: c = DOWN; break;
                case 0x6B: c = LEFT; break;
                case 0x74: c = RIGHT;break;
                // Extended numpad
                case 0x5A: c = ENTER; break;
                case 0x4A: c = '/';  break;
                // Extended modifier keys
                case 0x14: c = CTRL_R; break;
                case 0x11: c = ALT_R;  break;
                case 0x1F: c = WIN_L;  break;
                case 0x27: c = WIN_R;  break;
                case 0x2F: c = MENU;   break;
                default: break;
            }
        } else if ((modifiers & (SHIFT_L_BIT | SHIFT_R_BIT))) {
            // Shifted normal key: use shifted table
            c = EN_US.shifted[code];
        } else {
            // Normal key: use normal table
            c = EN_US.normal[code];
        }

        // Return character with release bit set if needed
        if (c) {
            if (is_release) {
                // Only special keys (bit 7 set) send release events
                if (c & 0x80) {
                    return c | 0x40;  // Set bit 6 for release
                }
                return -1;  // Regular ASCII: ignore release
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

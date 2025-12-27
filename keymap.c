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

#define ESC     0x27
#define F1      0x3B
#define F2      0x3C
#define F3      0x3D
#define F4      0x3E
#define F5      0x3F
#define F6      0x40
#define F7      0x41
#define F8      0x42
#define F9      0x43
#define F10     0x44
#define F11     0x57
#define F12     0x58
#define PRINT   0x46
#define SCROL   0x47
#define PAUSE   0x48
#define BKSP    0x0E
#define TAB     0x0F
#define CAPS    0x3A
#define ENTER   0x1C
#define LCTRL   0x1D
#define LSHIFT  0x2A
#define RSHIFT  0x36
#define LALT    0x38
#define SPACE   0x39
#define NUM     0x45
#define WIN     0x5B
#define MENU    0x5D
#define INS     0x52
#define DEL     0x53
#define HOME    0x47
#define END     0x4F
#define PGUP    0x49
#define PGDN    0x51
#define UP      0x48
#define DOWN    0x50
#define LEFT    0x4B
#define RIGHT   0x4D


const Keymap EN_US = {
    {
        0,  F9, 0, F5, F3, F1, F2, F12,
        0, F10, F8, F6, F4, TAB, '`', 0,
        0, LALT, LSHIFT, 0, LCTRL, 'q', '1', 0,
        0, 0, 'z', 's', 'a', 'w', '2', 0,
        0, 'c', 'x', 'd', 'e', '4', '3', 0,
        0, ' ', 'v', 'f', 't', 'r', '5', 0,
        0, 'n', 'b', 'h', 'g', 'y', '6', 0,
        0, 0, 'm', 'j', 'u', '7', '8', 0,
        0, ',', '.', 'k', 'i', '0', '9', 0,
        0, '.', '/', 'l', ';', 'p', '-', 0,
        0, 0, '\'', 0, '[', '=', 0, 0,
        CAPS, RSHIFT, ENTER, ']', 0, '\\', 0, 0,
        0, 0, 0, 0, 0, 0, BKSP, 0,
        0, '1', 0, '4', '7', 0, 0, 0,
        0, '.', '2', '5', '6', '8', ESC, NUM,
        F11, '+', '3', '-', '*', '9', SCROL, 0,
        0, 0, 0, F7
    },
    {
        0, F9, 0, F5, F3, F1, F2, F12,
        0, F10, F8, F6, F4, TAB, '~', 0,
        0, LALT, LSHIFT, 0, LCTRL, 'Q', '!', 0,
        0, 0, 'Z', 'S', 'A', 'W', '@', 0,
        0, 'C', 'X', 'D', 'E', '$', '#', 0,
        0, ' ', 'V', 'F', 'T', 'R', '%', 0,
        0, 'N', 'B', 'H', 'G', 'Y', '^', 0,
        0, 0, 'M', 'J', 'U', '&', '*', 0,
        0, '<', '>', 'K', 'I', ')', '(', 0,
        0, '?', '/', 'L', ':', 'P', '_', 0,
        0, 0, '"', 0, '{', '+', 0, 0,
        CAPS, RSHIFT, ENTER, '}', 0, '|', 0, 0,
        0, 0, 0, 0, 0, 0, BKSP, 0,
        0, END, 0, '4', HOME, 0, 0, 0,
        INS, DEL, '2', '5', '6', '8', ESC, NUM,
        F11, '+', PGDN, '-', '*', PGUP, SCROL, 0,
        0, 0, 0, F7
    }
};

#define BREAK   0x01
#define EXTEND  0x02
#define SHIFT_L 0x04
#define SHIFT_R 0x08
#define CTRL    0x10
#define ALT     0x20

// Static state that persists across calls
static uint8_t modifiers = 0;

static int get8859Code(uint8_t code) {
    int c;

    // Process one scancode per call - no loop
    if (!code) return 0;

    if (code == 0xF0) {
        modifiers |= BREAK;
        return -1;  // Wait for next scancode
    } else if (code == 0xE0) {
        modifiers |= EXTEND;
        return -1;  // Wait for next scancode
    } else {
        if (modifiers & BREAK) {
            // Key release
            if (code == LSHIFT) {
                modifiers &= ~SHIFT_L;
            } else if (code == RSHIFT) {
                modifiers &= ~SHIFT_R;
            } else if (code == LCTRL) {
                modifiers &= ~CTRL;
            } else if (code == LALT) {
                modifiers &= ~ALT;
            } else if (code == WIN) {
                modifiers &= ~WIN;
            }
            modifiers &= ~(BREAK | EXTEND);
            return -1;  // Key release, no character to return
        }

        // Key press
        if (code == LSHIFT) {
            modifiers |= SHIFT_L;
            modifiers &= ~(BREAK | EXTEND);
            return -1;
        } else if (code == RSHIFT) {
            modifiers |= SHIFT_R;
            modifiers &= ~(BREAK | EXTEND);
            return -1;
        } else if (code == LCTRL) {
            modifiers |= CTRL;
            modifiers &= ~(BREAK | EXTEND);
            return -1;
        } else if (code == LALT) {
            modifiers |= ALT;
            modifiers &= ~(BREAK | EXTEND);
            return -1;
        } else if (code == WIN) {
            modifiers |= WIN;
            modifiers &= ~(BREAK | EXTEND);
            return -1;
        }

        c = 0;
        if (modifiers & EXTEND) {
            // Extended scancodes (0xE0 prefix)
            if (code == LCTRL) {
                modifiers |= CTRL;
                modifiers &= ~EXTEND;
                return -1;
            } else if (code == LALT) {
                modifiers |= ALT;
                modifiers &= ~EXTEND;
                return -1;
            } else if (code == WIN) {
                modifiers |= WIN;
                modifiers &= ~EXTEND;
                return -1;
            }
            switch (code) {
                // Navigation keys
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
                // Numpad keys
                case 0x5A: c = '\n'; break;  // Numpad Enter
                case 0x4A: c = '/';  break;  // Numpad /
                // Menu/Apps key
                case 0x2F: c = MENU; break;
                default: break;
            }
        } else if ((modifiers & (SHIFT_L | SHIFT_R))) {
            c = EN_US.shifted[code];
        } else {
            c = EN_US.normal[code];
        }
        modifiers &= ~(BREAK | EXTEND);
        if (c) return c;
        return -1;
    }
}


static uint8_t UTF8buffer = 0;

void resetKeymap(void) {
    modifiers = 0;
    UTF8buffer = 0;
}

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

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

#ifndef keymap_h
#define keymap_h
#include <stdint.h>

#pragma warning disable 520

#define KEYMAP_SIZE 136

// Keymap values sent to host
// Bit 7 set for special keys
#define ESC     0x1B // Standard ASCII control codes
#define BKSP    0x08
#define TAB     0x09
#define ENTER   0x0D
#define SPACE   0x20
#define F1      0x80 // Special keys (0x80-0xA2)
#define F2      0x81
#define F3      0x82
#define F4      0x83
#define F5      0x84
#define F6      0x85
#define F7      0x86
#define F8      0x87
#define F9      0x88
#define F10     0x89
#define F11     0x8A
#define F12     0x8B
// Modifier and lock keys
#define CAPS    0x8C
#define SHIFT_L 0x8D
#define SHIFT_R 0x8E
#define CTRL_L  0x8F
#define CTRL_R  0x90
#define ALT_L   0x91
#define ALT_R   0x92
#define WIN_L   0x93
#define WIN_R   0x94
#define MENU    0x95

// Navigation cluster
#define INS     0x96
#define DEL     0x97
#define HOME    0x98
#define END     0x99
#define PGUP    0x9A
#define PGDN    0x9B
#define UP      0x9C
#define DOWN    0x9D
#define LEFT    0x9E
#define RIGHT   0x9F

// Lock keys
#define NUM     0xA0
#define SCROL   0xA1
#define PAUSE   0xA2

typedef struct {
    unsigned char normal[KEYMAP_SIZE];
    unsigned char shifted[KEYMAP_SIZE];
} Keymap;

extern const Keymap EN_US;

int getkbdchar(uint8_t code);
int getkbdcharn(uint8_t code);
int hasUTF8Buffered(void);

#endif

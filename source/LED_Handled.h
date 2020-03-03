/*
 * LEDhead for Wii
 * Copyright (C) 2017-2020 Nebiun
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
 
#ifndef _LED_Handled_h_
#define _LED_Handled_h_
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <stdarg.h>
#include <asndlib.h>
#include <mp3player.h>
#include <grrlib.h>

#define VAL_ROUNDING(v,d)	(((v) + (d) - 1)/(d))

#define IR_X_CORRECTION		64	
#define IR_Y_CORRECTION		20

#define realx(x)		((x) + (rmode->viWidth - 240)/2)
#define realy(y)		((y) + 20)

// draw_poweroff_a modes
#define POWER_POS_OFF	0
#define POWER_POS_MODE1	1
#define POWER_POS_MODE2	2

// draw_vblip/draw_oblip modes
#define BLIP_TYPE_NORMAL	0
#define BLIP_TYPE_BRIGHT	1
#define BLIP_TYPE_FLICKER	2

// draw_digit_f modes
#define DIGIT_TYPE_NORMAL	0	
#define DIGIT_TYPE_FLOAT	1	
#define DIGIT_TYPE_SPECIAL	2

extern int trace;
extern void print_text(int x, int y, u32 rgba, const char *fmt, ...);

extern int draw_digit(int x, int y, int val);
extern int print_digit(int x, int y, int pass, int val, int max);
extern int draw_digit_f(int x, int y, int val, int flag);

extern int draw_poweroff_a(int x, int y, int val);
extern int draw_vblip(int x, int y, int type);
extern int draw_oblip(int x, int y, int type);

extern void debugPrintf(int x, int y, u32 rgba, const char *fmt, ...);
extern void debugDestroy(void);
#endif
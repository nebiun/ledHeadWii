/*
 * LEDhead for Wii
 * Copyright (C) 2017-2020 Nebiun
 *
 * Based on the handheld electronic games by Mattel Electronics.
 * All trademarks copyrighted by their respective owners. This
 * program is not affiliated or endorsed by Mattel Electronics.
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

#define SCALE_X			_scale_x
#define SCALE_Y			_scale_y
#define realx(x)		(((x)*SCALE_X) + (rmode->viWidth - 240*SCALE_X)/2)
#define realy(y)		(((y)*SCALE_Y) + 20)

// draw_poweroff_a modes
#define SWITCH_POS_OFF		0
#define SWITCH_POS_ON		1
#define SWITCH_POS_MODE1	2
#define SWITCH_POS_MODE2	3
#define SWITCH_POS_PRO1		4
#define SWITCH_POS_PRO2		5

// draw_vblip/draw_oblip modes
#define BLIP_TYPE_NORMAL	0
#define BLIP_TYPE_BRIGHT	1
#define BLIP_TYPE_FLICKER	2

// draw_digit_f modes
#define DIGIT_TYPE_NORMAL	0	
#define DIGIT_TYPE_FLOAT	1	
#define DIGIT_TYPE_SPECIAL	2

// Switch types
#define SWITCH_TYPE_1		0		/* like baseball, football2 */
#define SWITCH_TYPE_2		1		/* like basketball, hockey, soccer */
#define SWITCH_TYPE_3		3		/* like football */
#define SWITCH_TYPE_4		4		/* like basketball2, soccer2 */

extern f32 _scale_x, _scale_y;

extern void print_text(int x, int y, u32 rgba, const char *fmt, ...);

extern int draw_digit(int x, int y, int val);
extern int print_digit(int x, int y, int pass, int val, int max);
extern int draw_digit_f(int x, int y, int val, int flag);

extern int draw_vblip(int x, int y, int type);
extern int draw_oblip(int x, int y, int type);

extern int draw_gear(int x, int y, int pos);
extern void destroy_gear(void);

extern int draw_switch(int type, int x, int y, int pos);
extern void destroy_switch(void);

extern void debugPrintf(int x, int y, u32 rgba, const char *fmt, ...);
extern void debugDestroy(void);
#endif
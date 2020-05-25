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

#ifndef __PLATFORM_BASKETBALL2_H__
#define __PLATFORM_BASKETBALL2_H__

// [general]
#define basketball2_digit_spacing 12
#define basketball2_digit_x 112
#define basketball2_digit_y 7
#define basketball2_digit_w 8
#define basketball2_digit_h 9
#define basketball2_blip_xspacing 22
#define basketball2_blip_yspacing 21
#define basketball2_blip_x 52
#define basketball2_blip_y 42
#define basketball2_power_off_x 29
#define basketball2_power_off_y 269
#define basketball2_basket_x 118
#define basketball2_basket_y 24

// Skills
#define LVL_HIGHSCHOOL		0		// Pro1
#define LVL_COLLAGE			1		// Pro2
#define LVL_ALLSTAR			2		// Pro1 + Man button
#define LVL_PROFESSIONAL	3		// Pro2 + Man button

// interface that the platform must provide for this game

// functions exported to the game context
void Basketball2_Init();
void Basketball2_Help();
void Basketball2_DeInit();
void Basketball2_Paint();
void Basketball2_ClearScreen();
void Basketball2_PlaySound(int nSound, unsigned int nFlags);
void Basketball2_StopSound();
void Basketball2_GetSize(int *w, int *h);

// "private" functions not exported to game context
void Basketball2_DrawBlip(int nBright, int x, int y);
void Basketball2_DrawStat(int nStat);
void Basketball2_DrawBasket(BOOL bBasket);

BOOL Basketball2_GetInputLEFT(BOOL *pChange);
BOOL Basketball2_GetInputLEFTUP(BOOL *pChange);
BOOL Basketball2_GetInputLEFTDOWN(BOOL *pChange);
BOOL Basketball2_GetInputRIGHT(BOOL *pChange);
BOOL Basketball2_GetInputRIGHTUP(BOOL *pChange);
BOOL Basketball2_GetInputRIGHTDOWN(BOOL *pChange);
BOOL Basketball2_GetInputUP(BOOL *pChange);
BOOL Basketball2_GetInputDOWN(BOOL *pChange);
BOOL Basketball2_GetInputSHOOT(BOOL *pChange);
BOOL Basketball2_GetInputPASS(BOOL *pChange);
BOOL Basketball2_TestForMovement();

#endif

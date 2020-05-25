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

#ifndef __PLATFORM_SOCCER2_H__
#define __PLATFORM_SOCCER2_H__

// [general]
#define soccer2_digit_spacing 	12
#define soccer2_digit_x 		112
#define soccer2_digit_y 		6
#define soccer2_digit_w 		8
#define soccer2_digit_h 		9
#define soccer2_blip_xspacing 	25
#define soccer2_blip_yspacing 	21
#define soccer2_blip_x 			43
#define soccer2_blip_y 			42
#define soccer2_power_off_x 	29
#define soccer2_power_off_y 	269
#define soccer2_goalarea_x		118
#define soccer2_goalarea_y		22

// Skills
#define LVL_ROOKIESLOW		0		// Pro1
#define LVL_ROOKIEFAST		1		// Pro2
#define LVL_PROSLOW			2		// Pro1 + Score button
#define LVL_PROFAST			3		// Pro2 + Score button

#define IS_LVLPRO(x)		(((x) == LVL_PROSLOW) || ((x) == LVL_PROFAST))

// interface that the platform must provide for this game

// functions exported to the game context
void Soccer2_Init();
void Soccer2_Help();
void Soccer2_DeInit();
void Soccer2_Paint();
void Soccer2_ClearScreen();
void Soccer2_PlaySound(int nSound, unsigned int nFlags);
void Soccer2_StopSound();
void Soccer2_GetSize(int *w, int *h);

// "private" functions not exported to game context
void Soccer2_DrawBlip(int nBright, int x, int y);
void Soccer2_DrawStat(int nStat);
void Soccer2_DrawSect(int nSect, BOOL side);
void Soccer2_DrawGoalareaFlag(BOOL status);

BOOL Soccer2_GetInputLEFTUP(BOOL *pChange);
BOOL Soccer2_GetInputLEFTDOWN(BOOL *pChange);
BOOL Soccer2_GetInputLEFT(BOOL *pChange);
BOOL Soccer2_GetInputRIGHTUP(BOOL *pChange);
BOOL Soccer2_GetInputRIGHTDOWN(BOOL *pChange);
BOOL Soccer2_GetInputUP(BOOL *pChange);
BOOL Soccer2_GetInputRIGHT(BOOL *pChange);
BOOL Soccer2_GetInputDOWN(BOOL *pChange);
BOOL Soccer2_GetInputKICK(BOOL *pChange);
BOOL Soccer2_GetInputPASS(BOOL *pChange);
BOOL Soccer2_GetInputSHOOT(BOOL *pChange);
BOOL Soccer2_GetInputTEAM(BOOL *pChange);
BOOL Soccer2_GetInputSCORE(BOOL *pChange);

BOOL Soccer2_TestForMovement();

#endif

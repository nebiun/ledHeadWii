/*

LEDhead
Copyright 2001, Peter Hirschberg
Author: Peter Hirschberg

The current version of this SOURCE CODE as well as the official
versions of the LEDHEAD APPLICATION are available from my website
at: http://www.peterhirschberg.com

Based on the handheld electronic games by Mattel Electronics.
All trademarks copyrighted by their respective owners. This
program is not affiliated or endorsed by Mattel Electronics.

License agreement:

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (license.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Email : peter@peterhirschberg.com
Website : http://www.peterhirschberg.com

*/

#ifndef __PLATFORM_FOOTBALL2_H__
#define __PLATFORM_FOOTBALL2_H__

// [general]
#define football2_digit_spacing 12
#define football2_digit_x 57
#define football2_digit_y 44
#define football2_digit_w 8
#define football2_digit_h 9
#define football2_blip_xspacing 1925
#define football2_blip_yspacing 1900
#define football2_blip_x 31
#define football2_blip_y 93
#define football2_power_off_x 91
#define football2_power_off_y 295
#define football2_pro_1_x 91
#define football2_pro_1_y 295
#define football2_pro_2_x 91
#define football2_pro_2_y 295

// Skills
#define LVL_ROOKIESLOW		0		// Pro1
#define LVL_ROOKIEFAST		1		// Pro2
#define LVL_PROSLOW			2		// Pro1 + Kick button
#define LVL_PROFAST			3		// Pro2 + Kick button

#define IS_LVLPRO(x)		(((x) == LVL_PROSLOW) || ((x) == LVL_PROFAST))

// interface that the platform must provide for this game

// functions exported to the game context
void Football2_Init();
void Football2_Help();
void Football2_DeInit();
void Football2_Paint();
void Football2_ClearScreen();
void Football2_PlaySound(int nSound, unsigned int nFlags);
void Football2_StopSound();
void Football2_GetSize(int *w, int *h);

// "private" functions not exported to game context
void Football2_DrawBlip(int nBright, int x, int y);
void Football2_DrawScores(int nHScore, int nVScore);
void Football2_DrawDown(int nDown, int nYards);
void Football2_DrawTime(float fTime);
void Football2_DrawYard(int nYard);

BOOL Football2_GetInputLEFT(BOOL *pChange);
BOOL Football2_GetInputUP(BOOL *pChange);
BOOL Football2_GetInputRIGHT(BOOL *pChange);
BOOL Football2_GetInputDOWN(BOOL *pChange);
BOOL Football2_GetInputKICK(BOOL *pChange);
BOOL Football2_GetInputPASS(BOOL *pChange);
BOOL Football2_GetInputSTATUS(BOOL *pChange);
BOOL Football2_GetInputSCORE(BOOL *pChange);
BOOL Football2_TestForMovement();

#endif

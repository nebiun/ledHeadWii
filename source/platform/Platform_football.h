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


#ifndef __PLATFORM_FOOTBALL_H__
#define __PLATFORM_FOOTBALL_H__
// [general]
#define football_digit_spacing 12
#define football_digit_x 56
#define football_digit_y 47
#define football_digit_w 8
#define football_digit_h 9
#define football_blip_xspacing 2085
#define football_blip_yspacing 1600
#define football_blip_x 35
#define football_blip_y 87
#define football_power_off_x 82
#define football_power_off_y 281
#define football_pro_1_x 82
#define football_pro_1_y 281
#define football_pro_2_x 82
#define football_pro_2_y 281

// interface that the platform must provide for this game

// functions exported to the game context
void Football_Init();
void Football_Help();
void Football_DeInit();
void Football_Paint();
void Football_ClearScreen();
void Football_PlaySound(int nSound, unsigned int nFlags);
void Football_StopSound();
void Football_GetSize(int *w, int *h);

// "private" functions not exported to game context
void Football_DrawBlip(int nBright, int x, int y);
void Football_DrawScores(int nHScore, int nVScore);
void Football_DrawDown(int nDown, int nYards);
void Football_DrawTime(float fTime);
void Football_DrawYard(int nYard);

BOOL Football_GetInputRUN(BOOL *pChange);
BOOL Football_GetInputUP(BOOL *pChange);
BOOL Football_GetInputDOWN(BOOL *pChange);
BOOL Football_GetInputKICK(BOOL *pChange);
BOOL Football_GetInputSTATUS(BOOL *pChange);
BOOL Football_GetInputSCORE(BOOL *pChange);
BOOL Football_TestForMovement();

#endif

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

#ifndef __PLATFORM_HOCKEY_H__
#define __PLATFORM_HOCKEY_H__
// [general]
#define hockey_digit_spacing 12
#define hockey_digit_x 112
#define hockey_digit_y 200
#define hockey_digit_w 8
#define hockey_digit_h 9
#define hockey_blip_xspacing 3950
#define hockey_blip_yspacing 4000
#define hockey_blip_x 40
#define hockey_blip_y 50
#define hockey_power_off_x 27
#define hockey_power_off_y 285
#define hockey_pro_1_x 27
#define hockey_pro_1_y 285
#define hockey_pro_2_x 27
#define hockey_pro_2_y 285

// interface that the platform must provide for this game

// functions exported to the game context
void Hockey_Init();
void Hockey_Help();
void Hockey_DeInit();
void Hockey_Paint();
void Hockey_ClearScreen();
void Hockey_PlaySound(int nSound, unsigned int nFlags);
void Hockey_StopSound();
void Hockey_GetSize(int *w, int *h);

// "private" functions not exported to game context
void Hockey_DrawBlip(int nBright, int x, int y);
void Hockey_DrawStat(int nStat);

BOOL Hockey_GetInputLEFT(BOOL *pChange);
BOOL Hockey_GetInputUP(BOOL *pChange);
BOOL Hockey_GetInputRIGHT(BOOL *pChange);
BOOL Hockey_GetInputDOWN(BOOL *pChange);
BOOL Hockey_GetInputTHROW(BOOL *pChange);
BOOL Hockey_TestForMovement();

#endif

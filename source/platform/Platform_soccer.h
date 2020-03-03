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


#ifndef __PLATFORM_SOCCER_H__
#define __PLATFORM_SOCCER_H__

// [general]
#define soccer_digit_spacing 12
#define soccer_digit_x 112
#define soccer_digit_y 194
#define soccer_digit_w 8
#define soccer_digit_h 9
#define soccer_blip_xspacing 3950
#define soccer_blip_yspacing 4000
#define soccer_blip_x 41
#define soccer_blip_y 46
#define soccer_power_off_x 27
#define soccer_power_off_y 285
#define soccer_pro_1_x 27
#define soccer_pro_1_y 285
#define soccer_pro_2_x 27
#define soccer_pro_2_y 285
#define soccer_goal_x 120
#define soccer_goal_y 27

// interface that the platform must provide for this game

// functions exported to the game context
void Soccer_Init();
void Soccer_Help();
void Soccer_DeInit();
void Soccer_Paint();
void Soccer_ClearScreen();
void Soccer_PlaySound(int nSound, unsigned int nFlags);
void Soccer_StopSound();
void Soccer_GetSize(int *w, int *h);

// "private" functions not exported to game context
void Soccer_DrawBlip(int nBright, int x, int y);
void Soccer_DrawStat(int nStat);
void Soccer_DrawGoal(BOOL bBasket);

BOOL Soccer_GetInputLEFT(BOOL *pChange);
BOOL Soccer_GetInputUP(BOOL *pChange);
BOOL Soccer_GetInputRIGHT(BOOL *pChange);
BOOL Soccer_GetInputDOWN(BOOL *pChange);
BOOL Soccer_GetInputTHROW(BOOL *pChange);
BOOL Soccer_TestForMovement();

#endif

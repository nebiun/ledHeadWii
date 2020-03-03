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

#ifndef __PLATFORM_HOCKEYCA_H__
#define __PLATFORM_HOCKEYCA_H__

// [general]
#define hockeyca_digit_spacing 12
#define hockeyca_digit_x 112
#define hockeyca_digit_y 200
#define hockeyca_digit_w 8
#define hockeyca_digit_h 9
#define hockeyca_blip_xspacing 3950
#define hockeyca_blip_yspacing 3900
#define hockeyca_blip_x 40
#define hockeyca_blip_y 55
#define hockeyca_power_off_x 27
#define hockeyca_power_off_y 285
#define hockeyca_pro_1_x 27
#define hockeyca_pro_1_y 285
#define hockeyca_pro_2_x 27
#define hockeyca_pro_2_y 285
#define hockeyca_goal_x 120
#define hockeyca_goal_y 33

// interface that the platform must provide for this game

// functions exported to the game context
void HockeyCa_Init();
void HockeyCa_Help();
void HockeyCa_DeInit();
void HockeyCa_Paint();
void HockeyCa_ClearScreen();
void HockeyCa_PlaySound(int nSound, unsigned int nFlags);
void HockeyCa_StopSound();
void HockeyCa_GetSize(int *w, int *h);

// "private" functions not exported to game context
void HockeyCa_DrawBlip(int nBright, int x, int y);
void HockeyCa_DrawStat(int nStat);
void HockeyCa_DrawGoal(BOOL bBasket);

BOOL HockeyCa_GetInputLEFT(BOOL *pChange);
BOOL HockeyCa_GetInputUP(BOOL *pChange);
BOOL HockeyCa_GetInputRIGHT(BOOL *pChange);
BOOL HockeyCa_GetInputDOWN(BOOL *pChange);
BOOL HockeyCa_GetInputTHROW(BOOL *pChange);
BOOL HockeyCa_TestForMovement();

#endif
